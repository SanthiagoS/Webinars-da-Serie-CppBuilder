
//---------------------------------------------------------------------------
#include "WebModuleUnit1.h"
#include "SerialManagerUnit.h"
bool ServidorOnline = true;
//---------------------------------------------------------------------------
#include <memory>
#include <Windows.h>
#include <System.JSON.hpp>
#include <System.NetEncoding.hpp>
#include <System.IOUtils.hpp>
#include <System.DateUtils.hpp>
#include <FMX.Graphics.hpp>
#include <System.UITypes.hpp>

#pragma package(smart_init)
#pragma classgroup "FMX.Controls.TControl"
#pragma link "AdPort"
#pragma link "OoMisc"
#pragma resource "*.dfm"

TComponentClass WebModuleClass = __classid(TWebModule1);

static bool GSerialOpened = false;
//---------------------------------------------------------------------------
namespace
{
    String WebRootDirectory()
    {
        return IncludeTrailingPathDelimiter(
            ExpandFileName(ExtractFilePath(ParamStr(0)) + L"wwwroot"));
    }
    //---------------------------------------------------------------------------
    String ContentTypeForFile(const String &FileName)
    {
        String ext = LowerCase(ExtractFileExt(FileName));

        if ((ext == L".html") || (ext == L".htm"))
            return L"text/html; charset=utf-8";
        if (ext == L".css")
            return L"text/css; charset=utf-8";
        if (ext == L".js")
            return L"application/javascript; charset=utf-8";
        if (ext == L".png")
            return L"image/png";
        if ((ext == L".jpg") || (ext == L".jpeg"))
            return L"image/jpeg";

        return L"application/octet-stream";
    }
    //---------------------------------------------------------------------------
    bool IsInsideWebRoot(const String &FileName, const String &RootDirectory)
    {
        return System::Pos(LowerCase(RootDirectory), LowerCase(FileName)) == 1;
    }
	//---------------------------------------------------------------------------
}
    __fastcall TWebModule1::TWebModule1(TComponent* Owner) : TWebModule(Owner)
    {
        // O handler abaixo serve os arquivos estaticos. O dispatcher nativo estava interceptando CSS.
        WebFileDispatcher1->WebFileExtensions->Clear();
        WebFileDispatcher1->RootDirectory = L"";
    }
    //---------------------------------------------------------------------------
	static uint16_t AlphaColorToRGB565(const TAlphaColor color)
    {
        const unsigned char red =
            static_cast<unsigned char>((color >> 16) & 0xFF);

        const unsigned char green =
            static_cast<unsigned char>((color >> 8) & 0xFF);

        const unsigned char blue = static_cast<unsigned char>(color & 0xFF);

        return static_cast<uint16_t>(
            ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3));
    }
	//---------------------------------------------------------------------------
    static bool SendPhotoToMegaTFT(const String &fileName)
    {
        if (SerialManagerUnit1 == nullptr ||
            !SerialManagerUnit1->IsDeviceConnected(L"TSX-MEGA-001"))
        {
            ::OutputDebugStringW(
                L"[RIW] MEGA nao conectado para envio da foto");

            return false;
        }

        std::unique_ptr<TBitmap> bitmap(new TBitmap());

        // ------------------------------------------------------
        // Carrega e reduz a imagem
        // ------------------------------------------------------

        try {
            bitmap->LoadFromFile(fileName);

            bitmap->Resize(80, 60);
        } catch (...) {
            ::OutputDebugStringW(
                L"[RIW] Falha ao carregar/redimensionar foto para TFT");

            return false;
        }

        // ------------------------------------------------------
        // Mapeia pixels do Bitmap
        // ------------------------------------------------------

        TBitmapData bitmapData;

        if (!bitmap->Map(TMapAccess::Read, bitmapData)) {
            ::OutputDebugStringW(L"[RIW] Falha ao mapear bitmap da foto");

            return false;
        }

        bool success = true;

        // ------------------------------------------------------
        // Envia inicio da imagem
        // ------------------------------------------------------

        if (!SerialManagerUnit1->SendCommandToDevice(
                L"TSX-MEGA-001", L"PHOTO_BEGIN:80,60"))
        {
            success = false;
        }

        // ------------------------------------------------------
        // Envia pixels
        //
        // 80 pixels por linha.
        // Dividimos em 2 comandos de 40 pixels.
        // ------------------------------------------------------

        if (success) {
            for (int y = 0; y < 60 && success; ++y) {
                for (int startX = 0; startX < 80; startX += 40) {
                    String hexPixels = L"";

                    for (int x = startX; x < startX + 40; ++x) {
                        const TAlphaColor color = bitmapData.GetPixel(x, y);

                        const uint16_t rgb565 = AlphaColorToRGB565(color);

                        hexPixels += IntToHex(rgb565, 4);
                    }

                    const String command = L"PHOTO_DATA:" + IntToStr(startX) +
                                           L"," + IntToStr(y) + L":" +
                                           hexPixels;

                    if (!SerialManagerUnit1->SendCommandToDevice(
                            L"TSX-MEGA-001", command))
                    {
                        success = false;

                        break;
                    }

                    // Dá tempo para o MEGA
                    // processar/desenhar o bloco.
                    Sleep(8);
                }
            }
        }

        // ------------------------------------------------------
        // Libera acesso aos pixels
        // ------------------------------------------------------

        bitmap->Unmap(bitmapData);

        if (!success) {
            ::OutputDebugStringW(L"[RIW] Falha durante envio da foto ao MEGA");

            return false;
        }

        Sleep(20);

        // ------------------------------------------------------
        // Finaliza a imagem
        // ------------------------------------------------------

        if (!SerialManagerUnit1->SendCommandToDevice(
                L"TSX-MEGA-001", L"PHOTO_END"))
        {
			::OutputDebugStringW(L"[RIW] Falha ao enviar PHOTO_END");

			return false;
        }

		::OutputDebugStringW(L"[RIW] Foto enviada ao TFT do MEGA");

		return true;
	}
	//---------------------------------------------------------------------------
    void __fastcall TWebModule1::WebModule1DefaultHandlerAction(
    TObject* Sender,
    TWebRequest* Request,
    TWebResponse* Response,
    bool &Handled
)
{
    // HEARTBEAT
    if (Request->PathInfo == "/ping") {
        Response->ContentType = "text/plain";

        if (ServidorOnline)
            Response->Content = "OK";
        else
            Response->Content = "OFFLINE";

        Handled = true;
        return;
    }

    // MULTI DEVICE STATUS
    if (Request->PathInfo == L"/api/devices") {
        Handled = true;

        Response->ContentType =
            L"application/json; charset=utf-8";

        if (SerialManagerUnit1 == nullptr) {
            Response->StatusCode = 503;

            Response->Content =
                L"{"
                L"\"count\":0,"
                L"\"devices\":[],"
                L"\"error\":\"SerialManager indisponivel\""
                L"}";

            return;
        }

        Response->StatusCode = 200;
        Response->Content =
            SerialManagerUnit1->GetDevicesJson();

        return;
    }

    // SERIAL MONITOR
    if (Request->PathInfo == L"/api/serial/monitor") {
        Handled = true;

        Response->ContentType =
            L"application/json; charset=utf-8";

        if (SerialManagerUnit1 == nullptr) {
            Response->StatusCode = 503;

            Response->Content =
                L"{"
                L"\"count\":0,"
                L"\"entries\":[],"
                L"\"error\":\"SerialManager indisponivel\""
                L"}";

            return;
        }

        Response->StatusCode = 200;
        Response->Content =
            SerialManagerUnit1->GetSerialMonitorJson();

        return;
    }

    // SERIAL SEND
    if (Request->PathInfo == L"/api/serial/send") {
        Handled = true;

        Response->ContentType =
            L"application/json; charset=utf-8";

        if (SerialManagerUnit1 == nullptr) {
            Response->StatusCode = 503;

            Response->Content =
                L"{"
                L"\"success\":false,"
                L"\"error\":\"SerialManager indisponivel\""
                L"}";

            return;
        }

        const String body =
            Request->Content;

        std::unique_ptr<TJSONValue> jsonValue(
            TJSONObject::ParseJSONValue(
                body
            )
        );

        TJSONObject* jsonObject =
            dynamic_cast<TJSONObject*>(
                jsonValue.get()
            );

        if (jsonObject == nullptr) {
            Response->StatusCode = 400;

            Response->Content =
                L"{"
                L"\"success\":false,"
                L"\"error\":\"JSON invalido\""
                L"}";

            return;
        }

        TJSONValue* deviceValue =
            jsonObject->GetValue(
                L"device_id"
            );

        TJSONValue* commandValue =
            jsonObject->GetValue(
                L"command"
            );

        const String deviceId =
            deviceValue
                ? deviceValue->Value()
                : L"";

        const String command =
            commandValue
                ? commandValue->Value()
                : L"";

        if (
            deviceId.IsEmpty() ||
            command.IsEmpty()
        ) {
            Response->StatusCode = 400;

            Response->Content =
                L"{"
                L"\"success\":false,"
                L"\"error\":\"Parametros invalidos\""
                L"}";

            return;
        }

        const bool sent =
            SerialManagerUnit1
                ->SendCommandToDevice(
                    deviceId,
                    command
                );

        if (!sent) {
            Response->StatusCode = 500;

            Response->Content =
                L"{"
                L"\"success\":false,"
                L"\"error\":\"Falha ao enviar comando\""
                L"}";

            return;
        }

        Response->StatusCode = 200;

        Response->Content =
            L"{"
            L"\"success\":true"
            L"}";

        return;
    }

    String requestPath =
        Request->PathInfo;

    if (
        requestPath.IsEmpty() ||
        requestPath == L"/"
    ) {
        requestPath =
            L"/index.html";
    }

    String relativePath =
        requestPath;

    while (
        relativePath.Length() > 0 &&
        (
            relativePath[1] == L'/' ||
            relativePath[1] == L'\\'
        )
    ) {
        relativePath.Delete(
            1,
            1
        );
    }

    relativePath =
        StringReplace(
            relativePath,
            L"/",
            L"\\",
            TReplaceFlags()
                << rfReplaceAll
        );

    String rootDirectory =
        WebRootDirectory();

    String fileName =
        ExpandFileName(
            rootDirectory +
            relativePath
        );

    if (
        IsInsideWebRoot(
            fileName,
            rootDirectory
        ) &&
        FileExists(
            fileName
        )
    ) {
        TFileStream* stream =
            new TFileStream(
                fileName,
                fmOpenRead |
                fmShareDenyWrite
            );

        Response->ContentType =
            ContentTypeForFile(
                fileName
            );

        Response->ContentLength =
            stream->Size;

        Response->ContentStream =
            stream;

        Response->FreeContentStream =
            true;

        Handled = true;
        return;
    }

    Response->StatusCode = 404;

    Response->ContentType =
        L"text/plain; charset=utf-8";

    Response->Content =
        L"Arquivo nao encontrado";

    Handled = true;
}
    //---------------------------------------------------------------------------
		void __fastcall TWebModule1::ProcessSerialLine(const String &RawLine)
    {
        String line = RawLine.Trim();

        const int firstBrace = line.Pos(L"{");
        const int lastBrace = line.LastDelimiter(L"}");

        if (firstBrace <= 0 || lastBrace < firstBrace)
            return;

        line = line.SubString(firstBrace, lastBrace - firstBrace + 1);

        std::unique_ptr<System::Json::TJSONValue> value(
            System::Json::TJSONObject::ParseJSONValue(line));

        auto* json = dynamic_cast<System::Json::TJSONObject*>(value.get());

        if (!json)
            return;

        String type;

        if (!json->TryGetValue<String>(L"type", type))
            return;

        ++FPacketsReceived;
        FLastSeen = Now();

        if (type == L"identity")
            ProcessIdentity(json);
        else if (type == L"status")
            ProcessStatus(json);
        else if (type == L"result")
            ProcessResult(json);
        else if (type == L"pong") {
            FDeviceConnected = true;
            FDeviceState = L"connected";
        }
    }
    //---------------------------------------------------------------------------
    void __fastcall TWebModule1::ProcessIdentity(
        System::Json::TJSONObject* Json)
    {
        System::Json::TJSONObject* device = nullptr;

        if (!Json->TryGetValue<System::Json::TJSONObject*>(L"device", device) ||
            !device)
        {
            return;
        }

        device->TryGetValue<String>(L"name", FDeviceName);
        device->TryGetValue<String>(L"id", FDeviceId);
        device->TryGetValue<String>(L"category", FDeviceCategory);
        device->TryGetValue<String>(L"mcu", FDeviceMCU);
        device->TryGetValue<String>(L"clock", FDeviceClock);
        device->TryGetValue<String>(L"firmware", FFirmware);
        device->TryGetValue<String>(L"protocol", FProtocol);

        System::Json::TJSONObject* memory = nullptr;

        if (Json->TryGetValue<System::Json::TJSONObject*>(L"memory", memory) &&
            memory)
        {
            memory->TryGetValue<int>(L"free_ram", FFreeRam);
        }

        System::Json::TJSONArray* capabilities = nullptr;

        if (Json->TryGetValue<System::Json::TJSONArray*>(
                L"capabilities", capabilities) &&
            capabilities)
        {
            for (int i = 0; i < capabilities->Count; ++i) {
                auto* capability = dynamic_cast<System::Json::TJSONObject*>(
                    capabilities->Items[i]);

                if (!capability)
                    continue;

                String id;
                capability->TryGetValue<String>(L"id", id);

                const bool state = GetJsonBool(capability, L"state", false);

                if (id == L"led1")
                    FLed1 = state;
                else if (id == L"led2")
                    FLed2 = state;
                else if (id == L"led3")
                    FLed3 = state;
                else if (id == L"relay1")
                    FRelay1 = state;
                else if (id == L"buzzer1")
                    FBuzzer1 = state;
            }
        }

        FDeviceConnected = true;
        FDeviceState = L"connected";
        FLastSeen = Now();
    }
	//---------------------------------------------------------------------------
    bool __fastcall TWebModule1::GetJsonBool(
        System::Json::TJSONObject* Json, const String &Name, bool DefaultValue)
    {
        System::Json::TJSONValue* value = Json->GetValue(Name);

        if (!value)
            return DefaultValue;

        return SameText(value->Value(), L"true");
    }
	//---------------------------------------------------------------------------
    void __fastcall TWebModule1::ProcessStatus(System::Json::TJSONObject* Json)
    {
        __int64 uptimeValue = 0;

        if (Json->TryGetValue<__int64>(L"uptime_ms", uptimeValue)) {
            FUptimeMs = static_cast<unsigned long>(uptimeValue);
        }

        Json->TryGetValue<int>(L"free_ram", FFreeRam);

        System::Json::TJSONArray* capabilities = nullptr;

        if (Json->TryGetValue<System::Json::TJSONArray*>(
                L"capabilities", capabilities) &&
            capabilities)
        {
            for (int i = 0; i < capabilities->Count; ++i) {
                auto* capability = dynamic_cast<System::Json::TJSONObject*>(
                    capabilities->Items[i]);

                if (!capability)
                    continue;

                String id;
                capability->TryGetValue<String>(L"id", id);

                const bool state = GetJsonBool(capability, L"state", false);

                if (id == L"led1")
                    FLed1 = state;
                else if (id == L"led2")
                    FLed2 = state;
                else if (id == L"led3")
                    FLed3 = state;
                else if (id == L"relay1")
                    FRelay1 = state;
                else if (id == L"buzzer1")
                    FBuzzer1 = state;
            }
        }

        FDeviceConnected = true;
        FDeviceState = L"connected";
        FLastSeen = Now();
    }
	//---------------------------------------------------------------------------
    void __fastcall TWebModule1::ProcessResult(System::Json::TJSONObject* Json)
    {
        String target;

        if (!Json->TryGetValue<String>(L"target", target))
            return;

        const bool value = GetJsonBool(Json, L"value", false);

        if (target == L"led1")
            FLed1 = value;
        else if (target == L"led2")
            FLed2 = value;
        else if (target == L"led3")
            FLed3 = value;
        else if (target == L"relay1")
            FRelay1 = value;
        else if (target == L"buzzer1")
            FBuzzer1 = value;

        FLastSeen = Now();
    }
	//---------------------------------------------------------------------------
    void __fastcall TWebModule1::WebModuleCreate(TObject* Sender)
    {
        ::OutputDebugStringW(L"[RIW] WebModuleCreate iniciado");

        FSerialBuffer = L"";
        FDeviceConnected = false;
        FDeviceState = L"opening_port";

        const int comNumber = 3;

        FPortName = L"COM" + IntToStr(comNumber);

        try {
            if (!GSerialOpened) {
                //            ApdComPort1->ComNumber = comNumber;
                //            ApdComPort1->Baud = 115200;
                //            ApdComPort1->Open = true;
                //
                //            GSerialOpened = true;
                //
                //            ::OutputDebugStringW(
                //                L"[RIW] COM3 aberta com sucesso"
                //            );
                //
                //            FDeviceState =
                //                L"waiting_device";
                //
                //            Sleep(2000);
                //
                //            ::OutputDebugStringW(
                //                L"[RIW] Enviando IDENTIFY"
                //            );
                //
                //            SendSerialCommand(
                //                L"IDENTIFY"
                //            );
            } else {
                ::OutputDebugStringW(L"[RIW] COM3 já estava aberta");
            }
        } catch (const Exception &E) {
            FDeviceConnected = false;
            FDeviceState = L"communication_error";

            const String message = L"[RIW] Erro ao abrir COM3: " + E.Message;

            ::OutputDebugStringW(message.c_str());
        }
    }
	//---------------------------------------------------------------------------
    void __fastcall TWebModule1::WebModule1DeviceStatusActionAction(
        TObject* Sender, TWebRequest* Request, TWebResponse* Response,
        bool &Handled)
    {
        Response->ContentType = L"application/json; charset=utf-8";

        if (SerialManagerUnit1 != nullptr && SerialManagerUnit1->IsConnected())
        {
            Response->Content = L"{"
                                L"\"connected\":true,"
                                L"\"state\":\"" +
                                SerialManagerUnit1->GetDeviceState() +
                                L"\","
                                L"\"serial\":{"
                                L"\"port\":\"" +
                                SerialManagerUnit1->GetPortName() +
                                L"\","
                                L"\"baud\":115200"
                                L"},"
                                L"\"device\":{"
                                L"\"name\":\"" +
                                SerialManagerUnit1->GetDeviceName() +
                                L"\","
                                L"\"id\":\"" +
                                SerialManagerUnit1->GetDeviceId() +
                                L"\","
                                L"\"firmware\":\"" +
                                SerialManagerUnit1->GetFirmware() +
                                L"\""
                                L"}"
                                L"}";
        } else {
            String state = L"waiting_device";
            String port = L"COM3";

            if (SerialManagerUnit1 != nullptr) {
                state = SerialManagerUnit1->GetDeviceState();

                if (SerialManagerUnit1->IsConnected()) {
                    port = SerialManagerUnit1->GetPortName();
                } else {
                    port = L"";
                }
            }

            Response->Content = L"{"
                                L"\"connected\":false,"
                                L"\"state\":\"" +
                                state +
                                L"\","
                                L"\"serial\":{"
                                L"\"port\":\"" +
                                port +
                                L"\","
                                L"\"baud\":115200"
                                L"},"
                                L"\"message\":\"Aguardando dispositivo\""
                                L"}";
        }

        Handled = true;
    }
    //---------------------------------------------------------------------------
		void __fastcall TWebModule1::WebModule1DeviceIdentityActionAction(
        TObject* Sender, TWebRequest* Request, TWebResponse* Response,
        bool &Handled)
    {
        Response->ContentType = L"application/json; charset=utf-8";

        if (SerialManagerUnit1 == nullptr) {
            Response->StatusCode = 503;
            Response->Content = L"{\"error\":\"SerialManager indisponivel\"}";

            Handled = true;
            return;
        }

        const std::string identityJson = SerialManagerUnit1->GetIdentityJson();

        if (identityJson.empty()) {
            Response->StatusCode = 404;
            Response->Content =
                L"{\"error\":\"Identidade ainda nao recebida\"}";

            Handled = true;
            return;
        }

        Response->Content = UTF8ToString(UTF8String(identityJson.c_str()));

        Handled = true;
    }
    //---------------------------------------------------------------------------
	void __fastcall TWebModule1::WebModule1MegaDisplayMessageActionAction(
        TObject* Sender, TWebRequest* Request, TWebResponse* Response,
        bool &Handled)
    {
        Handled = true;

        Response->ContentType = "application/json; charset=utf-8";

        if (SerialManagerUnit1 == nullptr ||
            !SerialManagerUnit1->IsDeviceConnected(L"TSX-MEGA-001"))
        {
            Response->StatusCode = 503;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Arduino Mega nao conectado\"}";

            return;
        }

        const String body = Request->Content;

        if (body.IsEmpty()) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Corpo da requisicao vazio\"}";

            return;
        }

        const String marker = L"\"text\":\"";

        const int startPos = body.Pos(marker);

        if (startPos <= 0) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Campo text nao encontrado\"}";

            return;
        }

        const int valueStart = startPos + marker.Length();

        const String rest = body.SubString(valueStart, body.Length());

        const int endPos = rest.Pos(L"\"");

        if (endPos <= 0) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Texto invalido\"}";

            return;
        }

        const String text = rest.SubString(1, endPos - 1).Trim();

        if (text.IsEmpty()) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Texto vazio\"}";

            return;
        }

        const bool sent = SerialManagerUnit1->SendMegaDisplayMessage(text);

        if (!sent) {
            Response->StatusCode = 500;

            Response->Content =
                "{\"success\":false,"
                "\"error\":\"Falha ao enviar mensagem ao Mega\"}";

            return;
        }

        Response->StatusCode = 200;

        Response->Content = "{\"success\":true}";
    }
    //---------------------------------------------------------------------------
	 void __fastcall TWebModule1::WebModule1MegaDisplayExpressionActionAction(
        TObject* Sender, TWebRequest* Request, TWebResponse* Response,
        bool &Handled)
    {
        Handled = true;

        Response->ContentType = "application/json; charset=utf-8";

        // Validação MultiSerial:
        // verifica especificamente se o Arduino Mega
        // está presente na lista de dispositivos conectados.
        if (SerialManagerUnit1 == nullptr ||
            !SerialManagerUnit1->IsDeviceConnected(L"TSX-MEGA-001"))
        {
            Response->StatusCode = 503;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Arduino Mega nao conectado\"}";

            return;
        }

        const String body = Request->Content;

        const String marker = L"\"expression\":\"";

        const int startPos = body.Pos(marker);

        if (startPos <= 0) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Expressao nao encontrada\"}";

            return;
        }

        const int valueStart = startPos + marker.Length();

        const String rest = body.SubString(valueStart, body.Length());

        const int endPos = rest.Pos(L"\"");

        if (endPos <= 0) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Expressao invalida\"}";

            return;
        }

        const String expression = rest.SubString(1, endPos - 1);

        const bool sent =
            SerialManagerUnit1->SendMegaDisplayExpression(expression);

        if (!sent) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Falha ao enviar expressao\"}";

            return;
        }

        Response->StatusCode = 200;

        Response->Content = "{\"success\":true}";
    }
    //---------------------------------------------------------------------------
	 void __fastcall TWebModule1::WebModule1MegaDisplaySignatureActionAction(
        TObject* Sender, TWebRequest* Request, TWebResponse* Response,
        bool &Handled)
    {
        Handled = true;

        Response->ContentType = "application/json; charset=utf-8";

        if (SerialManagerUnit1 == nullptr ||
            !SerialManagerUnit1->IsDeviceConnected(L"TSX-MEGA-001"))
        {
            Response->StatusCode = 503;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Arduino Mega nao conectado\"}";

            return;
        }

        const String body = Request->Content;

        if (body.IsEmpty()) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Corpo da requisicao vazio\"}";

            return;
        }

        std::unique_ptr<TJSONValue> jsonValue(
            TJSONObject::ParseJSONValue(body));

        TJSONObject* jsonObject = dynamic_cast<TJSONObject*>(jsonValue.get());

        if (jsonObject == nullptr) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"JSON invalido\"}";

            return;
        }

        TJSONValue* segmentsValue = jsonObject->GetValue(L"segments");

        TJSONArray* segments = dynamic_cast<TJSONArray*>(segmentsValue);

        if (segments == nullptr || segments->Count == 0) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Assinatura vazia\"}";

            return;
        }

        if (!SerialManagerUnit1->SendCommandToDevice(
                L"TSX-MEGA-001", L"TFT_CLEAR"))
        {
            Response->StatusCode = 500;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Falha ao limpar TFT\"}";

            return;
        }

        int sentLines = 0;

        for (int i = 0; i < segments->Count; ++i) {
            TJSONObject* segment =
                dynamic_cast<TJSONObject*>(segments->Items[i]);

            if (segment == nullptr) {
                continue;
            }

            TJSONValue* x1Value = segment->GetValue(L"x1");

            TJSONValue* y1Value = segment->GetValue(L"y1");

            TJSONValue* x2Value = segment->GetValue(L"x2");

            TJSONValue* y2Value = segment->GetValue(L"y2");

            if (x1Value == nullptr || y1Value == nullptr ||
                x2Value == nullptr || y2Value == nullptr)
            {
                continue;
            }

            const int x1 = StrToIntDef(x1Value->Value(), -1);

            const int y1 = StrToIntDef(y1Value->Value(), -1);

            const int x2 = StrToIntDef(x2Value->Value(), -1);

            const int y2 = StrToIntDef(y2Value->Value(), -1);

            if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0) {
                continue;
            }

            const String command = L"TFT_LINE:" + IntToStr(x1) + L"," +
                                   IntToStr(y1) + L"," + IntToStr(x2) + L"," +
                                   IntToStr(y2);

            if (SerialManagerUnit1->SendCommandToDevice(
                    L"TSX-MEGA-001", command))
            {
                ++sentLines;
            }
        }

        if (sentLines == 0) {
            Response->StatusCode = 500;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Nenhum traco enviado\"}";

            return;
        }

        Response->StatusCode = 200;

        Response->Content = "{\"success\":true,"
                            "\"lines\":" +
                            IntToStr(sentLines) + "}";
    }
    //---------------------------------------------------------------------------
	void __fastcall TWebModule1::
        WebModule1WebModule1MegaSignatureStartActionActionAction(
            TObject* Sender, TWebRequest* Request, TWebResponse* Response,
            bool &Handled)
    {
        Handled = true;

        Response->ContentType = "application/json; charset=utf-8";

        if (SerialManagerUnit1 == nullptr ||
            !SerialManagerUnit1->IsDeviceConnected(L"TSX-MEGA-001"))
        {
            Response->StatusCode = 503;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Arduino Mega nao conectado\"}";

            return;
        }

        SerialManagerUnit1->ResetSignatureCapture();

        const bool sent = SerialManagerUnit1->SendCommandToDevice(
            L"TSX-MEGA-001", L"SIGNATURE_BEGIN");

        if (!sent) {
            Response->StatusCode = 500;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Falha ao iniciar assinatura\"}";

            return;
        }

        Response->StatusCode = 200;

        Response->Content = "{\"success\":true,"
                            "\"status\":\"waiting_signature\"}";
    }
    //---------------------------------------------------------------------------
	void __fastcall TWebModule1::
        WebModule1WebModule1MegaSignatureLiveActionActionAction(TObject* Sender,
            TWebRequest* Request, TWebResponse* Response, bool &Handled)
    {
        Handled = true;

        Response->ContentType = "application/json; charset=utf-8";

        if (SerialManagerUnit1 == nullptr) {
            Response->StatusCode = 500;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"SerialManager indisponivel\"}";

            return;
        }

        const std::vector<String> lines =
            SerialManagerUnit1->GetSignatureLines();

        String json = L"{\"success\":true,";

        json += L"\"active\":" + String(SerialManagerUnit1->IsSignatureActive()
                                            ? L"true"
                                            : L"false");

        json += L",\"segments\":[";

        for (size_t i = 0; i < lines.size(); ++i) {
            if (i > 0) {
                json += L",";
            }

            json += lines[i];
        }

        json += L"]}";

        Response->StatusCode = 200;
        Response->Content = json;
    }
    //---------------------------------------------------------------------------
    void __fastcall TWebModule1::
        WebModule1WebModule1GallerySignaturesActionActionAction(TObject* Sender,
            TWebRequest* Request, TWebResponse* Response, bool &Handled)
    {
        Handled = true;

        Response->ContentType = "application/json; charset=utf-8";

        const String requestedDate = Request->QueryFields->Values[L"date"];

        String eventDate = requestedDate.Trim();

        if (eventDate.IsEmpty()) {
            eventDate = L"2026-08-04";
        }

        if (eventDate != L"2026-08-04" && eventDate != L"2026-08-05" &&
            eventDate != L"2026-08-06" && eventDate != L"2026-08-07")
        {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Data do evento invalida\"}";

            return;
        }

        const String folder = ExtractFilePath(ParamStr(0)) +
                              L"data\\gallery\\" + eventDate +
                              L"\\signatures\\";

        if (!DirectoryExists(folder)) {
            Response->StatusCode = 200;

            Response->Content = "{\"success\":true,"
                                "\"count\":0,"
                                "\"date\":\"" +
                                eventDate +
                                "\","
                                "\"signatures\":[]}";

            return;
        }

        TStringList* files = new TStringList();

        try {
            TSearchRec searchRec;

            if (FindFirst(folder + L"*.svg", faAnyFile, searchRec) == 0) {
                do {
                    if ((searchRec.Attr & faDirectory) == 0) {
                        files->Add(searchRec.Name);
                    }
                } while (FindNext(searchRec) == 0);

                FindClose(searchRec);
            }

            files->Sort();

            String json = L"{\"success\":true,";

            json += L"\"date\":\"" + eventDate + L"\",";

            json += L"\"count\":" + IntToStr(files->Count);

            json += L",\"signatures\":[";

            for (int i = 0; i < files->Count; ++i) {
                if (i > 0) {
                    json += L",";
                }

                json += L"\"" + files->Strings[i] + L"\"";
            }

            json += L"]}";

            Response->StatusCode = 200;

            Response->Content = json;
        } __finally
        {
            delete files;
        }
    }

    //---------------------------------------------------------------------------
	void __fastcall TWebModule1::
        WebModule1WebModule1GallerySignatureFileActionActionAction(
            TObject* Sender, TWebRequest* Request, TWebResponse* Response,
            bool &Handled)
    {
        Handled = true;

        const String eventDate = Request->QueryFields->Values[L"date"].Trim();

        const String fileName = Request->QueryFields->Values[L"name"].Trim();

        if (eventDate.IsEmpty() || fileName.IsEmpty()) {
            Response->StatusCode = 400;

            Response->ContentType = "application/json; charset=utf-8";

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Parametros ausentes\"}";

            return;
        }

        if (eventDate != L"2026-08-04" && eventDate != L"2026-08-05" &&
            eventDate != L"2026-08-06" && eventDate != L"2026-08-07")
        {
            Response->StatusCode = 400;

            Response->ContentType = "application/json; charset=utf-8";

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Data do evento invalida\"}";

            return;
        }

        if (fileName.Pos(L"..") > 0 || fileName.Pos(L"\\") > 0 ||
            fileName.Pos(L"/") > 0)
        {
            Response->StatusCode = 400;

            Response->ContentType = "application/json; charset=utf-8";

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Nome de arquivo invalido\"}";

            return;
        }

        const String fullPath = ExtractFilePath(ParamStr(0)) +
                                L"data\\gallery\\" + eventDate +
                                L"\\signatures\\" + fileName;

        if (!FileExists(fullPath)) {
            Response->StatusCode = 404;

            Response->ContentType = "application/json; charset=utf-8";

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Assinatura nao encontrada\"}";

            return;
        }

        TStringList* content = new TStringList();

        try {
            content->LoadFromFile(fullPath, TEncoding::UTF8);

            Response->StatusCode = 200;

            Response->ContentType = "image/svg+xml";

            Response->Content = content->Text;
        } __finally
        {
            delete content;
        }
    }
    //---------------------------------------------------------------------------
	void __fastcall TWebModule1::WebModule1GalleryPhotosActionAction(
        TObject* Sender, TWebRequest* Request, TWebResponse* Response,
        bool &Handled)
    {
        Handled = true;

        Response->ContentType = "application/json; charset=utf-8";

        const String body = Request->Content;

        if (body.IsEmpty()) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Corpo da requisicao vazio\"}";

            return;
        }

        std::unique_ptr<TJSONValue> jsonValue(
            TJSONObject::ParseJSONValue(body));

        TJSONObject* jsonObject = dynamic_cast<TJSONObject*>(jsonValue.get());

        if (jsonObject == nullptr) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"JSON invalido\"}";

            return;
        }

        String eventDate = L"";

        String imageBase64 = L"";

        TJSONValue* dateValue = jsonObject->GetValue(L"date");

        if (dateValue != nullptr) {
            eventDate = dateValue->Value();
        }

        TJSONValue* imageValue = jsonObject->GetValue(L"image");

        if (imageValue != nullptr) {
            imageBase64 = imageValue->Value();
        }

        if (eventDate.IsEmpty()) {
            eventDate = L"2026-08-04";
        }

        if (eventDate != L"2026-08-04" && eventDate != L"2026-08-05" &&
            eventDate != L"2026-08-06" && eventDate != L"2026-08-07")
        {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Data do evento invalida\"}";

            return;
        }

        if (imageBase64.IsEmpty()) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Imagem nao recebida\"}";

            return;
        }

        const String marker = L"base64,";

        const int markerPos = imageBase64.Pos(marker);

        if (markerPos > 0) {
            imageBase64 = imageBase64.SubString(
                markerPos + marker.Length(), imageBase64.Length());
        }

        TBytes imageBytes;

        try {
            imageBytes = TNetEncoding::Base64->DecodeStringToBytes(imageBase64);
        } catch (...) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Imagem Base64 invalida\"}";

            return;
        }

        const String folder = ExtractFilePath(ParamStr(0)) +
                              L"data\\gallery\\" + eventDate + L"\\photos\\";

        if (!ForceDirectories(folder)) {
            Response->StatusCode = 500;

            Response->Content =
                "{\"success\":false,"
                "\"error\":\"Falha ao criar diretorio de fotos\"}";

            return;
        }

        const String fileName =
            L"photo_" + FormatDateTime(L"yyyymmdd_hhnnss_zzz", Now()) + L".jpg";

        const String fullPath = folder + fileName;

        try {
            TFile::WriteAllBytes(fullPath, imageBytes);
            //Teste
            ::OutputDebugStringW((L"[RIW] Foto salva: " + fullPath).c_str());

            if (SerialManagerUnit1 != nullptr &&
                SerialManagerUnit1->IsDeviceConnected(L"TSX-MEGA-001"))
            {
                const bool photoSent = SendPhotoToMegaTFT(fullPath);

                if (!photoSent) {
                    // Fallback:
                    // se a foto real falhar,
                    // mantém a mensagem que já funciona.
                    SerialManagerUnit1->SendCommandToDevice(
                        L"TSX-MEGA-001", L"PHOTO_SUCCESS");

                    ::OutputDebugStringW(
                        L"[RIW] Falha no preview da foto. PHOTO_SUCCESS enviado como fallback.");
                } else {
                    ::OutputDebugStringW(
                        L"[RIW] Preview da foto enviado ao TFT.");
                }
            }

            //fim teste
        } catch (...) {
            Response->StatusCode = 500;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Falha ao salvar foto\"}";

            return;
        }

        ::OutputDebugStringW((L"[RIW] Foto salva: " + fullPath).c_str());

        Response->StatusCode = 200;

        Response->Content = L"{\"success\":true,"
                            L"\"date\":\"" +
                            eventDate +
                            L"\","
                            L"\"file\":\"" +
                            fileName + L"\"}";
    }
    //---------------------------------------------------------------------------
	void __fastcall TWebModule1::WebModule1GalleryPhotosListActionAction(
        TObject* Sender, TWebRequest* Request, TWebResponse* Response,
        bool &Handled)
    {
        Handled = true;

        Response->ContentType = "application/json; charset=utf-8";

        const String requestedDate = Request->QueryFields->Values[L"date"];

        String eventDate = requestedDate.Trim();

        if (eventDate.IsEmpty()) {
            eventDate = L"2026-08-04";
        }

        if (eventDate != L"2026-08-04" && eventDate != L"2026-08-05" &&
            eventDate != L"2026-08-06" && eventDate != L"2026-08-07")
        {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Data do evento invalida\"}";

            return;
        }

        const String folder = ExtractFilePath(ParamStr(0)) +
                              L"data\\gallery\\" + eventDate + L"\\photos\\";

        if (!DirectoryExists(folder)) {
            Response->StatusCode = 200;

            Response->Content = L"{\"success\":true,"
                                L"\"date\":\"" +
                                eventDate +
                                L"\","
                                L"\"count\":0,"
                                L"\"photos\":[]}";

            return;
        }

        TStringList* files = new TStringList();

        try {
            TSearchRec searchRec;

            if (FindFirst(folder + L"*.jpg", faAnyFile, searchRec) == 0) {
                do {
                    if ((searchRec.Attr & faDirectory) == 0) {
                        files->Add(searchRec.Name);
                    }
                } while (FindNext(searchRec) == 0);

                FindClose(searchRec);
            }

            files->Sort();

            String json = L"{\"success\":true,";

            json += L"\"date\":\"" + eventDate + L"\",";

            json += L"\"count\":" + IntToStr(files->Count);

            json += L",\"photos\":[";

            for (int i = 0; i < files->Count; ++i) {
                if (i > 0) {
                    json += L",";
                }

                json += L"\"" + files->Strings[i] + L"\"";
            }

            json += L"]}";

            Response->StatusCode = 200;

            Response->Content = json;
        } __finally
        {
            delete files;
        }
    }
    //---------------------------------------------------------------------------
	void __fastcall TWebModule1::WebModule1GalleryPhotoFileActionAction(
        TObject* Sender, TWebRequest* Request, TWebResponse* Response,
        bool &Handled)
    {
        Handled = true;

        const String eventDate = Request->QueryFields->Values[L"date"].Trim();

        const String fileName = Request->QueryFields->Values[L"name"].Trim();

        if (eventDate.IsEmpty() || fileName.IsEmpty()) {
            Response->StatusCode = 400;

            Response->ContentType = "application/json; charset=utf-8";

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Parametros ausentes\"}";

            return;
        }

        if (eventDate != L"2026-08-04" && eventDate != L"2026-08-05" &&
            eventDate != L"2026-08-06" && eventDate != L"2026-08-07")
        {
            Response->StatusCode = 400;

            Response->ContentType = "application/json; charset=utf-8";

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Data do evento invalida\"}";

            return;
        }

        if (fileName.Pos(L"..") > 0 || fileName.Pos(L"\\") > 0 ||
            fileName.Pos(L"/") > 0)
        {
            Response->StatusCode = 400;

            Response->ContentType = "application/json; charset=utf-8";

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Nome de arquivo invalido\"}";

            return;
        }

        const String fullPath = ExtractFilePath(ParamStr(0)) +
                                L"data\\gallery\\" + eventDate + L"\\photos\\" +
                                fileName;

        if (!FileExists(fullPath)) {
            Response->StatusCode = 404;

            Response->ContentType = "application/json; charset=utf-8";

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Foto nao encontrada\"}";

            return;
        }

        try {
            const TBytes imageBytes = TFile::ReadAllBytes(fullPath);

            Response->StatusCode = 200;

            Response->ContentType = "image/jpeg";

            Response->ContentStream = new TBytesStream(imageBytes);

            Response->FreeContentStream = true;
        } catch (...) {
            Response->StatusCode = 500;

            Response->ContentType = "application/json; charset=utf-8";

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Falha ao carregar foto\"}";
        }
    }
    //---------------------------------------------------------------------------
    void __fastcall TWebModule1::WebModule1DeviceCommandActionAction(
        TObject* Sender, TWebRequest* Request, TWebResponse* Response,
        bool &Handled)
    {
        Handled = true;

        Response->ContentType = "application/json; charset=utf-8";

        const String body = Request->Content;

        if (body.IsEmpty()) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Corpo da requisicao vazio\"}";

            return;
        }

        std::unique_ptr<TJSONValue> jsonValue(
            TJSONObject::ParseJSONValue(body));

        TJSONObject* jsonObject = dynamic_cast<TJSONObject*>(jsonValue.get());

        if (jsonObject == nullptr) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"JSON invalido\"}";

            return;
        }

        String deviceId = L"";

        String command = L"";

        TJSONValue* deviceValue = jsonObject->GetValue(L"device_id");

        if (deviceValue != nullptr) {
            deviceId = deviceValue->Value();
        }

        TJSONValue* commandValue = jsonObject->GetValue(L"command");

        if (commandValue != nullptr) {
            command = commandValue->Value();
        }

        if (deviceId.IsEmpty() || command.IsEmpty()) {
            Response->StatusCode = 400;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"device_id ou command ausente\"}";

            return;
        }

        if (SerialManagerUnit1 == nullptr ||
            !SerialManagerUnit1->IsDeviceConnected(deviceId))
        {
            Response->StatusCode = 404;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Dispositivo nao conectado\"}";

            return;
        }

        const bool sent =
            SerialManagerUnit1->SendCommandToDevice(deviceId, command);

        if (!sent) {
            Response->StatusCode = 500;

            Response->Content = "{\"success\":false,"
                                "\"error\":\"Falha ao enviar comando\"}";

            return;
        }

        ::OutputDebugStringW((L"[RIW] Comando enviado | Device: " + deviceId +
                              L" | Command: " + command)
                .c_str());

        Response->StatusCode = 200;

        Response->Content = L"{\"success\":true,"
                            L"\"device_id\":\"" +
                            deviceId +
                            L"\","
                            L"\"command\":\"" +
                            command + L"\"}";
    }
    //---------------------------------------------------------------------------

