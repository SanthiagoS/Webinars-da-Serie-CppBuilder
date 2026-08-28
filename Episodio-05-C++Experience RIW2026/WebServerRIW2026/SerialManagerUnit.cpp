//---------------------------------------------------------------------------

#include <System.SysUtils.hpp>
#include <cstdio>
#include <System.DateUtils.hpp>
#pragma hdrstop

#include "SerialManagerUnit.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma classgroup "FMX.Controls.TControl"
#pragma resource "*.dfm"

//---------------------------------------------------------------------------

TSerialManagerUnit1 *SerialManagerUnit1;

//---------------------------------------------------------------------------
__fastcall TSerialManagerUnit1::TSerialManagerUnit1(
	TComponent* Owner)
	: TDataModule(Owner),
	  FConnected(false),
	  FLastSeenTick(0)
{
}
//---------------------------------------------------------------------------
void __fastcall TSerialManagerUnit1::DataModuleCreate(
	TObject *Sender)
{
        ::OutputDebugStringW(
        L"[RIW] DataModuleCreate executado"
    );

    // --------------------------------------------------------
    // Estado legado
    // Será removido posteriormente quando o browser
    // estiver 100% baseado em múltiplos dispositivos.
    // --------------------------------------------------------

    FConnected = false;

    FDeviceName = L"";
    FDeviceId = L"";
    FFirmware = L"";
    FPortName = L"";

    FDeviceState =
        L"scanning_devices";

    // --------------------------------------------------------
    // NOVA arquitetura:
    // detecta TODAS as portas disponíveis.
    // --------------------------------------------------------

    const auto ports =
        EnumerateSerialPorts();

    if (ports.empty())
    {
        FDeviceState =
            L"waiting_device";

        ::OutputDebugStringW(
            L"[RIW] Nenhuma porta serial encontrada"
        );

        return;
    }

    ::OutputDebugStringW(
        L"[RIW] Iniciando scan MultiSerial"
    );

    ScanAllDevices();

    ::OutputDebugStringW(
        L"[RIW] Scan MultiSerial concluido"
    );

    // Não colocamos waiting_device aqui.
    //
    // A identificação acontece através dos callbacks
    // das instâncias TNativeSerialManager.
}
//---------------------------------------------------------------------------
void __fastcall TSerialManagerUnit1::DataModuleDestroy(
    TObject *Sender)
{
    if (FSerial)
    {
        FSerial->Close();
        FSerial.reset();
    }

    FConnected = false;
    FDeviceState = L"disconnected";
}
//---------------------------------------------------------------------------
void TSerialManagerUnit1::ProcessSerialLine(
	const std::string& Line)
{
	if (Line.empty())
	{
		return;
	}

	FLastSeenTick.store(
		::GetTickCount64()
	);

	const String Json =
		UTF8ToString(
			UTF8String(
				Line.c_str()
			)
		);

	if (
		Json.Pos(
			L"\"type\":\"identity\""
		) > 0)
	{
		FIdentityJson = Line;
	}

	if (
        Line.find(
            "\"type\":\"identity\""
		) == std::string::npos)
    {
        return;
	}

	FConnected = true;
	FDeviceState = L"connected";

	// --------------------------------------------------------
	// Arduino Uno
	// --------------------------------------------------------

	if (
		Line.find(
			"\"id\":\"TSX-UNO-001\""
		) != std::string::npos)
	{
		FDeviceName =
            L"Arduino Uno";

        FDeviceId =
			L"TSX-UNO-001";

		FFirmware =
            L"2.1.0";

        ::OutputDebugStringW(
			L"[RIW] Arduino Uno identificado"
        );

		return;
	}

	// --------------------------------------------------------
	// Arduino Mega
	// --------------------------------------------------------

	if (
		Line.find(
			"\"id\":\"TSX-MEGA-001\""
		) != std::string::npos)
	{
		FDeviceName =
			L"Arduino Mega 2560";

        FDeviceId =
			L"TSX-MEGA-001";

		FFirmware =
            L"1.0.0";

		::OutputDebugStringW(
            L"[RIW] Arduino Mega identificado"
        );

        return;
	}

	// --------------------------------------------------------
	// ESP32-S3
	// --------------------------------------------------------

	if (
		Line.find(
			"\"id\":\"TSX-ESP32-001\""
		) != std::string::npos)
	{
		FDeviceName =
			L"ESP32-S3";

        FDeviceId =
			L"TSX-ESP32-001";

        FFirmware =
			L"1.0.0";

        ::OutputDebugStringW(
			L"[RIW] ESP32-S3 identificado"
        );

		return;
	}

	// --------------------------------------------------------
	// Dispositivo TSX não reconhecido
	// --------------------------------------------------------

	FDeviceName =
		L"Dispositivo TSX";

	FDeviceId =
		L"UNKNOWN";

	FFirmware =
		L"-";

	::OutputDebugStringW(
        L"[RIW] Identidade TSX recebida, mas dispositivo desconhecido"
	);
}
//---------------------------------------------------------------------------
bool TSerialManagerUnit1::IsConnected() const
{
	 if (!FConnected.load())
	{
		return false;
	}

    const ULONGLONG lastSeen =
        FLastSeenTick.load();

    if (lastSeen == 0)
    {
        return false;
    }

    const ULONGLONG elapsed =
        ::GetTickCount64() - lastSeen;

    // O Uno envia status aproximadamente a cada 3 segundos.
    return elapsed <= 7000;
}
//---------------------------------------------------------------------------
String TSerialManagerUnit1::GetDeviceName() const
{
	return FDeviceName;
}
//---------------------------------------------------------------------------
String TSerialManagerUnit1::GetDeviceId() const
{
    return FDeviceId;
}
//---------------------------------------------------------------------------
String TSerialManagerUnit1::GetFirmware() const
{
    return FFirmware;
}
//---------------------------------------------------------------------------
String TSerialManagerUnit1::GetPortName() const
{
    return FPortName;
}
//---------------------------------------------------------------------------
String TSerialManagerUnit1::GetDeviceState() const
{
    if (IsConnected())
    {
        return L"connected";
    }

    if (FLastSeenTick.load() > 0)
    {
        return L"disconnected";
    }

    return FDeviceState;
}
//---------------------------------------------------------------------------
std::string TSerialManagerUnit1::GetIdentityJson() const
{
    return FIdentityJson;
}
//---------------------------------------------------------------------------
void TSerialManagerUnit1::DeviceRemoved()
{
    ::OutputDebugStringW(
        L"[RIW] SerialManager recebeu remoção de dispositivo"
	);
	// Tempo para o Windows atualizar as COMs
	::Sleep(300);

	// Revarre TODAS as portas e remove as que sumiram
	ScanAllDevices();

    FConnected.store(false);
    FLastSeenTick.store(0);

    FDeviceState = L"disconnected";
    FIdentityJson.clear();

    FDeviceName = L"";
    FDeviceId = L"";
    FFirmware = L"";

    if (FSerial)
    {
        FSerial->Close();
        FSerial.reset();
    }

}
//---------------------------------------------------------------------------
void TSerialManagerUnit1::DeviceArrived()
{
    ::OutputDebugStringW(
		L"[RIW] SerialManager recebeu chegada de dispositivo"
	);
    ::Sleep(500);

	ScanAllDevices();

    return;

    if (FSerial)
    {
        FSerial->Close();
        FSerial.reset();
    }

    FConnected = false;

    FDeviceName = L"";
    FDeviceId = L"";
    FFirmware = L"";
    FPortName = L"";

    FDeviceState = L"opening_port";

    const auto ports =
        EnumerateSerialPorts();

    if (ports.empty())
    {
        FDeviceState = L"waiting_device";

        ::OutputDebugStringW(
            L"[RIW] Nenhuma porta encontrada apos conexao USB"
        );

        return;
    }

    bool deviceFound = false;

    for (const auto& port : ports)
    {
        ::OutputDebugStringW(
            (L"[RIW] Testando nova porta: " +
             port).c_str()
        );

        if (ProbePort(port))
        {
            FPortName = port;
            deviceFound = true;

            ::OutputDebugStringW(
                (L"[RIW] Dispositivo TSX encontrado em " +
                 FPortName).c_str()
            );

            break;
        }
    }

    if (!deviceFound)
    {
        FDeviceState = L"waiting_device";

        ::OutputDebugStringW(
            L"[RIW] Nenhum dispositivo TSX encontrado apos conexao"
        );

        return;
    }

    // Agora abre a porta encontrada de forma definitiva.
    FSerial =
        std::make_unique<TNativeSerialManager>();

    FSerial->SetOnLineReceived(
        [this](const std::string& Line)
        {
            ProcessSerialLine(Line);
        }
    );

    if (!FSerial->Open(
            FPortName,
            CBR_115200))
    {
        FDeviceState = L"communication_error";

        ::OutputDebugStringW(
            (L"[RIW] Falha ao abrir definitivamente " +
             FPortName).c_str()
        );

        return;
    }

    FDeviceState = L"waiting_device";

    // Aguarda a placa estabilizar.
    ::Sleep(1800);

    if (FSerial->WriteLine(L"IDENTIFY"))
    {
        ::OutputDebugStringW(
            (L"[RIW] IDENTIFY enviado para " +
             FPortName).c_str()
        );
    }
    else
    {
        FDeviceState = L"communication_error";

        ::OutputDebugStringW(
            L"[RIW] Falha ao enviar IDENTIFY apos reconexao"
        );
    }
}
//---------------------------------------------------------------------------
bool TSerialManagerUnit1::SetCapabilityState(
    const String& Target,
    bool Value)
{
    if (
        !FSerial ||
        !IsConnected())
    {
        return false;
    }

    const String command =
        L"{\"command\":\"set\","
        L"\"target\":\"" + Target + L"\","
        L"\"value\":" +
        String(Value ? L"true" : L"false") +
        L"}";

    const bool sent =
        FSerial->WriteLine(command);

    if (sent)
    {
        const String message =
            L"[RIW] Comando enviado: " +
            command;

        ::OutputDebugStringW(
            message.c_str()
        );
    }

    return sent;
}
//---------------------------------------------------------------------------
std::vector<String> TSerialManagerUnit1::EnumerateSerialPorts()
{
    std::vector<String> ports;

    for (int i = 1; i <= 32; ++i)
    {
        const String portName =
            L"COM" + IntToStr(i);

        const String deviceName =
            L"\\\\.\\" + portName;

        HANDLE handle =
            ::CreateFileW(
                deviceName.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,
                0,
                nullptr
            );

        if (handle != INVALID_HANDLE_VALUE)
        {
            ports.push_back(portName);

            ::CloseHandle(handle);
        }
        else
        {
            const DWORD errorCode =
                ::GetLastError();

            if (
                errorCode == ERROR_ACCESS_DENIED ||
                errorCode == ERROR_SHARING_VIOLATION)
            {
                // Porta existe, mas já está em uso.
                ports.push_back(portName);
            }
        }
    }

    return ports;
}
//---------------------------------------------------------------------------
bool TSerialManagerUnit1::ProbePort(
    const String& PortName)
{
    ::OutputDebugStringW(
        (L"[RIW] Testando porta: " + PortName).c_str()
    );

    TNativeSerialManager probe;

    bool identified = false;

    probe.SetOnLineReceived(
        [&identified](const std::string& Line)
        {
            if (
                Line.find("\"type\":\"identity\"") !=
                std::string::npos
            )
            {
                identified = true;
            }
        }
    );

    if (!probe.Open(
            PortName,
            CBR_115200))
    {
        ::OutputDebugStringW(
            (L"[RIW] Falha ao abrir porta para teste: " +
             PortName).c_str()
        );

        return false;
    }

    // Aguarda a inicialização da placa.
    ::Sleep(1800);

    if (!probe.WriteLine(L"IDENTIFY"))
    {
        probe.Close();

        return false;
    }

    // Dá tempo para a placa responder.
    for (int i = 0; i < 20; ++i)
    {
        if (identified)
        {
            break;
        }

        ::Sleep(100);
    }

    probe.Close();

    if (identified)
    {
        ::OutputDebugStringW(
            (L"[RIW] Dispositivo TSX identificado em " +
             PortName).c_str()
        );
    }
    else
    {
        ::OutputDebugStringW(
            (L"[RIW] Nenhum dispositivo TSX identificado em " +
             PortName).c_str()
        );
    }

    return identified;
}
//---------------------------------------------------------------------------
bool TSerialManagerUnit1::SendMegaDisplayMessage(
    const String& Text)
{
    // --------------------------------------------------------
    // Localiza o Mega na arquitetura MultiSerial
    // --------------------------------------------------------

    TConnectedDevice* mega = nullptr;

    for (const auto& device : FDevices)
    {
        if (
            device &&
            device->Connected &&
            SameText(
                device->DeviceId,
                L"TSX-MEGA-001"
            )
        )
        {
            mega = device.get();
            break;
        }
    }

    if (mega == nullptr)
    {
        ::OutputDebugStringW(
            L"[RIW] Mega: dispositivo nao encontrado em FDevices"
        );

        return false;
    }

    if (!mega->Serial)
    {
        ::OutputDebugStringW(
            L"[RIW] Mega: serial MultiSerial indisponivel"
        );

        return false;
    }

    // --------------------------------------------------------
    // Sanitiza o texto
    // --------------------------------------------------------

    String safeText =
        Text.Trim();

    safeText =
        StringReplace(
            safeText,
            L"á",
            L"a",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"à",
            L"a",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"ã",
            L"a",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"â",
            L"a",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"é",
            L"e",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"ê",
            L"e",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"í",
            L"i",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"ó",
            L"o",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"ô",
            L"o",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"õ",
            L"o",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"ú",
            L"u",
            TReplaceFlags() << rfReplaceAll
        );

    safeText =
        StringReplace(
            safeText,
            L"ç",
            L"c",
            TReplaceFlags() << rfReplaceAll
        );

    if (safeText.IsEmpty())
    {
        ::OutputDebugStringW(
            L"[RIW] Mega: mensagem vazia"
        );

        return false;
    }

    // Evita quebrar o JSON simples do firmware
    safeText =
        StringReplace(
            safeText,
            L"\"",
            L"'",
            TReplaceFlags() << rfReplaceAll
        );

    // --------------------------------------------------------
    // Monta comando
    // --------------------------------------------------------

    const String command =
        L"{\"command\":\"show_message\","
        L"\"text\":\"" +
        safeText +
        L"\"}";

    // --------------------------------------------------------
    // ENVIA PELA SERIAL DO MEGA
    // --------------------------------------------------------

    const bool sent =
        mega->Serial->WriteLine(
            command
        );

    if (sent)
    {
        const String log =
            L"[RIW] Mega TFT enviado via MultiSerial "
            L"(" +
            mega->PortName +
            L"): " +
            command;

        ::OutputDebugStringW(
            log.c_str()
        );
    }
    else
    {
        ::OutputDebugStringW(
            L"[RIW] Mega TFT: falha WriteLine MultiSerial"
        );
    }

    return sent;
}
//---------------------------------------------------------------------------
bool TSerialManagerUnit1::SendMegaDisplayExpression(
    const String& Expression)
{
    // --------------------------------------------------------
    // Localiza o Mega na arquitetura MultiSerial
    // --------------------------------------------------------

    TConnectedDevice* mega = nullptr;

    for (const auto& device : FDevices)
    {
        if (
            device &&
            device->Connected &&
            SameText(
                device->DeviceId,
                L"TSX-MEGA-001"
            )
        )
        {
            mega = device.get();
            break;
        }
    }

    if (mega == nullptr)
    {
        ::OutputDebugStringW(
            L"[RIW] Mega: dispositivo nao encontrado em FDevices"
        );

        return false;
    }

    if (!mega->Serial)
    {
        ::OutputDebugStringW(
            L"[RIW] Mega: serial MultiSerial indisponivel"
        );

        return false;
    }

    const String expression =
        Expression
            .Trim()
            .LowerCase();

    if (
        expression != L"happy" &&
        expression != L"cool" &&
        expression != L"love" &&
        expression != L"surprise"
    )
    {
        ::OutputDebugStringW(
            L"[RIW] Mega: expressao invalida"
        );

        return false;
    }

    const String command =
        L"{\"command\":\"show_expression\","
        L"\"expression\":\"" +
        expression +
        L"\"}";

    // --------------------------------------------------------
    // ENVIA PELA SERIAL ESPECÍFICA DO MEGA
    // --------------------------------------------------------

    const bool sent =
        mega->Serial->WriteLine(
            command
        );

    if (sent)
    {
        const String log =
            L"[RIW] Expressao enviada ao Mega "
            L"(" +
            mega->PortName +
            L"): " +
            expression;

        ::OutputDebugStringW(
            log.c_str()
        );
    }
    else
    {
        ::OutputDebugStringW(
            L"[RIW] Mega: falha ao enviar expressao via MultiSerial"
        );
    }

    return sent;
}
//---------------------------------------------------------------------------
TConnectedDevice*
TSerialManagerUnit1::FindDeviceByPort(
    const String& PortName)
{
    for (auto& device : FDevices)
    {
        if (
            device &&
            device->PortName ==
            PortName)
        {
            return device.get();
        }
    }

    return nullptr;
}
//---------------------------------------------------------------------------
TConnectedDevice*
TSerialManagerUnit1::FindDeviceById(
    const String& DeviceId)
{
    for (auto& device : FDevices)
    {
        if (
            device &&
            device->DeviceId ==
            DeviceId)
        {
            return device.get();
        }
    }

    return nullptr;
}
//---------------------------------------------------------------------------
int TSerialManagerUnit1::
GetConnectedDeviceCount() const
{
    int count = 0;

    for (const auto& device : FDevices)
    {
        if (
            device &&
            device->Connected)
        {
            ++count;
        }
    }

    return count;
}
//---------------------------------------------------------------------------
bool TSerialManagerUnit1::AddDeviceFromPort(
	const String& PortName)
	{
		// Já existe?
		if (FindDeviceByPort(PortName))
		{
			return true;
		}

		auto device =
			std::make_unique<TConnectedDevice>();

		device->PortName =
			PortName;

		device->State =
			L"opening_port";

		device->Serial =
			std::make_unique<TNativeSerialManager>();

		TConnectedDevice* devicePtr =
			device.get();

		device->Serial->SetOnLineReceived(
			[this, devicePtr](
				const std::string& Line)
			{
				if (!devicePtr)
				{
					return;
				}

				const String json =
					UTF8ToString(
						UTF8String(
							Line.c_str()
						)
					);

			 // Registra tudo que chegou desta porta
			 // para o Monitor Serial.
			 AddSerialMonitorEntry(
			 L"RX",
			 devicePtr->PortName,
			 devicePtr->DeviceId,
			 json
			);

				// ------------------------------------------------
				// Função auxiliar para extrair valores simples
				// do JSON recebido.
				//
				// Exemplo:
				// "firmware":"1.1.0"
				// ------------------------------------------------

				auto extractJsonValue =
					[](
						const String& source,
						const String& key
					) -> String
				{
					const String marker =
						L"\"" +
						key +
						L"\":\"";

					int startPos =
						source.Pos(marker);

					if (startPos <= 0)
					{
						return L"";
					}

					startPos +=
						marker.Length();

					const String rest =
						source.SubString(
							startPos,
							source.Length()
						);

					const int endPos =
						rest.Pos(L"\"");

					if (endPos <= 0)
					{
						return L"";
					}

					return rest.SubString(
						1,
						endPos - 1
					);
				};


	// ------------------------------------------------
	// SIGNATURE START
	// ------------------------------------------------

	if (
		json.Pos(
			L"\"type\":\"signature_start\""
		) > 0)
	{
		if (!FSignatureActive)
		{
			FSignatureActive =
				true;

			FSignatureLines.clear();

			::OutputDebugStringW(
				L"[RIW] Assinatura iniciada no TFT"
			);
		}

		return;
	}

	// ------------------------------------------------
	// SIGNATURE LINE
	// ------------------------------------------------

	if (
		json.Pos(
			L"\"type\":\"signature_line\""
		) > 0)
	{
		if (
			FSignatureActive &&
			devicePtr->DeviceId ==
				L"TSX-MEGA-001"
		)
		{
			FSignatureLines.push_back(
				json
			);
		}

		return;
	}

	// ------------------------------------------------
	// SIGNATURE END
	// ------------------------------------------------

	if (
		json.Pos(
			L"\"type\":\"signature_end\""
		) > 0)
	{
		if (
			FSignatureActive &&
			devicePtr->DeviceId ==
				L"TSX-MEGA-001"
		)
		{
			FSignatureActive =
				false;

			const bool saved =
				SaveCurrentSignatureToSvg();

			const String log =
				L"[RIW] Assinatura concluida | Segmentos: " +
				IntToStr(
					static_cast<int>(
						FSignatureLines.size()
					)
				) +
				L" | Arquivo: " +
				(
					saved
						? FLastSignatureFile
						: L"ERRO"
				);

			::OutputDebugStringW(
				log.c_str()
			);
		}

		return;
	}

	// ------------------------------------------------
	// SIGNATURE LINE
	// ------------------------------------------------

	if (
		json.Pos(
			L"\"type\":\"signature_line\""
		) > 0)
	{
		if (
			FSignatureActive &&
			devicePtr->DeviceId ==
				L"TSX-MEGA-001")
		{
			FSignatureLines.push_back(
				json
			);
		}

		return;
	}

	// ------------------------------------------------
	// SIGNATURE END
	// ------------------------------------------------

	if (
		json.Pos(
			L"\"type\":\"signature_end\""
		) > 0)
	{
		if (
			FSignatureActive &&
			devicePtr->DeviceId ==
				L"TSX-MEGA-001")
		{
			FSignatureActive =
				false;

			const String log =
				L"[RIW] Assinatura concluida | Segmentos: " +
				IntToStr(
					static_cast<int>(
						FSignatureLines.size()
					)
				);

			::OutputDebugStringW(
				log.c_str()
			);
		}

		return;
	}


				// ------------------------------------------------
				// IDENTITY
				// ------------------------------------------------

				if (
					json.Pos(
						L"\"type\":\"identity\""
					) > 0)
				{
					devicePtr->Connected =
						true;

					devicePtr->State =
						L"connected";

					// --------------------------------------------
					// Identificação da placa
					// --------------------------------------------

					if (
						json.Pos(
							L"TSX-UNO-001"
						) > 0)
					{
						devicePtr->DeviceId =
							L"TSX-UNO-001";

						devicePtr->DeviceName =
							L"Arduino Uno";
					}
					else if (
						json.Pos(
							L"TSX-MEGA-001"
						) > 0)
					{
						devicePtr->DeviceId =
							L"TSX-MEGA-001";

						devicePtr->DeviceName =
							L"Arduino Mega 2560";
					}
					else if (
						json.Pos(
							L"TSX-ESP32-001"
						) > 0)
					{
						devicePtr->DeviceId =
							L"TSX-ESP32-001";

						devicePtr->DeviceName =
							L"ESP32-S3";
					}

					// --------------------------------------------
					// Informações técnicas vindas do firmware
					// --------------------------------------------

					devicePtr->Firmware =
						extractJsonValue(
							json,
							L"firmware"
						);

					devicePtr->Protocol =
						extractJsonValue(
							json,
							L"protocol"
						);

					devicePtr->Mcu =
						extractJsonValue(
							json,
							L"mcu"
						);

					devicePtr->Clock =
						extractJsonValue(
							json,
							L"clock"
						);

					devicePtr->Flash =
						extractJsonValue(
							json,
							L"flash"
						);

					devicePtr->Sram =
						extractJsonValue(
							json,
							L"sram"
						);

					devicePtr->Eeprom =
						extractJsonValue(
							json,
							L"eeprom"
						);

					devicePtr->Psram =
						extractJsonValue(
							json,
							L"psram"
						);

					// --------------------------------------------
					// Log
					// --------------------------------------------

					const String log =
						L"[RIW] MultiSerial identificado: " +
						devicePtr->DeviceName +
						L" em " +
						devicePtr->PortName;

					::OutputDebugStringW(
						log.c_str()
					);

					const String technicalLog =
						L"[RIW] Hardware: MCU=" +
						devicePtr->Mcu +
						L" | Clock=" +
						devicePtr->Clock +
						L" | Firmware=" +
						devicePtr->Firmware +
						L" | Flash=" +
						devicePtr->Flash;

					::OutputDebugStringW(
						technicalLog.c_str()
					);
				}
			}
		);

		if (
			!device->Serial->Open(
				PortName,
				CBR_115200
			))
		{
			const DWORD errorCode =
				::GetLastError();

			const String log =
				L"[RIW] Falha ao abrir porta multi: " +
				PortName +
				L" | Erro Windows: " +
				IntToStr(
					static_cast<int>(
						errorCode
					)
				);

			::OutputDebugStringW(
				log.c_str()
			);

			return false;
		}

		::OutputDebugStringW(
			(
				L"[RIW] Porta multi aberta: " +
				PortName
			).c_str()
		);

		// Algumas placas reiniciam ao abrir a serial.
		::Sleep(1800);

		AddSerialMonitorEntry(
		L"TX",
		device->PortName,
		device->DeviceId,
		L"IDENTIFY"
		);

		::OutputDebugStringW(
		(
			L"[RIW] TX [" +
			device->PortName +
			L"] IDENTIFY"
		).c_str()
	   );


		if (
			!device->Serial->WriteLine(
				L"IDENTIFY"
			))
		{
			::OutputDebugStringW(
				(
					L"[RIW] Falha IDENTIFY em " +
					PortName
				).c_str()
			);

			return false;
		}

		FDevices.push_back(
			std::move(device)
		);

		return true;
}
//---------------------------------------------------------------------------
void TSerialManagerUnit1::RemoveDeviceByPort(
	const String& PortName)
{
	for (
        auto it = FDevices.begin();
        it != FDevices.end();
        ++it)
    {
        if (
            *it &&
            (*it)->PortName ==
            PortName)
        {
            ::OutputDebugStringW(
                (
                    L"[RIW] Removendo dispositivo: " +
                    PortName
                ).c_str()
            );

            if ((*it)->Serial)
            {
                (*it)->Serial->Close();
            }

            FDevices.erase(it);

            return;
        }
    }
}
//---------------------------------------------------------------------------
void TSerialManagerUnit1::ScanAllDevices()
{
    const auto ports =
        EnumerateSerialPorts();

    // --------------------------------------------------------
    // 1. REMOVER dispositivos cujas portas não existem mais
    // --------------------------------------------------------

    for (auto it = FDevices.begin();
         it != FDevices.end();)
    {
        if (!(*it))
        {
            it = FDevices.erase(it);
            continue;
        }

        bool portStillExists = false;

        for (const auto& port : ports)
        {
            if (SameText(
                    port,
                    (*it)->PortName))
            {
                portStillExists = true;
                break;
            }
        }

        if (!portStillExists)
        {
            const String message =
                L"[RIW] Porta removida: " +
                (*it)->PortName +
                L" | " +
                (*it)->DeviceName;

            ::OutputDebugStringW(
                message.c_str()
            );

            if ((*it)->Serial)
            {
                (*it)->Serial->Close();
            }

            it = FDevices.erase(it);

            continue;
        }

        ++it;
    }

    // --------------------------------------------------------
    // 2. ADICIONAR portas novas
    // --------------------------------------------------------

    for (const auto& port : ports)
    {
        if (
            FindDeviceByPort(port) ==
            nullptr)
        {
            AddDeviceFromPort(
                port
            );
        }
    }
}
//---------------------------------------------------------------------------
String TSerialManagerUnit1::GetDevicesJson() const
{
    String json;

    json =
        L"{\"count\":" +
        IntToStr(
            GetConnectedDeviceCount()
        ) +
        L",\"devices\":[";

    bool first = true;

    for (const auto& device : FDevices)
    {
        if (
            !device ||
            !device->Connected)
        {
            continue;
        }

        if (!first)
        {
            json += L",";
        }

        first = false;

        json +=
            L"{"

            L"\"id\":\"" +
            device->DeviceId +
            L"\","

            L"\"name\":\"" +
            device->DeviceName +
            L"\","

            L"\"port\":\"" +
            device->PortName +
            L"\","

            L"\"connected\":true,"

            L"\"firmware\":\"" +
            device->Firmware +
            L"\","

            L"\"protocol\":\"" +
            device->Protocol +
            L"\","

            L"\"mcu\":\"" +
            device->Mcu +
            L"\","

            L"\"clock\":\"" +
            device->Clock +
            L"\","

            L"\"memory\":{"

                L"\"flash\":\"" +
                device->Flash +
                L"\","

                L"\"sram\":\"" +
                device->Sram +
                L"\","

                L"\"eeprom\":\"" +
                device->Eeprom +
                L"\","

                L"\"psram\":\"" +
                device->Psram +
                L"\""

            L"}"

            L"}";
    }

    json += L"]}";

    return json;
}
//---------------------------------------------------------------------------
void TSerialManagerUnit1::AddSerialMonitorEntry(
    const String& Direction,
    const String& Port,
    const String& DeviceId,
    const String& Text)
{
    std::lock_guard<std::mutex> lock(
		FSerialMonitorMutex
	);

    TSerialMonitorEntry entry;

    entry.Time =
        FormatDateTime(
            L"hh:nn:ss",
            Now()
        );

    entry.Direction =
        Direction;

    entry.Port =
        Port;

    entry.DeviceId =
        DeviceId;

    entry.Text =
        Text;

    FSerialMonitor.push_back(
        entry
    );

    // Evita crescimento ilimitado.
    // Para o monitor do browser, 300 linhas
    // são mais do que suficientes.
    if (FSerialMonitor.size() > 300)
    {
        FSerialMonitor.erase(
            FSerialMonitor.begin()
        );
    }
}
//---------------------------------------------------------------------------
bool TSerialManagerUnit1::IsDeviceConnected(
    const String& deviceId
) const
{
    for (const auto& device : FDevices)
    {
        if (
            device &&
            device->Connected &&
            SameText(
                device->DeviceId,
                deviceId
            )
        )
        {
            return true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------
String TSerialManagerUnit1::GetSerialMonitorJson() const
{
    std::lock_guard<std::mutex> lock(
    FSerialMonitorMutex
    );

    String json =
        L"{\"count\":" +
        IntToStr(
            static_cast<int>(
                FSerialMonitor.size()
            )
        ) +
        L",\"entries\":[";

    bool first = true;

    for (const auto& entry : FSerialMonitor)
    {
        if (!first)
        {
            json += L",";
        }

        first = false;

        String safeText =
            entry.Text;

        safeText =
            StringReplace(
                safeText,
                L"\\",
                L"\\\\",
                TReplaceFlags() << rfReplaceAll
            );

        safeText =
            StringReplace(
                safeText,
                L"\"",
                L"\\\"",
                TReplaceFlags() << rfReplaceAll
            );

        json +=
            L"{"
            L"\"time\":\"" +
            entry.Time +
            L"\","

            L"\"direction\":\"" +
            entry.Direction +
            L"\","

            L"\"port\":\"" +
            entry.Port +
            L"\","

            L"\"device_id\":\"" +
            entry.DeviceId +
            L"\","

            L"\"text\":\"" +
            safeText +
            L"\""
            L"}";
    }

    json += L"]}";

    return json;
}
//---------------------------------------------------------------------------
bool TSerialManagerUnit1::SendCommandToDevice(
    const String& DeviceId,
    const String& Command)
{
    if (DeviceId.IsEmpty())
    {
        ::OutputDebugStringW(
            L"[RIW] SendCommandToDevice: DeviceId vazio"
        );

        return false;
    }

    const String safeCommand =
        Command.Trim();

    if (safeCommand.IsEmpty())
    {
        ::OutputDebugStringW(
            L"[RIW] SendCommandToDevice: comando vazio"
        );

        return false;
    }

    TConnectedDevice* target =
        nullptr;

    for (const auto& device : FDevices)
    {
        if (
            device &&
            device->Connected &&
            SameText(
                device->DeviceId,
                DeviceId
            )
        )
        {
            target =
                device.get();

            break;
        }
    }

    if (target == nullptr)
    {
        ::OutputDebugStringW(
            (
                L"[RIW] SendCommandToDevice: dispositivo nao encontrado: " +
                DeviceId
            ).c_str()
        );

        return false;
    }

    if (!target->Serial)
    {
        ::OutputDebugStringW(
            (
                L"[RIW] SendCommandToDevice: serial indisponivel para " +
                DeviceId
            ).c_str()
        );

        return false;
    }

    const bool sent =
        target->Serial->WriteLine(
            safeCommand
        );

    if (!sent)
    {
        ::OutputDebugStringW(
            (
                L"[RIW] SendCommandToDevice: falha ao enviar para " +
                DeviceId
            ).c_str()
        );

        return false;
    }

    // Registra TX no Monitor Serial
    AddSerialMonitorEntry(
        L"TX",
        target->PortName,
        target->DeviceId,
        safeCommand
    );

    const String log =
        L"[RIW] TX enviado para " +
        target->DeviceId +
        L" em " +
        target->PortName +
        L": " +
        safeCommand;

    ::OutputDebugStringW(
        log.c_str()
    );

    return true;
}
//---------------------------------------------------------------------------
std::vector<String>
TSerialManagerUnit1::GetSignatureLines() const
{
    return FSignatureLines;
}
//---------------------------------------------------------------------------
bool
TSerialManagerUnit1::IsSignatureActive() const
{
    return FSignatureActive;
}
//---------------------------------------------------------------------------
void
TSerialManagerUnit1::ResetSignatureCapture()
{
    FSignatureActive =
        false;

    FSignatureLines.clear();
}
//---------------------------------------------------------------------------
bool TSerialManagerUnit1::SaveCurrentSignatureToSvg()
{
    if (FSignatureLines.empty())
    {
        return false;
    }

    const String eventDate =
    GetCurrentEventDate();

    const String folder =
	ExtractFilePath(
		ParamStr(0)
	) +
	L"data\\gallery\\" +
	eventDate +
	L"\\signatures\\";

    if (!ForceDirectories(folder))
    {
        return false;
    }

    const String fileName =
        L"signature_" +
        FormatDateTime(
            L"yyyymmdd_hhnnss_zzz",
            Now()
        ) +
        L".svg";

    const String fullPath =
        folder +
        fileName;

    String svg =
        L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
        L"<svg xmlns=\"http://www.w3.org/2000/svg\" "
        L"width=\"320\" height=\"240\" "
        L"viewBox=\"0 0 320 240\">\r\n"
        L"<rect width=\"320\" height=\"240\" fill=\"#000000\"/>\r\n"
        L"<g stroke=\"#ffffff\" "
        L"stroke-width=\"3\" "
        L"stroke-linecap=\"round\" "
        L"stroke-linejoin=\"round\">\r\n";

    auto extractNumber =
        [](
            const String& source,
            const String& name,
            int& value
        ) -> bool
    {
        const String marker =
            L"\"" +
            name +
            L"\":";

        const int markerPos =
            source.Pos(
                marker
            );

        if (markerPos <= 0)
        {
            return false;
        }

        const int valueStart =
            markerPos +
            marker.Length();

        String number;

        for (
            int i = valueStart;
            i <= source.Length();
            ++i)
        {
            const wchar_t ch =
                source[i];

            if (
                (ch >= L'0' && ch <= L'9') ||
                ch == L'-')
            {
                number +=
                    ch;
            }
            else
            {
                break;
            }
        }

        if (number.IsEmpty())
        {
            return false;
        }

        value =
            StrToIntDef(
                number,
                -1
            );

        return value >= 0;
    };

    for (
        const String& line :
        FSignatureLines)
    {
        int x1 = 0;
        int y1 = 0;
        int x2 = 0;
        int y2 = 0;

        if (
            !extractNumber(
                line,
                L"x1",
                x1
            ) ||
            !extractNumber(
                line,
                L"y1",
                y1
            ) ||
            !extractNumber(
                line,
                L"x2",
                x2
            ) ||
            !extractNumber(
                line,
                L"y2",
                y2
            ))
        {
            continue;
        }

        svg +=
            L"<line x1=\"" +
            IntToStr(x1) +
            L"\" y1=\"" +
            IntToStr(y1) +
            L"\" x2=\"" +
            IntToStr(x2) +
            L"\" y2=\"" +
            IntToStr(y2) +
            L"\"/>\r\n";
    }

    svg +=
        L"</g>\r\n"
        L"</svg>\r\n";

    FILE* file =
        _wfopen(
            fullPath.c_str(),
            L"wb"
        );

    if (file == nullptr)
    {
        return false;
    }

    const UTF8String utf8 =
        UTF8String(
            svg
        );

    const size_t written =
        fwrite(
            utf8.c_str(),
            1,
            utf8.Length(),
            file
        );

    fclose(
        file
    );

    if (
        written !=
        static_cast<size_t>(
            utf8.Length()
        ))
    {
        return false;
    }

    FLastSignatureFile =
        fileName;

    ::OutputDebugStringW(
        (
            L"[RIW] Assinatura salva: " +
            fullPath
        ).c_str()
    );

    return true;
}
//---------------------------------------------------------------------------
String
TSerialManagerUnit1::GetCurrentEventDate() const
	{
		const TDateTime now =
			Now();

		const int year =
			YearOf(now);

		const int month =
			MonthOf(now);

		const int day =
			DayOf(now);

		if (
			year == 2026 &&
			month == 8)
		{
			if (day == 4)
			{
				return L"2026-08-04";
			}

			if (day == 5)
			{
				return L"2026-08-05";
			}

			if (day == 6)
			{
				return L"2026-08-06";
			}

			if (day == 7)
			{
				return L"2026-08-07";
			}
		}

		return L"2026-08-04";
	}

//---------------------------------------------------------------------------
