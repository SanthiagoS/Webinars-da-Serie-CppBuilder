object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'Form1'
  ClientHeight = 455
  ClientWidth = 522
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  TextHeight = 15
  object Label5: TLabel
    Left = 232
    Top = 423
    Width = 69
    Height = 15
    Caption = 'IP do Esp32 : '
  end
  object gboxLed: TGroupBox
    Left = 21
    Top = 112
    Width = 233
    Height = 177
    Caption = 'Controle Leds'
    TabOrder = 0
    object BtnLedOn: TButton
      Left = 21
      Top = 112
      Width = 75
      Height = 25
      Caption = 'Led1 ON'
      TabOrder = 0
      OnClick = BtnLedOnClick
    end
    object BtnLedOff: TButton
      Left = 127
      Top = 112
      Width = 75
      Height = 25
      Caption = 'Led1 OFF'
      TabOrder = 1
      OnClick = BtnLedOffClick
    end
  end
  object gboxRele: TGroupBox
    Left = 280
    Top = 112
    Width = 217
    Height = 177
    Caption = 'Controle Rel'#234's'
    TabOrder = 1
    object BtnRele1ON: TButton
      Left = 14
      Top = 56
      Width = 75
      Height = 25
      Caption = 'Rel'#234' 1 ON'
      TabOrder = 0
      OnClick = BtnRele1ONClick
    end
    object BtnRele1OFF: TButton
      Left = 119
      Top = 56
      Width = 75
      Height = 25
      Caption = 'Rel'#234' 1 OFF'
      TabOrder = 1
      OnClick = BtnRele1OFFClick
    end
    object BtnRele2ON: TButton
      Left = 14
      Top = 112
      Width = 75
      Height = 25
      Caption = 'Rel'#234' 2 ON'
      TabOrder = 2
      OnClick = BtnRele2ONClick
    end
    object BtnRele2OFF: TButton
      Left = 119
      Top = 112
      Width = 75
      Height = 25
      Caption = 'Rel'#234' 2 OFF'
      TabOrder = 3
      OnClick = BtnRele2OFFClick
    end
  end
  object gboxSensores: TGroupBox
    Left = 21
    Top = 304
    Width = 476
    Height = 105
    Caption = 'Controle Sensores'
    TabOrder = 2
    object lblTemperatura: TLabel
      Left = 105
      Top = 40
      Width = 72
      Height = 15
      Caption = 'Temperatura'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblPotenciometro: TLabel
      Left = 105
      Top = 72
      Width = 87
      Height = 15
      Caption = 'Potenciometro:'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblPresenca: TLabel
      Left = 321
      Top = 40
      Width = 62
      Height = 15
      Caption = 'Presen'#231'a :  '
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblProximidade: TLabel
      Left = 321
      Top = 72
      Width = 79
      Height = 15
      Caption = 'Proximidade : '
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object Label1: TLabel
      Left = 14
      Top = 40
      Width = 73
      Height = 15
      Caption = 'Temperatura :'
    end
    object Label2: TLabel
      Left = 14
      Top = 72
      Width = 85
      Height = 15
      Caption = 'Potenci'#244'metro :'
    end
    object Label3: TLabel
      Left = 240
      Top = 40
      Width = 56
      Height = 15
      Caption = 'Presen'#231'a : '
    end
    object Label4: TLabel
      Left = 240
      Top = 72
      Width = 75
      Height = 15
      Caption = 'Proximidade : '
    end
  end
  object Panel1: TPanel
    AlignWithMargins = True
    Left = 5
    Top = 5
    Width = 512
    Height = 81
    Margins.Left = 5
    Margins.Top = 5
    Margins.Right = 5
    Margins.Bottom = 5
    Align = alTop
    Caption = 'VCL ESP32 Control'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -27
    Font.Name = 'Segoe UI'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 3
    ExplicitWidth = 511
    object lblIPServidor: TLabel
      Left = 1
      Top = 63
      Width = 510
      Height = 17
      Align = alBottom
      Alignment = taCenter
      BiDiMode = bdLeftToRight
      Caption = 'IP : Servidor REST IoT'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentBiDiMode = False
      ParentFont = False
      Layout = tlCenter
      ExplicitTop = 56
      ExplicitWidth = 125
    end
  end
  object edtIpServidor: TEdit
    Left = 320
    Top = 419
    Width = 121
    Height = 23
    TabOrder = 4
  end
  object Button1: TButton
    Left = 448
    Top = 417
    Width = 50
    Height = 25
    Caption = 'Aplicar'
    TabOrder = 5
    OnClick = Button1Click
  end
  object NetHTTPClient1: TNetHTTPClient
    UserAgent = 'Embarcadero URI Client/1.0'
    Left = 40
    Top = 24
  end
  object Timer1: TTimer
    OnTimer = Timer1Timer
    Left = 424
    Top = 32
  end
end
