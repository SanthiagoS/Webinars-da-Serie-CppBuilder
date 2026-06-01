//---------------------------------------------------------------------------

#ifndef untVCL_Esp32H
#define untVCL_Esp32H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <System.Net.HttpClient.hpp>
#include <System.Net.HttpClientComponent.hpp>
#include <System.Net.URLClient.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TNetHTTPClient *NetHTTPClient1;
	TTimer *Timer1;
	TGroupBox *gboxLed;
	TButton *BtnLedOn;
	TButton *BtnLedOff;
	TGroupBox *gboxRele;
	TButton *BtnRele1ON;
	TButton *BtnRele1OFF;
	TButton *BtnRele2ON;
	TButton *BtnRele2OFF;
	TGroupBox *gboxSensores;
	TLabel *lblTemperatura;
	TLabel *lblPotenciometro;
	TLabel *lblPresenca;
	TLabel *lblProximidade;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TPanel *Panel1;
	TLabel *lblIPServidor;
	TEdit *edtIpServidor;
	TLabel *Label5;
	TButton *Button1;
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall BtnLedOnClick(TObject *Sender);
	void __fastcall BtnLedOffClick(TObject *Sender);
	void __fastcall BtnRele1ONClick(TObject *Sender);
	void __fastcall BtnRele1OFFClick(TObject *Sender);
	void __fastcall BtnRele2ONClick(TObject *Sender);
	void __fastcall BtnRele2OFFClick(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
	String ESP_IP = edtIpServidor->Text;

public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
