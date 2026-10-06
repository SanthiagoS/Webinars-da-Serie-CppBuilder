//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "EmployeeRepository.h"

#include <FireDAC.Comp.Client.hpp>
#include <FireDAC.Phys.SQLite.hpp>
#include <FireDAC.Stan.Async.hpp>
#include <FireDAC.Stan.Def.hpp>
#include <FireDAC.Stan.Intf.hpp>
#include <FireDAC.Stan.Option.hpp>
#include <FireDAC.VCLUI.Wait.hpp>
#include <System.SysUtils.hpp>
#include <mutex>

#include <FireDAC.DApt.Intf.hpp>
#include <FireDAC.DApt.hpp>
#include <FireDAC.Phys.SQLiteWrapper.Stat.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)



namespace
{
    using Firedac::Comp::Client::TFDConnection;
    using Firedac::Comp::Client::TFDQuery;

    std::unique_ptr<TFDConnection> CreateConnection(const UnicodeString& DatabasePath)
    {
        std::unique_ptr<TFDConnection> Connection(new TFDConnection(nullptr));
        Connection->LoginPrompt = false;
        Connection->Params->Clear();
        Connection->Params->Values[L"DriverID"] = L"SQLite";
        Connection->Params->Values[L"Database"] = DatabasePath;
        Connection->Connected = true;
        return Connection;
    }
}

TEmployeeRepository::TEmployeeRepository(const UnicodeString& DatabasePath)
    : FDatabasePath(DatabasePath)
{
}

bool TEmployeeRepository::FindById(int Id, TEmployee& Employee) const
{
    Employee = TEmployee();

    if (Id <= 0)
    {
        return false;
    }

    // Depois do primeiro snapshot válido, o ERP Delphi é a fonte oficial.
    {
        std::lock_guard<std::mutex> Lock(FRemoteMutex);

        if (FRemoteSnapshotReady)
        {
            for (const TEmployee& Item : FRemoteEmployees)
            {
                if (Item.Id == Id)
                {
                    Employee = Item;
                    return true;
                }
            }

            return false;
        }
    }

    // Fallback legado: usado somente antes do primeiro snapshot do ERP.
    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"SELECT ID, Name, Phone, [E-mail] AS Email, Department, Seniority, RFIDTag "
            L"FROM Employee WHERE ID = :ID";
        Query->ParamByName(L"ID")->AsInteger = Id;
        Query->Open();

        if (Query->Eof)
        {
            return false;
        }

        return PopulateEmployeeFromCurrentRow(Employee, Query.get());
    }
    catch (const Exception&)
    {
        return false;
    }
}

bool TEmployeeRepository::FindByBleId(const UnicodeString& DeviceId, TEmployee& Employee) const
{
    (void)DeviceId;
    Employee = TEmployee();
    return false;
}

std::vector<TEmployee> TEmployeeRepository::GetAll() const
{
    // Snapshot remoto tem precedência, inclusive quando contém zero registros.
    {
        std::lock_guard<std::mutex> Lock(FRemoteMutex);

        if (FRemoteSnapshotReady)
        {
            return FRemoteEmployees;
        }
    }

    std::vector<TEmployee> Employees;

    try
    {
        if (!FileExists(FDatabasePath))
        {
            const UnicodeString Msg =
                L"Employees.s3db não encontrado.\r\n\r\nCaminho usado pelo C++:\r\n" +
                FDatabasePath;

            Application->MessageBox(
                Msg.c_str(),
                L"EmployeeRepository",
                MB_OK | MB_ICONERROR);

            OutputDebugStringW(
                (L"[EmployeeRepository] Banco não encontrado: " +
                 FDatabasePath + L"\r\n").c_str());

            return Employees;
        }

        std::unique_ptr<TFDConnection> Connection = CreateConnection(FDatabasePath);

        // Diagnóstico 1: confirmar exatamente qual arquivo foi aberto e quantos
        // registros existem na tabela Employee.
        {
            std::unique_ptr<TFDQuery> CountQuery(new TFDQuery(nullptr));
            CountQuery->Connection = Connection.get();
            CountQuery->SQL->Text = L"SELECT COUNT(*) AS Total FROM Employee";
            CountQuery->Open();

            const int Total = CountQuery->FieldByName(L"Total")->AsInteger;

            OutputDebugStringW(
                (L"[EmployeeRepository] Database=" + FDatabasePath +
                 L" | Employee.Count=" + IntToStr(Total) + L"\r\n").c_str());

            if (Total == 0)
            {
                const UnicodeString Msg =
                    L"O banco foi aberto corretamente, mas a tabela Employee possui 0 registros."
                    L"\r\n\r\nArquivo utilizado:\r\n" + FDatabasePath +
                    L"\r\n\r\nProvavelmente este não é o mesmo Employees.s3db usado pelo sistema Delphi.";

                Application->MessageBox(
                    Msg.c_str(),
                    L"Employees.s3db sem funcionários",
                    MB_OK | MB_ICONWARNING);

                return Employees;
            }
        }

        // RFIDTag não é necessário para o cadastro facial/BLE nesta etapa e o
        // código atual nem utiliza seu valor. Mantemos a consulta somente nos
        // campos efetivamente lidos por PopulateEmployeeFromCurrentRow().
        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"SELECT ID, Name, Phone, [E-mail] AS Email, Department, Seniority "
            L"FROM Employee ORDER BY Name";
        Query->Open();

        while (!Query->Eof)
        {
            TEmployee Employee;
            if (PopulateEmployeeFromCurrentRow(Employee, Query.get()))
            {
                Employees.push_back(Employee);
            }
            Query->Next();
        }

        OutputDebugStringW(
            (L"[EmployeeRepository] Funcionários carregados=" +
             IntToStr(static_cast<int>(Employees.size())) + L"\r\n").c_str());
    }
    catch (const Exception& E)
    {
        Employees.clear();

        const UnicodeString Msg =
            L"Erro ao consultar Employees.s3db.\r\n\r\nCaminho:\r\n" +
            FDatabasePath + L"\r\n\r\nErro:\r\n" + E.Message;

        Application->MessageBox(
            Msg.c_str(),
            L"Erro no EmployeeRepository",
            MB_OK | MB_ICONERROR);

        OutputDebugStringW(
            (L"[EmployeeRepository] GetAll ERROR: " +
             E.Message + L" | DB=" + FDatabasePath + L"\r\n").c_str());
    }

    return Employees;
}

void TEmployeeRepository::ReplaceRemoteSnapshot(
    const std::vector<TEmployee>& Employees)
{
    std::lock_guard<std::mutex> Lock(FRemoteMutex);

    FRemoteEmployees = Employees;
    FRemoteSnapshotReady = true;
}

void TEmployeeRepository::ClearRemoteSnapshot()
{
    std::lock_guard<std::mutex> Lock(FRemoteMutex);

    FRemoteEmployees.clear();
    FRemoteSnapshotReady = false;
}

bool TEmployeeRepository::HasRemoteSnapshot() const
{
    std::lock_guard<std::mutex> Lock(FRemoteMutex);
    return FRemoteSnapshotReady;
}

int TEmployeeRepository::RemoteEmployeeCount() const
{
    std::lock_guard<std::mutex> Lock(FRemoteMutex);

    if (!FRemoteSnapshotReady)
        return -1;

    return static_cast<int>(FRemoteEmployees.size());
}

UnicodeString TEmployeeRepository::DatabasePath() const
{
    return FDatabasePath;
}

bool TEmployeeRepository::PopulateEmployeeFromCurrentRow(TEmployee& Employee,
    TFDQuery* Query) const
{
    if (Query == nullptr || Query->Eof)
    {
        return false;
    }

    Employee.Id = Query->FieldByName(L"ID")->AsInteger;
    Employee.Name = Query->FieldByName(L"Name")->AsString;
    Employee.Phone = Query->FieldByName(L"Phone")->AsString;
    Employee.Email = Query->FieldByName(L"Email")->AsString;
    Employee.Department = Query->FieldByName(L"Department")->AsString;
    Employee.Seniority = Query->FieldByName(L"Seniority")->AsString;
    Employee.FaceKey = L"";
    Employee.BleId = L"";
    Employee.PhotoPath = L"";
    Employee.HasFaceEmbedding = false;
    Employee.FaceEmbedding.clear();
    return Employee.Id > 0;
}

