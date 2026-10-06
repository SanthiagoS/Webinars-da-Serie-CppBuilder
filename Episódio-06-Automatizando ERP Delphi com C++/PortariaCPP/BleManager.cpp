//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "BleManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

TBleManager::TBleManager()
    : FState(TBleScanState::Idle)
{
}

void TBleManager::StartScan()
{
    // BLE-specific discovery will be added here without using Bluetooth clássico.
    FState = TBleScanState::Scanning;
    FLastError = L"";
}

void TBleManager::StopScan()
{
    FState = TBleScanState::Idle;
}

bool TBleManager::IsScanning() const
{
    return FState == TBleScanState::Scanning;
}

TBleScanState TBleManager::State() const
{
    return FState;
}

UnicodeString TBleManager::StatusText() const
{
    switch (FState)
    {
    case TBleScanState::Idle:
        return L"aguardando";
    case TBleScanState::Scanning:
        return L"procurando dispositivos BLE";
    case TBleScanState::Error:
        return FLastError.IsEmpty() ? L"erro" : FLastError;
    default:
        return L"desconhecido";
    }
}
