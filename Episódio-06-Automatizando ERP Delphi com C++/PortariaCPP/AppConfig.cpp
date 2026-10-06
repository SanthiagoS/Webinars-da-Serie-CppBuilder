//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "AppConfig.h"

#include <System.IOUtils.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)

TAppConfig TAppConfig::Default()
{
    TAppConfig Config;
    const UnicodeString ExePath = ExtractFilePath(Application->ExeName);
    const UnicodeString ModelsPath = TPath::Combine(ExePath, L"models");
    const UnicodeString FaceModelsPath = TPath::Combine(ModelsPath, L"face");
    const UnicodeString DataPath = TPath::Combine(ExePath, L"data");

    Config.TerminalName = L"PORTARIA-01";
    Config.ApiUrl = L"http://127.0.0.1:3101/api";
    Config.DatabasePath =
        L"C:\\Users\\Public\\Documents\\Embarcadero\\Studio\\37.0\\Samples\\Data\\Employees.s3db";
    Config.FaceDataPath = TPath::Combine(DataPath, L"biometrics");
    Config.AccessDatabasePath = TPath::Combine(DataPath, L"access.s3db");
    Config.FaceRecognitionModelPath = TPath::Combine(FaceModelsPath,
        L"face_recognition_sface_2021dec.onnx");
    Config.FaceLandmarkModelPath = TPath::Combine(FaceModelsPath,
        L"face_detection_yunet_2023mar.onnx");
    Config.FaceRecognitionThreshold = 0.45;
    return Config;
}

