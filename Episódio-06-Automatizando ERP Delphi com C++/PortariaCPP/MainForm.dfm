object MainForm: TMainForm
  Left = 0
  Top = 0
  Caption = 'EmployeeAccess'
  ClientHeight = 861
  ClientWidth = 1586
  Color = clBlack
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWhite
  Font.Height = -19
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poScreenCenter
  WindowState = wsMaximized
  OnResize = FormResize
  TextHeight = 25
  object RootPanel: TPanel
    Left = 0
    Top = 0
    Width = 1586
    Height = 861
    Align = alClient
    BevelOuter = bvNone
    Color = 2036231
    ParentBackground = False
    TabOrder = 0
    ExplicitWidth = 1580
    ExplicitHeight = 844
    DesignSize = (
      1586
      861)
    object TopBarPanel: TPanel
      Left = 0
      Top = 0
      Width = 1586
      Height = 113
      Align = alTop
      BevelOuter = bvNone
      Color = 1314055
      ParentBackground = False
      TabOrder = 0
      ExplicitWidth = 1580
      DesignSize = (
        1586
        113)
      object LblAppName: TLabel
        Left = 28
        Top = 0
        Width = 320
        Height = 41
        AutoSize = False
        Caption = 'Employee Access'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -29
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
      end
      object LblTerminal: TLabel
        Left = 1134
        Top = 22
        Width = 240
        Height = 30
        Alignment = taRightJustify
        Anchors = [akTop, akRight]
        AutoSize = False
        Caption = 'PORTARIA 01'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 15658734
        Font.Height = -21
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        ExplicitLeft = 1132
      end
      object LblOnline: TLabel
        Left = 1392
        Top = 22
        Width = 128
        Height = 30
        Alignment = taCenter
        Anchors = [akTop, akRight]
        AutoSize = False
        Caption = 'ONLINE'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 8709673
        Font.Height = -21
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
        ExplicitLeft = 1390
      end
    end
    object ClockPanel: TPanel
      Left = 0
      Top = 113
      Width = 1586
      Height = 168
      Align = alTop
      BevelOuter = bvNone
      Color = 2036231
      ParentBackground = False
      TabOrder = 1
      ExplicitWidth = 1580
      object LblClock: TLabel
        Left = 0
        Top = 0
        Width = 1586
        Height = 94
        Align = alTop
        Alignment = taCenter
        AutoSize = False
        Caption = '00:00'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -80
        Font.Name = 'Segoe UI Light'
        Font.Style = []
        ParentFont = False
        Layout = tlCenter
        ExplicitWidth = 1584
      end
      object LblDate: TLabel
        Left = 0
        Top = 94
        Width = 1586
        Height = 34
        Align = alTop
        Alignment = taCenter
        AutoSize = False
        Caption = 'segunda-feira, 10 de agosto de 2026'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 11711154
        Font.Height = -21
        Font.Name = 'Segoe UI'
        Font.Style = []
        ParentFont = False
        Layout = tlCenter
        ExplicitWidth = 1584
      end
    end
    object PageHost: TPanel
      Left = 0
      Top = 281
      Width = 1586
      Height = 580
      Align = alClient
      BevelOuter = bvNone
      Color = 2036231
      ParentBackground = False
      TabOrder = 2
      ExplicitWidth = 1580
      ExplicitHeight = 563
      object IdentificationView: TPanel
        Left = 0
        Top = 0
        Width = 1586
        Height = 580
        Align = alClient
        BevelOuter = bvNone
        Color = 2036231
        ParentBackground = False
        TabOrder = 0
        ExplicitWidth = 1580
        ExplicitHeight = 563
        object BottomStatusPanel: TPanel
          Left = 0
          Top = 410
          Width = 1586
          Height = 170
          Align = alBottom
          BevelOuter = bvNone
          Color = 2036231
          ParentBackground = False
          TabOrder = 0
          ExplicitTop = 393
          ExplicitWidth = 1580
          object LblCameraStatus: TLabel
            Left = 44
            Top = 136
            Width = 1496
            Height = 24
            Alignment = taCenter
            AutoSize = False
            Caption = 'C'#195#8218'MERA: aguardando inicializa'#195#167#195#163'o'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = 11711154
            Font.Height = -16
            Font.Name = 'Segoe UI'
            Font.Style = []
            ParentFont = False
          end
          object FaceStatusCard: TPanel
            Left = 338
            Top = 12
            Width = 370
            Height = 72
            BevelOuter = bvNone
            Color = 1314055
            ParentBackground = False
            TabOrder = 0
            object LblFaceIndicator: TLabel
              Left = 0
              Top = 10
              Width = 370
              Height = 30
              Alignment = taCenter
              AutoSize = False
              Caption = 'FACE: aguardando'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = 5222143
              Font.Height = -24
              Font.Name = 'Segoe UI Semibold'
              Font.Style = []
              ParentFont = False
            end
            object LblFaceStatusDetail: TLabel
              Left = 0
              Top = 42
              Width = 370
              Height = 22
              Alignment = taCenter
              AutoSize = False
              Caption = 'Status: aguardando'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = 5222143
              Font.Height = -16
              Font.Name = 'Segoe UI'
              Font.Style = []
              ParentFont = False
            end
          end
          object BleStatusCard: TPanel
            Left = 876
            Top = 12
            Width = 370
            Height = 72
            BevelOuter = bvNone
            Color = 1314055
            ParentBackground = False
            TabOrder = 1
            object LblBleIndicator: TLabel
              Left = 0
              Top = 10
              Width = 370
              Height = 30
              Alignment = taCenter
              AutoSize = False
              Caption = 'BLE: aguardando'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = 5222143
              Font.Height = -24
              Font.Name = 'Segoe UI Semibold'
              Font.Style = []
              ParentFont = False
            end
            object LblBleStatusDetail: TLabel
              Left = 0
              Top = 42
              Width = 370
              Height = 22
              Alignment = taCenter
              AutoSize = False
              Caption = 'Status: aguardando'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = 5222143
              Font.Height = -16
              Font.Name = 'Segoe UI'
              Font.Style = []
              ParentFont = False
            end
          end
          object MenuCenterPanel: TPanel
            Left = 662
            Top = 82
            Width = 260
            Height = 84
            BevelOuter = bvNone
            Color = 2036231
            ParentBackground = False
            TabOrder = 2
            object LblMenuHint: TLabel
              Left = 0
              Top = 58
              Width = 260
              Height = 22
              Alignment = taCenter
              AutoSize = False
              Caption = 'Toque para abrir as op'#195#167#195#181'es'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = 11711154
              Font.Height = -15
              Font.Name = 'Segoe UI'
              Font.Style = []
              ParentFont = False
            end
            object BtnMenu: TPanel
              Left = 46
              Top = 0
              Width = 168
              Height = 56
              BevelOuter = bvNone
              Caption = #226#732#176' MENU'
              Color = 3947580
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -21
              Font.Name = 'Segoe UI Semibold'
              Font.Style = []
              ParentBackground = False
              ParentFont = False
              TabOrder = 0
              OnClick = MenuButtonClick
            end
          end
        end
        object CameraPanel: TPanel
          AlignWithMargins = True
          Left = 220
          Top = 0
          Width = 1146
          Height = 410
          Margins.Left = 220
          Margins.Top = 0
          Margins.Right = 220
          Margins.Bottom = 0
          Align = alClient
          BevelOuter = bvNone
          Color = 723723
          ParentBackground = False
          TabOrder = 1
          ExplicitWidth = 1140
          ExplicitHeight = 393
          object ShapeFaceFrame: TShape
            Left = 320
            Top = 56
            Width = 504
            Height = 220
            Brush.Style = bsClear
            Pen.Color = 9928780
            Pen.Width = 2
          end
          object LblFaceGlyph: TLabel
            Left = 0
            Top = 98
            Width = 1144
            Height = 72
            Alignment = taCenter
            AutoSize = False
            Caption = 'FACE ID'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = 5921370
            Font.Height = -53
            Font.Name = 'Segoe UI Light'
            Font.Style = []
            ParentFont = False
            Layout = tlCenter
          end
          object LblCameraPlaceholder: TLabel
            Left = 0
            Top = 174
            Width = 1144
            Height = 38
            Alignment = taCenter
            AutoSize = False
            Caption = 'C'#195#8218'MERA PRONTA PARA PREVIEW'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = 5921370
            Font.Height = -19
            Font.Name = 'Segoe UI Semibold'
            Font.Style = []
            ParentFont = False
            Layout = tlCenter
          end
          object LblPrompt: TLabel
            Left = 0
            Top = 330
            Width = 1144
            Height = 46
            Alignment = taCenter
            AutoSize = False
            Caption = 'Aproxime-se para identifica'#195#167#195#163'o'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWhite
            Font.Height = -35
            Font.Name = 'Segoe UI Semibold'
            Font.Style = []
            ParentFont = False
            Layout = tlCenter
          end
          object EmployeeResultPanel: TPanel
            Left = 322
            Top = 58
            Width = 500
            Height = 310
            BevelOuter = bvNone
            Color = 2036231
            ParentBackground = False
            TabOrder = 0
            Visible = False
            object LblEmployeeState: TLabel
              Left = 210
              Top = 38
              Width = 260
              Height = 36
              AutoSize = False
              Caption = 'IDENTIDADE CONFIRMADA'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = 8709673
              Font.Height = -21
              Font.Name = 'Segoe UI Semibold'
              Font.Style = []
              ParentFont = False
            end
            object LblName: TLabel
              Left = 210
              Top = 86
              Width = 260
              Height = 44
              AutoSize = False
              Caption = 'Nome'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = clWhite
              Font.Height = -32
              Font.Name = 'Segoe UI Semibold'
              Font.Style = []
              ParentFont = False
            end
            object LblDepartment: TLabel
              Left = 210
              Top = 142
              Width = 260
              Height = 30
              AutoSize = False
              Caption = 'Departamento'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = 11711154
              Font.Height = -21
              Font.Name = 'Segoe UI'
              Font.Style = []
              ParentFont = False
            end
            object LblIdentifiedAt: TLabel
              Left = 210
              Top = 184
              Width = 260
              Height = 34
              AutoSize = False
              Caption = '00:00:00'
              Font.Charset = DEFAULT_CHARSET
              Font.Color = 11711154
              Font.Height = -24
              Font.Name = 'Segoe UI Semibold'
              Font.Style = []
              ParentFont = False
            end
            object PhotoPanel: TPanel
              Left = 32
              Top = 44
              Width = 150
              Height = 150
              BevelOuter = bvNone
              Color = 2236962
              ParentBackground = False
              TabOrder = 0
              object LblPhoto: TLabel
                Left = 0
                Top = 58
                Width = 150
                Height = 34
                Alignment = taCenter
                AutoSize = False
                Caption = 'FOTO'
                Font.Charset = DEFAULT_CHARSET
                Font.Color = 11711154
                Font.Height = -21
                Font.Name = 'Segoe UI Semibold'
                Font.Style = []
                ParentFont = False
              end
            end
          end
        end
      end
      object SecondaryView: TPanel
        Left = 0
        Top = 0
        Width = 1586
        Height = 580
        Align = alClient
        BevelOuter = bvNone
        Color = 2036231
        ParentBackground = False
        TabOrder = 1
        Visible = False
        ExplicitWidth = 1580
        ExplicitHeight = 563
        DesignSize = (
          1586
          580)
        object LblAdminTitle: TLabel
          Left = 96
          Top = 138
          Width = 720
          Height = 56
          AutoSize = False
          Caption = 'Configura'#231#245'es'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -43
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
        end
        object LblAdminDescription: TLabel
          Left = 96
          Top = 214
          Width = 940
          Height = 96
          AutoSize = False
          Caption = 'Tela estrutural para funcionalidades administrativas futuras.'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 11711154
          Font.Height = -24
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
          WordWrap = True
        end
        object BtnBackFromPage: TPanel
          Left = 96
          Top = 42
          Width = 394
          Height = 58
          BevelOuter = bvNone
          Caption = #226#8224#144' VOLTAR PARA IDENTIFICA'#195#8225#195#402'O'
          Color = 3947580
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -19
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentBackground = False
          ParentFont = False
          TabOrder = 0
          OnClick = BackToIdentificationClick
        end
        object DeveloperPanel: TPanel
          Left = 1096
          Top = 96
          Width = 366
          Height = 444
          Anchors = [akTop, akRight]
          BevelOuter = bvNone
          Color = 1314055
          ParentBackground = False
          TabOrder = 1
          Visible = False
          ExplicitLeft = 1090
          object LblDeveloperStatus: TLabel
            Left = 24
            Top = 20
            Width = 318
            Height = 44
            AutoSize = False
            Caption = 
              'Selecione um funcion'#195#161'rio real do banco e capture a biometria fa' +
              'cial.'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = 11711154
            Font.Height = -16
            Font.Name = 'Segoe UI'
            Font.Style = []
            ParentFont = False
            WordWrap = True
          end
          object LstEmployees: TListBox
            Left = 24
            Top = 78
            Width = 318
            Height = 154
            Color = 2036231
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWhite
            Font.Height = -16
            Font.Name = 'Segoe UI'
            Font.Style = []
            ItemHeight = 21
            ParentFont = False
            TabOrder = 0
          end
          object BtnSimulate: TButton
            Left = 24
            Top = 252
            Width = 318
            Height = 52
            Caption = 'Cadastrar biometria facial'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -19
            Font.Name = 'Segoe UI Semibold'
            Font.Style = []
            ParentFont = False
            TabOrder = 1
            OnClick = SimulateClick
          end
          object BtnClear: TButton
            Left = 24
            Top = 318
            Width = 318
            Height = 52
            Caption = 'Atualizar funcion'#195#161'rios'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -19
            Font.Name = 'Segoe UI Semibold'
            Font.Style = []
            ParentFont = False
            TabOrder = 2
            OnClick = ClearClick
          end
          object BtnStartCamera: TButton
            Left = 24
            Top = 384
            Width = 318
            Height = 52
            Caption = 'Iniciar c'#195#162'mera'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -19
            Font.Name = 'Segoe UI Semibold'
            Font.Style = []
            ParentFont = False
            TabOrder = 3
            OnClick = StartCameraClick
          end
        end
      end
    end
    object OverlayMenu: TPanel
      Left = 1111
      Top = 0
      Width = 475
      Height = 861
      Anchors = [akTop, akRight, akBottom]
      BevelOuter = bvNone
      Color = 1314055
      ParentBackground = False
      TabOrder = 3
      Visible = False
      ExplicitLeft = 1105
      ExplicitHeight = 844
      DesignSize = (
        475
        861)
      object LblMenuHeader: TLabel
        Left = 30
        Top = 102
        Width = 390
        Height = 34
        AutoSize = False
        Caption = 'Opera'#195#167#195#181'es da portaria'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -24
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentFont = False
      end
      object LblMenuFooter: TLabel
        Left = 24
        Top = 816
        Width = 427
        Height = 24
        Alignment = taCenter
        Anchors = [akLeft, akRight, akBottom]
        AutoSize = False
        Caption = 'PORTARIA 01 | Vers'#195#163'o 1.0.0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = 11711154
        Font.Height = -15
        Font.Name = 'Segoe UI'
        Font.Style = []
        ParentFont = False
      end
      object BtnMenuBack: TPanel
        Left = 24
        Top = 24
        Width = 427
        Height = 58
        Anchors = [akLeft, akTop, akRight]
        BevelOuter = bvNone
        BiDiMode = bdLeftToRight
        Caption = '                                       FECHAR MENU  --> '
        Color = 3947580
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -19
        Font.Name = 'Segoe UI Semibold'
        Font.Style = []
        ParentBiDiMode = False
        ParentBackground = False
        ParentFont = False
        TabOrder = 0
        OnClick = BackToIdentificationClick
      end
      object MenuIdentification: TPanel
        Left = 24
        Top = 154
        Width = 427
        Height = 72
        Anchors = [akLeft, akTop, akRight]
        BevelOuter = bvNone
        Color = 6764528
        ParentBackground = False
        TabOrder = 1
        OnClick = MenuItemClick
        DesignSize = (
          427
          72)
        object LblMenuIdentificationIcon: TLabel
          Left = 18
          Top = 14
          Width = 48
          Height = 44
          Alignment = taCenter
          AutoSize = False
          Caption = 'ID'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 15658734
          Font.Height = -24
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          Layout = tlCenter
          OnClick = MenuItemClick
        end
        object LblMenuIdentificationTitle: TLabel
          Left = 78
          Top = 12
          Width = 260
          Height = 25
          AutoSize = False
          Caption = 'Identifica'#195#167#195#163'o'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -19
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuIdentificationDesc: TLabel
          Left = 78
          Top = 39
          Width = 288
          Height = 22
          AutoSize = False
          Caption = 'Tela principal de identifica'#195#167#195#163'o'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 11711154
          Font.Height = -15
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuIdentificationArrow: TLabel
          Left = 386
          Top = 24
          Width = 24
          Height = 24
          Alignment = taCenter
          Anchors = [akTop, akRight]
          AutoSize = False
          Caption = '>'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -21
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
      end
      object MenuPresent: TPanel
        Tag = 1
        Left = 24
        Top = 236
        Width = 427
        Height = 72
        Anchors = [akLeft, akTop, akRight]
        BevelOuter = bvNone
        Color = 2302755
        ParentBackground = False
        TabOrder = 2
        OnClick = MenuItemClick
        DesignSize = (
          427
          72)
        object LblMenuPresentIcon: TLabel
          Tag = 1
          Left = 18
          Top = 14
          Width = 48
          Height = 44
          Alignment = taCenter
          AutoSize = False
          Caption = 'P'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 15658734
          Font.Height = -24
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          Layout = tlCenter
          OnClick = MenuItemClick
        end
        object LblMenuPresentTitle: TLabel
          Tag = 1
          Left = 78
          Top = 12
          Width = 260
          Height = 25
          AutoSize = False
          Caption = 'Funcion'#195#161'rios presentes'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -19
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuPresentDesc: TLabel
          Tag = 1
          Left = 78
          Top = 39
          Width = 288
          Height = 22
          AutoSize = False
          Caption = 'Pessoas presentes na empresa'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 11711154
          Font.Height = -15
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuPresentArrow: TLabel
          Tag = 1
          Left = 386
          Top = 24
          Width = 24
          Height = 24
          Alignment = taCenter
          Anchors = [akTop, akRight]
          AutoSize = False
          Caption = '>'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -21
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
      end
      object MenuRecent: TPanel
        Tag = 2
        Left = 24
        Top = 318
        Width = 427
        Height = 72
        Anchors = [akLeft, akTop, akRight]
        BevelOuter = bvNone
        Color = 2302755
        ParentBackground = False
        TabOrder = 3
        OnClick = MenuItemClick
        DesignSize = (
          427
          72)
        object LblMenuRecentIcon: TLabel
          Tag = 2
          Left = 18
          Top = 14
          Width = 48
          Height = 44
          Alignment = taCenter
          AutoSize = False
          Caption = 'H'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 15658734
          Font.Height = -24
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          Layout = tlCenter
          OnClick = MenuItemClick
        end
        object LblMenuRecentTitle: TLabel
          Tag = 2
          Left = 78
          Top = 12
          Width = 260
          Height = 25
          AutoSize = False
          Caption = #195#353'ltimas entradas'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -19
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuRecentDesc: TLabel
          Tag = 2
          Left = 78
          Top = 39
          Width = 288
          Height = 22
          AutoSize = False
          Caption = 'Eventos recentes do terminal'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 11711154
          Font.Height = -15
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuRecentArrow: TLabel
          Tag = 2
          Left = 386
          Top = 24
          Width = 24
          Height = 24
          Alignment = taCenter
          Anchors = [akTop, akRight]
          AutoSize = False
          Caption = '>'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -21
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
      end
      object MenuCamera: TPanel
        Tag = 3
        Left = 24
        Top = 400
        Width = 427
        Height = 72
        Anchors = [akLeft, akTop, akRight]
        BevelOuter = bvNone
        Color = 2302755
        ParentBackground = False
        TabOrder = 4
        OnClick = MenuItemClick
        DesignSize = (
          427
          72)
        object LblMenuCameraIcon: TLabel
          Tag = 3
          Left = 18
          Top = 14
          Width = 48
          Height = 44
          Alignment = taCenter
          AutoSize = False
          Caption = 'CAM'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 15658734
          Font.Height = -18
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          Layout = tlCenter
          OnClick = MenuItemClick
        end
        object LblMenuCameraTitle: TLabel
          Tag = 3
          Left = 78
          Top = 12
          Width = 260
          Height = 25
          AutoSize = False
          Caption = 'C'#195#162'mera'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -19
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuCameraDesc: TLabel
          Tag = 3
          Left = 78
          Top = 39
          Width = 288
          Height = 22
          AutoSize = False
          Caption = 'Configura'#195#167#195#163'o e teste de v'#195#173'deo'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 11711154
          Font.Height = -15
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuCameraArrow: TLabel
          Tag = 3
          Left = 386
          Top = 24
          Width = 24
          Height = 24
          Alignment = taCenter
          Anchors = [akTop, akRight]
          AutoSize = False
          Caption = '>'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -21
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
      end
      object MenuBle: TPanel
        Tag = 4
        Left = 24
        Top = 482
        Width = 427
        Height = 72
        Anchors = [akLeft, akTop, akRight]
        BevelOuter = bvNone
        Color = 2302755
        ParentBackground = False
        TabOrder = 5
        OnClick = MenuItemClick
        DesignSize = (
          427
          72)
        object LblMenuBleIcon: TLabel
          Tag = 4
          Left = 18
          Top = 14
          Width = 48
          Height = 44
          Alignment = taCenter
          AutoSize = False
          Caption = 'BLE'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 15658734
          Font.Height = -18
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          Layout = tlCenter
          OnClick = MenuItemClick
        end
        object LblMenuBleTitle: TLabel
          Tag = 4
          Left = 78
          Top = 12
          Width = 260
          Height = 25
          AutoSize = False
          Caption = 'Dispositivos BLE'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -19
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuBleDesc: TLabel
          Tag = 4
          Left = 78
          Top = 39
          Width = 288
          Height = 22
          AutoSize = False
          Caption = 'Chaveiros encontrados e RSSI'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 11711154
          Font.Height = -15
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuBleArrow: TLabel
          Tag = 4
          Left = 386
          Top = 24
          Width = 24
          Height = 24
          Alignment = taCenter
          Anchors = [akTop, akRight]
          AutoSize = False
          Caption = '>'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -21
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
      end
      object MenuManual: TPanel
        Tag = 5
        Left = 24
        Top = 564
        Width = 427
        Height = 72
        Anchors = [akLeft, akTop, akRight]
        BevelOuter = bvNone
        Color = 2302755
        ParentBackground = False
        TabOrder = 6
        OnClick = MenuItemClick
        DesignSize = (
          427
          72)
        object LblMenuManualIcon: TLabel
          Tag = 5
          Left = 18
          Top = 14
          Width = 48
          Height = 44
          Alignment = taCenter
          AutoSize = False
          Caption = 'OK'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 15658734
          Font.Height = -22
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          Layout = tlCenter
          OnClick = MenuItemClick
        end
        object LblMenuManualTitle: TLabel
          Tag = 5
          Left = 78
          Top = 12
          Width = 260
          Height = 25
          AutoSize = False
          Caption = 'Libera'#195#167#195#163'o manual'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -19
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuManualDesc: TLabel
          Tag = 5
          Left = 78
          Top = 39
          Width = 288
          Height = 22
          AutoSize = False
          Caption = 'Libera'#195#167#195#163'o assistida pelo porteiro'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 11711154
          Font.Height = -15
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuManualArrow: TLabel
          Tag = 5
          Left = 386
          Top = 24
          Width = 24
          Height = 24
          Alignment = taCenter
          Anchors = [akTop, akRight]
          AutoSize = False
          Caption = '>'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -21
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
      end
      object MenuOccurrences: TPanel
        Tag = 6
        Left = 24
        Top = 646
        Width = 427
        Height = 72
        Anchors = [akLeft, akTop, akRight]
        BevelOuter = bvNone
        Color = 2302755
        ParentBackground = False
        TabOrder = 7
        OnClick = MenuItemClick
        DesignSize = (
          427
          72)
        object LblMenuOccurrencesIcon: TLabel
          Tag = 6
          Left = 18
          Top = 14
          Width = 48
          Height = 44
          Alignment = taCenter
          AutoSize = False
          Caption = '!'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 15658734
          Font.Height = -26
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          Layout = tlCenter
          OnClick = MenuItemClick
        end
        object LblMenuOccurrencesTitle: TLabel
          Tag = 6
          Left = 78
          Top = 12
          Width = 260
          Height = 25
          AutoSize = False
          Caption = 'Ocorr'#195#170'ncias'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -19
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuOccurrencesDesc: TLabel
          Tag = 6
          Left = 78
          Top = 39
          Width = 288
          Height = 22
          AutoSize = False
          Caption = 'Eventos e problemas de acesso'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 11711154
          Font.Height = -15
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuOccurrencesArrow: TLabel
          Tag = 6
          Left = 386
          Top = 24
          Width = 24
          Height = 24
          Alignment = taCenter
          Anchors = [akTop, akRight]
          AutoSize = False
          Caption = '>'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -21
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
      end
      object MenuSettings: TPanel
        Tag = 7
        Left = 24
        Top = 728
        Width = 427
        Height = 72
        Anchors = [akLeft, akTop, akRight]
        BevelOuter = bvNone
        Color = 2302755
        ParentBackground = False
        TabOrder = 8
        OnClick = MenuItemClick
        DesignSize = (
          427
          72)
        object LblMenuSettingsIcon: TLabel
          Tag = 7
          Left = 18
          Top = 14
          Width = 48
          Height = 44
          Alignment = taCenter
          AutoSize = False
          Caption = 'CFG'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 15658734
          Font.Height = -18
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          Layout = tlCenter
          OnClick = MenuItemClick
        end
        object LblMenuSettingsTitle: TLabel
          Tag = 7
          Left = 78
          Top = 12
          Width = 260
          Height = 25
          AutoSize = False
          Caption = 'Configura'#195#167#195#181'es'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -19
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuSettingsDesc: TLabel
          Tag = 7
          Left = 78
          Top = 39
          Width = 288
          Height = 22
          AutoSize = False
          Caption = 'Terminal, API e par'#195#162'metros'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = 11711154
          Font.Height = -15
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
        object LblMenuSettingsArrow: TLabel
          Tag = 7
          Left = 386
          Top = 24
          Width = 24
          Height = 24
          Alignment = taCenter
          Anchors = [akTop, akRight]
          AutoSize = False
          Caption = '>'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWhite
          Font.Height = -21
          Font.Name = 'Segoe UI Semibold'
          Font.Style = []
          ParentFont = False
          OnClick = MenuItemClick
        end
      end
    end
  end
  object ClockTimer: TTimer
    OnTimer = ClockTimerTimer
    Left = 1472
    Top = 104
  end
  object ReturnTimer: TTimer
    Enabled = False
    Interval = 5000
    OnTimer = ReturnTimerTimer
    Left = 1472
    Top = 160
  end
end
