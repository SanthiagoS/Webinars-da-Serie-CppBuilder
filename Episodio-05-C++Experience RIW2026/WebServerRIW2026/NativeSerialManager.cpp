#include <System.SysUtils.hpp>
#include <cstdio>

#pragma hdrstop

#include "NativeSerialManager.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
TNativeSerialManager::TNativeSerialManager()
    : FSerialHandle(INVALID_HANDLE_VALUE),
      FRunning(false),
      FLineBuffer()
{
}
//---------------------------------------------------------------------------
TNativeSerialManager::~TNativeSerialManager()
{
    Close();
}
//---------------------------------------------------------------------------
bool TNativeSerialManager::IsOpen() const
{
    return FSerialHandle != INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------------
bool TNativeSerialManager::Open(
    const String& PortName,
    DWORD BaudRate)
{
    Close();

    const String deviceName =
        L"\\\\.\\" + PortName;

    FSerialHandle = ::CreateFileW(
        deviceName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (FSerialHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DCB dcb = {};
    dcb.DCBlength = sizeof(DCB);

    if (!::GetCommState(FSerialHandle, &dcb))
    {
        Close();
        return false;
    }

    dcb.BaudRate = BaudRate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;

    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;

    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = TRUE;

    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;

    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;

    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fAbortOnError = FALSE;

    if (!::SetCommState(FSerialHandle, &dcb))
    {
        Close();
        return false;
    }

    COMMTIMEOUTS timeouts = {};

    timeouts.ReadIntervalTimeout = 20;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 0;

    timeouts.WriteTotalTimeoutConstant = 500;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    if (!::SetCommTimeouts(FSerialHandle, &timeouts))
    {
        Close();
        return false;
    }

    ::SetupComm(
        FSerialHandle,
        4096,
        4096
    );

    ::PurgeComm(
        FSerialHandle,
        PURGE_RXCLEAR |
        PURGE_TXCLEAR |
        PURGE_RXABORT |
        PURGE_TXABORT
    );

	FLineBuffer.clear();
    FRunning = true;

    try
    {
        FReadThread = std::thread(
            &TNativeSerialManager::ReadLoop,
            this
        );
    }
    catch (...)
    {
        FRunning = false;
        Close();
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void TNativeSerialManager::Close()
{

	FRunning = false;

    if (FSerialHandle != INVALID_HANDLE_VALUE)
    {
        ::CancelIoEx(
            FSerialHandle,
            nullptr
        );
    }

    if (FReadThread.joinable())
    {
        FReadThread.join();
    }

    if (FSerialHandle != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(FSerialHandle);
        FSerialHandle = INVALID_HANDLE_VALUE;
	}

    FLineBuffer.clear();
}
//---------------------------------------------------------------------------
bool TNativeSerialManager::WriteLine(
    const String& Text)
{
    if (!IsOpen())
    {
        return false;
    }

    const UTF8String data =
        UTF8String(Text + L"\r\n");

    DWORD bytesWritten = 0;

    std::lock_guard<std::mutex> lock(
        FWriteMutex
    );

    const BOOL success = ::WriteFile(
        FSerialHandle,
        data.c_str(),
        static_cast<DWORD>(data.Length()),
        &bytesWritten,
        nullptr
    );

    return
        success &&
        bytesWritten ==
            static_cast<DWORD>(data.Length());
}
//---------------------------------------------------------------------------
void TNativeSerialManager::ReadLoop()
{
    char buffer[256];

    while (FRunning)
    {
        DWORD bytesRead = 0;

        const BOOL success = ::ReadFile(
            FSerialHandle,
            buffer,
            sizeof(buffer),
            &bytesRead,
            nullptr
        );

        if (!FRunning)
        {
            break;
        }

        if (!success)
        {
            const DWORD errorCode = ::GetLastError();

            if (errorCode == ERROR_OPERATION_ABORTED)
            {
                break;
            }

            const String message =
                L"[RIW] ReadFile falhou. Erro: " +
                IntToStr(static_cast<int>(errorCode));

            ::OutputDebugStringW(message.c_str());

            ::Sleep(50);
            continue;
        }

        if (bytesRead == 0)
        {
            ::Sleep(5);
            continue;
        }

        FLineBuffer.append(
            buffer,
            static_cast<std::size_t>(bytesRead)
        );

        std::size_t lineEnd = 0;

        while (
            (lineEnd = FLineBuffer.find('\n')) !=
            std::string::npos
        )
        {
            std::string line =
                FLineBuffer.substr(0, lineEnd);

            FLineBuffer.erase(
                0,
                lineEnd + 1
            );

            if (
                !line.empty() &&
                line.back() == '\r'
            )
            {
                line.pop_back();
            }

			if (line.empty())
            {
				continue;
			}

			// Callback

            if (FOnLineReceived)
            {
                FOnLineReceived(line);
            }
        }

        if (FLineBuffer.size() > 8192)
        {
            FLineBuffer.clear();

            ::OutputDebugStringA(
                "[RIW] Buffer excedido e limpo\r\n"
            );
        }
    }
}
//---------------------------------------------------------------------------
void TNativeSerialManager::SetOnLineReceived(
    TLineReceived Callback)
{
    FOnLineReceived = Callback;
}
//---------------------------------------------------------------------------
