object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'Controle Empresarial de Esteiras - MCP'
  ClientHeight = 700
  ClientWidth = 1153
  Color = 2631720
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poScreenCenter
  OnCreate = FormCreate
  TextHeight = 15
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 1153
    Height = 108
    Align = alTop
    BevelOuter = bvNone
    Color = 1907997
    ParentBackground = False
    TabOrder = 0
    ExplicitWidth = 1147
    object Label1: TLabel
      Left = 32
      Top = 24
      Width = 336
      Height = 37
      Caption = 'MCP CONVEYOR CONTROL'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -27
      Font.Name = 'Segoe UI Semibold'
      Font.Style = []
      ParentFont = False
      Transparent = True
    end
    object Label4: TLabel
      Left = 35
      Top = 64
      Width = 376
      Height = 17
      Caption = 'MCP direto na aplica'#231#227'o | KaiChat via Bridge para MCP principal'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = 14208192
      Font.Height = -13
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
      Transparent = True
    end
    object pnlConexoes: TPanel
      Left = 782
      Top = 12
      Width = 363
      Height = 86
      BevelOuter = bvNone
      Color = 2631720
      ParentBackground = False
      TabOrder = 0
      object lblConexoesTitle: TLabel
        Left = 16
        Top = 8
        Width = 118
        Height = 15
        Caption = 'STATUS DE CONEXAO'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 16762424
        Font.Height = -12
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object imgMcpPrincipal: TImage
        Left = 16
        Top = 30
        Width = 22
        Height = 22
        Center = True
        Proportional = True
        Stretch = True
      end
      object lblMcpPrincipal: TLabel
        Left = 46
        Top = 33
        Width = 111
        Height = 13
        Caption = 'MCP Principal 3000: ...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 12513444
        Font.Height = -11
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object imgMcpBridge: TImage
        Left = 16
        Top = 58
        Width = 22
        Height = 22
        Center = True
        Proportional = True
        Stretch = True
      end
      object lblMcpBridge: TLabel
        Left = 46
        Top = 61
        Width = 100
        Height = 13
        Caption = 'MCP Bridge 3100: ...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 12513444
        Font.Height = -11
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 108
    Width = 1153
    Height = 592
    Margins.Left = 10
    Margins.Right = 10
    Align = alClient
    BevelOuter = bvNone
    Color = 2631720
    ParentBackground = False
    TabOrder = 1
    ExplicitWidth = 1147
    ExplicitHeight = 575
    object pnlIndicadores: TPanel
      Left = 24
      Top = 32
      Width = 198
      Height = 330
      BevelOuter = bvNone
      Color = 3487029
      ParentBackground = False
      TabOrder = 0
      object lblIndicadoresTitle: TLabel
        Left = 24
        Top = 24
        Width = 117
        Height = 21
        Caption = 'Indicadores LED'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 16762424
        Font.Height = -16
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object imgVerde: TImage
        Left = 12
        Top = 82
        Width = 48
        Height = 48
        Center = True
        Proportional = True
        Stretch = True
      end
      object lblOperacao: TLabel
        Left = 69
        Top = 96
        Width = 77
        Height = 17
        Caption = 'Opera'#231#227'o ok'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 14408667
        Font.Height = -13
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object imgAmarelo: TImage
        Left = 12
        Top = 168
        Width = 48
        Height = 48
        Center = True
        Proportional = True
        Stretch = True
      end
      object lblAtencao: TLabel
        Left = 69
        Top = 182
        Width = 50
        Height = 17
        Caption = 'Aten'#231#227'o'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 14408667
        Font.Height = -13
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object imgVermelho: TImage
        Left = 12
        Top = 254
        Width = 48
        Height = 48
        Center = True
        Proportional = True
        Stretch = True
      end
      object lblParada: TLabel
        Left = 69
        Top = 268
        Width = 98
        Height = 17
        Caption = 'Parada imediata'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 14408667
        Font.Height = -13
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
    end
    object pnlEsteiraCard: TPanel
      Left = 242
      Top = 32
      Width = 527
      Height = 330
      BevelOuter = bvNone
      Color = 3487029
      ParentBackground = False
      TabOrder = 1
      object lblEsteiraTitle: TLabel
        Left = 28
        Top = 24
        Width = 155
        Height = 21
        Caption = 'Vis'#227'o geral da esteira'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 16762424
        Font.Height = -16
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object lblEsteiraHint: TLabel
        Left = 193
        Top = 27
        Width = 321
        Height = 17
        Caption = 'Ilustra'#231#227'o centralizada para leitura r'#225'pida do processo'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 9539985
        Font.Height = -13
        Font.Name = 'Segoe UI'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object imgEsteira: TImage
        Left = 28
        Top = 85
        Width = 469
        Height = 236
        Center = True
        Proportional = True
        Stretch = True
      end
      object lblStatusEsteira: TLabel
        Left = 28
        Top = 51
        Width = 132
        Height = 28
        Caption = 'lblStatusEsteira'
        Color = clCream
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindow
        Font.Height = -20
        Font.Name = 'Segoe UI'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
    end
    object pnlControleCard: TPanel
      Left = 798
      Top = 32
      Width = 325
      Height = 330
      BevelOuter = bvNone
      Color = 3487029
      ParentBackground = False
      TabOrder = 2
      object lblControleTitle: TLabel
        Left = 24
        Top = 24
        Width = 114
        Height = 21
        Caption = 'Controle cr'#237'tico'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 16762424
        Font.Height = -16
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object lblControleHint: TLabel
        Left = 24
        Top = 51
        Width = 166
        Height = 17
        Caption = 'Parada rapida de seguranca'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 9539985
        Font.Height = -13
        Font.Name = 'Segoe UI'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object lblStatus: TLabel
        Left = 24
        Top = 92
        Width = 242
        Height = 37
        Caption = 'Status: Aguardando'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 16762424
        Font.Height = -27
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object lblVelocidade: TLabel
        Left = 24
        Top = 142
        Width = 102
        Height = 37
        Caption = 'Speed: 0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 14408667
        Font.Height = -27
        Font.Name = 'Segoe UI'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object Label5: TLabel
        Left = 24
        Top = 212
        Width = 156
        Height = 21
        Caption = 'Bot'#227'o de Emerg'#234'ncia'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 16762424
        Font.Height = -16
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        Transparent = True
      end
      object pbEmergenciaSwitch: TPaintBox
        Left = 24
        Top = 244
        Width = 224
        Height = 64
        Cursor = crHandPoint
        OnClick = pbEmergenciaSwitchClick
        OnPaint = pbEmergenciaSwitchPaint
      end
      object swEmergencia: TToggleSwitch
        Left = 256
        Top = 264
        Width = 106
        Height = 20
        StateCaptions.CaptionOn = 'ATIVA'
        StateCaptions.CaptionOff = 'NORMAL'
        TabOrder = 0
        Visible = False
        OnClick = swEmergenciaClick
      end
    end
    object Memo1: TMemo
      Left = 0
      Top = 432
      Width = 1153
      Height = 160
      Margins.Left = 5
      Margins.Right = 5
      Align = alBottom
      BorderStyle = bsNone
      Color = 2171169
      Font.Charset = DEFAULT_CHARSET
      Font.Color = 14408667
      Font.Height = -12
      Font.Name = 'Consolas'
      Font.Style = []
      Lines.Strings = (
        'Aguardando telemetria MCP...')
      ParentFont = False
      ReadOnly = True
      ScrollBars = ssVertical
      TabOrder = 3
      ExplicitTop = 415
      ExplicitWidth = 1147
    end
  end
  object Timer1: TTimer
    OnTimer = Timer1Timer
    Left = 48
    Top = 576
  end
  object IdHTTP1: TIdHTTP
    ProxyParams.BasicAuthentication = False
    ProxyParams.ProxyPort = 0
    Request.ContentLength = -1
    Request.ContentRangeEnd = -1
    Request.ContentRangeStart = -1
    Request.ContentRangeInstanceLength = -1
    Request.Accept = 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8'
    Request.BasicAuthentication = False
    Request.UserAgent = 'Mozilla/3.0 (compatible; Indy Library)'
    Request.Ranges.Units = 'bytes'
    Request.Ranges = <>
    HTTPOptions = [hoForceEncodeParams]
    Left = 48
    Top = 496
  end
  object tmSwitchAnimation: TTimer
    Enabled = False
    Interval = 15
    OnTimer = tmSwitchAnimationTimer
    Left = 144
    Top = 576
  end
  object tmConveyorAnimation: TTimer
    Enabled = False
    Interval = 45
    OnTimer = tmConveyorAnimationTimer
    Left = 256
    Top = 584
  end
end
