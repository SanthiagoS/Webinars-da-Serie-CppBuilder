//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FaceDetector.h"

#include <System.IOUtils.hpp>
#include <Winapi.Windows.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
//---------------------------------------------------------------------------
#pragma package(smart_init)

namespace
{
    std::string ToUtf8String(const UnicodeString& Value)
    {
        return AnsiString(Value).c_str();
    }

    std::string BridgeDllName()
    {
        const UnicodeString DllPath = TPath::Combine(
            ExtractFilePath(Application->ExeName), L"FaceDetectionBridge.dll");
        return ToUtf8String(DllPath);
    }

    std::string DetectorModelPath()
    {
        const UnicodeString ModelPath = TPath::Combine(TPath::Combine(
            TPath::Combine(ExtractFilePath(Application->ExeName), L"models"),
            L"face"), L"face_detection_yunet_2023mar.onnx");
        return ToUtf8String(ModelPath);
    }

    FaceRect ClipFaceRect(const FaceRect& Rect, int FrameWidth, int FrameHeight)
    {
        FaceRect Result = Rect;
        if (FrameWidth <= 0 || FrameHeight <= 0)
        {
            return FaceRect();
        }

        const int Right = std::min(FrameWidth, Result.X + Result.Width);
        const int Bottom = std::min(FrameHeight, Result.Y + Result.Height);
        Result.X = std::max(0, Result.X);
        Result.Y = std::max(0, Result.Y);
        Result.Width = std::max(0, Right - Result.X);
        Result.Height = std::max(0, Bottom - Result.Y);
        return Result;
    }

    FaceRect ExpandForRecognition(const FaceRect& DetectionBounds,
        int FrameWidth, int FrameHeight)
    {
        if (DetectionBounds.Empty())
        {
            return FaceRect();
        }

        const int MarginX = static_cast<int>(std::round(DetectionBounds.Width * 0.10));
        const int MarginTop = static_cast<int>(std::round(DetectionBounds.Height * 0.15));
        const int MarginBottom = static_cast<int>(std::round(DetectionBounds.Height * 0.10));

        FaceRect Expanded;
        Expanded.X = DetectionBounds.X - MarginX;
        Expanded.Y = DetectionBounds.Y - MarginTop;
        Expanded.Width = DetectionBounds.Width + (MarginX * 2);
        Expanded.Height = DetectionBounds.Height + MarginTop + MarginBottom;
        return ClipFaceRect(Expanded, FrameWidth, FrameHeight);
    }

    template <typename T>
    T ResolveProc(void* Library, const char* Name)
    {
        return reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(Library), Name));
    }
}

FaceDetector::FaceDetector()
    : FLibrary(nullptr),
      FHandle(nullptr),
      FCreate(nullptr),
      FDestroy(nullptr),
      FDetect(nullptr),
      FDetectEx(nullptr),
      FDraw(nullptr),
      FConfidenceThreshold(0.88f),
      FMinFaceSizePixels(80),
      FMinFaceSizeRatio(0.12),
      FLastDebugStats()
{
}

FaceDetector::~FaceDetector()
{
    Release();
}

bool FaceDetector::Initialize()
{
    Release();

    const std::string DllPath = BridgeDllName();
    FLibrary = LoadLibraryExA(DllPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (FLibrary == nullptr)
    {
        char Message[512];
        std::snprintf(Message, sizeof(Message),
            "FaceDetector Initialize: LoadLibrary failed path=%s error=%lu\n",
            DllPath.c_str(), GetLastError());
        OutputDebugStringA(Message);
        return false;
    }

    FCreate = ResolveProc<CreateFn>(FLibrary, "FaceBridge_Create");
    FDestroy = ResolveProc<DestroyFn>(FLibrary, "FaceBridge_Destroy");
    FDetect = ResolveProc<DetectFn>(FLibrary, "FaceBridge_Detect");
    FDetectEx = ResolveProc<DetectExFn>(FLibrary, "FaceBridge_DetectEx");
    FDraw = ResolveProc<DrawFn>(FLibrary, "FaceBridge_Draw");

    if (FCreate == nullptr || FDestroy == nullptr ||
        FDetect == nullptr || FDraw == nullptr)
    {
        OutputDebugStringA("FaceDetector Initialize: required bridge export not found\n");
        Release();
        return false;
    }

    const std::string ModelPath = DetectorModelPath();
    FHandle = FCreate(ModelPath.c_str(), FMinFaceSizePixels, FMinFaceSizeRatio);
    if (FHandle == nullptr)
    {
        char Message[512];
        std::snprintf(Message, sizeof(Message),
            "FaceDetector Initialize: FaceBridge_Create failed model=%s\n",
            ModelPath.c_str());
        OutputDebugStringA(Message);
        Release();
        return false;
    }

    OutputDebugStringA("FaceDetector Initialize: FaceDetectorYN/YuNet ready\n");
    return true;
}

std::vector<FaceDetection> FaceDetector::Detect(const unsigned char* Data,
    int Width, int Height, int Stride)
{
    std::vector<FaceDetection> Result;
    FLastDebugStats = FaceDetectionDebugStats();
    if (!IsInitialized() || Data == nullptr ||
        Width <= 0 || Height <= 0 || Stride <= 0)
    {
        return Result;
    }

    const int MaxFaces = 8;
    BridgeDetection RawDetections[MaxFaces] = {};
    int Count = 0;
    if (FDetectEx != nullptr)
    {
        BridgeDebugStats DebugStats = {};
        Count = FDetectEx(FHandle, Data, Width, Height, Stride,
            RawDetections, MaxFaces, &DebugStats);
        FLastDebugStats.RawDetections = DebugStats.RawDetections;
        FLastDebugStats.ValidDetections = DebugStats.ValidDetections;
    }
    else
    {
        Count = FDetect(FHandle, Data, Width, Height, Stride,
            RawDetections, MaxFaces);
        FLastDebugStats.RawDetections = -1;
        FLastDebugStats.ValidDetections = std::max(0, std::min(Count, MaxFaces));
    }

    const int ValidCount = std::max(0, std::min(Count, MaxFaces));
    FLastDebugStats.ValidDetections = std::max(0,
        std::min(FLastDebugStats.ValidDetections, MaxFaces));
    Result.reserve(ValidCount);
    for (int Index = 0; Index < ValidCount; ++Index)
    {
        FaceDetection Detection;
        Detection.DetectionBounds.X = RawDetections[Index].X;
        Detection.DetectionBounds.Y = RawDetections[Index].Y;
        Detection.DetectionBounds.Width = RawDetections[Index].Width;
        Detection.DetectionBounds.Height = RawDetections[Index].Height;
        Detection.DetectionBounds = ClipFaceRect(Detection.DetectionBounds, Width, Height);
        Detection.RecognitionBounds = ExpandForRecognition(
            Detection.DetectionBounds, Width, Height);
        Detection.Bounds = Detection.DetectionBounds;
        Detection.Confidence = RawDetections[Index].Confidence;
        Detection.RightEye.X = RawDetections[Index].RightEyeX;
        Detection.RightEye.Y = RawDetections[Index].RightEyeY;
        Detection.LeftEye.X = RawDetections[Index].LeftEyeX;
        Detection.LeftEye.Y = RawDetections[Index].LeftEyeY;
        Detection.Nose.X = RawDetections[Index].NoseX;
        Detection.Nose.Y = RawDetections[Index].NoseY;
        Detection.RightMouth.X = RawDetections[Index].RightMouthX;
        Detection.RightMouth.Y = RawDetections[Index].RightMouthY;
        Detection.LeftMouth.X = RawDetections[Index].LeftMouthX;
        Detection.LeftMouth.Y = RawDetections[Index].LeftMouthY;
        Result.push_back(Detection);
    }

    return Result;
}

FaceDetectionDebugStats FaceDetector::LastDebugStats() const
{
    return FLastDebugStats;
}

void FaceDetector::DrawDetections(unsigned char* Data, int Width, int Height,
    int Stride, const std::vector<FaceDetection>& Detections)
{
    if (!IsInitialized() || Data == nullptr || Detections.empty() ||
        Width <= 0 || Height <= 0 || Stride <= 0)
    {
        return;
    }

    const int MaxFaces = 8;
    BridgeDetection RawDetections[MaxFaces] = {};
    const int Count = std::min(static_cast<int>(Detections.size()), MaxFaces);
    for (int Index = 0; Index < Count; ++Index)
    {
        RawDetections[Index].X = Detections[Index].DetectionBounds.X;
        RawDetections[Index].Y = Detections[Index].DetectionBounds.Y;
        RawDetections[Index].Width = Detections[Index].DetectionBounds.Width;
        RawDetections[Index].Height = Detections[Index].DetectionBounds.Height;
        RawDetections[Index].Confidence = Detections[Index].Confidence;
    }

    FDraw(FHandle, Data, Width, Height, Stride, RawDetections, Count);
}

bool FaceDetector::IsInitialized() const
{
    return FLibrary != nullptr && FHandle != nullptr &&
        FDetect != nullptr && FDraw != nullptr;
}

float FaceDetector::ConfidenceThreshold() const
{
    return FConfidenceThreshold;
}

int FaceDetector::MinFaceSizePixels() const
{
    return FMinFaceSizePixels;
}

double FaceDetector::MinFaceSizeRatio() const
{
    return FMinFaceSizeRatio;
}

const char* FaceDetector::AlgorithmName() const
{
    return "OpenCV FaceDetectorYN / YuNet";
}

void FaceDetector::Release()
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
    FDetect = nullptr;
    FDetectEx = nullptr;
    FDraw = nullptr;
    FLastDebugStats = FaceDetectionDebugStats();
}
