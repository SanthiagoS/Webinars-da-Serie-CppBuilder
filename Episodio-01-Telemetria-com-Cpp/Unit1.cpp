//---------------------------------------------------------------------------
#include <vector>
#include <FMX.Objects3D.hpp>
#include <System.Math.Vectors.hpp>
#include <fmx.h>
#pragma hdrstop
#include "Unit1.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm1 *Form1;

const int MAX_PONTOS_ORBITA = 200;
float latAtual = 0.0f;
float lonAtual = 0.0f;

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
  busy = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
    modoAutomatico = false;

	ResetarTelemetria(Sender);

    timerAnimation->Interval = 16;
    timerAnimation->Enabled = true;

    timerTelemetry->Enabled = false;

    Switch1->IsChecked = false;
    Switch1->OnSwitch = Switch1Switch;

	// =========================
	   // BLOCO DE ANIMAÇÃO 3D //
    // =========================

	SphereSatelite->Visible = false;

    Model3D1->Position->X = 0;
	Model3D1->Position->Y = 0;
	Model3D1->Position->Z = 0;
	Model3D1->Scale->X = 1;
	Model3D1->Scale->Y = 1;
	Model3D1->Scale->Z = 1;
	Model3D1->Visible = true;
	Model3D1->Opacity = 1;

	SphereAtmosfera->Scale->X = 5;
	SphereAtmosfera->Scale->Y = 5;
	SphereAtmosfera->Scale->Z = 5;
	SphereAtmosfera->MaterialSource = LightMaterialSource1;


	LightMaterialSource1->Diffuse = TAlphaColorRec::Null;
	LightMaterialSource1->Emissive = 0xFF00BFFF;

	SphereAtmosfera->HitTest = false;
	SphereAtmosfera->TwoSide = true;
	SphereAtmosfera->Scale->X = 1.05;

	Viewport3D1->Align = TAlignLayout::Client;
	Viewport3D1->Color = TAlphaColorRec::Black;
	Viewport3D1->UsingDesignCamera = false;

	DummyTilt->Parent = Viewport3D1;
	DummyRotation->Parent = DummyTilt;
	DummyOrbit->Parent = DummyRotation;

	SphereEarth->Parent = DummyRotation;
	Model3D1->Parent = DummyOrbit;

	DummyTilt->Position->Point = Point3D(0,0,0);
	DummyRotation->Position->Point = Point3D(0,0,0);
	DummyOrbit->Position->Point = Point3D(0,0,0);
	SphereEarth->Position->Point = Point3D(0,0,0);

	DummyTilt->RotationAngle->Z = 23.44;

	SphereEarth->RotationAngle->X = 0;
	SphereEarth->RotationAngle->Y = 50;

	//Tamanho Terra
	//CUIDADO!! Se aumentar a terra terá de aumentar o efeito da *AZUL BORDA
	SphereEarth->Width = 5;
	SphereEarth->Height = 5;
	SphereEarth->Depth = 5;

	//EfeitoTerra no Viewport3D
	SphereAtmosfera->Parent = Viewport3D1;

	// ⚠️ POSIÇÃO ABSOLUTA
	SphereAtmosfera->Position->X = 0;
	SphereAtmosfera->Position->Y = 0;
	SphereAtmosfera->Position->Z = 0;

	// ⚠️ TAMANHO BEM MAIOR (*AZUL BORDA)
	SphereAtmosfera->Width  = 5.08;
	SphereAtmosfera->Height = 5.08;
	SphereAtmosfera->Depth  = 5.08;

	SphereAtmosfera->TwoSide = true;
	SphereAtmosfera->Opacity = 0.08;

	// 🔴 MATERIAL SIMPLES
	TLightMaterialSource *mat = new TLightMaterialSource(this);
	// remove preenchimento sólido
	mat->Diffuse  = TAlphaColorRec::Null;
	// brilho suave (Cor azul torno terra)
	mat->Emissive = TAlphaColorRec::Skyblue;

	SphereAtmosfera->MaterialSource = mat;
	SphereAtmosfera->SubdivisionsAxes = 64;

	Model3D1->Width = 1;
	Model3D1->Height = 1;
	Model3D1->Depth = 1;

	Camera1->Parent = Viewport3D1;
	Camera1->Position->X = 0;
    Camera1->Position->Y = -8;
    Camera1->Position->Z = -10;
    Camera1->RotationAngle->X = 30;

    Light1->Parent = Viewport3D1;
    Light1->LightType = TLightType::Directional;
	Light1->Position->X = -10;
	Light1->Position->Y = -0;
	Light1->Position->Z = -10;

	//Altere para seu diretório onde hospedou o projeto, apontando como o exemplo para a imagem.
    TextureMaterialSourceEarth->Texture->LoadFromFile(
        "C:\\Users\\thiago.silva\\Desktop\\TelemetriaCPP\\img\\earth1.jpg"
    );

    SphereEarth->MaterialSource = TextureMaterialSourceEarth;
    SphereEarth->TwoSide = true;
    SphereEarth->WrapMode = TMeshWrapMode::Original;
    SphereEarth->SubdivisionsAxes = 64;
    SphereEarth->SubdivisionsHeight = 64;

	//"BackGround Stars"
	TBitmap *bmp = new TBitmap();
	//Altere para seu diretório onde hospedou o projeto, apontando como o exemplo para a imagem.
	bmp->LoadFromFile("C:\\Users\\thiago.silva\\Desktop\\TelemetriaCPP\\img\\backgroundStars.jpg");

	Viewport3D1->Color = TAlphaColorRec::Null;
    Rectangle1->Fill->Kind = TBrushKind::Bitmap;
	Rectangle1->Fill->Bitmap->Bitmap->LoadFromFile("C:\\Users\\thiago.silva\\Desktop\\TelemetriaCPP\\img\\backgroundStars.jpg");
	Rectangle1->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;

	ApplyEarthRotation();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::timerAnimationTimer(TObject *Sender)
{
	//Terra gira
	DummyRotation->RotationAngle->Y += 0.05f;
	//EfeitoAtmosfera
    SphereAtmosfera->RotationAngle->Y += 0.01f;
	//Satélite automático
	if (modoAutomatico && temDados)
	{
		tempoOrbita += 0.05f;
		UpdateTelemetry(tempoOrbita);
		UpdateSatellitePosition(0, tempoOrbita * 10);
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::tbXChange(TObject *Sender)
{
	Model3D1->RotationAngle->X = tbX->Value;    // Pitch
	Model3D1->RotationAngle->Y = tbY->Value;    // Yaw
	Model3D1->RotationAngle->Z = tbZ->Value;    // Roll
}
//---------------------------------------------------------------------------
void __fastcall TForm1::tbYChange(TObject *Sender)
{
	tbXChange(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::tbZChange(TObject *Sender)
{
	tbXChange(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateTelemetry(float t)
{
	float altitude = 400 + 5 * sin(t);      // suave
	float velocity = 7.67 + 0.02 * cos(t);  // constante
    float latAtual = 0.0f;
	float lonAtual = 0.0f;

	lblAltitude->Text = FormatFloat("0.0 km", altitude);
	lblOrbitalVelocity->Text = FormatFloat("0.00 km/s", velocity);
}
//---------------------------------------------------------------------------
bool __fastcall TForm1::IsAutoMode()
{
	return Switch1->IsChecked; // true = automático
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Switch1Switch(TObject *Sender)
{
   modoAutomatico = Switch1->IsChecked;

	if (modoAutomatico) // AUTOMÁTICO
	{
		lblStatus->Text = "CONECTANDO...";
		timerTelemetry->Enabled = true;
	}
	else                // MANUAL
	{
		timerTelemetry->Enabled = false;
		ResetarTelemetria(Sender);
	}
}
//---------------------------------------------------------------------------
void TForm1::ApplyEarthRotation()
{
   float velocidadeTerra = tbTerraVelocidade->Value * 0.05;

	rotacaoY += velocidadeTerra;

	SphereEarth->RotationAngle->X = tbTerraX->Value;
	SphereEarth->RotationAngle->Y = tbTerraY->Value + rotacaoY;
	SphereEarth->RotationAngle->Z = tbTerraZ->Value;

	lblTX->Text = "X: " + String(tbTerraX->Value);
	lblTY->Text = "Y: " + String(tbTerraY->Value);
	lblTZ->Text = "Z: " + String(tbTerraZ->Value);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::tbTerraXChange(TObject *Sender)
{
  ApplyEarthRotation();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::tbTerraYChange(TObject *Sender)
{
  ApplyEarthRotation();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::tbTerraZChange(TObject *Sender)
{
  ApplyEarthRotation();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::edtTerraXChange(TObject *Sender)
{
  tbTerraX->Value = StrToFloat(edtTerraX->Text);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::edtTerraYChange(TObject *Sender)
{
  tbTerraY->Value = StrToFloat(edtTerraY->Text);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::edtTerraZChange(TObject *Sender)
{
  tbTerraZ->Value = StrToFloat(edtTerraZ->Text);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::tbTerraVelocidadeChange(TObject *Sender)
{
  ApplyEarthRotation();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormShow(TObject *Sender)
{
	//Valor padrão posição Terra
	 tbTerraX->Value = 49.0909080505371;
	 edtTerraX->Text = 49.0909080505371;
	 tbTerraY->Value = -128.571426391602;
	 edtTerraY->Text = -128.571426391602;
	 tbTerraZ->Value = 8;
	 edtTerraZ->Text = 8;

	 SphereAtmosfera->Scale->X = 1.02;
	 SphereAtmosfera->Scale->Y = 1.02;
	 SphereAtmosfera->Scale->Z = 1.02;
	 SphereAtmosfera->MaterialSource = LightMaterialSource1;

	 LightMaterialSource1->Diffuse = TAlphaColorRec::Null; // transparente

	 SphereAtmosfera->HitTest = false;
	 SphereAtmosfera->TwoSide = true;
	 LightMaterialSource1->Emissive = 0xFF00BFFF;          // azul espacial
	 SphereAtmosfera->Scale->X = 1.05;
}
//---------------------------------------------------------------------------
void  __fastcall TForm1::UpdateSatelliteData()
{
	float latitude;
	float longitude;

	if (IsAutoMode())
	{
		GetDemoSimulationData(latitude, longitude);
        lblStatus->Text = "Fonte: Demo Local";
	}
	else
	{
		return;
	}

	lblLatitude->Text = "Lat : " + FloatToStrF(latitude, ffFixed, 8, 4);
	lblLongitude->Text = "Lon : " + FloatToStrF(longitude, ffFixed, 8, 4);
}
//---------------------------------------------------------------------------

// DEMO SIMULATION LOCAL
// Esta função gera coordenadas fictícias de latitude/longitude
// para demonstração offline do projeto.
//
// Utilizar quando:
//
// 1) Cliente ainda não possui ambiente Linux configurado
// 2) InfluxDB não está disponível
// 3) Demonstração rápida sem infraestrutura
// 4) Testes visuais da animação 3D
//
// Em ambiente real, os dados devem vir de:
//
// BuscarTelemetria()
// -> Rocky Linux
// -> InfluxDB
// -> API HTTP
//
// ------------------------------------------------------------
void __fastcall TForm1::GetDemoSimulationData(float &lat, float &lon)
{
//	// SIMULAÇÃO (Rocky real)
    static float t = 0;
	t += 0.1;

	lat = -23.55 + sin(t) * 5;   // varia latitude
	lon = -46.63 + cos(t) * 5;   // varia longitude
}
//---------------------------------------------------------------------------
void TForm1::UpdateSatellitePosition(float lat, float lon)
{

	float radius = 3;

	float radLat = DegToRad(lat);
	float radLon = DegToRad(lon);

	float x = radius * cos(radLat) * cos(radLon);
	float y = radius * sin(radLat);
	float z = radius * cos(radLat) * sin(radLon);

	Model3D1->Scale->X = 1;
	Model3D1->Scale->Y = 1;
	Model3D1->Scale->Z = 1;

	// posição satélite
	Model3D1->Position->X = x;
	Model3D1->Position->Y = y;
	Model3D1->Position->Z = z;

	// criar ponto da trilha
	TSphere *ponto = new TSphere(this);
	ponto->Parent = DummyOrbit;

	ponto->Width = 0.03;
	ponto->Height = 0.03;
	ponto->Depth = 0.03;

    TLightMaterialSource *mat = new TLightMaterialSource(this);
	mat->Diffuse = (TAlphaColor)0x55FFFF00; // amarelo com transparência
    ponto->MaterialSource = mat;


	ponto->Position->X = Model3D1->Position->X;
	ponto->Position->Y = Model3D1->Position->Y;
	ponto->Position->Z = Model3D1->Position->Z;

	trilhaOrbita.push_back(ponto);

    for (int i = 0; i < trilhaOrbita.size(); i++)
	{
    float alpha = (float)i / trilhaOrbita.size();

	TLightMaterialSource *matFade = new TLightMaterialSource(this);

    // cor amarela com transparência progressiva
    int a = (int)(alpha * 255);
    TAlphaColor cor = (a << 24) | 0x00FFFF00;

    matFade->Diffuse = cor;

    trilhaOrbita[i]->MaterialSource = matFade;
   }

	// limite
    const int MAX_PONTOS_ORBITA = 200;

	if (trilhaOrbita.size() > MAX_PONTOS_ORBITA)
    {
        trilhaOrbita[0]->DisposeOf();
		trilhaOrbita.erase(trilhaOrbita.begin());
	}

}
//---------------------------------------------------------------------------
String __fastcall TForm1::BuscarTelemetria()
{
	//Rede Home
	String url = "http://192.168.15.19:8086/api/v2/query?org=iot-lab";

	//Rede Work
	//String url = "http://(Coloque o IP de sua empresa - secundário):8086/api/v2/query?org=iot-lab";

	TNetHTTPClient *client = new TNetHTTPClient(nullptr);

	client->ConnectionTimeout = 3000;
	client->ResponseTimeout = 3000;
	//Gere seu TOKEN e informe abaixo
	client->CustomHeaders["Authorization"] = "Token zQ3WUNhgvA6RxfT0Ldpn9CXSqGUtuA2Ri6dJeIPrjqlLvD_758l8SJ4-D-19pLOaxHwk7vatjhWUCjwRzOqfDQ==";


	client->ContentType = "application/vnd.flux";

	String flux =
		"from(bucket:\"sensores_test\")"
		" |> range(start: -1m)"
		" |> filter(fn: (r) => r._measurement == \"satellite\")"
		" |> filter(fn: (r) => r._field == \"lat\" or r._field == \"lon\")"
        " |> last()";

    TStringStream *body = new TStringStream(flux);
	TStringStream *response = new TStringStream(String("")); //

	String result = "";

    try
	 {
	   client->Post(url, body, response);
	   result = response->DataString;
	 }

	catch (...)
	{
	   result = "";
	}

	delete body;
	delete response;
	delete client;

	return result;
}
//---------------------------------------------------------------------------
 void __fastcall TForm1::AtualizarTelemetriaAsync()
{

if (!modoAutomatico) return;

	if (busy.exchange(true)) return;

	TTask::Run([this]()
	{
		String lat = "";
		String lon = "";
		bool conectado = false;
		String erroMsg = "";

		try
		{
			String data = BuscarTelemetria();

			if (data.Trim().IsEmpty())
				throw Exception(L"SEM DADOS");

			std::unique_ptr<TStringList> lines(new TStringList());
			lines->Text = data;

			for (int i = 0; i < lines->Count; i++)
			{
				String line = lines->Strings[i];

				if (line.Pos("_value") > 0)
					continue;

				std::unique_ptr<TStringList> cols(new TStringList());
				cols->Delimiter = ',';
				cols->StrictDelimiter = true;
				cols->DelimitedText = line;

				if (cols->Count < 7)
					continue;

                String field = cols->Strings[cols->Count - 2].Trim().Replace("\"", "");
				String value = cols->Strings[cols->Count - 3].Trim().Replace("\"", "");

                if (field == "lat") lat = value;
				if (field == "lon") lon = value;
			}

            if (!lat.IsEmpty() && !lon.IsEmpty())
                conectado = true;
			else
				throw Exception(L"SEM COORDENADAS");
        }
		catch (Exception &e)
		{
			erroMsg = e.Message;
			conectado = false;
		}
		catch (...)
		{
			erroMsg = "ERRO";
			conectado = false;
		}

        TThread::Queue(nullptr, [this, lat, lon, conectado, erroMsg]()
		{
			if (conectado)
			{
                lblLatitude->Text = "Lat : " + lat;
				lblLongitude->Text = "Lon : " + lon;
				lblStatus->Text = "Online - RECEBENDO DADOS";
                lblLatitude->TextSettings->FontColor = TAlphaColorRec::Yellow;
				lblLongitude->TextSettings->FontColor = TAlphaColorRec::Yellow;
				temDados = true;
			}
            else
			{
				lblStatus->Text = "Não obtendo dados de Simulador";

				lblLatitude->Text = "NO DATA";
				lblLongitude->Text = "NO DATA";
				lblLatitude->TextSettings->FontColor = TAlphaColorRec::Red;
                lblLongitude->TextSettings->FontColor = TAlphaColorRec::Red;

				lblAltitude->Text = "----";
				lblOrbitalVelocity->Text = "----";

				temDados = false;
			}

			busy = false;
		});
	});
}
//---------------------------------------------------------------------------
void __fastcall TForm1::timerTelemetryTimer(TObject *Sender)
{
		if (modoAutomatico)
	{
		AtualizarTelemetriaAsync();
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ResetarTelemetria(TObject *Sender)
{
    lblAltitude->Text = "----";
	lblOrbitalVelocity->Text = "----";

	lblLatitude->Text = "Lat : ----";
    lblLongitude->Text = "Lon : ----";

	lblStatus->Text = "MANUAL";

	temDados = false;
}
//---------------------------------------------------------------------------
void TForm1::ProcessarTelemetria(String data, double &lat, double &lon)
{
    lat = 0;
    lon = 0;

    TStringList *linhas = new TStringList();
    linhas->Text = data;

    for (int i = 0; i < linhas->Count; i++)
    {
        String linha = linhas->Strings[i];

        if (linha.Pos(",lat,") > 0)
        {
            TStringList *cols = new TStringList();
            cols->Delimiter = ',';
            cols->DelimitedText = linha;

            if (cols->Count > 6)
            {
                lat = StrToFloatDef(cols->Strings[6], 0);
            }

            delete cols;
        }

        if (linha.Pos(",lon,") > 0)
        {
            TStringList *cols = new TStringList();
            cols->Delimiter = ',';
            cols->DelimitedText = linha;

            if (cols->Count > 6)
            {
                lon = StrToFloatDef(cols->Strings[6], 0);
            }

            delete cols;
		}
	}

    delete linhas;
}

