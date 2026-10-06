//---------------------------------------------------------------------------
#ifndef EmployeeSyncClientH
#define EmployeeSyncClientH
//---------------------------------------------------------------------------
#include <System.hpp>
#include <Vcl.ExtCtrls.hpp>

#include <atomic>
#include <mutex>

class TEmployeeRepository;

struct TEmployeeSyncStatus
{
    bool Success = false;
    int EmployeeCount = 0;
    UnicodeString Endpoint;
    UnicodeString Message;
};

class TEmployeeSyncClient
{
private:
    TTimer* FTimer;
    TEmployeeRepository* FRepository;
    UnicodeString FApiUrl;

    std::atomic_bool FBusy;
    bool FEnabled;

    std::mutex FStateMutex;
    TEmployeeSyncStatus FLastStatus;
    bool FStatusPending;

    void __fastcall TimerTick(TObject* Sender);
    void SyncAsync();
    void PublishStatus(const TEmployeeSyncStatus& Status);

public:
    TEmployeeSyncClient(
        TEmployeeRepository* Repository,
        const UnicodeString& ApiUrl);

    ~TEmployeeSyncClient();

    void Start();
    void Stop();
    void SyncNow();

    bool TryConsumeStatus(TEmployeeSyncStatus& Status);

    static UnicodeString EmployeesEndpoint(
        const UnicodeString& ApiUrl);
};
//---------------------------------------------------------------------------
#endif

