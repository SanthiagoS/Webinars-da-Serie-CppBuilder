#include <vcl.h>
#pragma hdrstop
#include <tchar.h>

USEFORM("MainForm.cpp", frmPerformanceLabBaseline);

int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
    try
    {
        Application->Initialize();
        Application->MainFormOnTaskBar = true;
        Application->CreateForm(__classid(TfrmPerformanceLabBaseline), &frmPerformanceLabBaseline);
        Application->Run();
    }
    catch (Exception &exception)
    {
        Application->ShowException(&exception);
        return 1;
    }
    return 0;
}
