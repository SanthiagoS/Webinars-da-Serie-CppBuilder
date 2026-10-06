//---------------------------------------------------------------------------

#ifndef CameraManagerH
#define CameraManagerH
//---------------------------------------------------------------------------
#include <System.hpp>

#include <vector>

struct IMFMediaSource;
struct IMFSourceReader;
//---------------------------------------------------------------------------

enum class TCameraState
{
    Idle,
    Starting,
    Running,
    Error
};

struct TCameraFrame
{
    int Width;
    int Height;
    int Stride;
    int BytesPerPixel;
    std::vector<unsigned char> Data;

    TCameraFrame()
        : Width(0), Height(0), Stride(0), BytesPerPixel(0)
    {
    }

    bool Empty() const
    {
        return Width <= 0 || Height <= 0 || Data.empty();
    }
};

class TCameraManager
{
public:
    TCameraManager();
    ~TCameraManager();

    bool Open(int CameraIndex = 0);
    void Close();
    bool IsOpen() const;
    bool ReadFrame(TCameraFrame& Frame);

    void Start();
    void Stop();

    bool IsRunning() const;
    TCameraState State() const;
    UnicodeString StatusText() const;
    UnicodeString LastError() const;
    int CameraIndex() const;
    int FrameWidth() const;
    int FrameHeight() const;
    double ReportedFps() const;
    double MeasuredFps() const;

private:
    void ResetMetrics();
    void UpdateFrameMetrics();
    void SetError(const UnicodeString& ErrorText);
    void ReleaseCaptureObjects();

    IMFMediaSource* FSource;
    IMFSourceReader* FReader;
    bool FMediaFoundationStarted;
    bool FComInitialized;
    bool FRunning;
    TCameraState FState;
    UnicodeString FLastError;
    int FCameraIndex;
    int FFrameWidth;
    int FFrameHeight;
    int FFrameStride;
    int FBytesPerPixel;
    double FReportedFps;
    double FMeasuredFps;
    unsigned int FFrameCounter;
    double FLastMetricTick;
};

//---------------------------------------------------------------------------
#endif
