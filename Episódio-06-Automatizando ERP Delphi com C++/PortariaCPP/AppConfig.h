#ifndef AppConfigH
#define AppConfigH
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------

struct TAppConfig
{
    UnicodeString TerminalName;
    UnicodeString ApiUrl;
    UnicodeString DatabasePath;
    UnicodeString FaceDataPath;
    UnicodeString AccessDatabasePath;
    UnicodeString FaceRecognitionModelPath;
    UnicodeString FaceLandmarkModelPath;
    double FaceRecognitionThreshold;

    static TAppConfig Default();
};

//---------------------------------------------------------------------------
#endif

