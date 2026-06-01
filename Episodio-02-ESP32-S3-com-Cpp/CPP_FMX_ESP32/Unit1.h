//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Objects.hpp>
#include <FMX.Types.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Effects.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.StdCtrls.hpp>
#include <System.Net.HttpClient.hpp>
#include <System.Net.HttpClientComponent.hpp>
#include <System.Net.URLClient.hpp>
#include <FMX.Edit.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TRectangle *RectBackground;
	TRectangle *RecTop;
	TLabel *LblTitulo;
	TLabel *LblSubtitulo;
	TLabel *LblStatus;
	TLabel *LblIP;
	TLayout *LayoutCentral;
	TRectangle *CardTemperatura;
	TGlowEffect *GlowEffect1;
	TLabel *Label1;
	TLabel *LblTemperatura;
	TRectangle *CardProximidade;
	TGlowEffect *GlowEffect2;
	TLabel *Label2;
	TLabel *lblProximidade;
	TRectangle *CardPotenciometro;
	TGlowEffect *GlowEffect3;
	TLabel *Label3;
	TLabel *lblPotenciometro;
	TRectangle *CardPresenca;
	TGlowEffect *GlowEffect4;
	TLabel *Label4;
	TLabel *lblPresenca;
	TRectangle *RectControles;
	TGlowEffect *GlowEffect5;
	TLabel *Label5;
	TRectangle *CardLED;
	TGlowEffect *GlowEffect6;
	TLabel *Label6;
	TSwitch *SwitchLED;
	TRectangle *CardRele1;
	TGlowEffect *GlowEffect7;
	TLabel *Label7;
	TSwitch *SwitchRele1;
	TRectangle *CardRele2;
	TGlowEffect *GlowEffect8;
	TLabel *Label8;
	TSwitch *SwitchRele2;
	TRectangle *CardBuzzer;
	TGlowEffect *GlowEffect9;
	TSpeedButton *BtnBuzzer;
	TNetHTTPClient *NetHTTPClient1;
	TTimer *Timer1;
	TLayout *LayoutMain;
	TVertScrollBox *VertScrollBox1;
	TLayout *FlowLayoutSensores;
	TLayout *FlowLayoutSensores2;
	TRectangle *Rectangle1;
	TGlowEffect *GlowEffect10;
	TLabel *Label9;
	TSwitch *Switch1;
	TLabel *Label10;
	TPanel *pnelIPServer;
	TLabel *Label11;
	TToolBar *ToolBar1;
	TLabel *Label12;
	TEdit *edtIP;
	TButton *Button1;
	TGlowEffect *GlowEffect11;
	TIdTCPClient *IdTCPClient1;
	void __fastcall SwitchLEDSwitch(TObject *Sender);
	void __fastcall SwitchRele1Switch(TObject *Sender);
	void __fastcall SwitchRele2Switch(TObject *Sender);
	void __fastcall BtnBuzzerClick(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
    String ESP_IP = "192.168.15.63";

public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
