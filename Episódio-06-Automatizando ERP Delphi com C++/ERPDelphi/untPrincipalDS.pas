unit untPrincipalDS;

interface

uses
  System.SysUtils, System.Types, System.UITypes, System.Classes, System.Variants,
  System.IOUtils, System.Math, System.JSON, Data.DB,
  System.Net.URLClient, System.Net.HttpClient, System.Net.HttpClientComponent,
  untPortariaApi, ObservabilityHeartbeat,
  {$IFDEF MSWINDOWS}
  Winapi.Windows,
  {$ENDIF}
  FMX.Types, FMX.Controls, FMX.Forms, FMX.Graphics, FMX.Dialogs, FMX.Objects,
  FMX.StdCtrls, FMX.Layouts, FMX.Controls.Presentation, FMX.TabControl, FMX.Edit,
  FMX.Effects, FMX.Ani, FMX.ScrollBox, FMX.ListBox, FMX.Memo,
  FireDAC.Stan.Intf, FireDAC.Stan.Option, FireDAC.Stan.Error,
  FireDAC.Stan.Def, FireDAC.Stan.Pool, FireDAC.Stan.Async, FireDAC.Stan.Param,
  FireDAC.Comp.Client, FireDAC.Comp.DataSet, FireDAC.DApt,
  FireDAC.Phys, FireDAC.Phys.SQLite, FireDAC.Phys.SQLiteDef,
  FireDAC.FMXUI.Wait, FMX.Memo.Types;

type
  TSetorCRM = (scDashboard, scCompras, scSuporte, scVendas, scRH);

  TForm2 = class(TForm)
    lytRoot: TLayout;
    rctSidebar: TRectangle;
    imgLogo: TImage;
    lblProduto: TLabel;
    lblSubtitulo: TLabel;
    navDashboard: TRectangle;
    lblNavDashboard: TLabel;
    navCompras: TRectangle;
    lblNavCompras: TLabel;
    navSuporte: TRectangle;
    lblNavSuporte: TLabel;
    navVendas: TRectangle;
    lblNavVendas: TLabel;
    navRH: TRectangle;
    lblNavRH: TLabel;
    rctHeader: TRectangle;
    lblTitulo: TLabel;
    lblDescricao: TLabel;
    lblStatus: TLabel;
    lblRedeTitulo: TLabel;
    lblRedeIPv4: TLabel;
    lblRedeDNS: TLabel;
    lblRedeGateway: TLabel;
    tcSetores: TTabControl;
    tabDashboard: TTabItem;
    tabCompras: TTabItem;
    tabSuporte: TTabItem;
    tabVendas: TTabItem;
    tabRH: TTabItem;
    edtFuncionarioNome: TEdit;
    edtFuncionarioSetor: TEdit;
    edtFuncionarioEmail: TEdit;
    edtFuncionarioCelular: TEdit;
    edtFuncionarioSeniority: TEdit;
    edtFuncionarioEntrada: TEdit;
    edtFuncionarioSaida: TEdit;
    chkFuncionarioBloqueado: TCheckBox;
    lstFuncionarios: TListBox;
    btnConsultarFuncionario: TButton;
    btnNovoFuncionario: TButton;
    btnSalvarFuncionario: TButton;
    btnExcluirFuncionario: TButton;
    btnCadastrarIdentificacaoFacial: TButton;
    btnRemoverIdentificacaoFacial: TButton;
    btnTestarRegraAcesso: TButton;
    lblDbStatus: TLabel;
    pnelEsquerda: TRectangle;
    pnelMsgCompras: TRectangle;
    pnelAprovacoesDia: TRectangle;
    lblAprovacoesDia: TLabel;
    lblDescAprovDia: TLabel;
    lblComprasFornecedores: TLabel;
    lblDescComprasFornecedores: TLabel;
    pnelDireitaRH: TRectangle;
    lblSeniority: TLabel;
    lblEntrada: TLabel;
    lblSaida: TLabel;
    pnelFluxoModuloRH: TRectangle;
    lblCadastroRH: TLabel;
    lblNameFuncionario: TLabel;
    lblEmailFuncionario: TLabel;
    lblDepartment: TLabel;
    lblPhone: TLabel;
    lblFluxoModuloRH: TLabel;
    lblDescFluxoModuloRh: TLabel;
    lblFuncionarioBanco: TLabel;
    lblSuporteAtendimento: TRectangle;
    lblSuporteEAtendimento: TLabel;
    pnelIndicadoresSLA: TRectangle;
    lblDescSuporteAtendimento: TLabel;
    lblComercialPipeline: TRectangle;
    lblMetasEquipe: TLabel;
    lblDescMetasEquipe: TLabel;
    pnelMetasEquipe: TRectangle;
    lblComerciaElPipeline: TLabel;
    lblDescComercialPipeline: TLabel;
    lblReceitaPrevista: TRectangle;
    pnelResumoOperacional: TRectangle;
    pnelMetaAtingido: TRectangle;
    pnelResumoDepartamentos: TRectangle;
    lblReceitaPrevistaa: TLabel;
    lblValorReceitaPrevista: TLabel;
    lblResumoDeDepartamentos: TLabel;
    lblDescResumoDepartamentos: TLabel;
    lblResumoOperacional: TLabel;
    lblDescResumoOperacional: TLabel;
    lblIndicadoresSla: TLabel;
    lblDescIncidacoresSLA: TLabel;
    lblMetaXAtingido: TLabel;
    arcVendasPorc: TArc;
    lblComprasAtingido: TLabel;
    lblMetasCompras: TLabel;
    lblMetaCompras: TLabel;
    lblPorcCompras: TLabel;
    lblPorcVendas: TLabel;
    lblMetaVendas: TLabel;
    lblAtingidoVendas: TLabel;
    lblMetaAtingidoVendas: TLabel;
    arcVendas2: TArc;
    arcCompras2: TArc;
    arcCompras3: TArc;
    circleCompras: TCircle;
    lblCP: TLabel;
    circleDashBoard: TCircle;
    lblDB: TLabel;
    circleRH: TCircle;
    lblRH: TLabel;
    circleSuporte: TCircle;
    lblSP: TLabel;
    circleVD: TCircle;
    lblVD: TLabel;
    clCompras: TColorAnimation;
    clDashBoard: TColorAnimation;
    clRH: TColorAnimation;
    clSuporte: TColorAnimation;
    clVendas: TColorAnimation;
    procedure FormCreate(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure navDashboardClick(Sender: TObject);
    procedure navComprasClick(Sender: TObject);
    procedure navSuporteClick(Sender: TObject);
    procedure navVendasClick(Sender: TObject);
    procedure navRHClick(Sender: TObject);
    procedure btnConsultarFuncionarioClick(Sender: TObject);
    procedure btnNovoFuncionarioClick(Sender: TObject);
    procedure btnSalvarFuncionarioClick(Sender: TObject);
    procedure lstFuncionariosChange(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
  private
    FObservabilityHeartbeat: TObservabilityHeartbeat;
    FEmployeeID: Integer;
    FPortariaApi: TPortariaApiServer;
    FIdentificacaoFacialStatusTimer: TTimer;
    FIdentificacaoFacialRequestID: string;
    FIdentificacaoFacialEmployeeID: Integer;
    FIdentificacaoFacialEmployeeName: string;

    FExclusaoStatusTimer: TTimer;
    FExclusaoRequestID: string;
    FExclusaoEmployeeID: Integer;
    FExclusaoEmployeeName: string;
    FExclusaoSomenteRemocaoFacial: Boolean;

    //Banco de dados ERP Delphi
    fdConnEmployees: TFDConnection;
    fdSQLiteDriverLink: TFDPhysSQLiteDriverLink;
    qryFuncionarios: TFDQuery;
    qryFuncionario: TFDQuery;

    procedure CarregarLogo;
    procedure AjustarLayoutResponsivo;
    procedure ConfigurarMenuLateral;
    procedure PosicionarItemMenu(const AItem: TRectangle; const ALabel: TLabel;
      const ATop: Single; const ATextoCompleto, ATextoCurto: string;
      const ACompacto: Boolean);
    procedure AtualizarInfoRede;
    procedure SelecionarSetor(const ASetor: TSetorCRM);
    procedure AtualizarMenu(const ASetor: TSetorCRM);
    procedure AtualizarCabecalho(const ASetor: TSetorCRM);
    procedure ConfigurarBanco;
    procedure CriarControlesAcaoFuncionario;
    procedure CarregarFuncionarios;
    procedure CarregarFuncionarioSelecionado;
    procedure LimparFormularioFuncionario;
    procedure AtualizarEstadoFormularioFuncionario;
    procedure ExcluirFuncionarioSelecionado;
    procedure ExecutarExclusaoFuncionarioLocal(const AEmployeeID: Integer);
    procedure btnExcluirFuncionarioClick(Sender: TObject);
    procedure btnCadastrarIdentificacaoFacialClick(Sender: TObject);
    procedure btnRemoverIdentificacaoFacialClick(Sender: TObject);
    procedure btnTestarRegraAcessoClick(Sender: TObject);
    function SolicitarIdentificacaoFacial(const AEmployeeID: Integer;
      const AEmployeeName: string; out ARequestID, AErro: string): Boolean;
    function ConsultarStatusIdentificacaoFacial(const ARequestID: string;
      out AStatus, AMensagem: string; out ASamples: Integer): Boolean;
    procedure IdentificacaoFacialStatusTimer(Sender: TObject);
    procedure FinalizarAcompanhamentoIdentificacaoFacial;

    function SolicitarRemocaoIdentificacaoFacial(const AEmployeeID: Integer;
      const AEmployeeName: string; out ARequestID, AErro: string): Boolean;
    function ConsultarStatusRemocaoIdentificacaoFacial(
      const ARequestID: string; out AStatus, AMensagem: string): Boolean;
    procedure ExclusaoStatusTimer(Sender: TObject);
    procedure FinalizarAcompanhamentoExclusao;

    function EnviarEventoObservabilidade(const ALevel, AType, AMessage,
      AEmployeeName, AResult: string; const ADurationMs: Double = 0): Boolean;

    procedure PortariaAccessEvent(const AJson: string);
    function APIBaseURL: string;
    function EnviarPushNotificacao(const AEmployeeID: Integer; const ATitulo,
      AMensagem: string; out AResposta: string): Boolean;
    function LocalizarLogo: string;
    function CaminhoBanco: string;
  end;

var
  Form2: TForm2;

implementation

{$R *.fmx}

procedure TForm2.AjustarLayoutResponsivo;
const
  SCREEN_MARGIN = 16.0;
  CARD_GAP = 16.0;
var
  SidebarWidth: Single;
  HeaderHeight: Single;
  ContentWidth: Single;
  ContentHeight: Single;
  Compacto: Boolean;

  function CardDaAba(const ATab: TTabItem; const AIndex: Integer): TRectangle;
  var
    I: Integer;
    Count: Integer;
    Child: TFmxObject;
  begin
    Result := nil;
    Count := 0;
    for I := 0 to ATab.ChildrenCount - 1 do
    begin
      Child := ATab.Children[I];
      if Child is TRectangle then
      begin
        if Count = AIndex then
          Exit(TRectangle(Child));
        Inc(Count);
      end;
    end;

  end;

procedure AjustarFilhosCard(const AParent: TFmxObject; const AWidth: Single);
  var
    I: Integer;
    Child: TFmxObject;
    Controle: TControl;
    LarguraUtil: Single;
  begin
    for I := 0 to AParent.ChildrenCount - 1 do
    begin
      Child := AParent.Children[I];
      if Child is TControl then
      begin
        Controle := TControl(Child);
        if not ((Controle is TCircle) or (Controle is TArc)) then
        begin
          LarguraUtil := AWidth - Controle.Position.X - 20;
          if LarguraUtil > 40 then
            Controle.Width := LarguraUtil;
          if Controle is TLabel then
            TLabel(Controle).WordWrap := True;
        end;
        if Controle is TRectangle then
          AjustarFilhosCard(Controle, Controle.Width);
      end;
    end;
  end;

  procedure PosicionarCard(const ACard: TRectangle; const AX, AY, AWidth,
    AHeight: Single);
  begin
    if ACard = nil then
      Exit;

    ACard.Position.X := AX;
    ACard.Position.Y := AY;
    ACard.Width := Max(120.0, AWidth);
    ACard.Height := Max(80.0, AHeight);
    AjustarFilhosCard(ACard, ACard.Width);
  end;

  procedure AjustarDashboard;
  var
    AreaWidth: Single;
    TopRowHeight: Single;
    KpiWidth: Single;
    SummaryWidth: Single;
    MetaWidth: Single;
    BottomTop: Single;
  begin
    AreaWidth := Max(260.0, ContentWidth - (SCREEN_MARGIN * 2));

    if AreaWidth < 760 then
    begin
      PosicionarCard(CardDaAba(tabDashboard, 0), SCREEN_MARGIN, SCREEN_MARGIN,
        AreaWidth, 120);
      PosicionarCard(CardDaAba(tabDashboard, 1), SCREEN_MARGIN, 152,
        AreaWidth, 150);
      PosicionarCard(CardDaAba(tabDashboard, 2), SCREEN_MARGIN, 318,
        AreaWidth, 190);
      PosicionarCard(CardDaAba(tabDashboard, 3), SCREEN_MARGIN, 524,
        AreaWidth, 150);
      Exit;
    end;

    TopRowHeight := Min(292.0, Max(230.0, ContentHeight - 190.0));
    KpiWidth := Min(220.0, Max(180.0, AreaWidth * 0.22));
    SummaryWidth := Min(420.0, Max(300.0, AreaWidth * 0.38));
    MetaWidth := AreaWidth - KpiWidth - SummaryWidth - (CARD_GAP * 2);

    PosicionarCard(CardDaAba(tabDashboard, 0), SCREEN_MARGIN, SCREEN_MARGIN,
      KpiWidth, 132);
    PosicionarCard(CardDaAba(tabDashboard, 1), SCREEN_MARGIN + KpiWidth + CARD_GAP,
      SCREEN_MARGIN, SummaryWidth, TopRowHeight);
    PosicionarCard(CardDaAba(tabDashboard, 2), SCREEN_MARGIN + KpiWidth + SummaryWidth +
      (CARD_GAP * 2), SCREEN_MARGIN, MetaWidth, TopRowHeight);

    BottomTop := SCREEN_MARGIN + TopRowHeight + CARD_GAP;
    PosicionarCard(CardDaAba(tabDashboard, 3), SCREEN_MARGIN, BottomTop,
      AreaWidth, Max(120.0, ContentHeight - BottomTop - SCREEN_MARGIN));
  end;

  procedure AjustarAbaTexto(const ATab: TTabItem);
  var
    AreaWidth: Single;
    HeroHeight: Single;
    CardTop: Single;
  begin
    AreaWidth := Max(260.0, ContentWidth - (SCREEN_MARGIN * 2));
    HeroHeight := Min(160.0, Max(124.0, ContentHeight * 0.32));
    CardTop := SCREEN_MARGIN + HeroHeight + 20;

    PosicionarCard(CardDaAba(ATab, 0), SCREEN_MARGIN, SCREEN_MARGIN,
      AreaWidth, HeroHeight);
    PosicionarCard(CardDaAba(ATab, 1), SCREEN_MARGIN, CardTop,
      Min(570.0, AreaWidth), Max(140.0, ContentHeight - CardTop - SCREEN_MARGIN));
  end;

  procedure AjustarRH;
  var
    AreaWidth: Single;
    LeftWidth: Single;
    RightWidth: Single;
    CardHeight: Single;
    CardTop: Single;
  begin
    AreaWidth := Max(260.0, ContentWidth - (SCREEN_MARGIN * 2));

    if AreaWidth < 860 then
    begin
      PosicionarCard(CardDaAba(tabRH, 0), SCREEN_MARGIN, SCREEN_MARGIN,
        AreaWidth, 456);
      PosicionarCard(CardDaAba(tabRH, 1), SCREEN_MARGIN, 488,
        AreaWidth, 481);
      Exit;
    end;

    CardTop := SCREEN_MARGIN;
    LeftWidth := Min(430.0, Max(360.0, AreaWidth * 0.42));
    RightWidth := AreaWidth - LeftWidth - CARD_GAP;
    CardHeight := Max(420.0, ContentHeight - (SCREEN_MARGIN * 2));

    PosicionarCard(CardDaAba(tabRH, 0), SCREEN_MARGIN, CardTop,
      LeftWidth, CardHeight);
    PosicionarCard(CardDaAba(tabRH, 1), SCREEN_MARGIN + LeftWidth + CARD_GAP,
      CardTop, RightWidth, CardHeight);
  end;

begin
  Compacto := Width < 860;

  if Compacto then
    SidebarWidth := 92
  else
    SidebarWidth := 254;

   HeaderHeight := Max(80.0, rctHeader.Position.Y + rctHeader.Height + SCREEN_MARGIN);

  rctSidebar.Align := TAlignLayout.None;
  tcSetores.Align := TAlignLayout.None;
  tcSetores.Scale.X := 1;
  tcSetores.Scale.Y := 1;

  rctSidebar.Position.X := 0;
  rctSidebar.Position.Y := 0;
  rctSidebar.Width := SidebarWidth;
  rctSidebar.Height := lytRoot.Height;

  ContentWidth := Max(260.0, lytRoot.Width - SidebarWidth - (SCREEN_MARGIN * 2));
  ContentHeight := Max(220.0, lytRoot.Height - HeaderHeight - SCREEN_MARGIN);
  tcSetores.Position.X := SidebarWidth + SCREEN_MARGIN;
  tcSetores.Position.Y := HeaderHeight;
  tcSetores.Width := ContentWidth;
  tcSetores.Height := ContentHeight;

  lblProduto.Visible := not Compacto;
  lblSubtitulo.Visible := not Compacto;
  imgLogo.Position.X := IfThen(Compacto, 17.0, 18.0);
  imgLogo.Position.Y := 20;
  imgLogo.Width := IfThen(Compacto, 56.0, 68.0);
  imgLogo.Height := imgLogo.Width;
  lblProduto.Position.X := 96;
  lblProduto.Position.Y := 22;
  lblProduto.Width := 130;
  lblProduto.Height := 60;
  lblSubtitulo.Position.X := 18;
  lblSubtitulo.Position.Y := 96;
  lblSubtitulo.Width := 214;

  PosicionarItemMenu(navDashboard, lblNavDashboard, IfThen(Compacto, 108.0, 158.0),
    'Dashboard', 'DS', Compacto);
  PosicionarItemMenu(navCompras, lblNavCompras, IfThen(Compacto, 172.0, 224.0),
    'Compras', 'CP', Compacto);
  PosicionarItemMenu(navSuporte, lblNavSuporte, IfThen(Compacto, 236.0, 290.0),
    'Suporte', 'SP', Compacto);
  PosicionarItemMenu(navVendas, lblNavVendas, IfThen(Compacto, 300.0, 356.0),
    'Vendas', 'VD', Compacto);
  PosicionarItemMenu(navRH, lblNavRH, IfThen(Compacto, 364.0, 422.0),
    'Dados RH', 'RH', Compacto);

  AjustarDashboard;
  AjustarAbaTexto(tabCompras);
  AjustarAbaTexto(tabSuporte);
  AjustarAbaTexto(tabVendas);
  AjustarRH;
end;

procedure TForm2.ConfigurarMenuLateral;
  procedure DesativarHitTestFilhos(const AParent: TFmxObject);
  var
    I: Integer;
    Child: TFmxObject;
  begin
    for I := 0 to AParent.ChildrenCount - 1 do
    begin
      Child := AParent.Children[I];
      if Child is TControl then
        TControl(Child).HitTest := False;
      DesativarHitTestFilhos(Child);
    end;
  end;

  procedure PrepararItem(const AItem: TRectangle);
  begin
    AItem.HitTest := True;
    DesativarHitTestFilhos(AItem);
  end;
begin
  PrepararItem(navDashboard);
  PrepararItem(navCompras);
  PrepararItem(navSuporte);
  PrepararItem(navVendas);
  PrepararItem(navRH);
end;

procedure TForm2.PosicionarItemMenu(const AItem: TRectangle; const ALabel: TLabel;
  const ATop: Single; const ATextoCompleto, ATextoCurto: string;
  const ACompacto: Boolean);
var
  I: Integer;
  Child: TFmxObject;
  Icone: TCircle;
begin
  AItem.Position.X := IfThen(ACompacto, 14.0, 18.0);
  AItem.Position.Y := ATop;
  AItem.Width := IfThen(ACompacto, 64.0, rctSidebar.Width - 36.0);
  AItem.Height := 56;
  AItem.XRadius := 16;
  AItem.YRadius := 16;

  if ACompacto then
  begin
    ALabel.Text := ATextoCurto;
    ALabel.Position.X := 0;
    ALabel.Width := AItem.Width;
    ALabel.TextSettings.HorzAlign := TTextAlign.Center;
  end
  else
  begin
    ALabel.Text := ATextoCompleto;
    ALabel.Position.X := 66;
    ALabel.Width := AItem.Width - 78;
    ALabel.TextSettings.HorzAlign := TTextAlign.Leading;
  end;
  ALabel.Position.Y := 17;
  ALabel.Height := 24;
  ALabel.HitTest := False;

  for I := 0 to AItem.ChildrenCount - 1 do
  begin
    Child := AItem.Children[I];
    if Child is TCircle then
    begin
      Icone := TCircle(Child);
      Icone.Visible := not ACompacto;
      Icone.HitTest := False;
      Icone.Position.X := 16;
      Icone.Position.Y := 10;
      Icone.Width := 36;
      Icone.Height := 36;
    end;
  end;
end;

procedure TForm2.AtualizarCabecalho(const ASetor: TSetorCRM);
begin
  case ASetor of
    scDashboard:
      begin
        lblTitulo.Text := 'Painel de gestão executiva do gestor';
        lblDescricao.Text := 'Visão consolidada para decisão gerencial, leitura de metas e acompanhamento direto dos setores-chave da empresa.';
      end;
    scCompras:
      begin
        lblTitulo.Text := 'Compras e fornecedores';
        lblDescricao.Text := 'Pedidos, reposições, negociações e aprovações organizados para decisão imediata.';
      end;
    scSuporte:
      begin
        lblTitulo.Text := 'Suporte e atendimento';
        lblDescricao.Text := 'Fila, SLA, backlog e produtividade técnica sob visão direta do gestor.';
      end;
    scVendas:
      begin
        lblTitulo.Text := 'Comercial e performance';
        lblDescricao.Text := 'Pipeline, receita prevista e equipe em destaque com leitura visual imediata.';
      end;
    scRH:
      begin
        lblTitulo.Text := 'Dados de funcionarios';
        lblDescricao.Text := 'Módulo RH integrado ao banco Employees.s3db para consultar ou cadastrar novos funcionaáios.';
      end;
  end;
end;

procedure TForm2.AtualizarInfoRede;
begin
  lblStatus.Text := 'Informações da rede atual para uso gerencial e integrações futuras';
  lblRedeTitulo.Text := 'Conexão ativa';
  lblRedeIPv4.Text := 'Interface: Wi-Fi  |  IPv4: Localhost';
  lblRedeDNS.Text := 'DNS: fe80::dac6:78ff:fe66:2f38, localHost';
  lblRedeGateway.Text := 'Gateway: localhost  |  Origem: Get-NetIPConfiguration';
end;

procedure TForm2.AtualizarMenu(const ASetor: TSetorCRM);
  procedure MarcarItem(const AItem: TRectangle; const ASelecionado: Boolean);
  begin
    if ASelecionado then
      AItem.Fill.Color := $FF1D4ED8
    else
      AItem.Fill.Color := $00111C2D;
  end;
begin
  MarcarItem(navDashboard, ASetor = scDashboard);
  MarcarItem(navCompras, ASetor = scCompras);
  MarcarItem(navSuporte, ASetor = scSuporte);
  MarcarItem(navVendas, ASetor = scVendas);
  MarcarItem(navRH, ASetor = scRH);
end;

procedure TForm2.btnConsultarFuncionarioClick(Sender: TObject);
begin
  CarregarFuncionarioSelecionado;
end;

procedure TForm2.btnExcluirFuncionarioClick(Sender: TObject);
begin
  ExcluirFuncionarioSelecionado;
end;

procedure TForm2.btnRemoverIdentificacaoFacialClick(Sender: TObject);
var
  RequestID: string;
  Erro: string;
  NomeFuncionario: string;
begin
  if FEmployeeID <= 0 then
    raise Exception.Create(
      'Selecione um funcionário antes de remover a identificação facial.');

  if FIdentificacaoFacialRequestID <> '' then
  begin
    MessageDlg(
      'Existe um cadastro de identificação facial em andamento.' +
      sLineBreak +
      'Aguarde a conclusão antes de solicitar a remoção.',
      TMsgDlgType.mtWarning,
      [TMsgDlgBtn.mbOK],
      0);
    Exit;
  end;

  if FExclusaoRequestID <> '' then
  begin
    MessageDlg(
      'Já existe uma solicitação de remoção/exclusão aguardando a Portaria.',
      TMsgDlgType.mtInformation,
      [TMsgDlgBtn.mbOK],
      0);
    Exit;
  end;

  NomeFuncionario := Trim(edtFuncionarioNome.Text);
  if NomeFuncionario = '' then
    raise Exception.Create('O funcionário selecionado não possui nome.');

  if MessageDlg(
      'Remover somente a identificação facial de:' + sLineBreak +
      sLineBreak + NomeFuncionario + sLineBreak +
      'EmployeeId: ' + FEmployeeID.ToString + '?' + sLineBreak + sLineBreak +
      'O funcionário permanecerá cadastrado no Employees.s3db.',
      TMsgDlgType.mtConfirmation,
      [TMsgDlgBtn.mbYes, TMsgDlgBtn.mbNo],
      0) <> mrYes then
    Exit;

  if Assigned(btnExcluirFuncionario) then
    btnExcluirFuncionario.Enabled := False;

  if Assigned(btnCadastrarIdentificacaoFacial) then
    btnCadastrarIdentificacaoFacial.Enabled := False;

  if Assigned(btnRemoverIdentificacaoFacial) then
    btnRemoverIdentificacaoFacial.Enabled := False;

  FExclusaoSomenteRemocaoFacial := False;

  lblDbStatus.Text :=
    'Solicitando remoção da identificação facial para ' +
    NomeFuncionario + '...';

  if not SolicitarRemocaoIdentificacaoFacial(
      FEmployeeID,
      NomeFuncionario,
      RequestID,
      Erro) then
  begin
    lblDbStatus.Text :=
      'Falha ao solicitar remoção da identificação facial: ' + Erro;

    AtualizarEstadoFormularioFuncionario;

    MessageDlg(
      'Não foi possível solicitar a remoção da identificação facial.' +
      sLineBreak + sLineBreak + Erro,
      TMsgDlgType.mtError,
      [TMsgDlgBtn.mbOK],
      0);
    Exit;
  end;

  FExclusaoSomenteRemocaoFacial := True;
  FExclusaoRequestID := RequestID;
  FExclusaoEmployeeID := FEmployeeID;
  FExclusaoEmployeeName := NomeFuncionario;

  if Assigned(FExclusaoStatusTimer) then
    FExclusaoStatusTimer.Enabled := True;

  lblDbStatus.Text :=
    'Remoção facial PENDENTE para ' + NomeFuncionario +
    ' | aguardando Portaria | ' + RequestID;
end;

procedure TForm2.btnCadastrarIdentificacaoFacialClick(Sender: TObject);
var
  RequestID: string;
  Erro: string;
  NomeFuncionario: string;
begin
  if FEmployeeID <= 0 then
    raise Exception.Create(
      'Selecione um funcionário antes de solicitar a identificação facial.');

  NomeFuncionario := Trim(edtFuncionarioNome.Text);
  if NomeFuncionario = '' then
    raise Exception.Create('O funcionário selecionado não possui nome.');

  if MessageDlg(
      'Solicitar cadastro de identificação facial para:' + sLineBreak +
      sLineBreak + NomeFuncionario + sLineBreak +
      'EmployeeId: ' + FEmployeeID.ToString + '?',
      TMsgDlgType.mtConfirmation,
      [TMsgDlgBtn.mbYes, TMsgDlgBtn.mbNo],
      0) <> mrYes then
    Exit;

  btnCadastrarIdentificacaoFacial.Enabled := False;
  if Assigned(btnRemoverIdentificacaoFacial) then
    btnRemoverIdentificacaoFacial.Enabled := False;
  try
    lblDbStatus.Text :=
      'Enviando solicitação de identificação facial para a Portaria...';

    if SolicitarIdentificacaoFacial(
        FEmployeeID,
        NomeFuncionario,
        RequestID,
        Erro) then
    begin
      FIdentificacaoFacialRequestID := RequestID;
      FIdentificacaoFacialEmployeeID := FEmployeeID;
      FIdentificacaoFacialEmployeeName := NomeFuncionario;

      if Assigned(FIdentificacaoFacialStatusTimer) then
        FIdentificacaoFacialStatusTimer.Enabled := True;

      lblDbStatus.Text :=
        'Identificação facial PENDENTE para ' + NomeFuncionario +
        ' | ' + RequestID;

      MessageDlg(
        'Solicitação enviada com sucesso.' + sLineBreak + sLineBreak +
        'Funcionário: ' + NomeFuncionario + sLineBreak +
        'EmployeeId: ' + FEmployeeID.ToString + sLineBreak +
        'RequestId: ' + RequestID + sLineBreak + sLineBreak +
        'A Portaria iniciará a captura automaticamente.',
        TMsgDlgType.mtInformation,
        [TMsgDlgBtn.mbOK],
        0);
    end
    else
    begin
      lblDbStatus.Text := 'Falha ao solicitar identificação facial: ' + Erro;

      MessageDlg(
        'Não foi possível solicitar a identificação facial.' +
        sLineBreak + sLineBreak + Erro,
        TMsgDlgType.mtError,
        [TMsgDlgBtn.mbOK],
        0);
    end;
  finally
    btnCadastrarIdentificacaoFacial.Enabled :=
      (FEmployeeID > 0) and
      (FIdentificacaoFacialRequestID = '') and
      (FExclusaoRequestID = '');

    if Assigned(btnRemoverIdentificacaoFacial) then
      btnRemoverIdentificacaoFacial.Enabled :=
        (FEmployeeID > 0) and
        (FIdentificacaoFacialRequestID = '') and
        (FExclusaoRequestID = '');
  end;
end;

procedure TForm2.btnTestarRegraAcessoClick(Sender: TObject);
var
  Authorized: Boolean;
  Reason: string;
  EmployeeName: string;
  WorkStart: string;
  WorkEnd: string;
  StatusText: string;
  ReasonText: string;
begin
  if FEmployeeID <= 0 then
  begin
    MessageDlg(
      'Selecione um funcionário antes de testar a regra de acesso.',
      TMsgDlgType.mtInformation,
      [TMsgDlgBtn.mbOK],
      0);
    Exit;
  end;

  if not Assigned(FPortariaApi) then
  begin
    MessageDlg(
      'A regra de acesso do ERP ainda não está disponível.',
      TMsgDlgType.mtError,
      [TMsgDlgBtn.mbOK],
      0);
    Exit;
  end;

  if not FPortariaApi.EvaluateEmployeeAccess(
      FEmployeeID,
      Now,
      Authorized,
      Reason,
      EmployeeName,
      WorkStart,
      WorkEnd) then
  begin
    MessageDlg(
      'Não foi possível avaliar a regra de acesso.' + sLineBreak +
      Reason,
      TMsgDlgType.mtError,
      [TMsgDlgBtn.mbOK],
      0);
    Exit;
  end;

  if Authorized then
    StatusText := 'LIBERADO'
  else
    StatusText := 'BLOQUEADO';

  if SameText(Reason, 'ACCESS_GRANTED') then
    ReasonText := 'Dentro do horário e sem bloqueio administrativo'
  else if SameText(Reason, 'EMPLOYEE_BLOCKED') then
    ReasonText := 'Bloqueio administrativo definido pelo ERP'
  else if SameText(Reason, 'OUTSIDE_WORK_HOURS') then
    ReasonText := 'Fora do horário de expediente configurado no ERP'
  else if SameText(Reason, 'INVALID_WORK_SCHEDULE') then
    ReasonText := 'Horário de expediente inválido'
  else if SameText(Reason, 'EMPLOYEE_NOT_FOUND') then
    ReasonText := 'Funcionário não encontrado'
  else
    ReasonText := Reason;

  lblDbStatus.Text :=
    'Regra ERP: ' + StatusText + ' | ' + EmployeeName + ' | ' + Reason;

  MessageDlg(
    'DECISAO DO ERP: ' + StatusText + sLineBreak + sLineBreak +
    'Funcionário: ' + EmployeeName + sLineBreak +
    'EmployeeId: ' + FEmployeeID.ToString + sLineBreak +
    'Expediente: ' + WorkStart + ' - ' + WorkEnd + sLineBreak +
    'Horário avaliado: ' + FormatDateTime('hh:nn:ss', Now) + sLineBreak +
    'Motivo: ' + Reason + sLineBreak +
    ReasonText + sLineBreak + sLineBreak +
    'Etapa 2: esta decisão foi tomada somente pelo ERP Delphi.' + sLineBreak +
    'A Portaria ainda não esta consultando esta regra.',
    TMsgDlgType.mtInformation,
    [TMsgDlgBtn.mbOK],
    0);
end;

function TForm2.SolicitarIdentificacaoFacial(
  const AEmployeeID: Integer;
  const AEmployeeName: string;
  out ARequestID, AErro: string): Boolean;
const
  OBSERVABILITY_URL =
    'http://127.0.0.1:3101/api/enrollment/request';
var
  Client: TNetHTTPClient;
  Payload: TJSONObject;
  Source: TStringStream;
  Response: IHTTPResponse;
  ResponseText: string;
  JsonValue: TJSONValue;
  JsonObject: TJSONObject;
begin
  Result := False;
  ARequestID := '';
  AErro := '';

  Client := TNetHTTPClient.Create(nil);
  Payload := TJSONObject.Create;
  try
    Payload.AddPair(
      'employeeId',
      TJSONNumber.Create(AEmployeeID));
    Payload.AddPair(
      'employeeName',
      AEmployeeName);
    Payload.AddPair(
      'terminal',
      'PORTARIA-01');

    Source := TStringStream.Create(
      Payload.ToJSON,
      TEncoding.UTF8);
    try
      Client.ContentType := 'application/json';
      Client.Accept := 'application/json';

      Response := Client.Post(
        OBSERVABILITY_URL,
        Source);

      ResponseText :=
        Response.ContentAsString(TEncoding.UTF8);

      if not (Response.StatusCode in [200, 201]) then
      begin
        AErro := Format(
          'HTTP %d - %s',
          [Response.StatusCode, Response.StatusText]);

        if Trim(ResponseText) <> '' then
          AErro := AErro + ' | ' + ResponseText;

        Exit;
      end;

      JsonValue :=
        TJSONObject.ParseJSONValue(ResponseText);
      try
        if not (JsonValue is TJSONObject) then
        begin
          AErro := 'Resposta JSON invalida da API do ERP.';
          Exit;
        end;

        JsonObject := TJSONObject(JsonValue);

        if not JsonObject.TryGetValue<string>(
            'requestId',
            ARequestID) then
        begin
          AErro := 'API do ERP não retornou requestId.';
          Exit;
        end;

        Result := Trim(ARequestID) <> '';

        if not Result then
          AErro := 'API do ERP retornou requestId vazio.';
      finally
        JsonValue.Free;
      end;
    finally
      Source.Free;
    end;
  except
    on E: Exception do
    begin
      AErro := E.Message;
      Result := False;
    end;
  end;

  Payload.Free;
  Client.Free;
end;

function TForm2.ConsultarStatusIdentificacaoFacial(
  const ARequestID: string;
  out AStatus, AMensagem: string;
  out ASamples: Integer): Boolean;
const
  STATUS_URL =
    'http://127.0.0.1:3101/api/biometric/status';
var
  Client: TNetHTTPClient;
  Response: IHTTPResponse;
  ResponseText: string;
  JsonValue: TJSONValue;
  Root: TJSONObject;
  EnrollmentValue: TJSONValue;
  Enrollment: TJSONObject;
  RequestIDRecebido: string;
  SamplesValue: TJSONValue;
begin
  Result := False;
  AStatus := '';
  AMensagem := '';
  ASamples := 0;

  if Trim(ARequestID) = '' then
    Exit;

  Client := TNetHTTPClient.Create(nil);
  try
    Client.Accept := 'application/json';

    Response := Client.Get(STATUS_URL);
    if Response.StatusCode <> 200 then
      Exit;

    ResponseText :=
      Response.ContentAsString(TEncoding.UTF8);

    JsonValue :=
      TJSONObject.ParseJSONValue(ResponseText);
    try
      if not (JsonValue is TJSONObject) then
        Exit;

      Root := TJSONObject(JsonValue);
      EnrollmentValue := Root.GetValue('enrollment');

      if not (EnrollmentValue is TJSONObject) then
        Exit;

      Enrollment := TJSONObject(EnrollmentValue);

      if not Enrollment.TryGetValue<string>(
          'requestId',
          RequestIDRecebido) then
        Exit;

      // Nunca usar o status de outra solicitacao.
      if not SameText(
          Trim(RequestIDRecebido),
          Trim(ARequestID)) then
        Exit;

      Enrollment.TryGetValue<string>(
        'status',
        AStatus);

      Enrollment.TryGetValue<string>(
        'message',
        AMensagem);

      SamplesValue :=
        Enrollment.GetValue('samples');

      if Assigned(SamplesValue) then
        ASamples :=
          StrToIntDef(SamplesValue.Value, 0);

      Result := True;
    finally
      JsonValue.Free;
    end;
  except
    // Se a API do ERP estiver temporariamente indisponivel,
    // mantemos o acompanhamento ativo para a proxima tentativa.
    Result := False;
  end;

  Client.Free;
end;

procedure TForm2.FinalizarAcompanhamentoIdentificacaoFacial;
begin
  if Assigned(FIdentificacaoFacialStatusTimer) then
    FIdentificacaoFacialStatusTimer.Enabled := False;

  FIdentificacaoFacialRequestID := '';
  FIdentificacaoFacialEmployeeID := 0;
  FIdentificacaoFacialEmployeeName := '';

  AtualizarEstadoFormularioFuncionario;
end;



procedure TForm2.IdentificacaoFacialStatusTimer(
  Sender: TObject);
var
  Status: string;
  Mensagem: string;
  Samples: Integer;
  RequestID: string;
  EmployeeName: string;
  DetalheErro: string;
begin
  if Trim(FIdentificacaoFacialRequestID) = '' then
  begin
    FinalizarAcompanhamentoIdentificacaoFacial;
    Exit;
  end;

  RequestID := FIdentificacaoFacialRequestID;
  EmployeeName := FIdentificacaoFacialEmployeeName;

  if not ConsultarStatusIdentificacaoFacial(
      RequestID,
      Status,
      Mensagem,
      Samples) then
    Exit;

  if SameText(Status, 'completed') then
  begin
    lblDbStatus.Text :=
      'Identificação facial CONCLUÍDA para ' +
      EmployeeName +
      ' | ' + Samples.ToString + '/8 amostras';

    FinalizarAcompanhamentoIdentificacaoFacial;

    MessageDlg(
      'Identificação facial concluída com sucesso.' +
      sLineBreak + sLineBreak +
      'Funcionario: ' + EmployeeName + sLineBreak +
      'RequestId: ' + RequestID + sLineBreak +
      'Amostras: ' + Samples.ToString + '/8',
      TMsgDlgType.mtInformation,
      [TMsgDlgBtn.mbOK],
      0);

    Exit;
  end;

  if SameText(Status, 'failed') then
  begin
    DetalheErro := '';
    if Trim(Mensagem) <> '' then
      DetalheErro := ' | ' + Mensagem;

    lblDbStatus.Text :=
      'Identificação facial FALHOU para ' +
      EmployeeName +
      DetalheErro;

    DetalheErro := '';
    if Trim(Mensagem) <> '' then
      DetalheErro := sLineBreak + 'Motivo: ' + Mensagem;

    FinalizarAcompanhamentoIdentificacaoFacial;

    MessageDlg(
      'Falha no cadastro de identificação facial.' +
      sLineBreak + sLineBreak +
      'Funcionário: ' + EmployeeName + sLineBreak +
      'RequestId: ' + RequestID +
      DetalheErro,
      TMsgDlgType.mtError,
      [TMsgDlgBtn.mbOK],
      0);

    Exit;
  end;

  if SameText(Status, 'in_progress') then
  begin
    lblDbStatus.Text :=
      'Identificação facial EM ANDAMENTO para ' +
      EmployeeName +
      ' | ' + Samples.ToString + '/8 amostras' +
      ' | Portaria capturando...';
    Exit;
  end;

  // pending = API do ERP recebeu a solicitacao, mas a Portaria ainda
  // nao confirmou que assumiu o RequestId.
  lblDbStatus.Text :=
    'Identificação facial PENDENTE para ' +
    EmployeeName +
    ' | aguardando Portaria iniciar...';
end;

procedure TForm2.btnNovoFuncionarioClick(Sender: TObject);
begin
  LimparFormularioFuncionario;
  edtFuncionarioNome.SetFocus;

end;

procedure TForm2.btnSalvarFuncionarioClick(Sender: TObject);
var
  IsUpdate: Boolean;
  EmployeeID: Integer;
  EmployeeName: string;
  Department: string;
  EmployeeFaceKey: string;
  WorkStart: string;
  WorkEnd: string;
  AccessBlocked: Integer;
  ParsedTime: TDateTime;
  NewGuid: TGUID;

begin
  if not fdConnEmployees.Connected then
    Exit;

  if Trim(edtFuncionarioNome.Text) = '' then
    raise Exception.Create('Informe o nome do funcionário.');

  if Trim(edtFuncionarioSetor.Text) = '' then
    raise Exception.Create('Informe o setor do funcionário.');

  IsUpdate := FEmployeeID > 0;
  EmployeeID := FEmployeeID;
  EmployeeName := Trim(edtFuncionarioNome.Text);
  Department := Trim(edtFuncionarioSetor.Text);
  WorkStart := Trim(edtFuncionarioEntrada.Text);
  WorkEnd := Trim(edtFuncionarioSaida.Text);
  AccessBlocked := Ord(chkFuncionarioBloqueado.IsChecked);
  EmployeeFaceKey := '';

  if (WorkStart = '') or (not TryStrToTime(WorkStart, ParsedTime)) then
    raise Exception.Create('Informe um horário de entrada válido (HH:mm).');

  if (WorkEnd = '') or (not TryStrToTime(WorkEnd, ParsedTime)) then
    raise Exception.Create('Informe um horário de saida válido (HH:mm).');

  if not IsUpdate then
  begin
    if CreateGUID(NewGuid) <> 0 then
      raise Exception.Create(
        'Não foi possível gerar a chave de identidade facial do funcionário.');

    EmployeeFaceKey := GUIDToString(NewGuid);
  end;

  try
    qryFuncionario.Close;

    if IsUpdate then
    begin
      qryFuncionario.SQL.Text :=
       'update Employee set Name = :Name, Phone = :Phone, [E-mail] = :Email, ' +
        'Department = :Department, Seniority = :Seniority, ' +
        'AccessBlocked = :AccessBlocked, WorkStart = :WorkStart, WorkEnd = :WorkEnd ' +
        'where ID = :ID';
      qryFuncionario.ParamByName('ID').AsInteger := EmployeeID;
    end
    else
    begin
      qryFuncionario.SQL.Text :=
        'insert into Employee ' +
        //'(Name, Phone, [E-mail], Department, Seniority, FaceKey, ' +
        '(Name, [E-mail], Department, Seniority, FaceKey, ' +
        'AccessBlocked, WorkStart, WorkEnd) ' +
        //'values (:Name, :Phone, :Email, :Department, :Seniority, :FaceKey, ' +
        'values (:Name, :Email, :Department, :Seniority, :FaceKey, ' +
        ':AccessBlocked, :WorkStart, :WorkEnd)';
      qryFuncionario.ParamByName('FaceKey').AsString := EmployeeFaceKey;
    end;

    qryFuncionario.ParamByName('Name').AsString := EmployeeName;
    //qryFuncionario.ParamByName('Phone').AsString :=
      Trim(edtFuncionarioCelular.Text);
    qryFuncionario.ParamByName('Email').AsString :=
      Trim(edtFuncionarioEmail.Text);
    qryFuncionario.ParamByName('Department').AsString := Department;
    qryFuncionario.ParamByName('AccessBlocked').AsInteger := AccessBlocked;
    qryFuncionario.ParamByName('WorkStart').AsString := WorkStart;
    qryFuncionario.ParamByName('WorkEnd').AsString := WorkEnd;

    if Trim(edtFuncionarioSeniority.Text) = '' then
      qryFuncionario.ParamByName('Seniority').Clear
    else
      qryFuncionario.ParamByName('Seniority').AsInteger :=
        StrToIntDef(Trim(edtFuncionarioSeniority.Text), 0);

    qryFuncionario.ExecSQL;

    // Para inclusao, obtem o ID real gerado pelo SQLite na mesma conexao.
    if not IsUpdate then
    begin
      qryFuncionario.Close;
      qryFuncionario.SQL.Text :=
        'select last_insert_rowid() as ID';
      qryFuncionario.Open;
      EmployeeID :=
        qryFuncionario.FieldByName('ID').AsInteger;
      qryFuncionario.Close;
    end;

    if IsUpdate then
    begin
      lblDbStatus.Text :=
        'Funcionário atualizado no Employees.s3db';

      EnviarEventoObservabilidade(
        'SUCCESS',
        'EMPLOYEE_UPDATED',
        'Funcionario atualizado no ERP | EmployeeId: ' +
          EmployeeID.ToString +
          ' | Department: ' + Department,
        EmployeeName,
        'UPDATED');
    end
    else
    begin
      lblDbStatus.Text :=
        'Funcionário cadastrado no Employees.s3db';

      EnviarEventoObservabilidade(
        'SUCCESS',
        'EMPLOYEE_CREATED',
        'Funcionario criado no ERP | EmployeeId: ' +
          EmployeeID.ToString +
          ' | Department: ' + Department,
        EmployeeName,
        'CREATED');
    end;

    CarregarFuncionarios;
    LimparFormularioFuncionario;
  except
    on E: Exception do
    begin
      lblDbStatus.Text := 'Erro ao salvar: ' + E.Message;

      EnviarEventoObservabilidade(
        'ERROR',
        'EMPLOYEE_SAVE_FAILED',
        'Falha ao salvar funcionario no ERP | EmployeeId: ' +
          EmployeeID.ToString + ' | ' + E.Message,
        EmployeeName,
        'FAILED');
    end;
  end;
end;

function TForm2.APIBaseURL: string;
begin
  Result := 'http://192.168.15.47:3000/api';
end;

function TForm2.CaminhoBanco: string;
begin
  Result := 'C:\Users\Public\Documents\Embarcadero\Studio\37.0\Samples\Data\Employees.s3db';
end;

procedure TForm2.CarregarFuncionarioSelecionado;
begin
  if (not fdConnEmployees.Connected) or (lstFuncionarios.Selected = nil) then
    Exit;

  try
    qryFuncionario.Close;
    qryFuncionario.SQL.Text :=
      //'select ID, Name, Phone, [E-mail], Department, Seniority, AccessBlocked, WorkStart, WorkEnd from Employee where ID = :ID';
      'select ID, Name, [E-mail], Department, Seniority, AccessBlocked, WorkStart, WorkEnd from Employee where ID = :ID';
    qryFuncionario.ParamByName('ID').AsInteger := Integer(lstFuncionarios.Selected.Tag);
    qryFuncionario.Open;
    if not qryFuncionario.IsEmpty then
    begin
      FEmployeeID := qryFuncionario.FieldByName('ID').AsInteger;
      edtFuncionarioNome.Text := qryFuncionario.FieldByName('Name').AsString;
      //edtFuncionarioCelular.Text := qryFuncionario.FieldByName('Phone').AsString;
      edtFuncionarioEmail.Text := qryFuncionario.FieldByName('E-mail').AsString;
      edtFuncionarioSetor.Text := qryFuncionario.FieldByName('Department').AsString;
      edtFuncionarioSeniority.Text := qryFuncionario.FieldByName('Seniority').AsString;
      edtFuncionarioEntrada.Text := qryFuncionario.FieldByName('WorkStart').AsString;
      edtFuncionarioSaida.Text := qryFuncionario.FieldByName('WorkEnd').AsString;
      chkFuncionarioBloqueado.IsChecked := qryFuncionario.FieldByName('AccessBlocked').AsInteger <> 0;
      lblDbStatus.Text := 'Funcionário carregado para edição';
      AtualizarEstadoFormularioFuncionario;
    end;
  except
    on E: Exception do
      lblDbStatus.Text := 'Erro ao consultar: ' + E.Message;
  end;
end;

procedure TForm2.CarregarFuncionarios;
var
  Item: TListBoxItem;
begin
  lstFuncionarios.Clear;

  if not fdConnEmployees.Connected then
    Exit;

  try
    qryFuncionarios.Close;
    qryFuncionarios.SQL.Text := 'select ID, Name, Department from Employee order by Name';
    qryFuncionarios.Open;
    while not qryFuncionarios.Eof do
    begin
      Item := TListBoxItem.Create(lstFuncionarios);
      Item.Parent := lstFuncionarios;
      Item.Text := qryFuncionarios.FieldByName('Name').AsString + '  |  ' +
        qryFuncionarios.FieldByName('Department').AsString;
      Item.Tag := qryFuncionarios.FieldByName('ID').AsInteger;
      lstFuncionarios.AddObject(Item);
      qryFuncionarios.Next;
    end;
    lblDbStatus.Text := Format('Banco conectado: %d funcionario(s) carregado(s)', [lstFuncionarios.Count]);
  except
    on E: Exception do
      lblDbStatus.Text := 'Erro ao carregar funcionários: ' + E.Message;
  end;
end;

procedure TForm2.CarregarLogo;
var
  LogoPath: string;
begin
  LogoPath := LocalizarLogo;
  if LogoPath <> '' then
    imgLogo.Bitmap.LoadFromFile(LogoPath);
end;

procedure TForm2.ConfigurarBanco;

  procedure GarantirColunaEmployee(const AColumnName, ADefinition: string);
  var
    SchemaQuery: TFDQuery;
    Encontrou: Boolean;
  begin
    Encontrou := False;
    SchemaQuery := TFDQuery.Create(nil);
    try
      SchemaQuery.Connection := fdConnEmployees;
      SchemaQuery.SQL.Text := 'pragma table_info(Employee)';
      SchemaQuery.Open;
      while not SchemaQuery.Eof do
      begin
        if SameText(SchemaQuery.FieldByName('name').AsString, AColumnName) then
        begin
          Encontrou := True;
          Break;
        end;
        SchemaQuery.Next;
      end;
    finally
      SchemaQuery.Free;
    end;

    if not Encontrou then
      fdConnEmployees.ExecSQL(
        'alter table Employee add column ' + AColumnName + ' ' + ADefinition);
  end;

begin
  fdSQLiteDriverLink := TFDPhysSQLiteDriverLink.Create(Self);
  fdSQLiteDriverLink.VendorLib := 'C:\Program Files (x86)\Embarcadero\Studio\37.0\bin\sqlite3.dll';

  fdConnEmployees := TFDConnection.Create(Self);
  fdConnEmployees.LoginPrompt := False;
  fdConnEmployees.ResourceOptions.SilentMode := True;
  fdConnEmployees.UpdateOptions.LockWait := True;
  fdConnEmployees.Params.Clear;
  fdConnEmployees.Params.Add('DriverID=SQLite');
  fdConnEmployees.Params.Add('Database=' + CaminhoBanco);
  fdConnEmployees.Params.Add('OpenMode=ReadWriteCreate');
  fdConnEmployees.Params.Add('LockingMode=Normal');
  fdConnEmployees.Params.Add('Synchronous=Normal');

  qryFuncionarios := TFDQuery.Create(Self);
  qryFuncionarios.Connection := fdConnEmployees;

  qryFuncionario := TFDQuery.Create(Self);
  qryFuncionario.Connection := fdConnEmployees;

  try
    fdConnEmployees.Connected := True;

    // Etapa 1: o ERP passa a armazenar as regras corporativas de acesso.
    // Migracao aditiva: preserva todos os registros existentes.
    GarantirColunaEmployee('AccessBlocked', 'INTEGER NOT NULL DEFAULT 0');
    GarantirColunaEmployee('WorkStart', 'TEXT NOT NULL DEFAULT ''08:00''');
    GarantirColunaEmployee('WorkEnd', 'TEXT NOT NULL DEFAULT ''18:00''');

    lblDbStatus.Text := 'Banco Employees.s3db conectado | regras de acesso disponíveis';
  except
    on E: Exception do
    begin
      lblDbStatus.Text := 'Falha ao conectar no banco: ' + E.Message;
      if fdConnEmployees.Connected then
        fdConnEmployees.Connected := False;
    end;
  end;
end;

procedure TForm2.CriarControlesAcaoFuncionario;
begin
  // Estes botoes ficam no .fmx para ajuste manual pelo designer.
  btnExcluirFuncionario.OnClick := btnExcluirFuncionarioClick;
  btnCadastrarIdentificacaoFacial.OnClick :=
    btnCadastrarIdentificacaoFacialClick;
  btnRemoverIdentificacaoFacial.OnClick :=
    btnRemoverIdentificacaoFacialClick;
  btnTestarRegraAcesso.OnClick :=
    btnTestarRegraAcessoClick;

  // v0.6.4.1 - acompanha automaticamente o resultado da API do ERP.
  FIdentificacaoFacialStatusTimer := TTimer.Create(Self);
  FIdentificacaoFacialStatusTimer.Enabled := False;
  FIdentificacaoFacialStatusTimer.Interval := 1000;
  FIdentificacaoFacialStatusTimer.OnTimer :=
    IdentificacaoFacialStatusTimer;

  FIdentificacaoFacialRequestID := '';
  FIdentificacaoFacialEmployeeID := 0;
  FIdentificacaoFacialEmployeeName := '';

  // v0.6.4.2 - acompanha a revogacao antes de excluir o funcionario.
  FExclusaoStatusTimer := TTimer.Create(Self);
  FExclusaoStatusTimer.Enabled := False;
  FExclusaoStatusTimer.Interval := 1000;
  FExclusaoStatusTimer.OnTimer := ExclusaoStatusTimer;

  FExclusaoRequestID := '';
  FExclusaoEmployeeID := 0;
  FExclusaoEmployeeName := '';
  FExclusaoSomenteRemocaoFacial := False;
end;

procedure TForm2.ExcluirFuncionarioSelecionado;
var
  EmployeeID: Integer;
  EmployeeName: string;
  RequestID: string;
  Erro: string;
begin
  if not fdConnEmployees.Connected then
    Exit;

  if FEmployeeID <= 0 then
    raise Exception.Create(
      'Selecione um funcionário antes de excluir.');

  if FIdentificacaoFacialRequestID <> '' then
  begin
    MessageDlg(
      'Existe um cadastro de identificação facial em andamento.' +
      sLineBreak +
      'Aguarde a conclusão antes de excluir o funcionário.',
      TMsgDlgType.mtWarning,
      [TMsgDlgBtn.mbOK],
      0);
    Exit;
  end;

  if FExclusaoRequestID <> '' then
  begin
    MessageDlg(
      'A exclusão deste funcionário já esta aguardando confirmação da Portaria.',
      TMsgDlgType.mtInformation,
      [TMsgDlgBtn.mbOK],
      0);
    Exit;
  end;

  EmployeeID := FEmployeeID;
  EmployeeName := Trim(edtFuncionarioNome.Text);

  if MessageDlg(
      'Excluir o funcionário selecionado?' + sLineBreak + sLineBreak +
      EmployeeName + sLineBreak +
      'EmployeeId: ' + EmployeeID.ToString + sLineBreak + sLineBreak +
      'Antes da exclusao, o ERP solicitará a remoção da identificação facial ' +
      'na Portaria. O registro só será excluído depois da confirmação.',
      TMsgDlgType.mtConfirmation,
      [TMsgDlgBtn.mbYes, TMsgDlgBtn.mbNo],
      0) <> mrYes then
    Exit;

  if Assigned(btnExcluirFuncionario) then
    btnExcluirFuncionario.Enabled := False;

  if Assigned(btnCadastrarIdentificacaoFacial) then
    btnCadastrarIdentificacaoFacial.Enabled := False;

  if Assigned(btnRemoverIdentificacaoFacial) then
    btnRemoverIdentificacaoFacial.Enabled := False;

  FExclusaoSomenteRemocaoFacial := False;

  EnviarEventoObservabilidade(
    'INFO',
    'EMPLOYEE_DELETE_REQUEST',
    'Exclusao solicitada no ERP | EmployeeId: ' +
      EmployeeID.ToString +
      ' | aguardando remoção da identificação facial',
    EmployeeName,
    'REQUESTED');

  lblDbStatus.Text :=
    'Solicitando remoção da identificação facial para ' +
    EmployeeName + '...';

  if not SolicitarRemocaoIdentificacaoFacial(
      EmployeeID,
      EmployeeName,
      RequestID,
      Erro) then
  begin
    lblDbStatus.Text :=
      'Exclusão cancelada: não foi possível solicitar a remoção facial. ' +
      Erro;

    EnviarEventoObservabilidade(
      'ERROR',
      'EMPLOYEE_DELETE_FAILED',
      'Exclusao cancelada antes do DELETE | EmployeeId: ' +
        EmployeeID.ToString +
        ' | falha ao solicitar remoção da identificação facial | ' + Erro,
      EmployeeName,
      'FAILED');

    AtualizarEstadoFormularioFuncionario;

    MessageDlg(
      'O funcionário NÃO foi excluído.' +
      sLineBreak + sLineBreak +
      'Não foi possível solicitar a remoção da identificação facial:' +
      sLineBreak + Erro,
      TMsgDlgType.mtError,
      [TMsgDlgBtn.mbOK],
      0);

    Exit;
  end;

  FExclusaoRequestID := RequestID;
  FExclusaoEmployeeID := EmployeeID;
  FExclusaoEmployeeName := EmployeeName;

  EnviarEventoObservabilidade(
    'INFO',
    'EMPLOYEE_DELETE_WAITING_FACE_REMOVAL',
    'Exclusão aguardando Portaria | EmployeeId: ' +
      EmployeeID.ToString +
      ' | RequestId: ' + RequestID,
    EmployeeName,
    'PENDING');

  if Assigned(FExclusaoStatusTimer) then
    FExclusaoStatusTimer.Enabled := True;

  lblDbStatus.Text :=
    'Exclusao PENDENTE para ' + EmployeeName +
    ' | aguardando remoção da identificação facial | ' +
    RequestID;
end;

procedure TForm2.ExecutarExclusaoFuncionarioLocal(
  const AEmployeeID: Integer);
begin
  if not fdConnEmployees.Connected then
    raise Exception.Create(
      'Banco Employees.s3db não está conectado.');

  qryFuncionario.Close;
  qryFuncionario.SQL.Text :=
    'delete from Employee where ID = :ID';
  qryFuncionario.ParamByName('ID').AsInteger :=
    AEmployeeID;
  qryFuncionario.ExecSQL;
end;

procedure TForm2.FormCreate(Sender: TObject);
begin
  Constraints.MinWidth := 760;
  Constraints.MinHeight := 560;

  AtualizarInfoRede;
  ConfigurarMenuLateral;
  CriarControlesAcaoFuncionario;
  AjustarLayoutResponsivo;
  CarregarLogo;
  SelecionarSetor(scDashboard);
  ConfigurarBanco;
  if fdConnEmployees.Connected then
    CarregarFuncionarios;

  // API do ERP Delphi para o terminal C++Builder.
  // Porta 3101 evita conflito com a API local do terminal.
  FPortariaApi := TPortariaApiServer.Create(Self, CaminhoBanco, 3101);
  FPortariaApi.OnAccessEvent := PortariaAccessEvent;
  try
    FPortariaApi.Start;
    lblStatus.Text := 'ERP API online em ' + FPortariaApi.BaseUrl;
  except
    on E: Exception do
      lblStatus.Text := 'Falha ao iniciar ERP API: ' + E.Message;
  end;

  LimparFormularioFuncionario;
  //mmSMS.Text := 'Ola, esta e uma mensagem push enviada pelo gestor no Delphi System.';

   FObservabilityHeartbeat := TObservabilityHeartbeat.Create;

   FObservabilityHeartbeat.ServerUrl :=
  'http://127.0.0.1:8080';

   FObservabilityHeartbeat.Source :=
  'ERP-DELPHI';

   FObservabilityHeartbeat.Version :=
  '1.0.0';

   FObservabilityHeartbeat.Start;

end;

procedure TForm2.FormDestroy(Sender: TObject);
begin
  if Assigned(FIdentificacaoFacialStatusTimer) then
    FIdentificacaoFacialStatusTimer.Enabled := False;

  if Assigned(FExclusaoStatusTimer) then
    FExclusaoStatusTimer.Enabled := False;

  FreeAndNil(FObservabilityHeartbeat);
end;

function TForm2.EnviarPushNotificacao(const AEmployeeID: Integer; const ATitulo,
  AMensagem: string; out AResposta: string): Boolean;
var
  Client: TNetHTTPClient;
  Source: TStringStream;
  Response: IHTTPResponse;
  Payload: TJSONObject;
begin
  Result := False;
  AResposta := 'Falha ao enviar notificação.';
  Client := TNetHTTPClient.Create(nil);
  Payload := TJSONObject.Create;
  try
    Payload.AddPair('employeeId', TJSONNumber.Create(AEmployeeID));
    Payload.AddPair('title', ATitulo);
    Payload.AddPair('message', AMensagem);
    Payload.AddPair('requestedBy', 'delphi-system');

    Source := TStringStream.Create(Payload.ToJSON, TEncoding.UTF8);
    try
      Client.ContentType := 'application/json';
      Client.Accept := 'application/json';
      Response := Client.Post(APIBaseURL + '/notifications/push', Source);
      Result := Response.StatusCode in [200, 201];
      if Result then
        AResposta := 'Push enviado com sucesso.'
      else
        AResposta := Format('Falha ao enviar push. HTTP %d - %s',
          [Response.StatusCode, Response.StatusText]);
    finally
      Source.Free;
    end;
  except
    on E: Exception do
      AResposta := 'Erro ao enviar push: ' + E.Message;
  end;
  Payload.Free;
  Client.Free;
end;


function TForm2.EnviarEventoObservabilidade(
  const ALevel, AType, AMessage, AEmployeeName, AResult: string;
  const ADurationMs: Double): Boolean;
const
  EVENTS_URL = 'http://127.0.0.1:8080/api/events';
var
  Client: TNetHTTPClient;
  Payload: TJSONObject;
  Source: TStringStream;
  Response: IHTTPResponse;
begin
  Result := False;

  Client := TNetHTTPClient.Create(nil);
  Payload := TJSONObject.Create;
  try
    // Observabilidade e uma camada adicional: uma falha aqui nunca
    // deve impedir o CRUD corporativo do ERP.
    Client.ConnectionTimeout := 1000;
    Client.ResponseTimeout := 1000;
    Client.ContentType := 'application/json';
    Client.Accept := 'application/json';

    Payload.AddPair('source', 'ERP-DELPHI');
    Payload.AddPair('level', ALevel);
    Payload.AddPair('type', AType);
    Payload.AddPair('message', AMessage);
    Payload.AddPair('employee', AEmployeeName);
    Payload.AddPair('result', AResult);
    Payload.AddPair('durationMs', TJSONNumber.Create(ADurationMs));

    Source := TStringStream.Create(
      Payload.ToJSON,
      TEncoding.UTF8);
    try
      Response := Client.Post(EVENTS_URL, Source);
      Result := Response.StatusCode in [200, 201];
    finally
      Source.Free;
    end;
  except
    // Falha de telemetria nao altera o resultado da operacao do ERP.
    Result := False;
  end;

  Payload.Free;
  Client.Free;
end;


function TForm2.SolicitarRemocaoIdentificacaoFacial(
  const AEmployeeID: Integer;
  const AEmployeeName: string;
  out ARequestID, AErro: string): Boolean;
const
  REVOKE_URL =
    'http://127.0.0.1:3101/api/biometric/revoke';
var
  Client: TNetHTTPClient;
  Payload: TJSONObject;
  Source: TStringStream;
  Response: IHTTPResponse;
  ResponseText: string;
  JsonValue: TJSONValue;
  JsonObject: TJSONObject;
begin
  Result := False;
  ARequestID := '';
  AErro := '';

  Client := TNetHTTPClient.Create(nil);
  Payload := TJSONObject.Create;
  try
    Payload.AddPair(
      'employeeId',
      TJSONNumber.Create(AEmployeeID));
    Payload.AddPair(
      'employeeName',
      AEmployeeName);
    Payload.AddPair(
      'terminal',
      'PORTARIA-01');

    Source := TStringStream.Create(
      Payload.ToJSON,
      TEncoding.UTF8);
    try
      Client.ContentType := 'application/json';
      Client.Accept := 'application/json';

      Response := Client.Post(
        REVOKE_URL,
        Source);

      ResponseText :=
        Response.ContentAsString(TEncoding.UTF8);

      if not (Response.StatusCode in [200, 201]) then
      begin
        AErro := Format(
          'HTTP %d - %s',
          [Response.StatusCode, Response.StatusText]);

        if Trim(ResponseText) <> '' then
          AErro := AErro + ' | ' + ResponseText;

        Exit;
      end;

      JsonValue :=
        TJSONObject.ParseJSONValue(ResponseText);
      try
        if not (JsonValue is TJSONObject) then
        begin
          AErro := 'Resposta JSON inválida da API do ERP.';
          Exit;
        end;

        JsonObject := TJSONObject(JsonValue);

        if not JsonObject.TryGetValue<string>(
            'requestId',
            ARequestID) then
        begin
          AErro := 'API do ERP não retornou requestId.';
          Exit;
        end;

        Result := Trim(ARequestID) <> '';

        if not Result then
          AErro := 'API do ERP retornou requestId vazio.';
      finally
        JsonValue.Free;
      end;
    finally
      Source.Free;
    end;
  except
    on E: Exception do
    begin
      AErro := E.Message;
      Result := False;
    end;
  end;

  Payload.Free;
  Client.Free;
end;

function TForm2.ConsultarStatusRemocaoIdentificacaoFacial(
  const ARequestID: string;
  out AStatus, AMensagem: string): Boolean;
const
  STATUS_URL =
    'http://127.0.0.1:3101/api/biometric/status';
var
  Client: TNetHTTPClient;
  Response: IHTTPResponse;
  ResponseText: string;
  JsonValue: TJSONValue;
  Root: TJSONObject;
  RevokeValue: TJSONValue;
  Revoke: TJSONObject;
  RequestIDRecebido: string;
begin
  Result := False;
  AStatus := '';
  AMensagem := '';

  if Trim(ARequestID) = '' then
    Exit;

  Client := TNetHTTPClient.Create(nil);
  try
    Client.Accept := 'application/json';

    Response := Client.Get(STATUS_URL);

    if Response.StatusCode <> 200 then
      Exit;

    ResponseText :=
      Response.ContentAsString(TEncoding.UTF8);

    JsonValue :=
      TJSONObject.ParseJSONValue(ResponseText);
    try
      if not (JsonValue is TJSONObject) then
        Exit;

      Root := TJSONObject(JsonValue);
      RevokeValue := Root.GetValue('revoke');

      if not (RevokeValue is TJSONObject) then
        Exit;

      Revoke := TJSONObject(RevokeValue);

      if not Revoke.TryGetValue<string>(
          'requestId',
          RequestIDRecebido) then
        Exit;

      // Nunca aceitar o resultado de outra exclusao/revogacao.
      if not SameText(
          Trim(RequestIDRecebido),
          Trim(ARequestID)) then
        Exit;

      Revoke.TryGetValue<string>(
        'status',
        AStatus);

      Revoke.TryGetValue<string>(
        'message',
        AMensagem);

      Result := True;
    finally
      JsonValue.Free;
    end;
  except
    // Mantem a exclusao pendente se a API do ERP oscilar.
    Result := False;
  end;

  Client.Free;
end;

procedure TForm2.FinalizarAcompanhamentoExclusao;
begin
  if Assigned(FExclusaoStatusTimer) then
    FExclusaoStatusTimer.Enabled := False;

  FExclusaoRequestID := '';
  FExclusaoEmployeeID := 0;
  FExclusaoEmployeeName := '';
  FExclusaoSomenteRemocaoFacial := False;

  AtualizarEstadoFormularioFuncionario;
end;

procedure TForm2.ExclusaoStatusTimer(Sender: TObject);
var
  Status: string;
  Mensagem: string;
  RequestID: string;
  EmployeeName: string;
  EmployeeID: Integer;
  SomenteRemocaoFacial: Boolean;
begin
  if Trim(FExclusaoRequestID) = '' then
  begin
    FinalizarAcompanhamentoExclusao;
    Exit;
  end;

  RequestID := FExclusaoRequestID;
  EmployeeID := FExclusaoEmployeeID;
  EmployeeName := FExclusaoEmployeeName;
  SomenteRemocaoFacial := FExclusaoSomenteRemocaoFacial;

  if not ConsultarStatusRemocaoIdentificacaoFacial(
      RequestID,
      Status,
      Mensagem) then
    Exit;

  if SameText(Status, 'completed') then
  begin
    if SomenteRemocaoFacial then
    begin
      FinalizarAcompanhamentoExclusao;

      lblDbStatus.Text :=
        'Identificacao facial removida | ' + EmployeeName +
        ' | cadastro preservado';

      MessageDlg(
        'Identificação facial removida com sucesso.' +
        sLineBreak + sLineBreak +
        'Funcionario: ' + EmployeeName + sLineBreak +
        'EmployeeId: ' + EmployeeID.ToString + sLineBreak +
        'RequestId: ' + RequestID + sLineBreak + sLineBreak +
        'O cadastro do funcionário foi preservado.',
        TMsgDlgType.mtInformation,
        [TMsgDlgBtn.mbOK],
        0);

      Exit;
    end;

    // Fluxo de exclusao: primeiro para o polling e depois apaga o registro.
    if Assigned(FExclusaoStatusTimer) then
      FExclusaoStatusTimer.Enabled := False;

    try
      ExecutarExclusaoFuncionarioLocal(EmployeeID);

      FExclusaoRequestID := '';
      FExclusaoEmployeeID := 0;
      FExclusaoEmployeeName := '';

      lblDbStatus.Text :=
        'Funcionário excluído com seguranca: identificação facial removida e ' +
        'registro apagado do Employees.s3db';

      EnviarEventoObservabilidade(
        'SUCCESS',
        'EMPLOYEE_DELETED',
        'Funcionário excluído do ERP após confirmação da Portaria | EmployeeId: ' +
          EmployeeID.ToString +
          ' | RequestId: ' + RequestID,
        EmployeeName,
        'DELETED');

      CarregarFuncionarios;
      LimparFormularioFuncionario;

      MessageDlg(
        'Funcionário excluído com sucesso.' +
        sLineBreak + sLineBreak +
        'Funcionário: ' + EmployeeName + sLineBreak +
        'EmployeeId: ' + EmployeeID.ToString + sLineBreak +
        'Identificação facial: removida pela Portaria' + sLineBreak +
        'Registro ERP: excluido',
        TMsgDlgType.mtInformation,
        [TMsgDlgBtn.mbOK],
        0);
    except
      on E: Exception do
      begin
        // A identificacao facial ja foi removida, mas o registro corporativo
        // continua no banco. Isso fica explicito para correcao operacional.
        FExclusaoRequestID := '';
        FExclusaoEmployeeID := 0;
        FExclusaoEmployeeName := '';

        lblDbStatus.Text :=
          'Remoção facial concluída, mas falhou a exclusão no ERP: ' +
          E.Message;

        EnviarEventoObservabilidade(
          'ERROR',
          'EMPLOYEE_DELETE_FAILED',
          'Portaria removeu a identificação facial, mas o DELETE no ERP falhou | ' +
            'EmployeeId: ' + EmployeeID.ToString +
            ' | RequestId: ' + RequestID +
            ' | ' + E.Message,
          EmployeeName,
          'FAILED');

        AtualizarEstadoFormularioFuncionario;

        MessageDlg(
          'A identificação facial foi removida, mas o registro do funcionário ' +
          'NÃO foi excluido do ERP.' + sLineBreak + sLineBreak +
          E.Message,
          TMsgDlgType.mtError,
          [TMsgDlgBtn.mbOK],
          0);
      end;
    end;

    Exit;
  end;

  if SameText(Status, 'failed') then
  begin
    if SomenteRemocaoFacial then
    begin
      FinalizarAcompanhamentoExclusao;

      lblDbStatus.Text :=
        'Falha ao remover identificação facial de ' + EmployeeName;

      if Trim(Mensagem) <> '' then
        lblDbStatus.Text := lblDbStatus.Text + ' | ' + Mensagem;

      MessageDlg(
        'Nao foi possível remover a identificação facial.' +
        sLineBreak + sLineBreak +
        'Funcionário: ' + EmployeeName + sLineBreak +
        Mensagem,
        TMsgDlgType.mtError,
        [TMsgDlgBtn.mbOK],
        0);

      Exit;
    end;

    FinalizarAcompanhamentoExclusao;

    lblDbStatus.Text :=
      'Exclusão CANCELADA para ' + EmployeeName +
      ': falha na remoção da identificação facial';

    EnviarEventoObservabilidade(
      'ERROR',
      'EMPLOYEE_DELETE_CANCELLED',
      'Exclusão cancelada pela falha de remoção facial | EmployeeId: ' +
        EmployeeID.ToString +
        ' | RequestId: ' + RequestID +
        ' | ' + Mensagem,
      EmployeeName,
      'CANCELLED');

    if Trim(Mensagem) <> '' then
      lblDbStatus.Text :=
        lblDbStatus.Text + ' | ' + Mensagem;

    MessageDlg(
      'O funcionario NÃO foi excluído.' +
      sLineBreak + sLineBreak +
      'A Portaria informou falha na remoção da identificação facial.' +
      sLineBreak +
      Mensagem,
      TMsgDlgType.mtError,
      [TMsgDlgBtn.mbOK],
      0);

    Exit;
  end;

  if SomenteRemocaoFacial then
    lblDbStatus.Text :=
      'Remoção facial PENDENTE para ' + EmployeeName +
      ' | aguardando confirmação da Portaria...'
  else
    lblDbStatus.Text :=
      'Exclusao PENDENTE para ' + EmployeeName +
      ' | aguardando confirmação da Portaria...';
end;

procedure TForm2.PortariaAccessEvent(const AJson: string);
var
  JsonValue: TJSONValue;
  Obj: TJSONObject;
  EmployeeName: string;
  TerminalName: string;
  AuthMethod: string;
  EventName: string;
begin
  EmployeeName := '';
  TerminalName := '';
  AuthMethod := '';
  EventName := '';

  JsonValue := TJSONObject.ParseJSONValue(AJson);
  try
    if JsonValue is TJSONObject then
    begin
      Obj := TJSONObject(JsonValue);
      Obj.TryGetValue<string>('employeeName', EmployeeName);
      Obj.TryGetValue<string>('terminal', TerminalName);
      Obj.TryGetValue<string>('method', AuthMethod);
      Obj.TryGetValue<string>('event', EventName);
    end;

    lblDbStatus.Text := 'Evento da portaria recebido';
    if EmployeeName <> '' then
      lblDbStatus.Text := lblDbStatus.Text + ': ' + EmployeeName;
    if TerminalName <> '' then
      lblDbStatus.Text := lblDbStatus.Text + ' | ' + TerminalName;
    if AuthMethod <> '' then
      lblDbStatus.Text := lblDbStatus.Text + ' | ' + AuthMethod;
    if EventName <> '' then
      lblDbStatus.Text := lblDbStatus.Text + ' | ' + EventName;
  finally
    JsonValue.Free;
  end;
end;

procedure TForm2.FormResize(Sender: TObject);
begin
  AjustarLayoutResponsivo;
end;

procedure TForm2.AtualizarEstadoFormularioFuncionario;
begin
  if FEmployeeID > 0 then
  begin
    btnSalvarFuncionario.Text := 'Salvar alterações';

    if Assigned(btnExcluirFuncionario) then
      btnExcluirFuncionario.Enabled :=
        (FExclusaoRequestID = '') and
        (FIdentificacaoFacialRequestID = '');

    if Assigned(btnCadastrarIdentificacaoFacial) then
      btnCadastrarIdentificacaoFacial.Enabled :=
        (FIdentificacaoFacialRequestID = '') and
        (FExclusaoRequestID = '');

    if Assigned(btnRemoverIdentificacaoFacial) then
      btnRemoverIdentificacaoFacial.Enabled :=
        (FIdentificacaoFacialRequestID = '') and
        (FExclusaoRequestID = '');

    if Assigned(btnTestarRegraAcesso) then
      btnTestarRegraAcesso.Enabled := True;
  end
  else
  begin
    btnSalvarFuncionario.Text := 'Salvar novo funcionário';

    if Assigned(btnExcluirFuncionario) then
      btnExcluirFuncionario.Enabled := False;

    if Assigned(btnCadastrarIdentificacaoFacial) then
      btnCadastrarIdentificacaoFacial.Enabled := False;

    if Assigned(btnRemoverIdentificacaoFacial) then
      btnRemoverIdentificacaoFacial.Enabled := False;

    if Assigned(btnTestarRegraAcesso) then
      btnTestarRegraAcesso.Enabled := False;
  end;
end;

procedure TForm2.LimparFormularioFuncionario;
begin
  FEmployeeID := 0;
  lstFuncionarios.ItemIndex := -1;
  edtFuncionarioNome.Text := '';
  edtFuncionarioSetor.Text := '';
  edtFuncionarioEmail.Text := '';
  edtFuncionarioCelular.Text := '';
  edtFuncionarioSeniority.Text := '';
  edtFuncionarioEntrada.Text := '08:00';
  edtFuncionarioSaida.Text := '18:00';
  chkFuncionarioBloqueado.IsChecked := False;
  //mmSMS.Text := 'Ola, esta e uma mensagem enviada pelo modulo RH do Delphi System.';
  AtualizarEstadoFormularioFuncionario;
  if Assigned(fdConnEmployees) and fdConnEmployees.Connected then
    lblDbStatus.Text := 'Pronto para cadastrar um novo funcionário';
end;

function TForm2.LocalizarLogo: string;
const
  LOGO_FILE = 'delphi_logo.png';
var
  Candidatos: array[0..4] of string;
  I: Integer;
begin
  Result := '';
  Candidatos[0] := IncludeTrailingPathDelimiter(ExtractFilePath(ParamStr(0))) + LOGO_FILE;
  Candidatos[1] := IncludeTrailingPathDelimiter(GetCurrentDir) + LOGO_FILE;
  Candidatos[2] := IncludeTrailingPathDelimiter(ExpandFileName(ExtractFilePath(ParamStr(0)) + '..\..\')) + LOGO_FILE;
  Candidatos[3] := IncludeTrailingPathDelimiter(ExpandFileName(ExtractFilePath(ParamStr(0)) + '..\..\..\')) + LOGO_FILE;
  Candidatos[4] := 'C:\Users\thiago.silva\Desktop\Conference2026\DelphiSystem\' + LOGO_FILE;

  for I := Low(Candidatos) to High(Candidatos) do
    if TFile.Exists(Candidatos[I]) then
      Exit(Candidatos[I]);
end;

procedure TForm2.lstFuncionariosChange(Sender: TObject);
begin
  CarregarFuncionarioSelecionado;
end;

procedure TForm2.navComprasClick(Sender: TObject);
begin
  SelecionarSetor(scCompras);
end;

procedure TForm2.navDashboardClick(Sender: TObject);
begin
  SelecionarSetor(scDashboard);
end;

procedure TForm2.navRHClick(Sender: TObject);
begin
  SelecionarSetor(scRH);
end;

procedure TForm2.navSuporteClick(Sender: TObject);
begin
  SelecionarSetor(scSuporte);
end;

procedure TForm2.navVendasClick(Sender: TObject);
begin
  SelecionarSetor(scVendas);
end;

procedure TForm2.SelecionarSetor(const ASetor: TSetorCRM);
begin
  tcSetores.TabIndex := Ord(ASetor);
  case ASetor of
    scDashboard: tcSetores.ActiveTab := tabDashboard;
    scCompras: tcSetores.ActiveTab := tabCompras;
    scSuporte: tcSetores.ActiveTab := tabSuporte;
    scVendas: tcSetores.ActiveTab := tabVendas;
    scRH: tcSetores.ActiveTab := tabRH;
  end;
  AtualizarMenu(ASetor);
  AtualizarCabecalho(ASetor);
  AjustarLayoutResponsivo;
end;

end.
