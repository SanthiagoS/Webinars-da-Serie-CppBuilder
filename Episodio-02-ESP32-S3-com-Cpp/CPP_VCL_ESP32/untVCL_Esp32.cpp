//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "untVCL_Esp32.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
     try
    {
        String temp =
          NetHTTPClient1->Get(
			"http://" + ESP_IP + "/temperatura"
          )->ContentAsString();

		lblTemperatura->Caption =
          temp + " °C";
    }
    catch(...)
    {
		lblTemperatura->Caption =
          "Erro";
	}

    try
   {
	String pot =
	  NetHTTPClient1->Get(
		"http://" + ESP_IP + "/potenciometro"
	  )->ContentAsString();

	lblPotenciometro->Caption = pot;
   }
	 catch(...)
   {
	lblPotenciometro->Caption = "Erro";
   }


     try
	{
      String presenca =
	  NetHTTPClient1->Get(
		"http://" + ESP_IP + "/presenca"
	  )->ContentAsString();

	lblPresenca->Caption = presenca;
	}
	  catch(...)
	{
	  lblPresenca->Caption = "Erro";
	}

    try
	{
	  String prox =
	  NetHTTPClient1->Get(
		"http://" + ESP_IP + "/proximidade"
	  )->ContentAsString();

	  lblProximidade->Caption =
	  prox + " cm";
	}
	  catch(...)
	{
	  lblProximidade->Caption = "Erro";
    }

}
//---------------------------------------------------------------------------
void __fastcall TForm1::BtnLedOnClick(TObject *Sender)
{
      NetHTTPClient1->Get(
      "http://" + ESP_IP + "/led?estado=on"
    );
}
//---------------------------------------------------------------------------
void __fastcall TForm1::BtnLedOffClick(TObject *Sender)
{
      NetHTTPClient1->Get(
      "http://" + ESP_IP + "/led?estado=off"
	);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::BtnRele1ONClick(TObject *Sender)
{
       NetHTTPClient1->Get(
	  "http://" + ESP_IP + "/rele1?estado=on"
	);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::BtnRele1OFFClick(TObject *Sender)
{
	  NetHTTPClient1->Get(
	  "http://" + ESP_IP + "/rele1?estado=off"
	);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::BtnRele2ONClick(TObject *Sender)
{
         NetHTTPClient1->Get(
	  "http://" + ESP_IP + "/rele2?estado=on"
	);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::BtnRele2OFFClick(TObject *Sender)
{
  	  NetHTTPClient1->Get(
	  "http://" + ESP_IP + "/rele2?estado=off"
	);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
  ESP_IP = edtIpServidor->Text;
  lblIPServidor->Caption = "IP do Servidor : "+ ESP_IP;
}
//---------------------------------------------------------------------------
