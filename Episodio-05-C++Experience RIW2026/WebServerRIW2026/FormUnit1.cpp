
//---------------------------------------------------------------------------
#include "SerialManagerUnit.h"
#include <fmx.h>
#include <FMX.Platform.hpp>
#include <FMX.Platform.Win.hpp>
#include <Winapi.Windows.hpp>
#include <Winapi.ShellAPI.hpp>
#include <IdSSLOpenSSL.hpp>

#pragma hdrstop

#include "FormUnit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"

TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner),
    FOriginalWindowProc(nullptr),
	FWindowHandle(nullptr),
	FServer(nullptr),
    FSSLIOHandler(nullptr)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonStartClick(TObject *Sender)
{
   try
    {
        StartServer();
    }
    catch (const Exception& E)
    {
        ShowMessage(
            L"Erro ao iniciar servidor: " +
            E.Message
        );

        ButtonStart->Enabled = true;
        ButtonStop->Enabled = false;
        ButtonOpenBrowser->Enabled = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonStopClick(TObject *Sender)
{
   try
    {
        if (FServer && FServer->Active)
        {
            FServer->Active = false;
        }

        ButtonStart->Enabled = true;
        ButtonStop->Enabled = false;
        ButtonOpenBrowser->Enabled = false;
    }
    catch (const Exception& E)
    {
        ShowMessage(
            L"Erro ao parar servidor: " +
            E.Message
        );
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonOpenBrowserClick(TObject *Sender)
{
  if (!FServer || !FServer->Active)
    {
        ShowMessage(
            L"Inicie o servidor antes de abrir o navegador."
        );

        return;
    }

    const String url =
        L"http://localhost:" +
        EditPort->Text;

    ShellExecuteW(
        nullptr,
        L"open",
        url.c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ApplicationIdle(TObject *Sender, bool &Done)
{
  ButtonStart->Enabled = !FServer->Active;
  ButtonStop->Enabled = FServer->Active;
  EditPort->Enabled = !FServer->Active;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::StartServer()
{
    if (!FServer)
        return;

    if (FServer->Active)
        return;

    int port = StrToIntDef(EditPort->Text, 8080);

    FServer->DefaultPort = port;
    FServer->Active = true;

    ButtonStart->Enabled = false;
    ButtonStop->Enabled = true;
    ButtonOpenBrowser->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
    // ==================================================
    // INICIALIZAÇÃO DO WEBSERVER
    // ==================================================

	FServer = new TIdHTTPWebBrokerBridge(this);

	::Fmx::Forms::Application->OnIdle =
		ApplicationIdle;

	 ButtonStart->Enabled = true;
	 ButtonStop ->Enabled = false;
	 ButtonOpenBrowser->Enabled = false;

      auto* platformWindow =
        Fmx::Platform::Win::WindowHandleToPlatform(
            Handle
        );

    if (platformWindow == nullptr)
    {
        ::OutputDebugStringW(
            L"[RIW] Janela nativa FMX não encontrada"
        );

        return;
    }

    FWindowHandle = platformWindow->Wnd;

    if (FWindowHandle == nullptr)
    {
        ::OutputDebugStringW(
            L"[RIW] HWND do formulário inválido"
        );

        return;
    }

    ::SetWindowLongPtrW(
        FWindowHandle,
        GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this)
    );

    ::SetLastError(0);

    FOriginalWindowProc =
        reinterpret_cast<WNDPROC>(
            ::SetWindowLongPtrW(
                FWindowHandle,
                GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(
                    &TForm1::DeviceWindowProc
                )
            )
        );

    if (
        FOriginalWindowProc == nullptr &&
        ::GetLastError() != 0
    )
    {
        const String message =
            L"[RIW] Falha ao instalar monitor USB. Erro: " +
            IntToStr(
                static_cast<int>(
                    ::GetLastError()
                )
            );

        ::OutputDebugStringW(
            message.c_str()
        );

        return;
    }

    ::OutputDebugStringW(
        L"[RIW] Monitor USB do Windows ativado"
    );

}
//---------------------------------------------------------------------------
LRESULT CALLBACK TForm1::DeviceWindowProc(
    HWND Window,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam)
{
    TForm1* form =
        reinterpret_cast<TForm1*>(
            ::GetWindowLongPtrW(
                Window,
                GWLP_USERDATA
            )
        );

    if (
        form != nullptr &&
        Message == WM_DEVICECHANGE)
    {
        form->HandleDeviceChange(
            WParam,
            LParam
        );
    }

    if (
        form != nullptr &&
        form->FOriginalWindowProc != nullptr)
    {
        return ::CallWindowProcW(
            form->FOriginalWindowProc,
            Window,
            Message,
            WParam,
            LParam
        );
    }

    return ::DefWindowProcW(
        Window,
        Message,
        WParam,
        LParam
    );
}
//---------------------------------------------------------------------------
void TForm1::HandleDeviceChange(
    WPARAM EventType,
    LPARAM EventData)
{
   switch (EventType)
    {
        case DBT_DEVICEREMOVECOMPLETE:
        {
            ::OutputDebugStringW(
                L"[RIW] Windows detectou remoção USB"
            );

            if (SerialManagerUnit1 != nullptr)
            {
                SerialManagerUnit1->DeviceRemoved();
            }

            break;
        }

        case DBT_DEVICEARRIVAL:
        {
            ::OutputDebugStringW(
                L"[RIW] Windows detectou conexão USB"
            );

            if (SerialManagerUnit1 != nullptr)
            {
                SerialManagerUnit1->DeviceArrived();
            }

            break;
        }

        case DBT_DEVNODES_CHANGED:
        {
            ::OutputDebugStringW(
                L"[RIW] Windows alterou a lista de dispositivos"
            );

            break;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormDestroy(
    TObject *Sender)
{
    if (
        FWindowHandle != nullptr &&
        FOriginalWindowProc != nullptr)
    {
        ::SetWindowLongPtrW(
            FWindowHandle,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(
                FOriginalWindowProc
            )
        );

        ::SetWindowLongPtrW(
            FWindowHandle,
            GWLP_USERDATA,
            0
        );
    }

    FOriginalWindowProc = nullptr;
    FWindowHandle = nullptr;

    if (FServer != nullptr)
    {
        FServer->Active = false;

        delete FServer;
        FServer = nullptr;
    }
}
//---------------------------------------------------------------------------


