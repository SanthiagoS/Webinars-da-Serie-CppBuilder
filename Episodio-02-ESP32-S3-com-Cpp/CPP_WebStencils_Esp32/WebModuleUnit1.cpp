
//---------------------------------------------------------------------------
#include "WebModuleUnit1.h"
bool ServidorOnline = true;
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma classgroup "FMX.Controls.TControl"
#pragma resource "*.dfm"



TComponentClass WebModuleClass = __classid(TWebModule1);
//---------------------------------------------------------------------------
__fastcall TWebModule1::TWebModule1(TComponent* Owner)
	: TWebModule(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TWebModule1::WebModule1DefaultHandlerAction(TObject *Sender, TWebRequest *Request,
		  TWebResponse *Response, bool &Handled)
{

  // HEARTBEAT
	if(Request->PathInfo == "/ping")
	{
		Response->ContentType =
		  "text/plain";

		if(ServidorOnline)
		  Response->Content = "OK";
		else
		  Response->Content = "OFFLINE";

        Handled = true;

	return;
	}

	// REDIRECT INDEX
	Response->ContentType =
		"text/html; charset=utf-8";

    Response->Content =
	  "<html><body>"
      "<script>"
      "window.location='/index.html';"
	  "</script>"
      "</body></html>";

    Handled = true;
}
//---------------------------------------------------------------------------


void __fastcall TWebModule1::WebModule1PingActionAction(TObject *Sender, TWebRequest *Request,
          TWebResponse *Response, bool &Handled)
{
		Response->ContentType =
        "text/plain";

    Response->Content = "OK";

	Handled = true;
}
//---------------------------------------------------------------------------

