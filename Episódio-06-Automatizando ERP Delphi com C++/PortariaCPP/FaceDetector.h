//---------------------------------------------------------------------------

#ifndef FaceDetectorH
#define FaceDetectorH
//---------------------------------------------------------------------------
#include <vector>
//---------------------------------------------------------------------------

struct FaceRect
{
    int X;
    int Y;
    int Width;
    int Height;

    FaceRect()
        : X(0), Y(0), Width(0), Height(0)
    {
    }

    bool Empty() const
    {
        return Width <= 0 || Height <= 0;
    }
};

struct FacePoint
{
    float X;
    float Y;

    FacePoint()
        : X(0.0f), Y(0.0f)
    {
    }
};

struct FaceDetection
{
    FaceRect DetectionBounds;
    FaceRect RecognitionBounds;
    FaceRect Bounds;
    float Confidence;
    FacePoint RightEye;
    FacePoint LeftEye;
    FacePoint Nose;
    FacePoint RightMouth;
    FacePoint LeftMouth;
};

struct FaceDetectionDebugStats
{
    int RawDetections;
    int ValidDetections;

    FaceDetectionDebugStats()
        : RawDetections(-1), ValidDetections(0)
    {
    }
};

class FaceDetector
{
public:
    FaceDetector();
    ~FaceDetector();

    bool Initialize();
    std::vector<FaceDetection> Detect(const unsigned char* Data, int Width,
        int Height, int Stride);
    void DrawDetections(unsigned char* Data, int Width, int Height, int Stride,
        const std::vector<FaceDetection>& Detections);
    FaceDetectionDebugStats LastDebugStats() const;

    bool IsInitialized() const;
    float ConfidenceThreshold() const;
    int MinFaceSizePixels() const;
    double MinFaceSizeRatio() const;
    const char* AlgorithmName() const;

private:
    struct BridgeDetection
    {
        int X;
        int Y;
        int Width;
        int Height;
        float Confidence;
        float RightEyeX;
        float RightEyeY;
        float LeftEyeX;
        float LeftEyeY;
        float NoseX;
        float NoseY;
        float RightMouthX;
        float RightMouthY;
        float LeftMouthX;
        float LeftMouthY;
    };

    typedef void* (__cdecl *CreateFn)(const char*, int, double);
    typedef void (__cdecl *DestroyFn)(void*);
    typedef int (__cdecl *DetectFn)(void*, const unsigned char*, int, int, int,
        BridgeDetection*, int);
    struct BridgeDebugStats
    {
        int RawDetections;
        int ValidDetections;
    };
    typedef int (__cdecl *DetectExFn)(void*, const unsigned char*, int, int, int,
        BridgeDetection*, int, BridgeDebugStats*);
    typedef void (__cdecl *DrawFn)(void*, unsigned char*, int, int, int,
        const BridgeDetection*, int);

    void Release();

    void* FLibrary;
    void* FHandle;
    CreateFn FCreate;
    DestroyFn FDestroy;
    DetectFn FDetect;
    DetectExFn FDetectEx;
    DrawFn FDraw;
    float FConfidenceThreshold;
    int FMinFaceSizePixels;
    double FMinFaceSizeRatio;
    FaceDetectionDebugStats FLastDebugStats;
};

//---------------------------------------------------------------------------
#endif
