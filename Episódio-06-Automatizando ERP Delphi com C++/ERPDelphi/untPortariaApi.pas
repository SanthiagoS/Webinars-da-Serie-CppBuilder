unit untPortariaApi;

interface

uses
  System.SysUtils,
  System.Classes,
  System.JSON,
  System.Generics.Collections,
  System.SyncObjs,
  IdHTTPServer,
  IdCustomHTTPServer,
  IdContext,
  FireDAC.Comp.Client,
  FireDAC.Stan.Param,
  FireDAC.DApt,
  FireDAC.Phys,
  FireDAC.Phys.SQLite,
  FireDAC.Phys.SQLiteDef;

type
  TAccessEventNotify = procedure(const AJson: string) of object;

  TPortariaApiServer = class(TComponent)
  private
    FServer: TIdHTTPServer;
    FDatabasePath: string;
    FOnAccessEvent: TAccessEventNotify;

    FOperationLock: TCriticalSection;

    // ============================================================
    // CADASTRO DE IDENTIFICACAO FACIAL
    // ============================================================
    FEnrollmentRequestID: string;
    FEnrollmentEmployeeID: Integer;
    FEnrollmentEmployeeName: string;
    FEnrollmentTerminal: string;
    FEnrollmentStatus: string;
    FEnrollmentMessage: string;
    FEnrollmentSamples: Integer;

    // ============================================================
    // REMOCAO DE IDENTIFICACAO FACIAL
    // ============================================================
    FRevokeRequestID: string;
    FRevokeEmployeeID: Integer;
    FRevokeEmployeeName: string;
    FRevokeTerminal: string;
    FRevokeStatus: string;
    FRevokeMessage: string;

    function NewRequestID(const APrefix: string): string;

    function NormalizePath(const APath: string): string;

    function ReadRequestBody(
      ARequestInfo: TIdHTTPRequestInfo): string;

    function EmployeesToJson: string;

    procedure EnsureFaceIdentityKeys;

    procedure JsonResponse(
      AResponseInfo: TIdHTTPResponseInfo;
      const AJson: string;
      const AStatusCode: Integer = 200);

    procedure HttpCommand(
      AContext: TIdContext;
      ARequestInfo: TIdHTTPRequestInfo;
      AResponseInfo: TIdHTTPResponseInfo);

  public
    constructor Create(
      AOwner: TComponent;
      const ADatabasePath: string;
      const APort: Integer = 3101); reintroduce;

    destructor Destroy; override;

    procedure Start;
    procedure Stop;

    function IsActive: Boolean;
    function BaseUrl: string;

    // Etapa 2: regra corporativa de autorizacao permanece no ERP.
    // Nao executa liberacao fisica e ainda nao e exposta por REST.
    function EvaluateEmployeeAccess(
      const AEmployeeID: Integer;
      const AAt: TDateTime;
      out AAuthorized: Boolean;
      out AReason: string;
      out AEmployeeName: string;
      out AWorkStart: string;
      out AWorkEnd: string): Boolean;

    property OnAccessEvent: TAccessEventNotify
      read FOnAccessEvent
      write FOnAccessEvent;
  end;

implementation

{ TPortariaApiServer }

function TPortariaApiServer.NewRequestID(
  const APrefix: string): string;
begin
  Result :=
    APrefix + '-' +
    FormatDateTime('yyyymmddhhnnsszzz', Now);
end;

constructor TPortariaApiServer.Create(
  AOwner: TComponent;
  const ADatabasePath: string;
  const APort: Integer);
begin
  inherited Create(AOwner);

  FServer := nil;
  FOperationLock := TCriticalSection.Create;

  // ------------------------------------------------------------
  // Estado inicial - enrollment
  // ------------------------------------------------------------
  FEnrollmentRequestID := '';
  FEnrollmentEmployeeID := 0;
  FEnrollmentEmployeeName := '';
  FEnrollmentTerminal := '';
  FEnrollmentStatus := '';
  FEnrollmentMessage := '';
  FEnrollmentSamples := 0;

  // ------------------------------------------------------------
  // Estado inicial - revoke
  // ------------------------------------------------------------
  FRevokeRequestID := '';
  FRevokeEmployeeID := 0;
  FRevokeEmployeeName := '';
  FRevokeTerminal := '';
  FRevokeStatus := '';
  FRevokeMessage := '';

  FDatabasePath := ADatabasePath;

  // Chave imutavel vinculo a identificacao facial
  // ao registro real de Employee.
  EnsureFaceIdentityKeys;

  FServer := TIdHTTPServer.Create(Self);
  FServer.DefaultPort := APort;
  FServer.KeepAlive := True;

  FServer.OnCommandGet := HttpCommand;
  FServer.OnCommandOther := HttpCommand;
end;

destructor TPortariaApiServer.Destroy;
begin
  Stop;

  FreeAndNil(FOperationLock);

  inherited;
end;

function TPortariaApiServer.EvaluateEmployeeAccess(
  const AEmployeeID: Integer;
  const AAt: TDateTime;
  out AAuthorized: Boolean;
  out AReason: string;
  out AEmployeeName: string;
  out AWorkStart: string;
  out AWorkEnd: string): Boolean;
var
  Conn: TFDConnection;
  Query: TFDQuery;
  AccessBlocked: Boolean;
  StartTime: TDateTime;
  EndTime: TDateTime;
  CurrentTime: TDateTime;
  WithinSchedule: Boolean;
begin
  // Result indica se a regra conseguiu ser avaliada tecnicamente.
  // AAuthorized representa a decisao de negocio.
  Result := False;
  AAuthorized := False;
  AReason := 'ACCESS_EVALUATION_ERROR';
  AEmployeeName := '';
  AWorkStart := '';
  AWorkEnd := '';

  if AEmployeeID <= 0 then
  begin
    AReason := 'INVALID_EMPLOYEE_ID';
    Exit;
  end;

  Conn := TFDConnection.Create(nil);
  Query := TFDQuery.Create(nil);
  try
    try
      Conn.LoginPrompt := False;
      Conn.ResourceOptions.SilentMode := True;
      Conn.Params.Clear;
      Conn.Params.Add('DriverID=SQLite');
      Conn.Params.Add('Database=' + FDatabasePath);
      Conn.Params.Add('OpenMode=ReadOnly');
      Conn.Connected := True;

      Query.Connection := Conn;
      Query.SQL.Text :=
        'select ID, Name, AccessBlocked, WorkStart, WorkEnd ' +
        'from Employee where ID = :ID';
      Query.ParamByName('ID').AsInteger := AEmployeeID;
      Query.Open;

      if Query.Eof then
      begin
        AReason := 'EMPLOYEE_NOT_FOUND';
        Result := True;
        Exit;
      end;

      AEmployeeName := Query.FieldByName('Name').AsString;
      AWorkStart := Trim(Query.FieldByName('WorkStart').AsString);
      AWorkEnd := Trim(Query.FieldByName('WorkEnd').AsString);
      AccessBlocked := Query.FieldByName('AccessBlocked').AsInteger <> 0;

      // Regra 1: bloqueio administrativo sempre tem precedencia.
      if AccessBlocked then
      begin
        AReason := 'EMPLOYEE_BLOCKED';
        Result := True;
        Exit;
      end;

      // Regra 2: expediente precisa estar configurado corretamente.
      if (not TryStrToTime(AWorkStart, StartTime)) or
         (not TryStrToTime(AWorkEnd, EndTime)) or
         (Frac(StartTime) = Frac(EndTime)) then
      begin
        AReason := 'INVALID_WORK_SCHEDULE';
        Result := True;
        Exit;
      end;

      CurrentTime := Frac(AAt);
      StartTime := Frac(StartTime);
      EndTime := Frac(EndTime);

      // Expediente normal, por exemplo 09:00 -> 18:00.
      if StartTime < EndTime then
        WithinSchedule :=
          (CurrentTime >= StartTime) and
          (CurrentTime <= EndTime)
      else
        // Tambem suporta expediente que cruza meia-noite,
        // por exemplo 22:00 -> 06:00.
        WithinSchedule :=
          (CurrentTime >= StartTime) or
          (CurrentTime <= EndTime);

      if not WithinSchedule then
      begin
        AReason := 'OUTSIDE_WORK_HOURS';
        Result := True;
        Exit;
      end;

      AAuthorized := True;
      AReason := 'ACCESS_GRANTED';
      Result := True;
    except
      on E: Exception do
      begin
        AAuthorized := False;
        AReason := 'ACCESS_EVALUATION_ERROR: ' + E.Message;
        Result := False;
      end;
    end;
  finally
    Query.Free;
    Conn.Free;
  end;
end;

procedure TPortariaApiServer.EnsureFaceIdentityKeys;
var
  Conn: TFDConnection;
  Query: TFDQuery;
  UpdateQuery: TFDQuery;
  IDs: TList<Integer>;
  HasFaceKey: Boolean;
  EmployeeID: Integer;
  NewGuid: TGUID;
  FaceKey: string;
begin
  Conn := TFDConnection.Create(nil);
  Query := TFDQuery.Create(nil);
  UpdateQuery := TFDQuery.Create(nil);
  IDs := TList<Integer>.Create;

  try
    Conn.LoginPrompt := False;
    Conn.ResourceOptions.SilentMode := True;

    Conn.Params.Clear;
    Conn.Params.Add('DriverID=SQLite');
    Conn.Params.Add('Database=' + FDatabasePath);
    Conn.Params.Add('OpenMode=ReadWriteCreate');

    Conn.Connected := True;

    Query.Connection := Conn;
    UpdateQuery.Connection := Conn;

    HasFaceKey := False;

    Query.SQL.Text :=
      'pragma table_info(Employee)';

    Query.Open;

    while not Query.Eof do
    begin
      if SameText(
        Query.FieldByName('name').AsString,
        'FaceKey') then
      begin
        HasFaceKey := True;
        Break;
      end;

      Query.Next;
    end;

    Query.Close;

    if not HasFaceKey then
      Conn.ExecSQL(
        'alter table Employee add column FaceKey TEXT');

    Query.SQL.Text :=
      'select ID from Employee ' +
      'where FaceKey is null or trim(FaceKey) = ''''';

    Query.Open;

    while not Query.Eof do
    begin
      IDs.Add(
        Query.FieldByName('ID').AsInteger);

      Query.Next;
    end;

    Query.Close;

    for EmployeeID in IDs do
    begin
      if CreateGUID(NewGuid) <> 0 then
        raise Exception.Create(
          'Nao foi possivel gerar FaceKey para EmployeeId ' +
          EmployeeID.ToString);

      FaceKey := GUIDToString(NewGuid);

      UpdateQuery.Close;

      UpdateQuery.SQL.Text :=
        'update Employee ' +
        'set FaceKey = :FaceKey ' +
        'where ID = :ID';

      UpdateQuery
        .ParamByName('FaceKey')
        .AsString := FaceKey;

      UpdateQuery
        .ParamByName('ID')
        .AsInteger := EmployeeID;

      UpdateQuery.ExecSQL;
    end;

  finally
    IDs.Free;
    UpdateQuery.Free;
    Query.Free;
    Conn.Free;
  end;
end;

function TPortariaApiServer.NormalizePath(
  const APath: string): string;
begin
  Result := LowerCase(Trim(APath));

  if Result = '' then
    Result := '/';

  if (Length(Result) > 1) and
     Result.EndsWith('/') then
    Delete(Result, Length(Result), 1);
end;

function TPortariaApiServer.ReadRequestBody(
  ARequestInfo: TIdHTTPRequestInfo): string;
var
  Buffer: TStringStream;
begin
  Result := '';

  if (ARequestInfo = nil) or
     (ARequestInfo.PostStream = nil) or
     (ARequestInfo.PostStream.Size = 0) then
    Exit;

  ARequestInfo.PostStream.Position := 0;

  Buffer := TStringStream.Create(
    '',
    TEncoding.UTF8);

  try
    Buffer.CopyFrom(
      ARequestInfo.PostStream,
      ARequestInfo.PostStream.Size);

    Result := Buffer.DataString;

  finally
    Buffer.Free;
  end;
end;

procedure TPortariaApiServer.JsonResponse(
  AResponseInfo: TIdHTTPResponseInfo;
  const AJson: string;
  const AStatusCode: Integer);
begin
  AResponseInfo.ResponseNo := AStatusCode;
  AResponseInfo.ContentType := 'application/json';
  AResponseInfo.CharSet := 'utf-8';
  AResponseInfo.ContentText := AJson;
end;

function TPortariaApiServer.EmployeesToJson: string;
var
  Conn: TFDConnection;
  Query: TFDQuery;
  Root: TJSONObject;
  Items: TJSONArray;
  Obj: TJSONObject;
begin
  Root := TJSONObject.Create;
  Items := TJSONArray.Create;

  Conn := TFDConnection.Create(nil);
  Query := TFDQuery.Create(nil);

  try
    Root.AddPair(
      'source',
      'delphi-system');

    Root.AddPair(
      'database',
      FDatabasePath);

    Root.AddPair(
      'employees',
      Items);

    Conn.LoginPrompt := False;
    Conn.ResourceOptions.SilentMode := True;

    Conn.Params.Clear;
    Conn.Params.Add('DriverID=SQLite');
    Conn.Params.Add('Database=' + FDatabasePath);
    Conn.Params.Add('OpenMode=ReadOnly');

    Conn.Connected := True;

    Query.Connection := Conn;

    Query.SQL.Text :=
      'select ID, Name, Phone, [E-mail], ' +
      'Department, Seniority, FaceKey, ' +
      'AccessBlocked, WorkStart, WorkEnd ' +
      'from Employee ' +
      'order by Name';

    Query.Open;

    while not Query.Eof do
    begin
      Obj := TJSONObject.Create;

      Obj.AddPair(
        'id',
        TJSONNumber.Create(
          Query.FieldByName('ID').AsInteger));

      Obj.AddPair(
        'name',
        Query.FieldByName('Name').AsString);

      Obj.AddPair(
        'phone',
        Query.FieldByName('Phone').AsString);

      Obj.AddPair(
        'email',
        Query.FieldByName('E-mail').AsString);

      Obj.AddPair(
        'department',
        Query.FieldByName('Department').AsString);

      Obj.AddPair(
        'faceKey',
        Query.FieldByName('FaceKey').AsString);

      Obj.AddPair(
        'accessBlocked',
        TJSONBool.Create(Query.FieldByName('AccessBlocked').AsInteger <> 0));

      Obj.AddPair('workStart', Query.FieldByName('WorkStart').AsString);
      Obj.AddPair('workEnd', Query.FieldByName('WorkEnd').AsString);

      if Query.FieldByName('Seniority').IsNull then
      begin
        Obj.AddPair(
          'seniority',
          TJSONNull.Create);
      end
      else
      begin
        Obj.AddPair(
          'seniority',
          TJSONNumber.Create(
            Query.FieldByName('Seniority').AsInteger));
      end;

      Items.AddElement(Obj);

      Query.Next;
    end;

    Root.AddPair(
      'count',
      TJSONNumber.Create(Items.Count));

    Result := Root.ToJSON;

  finally
    Query.Free;
    Conn.Free;
    Root.Free;
  end;
end;

procedure TPortariaApiServer.HttpCommand(
  AContext: TIdContext;
  ARequestInfo: TIdHTTPRequestInfo;
  AResponseInfo: TIdHTTPResponseInfo);
var
  Path: string;
  Method: string;
  Body: string;

  Json: TJSONValue;
  Obj: TJSONObject;

  Ack: TJSONObject;

  EmployeeID: Integer;
  EmployeeName: string;
  Terminal: string;
  Credential: string;
  RequestID: string;
  Authorized: Boolean;
  AccessReason: string;
  WorkStart: string;
  WorkEnd: string;
  EvaluationOk: Boolean;
  Samples: Integer;
  ProgressMessage: string;
  ResultStatus: string;
  ResultMessage: string;
  RequestMatches: Boolean;

  StatusRoot: TJSONObject;
  EnrollmentObj: TJSONObject;
  RevokeObj: TJSONObject;

  ErrorJson: TJSONString;
begin
  Path :=
    NormalizePath(
      ARequestInfo.Document);

  Method :=
    UpperCase(
      ARequestInfo.Command);

  try

    // ============================================================
    // HEALTH
    // ============================================================

    // GET http://127.0.0.1:3101/api
    // GET http://127.0.0.1:3101/api/health
    if (Method = 'GET') and
       (
         (Path = '/api') or
         (Path = '/api/health')
       ) then
    begin
      JsonResponse(
        AResponseInfo,
        '{"status":"ok",' +
        '"service":"delphi-system",' +
        '"role":"erp",' +
        '"apiVersion":"1.2",' +
        '"portariaIntegration":true,' +
        '"operationalChannel":true}');

      Exit;
    end;

    // ============================================================
    // FUNCIONARIOS
    // ============================================================

    // GET http://127.0.0.1:3101/api/employees
    if (Method = 'GET') and
       (Path = '/api/employees') then
    begin
      JsonResponse(
        AResponseInfo,
        EmployeesToJson);

      Exit;
    end;

    // ============================================================
    // ETAPA 3 - DECISAO DE ACESSO CORPORATIVA
    // ============================================================
    // POST http://127.0.0.1:3101/api/access/authorize
    //
    // O ERP e a autoridade da regra de negocio. A Portaria informa
    // somente quem foi identificado, qual terminal realizou a leitura
    // e a credencial utilizada (FACE/RFID). Nesta etapa o endpoint e
    // validado isoladamente; a Portaria C++ ainda nao o consome.
    //
    // Exemplo de entrada:
    // {"employeeId":8,"terminal":"PORTARIA-01","credential":"FACE"}
    //
    if (Method = 'POST') and
       (Path = '/api/access/authorize') then
    begin
      Body := ReadRequestBody(ARequestInfo);

      if Trim(Body) = '' then
      begin
        JsonResponse(
          AResponseInfo,
          '{"ok":false,"error":"empty_body"}',
          400);
        Exit;
      end;

      Json := TJSONObject.ParseJSONValue(Body);
      try
        if not (Json is TJSONObject) then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_json"}',
            400);
          Exit;
        end;

        Obj := TJSONObject(Json);
        EmployeeID := 0;
        Terminal := '';
        Credential := '';

        Obj.TryGetValue<Integer>('employeeId', EmployeeID);
        Obj.TryGetValue<string>('terminal', Terminal);
        Obj.TryGetValue<string>('credential', Credential);

        Terminal := Trim(Terminal);
        Credential := UpperCase(Trim(Credential));

        if EmployeeID <= 0 then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_employee_id"}',
            400);
          Exit;
        end;

        // Terminal e credential fazem parte do contrato entre ERP e
        // Portaria. Ainda nao alteram a decisao, mas deixam a origem
        // da solicitacao explicitamente identificada.
        if Terminal = '' then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_terminal"}',
            400);
          Exit;
        end;

        if (Credential <> 'FACE') and
           (Credential <> 'RFID') then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_credential"}',
            400);
          Exit;
        end;

        EvaluationOk := EvaluateEmployeeAccess(
          EmployeeID,
          Now,
          Authorized,
          AccessReason,
          EmployeeName,
          WorkStart,
          WorkEnd);

        // Falha tecnica na avaliacao e diferente de uma negacao de
        // negocio. Negacoes como EMPLOYEE_BLOCKED e OUTSIDE_WORK_HOURS
        // retornam HTTP 200 com authorized=false.
        if not EvaluationOk then
        begin
          Ack := TJSONObject.Create;
          try
            Ack.AddPair('ok', TJSONBool.Create(False));
            Ack.AddPair('authorized', TJSONBool.Create(False));
            Ack.AddPair('employeeId', TJSONNumber.Create(EmployeeID));
            Ack.AddPair('terminal', Terminal);
            Ack.AddPair('credential', Credential);
            Ack.AddPair('reason', AccessReason);
            Ack.AddPair(
              'evaluatedAt',
              FormatDateTime('yyyy-mm-dd"T"hh:nn:ss', Now));

            JsonResponse(AResponseInfo, Ack.ToJSON, 500);
          finally
            Ack.Free;
          end;
          Exit;
        end;

        Ack := TJSONObject.Create;
        try
          Ack.AddPair('ok', TJSONBool.Create(True));
          Ack.AddPair('authorized', TJSONBool.Create(Authorized));
          Ack.AddPair('employeeId', TJSONNumber.Create(EmployeeID));
          Ack.AddPair('employeeName', EmployeeName);
          Ack.AddPair('terminal', Terminal);
          Ack.AddPair('credential', Credential);
          Ack.AddPair('reason', AccessReason);
          Ack.AddPair('workStart', WorkStart);
          Ack.AddPair('workEnd', WorkEnd);
          Ack.AddPair(
            'evaluatedAt',
            FormatDateTime('yyyy-mm-dd"T"hh:nn:ss', Now));

          JsonResponse(AResponseInfo, Ack.ToJSON, 200);
        finally
          Ack.Free;
        end;
      finally
        Json.Free;
      end;

      Exit;
    end;

         // ============================================================
    // IDENTIFICACAO FACIAL - CONSULTAR SOLICITACAO PENDENTE
    // ============================================================

    // GET
    // http://127.0.0.1:3101/api/enrollment/pending
    //
    // Este endpoint sera consultado futuramente pela Portaria C++Builder.
    // Por enquanto ele apenas expoe a solicitacao que esta em memoria.
    //
    if (Method = 'GET') and
       (Path = '/api/enrollment/pending') then
    begin
      Ack := TJSONObject.Create;
      try
        FOperationLock.Acquire;
        try
          if (FEnrollmentRequestID <> '') and
             SameText(FEnrollmentStatus, 'pending') then
          begin
            Ack.AddPair(
              'pending',
              TJSONBool.Create(True));

            Ack.AddPair(
              'requestId',
              FEnrollmentRequestID);

            Ack.AddPair(
              'employeeId',
              TJSONNumber.Create(
                FEnrollmentEmployeeID));

            Ack.AddPair(
              'employeeName',
              FEnrollmentEmployeeName);

            Ack.AddPair(
              'terminal',
              FEnrollmentTerminal);

            Ack.AddPair(
              'status',
              FEnrollmentStatus);

            Ack.AddPair(
              'samples',
              TJSONNumber.Create(
                FEnrollmentSamples));
          end
          else
          begin
            Ack.AddPair(
              'pending',
              TJSONBool.Create(False));
          end;

        finally
          FOperationLock.Release;
        end;

        JsonResponse(
          AResponseInfo,
          Ack.ToJSON);

      finally
        Ack.Free;
      end;

      Exit;
    end;

        // ============================================================
    // IDENTIFICACAO FACIAL - PROGRESSO DO CADASTRO
    // ============================================================

    // POST
    // http://127.0.0.1:3101/api/enrollment/progress
    //
    // A Portaria C++Builder usara este endpoint para informar
    // o progresso da captura: 1/8, 2/8 ... 8/8.
    //
    // IMPORTANTE:
    // Nesta etapa mantemos o status como "pending".
    // Alteramos apenas samples e message para nao modificar
    // ainda o comportamento atual do ERP.
    //
    if (Method = 'POST') and
       (Path = '/api/enrollment/progress') then
    begin
      Body := ReadRequestBody(ARequestInfo);

      if Trim(Body) = '' then
      begin
        JsonResponse(
          AResponseInfo,
          '{"ok":false,"error":"empty_body"}',
          400);

        Exit;
      end;

      Json :=
        TJSONObject.ParseJSONValue(Body);

      try
        if not (Json is TJSONObject) then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_json"}',
            400);

          Exit;
        end;

        Obj := TJSONObject(Json);

        RequestID := '';
        Samples := 0;
        ProgressMessage := '';

        Obj.TryGetValue<string>(
          'requestId',
          RequestID);

        Obj.TryGetValue<Integer>(
          'samples',
          Samples);

        Obj.TryGetValue<string>(
          'message',
          ProgressMessage);

        RequestID := Trim(RequestID);
        ProgressMessage := Trim(ProgressMessage);

        if RequestID = '' then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_request_id"}',
            400);

          Exit;
        end;

        if (Samples < 0) or (Samples > 8) then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_samples"}',
            400);

          Exit;
        end;



        FOperationLock.Acquire;
        try
          // Nunca permitir que o progresso de uma solicitacao
          // altere outra solicitacao atualmente ativa.
          if not SameText(
               RequestID,
               FEnrollmentRequestID) then




          begin
            JsonResponse(
              AResponseInfo,
              '{"ok":false,"error":"request_not_found"}',
              404);

            Exit;
          end;

          FEnrollmentSamples := Samples;

          if ProgressMessage <> '' then
            FEnrollmentMessage := ProgressMessage
          else
            FEnrollmentMessage :=
              Format(
                'Capturando identificacao facial: %d/8.',
                [Samples]);

        finally
          FOperationLock.Release;
        end;

        Ack := TJSONObject.Create;
        try
          Ack.AddPair(
            'ok',
            TJSONBool.Create(True));

          Ack.AddPair(
            'requestId',
            RequestID);

          Ack.AddPair(
            'status',
            'pending');

          Ack.AddPair(
            'samples',
            TJSONNumber.Create(Samples));

          JsonResponse(
            AResponseInfo,
            Ack.ToJSON);

        finally
          Ack.Free;
        end;

      finally
        Json.Free;
      end;

      Exit;
    end;

    // ============================================================
    // REMOCAO FACIAL - CONSULTAR PENDENCIA
    // ============================================================
    if (Method = 'GET') and
       (Path = '/api/revoke/pending') then
    begin
      Ack := TJSONObject.Create;
      try
        FOperationLock.Acquire;
        try
          if (FRevokeRequestID <> '') and
             SameText(FRevokeStatus, 'pending') then
          begin
            Ack.AddPair('pending', TJSONBool.Create(True));
            Ack.AddPair('requestId', FRevokeRequestID);
            Ack.AddPair(
              'employeeId',
              TJSONNumber.Create(FRevokeEmployeeID));
            Ack.AddPair('employeeName', FRevokeEmployeeName);
            Ack.AddPair('terminal', FRevokeTerminal);
            Ack.AddPair('status', FRevokeStatus);
          end
          else
          begin
            Ack.AddPair('pending', TJSONBool.Create(False));
          end;
        finally
          FOperationLock.Release;
        end;

        JsonResponse(AResponseInfo, Ack.ToJSON);
      finally
        Ack.Free;
      end;

      Exit;
    end;


         // ============================================================
    // IDENTIFICACAO FACIAL - RESULTADO FINAL DO CADASTRO
    // ============================================================

    // POST
    // http://127.0.0.1:3101/api/enrollment/result
    //
    // Status aceitos:
    //   completed
    //   failed
    //
    if (Method = 'POST') and
       (Path = '/api/enrollment/result') then
    begin
      Body := ReadRequestBody(ARequestInfo);

      if Trim(Body) = '' then
      begin
        JsonResponse(
          AResponseInfo,
          '{"ok":false,"error":"empty_body"}',
          400);

        Exit;
      end;

      Json :=
        TJSONObject.ParseJSONValue(Body);

      try
        if not (Json is TJSONObject) then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_json"}',
            400);

          Exit;
        end;

        Obj := TJSONObject(Json);

        RequestID := '';
        ResultStatus := '';
        ResultMessage := '';
        Samples := 0;

        Obj.TryGetValue<string>(
          'requestId',
          RequestID);

        Obj.TryGetValue<string>(
          'status',
          ResultStatus);

        Obj.TryGetValue<string>(
          'message',
          ResultMessage);

        Obj.TryGetValue<Integer>(
          'samples',
          Samples);

        RequestID := Trim(RequestID);

        ResultStatus :=
          LowerCase(
            Trim(ResultStatus));

        ResultMessage :=
          Trim(ResultMessage);

        if RequestID = '' then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_request_id"}',
            400);

          Exit;
        end;

        if not (
             SameText(ResultStatus, 'completed') or
             SameText(ResultStatus, 'failed')
           ) then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_status"}',
            400);

          Exit;
        end;

        if (Samples < 0) or
           (Samples > 8) then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_samples"}',
            400);

          Exit;
        end;

        RequestMatches := False;

        FOperationLock.Acquire;
        try
          RequestMatches :=
            SameText(
              RequestID,
              FEnrollmentRequestID);

          if RequestMatches then
          begin
            FEnrollmentStatus :=
              ResultStatus;

            FEnrollmentSamples :=
              Samples;

            if ResultMessage <> '' then
            begin
              FEnrollmentMessage :=
                ResultMessage;
            end
            else if SameText(
                    ResultStatus,
                    'completed') then
            begin
              FEnrollmentMessage :=
                'Identificacao facial cadastrada com sucesso.';
            end
            else
            begin
              FEnrollmentMessage :=
                'Falha no cadastro da identificacao facial.';
            end;
          end;

        finally
          FOperationLock.Release;
        end;

        if not RequestMatches then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"request_not_found"}',
            404);

          Exit;
        end;

        Ack := TJSONObject.Create;
        try
          Ack.AddPair(
            'ok',
            TJSONBool.Create(True));

          Ack.AddPair(
            'requestId',
            RequestID);

          Ack.AddPair(
            'status',
            ResultStatus);

          Ack.AddPair(
            'samples',
            TJSONNumber.Create(Samples));

          JsonResponse(
            AResponseInfo,
            Ack.ToJSON);

        finally
          Ack.Free;
        end;

      finally
        Json.Free;
      end;

      Exit;
    end;


    // ============================================================
    // IDENTIFICACAO FACIAL - SOLICITAR CADASTRO
    // ============================================================

    // POST
    // http://127.0.0.1:3101/api/enrollment/request
    if (Method = 'POST') and
       (Path = '/api/enrollment/request') then
    begin
      Body := ReadRequestBody(ARequestInfo);

      if Trim(Body) = '' then
      begin
        JsonResponse(
          AResponseInfo,
          '{"ok":false,"error":"empty_body"}',
          400);

        Exit;
      end;

      Json :=
        TJSONObject.ParseJSONValue(Body);

      try
        if not (Json is TJSONObject) then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_json"}',
            400);

          Exit;
        end;

        Obj := TJSONObject(Json);

        EmployeeID := 0;
        EmployeeName := '';
        Terminal := 'PORTARIA-01';

        Obj.TryGetValue<Integer>(
          'employeeId',
          EmployeeID);

        Obj.TryGetValue<string>(
          'employeeName',
          EmployeeName);

        Obj.TryGetValue<string>(
          'terminal',
          Terminal);

        EmployeeName := Trim(EmployeeName);
        Terminal := Trim(Terminal);

        if EmployeeID <= 0 then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_employee_id"}',
            400);

          Exit;
        end;

        if EmployeeName = '' then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_employee_name"}',
            400);

          Exit;
        end;

        if Terminal = '' then
          Terminal := 'PORTARIA-01';

        RequestID :=
          NewRequestID('ENR');

        FOperationLock.Acquire;
        try
          FEnrollmentRequestID :=
            RequestID;

          FEnrollmentEmployeeID :=
            EmployeeID;

          FEnrollmentEmployeeName :=
            EmployeeName;

          FEnrollmentTerminal :=
            Terminal;

          FEnrollmentStatus :=
            'pending';

          FEnrollmentMessage :=
            'Aguardando captura pela Portaria.';

          FEnrollmentSamples :=
            0;
        finally
          FOperationLock.Release;
        end;

        Ack := TJSONObject.Create;
        try
          Ack.AddPair(
            'ok',
            TJSONBool.Create(True));

          Ack.AddPair(
            'requestId',
            RequestID);

          Ack.AddPair(
            'status',
            'pending');

          Ack.AddPair(
            'employeeId',
            TJSONNumber.Create(EmployeeID));

          Ack.AddPair(
            'employeeName',
            EmployeeName);

          Ack.AddPair(
            'terminal',
            Terminal);

          JsonResponse(
            AResponseInfo,
            Ack.ToJSON,
            201);

        finally
          Ack.Free;
        end;

      finally
        Json.Free;
      end;

      Exit;
    end;

    // ============================================================
    // IDENTIFICACAO FACIAL - SOLICITAR REMOCAO
    // ============================================================

    // POST
    // http://127.0.0.1:3101/api/biometric/revoke
    if (Method = 'POST') and
       (Path = '/api/biometric/revoke') then
    begin
      Body := ReadRequestBody(ARequestInfo);

      if Trim(Body) = '' then
      begin
        JsonResponse(
          AResponseInfo,
          '{"ok":false,"error":"empty_body"}',
          400);

        Exit;
      end;

      Json :=
        TJSONObject.ParseJSONValue(Body);

      try
        if not (Json is TJSONObject) then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_json"}',
            400);

          Exit;
        end;

        Obj := TJSONObject(Json);

        EmployeeID := 0;
        EmployeeName := '';
        Terminal := 'PORTARIA-01';

        Obj.TryGetValue<Integer>(
          'employeeId',
          EmployeeID);

        Obj.TryGetValue<string>(
          'employeeName',
          EmployeeName);

        Obj.TryGetValue<string>(
          'terminal',
          Terminal);

        EmployeeName := Trim(EmployeeName);
        Terminal := Trim(Terminal);

        if EmployeeID <= 0 then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_employee_id"}',
            400);

          Exit;
        end;

        if Terminal = '' then
          Terminal := 'PORTARIA-01';

        RequestID :=
          NewRequestID('REV');

        FOperationLock.Acquire;
        try
          FRevokeRequestID :=
            RequestID;

          FRevokeEmployeeID :=
            EmployeeID;

          FRevokeEmployeeName :=
            EmployeeName;

          FRevokeTerminal :=
            Terminal;

          FRevokeStatus :=
            'pending';

          FRevokeMessage :=
            'Aguardando remocao pela Portaria.';
        finally
          FOperationLock.Release;
        end;

        Ack := TJSONObject.Create;
        try
          Ack.AddPair(
            'ok',
            TJSONBool.Create(True));

          Ack.AddPair(
            'requestId',
            RequestID);

          Ack.AddPair(
            'status',
            'pending');

          Ack.AddPair(
            'employeeId',
            TJSONNumber.Create(EmployeeID));

          Ack.AddPair(
            'employeeName',
            EmployeeName);

          Ack.AddPair(
            'terminal',
            Terminal);

          JsonResponse(
            AResponseInfo,
            Ack.ToJSON,
            201);

        finally
          Ack.Free;
        end;

      finally
        Json.Free;
      end;

      Exit;
    end;

    // ============================================================
    // IDENTIFICACAO FACIAL - STATUS OPERACIONAL
    // ============================================================

    // GET
    // http://127.0.0.1:3101/api/biometric/status
    //
    // Mantem propositalmente o mesmo formato de JSON que o
    // untPrincipalDS.pas atualmente espera do WebServer.
    //
    if (Method = 'GET') and
       (Path = '/api/biometric/status') then
    begin
      StatusRoot := TJSONObject.Create;

      try
        FOperationLock.Acquire;
        try

          // --------------------------------------------------------
          // Enrollment
          // --------------------------------------------------------
          if FEnrollmentRequestID <> '' then
          begin
            EnrollmentObj :=
              TJSONObject.Create;

            EnrollmentObj.AddPair(
              'requestId',
              FEnrollmentRequestID);

            EnrollmentObj.AddPair(
              'employeeId',
              TJSONNumber.Create(
                FEnrollmentEmployeeID));

            EnrollmentObj.AddPair(
              'employeeName',
              FEnrollmentEmployeeName);

            EnrollmentObj.AddPair(
              'terminal',
              FEnrollmentTerminal);

            EnrollmentObj.AddPair(
              'status',
              FEnrollmentStatus);

            EnrollmentObj.AddPair(
              'message',
              FEnrollmentMessage);

            EnrollmentObj.AddPair(
              'samples',
              TJSONNumber.Create(
                FEnrollmentSamples));

            StatusRoot.AddPair(
              'enrollment',
              EnrollmentObj);
          end
          else
          begin
            StatusRoot.AddPair(
              'enrollment',
              TJSONNull.Create);
          end;

          // --------------------------------------------------------
          // Revoke
          // --------------------------------------------------------
          if FRevokeRequestID <> '' then
          begin
            RevokeObj :=
              TJSONObject.Create;

            RevokeObj.AddPair(
              'requestId',
              FRevokeRequestID);

            RevokeObj.AddPair(
              'employeeId',
              TJSONNumber.Create(
                FRevokeEmployeeID));

            RevokeObj.AddPair(
              'employeeName',
              FRevokeEmployeeName);

            RevokeObj.AddPair(
              'terminal',
              FRevokeTerminal);

            RevokeObj.AddPair(
              'status',
              FRevokeStatus);

            RevokeObj.AddPair(
              'message',
              FRevokeMessage);

            StatusRoot.AddPair(
              'revoke',
              RevokeObj);
          end
          else
          begin
            StatusRoot.AddPair(
              'revoke',
              TJSONNull.Create);
          end;

        finally
          FOperationLock.Release;
        end;

        JsonResponse(
          AResponseInfo,
          StatusRoot.ToJSON);

      finally
        StatusRoot.Free;
      end;

      Exit;
    end;

    // ============================================================
    // EVENTOS DE ACESSO
    // ============================================================

    // POST
    // http://127.0.0.1:3101/api/access/events
    if (Method = 'POST') and
       (Path = '/api/access/events') then
    begin
      Body :=
        ReadRequestBody(ARequestInfo);

      if Trim(Body) = '' then
      begin
        JsonResponse(
          AResponseInfo,
          '{"ok":false,"error":"empty_body"}',
          400);

        Exit;
      end;

      Json :=
        TJSONObject.ParseJSONValue(Body);

      try
        if Json = nil then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_json"}',
            400);

          Exit;
        end;
      finally
        Json.Free;
      end;

      if Assigned(FOnAccessEvent) then
      begin
        TThread.Queue(
          nil,
          procedure
          begin
            if Assigned(FOnAccessEvent) then
              FOnAccessEvent(Body);
          end);
      end;

      Ack := TJSONObject.Create;

      try
        Ack.AddPair(
          'ok',
          TJSONBool.Create(True));

        Ack.AddPair(
          'receivedBy',
          'delphi-system');

        Ack.AddPair(
          'receivedAt',
          FormatDateTime(
            'yyyy-mm-dd"T"hh:nn:ss',
            Now));

        JsonResponse(
          AResponseInfo,
          Ack.ToJSON,
          201);

      finally
        Ack.Free;
      end;

      Exit;
    end;

        // ============================================================
    // IDENTIFICACAO FACIAL - RESULTADO DA REMOCAO
    // ============================================================
    // POST http://127.0.0.1:3101/api/revoke/result
    if (Method = 'POST') and
       (Path = '/api/revoke/result') then
    begin
      Body := ReadRequestBody(ARequestInfo);

      if Trim(Body) = '' then
      begin
        JsonResponse(
          AResponseInfo,
          '{"ok":false,"error":"empty_body"}',
          400);
        Exit;
      end;

      Json := TJSONObject.ParseJSONValue(Body);
      try
        if not (Json is TJSONObject) then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_json"}',
            400);
          Exit;
        end;

        Obj := TJSONObject(Json);

        RequestID := '';
        ResultStatus := '';
        ResultMessage := '';

        Obj.TryGetValue<string>(
          'requestId',
          RequestID);

        Obj.TryGetValue<string>(
          'status',
          ResultStatus);

        Obj.TryGetValue<string>(
          'message',
          ResultMessage);

        RequestID := Trim(RequestID);
        ResultStatus := LowerCase(Trim(ResultStatus));
        ResultMessage := Trim(ResultMessage);

        if RequestID = '' then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_request_id"}',
            400);
          Exit;
        end;

        if not (
             SameText(ResultStatus, 'completed') or
             SameText(ResultStatus, 'failed')
           ) then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"invalid_status"}',
            400);
          Exit;
        end;

        RequestMatches := False;

        FOperationLock.Acquire;
        try
          RequestMatches :=
            SameText(
              RequestID,
              FRevokeRequestID);

          if RequestMatches then
          begin
            FRevokeStatus := ResultStatus;

            if ResultMessage <> '' then
              FRevokeMessage := ResultMessage
            else if SameText(ResultStatus, 'completed') then
              FRevokeMessage :=
                'Identificacao facial removida com sucesso.'
            else
              FRevokeMessage :=
                'Falha ao remover identificacao facial.';
          end;
        finally
          FOperationLock.Release;
        end;

        if not RequestMatches then
        begin
          JsonResponse(
            AResponseInfo,
            '{"ok":false,"error":"request_not_found"}',
            404);
          Exit;
        end;

        Ack := TJSONObject.Create;
        try
          Ack.AddPair(
            'ok',
            TJSONBool.Create(True));

          Ack.AddPair(
            'requestId',
            RequestID);

          Ack.AddPair(
            'status',
            ResultStatus);

          JsonResponse(
            AResponseInfo,
            Ack.ToJSON);
        finally
          Ack.Free;
        end;

      finally
        Json.Free;
      end;

      Exit;
    end;


    // ============================================================
    // ROTA NAO ENCONTRADA
    // ============================================================

    JsonResponse(
      AResponseInfo,
      '{"ok":false,"error":"not_found"}',
      404);

  except
    on E: Exception do
    begin
      ErrorJson := TJSONString.Create(E.Message);
      try
        JsonResponse(
          AResponseInfo,
          '{"ok":false,"error":' +
          ErrorJson.ToJSON +
          '}',
          500);
      finally
        ErrorJson.Free;
      end;
    end;
  end;
end;

procedure TPortariaApiServer.Start;
begin
  if Assigned(FServer) and
     not FServer.Active then
    FServer.Active := True;
end;

procedure TPortariaApiServer.Stop;
begin
  if Assigned(FServer) and
     FServer.Active then
    FServer.Active := False;
end;

function TPortariaApiServer.IsActive: Boolean;
begin
  Result :=
    Assigned(FServer) and
    FServer.Active;
end;

function TPortariaApiServer.BaseUrl: string;
begin
  if not Assigned(FServer) then
  begin
    Result := '';
    Exit;
  end;

  Result :=
    Format(
      'http://127.0.0.1:%d/api',
      [FServer.DefaultPort]);
end;

end.
