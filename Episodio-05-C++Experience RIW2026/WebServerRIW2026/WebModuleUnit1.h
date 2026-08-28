 
//---------------------------------------------------------------------------
#ifndef WebModuleUnit1H
#define WebModuleUnit1H
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
#include <System.Classes.hpp>
#include <System.JSON.hpp>
#include <System.DateUtils.hpp>

#include <Web.HTTPApp.hpp>
#include <Web.Stencils.hpp>

//---------------------------------------------------------------------------
class TWebModule1 : public TWebModule
{
__published:	// IDE-managed Components
	TWebFileDispatcher *WebFileDispatcher1;
	TWebStencilsEngine *WebStencilsEngine1;
	TWebStencilsProcessor *WebStencilsProcessor1;
	void __fastcall WebModule1DefaultHandlerAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
    void __fastcall WebModuleCreate(TObject *Sender);
	void __fastcall WebModule1DeviceStatusActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1DeviceIdentityActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1MegaDisplayMessageActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1MegaDisplayExpressionActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1MegaDisplaySignatureActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1WebModule1MegaSignatureStartActionActionAction(TObject *Sender,
          TWebRequest *Request, TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1WebModule1MegaSignatureLiveActionActionAction(TObject *Sender,
          TWebRequest *Request, TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1WebModule1GallerySignaturesActionActionAction(TObject *Sender,
          TWebRequest *Request, TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1WebModule1GallerySignatureFileActionActionAction(TObject *Sender,
          TWebRequest *Request, TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1GalleryPhotosActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1GalleryPhotosListActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1GalleryPhotoFileActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1DeviceCommandActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
private:	// User declarations

     String FSerialBuffer;

    bool FDeviceConnected = false;
    String FDeviceState = L"disconnected";

    String FDeviceName;
    String FDeviceId;
    String FDeviceCategory;
    String FDeviceMCU;
    String FDeviceClock;
    String FFirmware;
    String FProtocol;
    String FPortName;

    int FFreeRam = 0;
    unsigned long FUptimeMs = 0;

    bool FLed1 = false;
    bool FLed2 = false;
    bool FLed3 = false;
    bool FRelay1 = false;
    bool FBuzzer1 = false;

    TDateTime FLastSeen;
    int FPacketsReceived = 0;

    void __fastcall ProcessSerialLine(const String& Line);
    void __fastcall ProcessIdentity(System::Json::TJSONObject* Json);
    void __fastcall ProcessStatus(System::Json::TJSONObject* Json);
    void __fastcall ProcessResult(System::Json::TJSONObject* Json);

    bool __fastcall GetJsonBool(
        System::Json::TJSONObject* Json,
        const String& Name,
        bool DefaultValue);

    String __fastcall BuildDeviceStatusJson();


public:
	__fastcall TWebModule1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TWebModule1 *WebModule1;
//---------------------------------------------------------------------------
#endif


