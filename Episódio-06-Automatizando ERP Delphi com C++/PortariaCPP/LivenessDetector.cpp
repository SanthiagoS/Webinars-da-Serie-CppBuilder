//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "LivenessDetector.h"

#include <Winapi.Windows.hpp>

#include <cstdio>
//---------------------------------------------------------------------------
#pragma package(smart_init)

LivenessDetector::LivenessDetector(LivenessMode Mode)
    : FMode(Mode),
      FModelAvailable(false)
{
}

void LivenessDetector::Reset()
{
}

LivenessResult LivenessDetector::Process(const unsigned char* FrameData,
    int FrameWidth, int FrameHeight, int FrameStride, const FaceDetection& Face)
{
    LivenessResult Result;
    if (FrameData == nullptr || FrameWidth <= 0 || FrameHeight <= 0 ||
        FrameStride <= 0 || Face.DetectionBounds.Empty())
    {
        DebugResult(Face, Result);
        return Result;
    }

    if (FMode == LivenessMode::AntiSpoofModel && FModelAvailable)
    {
        // Placeholder for a future OpenCV DNN/ONNX anti-spoofing model.
        Result.Completed = false;
        Result.IsLive = false;
        Result.Score = 0.0f;
        DebugResult(Face, Result);
        return Result;
    }

    // Prototype mode only advances the pipeline. It is not a security decision.
    Result.Completed = true;
    Result.IsLive = false;
    Result.Score = 0.0f;
    DebugResult(Face, Result);
    return Result;
}

LivenessMode LivenessDetector::Mode() const
{
    return FMode;
}

bool LivenessDetector::IsModelAvailable() const
{
    return FModelAvailable;
}

void LivenessDetector::DebugResult(const FaceDetection& Face,
    const LivenessResult& Result) const
{
    char Buffer[320];
    std::snprintf(Buffer, sizeof(Buffer),
        "LIVENESS: mode=%s face confidence=%.3f bounds=(%d,%d,%d,%d) completed=%d isLive=%d score=%.3f\n",
        FMode == LivenessMode::Prototype ? "Prototype" : "AntiSpoofModel",
        Face.Confidence,
        Face.DetectionBounds.X, Face.DetectionBounds.Y,
        Face.DetectionBounds.Width, Face.DetectionBounds.Height,
        Result.Completed ? 1 : 0,
        Result.IsLive ? 1 : 0,
        Result.Score);
    OutputDebugStringA(Buffer);
}
