#ifndef MainFormH
#define MainFormH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Skia.hpp>

#include <chrono>
#include <memory>
#include <vector>
#include <opencv2/core/types.hpp>

#include "AppConfig.h"
#include "EmployeeModel.h"
#include "FaceDetector.h"
#include "FaceRecognizer.h"
#include "FaceRepository.h"
#include "LivenessDetector.h"
//---------------------------------------------------------------------------

class TBleManager;
class TCameraManager;
struct TCameraFrame;
class TAccessEventRepository;
class TEmployeeRepository;
class TEmployeeSyncClient;
class TIdentificationManager;
class TIntegrationClient;
class TObservabilityHeartbeat;
class TRfidSerialReader;

enum class TAccessPage
{
    Identification,
    PresentEmployees,
    RecentEntries,
    Camera,
    BleDevices,
    ManualRelease,
    Incidents,
    Settings
};

enum class TFacePreviewState
{
    Waiting,
    TooFar,
    Detected,
    TooClose,
    Multiple,
    Recognizing,
    Recognized,
    NotRecognized,
    Enrolling
};

enum class IdentificationState
{
    WaitingForFace,
    AdjustDistance,
    Stabilizing,
    CheckingLiveness,
    ReadyForRecognition,
    Failed
};

enum class TFaceQualityIssue
{
    None,
    LowLight,
    Blurred
};

enum class FaceDistance
{
    NoFace,
    TooFar,
    Good,
    TooClose
};

enum class TTerminalLayoutMode
{
    Auto,
    Horizontal,
    TotemVertical
};

enum class TCameraRotationMode
{
    Auto,
    Deg0,
    Deg90CW,
    Deg90CCW,
    Deg180
};

enum class TRfidVisualState
{
    Idle,
    TagRead,
    Authorized,
    UnknownTag,
    Denied,
    InvalidLink
};

class TMainForm : public TForm
{
__published:
    TPanel *RootPanel;
    TPanel *TopBarPanel;
    TLabel *LblAppName;
    TLabel *LblTerminal;
    TLabel *LblOnline;
    TPanel *ClockPanel;
    TLabel *LblClock;
    TLabel *LblDate;
    TPanel *PageHost;
    TPanel *IdentificationView;
    TPanel *CameraPanel;
    TShape *ShapeFaceFrame;
    TLabel *LblFaceGlyph;
    TLabel *LblCameraPlaceholder;
    TLabel *LblPrompt;
    TPanel *EmployeeResultPanel;
    TPanel *PhotoPanel;
    TLabel *LblPhoto;
    TLabel *LblEmployeeState;
    TLabel *LblName;
    TLabel *LblDepartment;
    TLabel *LblIdentifiedAt;
    TPanel *BottomStatusPanel;
    TPanel *FaceStatusCard;
    TLabel *LblFaceIndicator;
    TLabel *LblFaceStatusDetail;
    TPanel *BleStatusCard;
    TLabel *LblBleIndicator;
    TLabel *LblBleStatusDetail;
    TLabel *LblCameraStatus;
    TPanel *MenuCenterPanel;
    TPanel *BtnMenu;
    TLabel *LblMenuHint;
    TPanel *SecondaryView;
    TPanel *BtnBackFromPage;
    TLabel *LblAdminTitle;
    TLabel *LblAdminDescription;
    TPanel *DeveloperPanel;
    TLabel *LblDeveloperStatus;
    TListBox *LstEmployees;
    TButton *BtnSimulate;
    TButton *BtnClear;
    TButton *BtnStartCamera;
    TPanel *OverlayMenu;
    TPanel *BtnMenuBack;
    TLabel *LblMenuHeader;
    TLabel *LblMenuFooter;
    TPanel *MenuIdentification;
    TLabel *LblMenuIdentificationIcon;
    TLabel *LblMenuIdentificationTitle;
    TLabel *LblMenuIdentificationDesc;
    TLabel *LblMenuIdentificationArrow;
    TPanel *MenuPresent;
    TLabel *LblMenuPresentIcon;
    TLabel *LblMenuPresentTitle;
    TLabel *LblMenuPresentDesc;
    TLabel *LblMenuPresentArrow;
    TPanel *MenuRecent;
    TLabel *LblMenuRecentIcon;
    TLabel *LblMenuRecentTitle;
    TLabel *LblMenuRecentDesc;
    TLabel *LblMenuRecentArrow;
    TPanel *MenuCamera;
    TLabel *LblMenuCameraIcon;
    TLabel *LblMenuCameraTitle;
    TLabel *LblMenuCameraDesc;
    TLabel *LblMenuCameraArrow;
    TPanel *MenuBle;
    TLabel *LblMenuBleIcon;
    TLabel *LblMenuBleTitle;
    TLabel *LblMenuBleDesc;
    TLabel *LblMenuBleArrow;
    TPanel *MenuManual;
    TLabel *LblMenuManualIcon;
    TLabel *LblMenuManualTitle;
    TLabel *LblMenuManualDesc;
    TLabel *LblMenuManualArrow;
    TPanel *MenuOccurrences;
    TLabel *LblMenuOccurrencesIcon;
    TLabel *LblMenuOccurrencesTitle;
    TLabel *LblMenuOccurrencesDesc;
    TLabel *LblMenuOccurrencesArrow;
    TPanel *MenuSettings;
    TLabel *LblMenuSettingsIcon;
    TLabel *LblMenuSettingsTitle;
    TLabel *LblMenuSettingsDesc;
    TLabel *LblMenuSettingsArrow;
    TTimer *ClockTimer;
	TTimer *ReturnTimer;


    void __fastcall FormResize(TObject *Sender);
    void __fastcall ClockTimerTimer(TObject *Sender);
    void __fastcall ReturnTimerTimer(TObject *Sender);
    void __fastcall MenuButtonClick(TObject *Sender);
    void __fastcall BackToIdentificationClick(TObject *Sender);
    void __fastcall MenuItemClick(TObject *Sender);
    void __fastcall StartCameraClick(TObject *Sender);
    void __fastcall SimulateClick(TObject *Sender);
    void __fastcall ClearClick(TObject *Sender);

private:
    TAppConfig FConfig;
    std::unique_ptr<TCameraManager> FCameraManager;
    std::unique_ptr<TBleManager> FBleManager;
    std::unique_ptr<TAccessEventRepository> FAccessEventRepository;
    std::unique_ptr<TEmployeeRepository> FEmployeeRepository;
    std::unique_ptr<TEmployeeSyncClient> FEmployeeSyncClient;
    std::unique_ptr<TIntegrationClient> FIntegrationClient;
    std::unique_ptr<TIdentificationManager> FIdentificationManager;
    std::unique_ptr<FaceDetector> FFaceDetector;
    std::unique_ptr<FaceRecognizer> FFaceRecognizer;
    std::unique_ptr<FaceRepository> FFaceRepository;
	std::unique_ptr<LivenessDetector> FLivenessDetector;
    std::unique_ptr<TObservabilityHeartbeat> FObservabilityHeartbeat;
    std::unique_ptr<TRfidSerialReader> FRfidSerialReader;
    TTimer* FRfidPollTimer;
    UnicodeString FLastRfidUid;
    TRfidVisualState FLastRfidState;

    // Painel administrativo do leitor RFID.
    // O UID capturado aqui é separado do último resultado da tela principal,
    // evitando vincular acidentalmente uma TAG usada anteriormente no acesso.
    UnicodeString FRfidAdminUid;
    TPanel* FRfidReaderCard;
    TLabel* FRfidReaderTitle;
    TLabel* FRfidReaderStatus;
    TLabel* FRfidReaderUid;
    TAccessPage FCurrentPage;
    TFacePreviewState FFacePreviewState;
    IdentificationState FIdentificationState;
    TFaceQualityIssue FFaceQualityIssue;
    TImage* FCameraPreviewImage;
    TTimer* FCameraPreviewTimer;
    TLabel* FCameraSelectorLabel;
    TComboBox* FCameraSelector;
    TImage* FCameraSettingsPreview;

    TPanel* FSettingsErpCard;
    TPanel* FSettingsDataCard;
    TPanel* FSettingsTerminalCard;

    TLabel* FSettingsErpTitle;
    TLabel* FSettingsErpStatus;
    TLabel* FSettingsErpInfo;
    TEdit* FSettingsApiEdit;
    TButton* FSettingsTestErpButton;

    TLabel* FSettingsDataTitle;
    TLabel* FSettingsEmployeeDbStatus;
    TLabel* FSettingsEmployeeDbPath;
    TLabel* FSettingsAccessDbStatus;
    TLabel* FSettingsAccessDbPath;
    TButton* FSettingsSelectDbButton;

    TLabel* FSettingsTerminalTitle;
    TLabel* FSettingsTerminalStatus;
    TEdit* FSettingsTerminalEdit;
    TButton* FSettingsSaveTerminalButton;
    TButton* FSettingsDiagnosticButton;
    Graphics::TBitmap* FCameraPreviewBitmap;
    TImage* FEmployeePhotoImage;
    Graphics::TBitmap* FEmployeePhotoBitmap;
    TShape* FRecognitionCheckShape;
	TLabel* FRecognitionCheckLabel;
	TSkAnimatedImage* FIdleLottie;
	TPaintBox* FTechBackground;
	TPaintBox* FHeaderDecor;
	TPaintBox* FCameraDecor;
	TPaintBox* FFooterDecor;
	TLabel* FBrandSubtitle;
	TPanel* FTerminalStatusCard;
	TLabel* FTerminalStatusTitle;
	TLabel* FProductVersion;
	TLabel* FCameraModeBadge;
	TPaintBox* FIdleVisual;
	TTimer* FIdleVisualTimer;
	int FIdleVisualPhase;

    bool FFaceRecognitionLocked;
    int FRecognizedEmployeeId;
    unsigned long FPreviewFrameCounter;
    int FPreviewCameraIndex;
    bool FCameraListInitialized;
    int FFaceHitFrames;
    int FFaceMissFrames;
    int FSingleFaceFrames;
    int FMultipleFaceFrames;
    int FFaceDetectionInterval;
    unsigned int FDetectionFramesThisSecond;
    unsigned int FLastDetectionMetricTick;
    double FMeasuredDetectionsPerSecond;
    std::vector<FaceDetection> FLastFaceDetections;
    FaceRect FCurrentFaceBounds;
    FaceDetection FSmoothedFaceDetection;
    bool FHasCurrentFaceBounds;
    bool FHasSmoothedFaceDetection;
    unsigned int FLastRecognitionAttemptTick;
    int FRecognitionCandidateId;
    int FRecognitionCandidateHits;
    FaceRecognitionResult FLastRecognitionResult;
    double FLastCorrectTestScore;
    double FLastUnknownTestScore;
    double FLastRecognitionDurationMs;
    bool FEnrollmentActive;
    bool FIncidentHistoryMode;
    int FEnrollmentEmployeeId;
    UnicodeString FRemoteEnrollmentRequestId;
    UnicodeString FRemoteEnrollmentEmployeeName;
    std::vector<FaceFeature> FEnrollmentSamples;
    FaceDetection FPrimaryFaceDetection;
    bool FHasPrimaryFace;
    bool FHasMultipleOperationalFaces;
    unsigned int FStableFaceStartTick;
    unsigned int FLastFaceSeenTick;

    // v0.4.3.2 - controla a animação de espera sem flicker.
    // Só mostramos o visual ocioso depois de uma ausência contínua
    // de face por IdleAnimationDelayMs.
    unsigned int FNoFaceSinceTick;

    bool FHadRecentFace;
    FaceDistance FLastDistance;
    FaceDistance FPreviousDistance;
    cv::Rect FLastFaceBounds;
    std::chrono::steady_clock::time_point FLastFaceTime;
    double FLastFaceRatio;
    double FPreviousFaceRatio;
    std::vector<TEmployee> FEmployees;
    TTerminalLayoutMode FLayoutMode;
    TCameraRotationMode FCameraRotationMode;

    bool FEmployeeSyncStateInitialized;
    bool FLastEmployeeSyncOnline;
    int FLastEmployeeSyncCount;

    void InitializeManagers();
    void InitializeRfidSerial();
    void InitializeRfidAdminReader();
    void UpdateRfidAdminReader();
    void ApplyRfidStatus();
    void __fastcall RfidPollTimerTimer(TObject* Sender);
    void ConfigureRuntimeView();
    void ConfigureCaptions();
    void InitializeCameraPreview();
    void InitializeCameraSelector();

    void InitializeSettingsControls();
    void RefreshSettingsView();
    void LoadPersistedSettings();
    void SavePersistedSettings();
    bool ApplyEmployeeDatabasePath(const UnicodeString& DatabasePath);
    bool TestErpConnection();
    void RunSystemDiagnostic();
    void RefreshCameraDeviceList();
    bool SwitchCamera(int CameraIndex);
    void UpdateCameraSettingsPreview();
    bool WaitForOperationalCameraFrame(int Attempts = 20);
    void StartCameraPreview();
    void StopCameraPreview();
    void UpdateCameraPreviewVisibility();
    void ResetFacePreviewState();
    void UpdateFacePreviewState(const std::vector<FaceDetection>& Detections,
        int FrameWidth, int FrameHeight);
    void ApplyFacePreviewStatus();
    void UpdateDetectionMetrics();
    void ProcessFaceEnrollment(const TCameraFrame& Frame);
    void ProcessFaceRecognition(const TCameraFrame& Frame);
    FaceRecognitionResult RecognizeFeature(const FaceFeature& Feature);
    bool BuildEnrollmentTemplate(FaceFeature& Template) const;
    void BeginFaceEnrollment();
    void BeginRemoteFaceEnrollment(
        int EmployeeId,
        const UnicodeString& EmployeeName,
        const UnicodeString& RequestId);
    void ProcessPendingBiometricCommands();
    void ProcessRemoteBiometricRevoke(
        int EmployeeId,
        const UnicodeString& EmployeeName,
        const UnicodeString& RequestId);
    void CompleteFaceEnrollment();
    void RejectFaceRecognition();
    void ClearRecognizedFaceResult();
    void UpdateEmployeePhotoSnapshot(const TCameraFrame& Frame,
        const FaceDetection& Detection);
    void __fastcall MenuHoverEnter(TObject* Sender);
    void __fastcall MenuHoverLeave(TObject* Sender);
    void UpdateClock();
    void RefreshIdentificationView();
    void RefreshEmployeeList();
    void ProcessEmployeeSyncStatus();
    void ReconcileRfidAssignments();
    void ReconfigureEmployeeSync();
    void RefreshRecentEntries();
    void RefreshPresentEmployees();
    void RefreshManualReleaseEmployees();
    void RefreshRfidTagEmployees();
    bool RegisterRfidTagManual();
    bool RemoveSelectedRfidTag();
    void ProcessRfidIdentification(const UnicodeString& Uid);
    void ShowRfidEmployeeResult(const TEmployee& Employee,
        const UnicodeString& Uid);
    void ShowRfidBlockedEmployeeResult(const TEmployee& Employee,
        const UnicodeString& Uid,
        const UnicodeString& Reason);
    bool RecordRfidAccessEvent(const TEmployee& Employee,
        const UnicodeString& Uid);
    UnicodeString NormalizeRfidUid(const UnicodeString& RawUid) const;
    bool IsValidRfidUid(const UnicodeString& Uid) const;
    void RefreshOccurrences();
    void RefreshOccurrenceEmployees();
    void UpdateOccurrencePageButtons();
    bool RegisterOccurrence();
    bool SetSelectedEmployeeBlocked(bool Blocked);
    bool ResolveEmployeeByName(const UnicodeString& Name, TEmployee& Employee);
    bool IsEmployeeAccessBlocked(const TEmployee& Employee,
        UnicodeString& Reason) const;
    void ShowBlockedEmployeeResult(const TEmployee& Employee,
        const UnicodeString& Reason,
        const TCameraFrame& Frame);
    void RecordFaceAccessEvent(const TEmployee& Employee, double Score);
    bool RecordManualAccessEvent(const TEmployee& Employee);
    void UpdateDeveloperPanel();
    bool TryGetSelectedEmployee(TEmployee& Employee) const;
    bool CanProcessRecognition() const;
    bool FaceInsideFrame(const FaceDetection& Detection, int FrameWidth, int FrameHeight) const;
    double FaceAverageBrightness(const TCameraFrame& Frame,
        const FaceDetection& Detection) const;
    bool FaceLightingEnough(const TCameraFrame& Frame,
        const FaceDetection& Detection) const;
    bool FaceSharpEnough(const TCameraFrame& Frame, const FaceDetection& Detection) const;
    void ShowPage(TAccessPage Page);
    void ShowIdentification();
    void OpenMenu();
    void CloseMenu();
    bool IsTotemLayout() const;
    UnicodeString LayoutModeCaption() const;
    void CycleLayoutMode();
    TCameraRotationMode EffectiveCameraRotation() const;
    UnicodeString CameraRotationCaption() const;
    void CycleCameraRotation();
    void LayoutOverlayMenu(bool TotemMode);
    void LayoutTerminal();
	void UpdateMenuSelection();
	void InitializeIdleAnimation();
	void InitializeTechBackground();
	void __fastcall TechBackgroundPaint(TObject* Sender);
	void InitializeCorporateDecor();
	void __fastcall HeaderDecorPaint(TObject* Sender);
	void __fastcall CameraDecorPaint(TObject* Sender);
	void __fastcall FooterDecorPaint(TObject* Sender);
	void InitializeIdleVisual();
	void __fastcall IdleVisualPaint(TObject* Sender);
	void __fastcall IdleVisualTimerTimer(TObject* Sender);
    void ApplyRoundedCorners(TWinControl* Control, int Radius);
    TAccessPage PageFromTag(int Tag) const;
    UnicodeString PageTitle(TAccessPage Page) const;
    UnicodeString PageDescription(TAccessPage Page) const;
    UnicodeString TerminalDisplayName() const;
    void __fastcall CameraPreviewTimerTimer(TObject *Sender);
    void __fastcall CameraSelectorChange(TObject *Sender);
    void __fastcall SettingsSelectDbClick(TObject *Sender);
    void __fastcall SettingsTestErpClick(TObject *Sender);
    void __fastcall SettingsSaveTerminalClick(TObject *Sender);
	void __fastcall SettingsDiagnosticClick(TObject *Sender);



public:
    __fastcall TMainForm(TComponent* Owner);
    virtual __fastcall ~TMainForm();
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif


