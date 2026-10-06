#ifndef ObservabilityHeartbeatH
#define ObservabilityHeartbeatH

#include <System.Classes.hpp>
#include <System.SysUtils.hpp>
#include <Vcl.ExtCtrls.hpp>

#include <atomic>
#include <mutex>

struct TBiometricCommand
{
    bool Pending = false;
    UnicodeString RequestId;
    int EmployeeId = 0;
    UnicodeString EmployeeName;
    UnicodeString Terminal;
};

class TObservabilityHeartbeat
{
private:
    TTimer* FTimer;
    UnicodeString FServerUrl;
    UnicodeString FSource;
    UnicodeString FVersion;
    std::atomic_bool FBusy;
    bool FEnabled;

    bool FCameraOnline;
    bool FFaceEngineOnline;

    std::mutex FCommandMutex;
    TBiometricCommand FEnrollmentCommand;
    TBiometricCommand FRevokeCommand;
    UnicodeString FLastEnrollmentDelivered;
    UnicodeString FLastRevokeDelivered;

    void __fastcall TimerTick(TObject* Sender);
    void SendHeartbeatAsync();
    void PollBiometricCommands();

    void SendBiometricResultAsync(
        const UnicodeString& Endpoint,
        const UnicodeString& RequestId,
        const UnicodeString& Status,
        const UnicodeString& Message,
        int Samples);

public:
    TObservabilityHeartbeat();
    ~TObservabilityHeartbeat();

    void Start();
    void Stop();
    void SendNow();

    void SendEvent(
        const UnicodeString& Level,
        const UnicodeString& Type,
        const UnicodeString& Message,
        const UnicodeString& Employee = L"",
        const UnicodeString& Result = L"",
        double DurationMs = 0.0);

    bool TryConsumeEnrollmentCommand(TBiometricCommand& Command);
    bool TryConsumeRevokeCommand(TBiometricCommand& Command);

    void SendEnrollmentProgress(
        const UnicodeString& RequestId,
        const UnicodeString& Message,
        int Samples);

    void SendEnrollmentResult(
        const UnicodeString& RequestId,
        const UnicodeString& Status,
        const UnicodeString& Message,
        int Samples);

    void SendRevokeResult(
        const UnicodeString& RequestId,
        const UnicodeString& Status,
        const UnicodeString& Message);

    void UpdateRuntimeStatus(
        bool CameraOnline,
        bool FaceEngineOnline);

    void SetServerUrl(const UnicodeString& Value);
    void SetSource(const UnicodeString& Value);
    void SetVersion(const UnicodeString& Value);
};

#endif


