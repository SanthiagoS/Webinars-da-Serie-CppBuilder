unit ObservabilityHeartbeat;

interface

uses
  System.SysUtils,
  System.Classes,
  System.JSON,
  System.Net.URLClient,
  System.Net.HttpClient,
  System.Net.HttpClientComponent,
  System.Threading,
  System.SyncObjs,
  Vcl.ExtCtrls;

type
  TObservabilityHeartbeat = class
  private
    FTimer: TTimer;
    FServerUrl: string;
    FSource: string;
    FVersion: string;
    FEnabled: Boolean;
    FBusy: Integer;

    procedure TimerTick(Sender: TObject);
    procedure SendHeartbeatAsync;
  public
    constructor Create;
    destructor Destroy; override;

    procedure Start;
    procedure Stop;
    procedure SendNow;

    property ServerUrl: string read FServerUrl write FServerUrl;
    property Source: string read FSource write FSource;
    property Version: string read FVersion write FVersion;
    property Enabled: Boolean read FEnabled;
  end;

implementation

constructor TObservabilityHeartbeat.Create;
begin
  inherited Create;

  FServerUrl := 'http://127.0.0.1:8080';
  FSource := 'ERP-DELPHI';
  FVersion := '1.0.0';
  FEnabled := False;
  FBusy := 0;

  FTimer := TTimer.Create(nil);
  FTimer.Enabled := False;
  FTimer.Interval := 5000;
  FTimer.OnTimer := TimerTick;
end;

destructor TObservabilityHeartbeat.Destroy;
begin
  Stop;
  FreeAndNil(FTimer);
  inherited;
end;

procedure TObservabilityHeartbeat.Start;
begin
  if FEnabled then
    Exit;

  FEnabled := True;
  FTimer.Enabled := True;

  SendNow;
end;

procedure TObservabilityHeartbeat.Stop;
begin
  FEnabled := False;

  if Assigned(FTimer) then
    FTimer.Enabled := False;
end;

procedure TObservabilityHeartbeat.TimerTick(Sender: TObject);
begin
  if not FEnabled then
    Exit;

  SendHeartbeatAsync;
end;

procedure TObservabilityHeartbeat.SendNow;
begin
  if not FEnabled then
    Exit;

  SendHeartbeatAsync;
end;

procedure TObservabilityHeartbeat.SendHeartbeatAsync;
var
  LUrl: string;
  LSource: string;
  LVersion: string;
begin
  // Evita duas requisicoes simultaneas.
  if TInterlocked.CompareExchange(FBusy, 1, 0) <> 0 then
    Exit;

  LUrl := FServerUrl;
  LSource := FSource;
  LVersion := FVersion;

  TTask.Run(
    procedure
    var
      LHttp: THTTPClient;
      LJson: TJSONObject;
      LBody: TStringStream;
      LResponse: IHTTPResponse;
    begin
      try
        LHttp := THTTPClient.Create;
        try
          LHttp.ConnectionTimeout := 800;
          LHttp.ResponseTimeout := 1200;

          LJson := TJSONObject.Create;
          try
            LJson.AddPair('source', LSource);
            LJson.AddPair('status', 'online');
            LJson.AddPair('version', LVersion);

            // Forma de construtor mais compativel entre versoes.
            LBody := TStringStream.Create(
              LJson.ToJSON,
              TEncoding.UTF8
            );
            try
              LBody.Position := 0;

              LResponse := LHttp.Post(
                LUrl + '/api/heartbeat',
                LBody,
                nil,
                [TNetHeader.Create(
                  'Content-Type',
                  'application/json'
                )]
              );

              // Sem dependencia funcional do servidor.
              // Se responder ou nao, o ERP segue normalmente.
              if Assigned(LResponse) then
              begin
                // reservado para telemetria futura
              end;
            finally
              LBody.Free;
            end;
          finally
            LJson.Free;
          end;
        finally
          LHttp.Free;
        end;
      except
        // O servidor de observabilidade e adicional.
        // Qualquer falha aqui nao interfere no ERP.
      end;

      TInterlocked.Exchange(FBusy, 0);
    end
  );
end;

end.

