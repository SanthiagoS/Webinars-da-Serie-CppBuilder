object frmPerformanceLabWorker: TfrmPerformanceLabWorker
  Left = 0
  Top = 0
  Caption = 'Teste 2 PERFORMANCE | Worker Thread'
  ClientHeight = 901
  ClientWidth = 1120
  Color = clBtnFace
  Constraints.MinHeight = 900
  Constraints.MinWidth = 1136
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -13
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poScreenCenter
  DesignSize = (
    1120
    901)
  TextHeight = 17
  object pnlHeader: TPanel
    Left = 0
    Top = 0
    Width = 1120
    Height = 76
    Align = alTop
    BevelOuter = bvNone
    Color = 5582348
    ParentBackground = False
    TabOrder = 5
    ExplicitWidth = 1114
    object lblTitle: TLabel
      Left = 20
      Top = 10
      Width = 1080
      Height = 24
      AutoSize = False
      Caption = 'Teste 2 PERFORMANCE | Worker Thread'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -22
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblSubtitle: TLabel
      Left = 20
      Top = 46
      Width = 1080
      Height = 24
      AutoSize = False
      Caption = 'Entendendo sobre Threads.'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
  end
  object grpControls: TGroupBox
    AlignWithMargins = True
    Left = 3
    Top = 79
    Width = 1114
    Height = 131
    Margins.Bottom = 10
    Align = alTop
    Caption = '1- Carga e interacao'
    TabOrder = 0
    ExplicitWidth = 1108
    object lblLoad: TLabel
      Left = 16
      Top = 26
      Width = 140
      Height = 24
      AutoSize = False
      Caption = 'Carga: 1 / 20'
    end
    object lblProbe: TLabel
      Left = 540
      Top = 57
      Width = 450
      Height = 24
      AutoSize = False
      Caption = 'Cliques atendidos: 0'
    end
    object lblProbeLatency: TLabel
      Left = 543
      Top = 94
      Width = 448
      Height = 24
      AutoSize = False
      Caption = 'Resposta do bot'#227'o: '#250'ltima -- ms | maior -- ms'
    end
    object lblStressDuration: TLabel
      Left = 13
      Top = 98
      Width = 260
      Height = 24
      AutoSize = False
      Caption = '2 - Teste de carga do processador (s)'
    end
    object trkLoad: TTrackBar
      Left = 156
      Top = 20
      Width = 340
      Height = 42
      Max = 20
      Min = 1
      Position = 1
      TabOrder = 0
      OnChange = trkLoadChange
    end
    object chkAuto: TCheckBox
      Left = 510
      Top = 26
      Width = 280
      Height = 24
      Caption = 'Aumentar carga a cada 8 ciclos'
      Checked = True
      State = cbChecked
      TabOrder = 1
    end
    object chkPersist: TCheckBox
      Left = 800
      Top = 26
      Width = 260
      Height = 24
      Caption = 'CSV: abrir / gravar / flush por item'
      Checked = True
      State = cbChecked
      TabOrder = 2
    end
    object btnStart: TButton
      Left = 16
      Top = 51
      Width = 100
      Height = 32
      Caption = 'Iniciar'
      TabOrder = 3
      OnClick = btnStartClick
    end
    object btnStop: TButton
      Left = 126
      Top = 51
      Width = 100
      Height = 32
      Caption = 'Parar'
      Enabled = False
      TabOrder = 4
      OnClick = btnStopClick
    end
    object btnReset: TButton
      Left = 236
      Top = 51
      Width = 100
      Height = 32
      Caption = 'Limpar'
      TabOrder = 5
      OnClick = btnResetClick
    end
    object btnProbe: TButton
      Left = 364
      Top = 51
      Width = 156
      Height = 32
      Caption = 'Testar resposta'
      TabOrder = 6
      OnClick = btnProbeClick
    end
    object cmbStressDuration: TComboBox
      Left = 260
      Top = 94
      Width = 76
      Height = 25
      Style = csDropDownList
      ItemIndex = 1
      TabOrder = 7
      Text = '4'
      Items.Strings = (
        '3'
        '4'
        '5')
    end
    object btnStress: TButton
      Left = 343
      Top = 89
      Width = 177
      Height = 32
      Caption = 'Executar teste pesado'
      TabOrder = 8
      OnClick = btnStressClick
    end
  end
  object pnlWorkPerformed: TPanel
    Left = 16
    Top = 223
    Width = 330
    Height = 174
    BevelOuter = bvNone
    Color = clWhite
    ParentBackground = False
    TabOrder = 1
    object lblWorkTitle: TLabel
      Left = 16
      Top = 10
      Width = 298
      Height = 26
      AutoSize = False
      Caption = 'Trabalho realizado'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -17
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblOperationsTitle: TLabel
      Left = 16
      Top = 43
      Width = 298
      Height = 24
      AutoSize = False
      Caption = 'Opera'#231#245'es executadas'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblOperationsValue: TLabel
      Left = 16
      Top = 67
      Width = 298
      Height = 40
      Hint = 
        'Cada opera'#231#227'o corresponde a uma rodada de seno, cosseno e raiz n' +
        'o c'#225'lculo do teste pesado.'
      AutoSize = False
      Caption = '0'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -30
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
    end
    object lblOperationsRateTitle: TLabel
      Left = 16
      Top = 116
      Width = 298
      Height = 24
      AutoSize = False
      Caption = 'Velocidade m'#233'dia'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblOperationsRateValue: TLabel
      Left = 16
      Top = 134
      Width = 298
      Height = 35
      AutoSize = False
      Caption = '-- milh'#245'es/s'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -23
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
  end
  object grpCpuUsage: TGroupBox
    AlignWithMargins = True
    Left = 358
    Top = 223
    Width = 396
    Height = 174
    Caption = 'Uso do processador'
    TabOrder = 2
    object lblCpuMachineTitle: TLabel
      Left = 12
      Top = 22
      Width = 280
      Height = 24
      AutoSize = False
      Caption = 'CPU do sistema'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblCpuMachineValue: TLabel
      Left = 300
      Top = 20
      Width = 82
      Height = 32
      Alignment = taRightJustify
      AutoSize = False
      Caption = '--%'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -24
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblCpuCoreTitle: TLabel
      Left = 12
      Top = 77
      Width = 285
      Height = 22
      AutoSize = False
      Caption = 'Worker Thread (% de 1 CPU logica)'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblCpuCoreValue: TLabel
      Left = 300
      Top = 74
      Width = 82
      Height = 32
      Alignment = taRightJustify
      AutoSize = False
      Caption = '--%'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -24
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblCpuNote: TLabel
      Left = 12
      Top = 134
      Width = 370
      Height = 44
      AutoSize = False
      Caption = 
        'Main Thread: interface, eventos e atualizacao visual. A carga de' +
        ' calculo e persistencia esta na Worker Thread.'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
      WordWrap = True
    end
    object pbCpuMachine: TProgressBar
      Left = 12
      Top = 50
      Width = 370
      Height = 18
      TabOrder = 0
    end
    object pbCpuCore: TProgressBar
      Left = 12
      Top = 108
      Width = 370
      Height = 18
      TabOrder = 1
    end
  end
  object grpScreenState: TGroupBox
    AlignWithMargins = True
    Left = 766
    Top = 223
    Width = 332
    Height = 174
    Anchors = [akLeft, akTop, akRight]
    Caption = 'Estado da tela'
    TabOrder = 3
    ExplicitWidth = 326
    object lblScreenState: TLabel
      Left = 16
      Top = 22
      Width = 306
      Height = 34
      AutoSize = False
      Caption = 'Respondendo'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGreen
      Font.Height = -25
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblFreezeTitle: TLabel
      Left = 16
      Top = 62
      Width = 306
      Height = 24
      AutoSize = False
      Caption = 'Tempo sem resposta da tela'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblFreezeTime: TLabel
      Left = 16
      Top = 86
      Width = 306
      Height = 39
      AutoSize = False
      Caption = '0,00 s'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -30
      Font.Name = 'Segoe UI'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblFreezeNote: TLabel
      Left = 16
      Top = 128
      Width = 306
      Height = 20
      AutoSize = False
      Caption = 'Sem espera prolongada registrada.'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblFreezeScope: TLabel
      Left = 16
      Top = 152
      Width = 306
      Height = 20
      AutoSize = False
      Caption = 'Maior espera desde Limpar ou Iniciar.'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
  end
  object pagesResults: TPageControl
    Left = 0
    Top = 220
    Width = 1120
    Height = 401
    ActivePage = tabOverview
    Align = alTop
    TabOrder = 4
    ExplicitWidth = 1114
    object tabOverview: TTabSheet
      Caption = 'Hist'#243'ricos e resultado'
      DesignSize = (
        1112
        369)
      object lblStress: TLabel
        Left = 12
        Top = 350
        Width = 1052
        Height = 34
        AutoSize = False
        Caption = 'Teste de carga do processador: aguardando execu'#231#227'o.'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -12
        Font.Name = 'Segoe UI'
        Font.Style = []
        ParentFont = False
        WordWrap = True
      end
      object chtOperations: TChart
        Left = 560
        Top = 0
        Width = 524
        Height = 238
        Legend.Alignment = laBottom
        Legend.Visible = False
        SubTitle.CustomPosition = True
        SubTitle.Font.Color = clGreen
        SubTitle.Font.Style = [fsBold, fsItalic]
        SubTitle.Frame.Visible = False
        SubTitle.Left = 0
        SubTitle.Text.Strings = (
          'Gr'#225'fico carga processador (pesado)')
        SubTitle.Top = 15
        Title.Font.Color = clWindowText
        Title.Font.Height = -13
        Title.Frame.Width = 13
        Title.Text.Strings = (
          'Trabalho realizado durante o teste')
        BottomAxis.Title.Caption = 'Tempo (s)'
        LeftAxis.AxisValuesFormat = '#,##0'
        LeftAxis.Title.Caption = 'Opera'#231#245'es'
        View3D = False
        Color = clWhite
        TabOrder = 0
        DefaultCanvas = 'TGDIPlusCanvas'
        ColorPaletteIndex = 13
        object OperationsSeries: TLineSeries
          SeriesColor = clGreen
          Title = 'Opera'#231#245'es executadas'
          Brush.BackColor = clDefault
          LinePen.Width = 2
          Pointer.InflateMargins = True
          Pointer.Style = psRectangle
          XValues.Name = 'X'
          XValues.Order = loAscending
          YValues.Name = 'Y'
          YValues.Order = loNone
        end
      end
      object chtCpuHistory: TChart
        Left = 12
        Top = 0
        Width = 524
        Height = 238
        Legend.Alignment = laBottom
        MarginTop = 9
        SubTitle.CustomPosition = True
        SubTitle.Font.Color = clGreen
        SubTitle.Font.Style = [fsBold, fsItalic]
        SubTitle.Font.Shadow.Color = 10000536
        SubTitle.Left = 15
        SubTitle.Margins.Left = 11
        SubTitle.Text.Strings = (
          'Gr'#225'fico carga e intera'#231#227'o')
        SubTitle.Top = 10
        Title.Font.Color = clWindowText
        Title.Font.Height = -13
        Title.Text.Strings = (
          'Uso do processador durante o teste')
        Title.TextAlignment = taLeftJustify
        BottomAxis.Title.Caption = 'Tempo (s)'
        LeftAxis.Automatic = False
        LeftAxis.Title.Caption = 'Uso (%)'
        View3D = False
        Color = clWhite
        TabOrder = 1
        DefaultCanvas = 'TGDIPlusCanvas'
        PrintMargins = (
          15
          29
          15
          29)
        ColorPaletteIndex = 13
        object MachineCPU: TLineSeries
          SeriesColor = clBlue
          Title = 'CPU do sistema'
          Brush.BackColor = clDefault
          LinePen.Width = 2
          Pointer.InflateMargins = True
          Pointer.Style = psRectangle
          XValues.Name = 'X'
          XValues.Order = loAscending
          YValues.Name = 'Y'
          YValues.Order = loNone
        end
        object CoreCPU: TLineSeries
          SeriesColor = clRed
          Title = 'Worker Thread'
          Brush.BackColor = clDefault
          LinePen.Width = 2
          Pointer.InflateMargins = True
          Pointer.Style = psRectangle
          XValues.Name = 'X'
          XValues.Order = loAscending
          YValues.Name = 'Y'
          YValues.Order = loNone
        end
        object MainThreadCPU: TLineSeries
          SeriesColor = clGreen
          Title = 'Main Thread'
          Brush.BackColor = clDefault
          LinePen.Width = 2
          Pointer.InflateMargins = True
          Pointer.Style = psRectangle
          XValues.Name = 'X'
          XValues.Order = loAscending
          YValues.Name = 'Y'
          YValues.Order = loNone
        end
      end
      object grpTestResult: TGroupBox
        Left = 8
        Top = 244
        Width = 1092
        Height = 100
        Anchors = [akLeft, akTop, akRight]
        Caption = 'RESULTADO DO TESTE'
        TabOrder = 2
        ExplicitWidth = 1086
        object lblResultOperations: TLabel
          Left = 12
          Top = 23
          Width = 350
          Height = 26
          AutoSize = False
          Caption = 'Opera'#231#245'es executadas: --'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -16
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
        end
        object lblResultRate: TLabel
          Left = 374
          Top = 23
          Width = 350
          Height = 26
          AutoSize = False
          Caption = 'Velocidade: --'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -16
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
        end
        object lblResultDuration: TLabel
          Left = 752
          Top = 23
          Width = 290
          Height = 26
          AutoSize = False
          Caption = 'Dura'#231#227'o: --'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -16
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
        end
        object lblResultFreeze: TLabel
          Left = 12
          Top = 61
          Width = 360
          Height = 26
          AutoSize = False
          Caption = 'Maior tempo sem resposta: --'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -16
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
        end
        object lblResultMachine: TLabel
          Left = 374
          Top = 61
          Width = 350
          Height = 26
          AutoSize = False
          Caption = 'CPU do sistema: --'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -16
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
        end
        object lblResultCore: TLabel
          Left = 752
          Top = 61
          Width = 290
          Height = 26
          AutoSize = False
          Caption = 'Worker Thread: -- | Main Thread: --'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -16
          Font.Name = 'Segoe UI'
          Font.Style = []
          ParentFont = False
        end
      end
    end
    object tabDetails: TTabSheet
      Caption = 'M'#233'tricas detalhadas'
      ImageIndex = 1
      object scrollDetails: TScrollBox
        Left = 0
        Top = 0
        Width = 1112
        Height = 369
        Align = alClient
        BorderStyle = bsNone
        TabOrder = 0
        DesignSize = (
          1086
          369)
        object grpMetrics: TGroupBox
          Left = 8
          Top = 8
          Width = 1072
          Height = 188
          Anchors = [akLeft, akTop, akRight]
          Caption = 
            'Medicoes reais | CPU do processo, atraso do timer e tempo de par' +
            'ede'
          TabOrder = 0
          object lblCycle: TLabel
            Left = 16
            Top = 24
            Width = 500
            Height = 24
            AutoSize = False
            Caption = 'Ciclo: -- ms'
          end
          object lblElapsed: TLabel
            Left = 550
            Top = 24
            Width = 475
            Height = 24
            AutoSize = False
            Caption = 'Tempo total: 0 s'
          end
          object lblCpu: TLabel
            Left = 16
            Top = 54
            Width = 1008
            Height = 24
            AutoSize = False
            Caption = 'CPU: --'
          end
          object lblLag: TLabel
            Left = 16
            Top = 84
            Width = 530
            Height = 24
            AutoSize = False
            Caption = 'Tempo de resposta da tela: -- ms'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -12
            Font.Name = 'Segoe UI'
            Font.Style = []
            ParentFont = False
          end
          object lblSamples: TLabel
            Left = 550
            Top = 84
            Width = 475
            Height = 24
            AutoSize = False
            Caption = 'Amostras: 0'
          end
          object lblHeartbeat: TLabel
            Left = 16
            Top = 114
            Width = 520
            Height = 24
            AutoSize = False
            Caption = 'Pulso UI: parado'
          end
          object lblBudget: TLabel
            Left = 550
            Top = 114
            Width = 475
            Height = 24
            AutoSize = False
            Caption = 'Orcamento por ciclo: 100 ms'
          end
          object lblBottleneck: TLabel
            Left = 16
            Top = 150
            Width = 1008
            Height = 24
            AutoSize = False
            Caption = 'Gargalo: aguardando medicao'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -13
            Font.Name = 'Segoe UI'
            Font.Style = [fsBold]
            ParentFont = False
          end
        end
        object grpStages: TGroupBox
          Left = 8
          Top = 206
          Width = 1072
          Height = 184
          Anchors = [akLeft, akTop, akRight]
          Caption = 'Participacao no ciclo (%) | maior etapa destacada acima'
          TabOrder = 1
          DesignSize = (
            1072
            184)
          object lblSim: TLabel
            Left = 16
            Top = 26
            Width = 260
            Height = 24
            AutoSize = False
            Caption = 'Simulacao'
          end
          object lblCalc: TLabel
            Left = 16
            Top = 64
            Width = 260
            Height = 24
            AutoSize = False
            Caption = 'Calculos'
          end
          object lblVisual: TLabel
            Left = 16
            Top = 102
            Width = 260
            Height = 24
            AutoSize = False
            Caption = 'Atualizacao visual'
          end
          object lblDisk: TLabel
            Left = 16
            Top = 140
            Width = 260
            Height = 24
            AutoSize = False
            Caption = 'Persistencia'
          end
          object barSim: TProgressBar
            Left = 282
            Top = 27
            Width = 758
            Height = 22
            Anchors = [akLeft, akTop, akRight]
            Smooth = True
            TabOrder = 0
          end
          object barCalc: TProgressBar
            Left = 282
            Top = 65
            Width = 758
            Height = 22
            Anchors = [akLeft, akTop, akRight]
            Smooth = True
            TabOrder = 1
          end
          object barVisual: TProgressBar
            Left = 282
            Top = 103
            Width = 758
            Height = 22
            Anchors = [akLeft, akTop, akRight]
            Smooth = True
            TabOrder = 2
          end
          object barDisk: TProgressBar
            Left = 282
            Top = 141
            Width = 758
            Height = 22
            Anchors = [akLeft, akTop, akRight]
            Smooth = True
            TabOrder = 3
          end
        end
        object gridSensors: TStringGrid
          Left = 8
          Top = 408
          Width = 550
          Height = 160
          ColCount = 4
          DefaultColWidth = 130
          DefaultRowHeight = 22
          FixedCols = 0
          RowCount = 2
          TabOrder = 2
        end
        object memoHistory: TMemo
          Left = 574
          Top = 408
          Width = 506
          Height = 160
          Anchors = [akLeft, akTop, akRight]
          ReadOnly = True
          ScrollBars = ssVertical
          TabOrder = 3
        end
      end
    end
  end
  object status: TStatusBar
    Left = 0
    Top = 879
    Width = 1120
    Height = 22
    Panels = <>
    SimplePanel = True
    ExplicitTop = 862
    ExplicitWidth = 1114
  end
  object tmrWork: TTimer
    Enabled = False
    Interval = 100
    OnTimer = tmrWorkTimer
    Left = 952
    Top = 8
  end
  object tmrHeartbeat: TTimer
    Enabled = False
    Interval = 100
    OnTimer = tmrHeartbeatTimer
    Left = 1000
    Top = 8
  end
  object appEvents: TApplicationEvents
    OnMessage = appEventsMessage
    Left = 1048
    Top = 8
  end
end
