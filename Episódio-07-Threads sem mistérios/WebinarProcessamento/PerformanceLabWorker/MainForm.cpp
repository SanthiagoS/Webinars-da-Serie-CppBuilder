#include <vcl.h>
#pragma hdrstop

#include "MainForm.h"
#include <System.IOUtils.hpp>
#include <System.SysUtils.hpp>
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#pragma link "VCLTee.Chart"
#pragma link "VCLTee.Series"
#pragma link "VCLTee.TeEngine"
#pragma link "VCLTee.TeeProcs"
#pragma package(smart_init)
#pragma resource "*.dfm"

TfrmPerformanceLabWorker *frmPerformanceLabWorker;

namespace
{
std::uint64_t FileTimeValue(const FILETIME &value)
{
    return (static_cast<std::uint64_t>(value.dwHighDateTime) << 32) +
        value.dwLowDateTime;
}

double TickPercent(std::uint64_t ticks, double elapsedMs, unsigned processors)
{
    if (elapsedMs <= 0.0)
        return 0.0;
    return ticks / (elapsedMs * 100.0 * std::max(1u, processors));
}

double ThreadPercent(std::uint64_t ticks, double elapsedMs)
{
    if (elapsedMs <= 0.0)
        return 0.0;
    return ticks / (elapsedMs * 100.0);
}

double ClampPercent(double value)
{
    return std::max(0.0, std::min(100.0, value));
}
}

double TPerformanceWorkerThread::Milliseconds(Clock::time_point from,
    Clock::time_point to)
{
    return std::chrono::duration<double, std::milli>(to - from).count();
}

std::uint64_t TPerformanceWorkerThread::ProcessCpuTicks()
{
    FILETIME created, exited, kernel, user;
    if (!GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user))
        return 0;
    return FileTimeValue(kernel) + FileTimeValue(user);
}

std::uint64_t TPerformanceWorkerThread::ThreadCpuTicks(HANDLE thread)
{
    if (!thread)
        return 0;
    FILETIME created, exited, kernel, user;
    if (!GetThreadTimes(thread, &created, &exited, &kernel, &user))
        return 0;
    return FileTimeValue(kernel) + FileTimeValue(user);
}

std::uint64_t TPerformanceWorkerThread::ThreadCpuTicksById(DWORD threadId)
{
    HANDLE thread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, threadId);
    if (!thread)
        return 0;
    const std::uint64_t ticks = ThreadCpuTicks(thread);
    CloseHandle(thread);
    return ticks;
}

std::uint64_t TPerformanceWorkerThread::SystemActiveTicks()
{
    FILETIME idle, kernel, user;
    if (!::GetSystemTimes(&idle, &kernel, &user))
        return 0;
    const std::uint64_t kernelTicks = FileTimeValue(kernel);
    const std::uint64_t userTicks = FileTimeValue(user);
    const std::uint64_t idleTicks = FileTimeValue(idle);
    return kernelTicks + userTicks >= idleTicks ?
        kernelTicks + userTicks - idleTicks : 0;
}

void TPerformanceWorkerThread::PersistItem(const UnicodeString &path,
    std::uint64_t cycle, int sensor, double value, double result)
{
    const UTF8String line(IntToStr(static_cast<__int64>(cycle)) + L";" +
        IntToStr(sensor) + L";" + FloatToStrF(value, ffFixed, 15, 6) + L";" +
        FloatToStrF(result, ffFixed, 15, 6) + L"\r\n");
    std::unique_ptr<TFileStream> file(new TFileStream(
        path, fmOpenWrite | fmShareDenyWrite));
    file->Seek(static_cast<__int64>(0), soEnd);
    file->WriteBuffer(line.c_str(), line.Length());
    if (!FlushFileBuffers(reinterpret_cast<HANDLE>(file->Handle)))
        RaiseLastOSError();
}

bool TPerformanceWorkerThread::IsCancelled() const
{
    return Terminated || control_->CancelRequested.load();
}

void TPerformanceWorkerThread::QueueCycle(
    const std::shared_ptr<TWorkerCycleResult> &result)
{
    const std::shared_ptr<TWorkerControl> state = control_;
    const CycleCallback callback = cycleCallback_;
	TThread::Queue(nullptr, [state, callback, result]()
    {
        if (state->AcceptCallbacks.load() && callback)
            callback(result);
    });
}

void TPerformanceWorkerThread::QueueStress(
    const std::shared_ptr<TWorkerStressResult> &result)
{
    const std::shared_ptr<TWorkerControl> state = control_;
    const StressCallback callback = stressCallback_;
	TThread::Queue(nullptr, [state, callback, result]()
    {
        if (state->AcceptCallbacks.load() && callback)
            callback(result);
    });
}

void TPerformanceWorkerThread::QueueError(const UnicodeString &message)
{
    const std::shared_ptr<TWorkerControl> state = control_;
    const ErrorCallback callback = errorCallback_;
    TThread::Queue(nullptr, [state, callback, message]()
    {
        if (state->AcceptCallbacks.load() && callback)
            callback(message);
    });
}

TPerformanceWorkerThread::TPerformanceWorkerThread(
    const std::shared_ptr<TWorkerControl> &control, bool stressMode,
    int stressSeconds, double stressSeed, DWORD mainThreadId,
    CycleCallback cycleCallback, StressCallback stressCallback,
    ErrorCallback errorCallback)
    : TThread(true),
      control_(control),
      stressMode_(stressMode),
      stressSeconds_(stressSeconds),
      stressSeed_(stressSeed),
      mainThreadId_(mainThreadId),
      processors_(1),
      cycleCallback_(cycleCallback),
      stressCallback_(stressCallback),
      errorCallback_(errorCallback)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    processors_ = std::max(1u, static_cast<unsigned>(info.dwNumberOfProcessors));
    FreeOnTerminate = false;
}

void TPerformanceWorkerThread::ExecutePipeline()
{
    std::uint64_t cycles = 0;
    std::uint64_t samples = 0;
    unsigned random = 123456789;

    while (!IsCancelled())
    {
        const int cycle = static_cast<int>(++cycles);
        int level = std::max(1, std::min(20, control_->LoadLevel.load()));
        if (control_->AutoIncrement.load() && cycle > 1 &&
            (cycle - 1) % 8 == 0 && level < 20)
        {
            ++level;
            control_->LoadLevel.store(level);
        }

        const int count = level * 25;
        const auto begin = std::chrono::steady_clock::now();
        const auto processBefore = ProcessCpuTicks();
        const auto systemBefore = SystemActiveTicks();
        const auto workerBefore = ThreadCpuTicks(::GetCurrentThread());
        const auto mainBefore = ThreadCpuTicksById(mainThreadId_);

        auto result = std::make_shared<TWorkerCycleResult>();
        result->Cycle = cycle;
        result->Level = level;
        result->Values.resize(count);
        result->Results.resize(count);

        for (int sensor = 0; sensor < count; ++sensor)
        {
            random = random * 1664525u + 1013904223u;
            result->Values[sensor] = 20.0 + (random % 80000) / 1000.0;
        }
        const auto simulated = std::chrono::steady_clock::now();

        for (int sensor = 0; sensor < count; ++sensor)
        {
            double value = result->Values[sensor];
            for (int iteration = 0; iteration < level * 2000; ++iteration)
                value = std::sin(value) + std::sqrt(std::fabs(value) + 1.0) +
                    std::cos(value * 0.7 + iteration * 0.00001);
            result->Results[sensor] = value;
        }
        const auto calculated = std::chrono::steady_clock::now();

        if (control_->PersistEnabled)
            for (int sensor = 0; sensor < count && !IsCancelled(); ++sensor)
                PersistItem(control_->CsvPath, cycle, sensor + 1,
                    result->Values[sensor], result->Results[sensor]);
        const auto persisted = std::chrono::steady_clock::now();

        const double elapsedMs = Milliseconds(begin, persisted);
        const std::uint64_t processAfter = ProcessCpuTicks();
        const std::uint64_t systemAfter = SystemActiveTicks();
        const std::uint64_t workerAfter = ThreadCpuTicks(::GetCurrentThread());
        const std::uint64_t mainAfter = ThreadCpuTicksById(mainThreadId_);

        result->Samples = samples += count;
        result->SimulationMs = Milliseconds(begin, simulated);
        result->CalculationMs = Milliseconds(simulated, calculated);
        result->PersistenceMs = Milliseconds(calculated, persisted);
        result->WorkerElapsedMs = elapsedMs;
        result->SystemPercent = TickPercent(systemAfter - systemBefore, elapsedMs, processors_);
        result->ProcessPercent = TickPercent(processAfter - processBefore, elapsedMs, processors_);
        result->WorkerPercent = ThreadPercent(workerAfter - workerBefore, elapsedMs);
        result->MainThreadPercent = ThreadPercent(mainAfter - mainBefore, elapsedMs);

        if (!IsCancelled())
            QueueCycle(result);
    }
}

void TPerformanceWorkerThread::ExecuteStress()
{
    std::vector<TWorkerStressSample> samples;
    samples.reserve(24);
    samples.push_back({0.0, 0, 0.0, 0.0, 0.0, 0.0});

    const auto cpuBefore = ProcessCpuTicks();
    const auto systemBefore = SystemActiveTicks();
    const auto workerBefore = ThreadCpuTicks(::GetCurrentThread());
    const auto mainBefore = ThreadCpuTicksById(mainThreadId_);
    const auto begin = std::chrono::steady_clock::now();
    const auto deadline = begin + std::chrono::seconds(stressSeconds_);
    auto sampledAt = begin;
    auto sampledCpu = cpuBefore;
    auto sampledSystem = systemBefore;
    auto sampledWorker = workerBefore;
    auto sampledMain = mainBefore;
    std::uint64_t iterations = 0;
    double value = stressSeed_;

    auto collectSample = [&](std::chrono::steady_clock::time_point at)
    {
        const double interval = Milliseconds(sampledAt, at);
        if (interval <= 0.0)
            return;

        const std::uint64_t cpu = ProcessCpuTicks();
        const std::uint64_t system = SystemActiveTicks();
        const std::uint64_t worker = ThreadCpuTicks(::GetCurrentThread());
        const std::uint64_t main = ThreadCpuTicksById(mainThreadId_);
        samples.push_back({
            Milliseconds(begin, at) / 1000.0,
            iterations,
            TickPercent(system - sampledSystem, interval, processors_),
            TickPercent(cpu - sampledCpu, interval, processors_),
            ThreadPercent(main - sampledMain, interval),
            ThreadPercent(worker - sampledWorker, interval)
        });
        sampledAt = at;
        sampledCpu = cpu;
        sampledSystem = system;
        sampledWorker = worker;
        sampledMain = main;
    };

	// Codigo executado fora da Main Thread: a mesma carga matematica do baseline.
    do
	{
	  // Codigo executado fora da Main Thread: a mesma carga matematica do baseline.
        for (int iteration = 0; iteration < 1024; ++iteration)
            value = std::sin(value) + std::sqrt(std::fabs(value) + 1.0) +
                std::cos(value * 0.7 + iteration * 0.00001);
        iterations += 1024;
        const auto now = std::chrono::steady_clock::now();
        if (Milliseconds(sampledAt, now) >= 250.0 && now < deadline)
            collectSample(now);
        if (now >= deadline || IsCancelled())
            break;
    }
    while (true);

    const auto end = std::chrono::steady_clock::now();
    collectSample(end);
    const auto cpuAfter = ProcessCpuTicks();
    const auto systemAfter = SystemActiveTicks();
    const auto workerAfter = ThreadCpuTicks(::GetCurrentThread());
    const auto mainAfter = ThreadCpuTicksById(mainThreadId_);
    const double elapsedMs = Milliseconds(begin, end);

    auto result = std::make_shared<TWorkerStressResult>();
    result->Iterations = iterations;
    result->Value = value;
    result->ElapsedMs = elapsedMs;
    result->ProcessCpuMs = (cpuAfter - cpuBefore) / 10000.0;
    result->SystemPercent = TickPercent(systemAfter - systemBefore, elapsedMs, processors_);
    result->ProcessPercent = TickPercent(cpuAfter - cpuBefore, elapsedMs, processors_);
    result->WorkerPercent = ThreadPercent(workerAfter - workerBefore, elapsedMs);
    result->MainThreadPercent = ThreadPercent(mainAfter - mainBefore, elapsedMs);
    result->Samples = std::move(samples);

	if (!IsCancelled())
	//Quando Termina, ExecutiveStress() chama a linha abaixo:
        QueueStress(result);
}

//Trabalho pesado sendo executado na WorkerThread.
//Quando ele é chamado (Start()) o Windows começa a executar.
void __fastcall TPerformanceWorkerThread::Execute()
{
	try
    {
        if (stressMode_)
            ExecuteStress();
        else
            ExecutePipeline();
    }
    catch (Exception &error)
    {
        if (!IsCancelled())
            QueueError(error.Message);
    }
    catch (const std::exception &error)
    {
        if (!IsCancelled())
            QueueError(UnicodeString(error.what()));
    }
    catch (...)
    {
        if (!IsCancelled())
            QueueError(L"Falha desconhecida na Worker Thread.");
    }
}

double TfrmPerformanceLabWorker::Milliseconds(Clock::time_point from,
    Clock::time_point to)
{
    return std::chrono::duration<double, std::milli>(to - from).count();
}

std::uint64_t TfrmPerformanceLabWorker::CpuTicks()
{
    return TPerformanceWorkerThread::ProcessCpuTicks();
}

std::uint64_t TfrmPerformanceLabWorker::MainThreadTicks()
{
    return TPerformanceWorkerThread::ThreadCpuTicks(::GetCurrentThread());
}

std::uint64_t TfrmPerformanceLabWorker::SystemActiveTicks()
{
    return TPerformanceWorkerThread::SystemActiveTicks();
}

UnicodeString TfrmPerformanceLabWorker::DisplayNumber(double value,
    const UnicodeString &format)
{
    TFormatSettings settings = TFormatSettings::Create();
    settings.DecimalSeparator = L',';
    settings.ThousandSeparator = L'.';
    return FormatFloat(format, value, settings);
}

void TfrmPerformanceLabWorker::UpdateCpuIndicators(double systemPercent,
    double workerPercent)
{
    systemPercent = ClampPercent(systemPercent);
    workerPercent = ClampPercent(workerPercent);
    pbCpuMachine->Position = static_cast<int>(std::round(systemPercent));
    pbCpuCore->Position = static_cast<int>(std::round(workerPercent));
    lblCpuMachineValue->Caption = DisplayNumber(systemPercent, L"0") + L"%";
    lblCpuCoreValue->Caption = DisplayNumber(workerPercent, L"0") + L"%";
}

void TfrmPerformanceLabWorker::UpdateLiveCpuMetrics()
{
    const auto now = Clock::now();
    const double interval = Milliseconds(cpuAt_, now);
    if (interval <= 0.0)
        return;

    const auto cpu = CpuTicks();
    const auto main = MainThreadTicks();
    const auto system = SystemActiveTicks();
    const double processPercent = TickPercent(cpu - cpuTicks_, interval, processors_);
    const double mainPercent = ThreadPercent(main - threadCpuTicks_, interval);
    const double systemPercent = TickPercent(system - systemTicks_, interval, processors_);
    latestMainThreadPercent_ = mainPercent;
    lblCpu->Caption = L"CPU do processo: " +
        DisplayNumber(processPercent, L"0.0") + L"% | Main Thread: " +
        DisplayNumber(mainPercent, L"0.0") + L"% de 1 CPU logica | Worker Thread: " +
        DisplayNumber(latestWorkerPercent_, L"0.0") + L"%";
    cpuTicks_ = cpu;
    threadCpuTicks_ = main;
    systemTicks_ = system;
    cpuAt_ = now;
    if (!worker_)
        UpdateCpuIndicators(systemPercent, latestWorkerPercent_);
}

void TfrmPerformanceLabWorker::UpdateScreenState(double lagMs)
{
    lblScreenState->Caption = lagMs > 500 ? L"Sem resposta" :
        (lagMs >= 100 ? L"Lenta" : L"Respondendo");
    lblScreenState->Font->Color = lagMs > 500 ? clRed :
        (lagMs >= 100 ? static_cast<TColor>(0x0080C0) : clGreen);
}

void TfrmPerformanceLabWorker::UpdateFreezeTime()
{
    const double maximum = std::max(maxLag_, static_cast<double>(maxProbeLatency_));
    lblFreezeTime->Caption = DisplayNumber(maximum / 1000.0, L"0.00") + L" s";
    lblFreezeTime->Font->Color = maximum > 2000 ? clRed :
        (maximum > 500 ? static_cast<TColor>(0x0080C0) : clWindowText);
    lblFreezeTime->Font->Style = maximum > 2000 ?
        TFontStyles() << fsBold << fsUnderline : TFontStyles() << fsBold;
    lblFreezeNote->Caption = maximum > 2000 ?
        L"A tela ficou parada por varios segundos." :
        (maximum > 500 ? L"A espera ja e perceptivel." :
                         L"Sem espera prolongada registrada.");
    if (hasTestResult_)
        lblResultFreeze->Caption = L"Maior tempo sem resposta: " +
            lblFreezeTime->Caption;
}

void TfrmPerformanceLabWorker::AddCpuSample(double seconds, double systemPercent,
    double workerPercent, double mainThreadPercent)
{
    MachineCPU->AddXY(seconds, ClampPercent(systemPercent));
    CoreCPU->AddXY(seconds, ClampPercent(workerPercent));
    MainThreadCPU->AddXY(seconds, ClampPercent(mainThreadPercent));
    if (MachineCPU->Count() > 150)
    {
        MachineCPU->Delete(0);
        CoreCPU->Delete(0);
        MainThreadCPU->Delete(0);
    }
}

__fastcall TfrmPerformanceLabWorker::TfrmPerformanceLabWorker(TComponent* Owner)
    : TForm(Owner)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    processors_ = std::max(1u, static_cast<unsigned>(info.dwNumberOfProcessors));
    gridSensors->Cells[0][0] = L"Sensor";
    gridSensors->Cells[1][0] = L"Leitura";
    gridSensors->Cells[2][0] = L"Resultado";
    gridSensors->Cells[3][0] = L"Ciclo";
    Reset();
}

__fastcall TfrmPerformanceLabWorker::~TfrmPerformanceLabWorker()
{
    StopWorker();
}

void TfrmPerformanceLabWorker::StopWorker()
{
    if (!worker_)
        return;

    workerControl_->AcceptCallbacks.store(false);
    workerControl_->CancelRequested.store(true);
    worker_->Terminate();
    worker_->WaitFor();
    delete worker_;
    worker_ = nullptr;
    workerControl_.reset();
}

void TfrmPerformanceLabWorker::Stop()
{
    StopWorker();
    tmrWork->Enabled = false;
    tmrHeartbeat->Enabled = false;
    stressRunning_ = false;
    btnStart->Enabled = true;
    btnStop->Enabled = false;
    btnReset->Enabled = true;
    btnStress->Enabled = true;
    cmbStressDuration->Enabled = true;
    chkPersist->Enabled = true;
}

void TfrmPerformanceLabWorker::Reset()
{
    Stop();
    cycles_ = samples_ = beats_ = 0;
    maxLag_ = 0;
    lblCycle->Caption = L"Ciclo: -- ms";
    lblElapsed->Caption = L"Tempo total: 0 s";
    lblCpu->Caption = L"CPU do processo: -- | Main Thread: -- | Worker Thread: --";
    lblLag->Caption = L"Tempo de resposta da tela: -- ms";
    lblSamples->Caption = L"Amostras: 0";
    lblHeartbeat->Caption = L"Pulso UI: parado";
    lblBottleneck->Caption = L"Gargalo: aguardando medicao";
    lblBottleneck->Font->Color = clWindowText;
    lblBudget->Caption = L"Orcamento por ciclo: 100 ms";
    lblProbe->Caption = L"Cliques atendidos: 0";
    btnProbe->Tag = 0;
    probeMessagePending_ = false;
    maxProbeLatency_ = 0;
    lblProbeLatency->Caption = L"Resposta do botao: ultima -- ms | maior -- ms";
    lblStress->Caption = L"Teste de carga do processador: aguardando execucao.";
    hasTestResult_ = false;
    stressResult_ = 0.5;
    latestWorkerPercent_ = 0.0;
    latestMainThreadPercent_ = 0.0;
    lblOperationsValue->Caption = L"0";
    lblOperationsRateValue->Caption = L"-- milhoes/s";
    lblResultOperations->Caption = L"Operacoes executadas: --";
    lblResultRate->Caption = L"Velocidade: --";
    lblResultDuration->Caption = L"Duracao: --";
    lblResultFreeze->Caption = L"Maior tempo sem resposta: --";
    lblResultMachine->Caption = L"CPU do sistema: --";
    lblResultCore->Caption = L"Worker Thread: -- | Main Thread: --";
    MachineCPU->Clear();
    CoreCPU->Clear();
    MainThreadCPU->Clear();
    OperationsSeries->Clear();
    pbCpuMachine->Position = 0;
    pbCpuCore->Position = 0;
    lblCpuMachineValue->Caption = L"--%";
    lblCpuCoreValue->Caption = L"--%";
    UpdateScreenState(0);
    UpdateFreezeTime();
    lblSim->Caption = L"Simulacao";
    lblCalc->Caption = L"Calculos";
    lblVisual->Caption = L"Atualizacao visual";
    lblDisk->Caption = L"Persistencia";
    barSim->Position = 0;
    barCalc->Position = 0;
    barVisual->Position = 0;
    barDisk->Position = 0;
    gridSensors->RowCount = 2;
    for (int column = 0; column < 4; ++column)
        gridSensors->Cells[column][1] = L"";
    memoHistory->Clear();
    status->SimpleText = L"Pronto. Inicie; o calculo e a persistencia ocorrerao na Worker Thread.";
}

void __fastcall TfrmPerformanceLabWorker::btnStartClick(TObject *Sender)
{
    const int selectedLoad = trkLoad->Position;
    Reset();
    trkLoad->Position = selectedLoad;
    trkLoadChange(nullptr);

    try
    {
        if (chkPersist->Checked)
        {
            const UnicodeString directory = System::Ioutils::TPath::Combine(
                System::Ioutils::TPath::GetDocumentsPath(), L"PerformanceLabWorker");
            if (!ForceDirectories(directory))
                throw Exception(L"Nao foi possivel criar a pasta de resultados.");
            GUID session;
            if (FAILED(CoCreateGuid(&session)))
                throw Exception(L"Nao foi possivel criar o identificador da sessao.");
            csvPath_ = System::Ioutils::TPath::Combine(directory,
                L"sensores-" + GUIDToString(session) + L".csv");
            std::unique_ptr<TFileStream> file(new TFileStream(csvPath_, fmCreate));
            const UTF8String header("cycle;sensor;value;result\r\n");
            file->WriteBuffer(header.c_str(), header.Length());
        }

        auto state = std::make_shared<TWorkerControl>();
        state->LoadLevel.store(trkLoad->Position);
        state->AutoIncrement.store(chkAuto->Checked);
        state->PersistEnabled = chkPersist->Checked;
        state->CsvPath = csvPath_;
        workerControl_ = state;

        cpuTicks_ = CpuTicks();
        threadCpuTicks_ = MainThreadTicks();
        systemTicks_ = SystemActiveTicks();
        started_ = cpuAt_ = beatAt_ = Clock::now();

        auto onCycle = [this, state](const std::shared_ptr<TWorkerCycleResult> &result)
        {
            if (state->AcceptCallbacks.load())
                HandleCycle(result);
        };
        auto onError = [this, state](const UnicodeString &message)
        {
            if (state->AcceptCallbacks.load())
                HandleWorkerError(message);
        };

        worker_ = new TPerformanceWorkerThread(state, false, 0, stressResult_,
            GetCurrentThreadId(), onCycle, nullptr, onError);
        worker_->Start();

        btnStart->Enabled = false;
        btnStop->Enabled = true;
        btnReset->Enabled = false;
        chkPersist->Enabled = false;
        status->SimpleText = chkPersist->Checked ?
            L"CSV: " + csvPath_ +
            L" | calculo/persistencia na Worker Thread." :
            L"Calculo na Worker Thread; persistencia desativada.";
        tmrHeartbeat->Enabled = true;
    }
    catch (Exception &error)
    {
        Stop();
        status->SimpleText = L"Falha ao iniciar: " + error.Message;
    }
}

void __fastcall TfrmPerformanceLabWorker::btnStopClick(TObject *Sender)
{
    Stop();
    lblHeartbeat->Caption = L"Pulso UI: parado";
    UpdateScreenState(0);
    status->SimpleText = chkPersist->Checked ? L"Parado. CSV: " + csvPath_ :
                                               L"Parado.";
}

void __fastcall TfrmPerformanceLabWorker::btnResetClick(TObject *Sender)
{
    Reset();
}

void __fastcall TfrmPerformanceLabWorker::btnProbeClick(TObject *Sender)
{
    ++btnProbe->Tag;
    lblProbe->Caption = L"Cliques atendidos: " + IntToStr(btnProbe->Tag);
    if (probeMessagePending_)
    {
        const DWORD latency = GetTickCount() - probeMessageTime_;
        probeMessagePending_ = false;
        maxProbeLatency_ = std::max(maxProbeLatency_, latency);
        lblProbeLatency->Caption = L"Resposta do botao: ultima " +
            IntToStr(static_cast<__int64>(latency)) + L" ms | maior " +
            IntToStr(static_cast<__int64>(maxProbeLatency_)) + L" ms";
        UpdateFreezeTime();
    }
}

void __fastcall TfrmPerformanceLabWorker::appEventsMessage(tagMSG &Msg,
    bool &Handled)
{
    probeMessagePending_ = Msg.hwnd == btnProbe->Handle &&
        (Msg.message == WM_LBUTTONUP ||
         (Msg.message == WM_KEYUP && Msg.wParam == VK_SPACE));
    if (probeMessagePending_)
        probeMessageTime_ = Msg.time;
}

void __fastcall TfrmPerformanceLabWorker::btnStressClick(TObject *Sender)
{
    if (worker_ || !btnStress->Enabled)
        return;

    Stop();
    const int seconds = std::max(3, std::min(5, cmbStressDuration->ItemIndex + 3));
    stressRunning_ = true;
    btnStress->Enabled = false;
    cmbStressDuration->Enabled = false;
    btnStart->Enabled = false;
    btnReset->Enabled = false;
    btnStop->Enabled = true;
    lblStress->Caption = L"Teste na Worker Thread por " +
        IntToStr(seconds) + L" s. A Main Thread continua livre.";
    pagesResults->ActivePage = tabOverview;
    lblScreenState->Caption = L"Respondendo";
    lblScreenState->Font->Color = clGreen;
    beatAt_ = Clock::now();
    cpuTicks_ = CpuTicks();
    threadCpuTicks_ = MainThreadTicks();
    systemTicks_ = SystemActiveTicks();
    tmrHeartbeat->Enabled = true;

    try
    {
        auto state = std::make_shared<TWorkerControl>();
        workerControl_ = state;
        auto onStress = [this, state](
            const std::shared_ptr<TWorkerStressResult> &result)
        {
            if (state->AcceptCallbacks.load())
                HandleStress(result);
        };
        auto onError = [this, state](const UnicodeString &message)
        {
            if (state->AcceptCallbacks.load())
                HandleWorkerError(message);
		};
		//Aqui a Worker Thread é criada, com o nome "Start()";
		//O programa passa para a Worker thread o ID da Main thread "GetCurrentThreadId()".

        worker_ = new TPerformanceWorkerThread(state, true, seconds, stressResult_,
			GetCurrentThreadId(), nullptr, onStress, onError);
        worker_->Start();
    }
	catch (Exception &error)
    {
        Stop();
        status->SimpleText = L"Falha ao iniciar stress: " + error.Message;
    }
}

void __fastcall TfrmPerformanceLabWorker::trkLoadChange(TObject *Sender)
{
    lblLoad->Caption = L"Carga: " + IntToStr(trkLoad->Position) + L" / 20";
    if (workerControl_)
        workerControl_->LoadLevel.store(trkLoad->Position);
}

void __fastcall TfrmPerformanceLabWorker::tmrWorkTimer(TObject *Sender)
{
    // O timer permanece no DFM para tornar a comparacao visivel.
    // O trabalho pesado nao e executado aqui: ele vive na Worker Thread.
}

void __fastcall TfrmPerformanceLabWorker::tmrHeartbeatTimer(TObject *Sender)
{
    const auto now = Clock::now();
    const double lag = std::max(0.0,
        Milliseconds(beatAt_, now) - tmrHeartbeat->Interval);
    beatAt_ = now;
    maxLag_ = std::max(maxLag_, lag);
    UpdateScreenState(lag);
    UpdateFreezeTime();
    UpdateLiveCpuMetrics();
    lblLag->Caption = L"Tempo de resposta da tela: " +
        FloatToStrF(lag, ffFixed, 12, 1) + L" ms | max: " +
        FloatToStrF(maxLag_, ffFixed, 12, 1) + L" ms";
    lblHeartbeat->Caption = L"Pulso UI: " +
        IntToStr(static_cast<__int64>(++beats_));
}

void TfrmPerformanceLabWorker::HandleCycle(
    const std::shared_ptr<TWorkerCycleResult> &result)
{
    // Voltando para a Main Thread para atualizar a interface.
    if (trkLoad->Position != result->Level)
    {
        trkLoad->Position = result->Level;
        trkLoadChange(nullptr);
    }

    gridSensors->RowCount = static_cast<int>(result->Values.size()) + 1;
    const auto visualBegin = Clock::now();
    for (std::size_t sensor = 0; sensor < result->Values.size(); ++sensor)
    {
        gridSensors->Cells[0][static_cast<int>(sensor) + 1] =
            IntToStr(static_cast<int>(sensor) + 1);
        gridSensors->Cells[1][static_cast<int>(sensor) + 1] =
            FloatToStrF(result->Values[sensor], ffFixed, 12, 3);
        gridSensors->Cells[2][static_cast<int>(sensor) + 1] =
            FloatToStrF(result->Results[sensor], ffFixed, 12, 6);
        gridSensors->Cells[3][static_cast<int>(sensor) + 1] =
            IntToStr(static_cast<__int64>(result->Cycle));
        gridSensors->Repaint();
    }
    const double visualMs = Milliseconds(visualBegin, Clock::now());
    const double total = result->WorkerElapsedMs + visualMs;
    const double stages[] = {
        result->SimulationMs, result->CalculationMs, visualMs,
        result->PersistenceMs
    };
    const UnicodeString names[] = {
        L"Simulacao", L"Calculos", L"Atualizacao visual", L"Persistencia"
    };
    TLabel* labels[] = {lblSim, lblCalc, lblVisual, lblDisk};
    TProgressBar* bars[] = {barSim, barCalc, barVisual, barDisk};
    int slowest = 0;
    for (int stage = 0; stage < 4; ++stage)
    {
        if (stages[stage] > stages[slowest])
            slowest = stage;
        labels[stage]->Caption = names[stage] + L": " +
            FloatToStrF(stages[stage], ffFixed, 12, 1) + L" ms";
        bars[stage]->Position = total > 0 ?
            static_cast<int>(100 * stages[stage] / total) : 0;
    }

    cycles_ = result->Cycle;
    samples_ = result->Samples;
    latestWorkerPercent_ = result->WorkerPercent;
    latestMainThreadPercent_ = result->MainThreadPercent;
    UpdateCpuIndicators(result->SystemPercent, result->WorkerPercent);
    AddCpuSample(Milliseconds(started_, Clock::now()) / 1000.0,
        result->SystemPercent, result->WorkerPercent, result->MainThreadPercent);
    lblCpu->Caption = L"CPU do processo: " +
        DisplayNumber(result->ProcessPercent, L"0.0") +
        L"% | Main Thread: " + DisplayNumber(result->MainThreadPercent, L"0.0") +
        L"% | Worker Thread: " + DisplayNumber(result->WorkerPercent, L"0.0") + L"%";
    lblCycle->Caption = L"Ciclo (4 etapas): " +
        FloatToStrF(total, ffFixed, 12, 1) + L" ms";
    lblElapsed->Caption = L"Tempo total: " +
        FloatToStrF(Milliseconds(started_, Clock::now()) / 1000.0,
            ffFixed, 12, 1) + L" s";
    lblSamples->Caption = L"Amostras: " +
        IntToStr(static_cast<__int64>(samples_)) + L" | Ciclos: " +
        IntToStr(static_cast<__int64>(cycles_));
    lblBudget->Caption = L"Excesso sobre 100 ms: " +
        FloatToStrF(std::max(0.0, total - 100.0), ffFixed, 12, 1) + L" ms";
    lblBottleneck->Caption = L"Maior etapa: " + names[slowest] +
        L" | calculo/persistencia na Worker Thread";
    lblBottleneck->Font->Color = total > 100 ? clRed : clGreen;
    memoHistory->Lines->Add(L"Ciclo " +
        IntToStr(static_cast<__int64>(cycles_)) + L" | carga " +
        IntToStr(result->Level) + L" | " + FloatToStrF(total, ffFixed, 12, 1) +
        L" ms | " + names[slowest]);
    if (memoHistory->Lines->Count > 150)
        memoHistory->Lines->Delete(0);
}

void TfrmPerformanceLabWorker::HandleStress(
    const std::shared_ptr<TWorkerStressResult> &result)
{
    // Voltando para a Main Thread: somente aqui os componentes VCL mudam.
    stressResult_ = result->Value;
    lblOperationsValue->Caption = DisplayNumber(
        static_cast<double>(result->Iterations), L"#,##0");
    const double operationsPerSecond =
        result->Iterations / (result->ElapsedMs / 1000.0);
    lblOperationsRateValue->Caption = DisplayNumber(
        operationsPerSecond / 1000000.0, L"0.0") + L" milhoes/s";
    UpdateCpuIndicators(result->SystemPercent, result->WorkerPercent);
    latestWorkerPercent_ = result->WorkerPercent;
    latestMainThreadPercent_ = result->MainThreadPercent;
    lblCpu->Caption = L"CPU do processo: " +
        DisplayNumber(result->ProcessPercent, L"0.0") +
        L"% | Main Thread: " + DisplayNumber(result->MainThreadPercent, L"0.0") +
        L"% | Worker Thread: " + DisplayNumber(result->WorkerPercent, L"0.0") + L"%";

    MachineCPU->Clear();
    CoreCPU->Clear();
    MainThreadCPU->Clear();
    OperationsSeries->Clear();
    for (const auto &sample : result->Samples)
    {
        OperationsSeries->AddXY(sample.ElapsedSeconds,
            static_cast<double>(sample.Operations));
        if (sample.ElapsedSeconds > 0)
            AddCpuSample(sample.ElapsedSeconds, sample.SystemPercent,
                sample.WorkerPercent, sample.MainThreadPercent);
    }

    hasTestResult_ = true;
    lblResultOperations->Caption = L"Operacoes executadas: " +
        lblOperationsValue->Caption;
    lblResultRate->Caption = L"Velocidade: " +
        lblOperationsRateValue->Caption;
    lblResultDuration->Caption = L"Duracao: " +
        DisplayNumber(result->ElapsedMs / 1000.0, L"0.00") + L" s";
    lblResultMachine->Caption = L"CPU do sistema: " +
        DisplayNumber(result->SystemPercent, L"0.0") + L"%";
    lblResultCore->Caption = L"Worker Thread: " +
        DisplayNumber(result->WorkerPercent, L"0.0") +
        L"% | Main Thread: " +
        DisplayNumber(result->MainThreadPercent, L"0.0") + L"%";
    UpdateFreezeTime();
    UpdateScreenState(0);
    lblStress->Caption = L"Teste concluido na Worker Thread | CPU do processo: " +
        DisplayNumber(result->ProcessCpuMs, L"#,##0.0") +
        L" ms | resultado: " + DisplayNumber(stressResult_, L"0.00000000") +
        L". A Main Thread permaneceu disponivel.";
    status->SimpleText = L"Tela respondendo. O stress terminou na Worker Thread.";

    StopWorker();
    stressRunning_ = false;
    btnStress->Enabled = true;
    cmbStressDuration->Enabled = true;
    btnStart->Enabled = true;
    btnReset->Enabled = true;
    btnStop->Enabled = false;
}

void TfrmPerformanceLabWorker::HandleWorkerError(const UnicodeString &message)
{
    StopWorker();
    stressRunning_ = false;
    btnStress->Enabled = true;
    cmbStressDuration->Enabled = true;
    btnStart->Enabled = true;
    btnReset->Enabled = true;
    btnStop->Enabled = false;
    status->SimpleText = L"Falha na Worker Thread: " + message;
}
