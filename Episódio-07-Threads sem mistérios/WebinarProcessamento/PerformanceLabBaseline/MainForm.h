#ifndef MainFormH
#define MainFormH

#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.AppEvnts.hpp>
#include <VCLTee.Chart.hpp>
#include <VCLTee.Series.hpp>
#include <VCLTee.TeEngine.hpp>
#include <VCLTee.TeeProcs.hpp>
#include <vector>
#include <chrono>
#include <cstdint>

class TfrmPerformanceLabBaseline : public TForm
{
__published:
    TPanel *pnlWorkPerformed;
    TLabel *lblWorkTitle;
    TLabel *lblOperationsTitle;
    TLabel *lblOperationsValue;
    TLabel *lblOperationsRateTitle;
    TLabel *lblOperationsRateValue;
    TGroupBox *grpCpuUsage;
    TLabel *lblCpuMachineTitle;
    TLabel *lblCpuMachineValue;
    TProgressBar *pbCpuMachine;
    TLabel *lblCpuCoreTitle;
    TLabel *lblCpuCoreValue;
    TProgressBar *pbCpuCore;
    TLabel *lblCpuNote;
    TGroupBox *grpScreenState;
    TLabel *lblScreenState;
    TLabel *lblFreezeTitle;
    TLabel *lblFreezeTime;
    TLabel *lblFreezeNote;
    TLabel *lblFreezeScope;
    TPageControl *pagesResults;
    TTabSheet *tabOverview;
    TTabSheet *tabDetails;
    TScrollBox *scrollDetails;
    TChart *chtCpuHistory;
    TLineSeries *MachineCPU;
    TLineSeries *CoreCPU;
    TChart *chtOperations;
    TLineSeries *OperationsSeries;
    TGroupBox *grpTestResult;
    TLabel *lblResultOperations;
    TLabel *lblResultRate;
    TLabel *lblResultDuration;
    TLabel *lblResultFreeze;
    TLabel *lblResultMachine;
    TLabel *lblResultCore;
    TPanel *pnlHeader;
    TLabel *lblTitle;
    TLabel *lblSubtitle;
    TGroupBox *grpControls;
    TLabel *lblLoad;
    TTrackBar *trkLoad;
    TCheckBox *chkAuto;
    TCheckBox *chkPersist;
    TButton *btnStart;
    TButton *btnStop;
    TButton *btnReset;
    TButton *btnProbe;
    TLabel *lblProbe;
    TLabel *lblProbeLatency;
    TLabel *lblStress;
    TLabel *lblStressDuration;
    TComboBox *cmbStressDuration;
    TButton *btnStress;
    TApplicationEvents *appEvents;
    TGroupBox *grpMetrics;
    TLabel *lblCycle;
    TLabel *lblElapsed;
    TLabel *lblCpu;
    TLabel *lblLag;
    TLabel *lblSamples;
    TLabel *lblHeartbeat;
    TLabel *lblBottleneck;
    TLabel *lblBudget;
    TGroupBox *grpStages;
    TLabel *lblSim;
    TLabel *lblCalc;
    TLabel *lblVisual;
    TLabel *lblDisk;
    TProgressBar *barSim;
    TProgressBar *barCalc;
    TProgressBar *barVisual;
    TProgressBar *barDisk;
    TStringGrid *gridSensors;
    TMemo *memoHistory;
    TStatusBar *status;
    TTimer *tmrWork;
    TTimer *tmrHeartbeat;
    void __fastcall btnStartClick(TObject *Sender);
    void __fastcall btnStopClick(TObject *Sender);
    void __fastcall btnResetClick(TObject *Sender);
    void __fastcall btnProbeClick(TObject *Sender);
    void __fastcall btnStressClick(TObject *Sender);
    void __fastcall appEventsMessage(tagMSG &Msg, bool &Handled);
    void __fastcall trkLoadChange(TObject *Sender);
    void __fastcall tmrWorkTimer(TObject *Sender);
    void __fastcall tmrHeartbeatTimer(TObject *Sender);
private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point started_, cpuAt_, beatAt_;
    std::uint64_t cpuTicks_ = 0;
    std::uint64_t threadCpuTicks_ = 0;
    std::uint64_t cycles_ = 0, samples_ = 0, beats_ = 0;
    unsigned random_ = 123456789;
    unsigned processors_ = 1;
    double maxLag_ = 0.0;
    DWORD probeMessageTime_ = 0;
    DWORD maxProbeLatency_ = 0;
    bool probeMessagePending_ = false;
    double stressResult_ = 0.5;
    UnicodeString csvPath_;
    static double Milliseconds(Clock::time_point from, Clock::time_point to);
    struct TOperationSample
    {
        double ElapsedSeconds;
        std::uint64_t Operations;
        double MachinePercent;
        double CorePercent;
    };
    bool hasTestResult_ = false;
    static std::uint64_t CpuTicks();
    static std::uint64_t MainThreadTicks();
    static UnicodeString DisplayNumber(double value, const UnicodeString &format);
    void UpdateCpuIndicators(double machinePercent, double corePercent);
    void UpdateScreenState(double lagMs);
    void UpdateFreezeTime();
    void AddCpuSample(double seconds, double machinePercent, double corePercent);
    void Stop();
    void Reset();
    void Persist(int sensor, double value, double result);
public:
    __fastcall TfrmPerformanceLabBaseline(TComponent* Owner);
};

extern PACKAGE TfrmPerformanceLabBaseline *frmPerformanceLabBaseline;
#endif
