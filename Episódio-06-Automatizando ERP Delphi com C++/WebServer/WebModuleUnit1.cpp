//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "WebModuleUnit1.h"

#include <System.SysUtils.hpp>
#include <System.DateUtils.hpp>
#include <System.JSON.hpp>

#include <mutex>
#include <vector>
#include <fstream>
#include <string>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TComponentClass WebModuleClass = __classid(TWebModule1);
//---------------------------------------------------------------------------

namespace
{
    std::mutex GStateMutex;

    struct THeartbeatState
    {
        bool Received = false;
        TDateTime LastHeartbeat = 0;
        UnicodeString ReportedStatus = L"simulated";
        UnicodeString Version = L"";
        UnicodeString Source = L"";
        bool CameraOnline = false;
        bool FaceEngineOnline = false;
    };

    THeartbeatState GErp;
    THeartbeatState GTerminal;

    struct TIncidentState
    {
        bool Open = false;
        UnicodeString IncidentId;
        TDateTime OpenedAt = 0;
    };

    TIncidentState GErpIncident;
    TIncidentState GTerminalIncident;

    const int HEARTBEAT_TIMEOUT_SECONDS = 15;

    struct TObservabilityEvent
    {
        UnicodeString Timestamp;
        UnicodeString Source;
        UnicodeString Level;
        UnicodeString Type;
        UnicodeString CorrelationId;
        UnicodeString Message;
        UnicodeString Employee;
        UnicodeString Result;
        double DurationMs = 0.0;
    };

    std::vector<TObservabilityEvent> GEvents;

    // v0.7.0
    // O arquivo JSONL é o histórico persistente.
    // A memória mantém apenas os eventos mais recentes para a API/dashboard.
    const size_t MAX_EVENTS_IN_MEMORY = 1000;
    const int DASHBOARD_EVENT_LIMIT = 20;

    bool GEventStoreLoaded = false;

    UnicodeString EventStoreDirectory()
    {
        return IncludeTrailingPathDelimiter(
            ExtractFilePath(ParamStr(0))) + L"data";
    }

    UnicodeString EventStorePath()
    {
        return IncludeTrailingPathDelimiter(
            EventStoreDirectory()) +
            L"observability-events.jsonl";
    }

    std::string ToUtf8Path(const UnicodeString& Value)
    {
        return UTF8String(Value).c_str();
    }

    struct TBiometricCommand
    {
        bool Pending = false;
        bool Dispatched = false;
        UnicodeString RequestId;
        int EmployeeId = 0;
        UnicodeString EmployeeName;
        UnicodeString Terminal;
        UnicodeString RequestedAt;
        UnicodeString Status;
        UnicodeString Message;
        int Samples = 0;
    };

    TBiometricCommand GEnrollmentCommand;
    TBiometricCommand GRevokeCommand;

    UnicodeString NewRequestId(const UnicodeString& Prefix)
    {
        return Prefix + L"-" +
            FormatDateTime(L"yyyymmddhhnnsszzz", Now());
    }

    UnicodeString JsonEscape(const UnicodeString& Value);
    UnicodeString JsonString(const UnicodeString& Value);

    UnicodeString InvariantNumber(double Value)
    {
        // FloatToStr respeita a localidade do Windows.
        // JSON exige ponto decimal.
        return StringReplace(
            FloatToStr(Value),
            L",",
            L".",
            TReplaceFlags() << rfReplaceAll);
    }

    UnicodeString EventJson(const TObservabilityEvent& Event)
    {
        return
            L"{"
            L"\"timestamp\":" + JsonString(Event.Timestamp) + L","
            L"\"source\":" + JsonString(Event.Source) + L","
            L"\"level\":" + JsonString(Event.Level) + L","
            L"\"type\":" + JsonString(Event.Type) + L","
            L"\"correlationId\":" + JsonString(Event.CorrelationId) + L","
            L"\"message\":" + JsonString(Event.Message) + L","
            L"\"employee\":" + JsonString(Event.Employee) + L","
            L"\"result\":" + JsonString(Event.Result) + L","
            L"\"durationMs\":" + InvariantNumber(Event.DurationMs) +
            L"}";
    }

    UnicodeString HtmlEscape(const UnicodeString& Value)
    {
        UnicodeString Result = Value;
        Result = StringReplace(Result, L"&", L"&amp;",
            TReplaceFlags() << rfReplaceAll);
        Result = StringReplace(Result, L"<", L"&lt;",
            TReplaceFlags() << rfReplaceAll);
        Result = StringReplace(Result, L">", L"&gt;",
            TReplaceFlags() << rfReplaceAll);
        Result = StringReplace(Result, L"\"", L"&quot;",
            TReplaceFlags() << rfReplaceAll);
        return Result;
    }

    bool IsAccessLogEvent(const TObservabilityEvent& Event)
    {
        const UnicodeString Type = UpperCase(Trim(Event.Type));

        return
            Type.Pos(L"FACE_") == 1 ||
            Type.Pos(L"RFID_") == 1 ||
            Type.Pos(L"ACCESS_") == 1;
    }

    UnicodeString AccessLogJson(
        const std::vector<TObservabilityEvent>& Events)
    {
        UnicodeString Json =
            L"{"
            L"\"source\":\"observability-event-store\","
            L"\"persistent\":true,"
            L"\"store\":\"data\\\\observability-events.jsonl\","
            L"\"events\":[";

        int Added = 0;

        for (const TObservabilityEvent& Event : Events)
        {
            if (!IsAccessLogEvent(Event))
                continue;

            if (Added > 0)
                Json += L",";

            Json += EventJson(Event);
            ++Added;
        }

        Json += L"],\"count\":" + IntToStr(Added) + L"}";
        return Json;
    }

    UnicodeString AccessLogHtml(
        const std::vector<TObservabilityEvent>& Events)
    {
        UnicodeString Rows;
        int Added = 0;

        for (const TObservabilityEvent& Event : Events)
        {
            if (!IsAccessLogEvent(Event))
                continue;

            UnicodeString LevelClass = LowerCase(Trim(Event.Level));
            if (LevelClass != L"success" &&
                LevelClass != L"warn" &&
                LevelClass != L"error" &&
                LevelClass != L"info")
            {
                LevelClass = L"info";
            }

            UnicodeString FilterFlags = L"";
            const UnicodeString UpperType = UpperCase(Event.Type);
            const UnicodeString UpperResult = UpperCase(Event.Result);
            const UnicodeString UpperMessage = UpperCase(Event.Message);

            if (UpperType.Pos(L"FACE") > 0)
                FilterFlags += L" FACE";

            if (UpperType.Pos(L"RFID") > 0)
                FilterFlags += L" RFID";

            if (UpperType.Pos(L"GRANTED") > 0 ||
                UpperResult.Pos(L"GRANTED") > 0 ||
                UpperResult.Pos(L"CONFIRMED") > 0 ||
                UpperMessage.Pos(L"AUTORIZADO") > 0)
            {
                FilterFlags += L" GRANTED";
            }

            if (UpperType.Pos(L"DENIED") > 0 ||
                UpperResult.Pos(L"DENIED") > 0 ||
                UpperMessage.Pos(L"NEGADO") > 0 ||
                UpperResult.Pos(L"FORA HORARIO") > 0 ||
                UpperResult.Pos(L"BLOQUE") > 0)
            {
                FilterFlags += L" DENIED";
            }

            if (UpperType.Pos(L"OFFLINE") > 0 ||
                UpperResult.Pos(L"ERP_UNAVAILABLE") > 0 ||
                UpperMessage.Pos(L"CONTING") > 0)
            {
                FilterFlags += L" OFFLINE";
            }

            Rows +=
                L"<tr data-filter='" + HtmlEscape(FilterFlags) + L"'>"
                L"<td class='time'>" + HtmlEscape(Event.Timestamp) + L"</td>"
                L"<td>" + HtmlEscape(Event.Source) + L"</td>"
                L"<td><span class='badge " + LevelClass + L"'>" +
                    HtmlEscape(Event.Level) + L"</span></td>"
                L"<td class='type'>" + HtmlEscape(Event.Type) + L"</td>"
                L"<td>" + HtmlEscape(Event.Employee) + L"</td>"
                L"<td>" + HtmlEscape(Event.Result) + L"</td>"
                L"<td>" + HtmlEscape(Event.Message) + L"</td>"
                L"<td class='num'>" + InvariantNumber(Event.DurationMs) + L" ms</td>"
                L"</tr>";

            ++Added;
        }

        if (Added == 0)
        {
            Rows =
                L"<tr><td colspan='8' class='empty'>"
                L"Nenhum evento de acesso recebido ainda."
                L"</td></tr>";
        }

        UnicodeString Html = LR"HTML(
<!doctype html>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Access Log | Rad Studio FLORENCE 13.1 Florence</title>
<style>
:root{--bg:#050e18;--panel:#0b1c2b;--line:#16415f;--blue:#27b8ff;--green:#20d89a;--yellow:#f5bd32;--red:#e34d5d;--text:#f4f8fc;--muted:#9db7ca}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font-family:Segoe UI,Arial,sans-serif}header{padding:22px 34px;border-bottom:1px solid var(--line);background:#030a12;display:flex;justify-content:space-between;gap:20px;align-items:center}.brand small{color:var(--blue);font-weight:700;letter-spacing:2px}.brand h1{margin:4px 0 2px;font-size:28px}.brand p{margin:0;color:var(--muted)}nav a{color:var(--text);text-decoration:none;border:1px solid var(--line);padding:9px 13px;border-radius:9px;margin-left:8px}nav a:hover{border-color:var(--blue)}main{padding:26px 34px}.summary{display:flex;gap:12px;margin-bottom:18px}.chip{border:1px solid var(--line);background:var(--panel);border-radius:10px;padding:10px 14px;color:var(--muted)}.chip strong{color:var(--text)}.tablewrap{border:1px solid var(--line);border-radius:14px;overflow:auto;background:var(--panel)}table{width:100%;border-collapse:collapse;min-width:1200px}th{position:sticky;top:0;background:#071521;color:var(--muted);font-size:11px;text-transform:uppercase;letter-spacing:1px;text-align:left;padding:13px;border-bottom:1px solid var(--line)}td{padding:12px 13px;border-bottom:1px solid rgba(22,65,95,.55);font-size:13px;vertical-align:top}tr:hover td{background:rgba(39,184,255,.04)}.time{white-space:nowrap;color:var(--muted)}.type{font-weight:700;color:#dff5ff}.num{text-align:right;white-space:nowrap}.badge{display:inline-block;padding:4px 8px;border-radius:999px;font-size:10px;font-weight:800}.badge.success{color:var(--green);background:rgba(32,216,154,.10)}.badge.warn{color:var(--yellow);background:rgba(245,189,50,.10)}.badge.error{color:var(--red);background:rgba(227,77,93,.10)}.badge.info{color:var(--blue);background:rgba(39,184,255,.10)}.empty{text-align:center;color:var(--muted);padding:38px}footer{padding:18px 34px;color:var(--muted);font-size:12px}.live{color:var(--green);font-weight:700}
.filters{display:flex;flex-wrap:wrap;gap:10px;margin:0 0 18px 0}
.filterbtn{appearance:none;border:1px solid var(--line);background:var(--panel);color:var(--muted);padding:10px 16px;border-radius:10px;font:600 13px Segoe UI,Arial,sans-serif;cursor:pointer;transition:.15s ease}
.filterbtn:hover{border-color:var(--blue);color:var(--text);transform:translateY(-1px)}
.filterbtn.active{border-color:var(--blue);background:rgba(39,184,255,.13);color:#fff;box-shadow:0 0 0 1px rgba(39,184,255,.12) inset}
.filtercount{margin-left:4px;color:var(--blue);font-weight:700}
</style>
</head>
<body>
<header><div class="brand"><small>RAD Studio FLORENCE 13.1 Florence</small><h1>Access Log</h1><p>FACE, RFID e decisões de acesso recebidas pelo WebServer</p></div><nav><a href="/">Dashboard</a><a href="/api/accesslog">JSON API</a></nav></header>
<main><div class="summary"><div class="chip"><strong>)HTML";

        Html += IntToStr(Added);
        Html += LR"HTML(</strong> eventos no histórico completo</div><div class="chip">Origem: <strong>observability-events.jsonl</strong></div><div class="chip">Sem limite de visualização</div><div class="chip"><span class="live">● LIVE</span> atualização a cada 5 s</div></div>
<div class="filters" id="filters">
<button type="button" class="filterbtn active" data-filter="ALL">Todos</button>
<button type="button" class="filterbtn" data-filter="FACE">FACE</button>
<button type="button" class="filterbtn" data-filter="RFID">RFID</button>
<button type="button" class="filterbtn" data-filter="GRANTED">Liberados</button>
<button type="button" class="filterbtn" data-filter="DENIED">Negados</button>
<button type="button" class="filterbtn" data-filter="OFFLINE">Contingência</button>
<span class="chip">Visíveis: <strong id="visibleCount">0</strong></span>
</div>
<div class="tablewrap"><table><thead><tr><th>Horário</th><th>Origem</th><th>Nível</th><th>Tipo</th><th>Funcionário</th><th>Resultado</th><th>Mensagem</th><th>Duração</th></tr></thead><tbody>)HTML";

        Html += Rows;
        Html += LR"HTML(</tbody></table></div></main>
<footer>Somente leitura • observabilidade não participa da decisão de acesso</footer>
<script>
(function(){
  const storageKey = 'accesslog-filter';
  const buttons = Array.from(document.querySelectorAll('.filterbtn'));
  const rows = Array.from(document.querySelectorAll('tbody tr[data-filter]'));
  const count = document.getElementById('visibleCount');

  function applyFilter(filter){
    let visible = 0;

    rows.forEach(function(row){
      const flags = (row.getAttribute('data-filter') || '').toUpperCase();
      const show = filter === 'ALL' || flags.indexOf(filter) !== -1;
      row.style.display = show ? '' : 'none';
      if (show) visible++;
    });

    buttons.forEach(function(btn){
      btn.classList.toggle(
        'active',
        btn.getAttribute('data-filter') === filter
      );
    });

    if (count) count.textContent = String(visible);

    try { localStorage.setItem(storageKey, filter); } catch(e) {}
  }

  buttons.forEach(function(btn){
    btn.addEventListener('click', function(){
      applyFilter(btn.getAttribute('data-filter'));
    });
  });

  let initial = 'ALL';
  try {
    const saved = localStorage.getItem(storageKey);
    if (saved) initial = saved;
  } catch(e) {}

  applyFilter(initial);

  // Atualiza a página mantendo o filtro selecionado.
  window.setTimeout(function(){
    window.location.reload();
  }, 5000);
})();
</script>
</body></html>)HTML";
        return Html;
    }


    bool EventFromJson(
        const UnicodeString& JsonText,
        TObservabilityEvent& Event)
    {
        std::unique_ptr<TJSONValue> Parsed(
            TJSONObject::ParseJSONValue(JsonText));

        TJSONObject* Obj =
            dynamic_cast<TJSONObject*>(Parsed.get());

        if (Obj == nullptr)
            return false;

        Event = TObservabilityEvent();

        TJSONValue* Value = nullptr;

        Value = Obj->GetValue(L"timestamp");
        if (Value != nullptr)
            Event.Timestamp = Value->Value();

        Value = Obj->GetValue(L"source");
        if (Value != nullptr)
            Event.Source = Value->Value();

        Value = Obj->GetValue(L"level");
        if (Value != nullptr)
            Event.Level = Value->Value();

        Value = Obj->GetValue(L"type");
        if (Value != nullptr)
            Event.Type = Value->Value();

        Value = Obj->GetValue(L"correlationId");
        if (Value != nullptr)
            Event.CorrelationId = Value->Value();

        Value = Obj->GetValue(L"message");
        if (Value != nullptr)
            Event.Message = Value->Value();

        Value = Obj->GetValue(L"employee");
        if (Value != nullptr)
            Event.Employee = Value->Value();

        Value = Obj->GetValue(L"result");
        if (Value != nullptr)
            Event.Result = Value->Value();

        Value = Obj->GetValue(L"durationMs");
        if (Value != nullptr)
        {
            UnicodeString DurationText = Value->Value();
            DurationText = StringReplace(
                DurationText,
                L".",
                FormatSettings.DecimalSeparator,
                TReplaceFlags() << rfReplaceAll);

            Event.DurationMs =
                StrToFloatDef(DurationText, 0.0);
        }

        return !Event.Timestamp.IsEmpty();
    }

    // ACCESSLOG: carrega TODO o histórico persistente diretamente do JSONL.
    // Não usa o limite de eventos mantidos em memória pelo Dashboard.
    std::vector<TObservabilityEvent> LoadAllPersistentAccessEvents()
    {
        std::vector<TObservabilityEvent> Events;

        try
        {
            const UnicodeString Path = EventStorePath();

            if (!FileExists(Path))
                return Events;

            std::ifstream Stream(
                ToUtf8Path(Path),
                std::ios::binary);

            if (!Stream)
                return Events;

            std::string Line;

            while (std::getline(Stream, Line))
            {
                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (Line.empty())
                    continue;

                const UTF8String Utf8 = Line.c_str();
                const UnicodeString Text = UnicodeString(Utf8);

                TObservabilityEvent Event;

                if (EventFromJson(Text, Event) &&
                    IsAccessLogEvent(Event))
                {
                    Events.insert(Events.begin(), Event);
                }
            }
        }
        catch (...)
        {
            // Falha de leitura do histórico não derruba o WebServer.
        }

        return Events;
    }

    void EnsureEventStoreDirectory()
    {
        const UnicodeString Directory =
            EventStoreDirectory();

        if (!DirectoryExists(Directory))
            ForceDirectories(Directory);
    }

    void AppendPersistentEvent(
        const TObservabilityEvent& Event)
    {
        try
        {
            EnsureEventStoreDirectory();

            std::ofstream Stream(
                ToUtf8Path(EventStorePath()),
                std::ios::binary | std::ios::app);

            if (!Stream)
                return;

            const UTF8String Utf8 =
                UTF8String(EventJson(Event));

            Stream.write(
                Utf8.c_str(),
                Utf8.Length());

            Stream.write("\r\n", 2);
        }
        catch (...)
        {
            // Observabilidade nunca deve derrubar o servidor por falha de disco.
        }
    }

    void AddEvent(
        const TObservabilityEvent& Event,
        bool Persist = true)
    {
        GEvents.insert(GEvents.begin(), Event);

        if (GEvents.size() > MAX_EVENTS_IN_MEMORY)
            GEvents.resize(MAX_EVENTS_IN_MEMORY);

        if (Persist)
            AppendPersistentEvent(Event);
    }

    void LoadPersistentEvents()
    {
        if (GEventStoreLoaded)
            return;

        GEventStoreLoaded = true;

        try
        {
            const UnicodeString Path =
                EventStorePath();

            if (!FileExists(Path))
                return;

            std::ifstream Stream(
                ToUtf8Path(Path),
                std::ios::binary);

            if (!Stream)
                return;

            std::string Line;

            while (std::getline(Stream, Line))
            {
                if (!Line.empty() &&
                    Line.back() == '\r')
                {
                    Line.pop_back();
                }

                if (Line.empty())
                    continue;

                const UTF8String Utf8 =
                    Line.c_str();

                const UnicodeString Text =
                    UnicodeString(Utf8);

                TObservabilityEvent Event;

                if (EventFromJson(Text, Event))
                    AddEvent(Event, false);
            }
        }
        catch (...)
        {
            // Se o arquivo estiver inválido, o WebServer continua operando.
        }
    }

    // Forward declarations:
    // Funções usadas antes de suas implementações.
    UnicodeString IsoNow();
    int AgeSeconds(
        const THeartbeatState& State,
        const TDateTime& NowValue);

    void AddSimpleEvent(
        const UnicodeString& Source,
        const UnicodeString& Level,
        const UnicodeString& Type,
        const UnicodeString& Message,
        const UnicodeString& Employee,
        const UnicodeString& Result)
    {
        TObservabilityEvent Event;
        Event.Timestamp = IsoNow();
        Event.Source = Source;
        Event.Level = Level;
        Event.Type = Type;
        Event.Message = Message;
        Event.Employee = Employee;
        Event.Result = Result;
        Event.DurationMs = 0.0;
        AddEvent(Event);
    }


    UnicodeString JsonEscape(const UnicodeString& Value)
    {
        UnicodeString Result = Value;
        Result = StringReplace(Result, L"\\", L"\\\\",
            TReplaceFlags() << rfReplaceAll);
        Result = StringReplace(Result, L"\"", L"\\\"",
            TReplaceFlags() << rfReplaceAll);
        Result = StringReplace(Result, L"\r", L"\\r",
            TReplaceFlags() << rfReplaceAll);
        Result = StringReplace(Result, L"\n", L"\\n",
            TReplaceFlags() << rfReplaceAll);
        return Result;
    }

    UnicodeString JsonString(const UnicodeString& Value)
    {
        return L"\"" + JsonEscape(Value) + L"\"";
    }

    UnicodeString IsoNow()
    {
        return FormatDateTime(L"yyyy-mm-dd\"T\"hh:nn:ss", Now());
    }

    UnicodeString IsoDateTime(const TDateTime& Value)
    {
        if (Value.Val <= 0.0)
            return L"";

        return FormatDateTime(L"yyyy-mm-dd\"T\"hh:nn:ss", Value);
    }

    double DateTimeDiffMs(
        const TDateTime& Later,
        const TDateTime& Earlier)
    {
        if (Later.Val <= 0.0 ||
            Earlier.Val <= 0.0 ||
            Later.Val <= Earlier.Val)
        {
            return 0.0;
        }

        return
            (Later.Val - Earlier.Val) *
            24.0 * 60.0 * 60.0 * 1000.0;
    }

    double ElapsedTodayMs(
        const TDateTime& NowValue)
    {
        const int WholeDays =
            static_cast<int>(NowValue.Val);

        const double Fraction =
            NowValue.Val - WholeDays;

        if (Fraction <= 0.0)
            return 0.0;

        return Fraction *
            24.0 * 60.0 * 60.0 * 1000.0;
    }

    UnicodeString IncidentIdAt(
        const UnicodeString& Prefix,
        const TDateTime& Value)
    {
        return Prefix + L"-" +
            FormatDateTime(
                L"yyyymmddhhnnss",
                Value);
    }

    void AddIncidentLifecycleEvent(
        const UnicodeString& Source,
        const UnicodeString& Level,
        const UnicodeString& Type,
        const UnicodeString& CorrelationId,
        const UnicodeString& Message,
        const UnicodeString& Result,
        const TDateTime& Timestamp,
        double DurationMs)
    {
        TObservabilityEvent Event;
        Event.Timestamp = IsoDateTime(Timestamp);
        Event.Source = Source;
        Event.Level = Level;
        Event.Type = Type;
        Event.CorrelationId = CorrelationId;
        Event.Message = Message;
        Event.Employee = L"";
        Event.Result = Result;
        Event.DurationMs = DurationMs;
        AddEvent(Event);
    }

    bool HeartbeatStateIsOffline(
        const THeartbeatState& State,
        const TDateTime& NowValue)
    {
        if (!State.Received)
            return false;

        const int Age =
            AgeSeconds(State, NowValue);

        return
            (Age > HEARTBEAT_TIMEOUT_SECONDS) ||
            SameText(State.ReportedStatus, L"offline");
    }

    TDateTime IncidentOpenTime(
        const THeartbeatState& State,
        const TDateTime& NowValue)
    {
        if (SameText(State.ReportedStatus, L"offline"))
        {
            if (State.LastHeartbeat.Val > 0.0)
                return State.LastHeartbeat;

            return NowValue;
        }

        if (State.LastHeartbeat.Val > 0.0)
        {
            return IncSecond(
                State.LastHeartbeat,
                HEARTBEAT_TIMEOUT_SECONDS);
        }

        return NowValue;
    }

    void OpenIncidentIfNeeded(
        const THeartbeatState& State,
        TIncidentState& Incident,
        const TDateTime& NowValue,
        const UnicodeString& DefaultSource,
        const UnicodeString& ComponentName,
        const UnicodeString& IdPrefix)
    {
        if (!HeartbeatStateIsOffline(
            State,
            NowValue))
        {
            return;
        }

        if (Incident.Open)
            return;

        Incident.Open = true;
        Incident.OpenedAt =
            IncidentOpenTime(
                State,
                NowValue);

        Incident.IncidentId =
            IncidentIdAt(
                IdPrefix,
                Incident.OpenedAt);

        const UnicodeString Source =
            State.Source.IsEmpty()
                ? DefaultSource
                : State.Source;

        AddIncidentLifecycleEvent(
            Source,
            L"WARN",
            L"INCIDENT_OPENED",
            Incident.IncidentId,
            ComponentName + L" sem comunicação",
            L"OPEN",
            Incident.OpenedAt,
            0.0);
    }

    void ResolveIncidentIfNeeded(
        TIncidentState& Incident,
        const THeartbeatState& State,
        const TDateTime& RecoveredAt,
        const UnicodeString& DefaultSource,
        const UnicodeString& ComponentName)
    {
        if (!Incident.Open)
            return;

        const double DurationMs =
            DateTimeDiffMs(
                RecoveredAt,
                Incident.OpenedAt);

        const UnicodeString Source =
            State.Source.IsEmpty()
                ? DefaultSource
                : State.Source;

        AddIncidentLifecycleEvent(
            Source,
            L"SUCCESS",
            L"INCIDENT_RESOLVED",
            Incident.IncidentId,
            ComponentName + L" comunicação restabelecida",
            L"RESOLVED",
            RecoveredAt,
            DurationMs);

        Incident = TIncidentState();
    }

    void EvaluateCurrentIncidents(
        const TDateTime& NowValue)
    {
        OpenIncidentIfNeeded(
            GErp,
            GErpIncident,
            NowValue,
            L"ERP-DELPHI",
            L"ERP Delphi",
            L"INC-ERP");

        OpenIncidentIfNeeded(
            GTerminal,
            GTerminalIncident,
            NowValue,
            L"PORTARIA-01",
            L"Portaria C++",
            L"INC-PORTARIA");
    }

    bool ReadJsonString(
        TJSONObject* Obj,
        const UnicodeString& Name,
        UnicodeString& Value)
    {
        Value = L"";
        if (Obj == nullptr)
            return false;

        TJSONValue* JsonValue = Obj->GetValue(Name);
        if (JsonValue == nullptr)
            return false;

        Value = JsonValue->Value();
        return true;
    }

    bool ReadJsonBool(
        TJSONObject* Obj,
        const UnicodeString& Name,
        bool& Value)
    {
        if (Obj == nullptr)
            return false;

        TJSONValue* JsonValue = Obj->GetValue(Name);
        if (JsonValue == nullptr)
            return false;

        const UnicodeString Text = LowerCase(JsonValue->Value());
        if (Text == L"true" || Text == L"1")
        {
            Value = true;
            return true;
        }

        if (Text == L"false" || Text == L"0")
        {
            Value = false;
            return true;
        }

        return false;
    }

    int AgeSeconds(const THeartbeatState& State, const TDateTime& NowValue)
    {
        if (!State.Received || State.LastHeartbeat.Val <= 0.0)
            return -1;

        return static_cast<int>(
            SecondsBetween(NowValue, State.LastHeartbeat));
    }

    UnicodeString EffectiveStatus(
        const THeartbeatState& State,
        const TDateTime& NowValue,
        int& Age,
        bool& IncidentOpen)
    {
        Age = AgeSeconds(State, NowValue);
        IncidentOpen = false;

        if (!State.Received)
            return L"simulated";

        if (Age > HEARTBEAT_TIMEOUT_SECONDS ||
            SameText(State.ReportedStatus, L"offline"))
        {
            IncidentOpen = true;
            return L"offline";
        }

        return L"online";
    }

    struct TOperationalMetrics
    {
        int EventsToday = 0;
        int ErrorsToday = 0;
        int EmployeeCreatedToday = 0;
        int EmployeeUpdatedToday = 0;
        int EmployeeDeletedToday = 0;
        int EnrollmentRequestsToday = 0;
        int EnrollmentCompletedToday = 0;
        int EnrollmentFailedToday = 0;
        int FaceRecognitionsToday = 0;
        int EmployeeSyncToday = 0;
        int OpenIncidents = 0;
        int IncidentsOpenedToday = 0;
        int IncidentsResolvedToday = 0;
        double ErpDowntimeMsToday = 0.0;
        double TerminalDowntimeMsToday = 0.0;
        double ErpAvailabilityPercent = 100.0;
        double TerminalAvailabilityPercent = 100.0;
        double FaceRecognitionDurationTotalMs = 0.0;
        int FaceRecognitionDurationSamples = 0;
    };

    bool EventIsToday(
        const TObservabilityEvent& Event,
        const UnicodeString& TodayKey)
    {
        if (Event.Timestamp.Length() < 10)
            return false;

        return SameText(
            Event.Timestamp.SubString(1, 10),
            TodayKey);
    }

    TOperationalMetrics BuildOperationalMetrics(
        const TDateTime& NowValue)
    {
        TOperationalMetrics Metrics;

        const UnicodeString TodayKey =
            FormatDateTime(L"yyyy-mm-dd", NowValue);

        for (const TObservabilityEvent& Event : GEvents)
        {
            if (!EventIsToday(Event, TodayKey))
                continue;

            ++Metrics.EventsToday;

            if (SameText(Event.Level, L"ERROR"))
                ++Metrics.ErrorsToday;

            if (SameText(Event.Type, L"INCIDENT_OPENED"))
            {
                ++Metrics.IncidentsOpenedToday;
            }
            else if (SameText(Event.Type, L"INCIDENT_RESOLVED"))
            {
                ++Metrics.IncidentsResolvedToday;

                if (Event.DurationMs > 0.0)
                {
                    const UnicodeString Source =
                        UpperCase(Event.Source);

                    if (Source.Pos(L"DELPHI") > 0)
                    {
                        Metrics.ErpDowntimeMsToday +=
                            Event.DurationMs;
                    }
                    else if (
                        Source.Pos(L"PORTARIA") > 0 ||
                        Source.Pos(L"CPP") > 0)
                    {
                        Metrics.TerminalDowntimeMsToday +=
                            Event.DurationMs;
                    }
                }
            }

            if (SameText(Event.Type, L"EMPLOYEE_CREATED"))
                ++Metrics.EmployeeCreatedToday;
            else if (SameText(Event.Type, L"EMPLOYEE_UPDATED"))
                ++Metrics.EmployeeUpdatedToday;
            else if (SameText(Event.Type, L"EMPLOYEE_DELETED"))
                ++Metrics.EmployeeDeletedToday;
            else if (SameText(Event.Type, L"ENROLLMENT_REQUEST"))
                ++Metrics.EnrollmentRequestsToday;
            else if (SameText(Event.Type, L"ENROLLMENT_RESULT"))
            {
                if (SameText(Event.Result, L"completed"))
                    ++Metrics.EnrollmentCompletedToday;
                else
                    ++Metrics.EnrollmentFailedToday;
            }
            else if (SameText(Event.Type, L"FACE_RECOGNITION"))
            {
                if (SameText(Event.Level, L"SUCCESS") ||
                    SameText(Event.Result, L"FACE_CONFIRMED"))
                {
                    ++Metrics.FaceRecognitionsToday;
                }

                if (Event.DurationMs > 0.0)
                {
                    Metrics.FaceRecognitionDurationTotalMs +=
                        Event.DurationMs;
                    ++Metrics.FaceRecognitionDurationSamples;
                }
            }
            else if (SameText(Event.Type, L"EMPLOYEE_SYNC"))
            {
                ++Metrics.EmployeeSyncToday;
            }
        }

        EvaluateCurrentIncidents(NowValue);

        if (GErpIncident.Open)
        {
            ++Metrics.OpenIncidents;

            double StartSerial =
                GErpIncident.OpenedAt.Val;

            const double TodayStartSerial =
                static_cast<int>(NowValue.Val);

            if (StartSerial < TodayStartSerial)
                StartSerial = TodayStartSerial;

            if (NowValue.Val > StartSerial)
            {
                Metrics.ErpDowntimeMsToday +=
                    (NowValue.Val - StartSerial) *
                    24.0 * 60.0 * 60.0 * 1000.0;
            }
        }

        if (GTerminalIncident.Open)
        {
            ++Metrics.OpenIncidents;

            double StartSerial =
                GTerminalIncident.OpenedAt.Val;

            const double TodayStartSerial =
                static_cast<int>(NowValue.Val);

            if (StartSerial < TodayStartSerial)
                StartSerial = TodayStartSerial;

            if (NowValue.Val > StartSerial)
            {
                Metrics.TerminalDowntimeMsToday +=
                    (NowValue.Val - StartSerial) *
                    24.0 * 60.0 * 60.0 * 1000.0;
            }
        }

        const double ObservedTodayMs =
            ElapsedTodayMs(NowValue);

        if (ObservedTodayMs > 0.0)
        {
            double ErpDowntime =
                Metrics.ErpDowntimeMsToday;

            double TerminalDowntime =
                Metrics.TerminalDowntimeMsToday;

            if (ErpDowntime > ObservedTodayMs)
                ErpDowntime = ObservedTodayMs;

            if (TerminalDowntime > ObservedTodayMs)
                TerminalDowntime = ObservedTodayMs;

            Metrics.ErpAvailabilityPercent =
                100.0 *
                (ObservedTodayMs - ErpDowntime) /
                ObservedTodayMs;

            Metrics.TerminalAvailabilityPercent =
                100.0 *
                (ObservedTodayMs - TerminalDowntime) /
                ObservedTodayMs;
        }

        return Metrics;
    }

    double AverageRecognitionMs(
        const TOperationalMetrics& Metrics)
    {
        if (Metrics.FaceRecognitionDurationSamples <= 0)
            return 0.0;

        return Metrics.FaceRecognitionDurationTotalMs /
            Metrics.FaceRecognitionDurationSamples;
    }

    UnicodeString CommandJson(
        const TBiometricCommand& Command)
    {
        if (!Command.Pending || Command.Dispatched)
            return L"{\"pending\":false}";

        return
            L"{"
            L"\"pending\":true,"
            L"\"requestId\":" + JsonString(Command.RequestId) + L","
            L"\"employeeId\":" + IntToStr(Command.EmployeeId) + L","
            L"\"employeeName\":" + JsonString(Command.EmployeeName) + L","
            L"\"terminal\":" + JsonString(Command.Terminal) + L","
            L"\"requestedAt\":" + JsonString(Command.RequestedAt) +
            L"}";
    }

    UnicodeString CommandStatusJson(
        const UnicodeString& Operation,
        const TBiometricCommand& Command)
    {
        return
            L"{"
            L"\"operation\":" + JsonString(Operation) + L","
            L"\"pending\":" +
                UnicodeString(Command.Pending ? L"true" : L"false") + L","
            L"\"dispatched\":" +
                UnicodeString(Command.Dispatched ? L"true" : L"false") + L","
            L"\"requestId\":" + JsonString(Command.RequestId) + L","
            L"\"employeeId\":" + IntToStr(Command.EmployeeId) + L","
            L"\"employeeName\":" + JsonString(Command.EmployeeName) + L","
            L"\"terminal\":" + JsonString(Command.Terminal) + L","
            L"\"requestedAt\":" + JsonString(Command.RequestedAt) + L","
            L"\"status\":" + JsonString(Command.Status) + L","
            L"\"message\":" + JsonString(Command.Message) + L","
            L"\"samples\":" + IntToStr(Command.Samples) +
            L"}";
    }
}
//---------------------------------------------------------------------------

__fastcall TWebModule1::TWebModule1(TComponent* Owner)
    : TWebModule(Owner)
{
    std::lock_guard<std::mutex> Lock(GStateMutex);
    LoadPersistentEvents();
}
//---------------------------------------------------------------------------

void __fastcall TWebModule1::WebModule1DefaultHandlerAction(
    TObject *Sender,
    TWebRequest *Request,
    TWebResponse *Response,
    bool &Handled)
{
    (void)Sender;

    const UnicodeString Path = LowerCase(Request->PathInfo);

    if ((Request->MethodType == mtGet) &&
        (Path == L"/api/health"))
    {
        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{"
            L"\"status\":\"ok\","
            L"\"service\":\"TSX Observability Server\","
            L"\"version\":\"0.7.3.2\","
            L"\"environment\":\"conference\","
            L"\"timestamp\":" + JsonString(IsoNow()) +
            L"}";
        Handled = true;
        return;
    }

    // =========================================================
    // POST /api/heartbeat
    // Aceita ERP-DELPHI e PORTARIA-01.
    // =========================================================
    if ((Request->MethodType == mtPost) &&
        (Path == L"/api/heartbeat"))
    {
        std::unique_ptr<TJSONValue> Parsed(
            TJSONObject::ParseJSONValue(Request->Content));

        TJSONObject* Obj =
            dynamic_cast<TJSONObject*>(Parsed.get());

        if (Obj == nullptr)
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"invalid_json\"}";
            Handled = true;
            return;
        }

        UnicodeString Source;
        UnicodeString Status;
        UnicodeString Version;
        bool CameraOnline = false;
        bool FaceEngineOnline = false;

        ReadJsonString(Obj, L"source", Source);
        ReadJsonString(Obj, L"status", Status);
        ReadJsonString(Obj, L"version", Version);
        ReadJsonBool(Obj, L"camera", CameraOnline);
        ReadJsonBool(Obj, L"faceEngine", FaceEngineOnline);

        if (Trim(Source).IsEmpty())
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"source_required\"}";
            Handled = true;
            return;
        }

        const UnicodeString UpperSource = UpperCase(Source);
        const bool IsErp = UpperSource.Pos(L"DELPHI") > 0;
        const bool IsTerminal =
            (UpperSource.Pos(L"PORTARIA") > 0) ||
            (UpperSource.Pos(L"CPP") > 0);

        if (!IsErp && !IsTerminal)
        {
            Response->StatusCode = 422;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"unsupported_source\"}";
            Handled = true;
            return;
        }

        const TDateTime ReceivedAt = Now();

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);

            THeartbeatState& Target =
                IsErp ? GErp : GTerminal;

            TIncidentState& Incident =
                IsErp ? GErpIncident : GTerminalIncident;

            const UnicodeString DefaultSource =
                IsErp ? L"ERP-DELPHI" : L"PORTARIA-01";

            const UnicodeString ComponentName =
                IsErp ? L"ERP Delphi" : L"Portaria C++";

            const UnicodeString IdPrefix =
                IsErp ? L"INC-ERP" : L"INC-PORTARIA";

            // Verifica o intervalo anterior antes de
            // sobrescrever o último heartbeat.
            OpenIncidentIfNeeded(
                Target,
                Incident,
                ReceivedAt,
                DefaultSource,
                ComponentName,
                IdPrefix);

            Target.Received = true;
            Target.LastHeartbeat = ReceivedAt;
            Target.Source = Source;
            Target.ReportedStatus =
                Trim(Status).IsEmpty()
                    ? L"online"
                    : LowerCase(Status);
            Target.Version = Version;

            if (IsTerminal)
            {
                Target.CameraOnline = CameraOnline;
                Target.FaceEngineOnline = FaceEngineOnline;
            }

            if (SameText(
                Target.ReportedStatus,
                L"offline"))
            {
                OpenIncidentIfNeeded(
                    Target,
                    Incident,
                    ReceivedAt,
                    DefaultSource,
                    ComponentName,
                    IdPrefix);
            }
            else
            {
                ResolveIncidentIfNeeded(
                    Incident,
                    Target,
                    ReceivedAt,
                    DefaultSource,
                    ComponentName);
            }
        }

        Response->StatusCode = 201;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{"
            L"\"ok\":true,"
            L"\"source\":" + JsonString(Source) + L","
            L"\"receivedAt\":" + JsonString(IsoDateTime(ReceivedAt)) + L","
            L"\"timeoutSeconds\":" + IntToStr(HEARTBEAT_TIMEOUT_SECONDS) +
            L"}";

        Handled = true;
        return;
    }


    // =========================================================
    // POST /api/events
    // =========================================================
    if ((Request->MethodType == mtPost) &&
        (Path == L"/api/events"))
    {
        std::unique_ptr<TJSONValue> Parsed(
            TJSONObject::ParseJSONValue(Request->Content));

        TJSONObject* Obj =
            dynamic_cast<TJSONObject*>(Parsed.get());

        if (Obj == nullptr)
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"invalid_json\"}";
            Handled = true;
            return;
        }

        TObservabilityEvent Event;
        Event.Timestamp = IsoNow();

        ReadJsonString(Obj, L"source", Event.Source);
        ReadJsonString(Obj, L"level", Event.Level);
        ReadJsonString(Obj, L"type", Event.Type);
        ReadJsonString(Obj, L"correlationId", Event.CorrelationId);
        ReadJsonString(Obj, L"message", Event.Message);
        ReadJsonString(Obj, L"employee", Event.Employee);
        ReadJsonString(Obj, L"result", Event.Result);

        UnicodeString DurationText;
        if (ReadJsonString(Obj, L"durationMs", DurationText))
            Event.DurationMs = StrToFloatDef(DurationText, 0.0);

        if (Trim(Event.Source).IsEmpty())
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"source_required\"}";
            Handled = true;
            return;
        }

        if (Trim(Event.Level).IsEmpty())
            Event.Level = L"INFO";

        if (Trim(Event.Type).IsEmpty())
            Event.Type = L"GENERAL";

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);
            AddEvent(Event);
        }

        Response->StatusCode = 201;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{"
            L"\"ok\":true,"
            L"\"timestamp\":" + JsonString(Event.Timestamp) + L","
            L"\"source\":" + JsonString(Event.Source) +
            L"}";

        Handled = true;
        return;
    }

    // =========================================================
    // GET /api/events
    // =========================================================
    if ((Request->MethodType == mtGet) &&
        (Path == L"/api/events"))
    {
        std::vector<TObservabilityEvent> Events;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);
            Events = GEvents;
        }

        UnicodeString Json =
            L"{"
            L"\"persistent\":true,"
            L"\"store\":\"data\\\\observability-events.jsonl\","
            L"\"retention\":\"until_file_removed\","
            L"\"memoryLimit\":" + IntToStr(
                static_cast<int>(MAX_EVENTS_IN_MEMORY)) + L","
            L"\"count\":" + IntToStr(
                static_cast<int>(Events.size())) + L","
            L"\"events\":[";

        for (size_t I = 0; I < Events.size(); ++I)
        {
            if (I > 0)
                Json += L",";

            Json += EventJson(Events[I]);
        }

        Json += L"]}";

        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content = Json;

        Handled = true;
        return;
    }


    // =========================================================
    // GET /api/accesslog
    // Lista somente eventos relacionados a FACE / RFID / ACCESS.
    // Usa o mesmo EventStore persistente do Dashboard.
    // =========================================================
    if ((Request->MethodType == mtGet) &&
        (Path == L"/api/accesslog"))
    {
        const std::vector<TObservabilityEvent> Events =
            LoadAllPersistentAccessEvents();

        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content = AccessLogJson(Events);
        Handled = true;
        return;
    }

    // =========================================================
    // GET /accesslog
    // Página de apresentação: tabela dos eventos de acesso.
    // =========================================================
    if ((Request->MethodType == mtGet) &&
        (Path == L"/accesslog"))
    {
        const std::vector<TObservabilityEvent> Events =
            LoadAllPersistentAccessEvents();

        Response->StatusCode = 200;
        Response->ContentType = L"text/html; charset=utf-8";
        Response->Content = AccessLogHtml(Events);
        Handled = true;
        return;
    }

    // =========================================================
    // IDENTIFICAÇÃO FACIAL - GESTÃO DE COMANDOS v0.7.0
    // =========================================================

    // ERP -> solicita cadastro de identificação facial.
    if ((Request->MethodType == mtPost) &&
        (Path == L"/api/enrollment/request"))
    {
        std::unique_ptr<TJSONValue> Parsed(
            TJSONObject::ParseJSONValue(Request->Content));
        TJSONObject* Obj = dynamic_cast<TJSONObject*>(Parsed.get());

        if (Obj == nullptr)
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"invalid_json\"}";
            Handled = true;
            return;
        }

        UnicodeString EmployeeIdText;
        UnicodeString EmployeeName;
        UnicodeString Terminal;

        ReadJsonString(Obj, L"employeeId", EmployeeIdText);
        ReadJsonString(Obj, L"employeeName", EmployeeName);
        ReadJsonString(Obj, L"terminal", Terminal);

        const int EmployeeId = StrToIntDef(EmployeeIdText, 0);

        if (EmployeeId <= 0)
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"employee_id_required\"}";
            Handled = true;
            return;
        }

        if (Trim(Terminal).IsEmpty())
            Terminal = L"PORTARIA-01";

        TBiometricCommand Command;
        Command.Pending = true;
        Command.Dispatched = false;
        Command.RequestId = NewRequestId(L"ENR");
        Command.EmployeeId = EmployeeId;
        Command.EmployeeName = EmployeeName;
        Command.Terminal = Terminal;
        Command.RequestedAt = IsoNow();
        Command.Status = L"pending";
        Command.Message = L"Aguardando terminal";
        Command.Samples = 0;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);
            GEnrollmentCommand = Command;

            AddSimpleEvent(
                L"ERP-DELPHI",
                L"INFO",
                L"ENROLLMENT_REQUEST",
                L"Identificação facial solicitada",
                EmployeeName,
                Command.RequestId);
        }

        Response->StatusCode = 201;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{"
            L"\"ok\":true,"
            L"\"requestId\":" + JsonString(Command.RequestId) + L","
            L"\"employeeId\":" + IntToStr(EmployeeId) + L","
            L"\"status\":\"pending\""
            L"}";
        Handled = true;
        return;
    }

    // Portaria -> consulta identificação facial pendente.
    if ((Request->MethodType == mtGet) &&
        (Path == L"/api/enrollment/pending"))
    {
        TBiometricCommand Command;
        {
            std::lock_guard<std::mutex> Lock(GStateMutex);
            Command = GEnrollmentCommand;
        }

        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content = CommandJson(Command);
        Handled = true;
        return;
    }

    // Portaria -> confirma que assumiu o comando e informa progresso 0/8..7/8.
    // O comando deixa de ser reentregue, mas continua operacionalmente aberto
    // até /api/enrollment/result concluir ou falhar.
    if ((Request->MethodType == mtPost) &&
        (Path == L"/api/enrollment/progress"))
    {
        std::unique_ptr<TJSONValue> Parsed(
            TJSONObject::ParseJSONValue(Request->Content));
        TJSONObject* Obj = dynamic_cast<TJSONObject*>(Parsed.get());

        if (Obj == nullptr)
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"invalid_json\"}";
            Handled = true;
            return;
        }

        UnicodeString RequestId;
        UnicodeString Message;
        UnicodeString SamplesText;

        ReadJsonString(Obj, L"requestId", RequestId);
        ReadJsonString(Obj, L"message", Message);
        ReadJsonString(Obj, L"samples", SamplesText);

        const int Samples =
            StrToIntDef(SamplesText, 0);

        bool Found = false;
        bool FirstDispatch = false;
        TBiometricCommand Current;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);

            if (!GEnrollmentCommand.RequestId.IsEmpty() &&
                SameText(GEnrollmentCommand.RequestId, RequestId) &&
                GEnrollmentCommand.Pending)
            {
                FirstDispatch =
                    !GEnrollmentCommand.Dispatched;

                GEnrollmentCommand.Dispatched = true;
                GEnrollmentCommand.Status = L"in_progress";
                GEnrollmentCommand.Message =
                    Trim(Message).IsEmpty()
                        ? L"Portaria capturando identificação facial"
                        : Message;
                GEnrollmentCommand.Samples =
                    Samples < 0 ? 0 : Samples;

                Current = GEnrollmentCommand;
                Found = true;

                // Um único START por RequestId. As atualizações 1/8..7/8
                // ficam no status, evitando poluir o histórico persistente.
                if (FirstDispatch)
                {
                    AddSimpleEvent(
                        L"PORTARIA-01",
                        L"INFO",
                        L"ENROLLMENT_STARTED",
                        L"Cadastro de identificação facial iniciado na Portaria",
                        GEnrollmentCommand.EmployeeName,
                        GEnrollmentCommand.RequestId);
                }
            }
        }

        if (!Found)
        {
            Response->StatusCode = 404;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"request_not_found_or_closed\"}";
            Handled = true;
            return;
        }

        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{"
            L"\"ok\":true,"
            L"\"status\":\"in_progress\","
            L"\"samples\":" + IntToStr(Current.Samples) +
            L"}";

        Handled = true;
        return;
    }

    // Portaria -> informa resultado da identificação facial.
    if ((Request->MethodType == mtPost) &&
        (Path == L"/api/enrollment/result"))
    {
        std::unique_ptr<TJSONValue> Parsed(
            TJSONObject::ParseJSONValue(Request->Content));
        TJSONObject* Obj = dynamic_cast<TJSONObject*>(Parsed.get());

        if (Obj == nullptr)
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"invalid_json\"}";
            Handled = true;
            return;
        }

        UnicodeString RequestId;
        UnicodeString Status;
        UnicodeString Message;
        UnicodeString SamplesText;

        ReadJsonString(Obj, L"requestId", RequestId);
        ReadJsonString(Obj, L"status", Status);
        ReadJsonString(Obj, L"message", Message);
        ReadJsonString(Obj, L"samples", SamplesText);

        TBiometricCommand Completed;
        bool Found = false;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);

            if (!GEnrollmentCommand.RequestId.IsEmpty() &&
                SameText(GEnrollmentCommand.RequestId, RequestId))
            {
                GEnrollmentCommand.Pending = false;
                GEnrollmentCommand.Dispatched = true;
                GEnrollmentCommand.Status =
                    Trim(Status).IsEmpty() ? L"completed" : LowerCase(Status);
                GEnrollmentCommand.Message = Message;
                GEnrollmentCommand.Samples =
                    StrToIntDef(SamplesText, 0);

                Completed = GEnrollmentCommand;
                Found = true;

                AddSimpleEvent(
                    L"PORTARIA-01",
                    SameText(GEnrollmentCommand.Status, L"completed")
                        ? L"SUCCESS" : L"ERROR",
                    L"ENROLLMENT_RESULT",
                    SameText(GEnrollmentCommand.Status, L"completed")
                        ? L"Identificação facial cadastrada"
                        : L"Falha no cadastro de identificação facial",
                    GEnrollmentCommand.EmployeeName,
                    GEnrollmentCommand.Status);
            }
        }

        if (!Found)
        {
            Response->StatusCode = 404;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"request_not_found\"}";
            Handled = true;
            return;
        }

        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{\"ok\":true,\"status\":" +
            JsonString(Completed.Status) + L"}";
        Handled = true;
        return;
    }

    // ERP -> solicita remoção da identificação facial antes da exclusão.
    if ((Request->MethodType == mtPost) &&
        (Path == L"/api/biometric/revoke"))
    {
        std::unique_ptr<TJSONValue> Parsed(
            TJSONObject::ParseJSONValue(Request->Content));
        TJSONObject* Obj = dynamic_cast<TJSONObject*>(Parsed.get());

        if (Obj == nullptr)
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"invalid_json\"}";
            Handled = true;
            return;
        }

        UnicodeString EmployeeIdText;
        UnicodeString EmployeeName;
        UnicodeString Terminal;

        ReadJsonString(Obj, L"employeeId", EmployeeIdText);
        ReadJsonString(Obj, L"employeeName", EmployeeName);
        ReadJsonString(Obj, L"terminal", Terminal);

        const int EmployeeId = StrToIntDef(EmployeeIdText, 0);

        if (EmployeeId <= 0)
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"employee_id_required\"}";
            Handled = true;
            return;
        }

        if (Trim(Terminal).IsEmpty())
            Terminal = L"PORTARIA-01";

        TBiometricCommand Command;
        Command.Pending = true;
        Command.RequestId = NewRequestId(L"REV");
        Command.EmployeeId = EmployeeId;
        Command.EmployeeName = EmployeeName;
        Command.Terminal = Terminal;
        Command.RequestedAt = IsoNow();
        Command.Status = L"pending";
        Command.Message = L"Aguardando terminal";
        Command.Samples = 0;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);
            GRevokeCommand = Command;

            AddSimpleEvent(
                L"ERP-DELPHI",
                L"WARN",
                L"FACIAL_REMOVAL_REQUEST",
                L"Remoção da identificação facial solicitada",
                EmployeeName,
                Command.RequestId);
        }

        Response->StatusCode = 201;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{"
            L"\"ok\":true,"
            L"\"requestId\":" + JsonString(Command.RequestId) + L","
            L"\"employeeId\":" + IntToStr(EmployeeId) + L","
            L"\"status\":\"pending\""
            L"}";
        Handled = true;
        return;
    }

    // Portaria -> consulta revogacao pendente.
    if ((Request->MethodType == mtGet) &&
        (Path == L"/api/biometric/pending"))
    {
        TBiometricCommand Command;
        {
            std::lock_guard<std::mutex> Lock(GStateMutex);
            Command = GRevokeCommand;
        }

        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content = CommandJson(Command);
        Handled = true;
        return;
    }

    // Portaria -> resultado da revogacao.
    if ((Request->MethodType == mtPost) &&
        (Path == L"/api/biometric/result"))
    {
        std::unique_ptr<TJSONValue> Parsed(
            TJSONObject::ParseJSONValue(Request->Content));
        TJSONObject* Obj = dynamic_cast<TJSONObject*>(Parsed.get());

        if (Obj == nullptr)
        {
            Response->StatusCode = 400;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"invalid_json\"}";
            Handled = true;
            return;
        }

        UnicodeString RequestId;
        UnicodeString Status;
        UnicodeString Message;

        ReadJsonString(Obj, L"requestId", RequestId);
        ReadJsonString(Obj, L"status", Status);
        ReadJsonString(Obj, L"message", Message);

        TBiometricCommand Completed;
        bool Found = false;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);

            if (!GRevokeCommand.RequestId.IsEmpty() &&
                SameText(GRevokeCommand.RequestId, RequestId))
            {
                GRevokeCommand.Pending = false;
                GRevokeCommand.Status =
                    Trim(Status).IsEmpty() ? L"completed" : LowerCase(Status);
                GRevokeCommand.Message = Message;

                Completed = GRevokeCommand;
                Found = true;

                AddSimpleEvent(
                    L"PORTARIA-01",
                    SameText(GRevokeCommand.Status, L"completed")
                        ? L"SUCCESS" : L"ERROR",
                    L"FACIAL_REMOVAL_RESULT",
                    SameText(GRevokeCommand.Status, L"completed")
                        ? L"Identificação facial removida"
                        : L"Falha ao remover identificação facial",
                    GRevokeCommand.EmployeeName,
                    GRevokeCommand.Status);
            }
        }

        if (!Found)
        {
            Response->StatusCode = 404;
            Response->ContentType = L"application/json; charset=utf-8";
            Response->Content =
                L"{\"ok\":false,\"error\":\"request_not_found\"}";
            Handled = true;
            return;
        }

        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{\"ok\":true,\"status\":" +
            JsonString(Completed.Status) + L"}";
        Handled = true;
        return;
    }

    // ERP / dashboard -> consulta o último estado das operações de identificação facial.
    if ((Request->MethodType == mtGet) &&
        (Path == L"/api/biometric/status"))
    {
        TBiometricCommand Enrollment;
        TBiometricCommand Revoke;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);
            Enrollment = GEnrollmentCommand;
            Revoke = GRevokeCommand;
        }

        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{"
            L"\"enrollment\":" +
                CommandStatusJson(L"enrollment", Enrollment) + L","
            L"\"revoke\":" +
                CommandStatusJson(L"revoke", Revoke) +
            L"}";
        Handled = true;
        return;
    }


    // =========================================================
    // GET /api/metrics
    // Métricas operacionais calculadas a partir dos logs persistentes
    // atualmente carregados (até 1000 eventos mais recentes).
    // =========================================================
    if ((Request->MethodType == mtGet) &&
        (Path == L"/api/metrics"))
    {
        TOperationalMetrics Metrics;
        int LoadedEvents = 0;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);

            Metrics = BuildOperationalMetrics(Now());
            LoadedEvents =
                static_cast<int>(GEvents.size());
        }

        const double AvgRecognitionMs =
            AverageRecognitionMs(Metrics);

        Response->StatusCode = 200;
        Response->ContentType =
            L"application/json; charset=utf-8";

        Response->Content =
            L"{"
            L"\"period\":\"today\","
            L"\"scope\":\"loaded_persistent_events\","
            L"\"loadedEvents\":" + IntToStr(LoadedEvents) + L","
            L"\"memoryLimit\":" +
                IntToStr(static_cast<int>(MAX_EVENTS_IN_MEMORY)) + L","
            L"\"eventsToday\":" +
                IntToStr(Metrics.EventsToday) + L","
            L"\"errorsToday\":" +
                IntToStr(Metrics.ErrorsToday) + L","
            L"\"employeesCreatedToday\":" +
                IntToStr(Metrics.EmployeeCreatedToday) + L","
            L"\"employeesUpdatedToday\":" +
                IntToStr(Metrics.EmployeeUpdatedToday) + L","
            L"\"employeesDeletedToday\":" +
                IntToStr(Metrics.EmployeeDeletedToday) + L","
            L"\"enrollmentRequestsToday\":" +
                IntToStr(Metrics.EnrollmentRequestsToday) + L","
            L"\"enrollmentCompletedToday\":" +
                IntToStr(Metrics.EnrollmentCompletedToday) + L","
            L"\"enrollmentFailedToday\":" +
                IntToStr(Metrics.EnrollmentFailedToday) + L","
            L"\"faceRecognitionsToday\":" +
                IntToStr(Metrics.FaceRecognitionsToday) + L","
            L"\"averageFaceRecognitionMs\":" +
                InvariantNumber(AvgRecognitionMs) + L","
            L"\"employeeSyncToday\":" +
                IntToStr(Metrics.EmployeeSyncToday) + L","
            L"\"openIncidents\":" +
                IntToStr(Metrics.OpenIncidents) + L","
            L"\"incidentsOpenedToday\":" +
                IntToStr(Metrics.IncidentsOpenedToday) + L","
            L"\"incidentsResolvedToday\":" +
                IntToStr(Metrics.IncidentsResolvedToday) + L","
            L"\"erpDowntimeSecondsToday\":" +
                InvariantNumber(
                    Metrics.ErpDowntimeMsToday / 1000.0) + L","
            L"\"terminalDowntimeSecondsToday\":" +
                InvariantNumber(
                    Metrics.TerminalDowntimeMsToday / 1000.0) + L","
            L"\"erpAvailabilityPercent\":" +
                InvariantNumber(
                    Metrics.ErpAvailabilityPercent) + L","
            L"\"terminalAvailabilityPercent\":" +
                InvariantNumber(
                    Metrics.TerminalAvailabilityPercent) +
            L"}";

        Handled = true;
        return;
    }

    // =========================================================
    // GET /api/incidents
    // Histórico resumido de incidentes e disponibilidade.
    // =========================================================
    if ((Request->MethodType == mtGet) &&
        (Path == L"/api/incidents"))
    {
        const TDateTime NowValue = Now();

        TOperationalMetrics Metrics;
        TIncidentState ErpIncident;
        TIncidentState TerminalIncident;
        TObservabilityEvent LastIncidentEvent;
        bool HasLastIncident = false;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);

            Metrics = BuildOperationalMetrics(NowValue);

            ErpIncident = GErpIncident;
            TerminalIncident = GTerminalIncident;

            for (const TObservabilityEvent& Event : GEvents)
            {
                if (SameText(Event.Type, L"INCIDENT_OPENED") ||
                    SameText(Event.Type, L"INCIDENT_RESOLVED"))
                {
                    LastIncidentEvent = Event;
                    HasLastIncident = true;
                    break;
                }
            }
        }

        UnicodeString LastIncidentJson =
            L"{\"exists\":false}";

        if (HasLastIncident)
        {
            LastIncidentJson =
                L"{"
                L"\"exists\":true,"
                L"\"source\":" +
                    JsonString(LastIncidentEvent.Source) + L","
                L"\"type\":" +
                    JsonString(LastIncidentEvent.Type) + L","
                L"\"status\":" +
                    JsonString(LastIncidentEvent.Result) + L","
                L"\"timestamp\":" +
                    JsonString(LastIncidentEvent.Timestamp) + L","
                L"\"correlationId\":" +
                    JsonString(LastIncidentEvent.CorrelationId) + L","
                L"\"message\":" +
                    JsonString(LastIncidentEvent.Message) + L","
                L"\"durationSeconds\":" +
                    InvariantNumber(
                        LastIncidentEvent.DurationMs / 1000.0) +
                L"}";
        }

        Response->StatusCode = 200;
        Response->ContentType =
            L"application/json; charset=utf-8";

        Response->Content =
            L"{"
            L"\"period\":\"today\","
            L"\"openedToday\":" +
                IntToStr(Metrics.IncidentsOpenedToday) + L","
            L"\"resolvedToday\":" +
                IntToStr(Metrics.IncidentsResolvedToday) + L","
            L"\"openNow\":" +
                IntToStr(Metrics.OpenIncidents) + L","

            L"\"erp\":{"
                L"\"open\":" +
                    UnicodeString(
                        ErpIncident.Open ? L"true" : L"false") + L","
                L"\"availabilityPercent\":" +
                    InvariantNumber(
                        Metrics.ErpAvailabilityPercent) + L","
                L"\"downtimeSeconds\":" +
                    InvariantNumber(
                        Metrics.ErpDowntimeMsToday / 1000.0) + L","
                L"\"incidentId\":" +
                    JsonString(ErpIncident.IncidentId) + L","
                L"\"openedAt\":" +
                    JsonString(
                        ErpIncident.Open
                            ? IsoDateTime(ErpIncident.OpenedAt)
                            : L"") +
            L"},"

            L"\"terminal\":{"
                L"\"open\":" +
                    UnicodeString(
                        TerminalIncident.Open ? L"true" : L"false") + L","
                L"\"availabilityPercent\":" +
                    InvariantNumber(
                        Metrics.TerminalAvailabilityPercent) + L","
                L"\"downtimeSeconds\":" +
                    InvariantNumber(
                        Metrics.TerminalDowntimeMsToday / 1000.0) + L","
                L"\"incidentId\":" +
                    JsonString(TerminalIncident.IncidentId) + L","
                L"\"openedAt\":" +
                    JsonString(
                        TerminalIncident.Open
                            ? IsoDateTime(TerminalIncident.OpenedAt)
                            : L"") +
            L"},"

            L"\"lastIncident\":" +
                LastIncidentJson +
            L"}";

        Handled = true;
        return;
    }

    // =========================================================
    // GET /api/logs/info
    // Explica onde os logs estão e quanto tempo persistem.
    // =========================================================
    if ((Request->MethodType == mtGet) &&
        (Path == L"/api/logs/info"))
    {
        int EventCount = 0;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);
            EventCount = static_cast<int>(GEvents.size());
        }

        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{"
            L"\"persistent\":true,"
            L"\"storage\":\"jsonl\","
            L"\"path\":\"data\\\\observability-events.jsonl\","
            L"\"retention\":\"until_file_removed\","
            L"\"eventsLoaded\":" + IntToStr(EventCount) + L","
            L"\"apiMemoryLimit\":" +
                IntToStr(static_cast<int>(MAX_EVENTS_IN_MEMORY)) + L","
            L"\"dashboardLimit\":" +
                IntToStr(DASHBOARD_EVENT_LIMIT) +
            L"}";

        Handled = true;
        return;
    }

    if ((Request->MethodType == mtGet) &&
        (Path == L"/api/status"))
    {
        const TDateTime NowValue = Now();

        THeartbeatState Erp;
        THeartbeatState Terminal;
        TIncidentState ErpIncidentState;
        TIncidentState TerminalIncidentState;

        {
            std::lock_guard<std::mutex> Lock(GStateMutex);

            EvaluateCurrentIncidents(NowValue);

            Erp = GErp;
            Terminal = GTerminal;
            ErpIncidentState = GErpIncident;
            TerminalIncidentState = GTerminalIncident;
        }

        int ErpAge = -1;
        int TerminalAge = -1;
        bool ErpIncident = false;
        bool TerminalIncident = false;

        const UnicodeString ErpStatus =
            EffectiveStatus(Erp, NowValue, ErpAge, ErpIncident);

        const UnicodeString TerminalStatus =
            EffectiveStatus(Terminal, NowValue, TerminalAge, TerminalIncident);

        Response->StatusCode = 200;
        Response->ContentType = L"application/json; charset=utf-8";
        Response->Content =
            L"{"
            L"\"timestamp\":" + JsonString(IsoNow()) + L","

            L"\"server\":{"
                L"\"name\":\"TSX Observability Server\","
                L"\"status\":\"online\","
                L"\"port\":8080"
            L"},"

            L"\"erp\":{"
                L"\"name\":\"ERP Delphi\","
                L"\"source\":" + JsonString(
                    Erp.Source.IsEmpty() ? L"ERP-DELPHI" : Erp.Source) + L","
                L"\"status\":" + JsonString(ErpStatus) + L","
                L"\"heartbeatReceived\":" +
                    UnicodeString(Erp.Received ? L"true" : L"false") + L","
                L"\"lastSeen\":" + JsonString(
                    Erp.Received ? IsoDateTime(Erp.LastHeartbeat) : L"simulated") + L","
                L"\"ageSeconds\":" + IntToStr(ErpAge) + L","
                L"\"version\":" + JsonString(Erp.Version) +
            L"},"

            L"\"terminal\":{"
                L"\"name\":\"Portaria C++Builder\","
                L"\"source\":" + JsonString(
                    Terminal.Source.IsEmpty() ? L"PORTARIA-01" : Terminal.Source) + L","
                L"\"status\":" + JsonString(TerminalStatus) + L","
                L"\"heartbeatReceived\":" +
                    UnicodeString(Terminal.Received ? L"true" : L"false") + L","
                L"\"camera\":" +
                    UnicodeString(Terminal.CameraOnline ? L"true" : L"false") + L","
                L"\"faceEngine\":" +
                    UnicodeString(Terminal.FaceEngineOnline ? L"true" : L"false") + L","
                L"\"lastSeen\":" + JsonString(
                    Terminal.Received ? IsoDateTime(Terminal.LastHeartbeat) : L"simulated") + L","
                L"\"ageSeconds\":" + IntToStr(TerminalAge) + L","
                L"\"version\":" + JsonString(Terminal.Version) +
            L"},"

            L"\"incidents\":{"
                L"\"erp\":{"
                    L"\"open\":" + UnicodeString(ErpIncidentState.Open ? L"true" : L"false") + L","
                    L"\"message\":" + JsonString(
                        ErpIncidentState.Open ? L"ERP Delphi sem comunicação" : L"") + L","
                    L"\"incidentId\":" + JsonString(
                        ErpIncidentState.IncidentId) + L","
                    L"\"openedAt\":" + JsonString(
                        ErpIncidentState.Open
                            ? IsoDateTime(ErpIncidentState.OpenedAt)
                            : L"") +
                L"},"
                L"\"terminal\":{"
                    L"\"open\":" + UnicodeString(TerminalIncidentState.Open ? L"true" : L"false") + L","
                    L"\"message\":" + JsonString(
                        TerminalIncidentState.Open ? L"Portaria C++ sem comunicação" : L"") + L","
                    L"\"incidentId\":" + JsonString(
                        TerminalIncidentState.IncidentId) + L","
                    L"\"openedAt\":" + JsonString(
                        TerminalIncidentState.Open
                            ? IsoDateTime(TerminalIncidentState.OpenedAt)
                            : L"") +
                L"}"
            L"}"
            L"}";

        Handled = true;
        return;
    }

    // =========================================================
    // Dashboard
    // =========================================================
    if ((Request->MethodType == mtGet) &&
        ((Path == L"") || (Path == L"/")))
    {
        Response->StatusCode = 200;
        Response->ContentType = L"text/html; charset=utf-8";
        Response->Content = LR"HTML(
<!doctype html>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>C++ Observability Center</title>
<style>
:root{--bg:#050e18;--panel:#0b1c2b;--line:#16415f;--blue:#27b8ff;
--green:#20d89a;--yellow:#f5bd32;--red:#e34d5d;--text:#f4f8fc;--muted:#9db7ca}
*{box-sizing:border-box}
body{margin:0;color:var(--text);font-family:Segoe UI,Arial,sans-serif;
background:linear-gradient(rgba(23,61,87,.17) 1px,transparent 1px),
linear-gradient(90deg,rgba(23,61,87,.17) 1px,transparent 1px),var(--bg);
background-size:58px 58px}
header{padding:25px 42px 19px;display:flex;justify-content:space-between;align-items:center;
border-bottom:1px solid var(--line);background:rgba(3,10,18,.94)}
.brand small{color:var(--blue);font-weight:700;letter-spacing:2px}
.brand h1{margin:5px 0 3px;font-size:30px}.brand p{margin:0;color:var(--muted)}
.api{border:1px solid var(--line);border-radius:999px;padding:10px 18px;font-size:13px;font-weight:700}
main{padding:28px 42px}.hero{display:grid;grid-template-columns:1fr 72px 1fr 72px 1fr;gap:12px;align-items:center}
.card{background:linear-gradient(180deg,rgba(16,40,58,.92),rgba(7,23,36,.94));
border:1px solid var(--line);border-radius:14px;padding:22px;min-height:184px}
.label{color:var(--muted);font-size:11px;font-weight:700;letter-spacing:1.2px}
.card h2{margin:11px 0 9px;font-size:23px}.state{display:inline-flex;align-items:center;gap:8px;font-weight:700}
.dot{width:9px;height:9px;border-radius:50%;display:inline-block}
.online{color:var(--green)}.online .dot{background:var(--green)}
.simulated{color:var(--yellow)}.simulated .dot{background:var(--yellow)}
.offline{color:var(--red)}.offline .dot{background:var(--red)}
.meta{margin-top:16px;color:var(--muted);font-size:12px;line-height:1.7}.arrow{color:var(--blue);text-align:center;font-size:28px}
.incidents{margin-top:22px;display:grid;grid-template-columns:1fr 1fr;gap:14px}
.incident{padding:16px 18px;border-radius:12px;border:1px solid rgba(32,216,154,.35);background:rgba(7,22,34,.82)}
.incident.error{border-color:rgba(227,77,93,.7);background:rgba(54,10,17,.52)}
.incident strong{display:block;margin-bottom:5px}.incident span{color:var(--muted)}
.metrics-head{margin-top:22px;display:flex;justify-content:space-between;align-items:center;margin-bottom:10px}
.metrics-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:12px}
.metric{border:1px solid var(--line);border-radius:12px;background:rgba(7,22,34,.88);padding:16px;min-height:90px}
.metric .metric-label{color:var(--muted);font-size:11px;text-transform:uppercase;letter-spacing:.8px;font-weight:700}
.metric .metric-value{font-size:28px;font-weight:800;margin-top:7px;color:#f5f7fb}
.metric .metric-detail{font-size:11px;color:var(--muted);margin-top:4px}
.metric.alert .metric-value{color:var(--red)}
.metric.good .metric-value{color:var(--green)}
.incident-metrics{display:grid;grid-template-columns:repeat(3,1fr);gap:12px}
.last-incident{margin-top:12px;border:1px solid var(--line);border-radius:12px;background:rgba(7,22,34,.88);padding:16px}
.last-incident strong{display:block;margin-bottom:5px}
@media(max-width:1100px){
 .metrics-grid{grid-template-columns:repeat(2,1fr)}
 .incident-metrics{grid-template-columns:1fr}
}
.info{margin-top:16px;border-left:3px solid var(--blue);background:rgba(7,22,34,.78);
padding:14px 16px;color:var(--muted);line-height:1.5}
footer{padding:0 42px 26px;color:#758fa3;font-size:11px;letter-spacing:1px}
</style>
</head>
<body>
<header>
 <div class="brand"><small>RAD Studio FLORENCE 13.1 • OBSERVABILITY</small><h1>Observability Center</h1>
 <p>ERP Delphi + Portaria C++Builder 13.1 Florence</p></div>
 <div class="api online" id="apiStatus">OBSERVABILITY API • ONLINE • v0.7.3.2</div>
</header>
<main>
<div class="hero">
 <article class="card">
  <div class="label">ORIGEM CORPORATIVA</div><h2>ERP Delphi</h2>
  <div id="erpState" class="state simulated"><span class="dot"></span><span>SIMULATED</span></div>
  <div class="meta">Heartbeat: <span id="erpHeartbeat">--</span><br>
  Último contato: <span id="erpLastSeen">--</span><br>
  Versão: <span id="erpVersion">--</span></div>
 </article>
 <div class="arrow">→</div>
 <article class="card">
  <div class="label">CAMADA DE OBSERVABILIDADE</div><h2>WebServer C++</h2>
  <div class="state online"><span class="dot"></span><span>ONLINE</span></div>
  <div class="meta">Porta 8080<br>REST / JSON<br>Timeout: 15s</div>
 </article>
 <div class="arrow">←</div>
 <article class="card">
  <div class="label">OPERAÇÃO</div><h2>Portaria C++Builder</h2>
  <div id="terminalState" class="state simulated"><span class="dot"></span><span>SIMULATED</span></div>
  <div class="meta">Heartbeat: <span id="terminalHeartbeat">--</span><br>
  Câmera: <span id="camera">--</span><br>
  FACE Engine: <span id="face">--</span><br>
  Último contato: <span id="terminalLastSeen">--</span></div>
 </article>
</div>

<div class="incidents">
 <div id="erpIncident" class="incident">
  <strong>ERP Delphi</strong><span>Sem incidente real.</span>
 </div>
 <div id="terminalIncident" class="incident">
  <strong>Portaria C++</strong><span>Sem incidente real.</span>
 </div>
 <div style="margin-top:8px;color:var(--muted);font-size:11px">
  Persistência: <code>data\observability-events.jsonl</code> • API mantém os últimos 1000 eventos em memória • arquivo permanece até ser removido.
 </div>
</div>

<div class="metrics-head">
 <strong>Métricas operacionais • hoje</strong>
 <span style="color:var(--muted);font-size:12px">calculadas a partir dos logs persistentes carregados</span>
</div>
<div class="metrics-grid">
 <div class="metric">
  <div class="metric-label">Eventos hoje</div>
  <div id="metricEvents" class="metric-value">--</div>
  <div class="metric-detail">operações observadas</div>
 </div>
 <div id="metricErrorsCard" class="metric">
  <div class="metric-label">Erros hoje</div>
  <div id="metricErrors" class="metric-value">--</div>
  <div class="metric-detail">eventos com nível ERROR</div>
 </div>
 <div class="metric">
  <div class="metric-label">Funcionários</div>
  <div id="metricEmployees" class="metric-value">--</div>
  <div id="metricEmployeesDetail" class="metric-detail">--</div>
 </div>
 <div class="metric">
  <div class="metric-label">Cadastros faciais</div>
  <div id="metricEnrollments" class="metric-value">--</div>
  <div id="metricEnrollmentsDetail" class="metric-detail">--</div>
 </div>
 <div class="metric good">
  <div class="metric-label">Faces confirmadas</div>
  <div id="metricRecognitions" class="metric-value">--</div>
  <div class="metric-detail">FACE_RECOGNITION com sucesso</div>
 </div>
 <div class="metric">
  <div class="metric-label">Tempo médio FACE</div>
  <div id="metricRecognitionMs" class="metric-value">--</div>
  <div class="metric-detail">tempo da comparação facial</div>
 </div>
 <div class="metric">
  <div class="metric-label">Sincronizações ERP</div>
  <div id="metricSync" class="metric-value">--</div>
  <div class="metric-detail">snapshots recebidos pela Portaria</div>
 </div>
 <div id="metricIncidentsCard" class="metric">
  <div class="metric-label">Incidentes hoje</div>
  <div id="metricIncidents" class="metric-value">--</div>
  <div id="metricIncidentsDetail" class="metric-detail">--</div>
 </div>
</div>

<div class="metrics-head">
 <strong>Histórico de incidentes • hoje</strong>
 <span style="color:var(--muted);font-size:12px">heartbeat → incidente → recuperação → disponibilidade observada</span>
</div>
<div class="incident-metrics">
 <div class="metric">
  <div class="metric-label">Disponibilidade ERP</div>
  <div id="erpAvailability" class="metric-value">--</div>
  <div id="erpDowntime" class="metric-detail">--</div>
 </div>
 <div class="metric">
  <div class="metric-label">Disponibilidade Portaria</div>
  <div id="terminalAvailability" class="metric-value">--</div>
  <div id="terminalDowntime" class="metric-detail">--</div>
 </div>
 <div class="metric">
  <div class="metric-label">Estado dos incidentes</div>
  <div id="incidentStateCount" class="metric-value">--</div>
  <div id="incidentStateDetail" class="metric-detail">--</div>
 </div>
</div>
<div id="lastIncidentPanel" class="last-incident">
 <strong>Último incidente</strong>
 <span style="color:var(--muted)">Nenhum incidente registrado.</span>
</div>

<div style="margin-top:22px">
 <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:10px">
  <strong>Logs operacionais</strong>
  <span style="color:var(--muted);font-size:12px">persistente • últimos 20 exibidos</span>
 </div>
 <div id="eventsPanel" style="border:1px solid var(--line);border-radius:12px;overflow:hidden;background:rgba(7,22,34,.88)">
  <div style="padding:16px;color:var(--muted)">Aguardando logs operacionais...</div>
 </div>
</div>

<div style="margin-top:22px">
 <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:10px">
  <strong>Gestão de identificação facial</strong>
  <span style="color:var(--muted);font-size:12px">fila de comandos ERP → Portaria</span>
 </div>
 <div style="display:grid;grid-template-columns:1fr 1fr;gap:14px">
  <div class="incident">
   <strong>Cadastro de identificação facial</strong>
   <span id="enrollmentStatus">Nenhuma solicitação.</span>
  </div>
  <div class="incident">
   <strong>Remoção da identificação facial</strong>
   <span id="revokeStatus">Nenhuma solicitação.</span>
  </div>
 </div>
</div>

<div class="info">
<strong>v0.7.3.2:</strong> lifecycle persistente de incidentes +
disponibilidade observada. O WebServer registra
<code>INCIDENT_OPENED</code> e <code>INCIDENT_RESOLVED</code>
no mesmo histórico JSONL.
</div>
</main>
<footer>Rad Studio FLORENCE 13.1  OBSERVABILITY CENTER • CONFERENCE 2026 • v0.7.3.2 • INCIDENTES + DISPONIBILIDADE</footer>

<script>
const $=id=>document.getElementById(id);
function cls(v){v=String(v||'').toLowerCase();return v==='online'?'online':v==='simulated'?'simulated':'offline'}
function state(id,v){const e=$(id);e.className='state '+cls(v);e.querySelector('span:last-child').textContent=String(v||'--').toUpperCase()}
async function refreshStatus(){
 try{
  const r=await fetch('/api/status',{cache:'no-store'});
  if(!r.ok) throw new Error('HTTP '+r.status);
  const s=await r.json();

  $('apiStatus').textContent='OBSERVABILITY API • ONLINE • v0.7.3.2';
  $('apiStatus').className='api online';

  state('erpState',s.erp.status);state('terminalState',s.terminal.status);
  $('erpHeartbeat').textContent=s.erp.heartbeatReceived?'recebido':'não recebido';
  $('erpLastSeen').textContent=s.erp.lastSeen;$('erpVersion').textContent=s.erp.version||'--';
  $('terminalHeartbeat').textContent=s.terminal.heartbeatReceived?'recebido':'não recebido';
  $('terminalLastSeen').textContent=s.terminal.lastSeen;

  const terminalTelemetryFresh=
    String(s.terminal.status||'').toLowerCase()==='online';

  $('camera').textContent=
    terminalTelemetryFresh
      ? (s.terminal.camera?'ONLINE':'OFFLINE')
      : 'SEM TELEMETRIA';

  $('face').textContent=
    terminalTelemetryFresh
      ? (s.terminal.faceEngine?'ONLINE':'OFFLINE')
      : 'SEM TELEMETRIA';

  const ei=$('erpIncident');
  ei.className='incident '+(s.incidents.erp.open?'error':'');
  ei.innerHTML='<strong>ERP Delphi</strong><span>'+
    (s.incidents.erp.open?s.incidents.erp.message+' • último heartbeat há '+s.erp.ageSeconds+'s':
      (s.erp.heartbeatReceived?'Comunicando normalmente.':'Aguardando primeiro heartbeat real.'))+'</span>';

  const ti=$('terminalIncident');
  ti.className='incident '+(s.incidents.terminal.open?'error':'');
  ti.innerHTML='<strong>Portaria C++</strong><span>'+
    (s.incidents.terminal.open?s.incidents.terminal.message+' • último heartbeat há '+s.terminal.ageSeconds+'s':
      (s.terminal.heartbeatReceived?'Comunicando normalmente.':'Aguardando primeiro heartbeat real.'))+'</span>';
 }catch(e){
  $('apiStatus').textContent='OBSERVABILITY API • OFFLINE';
  $('apiStatus').className='api offline';
 }
}

async function refreshMetrics(){
 try{
  const mr=await fetch('/api/metrics',{cache:'no-store'});
  if(!mr.ok) throw new Error('HTTP '+mr.status);

  const m=await mr.json();

  $('metricEvents').textContent=Number(m.eventsToday||0);
  $('metricErrors').textContent=Number(m.errorsToday||0);

  const created=Number(m.employeesCreatedToday||0);
  const updated=Number(m.employeesUpdatedToday||0);
  const deleted=Number(m.employeesDeletedToday||0);
  $('metricEmployees').textContent=created;
  $('metricEmployeesDetail').textContent=
    created+' criado(s) • '+updated+' alterado(s) • '+deleted+' excluído(s)';

  const enrollOk=Number(m.enrollmentCompletedToday||0);
  const enrollFail=Number(m.enrollmentFailedToday||0);
  const enrollReq=Number(m.enrollmentRequestsToday||0);
  $('metricEnrollments').textContent=enrollOk;
  $('metricEnrollmentsDetail').textContent=
    enrollReq+' solicitado(s) • '+enrollOk+' concluído(s) • '+enrollFail+' falha(s)';

  $('metricRecognitions').textContent=
    Number(m.faceRecognitionsToday||0);

  const avg=Number(m.averageFaceRecognitionMs||0);
  $('metricRecognitionMs').textContent=
    avg>0 ? avg.toFixed(0)+' ms' : '0 ms';

  $('metricSync').textContent=
    Number(m.employeeSyncToday||0);

  const incidentsToday=
    Number(m.incidentsOpenedToday||0);
  const incidentsResolved=
    Number(m.incidentsResolvedToday||0);
  const incidentsOpen=
    Number(m.openIncidents||0);

  $('metricIncidents').textContent=
    incidentsToday;

  $('metricIncidentsDetail').textContent=
    incidentsResolved+' resolvido(s) • '+
    incidentsOpen+' aberto(s)';

  $('metricErrorsCard').className=
    Number(m.errorsToday||0)>0 ? 'metric alert' : 'metric good';

  $('metricIncidentsCard').className=
    incidentsOpen>0 ? 'metric alert' :
    incidentsToday>0 ? 'metric' :
    'metric good';
 }catch(e){
  ['metricEvents','metricErrors','metricEmployees','metricEnrollments',
   'metricRecognitions','metricRecognitionMs','metricSync','metricIncidents']
   .forEach(id=>$(id).textContent='--');
 }
}

function formatDuration(totalSeconds){
 const value=Math.max(0,Math.round(Number(totalSeconds||0)));
 const h=Math.floor(value/3600);
 const m=Math.floor((value%3600)/60);
 const s=value%60;
 return String(h).padStart(2,'0')+':'+
        String(m).padStart(2,'0')+':'+
        String(s).padStart(2,'0');
}

async function refreshIncidents(){
 try{
  const ir=await fetch('/api/incidents',{cache:'no-store'});
  if(!ir.ok) throw new Error('HTTP '+ir.status);

  const d=await ir.json();

  const erpAvailability=
    Number(d.erp && d.erp.availabilityPercent || 100);
  const terminalAvailability=
    Number(d.terminal && d.terminal.availabilityPercent || 100);

  $('erpAvailability').textContent=
    erpAvailability.toFixed(2)+'%';
  $('terminalAvailability').textContent=
    terminalAvailability.toFixed(2)+'%';

  $('erpDowntime').textContent=
    'indisponível '+formatDuration(
      d.erp && d.erp.downtimeSeconds);
  $('terminalDowntime').textContent=
    'indisponível '+formatDuration(
      d.terminal && d.terminal.downtimeSeconds);

  const openNow=Number(d.openNow||0);
  const openedToday=Number(d.openedToday||0);
  const resolvedToday=Number(d.resolvedToday||0);

  $('incidentStateCount').textContent=openNow;
  $('incidentStateDetail').textContent=
    openedToday+' aberto(s) hoje • '+
    resolvedToday+' resolvido(s)';

  const panel=$('lastIncidentPanel');
  const last=d.lastIncident||{};

  if(!last.exists){
    panel.innerHTML=
      '<strong>Último incidente</strong>'+
      '<span style="color:var(--muted)">Nenhum incidente registrado.</span>';
    return;
  }

  const resolved=
    String(last.status||'').toUpperCase()==='RESOLVED';
  const statusColor=
    resolved ? 'var(--green)' : 'var(--red)';

  const duration=
    Number(last.durationSeconds||0)>0
      ? ' • duração '+formatDuration(last.durationSeconds)
      : '';

  panel.innerHTML=
    '<strong>Último incidente</strong>'+
    '<span style="color:'+statusColor+';font-weight:700">'+
      String(last.status||'--').toUpperCase()+
    '</span> • '+
    '<strong style="display:inline">'+String(last.source||'--')+'</strong>'+
    ' • '+String(last.timestamp||'--')+
    duration+
    '<br><span style="color:var(--muted)">'+
      String(last.message||'--')+
      (last.correlationId?' • '+String(last.correlationId):'')+
    '</span>';
 }catch(e){
  $('erpAvailability').textContent='--';
  $('terminalAvailability').textContent='--';
  $('erpDowntime').textContent='--';
  $('terminalDowntime').textContent='--';
  $('incidentStateCount').textContent='--';
 }
}

async function refreshLogs(){
 const panel=$('eventsPanel');
 try{
  const er=await fetch('/api/events',{cache:'no-store'});
  if(!er.ok) throw new Error('HTTP '+er.status);
  const ed=await er.json();
  const events=(ed.events||[]).slice(0,20);

  if(events.length===0){
    panel.innerHTML='<div style="padding:16px;color:var(--muted)">Nenhum log persistente registrado ainda.</div>';
    return;
  }

  panel.innerHTML=events.map(ev=>{
    const level=String(ev.level||'INFO').toUpperCase();
    const color=level==='ERROR'?'var(--red)':level==='WARN'?'var(--yellow)':level==='SUCCESS'?'var(--green)':'var(--blue)';
    const parts=String(ev.timestamp||'').split('T');
    const hh=parts.length>1?parts[1]:'--';
    return '<div style="display:grid;grid-template-columns:82px 120px 90px 1fr 170px;gap:10px;padding:11px 14px;border-bottom:1px solid rgba(22,65,95,.7);align-items:center">'+
      '<span style="color:var(--muted)">'+hh+'</span>'+
      '<strong>'+String(ev.source||'--')+'</strong>'+
      '<span style="color:'+color+';font-weight:700">'+level+'</span>'+
      '<span><small style="color:var(--muted)">'+String(ev.type||'GENERAL')+'</small><br>'+String(ev.message||'--')+(ev.employee?' • <strong>'+String(ev.employee)+'</strong>':'')+'</span>'+
      '<span style="color:var(--muted);text-align:right">'+(ev.result?String(ev.result):'')+
        (Number(ev.durationMs)>0?' • '+Number(ev.durationMs).toFixed(0)+' ms':'')+'</span></div>';
  }).join('');
 }catch(e){
  panel.innerHTML='<div style="padding:16px;color:var(--red)">Não foi possível carregar os logs. A Observability API principal pode continuar online.</div>';
 }
}

async function refreshFacial(){
 try{
  const br=await fetch('/api/biometric/status',{cache:'no-store'});
  if(!br.ok) throw new Error('HTTP '+br.status);
  const bd=await br.json();

  function facialText(item){
    if(!item || !item.requestId) return 'Nenhuma solicitação.';
    let text='EmployeeId '+item.employeeId;
    if(item.employeeName) text+=' • '+item.employeeName;
    const status=String(item.status||'--').toLowerCase();
    if(status==='in_progress'){
      text+=' • EM ANDAMENTO • '+Number(item.samples||0)+'/8 amostras';
    }else{
      text+=' • '+status.toUpperCase();
      if(item.pending && !item.dispatched) text+=' • aguardando Portaria';
    }
    if(item.message) text+=' • '+item.message;
    return text;
  }

  $('enrollmentStatus').textContent=facialText(bd.enrollment);
  $('revokeStatus').textContent=facialText(bd.revoke);
 }catch(e){
  $('enrollmentStatus').textContent='Status de identificação facial indisponível.';
  $('revokeStatus').textContent='Status de remoção facial indisponível.';
 }
}

async function refresh(){
 await refreshStatus();
 await Promise.all([
   refreshMetrics(),
   refreshIncidents(),
   refreshLogs(),
   refreshFacial()
 ]);
}

refresh();setInterval(refresh,1000);
</script>
</body>
</html>
)HTML";
        Handled = true;
        return;
    }

    Response->StatusCode = 404;
    Response->ContentType = L"application/json; charset=utf-8";
    Response->Content =
        L"{\"status\":\"error\",\"message\":\"endpoint_not_found\"}";
    Handled = true;
}
//---------------------------------------------------------------------------

