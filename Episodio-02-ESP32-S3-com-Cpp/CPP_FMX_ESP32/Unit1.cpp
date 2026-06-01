//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SwitchLEDSwitch(TObject *Sender)
{
      try
    {
        String estado =
          SwitchLED->IsChecked ? "on" : "off";

        NetHTTPClient1->Get(
		  "http://" + ESP_IP +
          "/led?estado=" + estado
        );
    }
    catch(...)
    {
        ShowMessage("Erro ao controlar LED");
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SwitchRele1Switch(TObject *Sender)
{
      try
    {
        String estado =
          SwitchRele1->IsChecked ? "on" : "off";

        NetHTTPClient1->Get(
          "http://" + ESP_IP +
          "/rele1?estado=" + estado
        );
    }
    catch(...)
    {
        ShowMessage("Erro Relé 1");
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SwitchRele2Switch(TObject *Sender)
{
      try
    {
        String estado =
          SwitchRele2->IsChecked ? "on" : "off";

        NetHTTPClient1->Get(
          "http://" + ESP_IP +
          "/rele2?estado=" + estado
        );
    }
    catch(...)
    {
        ShowMessage("Erro Relé 2");
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::BtnBuzzerClick(TObject *Sender)
{
      try
    {
        NetHTTPClient1->Get(
          "http://" + ESP_IP + "/buzzer"
        );
    }
    catch(...)
    {
        ShowMessage("Erro buzzer");
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
  //Temperatura:
  try
  {
	String temp =
	  NetHTTPClient1->Get(
	   "http://" + ESP_IP + "/temperatura"
	  )->ContentAsString();

	LblTemperatura->Text =
	  temp + "°C";
  }
	catch(...)
  {
	LblTemperatura->Text = "--";
  }

  //Proximidade:
  try
  {
    String prox =
      NetHTTPClient1->Get(
       "http://" + ESP_IP + "/proximidade"
      )->ContentAsString();

	lblProximidade->Text =
	  prox + " cm";
  }
	catch(...)
  {
	lblProximidade->Text = "--";
  }

  //Potenciômetro:
  try
  {
	String pot =
	  NetHTTPClient1->Get(
	   "http://" + ESP_IP + "/potenciometro"
	  )->ContentAsString();

	lblPotenciometro->Text = pot;
  }
	catch(...)
  {
	lblPotenciometro->Text = "--";
  }

  //Presença:
  try
  {
	String presenca =
	  NetHTTPClient1->Get(
	   "http://" + ESP_IP + "/presenca"
	  )->ContentAsString();

	lblPresenca->Text = presenca;
  }
	catch(...)
  {
	lblPresenca->Text = "--";
  }
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button1Click(TObject *Sender)
{

  String ip = edtIP->Text;
  TStringDynArray partes = SplitString(ip, ".");


  if (partes.Length != 4)
	{
	  ShowMessage("IP INVÁLIDO!");
      edtIP->Text = "";
	  return;
	}

  bool ipValido = true;

	 for (int i = 0; i < 4; i++)
	{
	int numero;

	//  número
	if (!TryStrToInt(partes[i], numero))
	{
		ipValido = false;
		break;
	}

	// faixa
	if (numero < 0 || numero > 255)
	{
		ipValido = false;
		break;
	}
   }

	  if (!ipValido)
	  {
		ShowMessage("IP inválido!");
	  }
   else
   {
	ShowMessage("IP :" + edtIP->Text + "  associado ao sistema!!");
	pnelIPServer->Visible = false;
	LblIP->Text = "IP ESP32 : " + ip;
	ESP_IP = ip;
   }

  TIdTCPClient *Cliente = new TIdTCPClient(NULL);

   try
   {
	Cliente->Host = ESP_IP; // IP digitado
	Cliente->Port = 80;           // Porta do servidor

	Cliente->ConnectTimeout = 3000; // 3 segundos
	Cliente->Connect();

	// IF conexão realizada
	if (Cliente->Connected())
	{
		Timer1->Enabled = true;
	}
    else
    {
		ShowMessage("Falha na conexão.");
		Timer1->Enabled = true;
    }
  }
  catch (Exception &e)
	{
	  ShowMessage("Não foi possível conectar no IP.");
	}

   Cliente->Disconnect();
   delete Cliente;

}
//---------------------------------------------------------------------------

