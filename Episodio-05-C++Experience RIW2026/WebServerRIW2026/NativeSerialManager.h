#ifndef NativeSerialManagerH
#define NativeSerialManagerH

#include <System.hpp>
#include <Winapi.Windows.hpp>

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <functional>

// =====================================================
// GERENCIADOR SERIAL NATIVO DO WINDOWS
// =====================================================

class TNativeSerialManager
{
  public:
	using TLineReceivedCallback =
		std::function<void(const String&)>;

    using TLineReceived =
    std::function<void(
        const std::string&
	)>;


  private:
	HANDLE FSerialHandle;

	std::atomic<bool> FRunning;
	std::thread FReadThread;
	std::mutex FWriteMutex;
    std::string FLineBuffer;

	void ReadLoop();

	TLineReceived FOnLineReceived;

  public:
	TNativeSerialManager();
	~TNativeSerialManager();

	bool Open(
        const String& PortName,
        DWORD BaudRate = CBR_115200
    );

    void Close();

    bool IsOpen() const;

    bool WriteLine(
        const String& Text
	);

    void SetOnLineReceived(
    TLineReceived Callback
   );

 };

#endif
