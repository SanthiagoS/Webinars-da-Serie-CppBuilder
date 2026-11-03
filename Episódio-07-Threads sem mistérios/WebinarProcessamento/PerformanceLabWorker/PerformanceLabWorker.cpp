#include <vcl.h>
#pragma hdrstop
#include <tchar.h>

USEFORM("MainForm.cpp", frmPerformanceLabWorker);

int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
    try
    {
        Application->Initialize();
        Application->MainFormOnTaskBar = true;
	Application->CreateForm(__classid(TfrmPerformanceLabWorker), &frmPerformanceLabWorker);
        Application->Run();
    }
    catch (Exception &exception)
    {
        Application->ShowException(&exception);
        return 1;
    }
    return 0;
}
