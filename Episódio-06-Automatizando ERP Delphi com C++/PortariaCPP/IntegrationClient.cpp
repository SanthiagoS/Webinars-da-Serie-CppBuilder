//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "IntegrationClient.h"

#include <System.SysUtils.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)

TIntegrationClient::TIntegrationClient(const UnicodeString& ApiUrl)
    : FApiUrl(ApiUrl)
{
}

bool TIntegrationClient::SendEmployeeArrival(const TEmployee& Employee,
    const UnicodeString& Terminal, bool FaceConfirmed, bool BleConfirmed)
{
    const UnicodeString FaceValue = FaceConfirmed ? L"true" : L"false";
    const UnicodeString BleValue = BleConfirmed ? L"true" : L"false";

    FLastPayload =
        L"{\r\n"
        L"  \"event\": \"employee_arrival\",\r\n"
        L"  \"employee_id\": " + IntToStr(Employee.Id) + L",\r\n"
        L"  \"name\": \"" + EscapeJson(Employee.Name) + L"\",\r\n"
        L"  \"department\": \"" + EscapeJson(Employee.Department) + L"\",\r\n"
        L"  \"terminal\": \"" + EscapeJson(Terminal) + L"\",\r\n"
        L"  \"face_confirmed\": " + FaceValue + L",\r\n"
        L"  \"ble_confirmed\": " + BleValue + L"\r\n"
        L"}";

    // No network call yet. REST delivery will be added without changing the UI.
    return true;
}

UnicodeString TIntegrationClient::ApiUrl() const
{
    return FApiUrl;
}

UnicodeString TIntegrationClient::LastPayload() const
{
    return FLastPayload;
}

UnicodeString TIntegrationClient::EscapeJson(const UnicodeString& Value) const
{
    UnicodeString Result;

    for (int Index = 1; Index <= Value.Length(); ++Index)
    {
        const wchar_t Ch = Value[Index];
        switch (Ch)
        {
        case L'\\':
            Result += L"\\\\";
            break;
        case L'"':
            Result += L"\\\"";
            break;
        case L'\r':
            Result += L"\\r";
            break;
        case L'\n':
            Result += L"\\n";
            break;
        case L'\t':
            Result += L"\\t";
            break;
        default:
            Result += Ch;
            break;
        }
    }

    return Result;
}
