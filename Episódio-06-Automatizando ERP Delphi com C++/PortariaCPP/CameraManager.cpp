//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "CameraManager.h"

#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <objbase.h>

#include <chrono>
#include <cstdlib>
//---------------------------------------------------------------------------
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma package(smart_init)

namespace
{
    double CurrentTickSeconds()
    {
        using Clock = std::chrono::steady_clock;
        return std::chrono::duration<double>(Clock::now().time_since_epoch()).count();
    }

    UnicodeString HResultText(HRESULT Result)
    {
        return L"HRESULT 0x" + IntToHex(static_cast<int>(Result), 8);
    }

    template <typename T>
    void SafeRelease(T*& InterfacePtr)
    {
        if (InterfacePtr != nullptr)
        {
            InterfacePtr->Release();
            InterfacePtr = nullptr;
        }
    }

    void ReleaseDevices(IMFActivate** Devices, UINT32 Count)
    {
        if (Devices == nullptr)
        {
            return;
        }

        for (UINT32 Index = 0; Index < Count; ++Index)
        {
            SafeRelease(Devices[Index]);
        }
        CoTaskMemFree(Devices);
    }

    // Perfil operacional da Portaria.
    // Mesmo que a webcam suporte 4K, não precisamos processar 3840x2160
    // para identificação facial em tempo real.
    const UINT32 CAMERA_TARGET_WIDTH = 1280;
    const UINT32 CAMERA_TARGET_HEIGHT = 720;
    const UINT32 CAMERA_TARGET_FPS = 30;

    UINT32 AbsDiff(UINT32 A, UINT32 B)
    {
        return A >= B ? (A - B) : (B - A);
    }

    bool SelectOperationalVideoMode(
        IMFSourceReader* Reader,
        UINT32& SelectedWidth,
        UINT32& SelectedHeight,
        UINT32& SelectedFpsNumerator,
        UINT32& SelectedFpsDenominator)
    {
        if (Reader == nullptr)
        {
            return false;
        }

        bool Found = false;
        unsigned long long BestScore = 0;
        UINT32 BestWidth = 0;
        UINT32 BestHeight = 0;
        UINT32 BestFpsNumerator = CAMERA_TARGET_FPS;
        UINT32 BestFpsDenominator = 1;

        for (DWORD TypeIndex = 0; ; ++TypeIndex)
        {
            IMFMediaType* NativeType = nullptr;
            const HRESULT TypeResult = Reader->GetNativeMediaType(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                TypeIndex,
                &NativeType);

            if (TypeResult == MF_E_NO_MORE_TYPES)
            {
                break;
            }

            if (FAILED(TypeResult) || NativeType == nullptr)
            {
                SafeRelease(NativeType);
                continue;
            }

            GUID MajorType = GUID_NULL;
            GUID SubType = GUID_NULL;
            UINT32 Width = 0;
            UINT32 Height = 0;
            UINT32 FpsNumerator = 0;
            UINT32 FpsDenominator = 0;

            NativeType->GetGUID(MF_MT_MAJOR_TYPE, &MajorType);
            NativeType->GetGUID(MF_MT_SUBTYPE, &SubType);

            const bool HasSize = SUCCEEDED(
                MFGetAttributeSize(
                    NativeType,
                    MF_MT_FRAME_SIZE,
                    &Width,
                    &Height));

            const bool HasFps = SUCCEEDED(
                MFGetAttributeRatio(
                    NativeType,
                    MF_MT_FRAME_RATE,
                    &FpsNumerator,
                    &FpsDenominator)) &&
                FpsDenominator != 0;

            if (!IsEqualGUID(MajorType, MFMediaType_Video) ||
                !HasSize ||
                Width == 0 ||
                Height == 0)
            {
                SafeRelease(NativeType);
                continue;
            }

            double Fps = HasFps
                ? static_cast<double>(FpsNumerator) /
                    static_cast<double>(FpsDenominator)
                : 0.0;

            // Queremos preferir modos <= 1280x720.
            // Resoluções acima do perfil recebem penalidade forte para
            // impedir a seleção automática de 4K/Full HD quando existe
            // um modo operacional menor.
            unsigned long long Score = 0;

            if (Width > CAMERA_TARGET_WIDTH ||
                Height > CAMERA_TARGET_HEIGHT)
            {
                Score += 100000000ULL;
            }

            Score +=
                static_cast<unsigned long long>(
                    AbsDiff(Width, CAMERA_TARGET_WIDTH)) * 1000ULL;

            Score +=
                static_cast<unsigned long long>(
                    AbsDiff(Height, CAMERA_TARGET_HEIGHT)) * 1000ULL;

            if (Fps > 0.0)
            {
                const double FpsDiff =
                    Fps >= CAMERA_TARGET_FPS
                        ? (Fps - CAMERA_TARGET_FPS)
                        : (CAMERA_TARGET_FPS - Fps);

                Score += static_cast<unsigned long long>(FpsDiff * 10000.0);

                // Evita escolher 60/120 FPS quando 30 FPS está disponível.
                if (Fps > 35.0)
                {
                    Score += 5000000ULL;
                }

                // Também evita modos muito lentos.
                if (Fps < 15.0)
                {
                    Score += 10000000ULL;
                }
            }
            else
            {
                Score += 2000000ULL;
            }

            // Em webcams USB externas, MJPEG costuma reduzir bastante
            // a banda no barramento. É uma preferência, não obrigação.
            if (IsEqualGUID(SubType, MFVideoFormat_MJPG))
            {
                if (Score >= 250000ULL)
                    Score -= 250000ULL;
            }
            else if (IsEqualGUID(SubType, MFVideoFormat_NV12))
            {
                if (Score >= 100000ULL)
                    Score -= 100000ULL;
            }

            if (!Found || Score < BestScore)
            {
                Found = true;
                BestScore = Score;
                BestWidth = Width;
                BestHeight = Height;

                if (HasFps)
                {
                    BestFpsNumerator = FpsNumerator;
                    BestFpsDenominator = FpsDenominator;
                }
                else
                {
                    BestFpsNumerator = CAMERA_TARGET_FPS;
                    BestFpsDenominator = 1;
                }
            }

            SafeRelease(NativeType);
        }

        if (!Found)
        {
            return false;
        }

        SelectedWidth = BestWidth;
        SelectedHeight = BestHeight;
        SelectedFpsNumerator = BestFpsNumerator;
        SelectedFpsDenominator = BestFpsDenominator;
        return true;
    }


    unsigned long long OperationalModeScore(
        UINT32 Width,
        UINT32 Height,
        UINT32 FpsNumerator,
        UINT32 FpsDenominator,
        const GUID& SubType)
    {
        const double Fps =
            (FpsDenominator != 0)
                ? static_cast<double>(FpsNumerator) /
                    static_cast<double>(FpsDenominator)
                : 0.0;

        unsigned long long Score = 0;

        if (Width > CAMERA_TARGET_WIDTH ||
            Height > CAMERA_TARGET_HEIGHT)
        {
            Score += 100000000ULL;
        }

        Score +=
            static_cast<unsigned long long>(
                AbsDiff(Width, CAMERA_TARGET_WIDTH)) * 1000ULL;

        Score +=
            static_cast<unsigned long long>(
                AbsDiff(Height, CAMERA_TARGET_HEIGHT)) * 1000ULL;

        if (Fps > 0.0)
        {
            const double FpsDiff =
                Fps >= CAMERA_TARGET_FPS
                    ? (Fps - CAMERA_TARGET_FPS)
                    : (CAMERA_TARGET_FPS - Fps);

            Score +=
                static_cast<unsigned long long>(
                    FpsDiff * 10000.0);

            if (Fps > 35.0)
                Score += 5000000ULL;

            if (Fps < 15.0)
                Score += 10000000ULL;
        }
        else
        {
            Score += 2000000ULL;
        }

        // MJPEG reduz bastante a banda USB de webcams 4K.
        if (IsEqualGUID(SubType, MFVideoFormat_MJPG))
        {
            if (Score >= 500000ULL)
                Score -= 500000ULL;
        }
        else if (IsEqualGUID(SubType, MFVideoFormat_NV12))
        {
            if (Score >= 150000ULL)
                Score -= 150000ULL;
        }

        return Score;
    }

    bool ConfigureNativeCaptureMode(
        IMFMediaSource* Source,
        UINT32& SelectedWidth,
        UINT32& SelectedHeight,
        UINT32& SelectedFpsNumerator,
        UINT32& SelectedFpsDenominator)
    {
        if (Source == nullptr)
            return false;

        IMFPresentationDescriptor* Presentation = nullptr;
        HRESULT Result =
            Source->CreatePresentationDescriptor(&Presentation);

        if (FAILED(Result) || Presentation == nullptr)
        {
            SafeRelease(Presentation);
            return false;
        }

        DWORD StreamCount = 0;
        Result = Presentation->GetStreamDescriptorCount(&StreamCount);

        if (FAILED(Result))
        {
            SafeRelease(Presentation);
            return false;
        }

        bool Configured = false;

        for (DWORD StreamIndex = 0;
             StreamIndex < StreamCount && !Configured;
             ++StreamIndex)
        {
            BOOL Selected = FALSE;
            IMFStreamDescriptor* StreamDescriptor = nullptr;

            Result = Presentation->GetStreamDescriptorByIndex(
                StreamIndex,
                &Selected,
                &StreamDescriptor);

            if (FAILED(Result) || StreamDescriptor == nullptr)
            {
                SafeRelease(StreamDescriptor);
                continue;
            }

            IMFMediaTypeHandler* Handler = nullptr;
            Result = StreamDescriptor->GetMediaTypeHandler(&Handler);

            if (FAILED(Result) || Handler == nullptr)
            {
                SafeRelease(Handler);
                SafeRelease(StreamDescriptor);
                continue;
            }

            GUID MajorType = GUID_NULL;
            Handler->GetMajorType(&MajorType);

            if (!IsEqualGUID(MajorType, MFMediaType_Video))
            {
                SafeRelease(Handler);
                SafeRelease(StreamDescriptor);
                continue;
            }

            DWORD TypeCount = 0;
            Handler->GetMediaTypeCount(&TypeCount);

            IMFMediaType* BestType = nullptr;
            unsigned long long BestScore = 0;
            bool Found = false;

            for (DWORD TypeIndex = 0;
                 TypeIndex < TypeCount;
                 ++TypeIndex)
            {
                IMFMediaType* Type = nullptr;

                if (FAILED(Handler->GetMediaTypeByIndex(
                        TypeIndex, &Type)) ||
                    Type == nullptr)
                {
                    SafeRelease(Type);
                    continue;
                }

                GUID SubType = GUID_NULL;
                UINT32 Width = 0;
                UINT32 Height = 0;
                UINT32 FpsNumerator = 0;
                UINT32 FpsDenominator = 0;

                Type->GetGUID(MF_MT_SUBTYPE, &SubType);

                const bool HasSize =
                    SUCCEEDED(MFGetAttributeSize(
                        Type,
                        MF_MT_FRAME_SIZE,
                        &Width,
                        &Height));

                const bool HasFps =
                    SUCCEEDED(MFGetAttributeRatio(
                        Type,
                        MF_MT_FRAME_RATE,
                        &FpsNumerator,
                        &FpsDenominator)) &&
                    FpsDenominator != 0;

                if (!HasSize ||
                    Width == 0 ||
                    Height == 0)
                {
                    SafeRelease(Type);
                    continue;
                }

                if (!HasFps)
                {
                    FpsNumerator = CAMERA_TARGET_FPS;
                    FpsDenominator = 1;
                }

                const unsigned long long Score =
                    OperationalModeScore(
                        Width,
                        Height,
                        FpsNumerator,
                        FpsDenominator,
                        SubType);

                if (!Found || Score < BestScore)
                {
                    SafeRelease(BestType);
                    BestType = Type;
                    BestType->AddRef();

                    Found = true;
                    BestScore = Score;
                    SelectedWidth = Width;
                    SelectedHeight = Height;
                    SelectedFpsNumerator = FpsNumerator;
                    SelectedFpsDenominator = FpsDenominator;
                }

                SafeRelease(Type);
            }

            if (Found && BestType != nullptr)
            {
                Result = Handler->SetCurrentMediaType(BestType);

                if (SUCCEEDED(Result))
                {
                    if (!Selected)
                        Presentation->SelectStream(StreamIndex);

                    Configured = true;
                }
            }

            SafeRelease(BestType);
            SafeRelease(Handler);
            SafeRelease(StreamDescriptor);
        }

        SafeRelease(Presentation);
        return Configured;
    }

    HRESULT SetRgb32OutputType(
        IMFSourceReader* Reader,
        UINT32 Width,
        UINT32 Height,
        UINT32 FpsNumerator,
        UINT32 FpsDenominator)
    {
        if (Reader == nullptr)
        {
            return E_POINTER;
        }

        IMFMediaType* RequestedType = nullptr;
        HRESULT Result = MFCreateMediaType(&RequestedType);

        if (FAILED(Result) || RequestedType == nullptr)
        {
            SafeRelease(RequestedType);
            return Result;
        }

        RequestedType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        RequestedType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);

        if (Width > 0 && Height > 0)
        {
            MFSetAttributeSize(
                RequestedType,
                MF_MT_FRAME_SIZE,
                Width,
                Height);
        }

        if (FpsNumerator > 0 && FpsDenominator > 0)
        {
            MFSetAttributeRatio(
                RequestedType,
                MF_MT_FRAME_RATE,
                FpsNumerator,
                FpsDenominator);
        }

        Result = Reader->SetCurrentMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            nullptr,
            RequestedType);

        SafeRelease(RequestedType);
        return Result;
    }
}

TCameraManager::TCameraManager()
    : FSource(nullptr),
      FReader(nullptr),
      FMediaFoundationStarted(false),
      FComInitialized(false),
      FRunning(false),
      FState(TCameraState::Idle),
      FCameraIndex(-1),
      FFrameWidth(0),
      FFrameHeight(0),
      FFrameStride(0),
      FBytesPerPixel(0),
      FReportedFps(0.0),
      FMeasuredFps(0.0),
      FFrameCounter(0),
      FLastMetricTick(0.0)
{
}

TCameraManager::~TCameraManager()
{
    Close();
}

bool TCameraManager::Open(int CameraIndex)
{
    Close();

    FState = TCameraState::Starting;
    FLastError = L"";
    FCameraIndex = CameraIndex;
    ResetMetrics();

    HRESULT Result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(Result))
    {
        FComInitialized = true;
    }
    else if (Result != RPC_E_CHANGED_MODE)
    {
        SetError(L"COM indisponível: " + HResultText(Result));
        return false;
    }

    Result = MFStartup(MF_VERSION, MFSTARTUP_LITE);
    if (FAILED(Result))
    {
        SetError(L"Media Foundation indisponível: " + HResultText(Result));
        return false;
    }
    FMediaFoundationStarted = true;

    IMFAttributes* Attributes = nullptr;
    Result = MFCreateAttributes(&Attributes, 1);
    if (FAILED(Result))
    {
        SetError(L"Falha ao preparar enumeração da webcam: " + HResultText(Result));
        return false;
    }

    Result = Attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(Result))
    {
        SafeRelease(Attributes);
        SetError(L"Falha ao selecionar webcams: " + HResultText(Result));
        return false;
    }

    IMFActivate** Devices = nullptr;
    UINT32 DeviceCount = 0;
    Result = MFEnumDeviceSources(Attributes, &Devices, &DeviceCount);
    SafeRelease(Attributes);

    if (FAILED(Result) || DeviceCount == 0)
    {
        ReleaseDevices(Devices, DeviceCount);
        SetError(L"Nenhuma webcam encontrada");
        return false;
    }

    if (CameraIndex < 0 || static_cast<UINT32>(CameraIndex) >= DeviceCount)
    {
        ReleaseDevices(Devices, DeviceCount);
        SetError(L"Índice de câmera inválido");
        return false;
    }

    Result = Devices[CameraIndex]->ActivateObject(__uuidof(IMFMediaSource),
        reinterpret_cast<void**>(&FSource));
    ReleaseDevices(Devices, DeviceCount);

    if (FAILED(Result) || FSource == nullptr)
    {
        SetError(L"Falha ao abrir webcam: " + HResultText(Result));
        return false;
    }

    // v2 - configura o STREAM NATIVO antes de criar o SourceReader.
    // Isto evita que uma webcam 4K continue trafegando 3840x2160 internamente
    // e seja apenas reduzida depois pelo Media Foundation.
    UINT32 NativeWidth = CAMERA_TARGET_WIDTH;
    UINT32 NativeHeight = CAMERA_TARGET_HEIGHT;
    UINT32 NativeFpsNumerator = CAMERA_TARGET_FPS;
    UINT32 NativeFpsDenominator = 1;

    const bool NativeModeConfigured =
        ConfigureNativeCaptureMode(
            FSource,
            NativeWidth,
            NativeHeight,
            NativeFpsNumerator,
            NativeFpsDenominator);

    IMFAttributes* ReaderAttributes = nullptr;
    Result = MFCreateAttributes(&ReaderAttributes, 1);
    if (SUCCEEDED(Result))
    {
        ReaderAttributes->SetUINT32(
            MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING,
            TRUE);

        // Prioriza baixa latência: para a Portaria interessa o frame atual,
        // não uma fila de frames antigos.
        ReaderAttributes->SetUINT32(MF_LOW_LATENCY, TRUE);
    }

    Result = MFCreateSourceReaderFromMediaSource(FSource, ReaderAttributes, &FReader);
    SafeRelease(ReaderAttributes);
    if (FAILED(Result) || FReader == nullptr)
    {
        SetError(L"Falha ao criar leitor de vídeo: " + HResultText(Result));
        return false;
    }

    // Seleciona um modo de captura adequado para tempo real.
    // O código anterior solicitava apenas RGB32 e deixava a resolução
    // totalmente a cargo do driver. Em webcams 4K isso podia resultar
    // em 3840x2160 RGB32 e causar alto consumo/latência.
    UINT32 RequestedWidth =
        NativeModeConfigured ? NativeWidth : CAMERA_TARGET_WIDTH;
    UINT32 RequestedHeight =
        NativeModeConfigured ? NativeHeight : CAMERA_TARGET_HEIGHT;
    UINT32 RequestedFpsNumerator =
        NativeModeConfigured ? NativeFpsNumerator : CAMERA_TARGET_FPS;
    UINT32 RequestedFpsDenominator =
        NativeModeConfigured ? NativeFpsDenominator : 1;

    if (!NativeModeConfigured)
    {
        SelectOperationalVideoMode(
            FReader,
            RequestedWidth,
            RequestedHeight,
            RequestedFpsNumerator,
            RequestedFpsDenominator);
    }

    Result = SetRgb32OutputType(
        FReader,
        RequestedWidth,
        RequestedHeight,
        RequestedFpsNumerator,
        RequestedFpsDenominator);

    if (FAILED(Result))
    {
        // Alguns drivers aceitam a resolução mas rejeitam a taxa
        // explicitamente informada. Tenta novamente mantendo o limite
        // de resolução e deixando o FPS ser negociado pelo driver.
        Result = SetRgb32OutputType(
            FReader,
            RequestedWidth,
            RequestedHeight,
            0,
            0);
    }

    if (FAILED(Result))
    {
        // Último fallback controlado: 640x480. É melhor operar com
        // resolução menor do que retornar silenciosamente para 4K.
        Result = SetRgb32OutputType(
            FReader,
            640,
            480,
            CAMERA_TARGET_FPS,
            1);
    }

    if (FAILED(Result))
    {
        SetError(
            L"Webcam não aceitou o perfil operacional RGB32: " +
            HResultText(Result));
        return false;
    }

    IMFMediaType* CurrentType = nullptr;
    Result = FReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &CurrentType);
    if (FAILED(Result) || CurrentType == nullptr)
    {
        SetError(L"Falha ao ler formato da webcam: " + HResultText(Result));
        return false;
    }

    UINT32 Width = 0;
    UINT32 Height = 0;
    if (SUCCEEDED(MFGetAttributeSize(CurrentType, MF_MT_FRAME_SIZE, &Width, &Height)))
    {
        FFrameWidth = static_cast<int>(Width);
        FFrameHeight = static_cast<int>(Height);
    }

    UINT32 FpsNumerator = 0;
    UINT32 FpsDenominator = 0;
    if (SUCCEEDED(MFGetAttributeRatio(CurrentType, MF_MT_FRAME_RATE,
        &FpsNumerator, &FpsDenominator)) && FpsDenominator != 0)
    {
        FReportedFps = static_cast<double>(FpsNumerator) / FpsDenominator;
    }

    LONG DefaultStride = 0;
    if (SUCCEEDED(CurrentType->GetUINT32(MF_MT_DEFAULT_STRIDE,
        reinterpret_cast<UINT32*>(&DefaultStride))))
    {
        FFrameStride = DefaultStride;
    }

    SafeRelease(CurrentType);

    FBytesPerPixel = 4;
    if (FFrameStride == 0 && FFrameWidth > 0)
    {
        FFrameStride = FFrameWidth * FBytesPerPixel;
    }

    FRunning = true;
    FState = TCameraState::Running;
    FLastMetricTick = CurrentTickSeconds();
    return true;
}

void TCameraManager::Close()
{
    ReleaseCaptureObjects();

    if (FMediaFoundationStarted)
    {
        MFShutdown();
        FMediaFoundationStarted = false;
    }

    if (FComInitialized)
    {
        CoUninitialize();
        FComInitialized = false;
    }

    FRunning = false;
    FState = TCameraState::Idle;
    FCameraIndex = -1;
    ResetMetrics();
}

bool TCameraManager::IsOpen() const
{
    return FRunning && FReader != nullptr;
}

bool TCameraManager::ReadFrame(TCameraFrame& Frame)
{
    Frame = TCameraFrame();

    if (!IsOpen())
    {
        return false;
    }

    DWORD StreamIndex = 0;
    DWORD Flags = 0;
    LONGLONG Timestamp = 0;
    IMFSample* Sample = nullptr;
    HRESULT Result = FReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0,
        &StreamIndex, &Flags, &Timestamp, &Sample);

    if (FAILED(Result) || (Flags & MF_SOURCE_READERF_ERROR) != 0)
    {
        SafeRelease(Sample);
        SetError(L"Falha ao ler frame da webcam: " + HResultText(Result));
        return false;
    }

    if ((Flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0 || Sample == nullptr)
    {
        SafeRelease(Sample);
        return false;
    }

    IMFMediaBuffer* Buffer = nullptr;
    Result = Sample->ConvertToContiguousBuffer(&Buffer);
    SafeRelease(Sample);

    if (FAILED(Result) || Buffer == nullptr)
    {
        SetError(L"Falha ao obter buffer de vídeo: " + HResultText(Result));
        return false;
    }

    BYTE* BufferData = nullptr;
    DWORD MaxLength = 0;
    DWORD CurrentLength = 0;
    Result = Buffer->Lock(&BufferData, &MaxLength, &CurrentLength);
    if (FAILED(Result) || BufferData == nullptr || CurrentLength == 0)
    {
        SafeRelease(Buffer);
        SetError(L"Falha ao mapear buffer de vídeo: " + HResultText(Result));
        return false;
    }

    Frame.Width = FFrameWidth;
    Frame.Height = FFrameHeight;
    Frame.Stride = FFrameStride;
    Frame.BytesPerPixel = FBytesPerPixel;
    Frame.Data.assign(BufferData, BufferData + CurrentLength);

    Buffer->Unlock();
    SafeRelease(Buffer);

    if (Frame.Empty())
    {
        SetError(L"Frame de vídeo vazio");
        return false;
    }

    UpdateFrameMetrics();
    return true;
}

void TCameraManager::Start()
{
    Open(0);
}

void TCameraManager::Stop()
{
    Close();
}

bool TCameraManager::IsRunning() const
{
    return IsOpen();
}

TCameraState TCameraManager::State() const
{
    return FState;
}

UnicodeString TCameraManager::StatusText() const
{
    switch (FState)
    {
    case TCameraState::Idle:
        return L"aguardando inicialização";
    case TCameraState::Starting:
        return L"inicializando";
    case TCameraState::Running:
        return L"ativa";
    case TCameraState::Error:
        return L"indisponível";
    default:
        return L"desconhecido";
    }
}

UnicodeString TCameraManager::LastError() const
{
    return FLastError;
}

int TCameraManager::CameraIndex() const
{
    return FCameraIndex;
}

int TCameraManager::FrameWidth() const
{
    return FFrameWidth;
}

int TCameraManager::FrameHeight() const
{
    return FFrameHeight;
}

double TCameraManager::ReportedFps() const
{
    return FReportedFps;
}

double TCameraManager::MeasuredFps() const
{
    return FMeasuredFps;
}

void TCameraManager::ResetMetrics()
{
    FFrameWidth = 0;
    FFrameHeight = 0;
    FFrameStride = 0;
    FBytesPerPixel = 0;
    FReportedFps = 0.0;
    FMeasuredFps = 0.0;
    FFrameCounter = 0;
    FLastMetricTick = 0.0;
}

void TCameraManager::UpdateFrameMetrics()
{
    ++FFrameCounter;

    const double Now = CurrentTickSeconds();
    if (FLastMetricTick <= 0.0)
    {
        FLastMetricTick = Now;
        return;
    }

    const double Elapsed = Now - FLastMetricTick;
    if (Elapsed >= 1.0)
    {
        FMeasuredFps = FFrameCounter / Elapsed;
        FFrameCounter = 0;
        FLastMetricTick = Now;
    }
}

void TCameraManager::SetError(const UnicodeString& ErrorText)
{
    FLastError = ErrorText;
    FRunning = false;
    FState = TCameraState::Error;
    ReleaseCaptureObjects();
}

void TCameraManager::ReleaseCaptureObjects()
{
    if (FSource != nullptr)
    {
        FSource->Shutdown();
    }

    SafeRelease(FReader);
    SafeRelease(FSource);
}
