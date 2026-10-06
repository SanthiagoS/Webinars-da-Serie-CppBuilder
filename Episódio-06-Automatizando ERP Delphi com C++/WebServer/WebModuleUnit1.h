//---------------------------------------------------------------------------
#ifndef WebModuleUnit1H
#define WebModuleUnit1H
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
#include <System.Classes.hpp>
#include <Web.HTTPApp.hpp>
//---------------------------------------------------------------------------
class TWebModule1 : public TWebModule
{
__published:
    void __fastcall WebModule1DefaultHandlerAction(
        TObject *Sender,
        TWebRequest *Request,
        TWebResponse *Response,
        bool &Handled);

public:
    __fastcall TWebModule1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TWebModule1 *WebModule1;
//---------------------------------------------------------------------------
#endif

