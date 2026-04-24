//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.TabControl.hpp>
#include <FMX.Types.hpp>
#include <FMX.ListView.Adapters.Base.hpp>
#include <FMX.ListView.Appearances.hpp>
#include <FMX.ListView.hpp>
#include <FMX.ListView.Types.hpp>
#include <FMX.Controls3D.hpp>
#include <FMX.Objects3D.hpp>
#include <FMX.Types3D.hpp>
#include <FMX.Viewport3D.hpp>
#include <System.Math.Vectors.hpp>
#include <FMX.MaterialSources.hpp>
#include <FMX.Ani.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.Effects.hpp>
#include <FMX.Edit.hpp>
#include <System.Net.HttpClient.hpp>
#include <System.Net.HttpClientComponent.hpp>
#include <System.Net.URLClient.hpp>
#include <System.Threading.hpp>
#include <System.Classes.hpp>
#include <FMX.Memo.hpp>
#include <FMX.Memo.Types.hpp>
#include <FMX.ScrollBox.hpp>
#include <FMX.Objects.hpp>
#include <atomic>
#include <System.DateUtils.hpp>
#include <algorithm>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TTabControl *TabControl1;
	TTabItem *tbSatellite;
	TTabItem *TabItem2;
	TToolBar *ToolBar1;
	TPanel *pnelEsquerda;
	TPanel *pnelDireita;
	TPanel *pnelControlMode;
	TPanel *pnelTelemetry;
	TPanel *pnelAttitude;
	TPanel *pnelTemperature;
	TPanel *pnelSwitchControlMode;
	TPanel *pnelManualControl;
	TPanel *pnelSwitchManualControl;
	TLabel *lblControlMode;
	TLabel *lblManualControl;
	TLabel *lblTelemetryData;
	TLabel *lblPosition;
	TLabel *lblTemperature;
	TPanel *pnelGraficoTemperature;
	TSwitch *Switch1;
	TLabel *Label1;
	TLabel *Label2;
	TViewport3D *Viewport3D1;
	TDummy *DummyEarth;
	TSphere *SphereEarth;
	TSphere *SphereSatelite;
	TCamera *Camera1;
	TLight *Light1;
	TDummy *DummyOrbit;
	TTimer *timerAnimation;
	TTrackBar *tbX;
	TTrackBar *tbY;
	TTrackBar *tbZ;
	TTextureMaterialSource *TextureMaterialSourceEarth;
	TBitmapAnimation *BitmapAnimation1;
	TListBox *lboxTelemetriaData;
	TListBoxItem *ListBoxItem1;
	TListBoxItem *ListBoxItem2;
	TLabel *lblAltitude;
	TLabel *lblOrbitalVelocity;
	TDummy *DummyTilt;
	TDummy *DummyRotation;
	TPanel *pnelTerraManual;
	TLabel *Label3;
	TPanel *pnelTerraManualControl;
	TTrackBar *tbTerraZ;
	TTrackBar *tbTerraX;
	TTrackBar *tbTerraY;
	TLabel *lblTerraX;
	TLabel *lblTerraY;
	TLabel *lblTerraZ;
	TLabel *lblTX;
	TLabel *lblTY;
	TLabel *lblTZ;
	TEdit *edtTerraX;
	TEdit *edtTerraY;
	TEdit *edtTerraZ;
	TShadowEffect *ShadowEffect1;
	TSphere *SphereAtmosfera;
	TLightMaterialSource *LightMaterialSource1;
	TLabel *Label5;
	TStyleBook *StyleBook1;
	TListBox *lboxPosition;
	TListBoxItem *lboxLatitude;
	TListBoxItem *lboxLongitude;
	TLabel *lblLatitude;
	TLabel *lblLongitude;
	TModel3D *Model3D1;
	TLightMaterialSource *Model3D1Mat01;
	TLightMaterialSource *Model3D1Mat41;
	TLightMaterialSource *Model3D1Mat21;
	TLightMaterialSource *Model3D1Mat31;
	TLightMaterialSource *Model3D1Mat11;
	TLightMaterialSource *Model3D1Mat22;
	TLightMaterialSource *Model3D1Mat02;
	TLightMaterialSource *Model3D1Mat12;
	TLightMaterialSource *Model3D1Mat32;
	TLightMaterialSource *Model3D1Mat42;
	TLightMaterialSource *Model3D1Mat51;
	TPanel *pnelSensors;
	TPanel *Panel1;
	TLabel *Label6;
	TListBox *ListBox1;
	TListBoxItem *ListBoxItem3;
	TListBoxItem *ListBoxItem4;
	TListBoxItem *ListBoxItem5;
	TLabel *Label7;
	TLabel *Label8;
	TLabel *Label9;
	TPanel *Panel2;
	TPanel *pnelSatelliteSimulation;
	TLabel *lblSatelliteSimulation;
	TLabel *Label12;
	TSwitch *Switch2;
	TLabel *Label13;
	TLabel *Label14;
	TListBoxGroupHeader *ListBoxGroupHeader1;
	TListBoxGroupHeader *ListBoxGroupHeader2;
	TSwitch *Switch3;
	TLabel *Label15;
	TLabel *Label16;
	TLabel *Label17;
	TSwitch *Switch4;
	TLabel *Label18;
	TPanel *Panel4;
	TGridPanelLayout *GridPanelLayout1;
	TNetHTTPClient *NetHTTPClient1;
	TLabel *lblStatus;
	TTimer *timerTelemetry;
	TLightMaterialSource *Model3D1Mat23;
	TLightMaterialSource *Model3D1Mat03;
	TLightMaterialSource *Model3D1Mat13;
	TLightMaterialSource *Model3D1Mat33;
	TLightMaterialSource *Model3D1Mat43;
	TLightMaterialSource *Model3D1Mat52;
	TTrackBar *tbTerraVelocidade;
	TTextureMaterialSource *TextureMaterialSourceBackGround;
	TRectangle *Rectangle1;
	TImage *imgNasa;
	TImage *imgCpp;
	TGlowEffect *efeitoCpp;
	TGlowEffect *efeitoNasa;
	TShadowEffect *efeitoPanelEsquerda;
	TShadowEffect *ShadowEffect2;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall timerAnimationTimer(TObject *Sender);
	void __fastcall tbXChange(TObject *Sender);
	void __fastcall tbYChange(TObject *Sender);
	void __fastcall tbZChange(TObject *Sender);
	void __fastcall Switch1Switch(TObject *Sender);
	void __fastcall tbTerraXChange(TObject *Sender);
	void __fastcall tbTerraYChange(TObject *Sender);
	void __fastcall tbTerraZChange(TObject *Sender);
	void __fastcall edtTerraXChange(TObject *Sender);
	void __fastcall edtTerraYChange(TObject *Sender);
	void __fastcall edtTerraZChange(TObject *Sender);
	void __fastcall tbTerraVelocidadeChange(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall timerTelemetryTimer(TObject *Sender);


private:	// User declarations
     bool __fastcall IsAutoMode();
	 void __fastcall UpdateTelemetry(float t);
	 void __fastcall ApplyEarthRotation();
	 float rotacaoY = 0;
	 void __fastcall UpdateSatelliteData();
	 void __fastcall GetDemoSimulationData(float &lat, float &lon);
	 void __fastcall UpdateSatellitePosition(float lat, float lon);
	 String __fastcall BuscarTelemetria();
	 void __fastcall AtualizarTelemetriaAsync();
	 std::atomic<bool> busy = false;
	 float tempoOrbita = 0;
	 bool modoAutomatico = false;
	 void __fastcall ResetarTelemetria (TObject *Sender);
	 bool temDados = false;
	 void ProcessarTelemetria(String data, double &lat, double &lon);
	 std::vector<TSphere*> trilhaOrbita;

public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
