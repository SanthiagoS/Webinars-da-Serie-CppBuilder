//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "IdentificationManager.h"

#include "EmployeeRepository.h"
#include "IntegrationClient.h"

#include <System.SysUtils.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)

namespace
{
    constexpr double MinFaceConfidence = 0.80;
}

UnicodeString IdentificationFactorStatusText(TIdentificationFactorStatus Status)
{
    switch (Status)
    {
    case TIdentificationFactorStatus::Waiting:
        return L"aguardando";
    case TIdentificationFactorStatus::Detected:
        return L"detectado";
    case TIdentificationFactorStatus::Confirmed:
        return L"confirmado";
    case TIdentificationFactorStatus::Error:
        return L"erro";
    default:
        return L"desconhecido";
    }
}

TIdentificationManager::TIdentificationManager(TEmployeeRepository& Repository,
    TIntegrationClient& Integration, const TAppConfig& Config)
    : FRepository(Repository),
      FIntegration(Integration),
      FConfig(Config)
{
    ClearIdentification();
}

void TIdentificationManager::FaceDetected(int EmployeeId, double Confidence)
{
    FFaceEmployeeId = EmployeeId;
    FFaceConfidence = Confidence;
    FFaceStatus = Confidence >= MinFaceConfidence
        ? TIdentificationFactorStatus::Confirmed
        : TIdentificationFactorStatus::Detected;

    ProcessIdentification();
}

void TIdentificationManager::BleDetected(const UnicodeString& DeviceId, int RSSI)
{
    FBleDeviceId = DeviceId;
    FBleRssi = RSSI;
    FBleStatus = DeviceId.IsEmpty()
        ? TIdentificationFactorStatus::Error
        : TIdentificationFactorStatus::Detected;

    ProcessIdentification();
}

void TIdentificationManager::ProcessIdentification()
{
    if (FFaceStatus != TIdentificationFactorStatus::Confirmed ||
        FBleStatus == TIdentificationFactorStatus::Waiting ||
        FBleStatus == TIdentificationFactorStatus::Error)
    {
        return;
    }

    TEmployee Employee;
    if (!FRepository.FindById(FFaceEmployeeId, Employee))
    {
        FLastEvent = L"Funcionário detectado, mas ainda sem vínculo no repositório";
        return;
    }

    if (!Employee.BleId.IsEmpty() && SameText(Employee.BleId, FBleDeviceId))
    {
        FBleStatus = TIdentificationFactorStatus::Confirmed;
        FCurrentEmployee = Employee;
        FHasCurrentEmployee = true;
        FIdentifiedAt = Now();
        FLastEvent = L"Identificação confirmada em " + FormatDateTime(L"hh:nn:ss", FIdentifiedAt);
        FIntegration.SendEmployeeArrival(FCurrentEmployee, FConfig.TerminalName, true, true);
        return;
    }

    FBleStatus = TIdentificationFactorStatus::Error;
    FLastEvent = L"BLE detectado não corresponde ao funcionário";
}

void TIdentificationManager::ClearIdentification()
{
    FFaceStatus = TIdentificationFactorStatus::Waiting;
    FBleStatus = TIdentificationFactorStatus::Waiting;
    FFaceEmployeeId = 0;
    FFaceConfidence = 0.0;
    FBleDeviceId = L"";
    FBleRssi = 0;
    FHasCurrentEmployee = false;
    FCurrentEmployee = TEmployee();
    FIdentifiedAt = 0;
    FLastEvent = L"Nenhuma identificação realizada";
}

void TIdentificationManager::SimulateDemoIdentification()
{
    const std::vector<TEmployee> Employees = FRepository.GetAll();
    if (Employees.empty())
    {
        FLastEvent = L"Nenhum funcionário disponível para simulação administrativa";
        return;
    }

    FCurrentEmployee = Employees.front();
    FHasCurrentEmployee = true;
    FIdentifiedAt = Now();
    FFaceEmployeeId = FCurrentEmployee.Id;
    FFaceConfidence = 0.99;
    FBleDeviceId = L"";
    FBleRssi = 0;
    FFaceStatus = TIdentificationFactorStatus::Confirmed;
    FBleStatus = TIdentificationFactorStatus::Waiting;
    FLastEvent = L"Identificação administrativa em " + FormatDateTime(L"hh:nn:ss", FIdentifiedAt);
}

bool TIdentificationManager::HasCurrentEmployee() const
{
    return FHasCurrentEmployee;
}

TEmployee TIdentificationManager::CurrentEmployee() const
{
    return FCurrentEmployee;
}

TDateTime TIdentificationManager::IdentifiedAt() const
{
    return FIdentifiedAt;
}

TIdentificationFactorStatus TIdentificationManager::FaceStatus() const
{
    return FFaceStatus;
}

TIdentificationFactorStatus TIdentificationManager::BleStatus() const
{
    return FBleStatus;
}

UnicodeString TIdentificationManager::LastEvent() const
{
    return FLastEvent;
}

bool TIdentificationManager::IdentityConfirmed() const
{
    return FHasCurrentEmployee &&
        FFaceStatus == TIdentificationFactorStatus::Confirmed &&
        FBleStatus == TIdentificationFactorStatus::Confirmed;
}
