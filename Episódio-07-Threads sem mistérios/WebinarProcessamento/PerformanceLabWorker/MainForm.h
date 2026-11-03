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
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

struct TWorkerCycleResult
{
    int Cycle = 0;
    int Level = 1;
    std::uint64_t Samples = 0;
    std::vector<double> Values;
    std::vector<double> Results;
    double SimulationMs = 0.0;
    double CalculationMs = 0.0;
    double PersistenceMs = 0.0;
    double WorkerElapsedMs = 0.0;
    double SystemPercent = 0.0;
    double ProcessPercent = 0.0;
    double MainThreadPercent = 0.0;
    double WorkerPercent = 0.0;
};

struct TWorkerStressSample
{
    double ElapsedSeconds = 0.0;
    std::uint64_t Operations = 0;
    double SystemPercent = 0.0;
    double ProcessPercent = 0.0;
    double MainThreadPercent = 0.0;
    double WorkerPercent = 0.0;
};

struct TWorkerStressResult
{
    std::uint64_t Iterations = 0;
    double Value = 0.0;
    double ElapsedMs = 0.0;
    double ProcessCpuMs = 0.0;
    double SystemPercent = 0.0;
    double ProcessPercent = 0.0;
    double MainThreadPercent = 0.0;
    double WorkerPercent = 0.0;
    std::vector<TWorkerStressSample> Samples;
};

struct TWorkerControl
{
    std::atomic<bool> CancelRequested{false};
    std::atomic<bool> AcceptCallbacks{true};
    std::atomic<int> LoadLevel{1};
    std::atomic<bool> AutoIncrement{true};
    bool PersistEnabled = false;
    UnicodeString CsvPath;
};

class TPerformanceWorkerThread : public TThread
{
public:
    using CycleCallback = std::function<void(const std::shared_ptr<TWorkerCycleResult>&)>;
    using StressCallback = std::function<void(const std::shared_ptr<TWorkerStressResult>&)>;
    using ErrorCallback = std::function<void(const UnicodeString&)>;

public:
    using Clock = std::chrono::steady_clock;
    std::shared_ptr<TWorkerControl> control_;
    bool stressMode_;
    int stressSeconds_;
    double stressSeed_;
    DWORD mainThreadId_;
    unsigned processors_;
    CycleCallback cycleCallback_;
    StressCallback stressCallback_;
    ErrorCallback errorCallback_;

    static double Milliseconds(std::chrono::steady_clock::time_point from,
        std::chrono::steady_clock::time_point to);
    static std::uint64_t ProcessCpuTicks();
    static std::uint64_t ThreadCpuTicks(HANDLE thread);
    static std::uint64_t ThreadCpuTicksById(DWORD threadId);
    static std::uint64_t SystemActiveTicks();
    static void PersistItem(const UnicodeString &path, std::uint64_t cycle,
        int sensor, double value, double result);

private:
    bool IsCancelled() const;
    void QueueCycle(const std::shared_ptr<TWorkerCycleResult> &result);
    void QueueStress(const std::shared_ptr<TWorkerStressResult> &result);
    void QueueError(const UnicodeString &message);
    void ExecutePipeline();
    void ExecuteStress();

protected:
    void __fastcall Execute() override;

public:
    TPerformanceWorkerThread(const std::shared_ptr<TWorkerControl> &control,
        bool stressMode, int stressSeconds, double stressSeed, DWORD mainThreadId,
        CycleCallback cycleCallback, StressCallback stressCallback,
        ErrorCallback errorCallback);
};

class TfrmPerformanceLabWorker : public TForm
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
    TLineSeries *MainThreadCPU;
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
    std::uint64_t systemTicks_ = 0;
    std::uint64_t cycles_ = 0, samples_ = 0, beats_ = 0;
    unsigned processors_ = 1;
    double maxLag_ = 0.0;
    DWORD probeMessageTime_ = 0;
    DWORD maxProbeLatency_ = 0;
    bool probeMessagePending_ = false;
    bool stressRunning_ = false;
    bool hasTestResult_ = false;
    double stressResult_ = 0.5;
    double latestWorkerPercent_ = 0.0;
    double latestMainThreadPercent_ = 0.0;
    UnicodeString csvPath_;
    TPerformanceWorkerThread *worker_ = nullptr;
    std::shared_ptr<TWorkerControl> workerControl_;

    static double Milliseconds(Clock::time_point from, Clock::time_point to);
    static std::uint64_t CpuTicks();
    static std::uint64_t MainThreadTicks();
    static std::uint64_t SystemActiveTicks();
    static UnicodeString DisplayNumber(double value, const UnicodeString &format);
    void UpdateCpuIndicators(double systemPercent, double workerPercent);
    void UpdateLiveCpuMetrics();
    void UpdateScreenState(double lagMs);
    void UpdateFreezeTime();
    void AddCpuSample(double seconds, double systemPercent, double workerPercent,
        double mainThreadPercent);
    void HandleCycle(const std::shared_ptr<TWorkerCycleResult> &result);
    void HandleStress(const std::shared_ptr<TWorkerStressResult> &result);
    void HandleWorkerError(const UnicodeString &message);
    void StopWorker();
    void Stop();
    void Reset();
public:
    __fastcall TfrmPerformanceLabWorker(TComponent* Owner);
    __fastcall ~TfrmPerformanceLabWorker();
};

extern PACKAGE TfrmPerformanceLabWorker *frmPerformanceLabWorker;
#endif
