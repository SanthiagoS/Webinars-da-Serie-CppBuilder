//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "EmployeeSyncClient.h"
#include "EmployeeRepository.h"

#include <IdHTTP.hpp>
#include <System.JSON.hpp>
#include <System.SysUtils.hpp>
#include <System.Threading.hpp>

#include <memory>
#include <vector>
//---------------------------------------------------------------------------
#pragma package(smart_init)

namespace
{
    bool ReadJsonString(
        TJSONObject* Obj,
        const UnicodeString& Name,
        UnicodeString& Value)
    {
        if (Obj == nullptr)
            return false;

        TJSONValue* JsonValue = Obj->GetValue(Name);

        if (JsonValue == nullptr)
            return false;

        Value = JsonValue->Value();
        return true;
    }

    bool ReadJsonAny(
        TJSONObject* Obj,
        const UnicodeString& Name1,
        const UnicodeString& Name2,
        const UnicodeString& Name3,
        UnicodeString& Value)
    {
        Value = L"";

        if (ReadJsonString(Obj, Name1, Value))
            return true;

        if (!Name2.IsEmpty() &&
            ReadJsonString(Obj, Name2, Value))
        {
            return true;
        }

        if (!Name3.IsEmpty() &&
            ReadJsonString(Obj, Name3, Value))
        {
            return true;
        }

        return false;
    }

    bool ParseEmployee(
        TJSONObject* Obj,
        TEmployee& Employee)
    {
        Employee = TEmployee();

        if (Obj == nullptr)
            return false;

        UnicodeString IdText;

        ReadJsonAny(
            Obj,
            L"id",
            L"ID",
            L"employeeId",
            IdText);

        Employee.Id =
            StrToIntDef(IdText, 0);

        if (Employee.Id <= 0)
            return false;

        ReadJsonAny(
            Obj,
            L"name",
            L"Name",
            L"employeeName",
            Employee.Name);

        ReadJsonAny(
            Obj,
            L"department",
            L"Department",
            L"",
            Employee.Department);

        ReadJsonAny(
            Obj,
            L"email",
            L"Email",
            L"e-mail",
            Employee.Email);

        ReadJsonAny(
            Obj,
            L"phone",
            L"Phone",
            L"",
            Employee.Phone);

        ReadJsonAny(
            Obj,
            L"seniority",
            L"Seniority",
            L"",
            Employee.Seniority);

        ReadJsonAny(
            Obj,
            L"faceKey",
            L"FaceKey",
            L"identityKey",
            Employee.FaceKey);

        ReadJsonAny(
            Obj,
            L"rfidTag",
            L"RFIDTag",
            L"rfid",
            Employee.BleId);

        Employee.PhotoPath = L"";
        Employee.FaceEmbedding.clear();
        Employee.HasFaceEmbedding = false;

        return true;
    }

    bool ExtractEmployeeArray(
        TJSONValue* RootValue,
        TJSONArray*& Items)
    {
        Items = nullptr;

        if (RootValue == nullptr)
            return false;

        Items =
            dynamic_cast<TJSONArray*>(
                RootValue);

        if (Items != nullptr)
            return true;

        TJSONObject* Root =
            dynamic_cast<TJSONObject*>(
                RootValue);

        if (Root == nullptr)
            return false;

        if (Root->TryGetValue<TJSONArray*>(
                L"employees",
                Items) &&
            Items != nullptr)
        {
            return true;
        }

        if (Root->TryGetValue<TJSONArray*>(
                L"Employees",
                Items) &&
            Items != nullptr)
        {
            return true;
        }

        if (Root->TryGetValue<TJSONArray*>(
                L"data",
                Items) &&
            Items != nullptr)
        {
            return true;
        }

        return false;
    }

    bool ParseEmployeesResponse(
        const UnicodeString& Content,
        std::vector<TEmployee>& Employees)
    {
        Employees.clear();

        std::unique_ptr<TJSONValue> Parsed(
            TJSONObject::ParseJSONValue(Content));

        if (Parsed == nullptr)
            return false;

        TJSONArray* Items = nullptr;

        if (!ExtractEmployeeArray(
                Parsed.get(),
                Items))
        {
            return false;
        }

        for (int Index = 0;
             Index < Items->Count;
             ++Index)
        {
            TJSONObject* Obj =
                dynamic_cast<TJSONObject*>(
                    Items->Items[Index]);

            if (Obj == nullptr)
                continue;

            TEmployee Employee;

            if (ParseEmployee(
                    Obj,
                    Employee))
            {
                Employees.push_back(Employee);
            }
        }

        // Zero funcionários também é um snapshot válido.
        return true;
    }
}

TEmployeeSyncClient::TEmployeeSyncClient(
    TEmployeeRepository* Repository,
    const UnicodeString& ApiUrl)
    : FTimer(nullptr),
      FRepository(Repository),
      FApiUrl(ApiUrl),
      FBusy(false),
      FEnabled(false),
      FStatusPending(false)
{
    FTimer = new TTimer(nullptr);
    FTimer->Enabled = false;
    FTimer->Interval = 5000;
    FTimer->OnTimer = TimerTick;
}

TEmployeeSyncClient::~TEmployeeSyncClient()
{
    Stop();

    // A tarefa usa "this" apenas para publicar o resultado.
    // Antes de destruir o objeto, aguarda uma requisição curta em andamento.
    for (int WaitIndex = 0;
         WaitIndex < 350 && FBusy.load();
         ++WaitIndex)
    {
        Sleep(10);
    }

    delete FTimer;
    FTimer = nullptr;
}

void TEmployeeSyncClient::Start()
{
    if (FEnabled)
        return;

    FEnabled = true;

    if (FTimer != nullptr)
        FTimer->Enabled = true;

    SyncNow();
}

void TEmployeeSyncClient::Stop()
{
    FEnabled = false;

    if (FTimer != nullptr)
        FTimer->Enabled = false;
}

void TEmployeeSyncClient::SyncNow()
{
    if (!FEnabled)
        return;

    SyncAsync();
}

bool TEmployeeSyncClient::TryConsumeStatus(
    TEmployeeSyncStatus& Status)
{
    std::lock_guard<std::mutex> Lock(
        FStateMutex);

    if (!FStatusPending)
        return false;

    Status = FLastStatus;
    FStatusPending = false;

    return true;
}

UnicodeString TEmployeeSyncClient::EmployeesEndpoint(
    const UnicodeString& ApiUrl)
{
    UnicodeString Url =
        Trim(ApiUrl);

    while (!Url.IsEmpty() &&
           Url[Url.Length()] == L'/')
    {
        Url =
            Url.SubString(
                1,
                Url.Length() - 1);
    }

    if (Url.IsEmpty())
        return L"";

    const UnicodeString LowerUrl =
        LowerCase(Url);

    const int ApiPosition =
        LowerUrl.Pos(L"/api");

    if (ApiPosition > 0)
    {
        return
            Url.SubString(
                1,
                ApiPosition - 1) +
            L"/api/employees";
    }

    return Url + L"/api/employees";
}

void __fastcall TEmployeeSyncClient::TimerTick(
    TObject* Sender)
{
    (void)Sender;

    if (!FEnabled)
        return;

    SyncAsync();
}

void TEmployeeSyncClient::PublishStatus(
    const TEmployeeSyncStatus& Status)
{
    std::lock_guard<std::mutex> Lock(
        FStateMutex);

    FLastStatus = Status;
    FStatusPending = true;
}

void TEmployeeSyncClient::SyncAsync()
{
    bool Expected = false;

    if (!FBusy.compare_exchange_strong(
            Expected,
            true))
    {
        return;
    }

    const UnicodeString Endpoint =
        EmployeesEndpoint(FApiUrl);

    TEmployeeRepository* Repository =
        FRepository;

    TTask::Run(
        [this, Endpoint, Repository]()
        {
            TEmployeeSyncStatus Status;
            Status.Endpoint = Endpoint;

            try
            {
                if (Endpoint.IsEmpty())
                {
                    throw Exception(
                        L"Endpoint do ERP Delphi não configurado.");
                }

                if (Repository == nullptr)
                {
                    throw Exception(
                        L"EmployeeRepository indisponível.");
                }

                std::unique_ptr<TIdHTTP> Http(
                    new TIdHTTP(nullptr));

                Http->ConnectTimeout = 1200;
                Http->ReadTimeout = 1800;
                Http->Request->Accept =
                    L"application/json";
                Http->Request->CharSet =
                    L"utf-8";

                const UnicodeString Content =
                    Http->Get(Endpoint);

                std::vector<TEmployee> Employees;

                if (!ParseEmployeesResponse(
                        Content,
                        Employees))
                {
                    throw Exception(
                        L"JSON de funcionários inválido.");
                }

                Repository->ReplaceRemoteSnapshot(
                    Employees);

                Status.Success = true;
                Status.EmployeeCount =
                    static_cast<int>(
                        Employees.size());
                Status.Message =
                    L"Funcionários sincronizados do ERP Delphi.";
            }
            catch (const Exception& E)
            {
                Status.Success = false;
                Status.EmployeeCount = 0;
                Status.Message = E.Message;
            }
            catch (...)
            {
                Status.Success = false;
                Status.EmployeeCount = 0;
                Status.Message =
                    L"Falha não identificada na sincronização.";
            }

            PublishStatus(Status);
            FBusy = false;
        });
}
//---------------------------------------------------------------------------

