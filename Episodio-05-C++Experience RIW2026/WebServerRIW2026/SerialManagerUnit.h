//---------------------------------------------------------------------------

#ifndef SerialManagerUnitH
#define SerialManagerUnitH

//---------------------------------------------------------------------------

#include <System.Classes.hpp>
#include <System.SysUtils.hpp>

#include <memory>
#include <atomic>
#include <vector>
#include <mutex>

#include "NativeSerialManager.h"

//---------------------------------------------------------------------------

struct TConnectedDevice
{
    String PortName;
    String DeviceId;
    String DeviceName;
    String Firmware;
    String Protocol;
    String State;

    String Mcu;
    String Clock;

    String Flash;
    String Sram;
    String Eeprom;
    String Psram;

    bool Connected;

    std::unique_ptr<TNativeSerialManager> Serial;

    TConnectedDevice() : Connected(false) {}
};

class TSerialManagerUnit1 : public TDataModule
{
  __published:
    void __fastcall DataModuleCreate(TObject* Sender);
    void __fastcall DataModuleDestroy(TObject* Sender);
  private:
    std::unique_ptr<TNativeSerialManager> FSerial;

    std::atomic<bool> FConnected;
    std::atomic<ULONGLONG> FLastSeenTick;

    String FDeviceName;
    String FDeviceId;
    String FFirmware;
    String FPortName;
    String FDeviceState;

    std::string FIdentityJson;

    void ProcessSerialLine(const std::string &Line);
    std::vector<String> EnumerateSerialPorts();
    mutable std::mutex FSerialMonitorMutex;

    bool ProbePort(const String &PortName);
    std::vector<std::unique_ptr<TConnectedDevice> > FDevices;

    TConnectedDevice* FindDeviceByPort(const String &PortName);

    TConnectedDevice* FindDeviceById(const String &DeviceId);

    bool AddDeviceFromPort(const String &PortName);

    void RemoveDeviceByPort(const String &PortName);

    struct TSerialMonitorEntry
    {
        String Time;
        String Direction;
        String Port;
        String DeviceId;
        String Text;
    };

    std::vector<TSerialMonitorEntry> FSerialMonitor;

    void AddSerialMonitorEntry(const String &Direction, const String &Port,
        const String &DeviceId, const String &Text);
    String GetCurrentEventDate() const;

    bool FSignatureActive = false;

    std::vector<String> FSignatureLines;

    String FLastSignatureFile;
    bool SaveCurrentSignatureToSvg();
  public:
    __fastcall TSerialManagerUnit1(TComponent* Owner);
    String GetDevicesJson() const;
    void DeviceRemoved();
    void DeviceArrived();
    bool SendMegaDisplayMessage(const String &Text);
    bool SendMegaDisplayExpression(const String &Expression);

    bool IsConnected() const;

    String GetDeviceName() const;
    String GetDeviceId() const;
    String GetFirmware() const;
    String GetPortName() const;
    String GetDeviceState() const;
    std::string GetIdentityJson() const;

    bool SetCapabilityState(const String &Target, bool Value);
    int GetConnectedDeviceCount() const;
    void ScanAllDevices();
    bool IsDeviceConnected(const String &deviceId) const;
    String GetSerialMonitorJson() const;
    bool SendCommandToDevice(const String &DeviceId, const String &Command);
    std::vector<String> GetSignatureLines() const;
    bool IsSignatureActive() const;
    void ResetSignatureCapture();
};

//---------------------------------------------------------------------------

extern PACKAGE TSerialManagerUnit1 *SerialManagerUnit1;

//---------------------------------------------------------------------------

#endif

