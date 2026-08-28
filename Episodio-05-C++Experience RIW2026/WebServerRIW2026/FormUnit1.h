
//---------------------------------------------------------------------------

#ifndef FormUnit1H
#define FormUnit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.SysUtils.hpp>

#include <Winapi.Windows.hpp>
#include <dbt.h>
#include <IdSSLOpenSSL.hpp>

#include <FMX.Controls.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Edit.hpp>
#include <FMX.Types.hpp>
#include <FMX.Platform.Win.hpp>

#include <IdHTTPWebBrokerBridge.hpp>
#include <FMX.Colors.hpp>
#include <FMX.Objects.hpp>
#include <FMX.Effects.hpp>

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:
	TButton *ButtonStart;
	TButton *ButtonStop;
    TButton *ButtonOpenBrowser;
    TEdit *EditPort;
	TLabel *lblWebserver;
	TPanel *pnelSuperior;
	TPanel *pnelInferior;
	TImage *imgRIW;
	TImage *imgCPP;
	TColorBox *colorBranco;
	TColorBox *colorAzulClaro;
	TLabel *lblThiagoSantos;
	TShadowEffect *ShadowEffect1;
	TShadowEffect *ShadowEffect2;

	void __fastcall ButtonStartClick(TObject *Sender);
	void __fastcall ButtonStopClick(TObject *Sender);
	void __fastcall ButtonOpenBrowserClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);

  private:
   WNDPROC FOriginalWindowProc;
   HWND FWindowHandle;

  static LRESULT CALLBACK DeviceWindowProc(
	HWND Window,
	UINT Message,
	WPARAM WParam,
	LPARAM LParam
  );

  void HandleDeviceChange(
	WPARAM EventType,
	LPARAM EventData
  );


	TIdHTTPWebBrokerBridge *FServer;
    TIdServerIOHandlerSSLOpenSSL *FSSLIOHandler;

    void __fastcall ApplicationIdle(
        System::TObject *Sender,
        bool &Done
    );

    void __fastcall StartServer();

public:
    __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif


