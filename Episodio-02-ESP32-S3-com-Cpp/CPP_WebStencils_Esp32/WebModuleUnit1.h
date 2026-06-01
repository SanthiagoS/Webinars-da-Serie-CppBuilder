 
//---------------------------------------------------------------------------
#ifndef WebModuleUnit1H
#define WebModuleUnit1H
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
#include <System.Classes.hpp>
#include <Web.HTTPApp.hpp>
#include <Web.Stencils.hpp>
//---------------------------------------------------------------------------
class TWebModule1 : public TWebModule
{
__published:	// IDE-managed Components
	TWebStencilsEngine *WebStencilsEngine1;
	TWebStencilsProcessor *WebStencilsProcessor1;
	TWebFileDispatcher *WebFileDispatcher1;
	void __fastcall WebModule1DefaultHandlerAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
	void __fastcall WebModule1PingActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled);
private:	// User declarations

public:		// User declarations
	__fastcall TWebModule1(TComponent* Owner);

};

//---------------------------------------------------------------------------
extern PACKAGE TWebModule1 *WebModule1;
extern bool ServidorOnline;
//---------------------------------------------------------------------------
#endif


