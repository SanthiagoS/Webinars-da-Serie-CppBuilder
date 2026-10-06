//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FaceRecognizer.h"

#include <Winapi.Windows.hpp>

#include <algorithm>
//---------------------------------------------------------------------------
#pragma package(smart_init)

namespace
{
    const char* BridgeDllName()
    {
        return "FaceDetectionBridge.dll";
    }

    template <typename T>
    T ResolveProc(void* Library, const char* Name)
    {
        return reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(Library), Name));
    }
}

FaceRecognizer::FaceRecognizer()
    : FLibrary(nullptr),
      FHandle(nullptr),
      FCreate(nullptr),
      FDestroy(nullptr),
      FFeatureSize(nullptr),
      FExtractFeature(nullptr),
      FCompare(nullptr),
      FCachedFeatureSize(0)
{
}

FaceRecognizer::~FaceRecognizer()
{
    Release();
}

bool FaceRecognizer::Initialize(const std::string& SFaceModelPath,
    const std::string& YuNetModelPath)
{
    Release();

    FLibrary = LoadLibraryA(BridgeDllName());
    if (FLibrary == nullptr)
    {
        return false;
    }

    FCreate = ResolveProc<CreateFn>(FLibrary, "FaceBridge_RecognizerCreate");
    FDestroy = ResolveProc<DestroyFn>(FLibrary, "FaceBridge_RecognizerDestroy");
    FFeatureSize = ResolveProc<FeatureSizeFn>(FLibrary,
        "FaceBridge_RecognizerFeatureSize");
    FExtractFeature = ResolveProc<ExtractFeatureFn>(FLibrary,
        "FaceBridge_ExtractFeature");
    FCompare = ResolveProc<CompareFn>(FLibrary, "FaceBridge_CompareFeatures");

    if (FCreate == nullptr || FDestroy == nullptr || FFeatureSize == nullptr ||
        FExtractFeature == nullptr || FCompare == nullptr)
    {
        Release();
        return false;
    }

    FHandle = FCreate(SFaceModelPath.c_str(), YuNetModelPath.c_str());
    if (FHandle == nullptr)
    {
        Release();
        return false;
    }

    FCachedFeatureSize = FFeatureSize(FHandle);
    if (FCachedFeatureSize <= 0)
    {
        Release();
        return false;
    }

    return true;
}

bool FaceRecognizer::ExtractFeature(const unsigned char* Data, int Width,
    int Height, int Stride, const FaceRect& Bounds, FaceFeature& Feature)
{
    Feature.Values.clear();
    if (!IsInitialized() || Data == nullptr || Bounds.Empty() ||
        Width <= 0 || Height <= 0 || Stride <= 0)
    {
        return false;
    }

    BridgeRect Rect;
    Rect.X = Bounds.X;
    Rect.Y = Bounds.Y;
    Rect.Width = Bounds.Width;
    Rect.Height = Bounds.Height;

    std::vector<float> Values(FCachedFeatureSize);
    int ActualSize = 0;
    if (!FExtractFeature(FHandle, Data, Width, Height, Stride, &Rect,
        Values.data(), static_cast<int>(Values.size()), &ActualSize))
    {
        return false;
    }

    if (ActualSize <= 0 || ActualSize > static_cast<int>(Values.size()))
    {
        return false;
    }

    Values.resize(ActualSize);
    Feature.Values = Values;
    return true;
}

double FaceRecognizer::Compare(const FaceFeature& FeatureA,
    const FaceFeature& FeatureB) const
{
    if (!IsInitialized() || FeatureA.Empty() || FeatureB.Empty() ||
        FeatureA.Values.size() != FeatureB.Values.size())
    {
        return -1.0;
    }

    return FCompare(FHandle, FeatureA.Values.data(),
        static_cast<int>(FeatureA.Values.size()), FeatureB.Values.data(),
        static_cast<int>(FeatureB.Values.size()));
}

bool FaceRecognizer::IsInitialized() const
{
    return FLibrary != nullptr && FHandle != nullptr &&
        FExtractFeature != nullptr && FCompare != nullptr;
}

int FaceRecognizer::FeatureSize() const
{
    return FCachedFeatureSize;
}

const char* FaceRecognizer::ModelName() const
{
    return "OpenCV SFace / FaceRecognizerSF";
}

const char* FaceRecognizer::MetricName() const
{
    return "FR_COSINE";
}

void FaceRecognizer::Release()
{
    if (FHandle != nullptr && FDestroy != nullptr)
    {
        FDestroy(FHandle);
        FHandle = nullptr;
    }

    if (FLibrary != nullptr)
    {
        FreeLibrary(static_cast<HMODULE>(FLibrary));
        FLibrary = nullptr;
    }

    FCreate = nullptr;
    FDestroy = nullptr;
    FFeatureSize = nullptr;
    FExtractFeature = nullptr;
    FCompare = nullptr;
    FCachedFeatureSize = 0;
}
