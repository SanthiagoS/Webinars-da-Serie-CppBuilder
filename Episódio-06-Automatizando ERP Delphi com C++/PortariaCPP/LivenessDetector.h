//---------------------------------------------------------------------------

#ifndef LivenessDetectorH
#define LivenessDetectorH
//---------------------------------------------------------------------------
#include "FaceDetector.h"

#include <vector>
//---------------------------------------------------------------------------

struct LivenessResult
{
    bool Completed;
    bool IsLive;
    float Score;

    LivenessResult()
        : Completed(false), IsLive(false), Score(0.0f)
    {
    }
};

enum class LivenessMode
{
    Prototype,
    AntiSpoofModel
};

class LivenessDetector
{
public:
    explicit LivenessDetector(LivenessMode Mode = LivenessMode::Prototype);

    void Reset();
    LivenessResult Process(const unsigned char* FrameData, int FrameWidth,
        int FrameHeight, int FrameStride, const FaceDetection& Face);

    LivenessMode Mode() const;
    bool IsModelAvailable() const;

private:
    void DebugResult(const FaceDetection& Face, const LivenessResult& Result) const;

    LivenessMode FMode;
    bool FModelAvailable;
};

//---------------------------------------------------------------------------
#endif
