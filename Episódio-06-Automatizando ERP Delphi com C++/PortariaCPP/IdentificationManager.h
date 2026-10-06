//---------------------------------------------------------------------------

#ifndef IdentificationManagerH
#define IdentificationManagerH
//---------------------------------------------------------------------------
#include <System.hpp>

#include "AppConfig.h"
#include "EmployeeModel.h"
//---------------------------------------------------------------------------

class TEmployeeRepository;
class TIntegrationClient;

enum class TIdentificationFactorStatus
{
    Waiting,
    Detected,
    Confirmed,
    Error
};

UnicodeString IdentificationFactorStatusText(TIdentificationFactorStatus Status);

class TIdentificationManager
{
public:
    TIdentificationManager(TEmployeeRepository& Repository, TIntegrationClient& Integration,
        const TAppConfig& Config);

    void FaceDetected(int EmployeeId, double Confidence);
    void BleDetected(const UnicodeString& DeviceId, int RSSI);
    void ProcessIdentification();
    void ClearIdentification();
    void SimulateDemoIdentification();

    bool HasCurrentEmployee() const;
    TEmployee CurrentEmployee() const;
    TDateTime IdentifiedAt() const;

    TIdentificationFactorStatus FaceStatus() const;
    TIdentificationFactorStatus BleStatus() const;
    UnicodeString LastEvent() const;
    bool IdentityConfirmed() const;

private:
    TEmployeeRepository& FRepository;
    TIntegrationClient& FIntegration;
    TAppConfig FConfig;

    TIdentificationFactorStatus FFaceStatus;
    TIdentificationFactorStatus FBleStatus;
    int FFaceEmployeeId;
    double FFaceConfidence;
    UnicodeString FBleDeviceId;
    int FBleRssi;
    bool FHasCurrentEmployee;
    TEmployee FCurrentEmployee;
    TDateTime FIdentifiedAt;
    UnicodeString FLastEvent;
};

//---------------------------------------------------------------------------
#endif
