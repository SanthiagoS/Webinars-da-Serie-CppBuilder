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

TfrmPerformanceLabBaseline *frmPerformanceLabBaseline;

double TfrmPerformanceLabBaseline::Milliseconds(Clock::time_point from, Clock::time_point to)
{
    return std::chrono::duration<double, std::milli>(to - from).count();
}

std::uint64_t TfrmPerformanceLabBaseline::CpuTicks()
{
    FILETIME created, exited, kernel, user;
    if (!GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user))
        RaiseLastOSError();
    return (static_cast<std::uint64_t>(kernel.dwHighDateTime) << 32) +
        kernel.dwLowDateTime + (static_cast<std::uint64_t>(user.dwHighDateTime) << 32) +
        user.dwLowDateTime;
}

std::uint64_t TfrmPerformanceLabBaseline::MainThreadTicks()
{
    FILETIME created, exited, kernel, user;
    if (!GetThreadTimes(GetCurrentThread(), &created, &exited, &kernel, &user))
        RaiseLastOSError();
    return (static_cast<std::uint64_t>(kernel.dwHighDateTime) << 32) +
        kernel.dwLowDateTime + (static_cast<std::uint64_t>(user.dwHighDateTime) << 32) +
        user.dwLowDateTime;
}

UnicodeString TfrmPerformanceLabBaseline::DisplayNumber(double value, const UnicodeString &format)
{
    TFormatSettings settings = TFormatSettings::Create();
    settings.DecimalSeparator = L',';
    settings.ThousandSeparator = L'.';
    return FormatFloat(format, value, settings);
}

void TfrmPerformanceLabBaseline::UpdateCpuIndicators(double machinePercent, double corePercent)
{
    machinePercent = std::max(0.0, std::min(100.0, machinePercent));
    corePercent = std::max(0.0, std::min(100.0, corePercent));
    pbCpuMachine->Position = static_cast<int>(std::round(machinePercent));
    pbCpuCore->Position = static_cast<int>(std::round(corePercent));
    lblCpuMachineValue->Caption = DisplayNumber(machinePercent, L"0") + L"%";
    lblCpuCoreValue->Caption = DisplayNumber(corePercent, L"0") + L"%";
}

void TfrmPerformanceLabBaseline::AddCpuSample(double seconds, double machinePercent, double corePercent)
{
    MachineCPU->AddXY(seconds, std::max(0.0, std::min(100.0, machinePercent)));
    CoreCPU->AddXY(seconds, std::max(0.0, std::min(100.0, corePercent)));
    if (MachineCPU->Count() > 150)
    {
        MachineCPU->Delete(0);
        CoreCPU->Delete(0);
    }
}

void TfrmPerformanceLabBaseline::UpdateScreenState(double lagMs)
{
    lblScreenState->Caption = lagMs > 500 ? L"Sem resposta" :
        (lagMs >= 100 ? L"Lenta" : L"Respondendo");
    lblScreenState->Font->Color = lagMs > 500 ? clRed :
        (lagMs >= 100 ? static_cast<TColor>(0x0080C0) : clGreen);
}

void TfrmPerformanceLabBaseline::UpdateFreezeTime()
{
    const double maximum = std::max(maxLag_, static_cast<double>(maxProbeLatency_));
    lblFreezeTime->Caption = DisplayNumber(maximum / 1000.0, L"0.00") + L" s";
    lblFreezeTime->Font->Color = maximum > 2000 ? clRed :
        (maximum > 500 ? static_cast<TColor>(0x0080C0) : clWindowText);
    lblFreezeTime->Font->Style = maximum > 2000 ?
        TFontStyles() << fsBold << fsUnderline : TFontStyles() << fsBold;
    lblFreezeNote->Caption = maximum > 2000 ? L"A tela ficou parada por v\u00e1rios segundos." :
        (maximum > 500 ? L"A espera j\u00e1 \u00e9 percept\u00edvel." : L"Sem espera prolongada registrada.");
    if (hasTestResult_)
        lblResultFreeze->Caption = L"Maior tempo sem resposta: " + lblFreezeTime->Caption;
}

__fastcall TfrmPerformanceLabBaseline::TfrmPerformanceLabBaseline(TComponent* Owner)
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

void TfrmPerformanceLabBaseline::Stop()
{
    tmrWork->Enabled = false;
    tmrHeartbeat->Enabled = false;
    btnStart->Enabled = true;
    btnStop->Enabled = false;
    btnReset->Enabled = true;
    chkPersist->Enabled = true;
}

void TfrmPerformanceLabBaseline::Reset()
{
    Stop();
    cycles_ = samples_ = beats_ = 0;
    random_ = 123456789;
    maxLag_ = 0;
    trkLoad->Position = 1;
    trkLoadChange(nullptr);
    lblCycle->Caption = L"Ciclo: -- ms";
    lblElapsed->Caption = L"Tempo total: 0 s";
    lblCpu->Caption = L"CPU: --";
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
    lblProbeLatency->Caption = L"Resposta do bot\u00e3o: \u00faltima -- ms | maior -- ms";
    lblStress->Caption = L"Teste de carga do processador: aguardando execu\u00e7\u00e3o.";
    hasTestResult_ = false;
    lblOperationsValue->Caption = L"0";
    lblOperationsRateValue->Caption = L"-- milh\u00f5es/s";
    lblResultOperations->Caption = L"Opera\u00e7\u00f5es executadas: --";
    lblResultRate->Caption = L"Velocidade: --";
    lblResultDuration->Caption = L"Dura\u00e7\u00e3o: --";
    lblResultFreeze->Caption = L"Maior tempo sem resposta: --";
    lblResultMachine->Caption = L"CPU da m\u00e1quina: --";
    lblResultCore->Caption = L"N\u00facleo principal: --";
    MachineCPU->Clear();
    CoreCPU->Clear();
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
    status->SimpleText = L"Pronto. Inicie e mova a janela para observar a responsividade.";
}

void __fastcall TfrmPerformanceLabBaseline::btnStartClick(TObject *Sender)
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
                System::Ioutils::TPath::GetDocumentsPath(), L"PerformanceLabBaseline");
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
        cpuTicks_ = CpuTicks();
        threadCpuTicks_ = MainThreadTicks();
        started_ = cpuAt_ = beatAt_ = Clock::now();
        btnStart->Enabled = false;
        btnStop->Enabled = true;
        btnReset->Enabled = false;
        chkPersist->Enabled = false;
        status->SimpleText = chkPersist->Checked ? L"CSV: " + csvPath_ : L"Persistencia desativada nesta execucao.";
        tmrHeartbeat->Enabled = true;
        tmrWork->Enabled = true;
    }
    catch (Exception &error)
    {
        Stop();
        status->SimpleText = L"Falha ao iniciar: " + error.Message;
    }
}

void __fastcall TfrmPerformanceLabBaseline::btnStopClick(TObject *Sender)
{
    Stop();
    lblHeartbeat->Caption = L"Pulso UI: parado";
    UpdateScreenState(0);
    status->SimpleText = chkPersist->Checked ? L"Parado. CSV: " + csvPath_ : L"Parado.";
}

void __fastcall TfrmPerformanceLabBaseline::btnResetClick(TObject *Sender)
{
    Reset();
}

void __fastcall TfrmPerformanceLabBaseline::btnProbeClick(TObject *Sender)
{
    ++btnProbe->Tag;
    lblProbe->Caption = L"Cliques atendidos: " + IntToStr(btnProbe->Tag);
    if (probeMessagePending_)
    {
        const DWORD latency = GetTickCount() - probeMessageTime_;
        probeMessagePending_ = false;
        maxProbeLatency_ = std::max(maxProbeLatency_, latency);
        lblProbeLatency->Caption = L"Resposta do bot\u00e3o: \u00faltima " +
            IntToStr(static_cast<__int64>(latency)) + L" ms | maior " +
            IntToStr(static_cast<__int64>(maxProbeLatency_)) + L" ms";
        UpdateFreezeTime();
    }
}

void __fastcall TfrmPerformanceLabBaseline::appEventsMessage(tagMSG &Msg, bool &Handled)
{
    probeMessagePending_ = Msg.hwnd == btnProbe->Handle &&
        (Msg.message == WM_LBUTTONUP ||
         (Msg.message == WM_KEYUP && Msg.wParam == VK_SPACE));
    if (probeMessagePending_)
        probeMessageTime_ = Msg.time;
}

void __fastcall TfrmPerformanceLabBaseline::btnStressClick(TObject *Sender)
{
    if (!btnStress->Enabled)
        return;
    Stop();
    const int seconds = std::max(3, std::min(5, cmbStressDuration->ItemIndex + 3));
    btnStress->Enabled = false;
    cmbStressDuration->Enabled = false;
    btnStart->Enabled = false;
    btnReset->Enabled = false;
    lblStress->Caption = L"Teste de carga do processador: a tela ficar\u00e1 sem resposta por " +
        IntToStr(seconds) + L" s. Clique em Testar resposta.";
    pagesResults->ActivePage = tabOverview;
    lblScreenState->Caption = L"Sem resposta";
    lblScreenState->Font->Color = clRed;
    grpScreenState->Repaint();
    lblStress->Repaint();
    beatAt_ = Clock::now();
    tmrHeartbeat->Enabled = true;
    try
    {
        std::vector<TOperationSample> operationSamples;
        operationSamples.reserve(24);
        operationSamples.push_back({0.0, 0, 0.0, 0.0});
        const auto cpuBefore = CpuTicks();
        const auto threadBefore = MainThreadTicks();
        const auto begin = Clock::now();
        const auto deadline = begin + std::chrono::seconds(seconds);
        auto sampledAt = begin;
        auto sampledCpu = cpuBefore;
        auto sampledThread = threadBefore;
        std::uint64_t iterations = 0;
        double value = stressResult_;
        auto collectSample = [&](Clock::time_point at, std::uint64_t cpu, std::uint64_t thread)
        {
            const double interval = Milliseconds(sampledAt, at);
            if (interval <= 0)
                return;
            operationSamples.push_back({Milliseconds(begin, at) / 1000.0, iterations,
                (cpu - sampledCpu) / (interval * 100.0 * processors_),
                (thread - sampledThread) / (interval * 100.0)});
            sampledAt = at;
            sampledCpu = cpu;
            sampledThread = thread;
        };
        // Durante o bloqueio real da Main Thread a interface não pode
        // atualizar. Coletamos as métricas em memória e desenhamos o
        // histórico quando a thread principal volta a responder.
        do
        {
            for (int iteration = 0; iteration < 1024; ++iteration)
                value = std::sin(value) + std::sqrt(std::fabs(value) + 1.0) +
                    std::cos(value * 0.7 + iteration * 0.00001);
            iterations += 1024;
            const auto now = Clock::now();
            if (Milliseconds(sampledAt, now) >= 250.0 && now < deadline)
                collectSample(now, CpuTicks(), MainThreadTicks());
            if (now >= deadline)
                break;
        }
        while (true);
        const auto end = Clock::now();
        const auto cpuAfter = CpuTicks();
        const auto threadAfter = MainThreadTicks();
        collectSample(end, cpuAfter, threadAfter);
        stressResult_ = value;
        const double elapsedMs = Milliseconds(begin, end);
        const double cpuMs = (cpuAfter - cpuBefore) / 10000.0;
        const double corePercent = (threadAfter - threadBefore) / (elapsedMs * 100.0);
        const double machinePercent = cpuMs * 100.0 / (elapsedMs * processors_);
        const double operationsPerSecond = iterations / (elapsedMs / 1000.0);
        lblOperationsValue->Caption = DisplayNumber(static_cast<double>(iterations), L"#,##0");
        lblOperationsRateValue->Caption = DisplayNumber(operationsPerSecond / 1000000.0, L"0.0") + L" milh\u00f5es/s";
        UpdateCpuIndicators(machinePercent, corePercent);
        MachineCPU->Clear();
        CoreCPU->Clear();
        OperationsSeries->Clear();
        for (const auto &sample : operationSamples)
        {
            OperationsSeries->AddXY(sample.ElapsedSeconds, static_cast<double>(sample.Operations));
            if (sample.ElapsedSeconds > 0)
                AddCpuSample(sample.ElapsedSeconds, sample.MachinePercent, sample.CorePercent);
        }
        hasTestResult_ = true;
        lblResultOperations->Caption = L"Opera\u00e7\u00f5es executadas: " + lblOperationsValue->Caption;
        lblResultRate->Caption = L"Velocidade: " + lblOperationsRateValue->Caption;
        lblResultDuration->Caption = L"Dura\u00e7\u00e3o: " + DisplayNumber(elapsedMs / 1000.0, L"0.00") + L" s";
        lblResultMachine->Caption = L"CPU da m\u00e1quina: " + lblCpuMachineValue->Caption;
        lblResultCore->Caption = L"N\u00facleo principal: " + lblCpuCoreValue->Caption;
        tmrHeartbeatTimer(nullptr);
        UpdateScreenState(0);
        lblStress->Caption = L"Teste conclu\u00eddo | CPU: " + DisplayNumber(cpuMs, L"#,##0.0") +
            L" ms | resultado do c\u00e1lculo: " + DisplayNumber(stressResult_, L"0.00000000") +
            L". Os gr\u00e1ficos foram preenchidos ap\u00f3s o bloqueio.";
        status->SimpleText = L"Tela respondendo. Iniciar abre uma nova sess\u00e3o do teste cont\u00ednuo.";
    }
    catch (Exception &error)
    {
        status->SimpleText = L"Falha no teste pesado: " + error.Message;
    }
    catch (const std::exception &error)
    {
        status->SimpleText = L"Falha no teste pesado: " + UnicodeString(error.what());
    }
    UpdateScreenState(0);
    btnStress->Enabled = true;
    cmbStressDuration->Enabled = true;
    btnStart->Enabled = true;
    btnReset->Enabled = true;
    btnStop->Enabled = true;
}

void __fastcall TfrmPerformanceLabBaseline::trkLoadChange(TObject *Sender)
{
    lblLoad->Caption = L"Carga: " + IntToStr(trkLoad->Position) + L" / 20";
}

void TfrmPerformanceLabBaseline::Persist(int sensor, double value, double result)
{
    const UTF8String line(IntToStr(static_cast<__int64>(cycles_)) + L";" +
        IntToStr(sensor) + L";" + FloatToStrF(value, ffFixed, 15, 6) + L";" +
        FloatToStrF(result, ffFixed, 15, 6) + L"\r\n");
    std::unique_ptr<TFileStream> file(new TFileStream(csvPath_, fmOpenWrite | fmShareDenyWrite));
    file->Seek(static_cast<__int64>(0), soEnd);
    file->WriteBuffer(line.c_str(), line.Length());
    if (!FlushFileBuffers(reinterpret_cast<HANDLE>(file->Handle)))
        RaiseLastOSError();
}

void __fastcall TfrmPerformanceLabBaseline::tmrWorkTimer(TObject *Sender)
{
    const auto begin = Clock::now();
    try
    {
        ++cycles_;
        if (chkAuto->Checked && cycles_ > 1 && (cycles_ - 1) % 8 == 0 &&
            trkLoad->Position < trkLoad->Max)
        {
            trkLoad->Position++;
            trkLoadChange(nullptr);
        }
        const int level = trkLoad->Position;
        const int count = level * 25;
        std::vector<double> values(count), results(count);
        for (int sensor = 0; sensor < count; ++sensor)
        {
            random_ = random_ * 1664525u + 1013904223u;
            values[sensor] = 20.0 + (random_ % 80000) / 1000.0;
        }
        const auto simulated = Clock::now();
        for (int sensor = 0; sensor < count; ++sensor)
        {
            double value = values[sensor];
            for (int iteration = 0; iteration < level * 2000; ++iteration)
                value = std::sin(value) + std::sqrt(std::fabs(value) + 1.0) +
                    std::cos(value * 0.7 + iteration * 0.00001);
            results[sensor] = value;
        }
        const auto calculated = Clock::now();
        gridSensors->RowCount = count + 1;
        for (int sensor = 0; sensor < count; ++sensor)
        {
            gridSensors->Cells[0][sensor + 1] = IntToStr(sensor + 1);
            gridSensors->Cells[1][sensor + 1] = FloatToStrF(values[sensor], ffFixed, 12, 3);
            gridSensors->Cells[2][sensor + 1] = FloatToStrF(results[sensor], ffFixed, 12, 6);
            gridSensors->Cells[3][sensor + 1] = IntToStr(static_cast<__int64>(cycles_));
            gridSensors->Repaint();
        }
        const auto painted = Clock::now();
        if (chkPersist->Checked)
            for (int sensor = 0; sensor < count; ++sensor)
                Persist(sensor + 1, values[sensor], results[sensor]);
        const auto persisted = Clock::now();
        samples_ += count;
        const double stages[] = {
            Milliseconds(begin, simulated), Milliseconds(simulated, calculated),
            Milliseconds(calculated, painted), Milliseconds(painted, persisted)
        };
        const UnicodeString names[] = {L"Simulacao", L"Calculos", L"Atualizacao visual", L"Persistencia"};
        TLabel* labels[] = {lblSim, lblCalc, lblVisual, lblDisk};
        TProgressBar* bars[] = {barSim, barCalc, barVisual, barDisk};
        const double total = Milliseconds(begin, persisted);
        int slowest = 0;
        for (int stage = 0; stage < 4; ++stage)
        {
            if (stages[stage] > stages[slowest])
                slowest = stage;
            labels[stage]->Caption = names[stage] + L": " +
                FloatToStrF(stages[stage], ffFixed, 12, 1) + L" ms";
            bars[stage]->Position = total > 0 ? static_cast<int>(100 * stages[stage] / total) : 0;
        }
        const auto cpu = CpuTicks();
        const auto threadCpu = MainThreadTicks();
        const double interval = Milliseconds(cpuAt_, persisted);
        const double corePercent = interval > 0 ? (cpu - cpuTicks_) / (interval * 100.0) : 0;
        const double mainThreadPercent = interval > 0 ?
            (threadCpu - threadCpuTicks_) / (interval * 100.0) : 0;
        threadCpuTicks_ = threadCpu;
        UpdateCpuIndicators(corePercent / processors_, mainThreadPercent);
        AddCpuSample(Milliseconds(started_, persisted) / 1000.0,
            corePercent / processors_, mainThreadPercent);
        cpuTicks_ = cpu;
        cpuAt_ = persisted;
        lblCpu->Caption = L"CPU: " + FloatToStrF(corePercent / processors_, ffFixed, 8, 1) +
            L"% da maquina | " + FloatToStrF(corePercent, ffFixed, 8, 1) + L"% de 1 nucleo";
        lblCycle->Caption = L"Ciclo (4 etapas): " + FloatToStrF(total, ffFixed, 12, 1) + L" ms";
        lblElapsed->Caption = L"Tempo total: " + FloatToStrF(Milliseconds(started_, persisted) / 1000, ffFixed, 12, 1) + L" s";
        lblSamples->Caption = L"Amostras: " + IntToStr(static_cast<__int64>(samples_)) +
            L" | Ciclos: " + IntToStr(static_cast<__int64>(cycles_));
        lblBudget->Caption = L"Excesso sobre 100 ms: " +
            FloatToStrF(std::max(0.0, total - 100.0), ffFixed, 12, 1) + L" ms";
        lblBottleneck->Caption = L"Maior etapa: " + names[slowest] +
            L" | Todas disputam a Main Thread";
        lblBottleneck->Font->Color = total > 100 ? clRed : clGreen;
        memoHistory->Lines->Add(L"Ciclo " + IntToStr(static_cast<__int64>(cycles_)) +
            L" | carga " + IntToStr(level) + L" | " + FloatToStrF(total, ffFixed, 12, 1) +
            L" ms | " + names[slowest]);
        if (memoHistory->Lines->Count > 150)
            memoHistory->Lines->Delete(0);
    }
    catch (Exception &error)
    {
        Stop();
        status->SimpleText = L"Execucao interrompida: " + error.Message;
    }
    catch (const std::exception &error)
    {
        Stop();
        status->SimpleText = L"Execucao interrompida: " + UnicodeString(error.what());
    }
}

void __fastcall TfrmPerformanceLabBaseline::tmrHeartbeatTimer(TObject *Sender)
{
    const auto now = Clock::now();
    const double lag = std::max(0.0, Milliseconds(beatAt_, now) - tmrHeartbeat->Interval);
    beatAt_ = now;
    maxLag_ = std::max(maxLag_, lag);
    UpdateScreenState(lag);
    UpdateFreezeTime();
    lblLag->Caption = L"Tempo de resposta da tela: " + FloatToStrF(lag, ffFixed, 12, 1) +
        L" ms | max: " + FloatToStrF(maxLag_, ffFixed, 12, 1) + L" ms";
    lblHeartbeat->Caption = L"Pulso UI: " + IntToStr(static_cast<__int64>(++beats_));
}

