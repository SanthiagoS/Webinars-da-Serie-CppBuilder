//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
    FSwitchKnobX = 8;
    FSwitchTargetX = 8;
    FConveyorOffset = 0;
    FCurrentSpeed = 0;
}
//---------------------------------------------------------------------------
namespace
{
    TColor CorporateColor(int red, int green, int blue)
    {
        return static_cast<TColor>(RGB(red, green, blue));
    }

    TColor SurfaceColor(TControl *Control)
    {
        if (Control && Control->Parent)
        {
            if (TPanel *panel = dynamic_cast<TPanel*>(Control->Parent))
                return panel->Color;

            if (TForm *form = dynamic_cast<TForm*>(Control->Parent))
                return form->Color;
        }

        return CorporateColor(245, 247, 250);
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateTelemetry()
{
    UpdateDashboard();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
    UpdateDashboard();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AddTelemetryLog(const String &Message)
{
    Memo1->Lines->Add(FormatDateTime("hh:nn:ss", Now()) + " | " + Message);

    while (Memo1->Lines->Count > 300)
        Memo1->Lines->Delete(0);

    Memo1->SelStart = Memo1->Text.Length();
    Memo1->SelLength = 0;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ApplyEmergencyUi(bool Active)
{
    if (Active)
    {
        lblStatus->Caption = "Status: EMERGENCIA";
        lblStatus->Font->Color = CorporateColor(227, 77, 77);
        lblVelocidade->Caption = "Speed: 0";
        imgVerde->Visible = false;
        imgAmarelo->Visible = false;
        imgVermelho->Visible = true;
        return;
    }

    lblStatus->Font->Color = CorporateColor(235, 238, 242);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DrawEmergencySwitch()
{
    TCanvas *canvas = pbEmergenciaSwitch->Canvas;
    const int w = pbEmergenciaSwitch->Width;
    const int h = pbEmergenciaSwitch->Height;
    const bool active = (swEmergencia->State == tssOn);
    const int press = FSwitchKnobX;
    const String stateText = active ? "EMERGENCIA" : "NORMAL";

    canvas->Brush->Color = SurfaceColor(pbEmergenciaSwitch);
    canvas->Pen->Color = SurfaceColor(pbEmergenciaSwitch);
    canvas->Rectangle(0, 0, w, h);

    canvas->Brush->Color = CorporateColor(24, 27, 29);
    canvas->Pen->Color = CorporateColor(255, 190, 56);
    canvas->Pen->Width = 2;
    canvas->RoundRect(2, 4, w - 2, h - 4, 18, 18);

    canvas->Pen->Width = 3;
    canvas->Pen->Color = CorporateColor(255, 190, 56);
    for (int x = 10; x < w - 10; x += 24)
    {
        canvas->MoveTo(x, h - 15);
        canvas->LineTo(x + 16, h - 5);
    }

    canvas->Brush->Color = CorporateColor(82, 87, 92);
    canvas->Pen->Color = CorporateColor(15, 17, 19);
    canvas->Pen->Width = 2;
    canvas->Ellipse(17, 13 + press, 77, 61 + press);

    canvas->Brush->Color = active ? CorporateColor(255, 73, 64) : CorporateColor(190, 30, 30);
    canvas->Pen->Color = active ? CorporateColor(255, 174, 160) : CorporateColor(111, 19, 19);
    canvas->Ellipse(12, 7 + press, 82, 53 + press);

    canvas->Brush->Color = active ? CorporateColor(255, 135, 122) : CorporateColor(227, 77, 77);
    canvas->Pen->Color = canvas->Brush->Color;
    canvas->Ellipse(28, 14 + press, 58, 29 + press);

    canvas->Brush->Style = bsClear;
    canvas->Font->Name = "Segoe UI Semibold";
    canvas->Font->Size = 10;
    canvas->Font->Color = active ? CorporateColor(255, 219, 104) : clWhite;
    canvas->TextOut(96, 14, "BOTAO DE");
    canvas->Font->Size = 12;
    canvas->TextOut(96, 31, "EMERGENCIA");
    canvas->Font->Size = 8;
    canvas->Font->Color = active ? CorporateColor(255, 96, 82) : CorporateColor(125, 236, 160);
    canvas->TextOut(96, 52, stateText);
    canvas->Brush->Style = bsSolid;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::StartSwitchAnimation(bool Active)
{
    FSwitchTargetX = Active ? 7 : 0;
    tmSwitchAnimation->Enabled = true;
    pbEmergenciaSwitch->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SendJsonPost(const String &Url, const String &JsonBody)
{
    TStringStream *Body = new TStringStream(JsonBody);

    try
    {
        IdHTTP1->Request->ContentType = "application/json";
        IdHTTP1->Post(Url, Body);
    }
    __finally
    {
        delete Body;
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::swEmergenciaClick(TObject *Sender)
{
    try
    {
        if (swEmergencia->State == tssOn)
        {
			SendJsonPost("http://localhost:3000/emergency_stop", "{}");

            FCurrentSpeed = 0;
			ApplyEmergencyUi(true);
			StartSwitchAnimation(true);
            DrawConveyorImage();
			AddTelemetryLog("EMERGENCIA acionada: POST /emergency_stop");
			return;
		}

		SendJsonPost("http://localhost:3000/set_speed", "{\"speed\":0}");

        FCurrentSpeed = 0;
        ApplyEmergencyUi(false);
        StartSwitchAnimation(false);
        DrawConveyorImage();
        AddTelemetryLog("Emergencia desativada: POST /set_speed speed=0");
    }
    catch (const Exception &e)
    {
        AddTelemetryLog("ERRO controle emergencia: " + e.Message);
        lblStatus->Caption = "Status: MCP ERROR";
        lblStatus->Font->Color = CorporateColor(227, 77, 77);
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::pbEmergenciaSwitchClick(TObject *Sender)
{
    swEmergencia->State = (swEmergencia->State == tssOn) ? tssOff : tssOn;
    swEmergenciaClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::pbEmergenciaSwitchPaint(TObject *Sender)
{
    DrawEmergencySwitch();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::tmSwitchAnimationTimer(TObject *Sender)
{
    const int distance = FSwitchTargetX - FSwitchKnobX;

    if (distance == 0)
    {
        tmSwitchAnimation->Enabled = false;
        return;
    }

    int step = distance / 3;
    if (step == 0)
        step = (distance > 0) ? 1 : -1;

    FSwitchKnobX += step;

    if ((step > 0 && FSwitchKnobX > FSwitchTargetX) ||
        (step < 0 && FSwitchKnobX < FSwitchTargetX))
    {
        FSwitchKnobX = FSwitchTargetX;
    }

    pbEmergenciaSwitch->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::tmConveyorAnimationTimer(TObject *Sender)
{
    const int step = ConveyorAnimationStep();

    if (step <= 0)
        return;

    FConveyorOffset = (FConveyorOffset + step) % 10000;
    DrawConveyorImage();
}
//---------------------------------------------------------------------------
int __fastcall TForm1::ConveyorAnimationStep() const
{
    if ((swEmergencia->State == tssOn) || (FCurrentSpeed <= 0))
        return 0;

    const int normalizedSpeed = (FCurrentSpeed <= 10) ? (FCurrentSpeed * 10) : FCurrentSpeed;
    const int speed = (normalizedSpeed > 100) ? 100 : normalizedSpeed;
    return 1 + ((speed * 11) / 100);
}
//---------------------------------------------------------------------------
bool __fastcall TForm1::CheckPort(int Port)
{
    TIdTCPClient *Client = new TIdTCPClient(nullptr);
    bool connected = false;

    try
    {
        Client->Host = "127.0.0.1";
        Client->Port = Port;
        Client->ConnectTimeout = 250;
        Client->ReadTimeout = 250;
        Client->Connect();
        connected = Client->Connected();
        Client->Disconnect();
    }
    catch (...)
    {
        connected = false;
    }

    delete Client;
    return connected;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DrawConnectionLed(TImage *Image, bool Connected)
{
    DrawLedImage(
        Image,
        Connected ? CorporateColor(40, 214, 120) : CorporateColor(227, 77, 77),
        Connected ? CorporateColor(130, 255, 184) : CorporateColor(255, 150, 150));
}
//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateConnectionIndicators(bool MainConnected, bool BridgeConnected)
{
    DrawConnectionLed(imgMcpPrincipal, MainConnected);
    DrawConnectionLed(imgMcpBridge, BridgeConnected);

    lblMcpPrincipal->Caption = MainConnected ? "MCP Principal 3000: ONLINE" : "MCP Principal 3000: OFFLINE";
    lblMcpBridge->Caption = BridgeConnected ? "MCP Bridge 3100: ONLINE" : "MCP Bridge 3100: OFFLINE";

    lblMcpPrincipal->Font->Color = MainConnected ? CorporateColor(125, 236, 160) : CorporateColor(255, 132, 132);
    lblMcpBridge->Font->Color = BridgeConnected ? CorporateColor(125, 236, 160) : CorporateColor(255, 132, 132);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateDashboard()
{
    TStringStream *Response = nullptr;
    TJSONObject *obj = nullptr;
    const bool mainConnected = CheckPort(3000);
    const bool bridgeConnected = CheckPort(3100);

    UpdateConnectionIndicators(mainConnected, bridgeConnected);

    try
    {
        Response = new TStringStream();

        IdHTTP1->Get("http://localhost:3000/api/dashboard", Response);

        String json = Response->DataString;
        obj = static_cast<TJSONObject*>(TJSONObject::ParseJSONValue(json));

        if (!obj)
        {
            FCurrentSpeed = 0;
            DrawConveyorImage();
            lblStatus->Caption = "Status: JSON INVALID";
            AddTelemetryLog("JSON INVALID: " + json);
        }
        else
        {
            TJSONValue *vSpeed = obj->GetValue("speed");
            int speed = (vSpeed ? StrToIntDef(vSpeed->Value(), 0) : 0);
            FCurrentSpeed = (swEmergencia->State == tssOn) ? 0 : speed;

            TJSONValue *vLevel = obj->GetValue("level");
            String level = (vLevel ? vLevel->Value() : "UNKNOWN");

            TJSONValue *vLed = obj->GetValue("led");
            String led = (vLed ? vLed->Value() : "OFF");

            TJSONValue *vTimestamp = obj->GetValue("timestamp");
            String timestamp = (vTimestamp ? vTimestamp->Value() : "sem timestamp");

            if (swEmergencia->State == tssOn)
            {
                ApplyEmergencyUi(true);
            }
            else
            {
                ApplyEmergencyUi(false);
                lblVelocidade->Caption = "Speed: " + IntToStr(speed);
                lblStatus->Caption = "Status: " + level;

                imgVerde->Visible = (led == "GREEN");
                imgAmarelo->Visible = (led == "YELLOW");
                imgVermelho->Visible = (led == "RED");
            }

            DrawConveyorImage();

            AddTelemetryLog(
                "timestamp=" + timestamp +
                " | speed=" + IntToStr(speed) +
                " | level=" + level +
                " | led=" + led +
                " | mcp3000=" + String(mainConnected ? "ONLINE" : "OFFLINE") +
                " | bridge3100=" + String(bridgeConnected ? "ONLINE" : "OFFLINE") +
                " | emergency=" + String(swEmergencia->State == tssOn ? "ON" : "OFF"));
        }
    }
    catch (const Exception &e)
    {
        FCurrentSpeed = 0;
        DrawConveyorImage();
        lblStatus->Caption = mainConnected ? "Status: MCP ERROR" : "Status: MCP OFFLINE";
        lblStatus->Font->Color = CorporateColor(227, 77, 77);
        AddTelemetryLog("MCP ERROR: " + e.Message);
    }

    if (obj) delete obj;
    if (Response) delete Response;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ConfigureImage(TImage *Image)
{
    Image->Center = true;
    Image->Proportional = true;
    Image->Stretch = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DrawConveyorImage()
{
    TBitmap *bitmap = imgEsteira->Picture->Bitmap;
    bitmap->SetSize(imgEsteira->Width, imgEsteira->Height);

    TCanvas *canvas = bitmap->Canvas;
    const int w = bitmap->Width;
    const int h = bitmap->Height;
    const int beltTop = 92;
    const int beltBottom = 156;
    const int phase = FConveyorOffset % 48;
    const bool emergency = (swEmergencia->State == tssOn);
    const bool moving = (!emergency && (FCurrentSpeed > 0));

    canvas->Brush->Color = SurfaceColor(imgEsteira);
    canvas->FillRect(TRect(0, 0, w, h));

    canvas->Pen->Color = CorporateColor(57, 62, 66);
    canvas->Pen->Width = 2;
    canvas->Brush->Color = CorporateColor(31, 35, 38);
    canvas->RoundRect(8, 10, w - 8, h - 10, 18, 18);

    canvas->Brush->Color = CorporateColor(43, 48, 52);
    canvas->Pen->Color = CorporateColor(20, 23, 25);
    canvas->RoundRect(22, 24, w - 22, h - 24, 12, 12);

    canvas->Pen->Width = 4;
    for (int x = 34; x < w - 40; x += 34)
    {
        canvas->Pen->Color = CorporateColor(255, 190, 56);
        canvas->MoveTo(x, 38);
        canvas->LineTo(x + 18, 52);
        canvas->Pen->Color = CorporateColor(17, 18, 20);
        canvas->MoveTo(x + 18, 38);
        canvas->LineTo(x + 36, 52);
    }

    canvas->Pen->Width = 1;
    canvas->Pen->Color = CorporateColor(95, 101, 106);
    canvas->Brush->Color = CorporateColor(95, 101, 106);
    for (int x = 44; x < w - 44; x += 54)
    {
        canvas->Ellipse(x, 62, x + 8, 70);
        canvas->Ellipse(x, h - 42, x + 8, h - 34);
    }

    canvas->Brush->Color = CorporateColor(78, 84, 89);
    canvas->Pen->Color = CorporateColor(47, 52, 56);
    canvas->Rectangle(88, beltBottom + 12, 124, h - 36);
    canvas->Rectangle(w - 124, beltBottom + 12, w - 88, h - 36);

    canvas->Pen->Color = CorporateColor(11, 13, 15);
    canvas->Pen->Width = 3;
    canvas->Brush->Color = emergency ? CorporateColor(84, 40, 40) : (moving ? CorporateColor(37, 45, 52) : CorporateColor(48, 51, 54));
    canvas->RoundRect(34, beltTop, w - 34, beltBottom, 20, 20);

    canvas->Pen->Width = 2;
    canvas->Pen->Color = emergency ? CorporateColor(145, 65, 65) : (moving ? CorporateColor(98, 121, 134) : CorporateColor(80, 86, 91));
    for (int x = 40 - phase; x < w - 48; x += 48)
    {
        canvas->MoveTo(x, beltTop + 8);
        canvas->LineTo(x + 34, beltBottom - 8);
    }

    canvas->Pen->Color = CorporateColor(14, 17, 19);
    canvas->Brush->Color = CorporateColor(15, 18, 20);
    canvas->Ellipse(38, beltTop - 18, 128, beltBottom + 18);
    canvas->Ellipse(w - 128, beltTop - 18, w - 38, beltBottom + 18);

    canvas->Brush->Color = CorporateColor(121, 130, 137);
    canvas->Pen->Color = CorporateColor(73, 79, 84);
    canvas->Ellipse(62, beltTop + 6, 104, beltBottom - 6);
    canvas->Ellipse(w - 104, beltTop + 6, w - 62, beltBottom - 6);

    canvas->Brush->Color = CorporateColor(214, 145, 54);
    canvas->Pen->Color = CorporateColor(115, 74, 28);
    const int travelWidth = w - 190;
    const int boxX = 60 + ((FConveyorOffset * 2) % travelWidth);
    canvas->RoundRect(boxX, beltTop - 34, boxX + 74, beltTop + 8, 8, 8);
    canvas->Pen->Color = CorporateColor(136, 87, 32);
    canvas->MoveTo(boxX + 36, beltTop - 34);
    canvas->LineTo(boxX + 36, beltTop + 8);

    canvas->Brush->Color = emergency ? CorporateColor(227, 77, 77) : (moving ? CorporateColor(40, 214, 120) : CorporateColor(242, 180, 58));
    canvas->Pen->Color = emergency ? CorporateColor(255, 160, 150) : (moving ? CorporateColor(130, 255, 184) : CorporateColor(255, 228, 161));
    canvas->Ellipse(w - 82, 32, w - 48, 66);

    canvas->Brush->Style = bsClear;
    canvas->Font->Name = "Segoe UI Semibold";
    canvas->Font->Size = 9;
    canvas->Font->Color = emergency ? CorporateColor(255, 132, 132) : (moving ? CorporateColor(168, 202, 218) : CorporateColor(255, 228, 161));
	lblStatusEsteira->Caption = (42, 30, emergency ? "ESTEIRA BLOQUEADA" : (moving ? "LINHA INDUSTRIAL EM MOVIMENTO" : "ESTEIRA PARADA - SPEED 0"));
	//lblStatusEsteira->Text =
	canvas->Font->Size = 8;
	canvas->Font->Color = CorporateColor(255, 190, 56);
    //canvas->TextOut(42, h - 34, "VCL CANVAS - READY FOR SKIA/LOTTIE UPGRADE");
    canvas->Brush->Style = bsSolid;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DrawLedImage(TImage *Image, TColor LedColor, TColor GlowColor)
{
    TBitmap *bitmap = Image->Picture->Bitmap;
    bitmap->SetSize(Image->Width, Image->Height);

    TCanvas *canvas = bitmap->Canvas;
    const int w = bitmap->Width;
    const int h = bitmap->Height;
    const int size = (w < h) ? w : h;
    const int left = (w - size) / 2;
    const int top = (h - size) / 2;
    const int outer = (size < 24) ? 1 : 2;
    const int glow = (size < 36) ? 4 : 6;
    const int rim = (size < 36) ? 7 : 12;
    const int core = (size < 36) ? 10 : 18;

    canvas->Brush->Color = SurfaceColor(Image);
    canvas->FillRect(TRect(0, 0, w, h));

    canvas->Pen->Color = CorporateColor(226, 231, 236);
    canvas->Brush->Color = CorporateColor(247, 249, 251);
    canvas->Ellipse(left + outer, top + outer, left + size - outer, top + size - outer);

    canvas->Pen->Color = GlowColor;
    canvas->Brush->Color = GlowColor;
    canvas->Ellipse(left + glow, top + glow, left + size - glow, top + size - glow);

    canvas->Pen->Color = CorporateColor(42, 52, 64);
    canvas->Brush->Color = CorporateColor(42, 52, 64);
    canvas->Ellipse(left + rim, top + rim, left + size - rim, top + size - rim);

    canvas->Pen->Color = LedColor;
    canvas->Brush->Color = LedColor;
    canvas->Ellipse(left + core, top + core, left + size - core, top + size - core);

    canvas->Pen->Color = clWhite;
    canvas->Brush->Color = clWhite;
    canvas->Ellipse(left + (size / 3), top + (size / 4), left + (size / 2), top + (size / 2));
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
    ConfigureImage(imgEsteira);
    ConfigureImage(imgVerde);
    ConfigureImage(imgAmarelo);
    ConfigureImage(imgVermelho);
    ConfigureImage(imgMcpPrincipal);
    ConfigureImage(imgMcpBridge);

    IdHTTP1->ConnectTimeout = 700;
    IdHTTP1->ReadTimeout = 900;

    DrawConveyorImage();
    DrawLedImage(imgVerde, CorporateColor(32, 178, 107), CorporateColor(185, 240, 209));
    DrawLedImage(imgAmarelo, CorporateColor(242, 180, 58), CorporateColor(255, 228, 161));
    DrawLedImage(imgVermelho, CorporateColor(227, 77, 77), CorporateColor(255, 196, 196));
    UpdateConnectionIndicators(false, false);

    Memo1->Lines->Clear();
    Timer1->Interval = 1000;
    tmSwitchAnimation->Interval = 15;
    tmSwitchAnimation->Enabled = false;
    tmConveyorAnimation->Interval = 45;
    tmConveyorAnimation->Enabled = true;
    FSwitchKnobX = (swEmergencia->State == tssOn) ? 7 : 0;
    FSwitchTargetX = FSwitchKnobX;
    FConveyorOffset = 0;
    AddTelemetryLog("Sistema iniciado. Aguardando telemetria MCP.");
}
//---------------------------------------------------------------------------

