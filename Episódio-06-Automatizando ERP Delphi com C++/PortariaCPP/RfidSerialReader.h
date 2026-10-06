#ifndef RfidSerialReaderH
#define RfidSerialReaderH
//---------------------------------------------------------------------------
#include <System.hpp>

#include <atomic>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
//---------------------------------------------------------------------------

class TRfidSerialReader
{
public:
    TRfidSerialReader();
    ~TRfidSerialReader();

    void Start(const UnicodeString& PortName, unsigned int BaudRate = 115200);
    void Stop();

    bool IsRunning() const;
    bool IsPortOpen() const;
    bool IsDeviceReady() const;

    bool TryPopTag(UnicodeString& Uid);

    // Firmware ESP32-S3 RFID 0.4.0
    // Os comandos sao apenas enfileirados aqui. A escrita na COM continua
    // sendo feita exclusivamente pela thread WorkerProc().
    bool RequestStatus();
    bool RecoverReader();
    bool RestartDevice();

private:
    std::wstring FPortName;
    unsigned int FBaudRate;

    std::atomic<bool> FStopRequested;
    std::atomic<bool> FRunning;
    std::atomic<bool> FPortOpen;
    std::atomic<bool> FDeviceReady;

    // Heartbeat do firmware 0.4.0 chega a cada ~2 s. Estes ticks permitem
    // detectar perda do protocolo e solicitar recovery sem bloquear a VCL.
    std::atomic<unsigned long long> FPortOpenedTick;
    std::atomic<unsigned long long> FLastProtocolRxTick;
    std::atomic<unsigned long long> FLastRecoveryRequestTick;

    std::thread FWorker;

    std::mutex FQueueMutex;
    std::deque<std::string> FTagQueue;

    std::mutex FCommandMutex;
    std::deque<std::string> FCommandQueue;

    void WorkerProc();
    void ProcessLine(const std::string& Line);
    void PushTag(const std::string& Uid);

    bool QueueProtocolCommand(const char* Command);
    bool TryPopCommand(std::string& Command);
    void ProcessAutomaticRecovery();

    static bool ExtractJsonString(const std::string& Line,
        const std::string& Name,
        std::string& Value);
};
//---------------------------------------------------------------------------
#endif
