//---------------------------------------------------------------------------

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/objdetect/face.hpp>

#include <Windows.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <memory>
#include <vector>
//---------------------------------------------------------------------------

struct FaceBridgeDetection
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

struct FaceBridgeDebugStats
{
    int RawDetections;
    int ValidDetections;
};


static cv::Mat BuildDetectionEnhancedFrame(const cv::Mat& FrameBgr)
{
    if (FrameBgr.empty())
        return cv::Mat();

    // Contraluz é comum em webcams USB externas. Equalizamos somente
    // a luminância local para ajudar o YuNet a localizar a face.
    // O frame original continua sendo usado para reconhecimento.
    cv::Mat Lab;
    cv::cvtColor(FrameBgr, Lab, cv::COLOR_BGR2Lab);

    std::vector<cv::Mat> Channels;
    cv::split(Lab, Channels);
    if (Channels.size() != 3)
        return FrameBgr.clone();

    cv::Ptr<cv::CLAHE> Clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    Clahe->apply(Channels[0], Channels[0]);

    cv::merge(Channels, Lab);

    cv::Mat Enhanced;
    cv::cvtColor(Lab, Enhanced, cv::COLOR_Lab2BGR);
    return Enhanced;
}

class BridgeDetector
{
public:
    bool Initialize(const char* YuNetModelPath, int MinFaceSizePixels,
        double MinFaceSizeRatio)
    {
        if (YuNetModelPath == nullptr)
        {
            return false;
        }

        FMinFaceSizePixels = MinFaceSizePixels;
        FMinFaceSizeRatio = MinFaceSizeRatio;
        FDetector = cv::FaceDetectorYN::create(YuNetModelPath, "",
            cv::Size(320, 320), FScoreThreshold, FNmsThreshold, FTopK);

        // v0.4.4 - segundo detector usado SOMENTE quando o detector
        // principal não encontra nenhuma face válida. O score menor
        // serve apenas para LOCALIZAÇÃO; identificação continua usando
        // os thresholds/FaceKey/liveness existentes.
        FFallbackDetector = cv::FaceDetectorYN::create(YuNetModelPath, "",
            cv::Size(320, 320), FFallbackScoreThreshold,
            FNmsThreshold, FTopK);

        return !FDetector.empty() && !FFallbackDetector.empty();
    }

    int Detect(const unsigned char* Data, int Width, int Height, int Stride,
        FaceBridgeDetection* Output, int Capacity)
    {
        return DetectEx(Data, Width, Height, Stride, Output, Capacity, nullptr);
    }

    int DetectEx(const unsigned char* Data, int Width, int Height, int Stride,
        FaceBridgeDetection* Output, int Capacity, FaceBridgeDebugStats* DebugStats)
    {
        if (DebugStats != nullptr)
        {
            DebugStats->RawDetections = 0;
            DebugStats->ValidDetections = 0;
        }

        if (Data == nullptr || Output == nullptr || Capacity <= 0 ||
            Width <= 0 || Height <= 0 || Stride <= 0 || FDetector.empty())
        {
            return 0;
        }

        cv::Mat FrameBgra(Height, Width, CV_8UC4, const_cast<unsigned char*>(Data),
            static_cast<size_t>(Stride));
        cv::Mat FrameBgr;
        cv::cvtColor(FrameBgra, FrameBgr, cv::COLOR_BGRA2BGR);

        const int MinDimension = std::min(Width, Height);
        const int MinFaceSize = std::max(FMinFaceSizePixels,
            static_cast<int>(MinDimension * FMinFaceSizeRatio));
        const cv::Rect FrameBounds(0, 0, Width, Height);

        auto AppendFaces =
            [&](const cv::Mat& Faces, int& Count)
            {
                if (Faces.empty())
                    return;

                for (int Row = 0; Row < Faces.rows && Count < Capacity; ++Row)
                {
                    const float* Face = Faces.ptr<float>(Row);
                    const float Confidence = Face[14];

                    cv::Rect Candidate(
                        static_cast<int>(std::round(Face[0])),
                        static_cast<int>(std::round(Face[1])),
                        static_cast<int>(std::round(Face[2])),
                        static_cast<int>(std::round(Face[3])));

                    Candidate = Candidate & FrameBounds;
                    DebugCandidate(Width, Height, Candidate, Confidence);

                    if (!IsValidFace(Candidate, Confidence, MinFaceSize,
                            FFallbackActive ? FFallbackScoreThreshold
                                            : FScoreThreshold))
                    {
                        continue;
                    }

                    Output[Count].X = Candidate.x;
                    Output[Count].Y = Candidate.y;
                    Output[Count].Width = Candidate.width;
                    Output[Count].Height = Candidate.height;
                    Output[Count].Confidence = Confidence;
                    Output[Count].RightEyeX = Face[4];
                    Output[Count].RightEyeY = Face[5];
                    Output[Count].LeftEyeX = Face[6];
                    Output[Count].LeftEyeY = Face[7];
                    Output[Count].NoseX = Face[8];
                    Output[Count].NoseY = Face[9];
                    Output[Count].RightMouthX = Face[10];
                    Output[Count].RightMouthY = Face[11];
                    Output[Count].LeftMouthX = Face[12];
                    Output[Count].LeftMouthY = Face[13];
                    ++Count;
                }
            };

        int Count = 0;
        int RawCount = 0;

        // 1) detector normal, frame original
        FFallbackActive = false;
        FDetector->setInputSize(FrameBgr.size());

        cv::Mat Faces;
        FDetector->detect(FrameBgr, Faces);
        RawCount += Faces.empty() ? 0 : Faces.rows;
        AppendFaces(Faces, Count);

        // 2) fallback apenas se nenhuma face válida foi localizada.
        if (Count == 0)
        {
            const cv::Mat Enhanced =
                BuildDetectionEnhancedFrame(FrameBgr);

            if (!Enhanced.empty())
            {
                FFallbackActive = true;
                FFallbackDetector->setInputSize(Enhanced.size());

                cv::Mat FallbackFaces;
                FFallbackDetector->detect(Enhanced, FallbackFaces);

                const int FallbackRaw =
                    FallbackFaces.empty() ? 0 : FallbackFaces.rows;

                RawCount += FallbackRaw;

                if (FallbackRaw > 0)
                {
                    char Buffer[160];
                    std::snprintf(Buffer, sizeof(Buffer),
                        "FACE fallback: frame=%dx%d raw=%d threshold=%.2f\n",
                        Width, Height, FallbackRaw,
                        FFallbackScoreThreshold);
                    OutputDebugStringA(Buffer);
                }

                AppendFaces(FallbackFaces, Count);
            }
        }

        FFallbackActive = false;

        if (DebugStats != nullptr)
        {
            DebugStats->RawDetections = RawCount;
            DebugStats->ValidDetections = Count;
        }

        return Count;
    }

    void Draw(unsigned char* Data, int Width, int Height, int Stride,
        const FaceBridgeDetection* Detections, int Count)
    {
        if (Data == nullptr || Detections == nullptr || Count <= 0 ||
            Width <= 0 || Height <= 0 || Stride <= 0)
        {
            return;
        }

        cv::Mat Frame(Height, Width, CV_8UC4, Data, static_cast<size_t>(Stride));
        const cv::Rect FrameBounds(0, 0, Width, Height);
        for (int Index = 0; Index < Count; ++Index)
        {
            cv::Rect Bounds(Detections[Index].X, Detections[Index].Y,
                Detections[Index].Width, Detections[Index].Height);
            Bounds = Bounds & FrameBounds;
            if (Bounds.width > 0 && Bounds.height > 0)
            {
                cv::rectangle(Frame, Bounds, cv::Scalar(80, 255, 80, 255), 2);
            }
        }
    }

private:
    static void DebugCandidate(int FrameWidth, int FrameHeight,
        const cv::Rect& Bounds, float Confidence)
    {
        const double Ratio = Bounds.height > 0
            ? static_cast<double>(Bounds.width) / Bounds.height
            : 0.0;
        char Buffer[256];
        std::snprintf(Buffer, sizeof(Buffer),
            "FACE candidate: frame: %dx%d face: x=%d y=%d w=%d h=%d ratio=%.3f confidence=%.3f\n",
            FrameWidth, FrameHeight, Bounds.x, Bounds.y, Bounds.width,
            Bounds.height, Ratio, Confidence);
        OutputDebugStringA(Buffer);
    }

    bool IsValidFace(const cv::Rect& Bounds, float Confidence,
        int MinFaceSize, float RequiredScore) const
    {
        if (Confidence < RequiredScore || Bounds.width < MinFaceSize ||
            Bounds.height < MinFaceSize)
        {
            return false;
        }

        const double AspectRatio = static_cast<double>(Bounds.width) / Bounds.height;
        return AspectRatio >= FMinAspectRatio && AspectRatio <= FMaxAspectRatio;
    }

    cv::Ptr<cv::FaceDetectorYN> FDetector;
    cv::Ptr<cv::FaceDetectorYN> FFallbackDetector;

    // Detector normal.
    const float FScoreThreshold = 0.82f;

    // v0.4.4 - fallback de localização para webcam USB/contraluz.
    // Não altera o threshold de comparação de identidade.
    const float FFallbackScoreThreshold = 0.70f;

    bool FFallbackActive = false;

    const float FNmsThreshold = 0.30f;
    const int FTopK = 5000;
    const double FMinAspectRatio = 0.65;
    const double FMaxAspectRatio = 1.45;
    int FMinFaceSizePixels = 80;
    double FMinFaceSizeRatio = 0.12;
};

struct FaceBridgeRect
{
    int X;
    int Y;
    int Width;
    int Height;
};

class BridgeRecognizer
{
public:
    bool Initialize(const char* SFaceModelPath, const char* YuNetModelPath)
    {
        if (SFaceModelPath == nullptr || YuNetModelPath == nullptr)
        {
            return false;
        }

        FRecognizer = cv::FaceRecognizerSF::create(SFaceModelPath, "");

        FDetector = cv::FaceDetectorYN::create(YuNetModelPath, "",
            cv::Size(320, 320), 0.82f, 0.3f, 5000);

        FFallbackDetector = cv::FaceDetectorYN::create(YuNetModelPath, "",
            cv::Size(320, 320), 0.70f, 0.3f, 5000);

        return !FRecognizer.empty() &&
            !FDetector.empty() &&
            !FFallbackDetector.empty();
    }

    int FeatureSize() const
    {
        return 128;
    }

    bool ExtractFeature(const unsigned char* Data, int Width, int Height,
        int Stride, const FaceBridgeRect* Bounds, float* Feature,
        int FeatureCapacity, int* OutFeatureSize)
    {
        if (Data == nullptr || Bounds == nullptr || Feature == nullptr ||
            Width <= 0 || Height <= 0 || Stride <= 0 ||
            FeatureCapacity < FeatureSize() || FRecognizer.empty() ||
            FDetector.empty() || FFallbackDetector.empty())
        {
            return false;
        }

        cv::Mat FrameBgra(Height, Width, CV_8UC4,
            const_cast<unsigned char*>(Data), static_cast<size_t>(Stride));
        cv::Mat FrameBgr;
        cv::cvtColor(FrameBgra, FrameBgr, cv::COLOR_BGRA2BGR);

        cv::Mat FaceData;
        if (!FindFaceData(FrameBgr, *Bounds, FaceData))
        {
            return false;
        }

        cv::Mat Aligned;
        FRecognizer->alignCrop(FrameBgr, FaceData, Aligned);
        if (Aligned.empty())
        {
            return false;
        }

        cv::Mat RawFeature;
        FRecognizer->feature(Aligned, RawFeature);
        if (RawFeature.empty())
        {
            return false;
        }

        cv::Mat FeatureRow = RawFeature.reshape(1, 1);
        if (FeatureRow.cols > FeatureCapacity)
        {
            return false;
        }

        cv::Mat FeatureFloat;
        FeatureRow.convertTo(FeatureFloat, CV_32F);
        NormalizeFeature(FeatureFloat);

        std::memcpy(Feature, FeatureFloat.ptr<float>(0),
            sizeof(float) * FeatureFloat.cols);
        if (OutFeatureSize != nullptr)
        {
            *OutFeatureSize = FeatureFloat.cols;
        }
        return true;
    }

    double CompareFeatures(const float* FeatureA, int SizeA,
        const float* FeatureB, int SizeB) const
    {
        if (FeatureA == nullptr || FeatureB == nullptr ||
            SizeA <= 0 || SizeA != SizeB || FRecognizer.empty())
        {
            return -1.0;
        }

        cv::Mat A(1, SizeA, CV_32F, const_cast<float*>(FeatureA));
        cv::Mat B(1, SizeB, CV_32F, const_cast<float*>(FeatureB));
        return FRecognizer->match(A, B, cv::FaceRecognizerSF::FR_COSINE);
    }

private:
    static double IntersectionOverUnion(const cv::Rect2f& A,
        const cv::Rect2f& B)
    {
        const float X1 = std::max(A.x, B.x);
        const float Y1 = std::max(A.y, B.y);
        const float X2 = std::min(A.x + A.width, B.x + B.width);
        const float Y2 = std::min(A.y + A.height, B.y + B.height);
        const float Intersection = std::max(0.0f, X2 - X1) *
            std::max(0.0f, Y2 - Y1);
        const float Union = A.area() + B.area() - Intersection;
        return Union <= 0.0f ? 0.0 : Intersection / Union;
    }

    static void NormalizeFeature(cv::Mat& Feature)
    {
        double Norm = cv::norm(Feature, cv::NORM_L2);
        if (Norm > 0.0)
        {
            Feature /= Norm;
        }
    }

    bool FindFaceData(const cv::Mat& FrameBgr, const FaceBridgeRect& Bounds,
        cv::Mat& FaceData)
    {
        FDetector->setInputSize(FrameBgr.size());

        cv::Mat Faces;
        FDetector->detect(FrameBgr, Faces);

        // Se o detector normal não reencontrar a face para alinhamento,
        // usamos o mesmo fallback de contraste da etapa de localização.
        if (Faces.empty())
        {
            const cv::Mat Enhanced =
                BuildDetectionEnhancedFrame(FrameBgr);

            if (!Enhanced.empty())
            {
                FFallbackDetector->setInputSize(Enhanced.size());
                FFallbackDetector->detect(Enhanced, Faces);
            }
        }

        if (Faces.empty())
            return false;

        const cv::Rect2f Target(static_cast<float>(Bounds.X),
            static_cast<float>(Bounds.Y), static_cast<float>(Bounds.Width),
            static_cast<float>(Bounds.Height));
        int BestIndex = -1;
        double BestIou = 0.0;
        for (int Row = 0; Row < Faces.rows; ++Row)
        {
            const float* Data = Faces.ptr<float>(Row);
            const cv::Rect2f Candidate(Data[0], Data[1], Data[2], Data[3]);
            const double Iou = IntersectionOverUnion(Target, Candidate);
            if (Iou > BestIou)
            {
                BestIou = Iou;
                BestIndex = Row;
            }
        }

        if (BestIndex < 0 || BestIou < 0.15)
        {
            return false;
        }

        FaceData = Faces.row(BestIndex).clone();
        return true;
    }

    cv::Ptr<cv::FaceRecognizerSF> FRecognizer;
    cv::Ptr<cv::FaceDetectorYN> FDetector;
    cv::Ptr<cv::FaceDetectorYN> FFallbackDetector;
};

extern "C" __declspec(dllexport) void* __cdecl FaceBridge_Create(
    const char* YuNetModelPath, int MinFaceSizePixels, double MinFaceSizeRatio)
{
    try
    {
        std::unique_ptr<BridgeDetector> Detector(new BridgeDetector());
        if (!Detector->Initialize(YuNetModelPath, MinFaceSizePixels, MinFaceSizeRatio))
        {
            return nullptr;
        }
        return Detector.release();
    }
    catch (...)
    {
        return nullptr;
    }
}

extern "C" __declspec(dllexport) void __cdecl FaceBridge_Destroy(void* Handle)
{
    delete static_cast<BridgeDetector*>(Handle);
}

extern "C" __declspec(dllexport) int __cdecl FaceBridge_Detect(void* Handle,
    const unsigned char* Data, int Width, int Height, int Stride,
    FaceBridgeDetection* Output, int Capacity)
{
    BridgeDetector* Detector = static_cast<BridgeDetector*>(Handle);
    if (Detector == nullptr)
    {
        return 0;
    }
    try
    {
        return Detector->Detect(Data, Width, Height, Stride, Output, Capacity);
    }
    catch (...)
    {
        return 0;
    }
}

extern "C" __declspec(dllexport) int __cdecl FaceBridge_DetectEx(void* Handle,
    const unsigned char* Data, int Width, int Height, int Stride,
    FaceBridgeDetection* Output, int Capacity, FaceBridgeDebugStats* DebugStats)
{
    BridgeDetector* Detector = static_cast<BridgeDetector*>(Handle);
    if (Detector == nullptr)
    {
        if (DebugStats != nullptr)
        {
            DebugStats->RawDetections = 0;
            DebugStats->ValidDetections = 0;
        }
        return 0;
    }
    try
    {
        return Detector->DetectEx(Data, Width, Height, Stride, Output, Capacity,
            DebugStats);
    }
    catch (...)
    {
        if (DebugStats != nullptr)
        {
            DebugStats->RawDetections = 0;
            DebugStats->ValidDetections = 0;
        }
        return 0;
    }
}

extern "C" __declspec(dllexport) void __cdecl FaceBridge_Draw(void* Handle,
    unsigned char* Data, int Width, int Height, int Stride,
    const FaceBridgeDetection* Detections, int Count)
{
    BridgeDetector* Detector = static_cast<BridgeDetector*>(Handle);
    if (Detector == nullptr)
    {
        return;
    }
    try
    {
        Detector->Draw(Data, Width, Height, Stride, Detections, Count);
    }
    catch (...)
    {
    }
}

extern "C" __declspec(dllexport) void* __cdecl FaceBridge_RecognizerCreate(
    const char* SFaceModelPath, const char* YuNetModelPath)
{
    try
    {
        std::unique_ptr<BridgeRecognizer> Recognizer(new BridgeRecognizer());
        if (!Recognizer->Initialize(SFaceModelPath, YuNetModelPath))
        {
            return nullptr;
        }
        return Recognizer.release();
    }
    catch (...)
    {
        return nullptr;
    }
}

extern "C" __declspec(dllexport) void __cdecl FaceBridge_RecognizerDestroy(
    void* Handle)
{
    delete static_cast<BridgeRecognizer*>(Handle);
}

extern "C" __declspec(dllexport) int __cdecl FaceBridge_RecognizerFeatureSize(
    void* Handle)
{
    BridgeRecognizer* Recognizer = static_cast<BridgeRecognizer*>(Handle);
    return Recognizer == nullptr ? 0 : Recognizer->FeatureSize();
}

extern "C" __declspec(dllexport) bool __cdecl FaceBridge_ExtractFeature(
    void* Handle, const unsigned char* Data, int Width, int Height, int Stride,
    const FaceBridgeRect* Bounds, float* Feature, int FeatureCapacity,
    int* FeatureSize)
{
    BridgeRecognizer* Recognizer = static_cast<BridgeRecognizer*>(Handle);
    if (Recognizer == nullptr)
    {
        return false;
    }
    try
    {
        return Recognizer->ExtractFeature(Data, Width, Height, Stride, Bounds,
            Feature, FeatureCapacity, FeatureSize);
    }
    catch (...)
    {
        return false;
    }
}

extern "C" __declspec(dllexport) double __cdecl FaceBridge_CompareFeatures(
    void* Handle, const float* FeatureA, int SizeA, const float* FeatureB,
    int SizeB)
{
    BridgeRecognizer* Recognizer = static_cast<BridgeRecognizer*>(Handle);
    if (Recognizer == nullptr)
    {
        return -1.0;
    }
    try
    {
        return Recognizer->CompareFeatures(FeatureA, SizeA, FeatureB, SizeB);
    }
    catch (...)
    {
        return -1.0;
    }
}
