//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.WinXCtrls.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdHTTP.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <System.JSON.hpp>
#include <System.SysUtils.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TPanel *Panel1;
	TLabel *Label1;
	TPanel *pnlConexoes;
	TLabel *lblConexoesTitle;
	TImage *imgMcpPrincipal;
	TLabel *lblMcpPrincipal;
	TImage *imgMcpBridge;
	TLabel *lblMcpBridge;
	TPanel *Panel2;
	TPanel *pnlIndicadores;
	TLabel *lblIndicadoresTitle;
	TImage *imgVerde;
	TLabel *lblOperacao;
	TImage *imgAmarelo;
	TLabel *lblAtencao;
	TImage *imgVermelho;
	TLabel *lblParada;
	TPanel *pnlEsteiraCard;
	TLabel *lblEsteiraTitle;
	TLabel *lblEsteiraHint;
	TImage *imgEsteira;
	TPanel *pnlControleCard;
	TLabel *lblControleTitle;
	TLabel *lblControleHint;
	TLabel *lblStatus;
	TLabel *lblVelocidade;
	TLabel *Label5;
	TPaintBox *pbEmergenciaSwitch;
	TToggleSwitch *swEmergencia;
	TMemo *Memo1;
	TLabel *Label4;
	TTimer *Timer1;
	TIdHTTP *IdHTTP1;
	TTimer *tmSwitchAnimation;
	TTimer *tmConveyorAnimation;
	TLabel *lblStatusEsteira;
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall UpdateTelemetry();
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall UpdateDashboard();
	void __fastcall swEmergenciaClick(TObject *Sender);
	void __fastcall pbEmergenciaSwitchClick(TObject *Sender);
	void __fastcall pbEmergenciaSwitchPaint(TObject *Sender);
	void __fastcall tmSwitchAnimationTimer(TObject *Sender);
	void __fastcall tmConveyorAnimationTimer(TObject *Sender);
private:	// User declarations
	int FSwitchKnobX;
	int FSwitchTargetX;
	int FConveyorOffset;
	int FCurrentSpeed;
	void __fastcall AddTelemetryLog(const String &Message);
	void __fastcall ApplyEmergencyUi(bool Active);
	void __fastcall ConfigureImage(TImage *Image);
	bool __fastcall CheckPort(int Port);
	int __fastcall ConveyorAnimationStep() const;
	void __fastcall DrawConveyorImage();
	void __fastcall DrawConnectionLed(TImage *Image, bool Connected);
	void __fastcall DrawLedImage(TImage *Image, TColor LedColor, TColor GlowColor);
	void __fastcall DrawEmergencySwitch();
	void __fastcall UpdateConnectionIndicators(bool MainConnected, bool BridgeConnected);
	void __fastcall SendJsonPost(const String &Url, const String &JsonBody);
	void __fastcall StartSwitchAnimation(bool Active);
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
