
//---------------------------------------------------------------------------

#include <fmx.h>
#ifdef _WIN32
#include <tchar.h>
#endif
#pragma hdrstop
#include <System.StartUpCopy.hpp>
#include <Web.WebReq.hpp>
#ifdef USEPACKAGES
#pragma link "IndySystem.bpi"
#pragma link "IndyCore.bpi"
#pragma link "IndyProtocols.bpi"
#else
#pragma comment(lib, "IndySystem")
#pragma comment(lib, "IndyCore")
#pragma comment(lib, "IndyProtocols")
#endif
#pragma link "IdHTTPWebBrokerBridge"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

USEFORM("WebModuleUnit1.cpp", WebModule1); /* TWebModule: File Type */
USEFORM("FormUnit1.cpp", Form1);
USEFORM("SerialManagerUnit.cpp", SerialManagerUnit1); /* TDataModule: File Type */
//---------------------------------------------------------------------------
extern PACKAGE TComponentClass WebModuleClass;

extern "C" int FMXmain()
{
    try
    {
        if (WebRequestHandler() != nullptr)
        {
            WebRequestHandler()->WebModuleClass =
                WebModuleClass;
        }

        Application->Initialize();

        Application->CreateForm(__classid(TSerialManagerUnit1), &SerialManagerUnit1);
		Application->CreateForm(__classid(TForm1), &Form1);
		Application->Run();
    }
    catch (Exception &exception)
    {
        Sysutils::ShowException(
            &exception,
            System::ExceptAddr()
        );
    }

    return 0;
}
//---------------------------------------------------------------------------



