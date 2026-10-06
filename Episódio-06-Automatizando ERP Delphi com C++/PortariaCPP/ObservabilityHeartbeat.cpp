#include <vcl.h>
#pragma hdrstop

#include "ObservabilityHeartbeat.h"

#include <IdHTTP.hpp>
#include <System.JSON.hpp>
#include <System.Threading.hpp>


namespace
{
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

    bool ParseBiometricCommand(
        const UnicodeString& Content,
        TBiometricCommand& Command)
    {
        Command = TBiometricCommand();

        std::unique_ptr<TJSONValue> Parsed(
            TJSONObject::ParseJSONValue(Content));

        TJSONObject* Obj =
            dynamic_cast<TJSONObject*>(Parsed.get());

        if (Obj == nullptr)
            return false;

        bool Pending = false;
        if (!ReadJsonBool(Obj, L"pending", Pending))
            return false;

        Command.Pending = Pending;

        if (!Pending)
            return true;

        UnicodeString EmployeeIdText;

        ReadJsonString(Obj, L"requestId", Command.RequestId);
        ReadJsonString(Obj, L"employeeId", EmployeeIdText);
        ReadJsonString(Obj, L"employeeName", Command.EmployeeName);
        ReadJsonString(Obj, L"terminal", Command.Terminal);

        Command.EmployeeId =
            StrToIntDef(EmployeeIdText, 0);

        return !Command.RequestId.IsEmpty() &&
            Command.EmployeeId > 0;
    }
}


TObservabilityHeartbeat::TObservabilityHeartbeat()
    : FTimer(nullptr),
      FServerUrl(L"http://127.0.0.1:8080"),
      FSource(L"PORTARIA-01"),
      FVersion(L"1.0.0"),
      FBusy(false),
      FEnabled(false),
      FCameraOnline(false),
      FFaceEngineOnline(false)
{
    FTimer = new TTimer(nullptr);
    FTimer->Enabled = false;
    FTimer->Interval = 5000;
    FTimer->OnTimer = TimerTick;
}

TObservabilityHeartbeat::~TObservabilityHeartbeat()
{
    Stop();

    delete FTimer;
    FTimer = nullptr;
}

void TObservabilityHeartbeat::SetServerUrl(
    const UnicodeString& Value)
{
    FServerUrl = Value;
}

void TObservabilityHeartbeat::SetSource(
    const UnicodeString& Value)
{
    FSource = Value;
}

void TObservabilityHeartbeat::SetVersion(
    const UnicodeString& Value)
{
    FVersion = Value;
}

void TObservabilityHeartbeat::UpdateRuntimeStatus(
    bool CameraOnline,
    bool FaceEngineOnline)
{
    FCameraOnline = CameraOnline;
    FFaceEngineOnline = FaceEngineOnline;
}

void TObservabilityHeartbeat::Start()
{
    if (FEnabled)
        return;

    FEnabled = true;

    if (FTimer != nullptr)
        FTimer->Enabled = true;

    SendNow();
}

void TObservabilityHeartbeat::Stop()
{
    FEnabled = false;

    if (FTimer != nullptr)
        FTimer->Enabled = false;
}

void TObservabilityHeartbeat::SendNow()
{
    if (!FEnabled)
        return;

    SendHeartbeatAsync();
}

void __fastcall TObservabilityHeartbeat::TimerTick(
    TObject* Sender)
{
    (void)Sender;

    if (!FEnabled)
        return;

    SendHeartbeatAsync();
}

void TObservabilityHeartbeat::SendHeartbeatAsync()
{
    bool Expected = false;

    if (!FBusy.compare_exchange_strong(Expected, true))
        return;

    const UnicodeString Url = FServerUrl;
    const UnicodeString Source = FSource;
    const UnicodeString Version = FVersion;
    const bool CameraOnline = FCameraOnline;
    const bool FaceEngineOnline = FFaceEngineOnline;

    TTask::Run(
        [this, Url, Source, Version, CameraOnline, FaceEngineOnline]()
        {
            try
            {
                std::unique_ptr<TIdHTTP> Http(
                    new TIdHTTP(nullptr));

                Http->ConnectTimeout = 800;
                Http->ReadTimeout = 1200;
                Http->Request->ContentType = L"application/json";
                Http->Request->CharSet = L"utf-8";

                std::unique_ptr<TJSONObject> Json(
                    new TJSONObject());

                Json->AddPair(L"source", Source);
                Json->AddPair(L"status", L"online");
                Json->AddPair(L"version", Version);
                Json->AddPair(
                    L"camera",
                    CameraOnline
                        ? static_cast<TJSONValue*>(new TJSONTrue())
                        : static_cast<TJSONValue*>(new TJSONFalse()));
                Json->AddPair(
                    L"faceEngine",
                    FaceEngineOnline
                        ? static_cast<TJSONValue*>(new TJSONTrue())
                        : static_cast<TJSONValue*>(new TJSONFalse()));

                std::unique_ptr<TStringStream> Body(
                    new TStringStream(
                        Json->ToJSON(),
                        TEncoding::UTF8,
                        false));

                Body->Position = 0;

                try
                {
                    Http->Post(
                        Url + L"/api/heartbeat",
                        Body.get());
                }
                catch (...)
                {
                    // Observabilidade é adicional:
                    // nunca interromper o terminal por falha do WebServer.
                }
            }
            catch (...)
            {
                // Mantém a Portaria independente.
            }

            // Além do heartbeat, consulta comandos biométricos pendentes.
            // Tudo continua em background: falha do WebServer nunca bloqueia a Portaria.
            try
            {
                PollBiometricCommands();
            }
            catch (...)
            {
            }

            FBusy = false;
        });
}



void TObservabilityHeartbeat::PollBiometricCommands()
{
    const UnicodeString Url = FServerUrl;
    const UnicodeString Terminal = FSource;

    // ---------------------------------------------------------
    // Cadastro facial pendente
    // ---------------------------------------------------------
    try
    {
        std::unique_ptr<TIdHTTP> Http(new TIdHTTP(nullptr));
        Http->ConnectTimeout = 600;
        Http->ReadTimeout = 900;

        const UnicodeString Content =
            Http->Get(Url + L"/api/enrollment/pending");

        TBiometricCommand Command;

        if (ParseBiometricCommand(Content, Command) &&
            Command.Pending &&
            (Command.Terminal.IsEmpty() ||
             SameText(Command.Terminal, Terminal)))
        {
            std::lock_guard<std::mutex> Lock(FCommandMutex);

            if (!SameText(
                    Command.RequestId,
                    FLastEnrollmentDelivered))
            {
                FEnrollmentCommand = Command;
            }
        }
    }
    catch (...)
    {
        // Gestão biométrica é adicional e não derruba o terminal.
    }

    // ---------------------------------------------------------
    // Revogação de biometria pendente
    // ---------------------------------------------------------
    try
    {
        std::unique_ptr<TIdHTTP> Http(new TIdHTTP(nullptr));
        Http->ConnectTimeout = 600;
        Http->ReadTimeout = 900;

        const UnicodeString Content =
            Http->Get(Url + L"/api/biometric/pending");

        TBiometricCommand Command;

        if (ParseBiometricCommand(Content, Command) &&
            Command.Pending &&
            (Command.Terminal.IsEmpty() ||
             SameText(Command.Terminal, Terminal)))
        {
            std::lock_guard<std::mutex> Lock(FCommandMutex);

            if (!SameText(
                    Command.RequestId,
                    FLastRevokeDelivered))
            {
                FRevokeCommand = Command;
            }
        }
    }
    catch (...)
    {
        // Mantém a operação local independente do WebServer.
    }
}

bool TObservabilityHeartbeat::TryConsumeEnrollmentCommand(
    TBiometricCommand& Command)
{
    std::lock_guard<std::mutex> Lock(FCommandMutex);

    if (!FEnrollmentCommand.Pending)
        return false;

    Command = FEnrollmentCommand;
    FEnrollmentCommand.Pending = false;
    FLastEnrollmentDelivered = Command.RequestId;

    return true;
}

bool TObservabilityHeartbeat::TryConsumeRevokeCommand(
    TBiometricCommand& Command)
{
    std::lock_guard<std::mutex> Lock(FCommandMutex);

    if (!FRevokeCommand.Pending)
        return false;

    Command = FRevokeCommand;
    FRevokeCommand.Pending = false;
    FLastRevokeDelivered = Command.RequestId;

    return true;
}

void TObservabilityHeartbeat::SendBiometricResultAsync(
    const UnicodeString& Endpoint,
    const UnicodeString& RequestId,
    const UnicodeString& Status,
    const UnicodeString& Message,
    int Samples)
{
    const UnicodeString Url = FServerUrl;

    TTask::Run(
        [Url, Endpoint, RequestId, Status, Message, Samples]()
        {
            // Pequenas tentativas locais ajudam a não perder o RESULT
            // por uma oscilação momentânea do WebServer.
            for (int Attempt = 0; Attempt < 3; ++Attempt)
            {
                try
                {
                    std::unique_ptr<TIdHTTP> Http(
                        new TIdHTTP(nullptr));

                    Http->ConnectTimeout = 800;
                    Http->ReadTimeout = 1200;
                    Http->Request->ContentType = L"application/json";
                    Http->Request->CharSet = L"utf-8";

                    std::unique_ptr<TJSONObject> Json(
                        new TJSONObject());

                    Json->AddPair(L"requestId", RequestId);
                    Json->AddPair(L"status", Status);
                    Json->AddPair(L"message", Message);

                    if (Samples >= 0)
                        Json->AddPair(L"samples", IntToStr(Samples));

                    std::unique_ptr<TStringStream> Body(
                        new TStringStream(
                            Json->ToJSON(),
                            TEncoding::UTF8,
                            false));

                    Body->Position = 0;

                    Http->Post(
                        Url + Endpoint,
                        Body.get());

                    return;
                }
                catch (...)
                {
                    if (Attempt < 2)
                        Sleep(150);
                }
            }
        });
}

void TObservabilityHeartbeat::SendEnrollmentProgress(
    const UnicodeString& RequestId,
    const UnicodeString& Message,
    int Samples)
{
    SendBiometricResultAsync(
        L"/api/enrollment/progress",
        RequestId,
        L"in_progress",
        Message,
        Samples);
}

void TObservabilityHeartbeat::SendEnrollmentResult(
    const UnicodeString& RequestId,
    const UnicodeString& Status,
    const UnicodeString& Message,
    int Samples)
{
    SendBiometricResultAsync(
        L"/api/enrollment/result",
        RequestId,
        Status,
        Message,
        Samples);
}

void TObservabilityHeartbeat::SendRevokeResult(
    const UnicodeString& RequestId,
    const UnicodeString& Status,
    const UnicodeString& Message)
{
    SendBiometricResultAsync(
        L"/api/biometric/result",
        RequestId,
        Status,
        Message,
        -1);
}


void TObservabilityHeartbeat::SendEvent(
    const UnicodeString& Level,
    const UnicodeString& Type,
    const UnicodeString& Message,
    const UnicodeString& Employee,
    const UnicodeString& Result,
    double DurationMs)
{
    const UnicodeString Url = FServerUrl;
    const UnicodeString Source = FSource;

    TTask::Run(
        [Url, Source, Level, Type, Message, Employee, Result, DurationMs]()
        {
            try
            {
                std::unique_ptr<TIdHTTP> Http(new TIdHTTP(nullptr));

                Http->ConnectTimeout = 800;
                Http->ReadTimeout = 1200;
                Http->Request->ContentType = L"application/json";
                Http->Request->CharSet = L"utf-8";

                std::unique_ptr<TJSONObject> Json(new TJSONObject());

                Json->AddPair(L"source", Source);
                Json->AddPair(L"level", Level);
                Json->AddPair(L"type", Type);
                Json->AddPair(L"message", Message);
                Json->AddPair(L"employee", Employee);
                Json->AddPair(L"result", Result);
                Json->AddPair(L"durationMs", FloatToStr(DurationMs));

                std::unique_ptr<TStringStream> Body(
                    new TStringStream(Json->ToJSON(), TEncoding::UTF8, false));

                Body->Position = 0;

                try
                {
                    Http->Post(Url + L"/api/events", Body.get());
                }
                catch (...)
                {
                }
            }
            catch (...)
            {
            }
        });
}


