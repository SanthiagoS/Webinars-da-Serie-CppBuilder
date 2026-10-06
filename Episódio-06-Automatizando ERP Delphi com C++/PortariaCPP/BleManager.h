//---------------------------------------------------------------------------

#ifndef BleManagerH
#define BleManagerH
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------

enum class TBleScanState
{
    Idle,
    Scanning,
    Error
};

class TBleManager
{
public:
    TBleManager();

    void StartScan();
    void StopScan();

    bool IsScanning() const;
    TBleScanState State() const;
    UnicodeString StatusText() const;

private:
    TBleScanState FState;
    UnicodeString FLastError;
};

//---------------------------------------------------------------------------
#endif
