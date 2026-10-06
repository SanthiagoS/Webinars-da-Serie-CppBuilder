//---------------------------------------------------------------------------

#ifndef FaceRecognizerH
#define FaceRecognizerH
//---------------------------------------------------------------------------
#include "FaceDetector.h"

#include <string>
#include <vector>
//---------------------------------------------------------------------------

struct FaceFeature
{
    std::vector<float> Values;

    bool Empty() const
    {
        return Values.empty();
    }
};

struct FaceRecognitionResult
{
    bool Recognized = false;
    int EmployeeId = 0;
    double Score = -1.0;
};

class FaceRecognizer
{
public:
    FaceRecognizer();
    ~FaceRecognizer();

    bool Initialize(const std::string& SFaceModelPath,
        const std::string& YuNetModelPath);
    bool ExtractFeature(const unsigned char* Data, int Width, int Height,
        int Stride, const FaceRect& Bounds, FaceFeature& Feature);
    double Compare(const FaceFeature& FeatureA, const FaceFeature& FeatureB) const;

    bool IsInitialized() const;
    int FeatureSize() const;
    const char* ModelName() const;
    const char* MetricName() const;

private:
    struct BridgeRect
    {
        int X;
        int Y;
        int Width;
        int Height;
    };

    typedef void* (__cdecl *CreateFn)(const char*, const char*);
    typedef void (__cdecl *DestroyFn)(void*);
    typedef int (__cdecl *FeatureSizeFn)(void*);
    typedef bool (__cdecl *ExtractFeatureFn)(void*, const unsigned char*, int,
        int, int, const BridgeRect*, float*, int, int*);
    typedef double (__cdecl *CompareFn)(void*, const float*, int,
        const float*, int);

    void Release();

    void* FLibrary;
    void* FHandle;
    CreateFn FCreate;
    DestroyFn FDestroy;
    FeatureSizeFn FFeatureSize;
    ExtractFeatureFn FExtractFeature;
    CompareFn FCompare;
    int FCachedFeatureSize;
};

//---------------------------------------------------------------------------
#endif
