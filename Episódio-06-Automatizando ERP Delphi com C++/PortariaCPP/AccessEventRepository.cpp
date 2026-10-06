//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "AccessEventRepository.h"

#include <FireDAC.Comp.Client.hpp>
#include <FireDAC.DApt.hpp>
#include <FireDAC.DApt.Intf.hpp>
#include <FireDAC.Phys.SQLite.hpp>
#include <FireDAC.Phys.SQLiteWrapper.Stat.hpp>
#include <FireDAC.Stan.Async.hpp>
#include <FireDAC.Stan.Def.hpp>
#include <FireDAC.Stan.Intf.hpp>
#include <FireDAC.Stan.Option.hpp>

#include <System.IOUtils.hpp>
#include <System.SysUtils.hpp>
#include <Winapi.Windows.hpp>

#include <algorithm>
#include <memory>
//---------------------------------------------------------------------------
#pragma package(smart_init)

namespace
{
    std::unique_ptr<TFDConnection> CreateAccessConnection(
        const UnicodeString& DatabasePath)
    {
        std::unique_ptr<TFDConnection> Connection(new TFDConnection(nullptr));
        Connection->LoginPrompt = false;
        Connection->Params->Clear();
        Connection->Params->Values[L"DriverID"] = L"SQLite";
        Connection->Params->Values[L"Database"] = DatabasePath;
        Connection->Params->Values[L"OpenMode"] = L"CreateUTF8";
        Connection->Params->Values[L"LockingMode"] = L"Normal";
        Connection->Connected = true;
        return Connection;
    }

    UnicodeString SafeField(TFDQuery* Query, const UnicodeString& Name)
    {
        if (Query == nullptr)
        {
            return UnicodeString();
        }

        TField* Field = Query->FindField(Name);
        return Field == nullptr || Field->IsNull
            ? UnicodeString()
            : Field->AsString;
    }
}

TAccessEventRepository::TAccessEventRepository(
    const UnicodeString& DatabasePath)
    : FDatabasePath(DatabasePath)
{
}

bool TAccessEventRepository::Initialize() const
{
    try
    {
        const UnicodeString Directory = ExtractFilePath(FDatabasePath);
        if (!Directory.IsEmpty() && !DirectoryExists(Directory))
        {
            ForceDirectories(Directory);
        }

        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"CREATE TABLE IF NOT EXISTS AccessEvent ("
            L"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
            L"EmployeeID INTEGER NOT NULL, "
            L"EmployeeName TEXT NOT NULL, "
            L"Department TEXT, "
            L"EventType TEXT NOT NULL, "
            L"IdentificationMethod TEXT NOT NULL, "
            L"EventDateTime TEXT NOT NULL, "
            L"Terminal TEXT NOT NULL, "
            L"Status TEXT NOT NULL, "
            L"RecognitionScore REAL DEFAULT 0"
            L")";
        Query->ExecSQL();

        Query->SQL->Text =
            L"CREATE INDEX IF NOT EXISTS IX_AccessEvent_DateTime "
            L"ON AccessEvent(EventDateTime DESC)";
        Query->ExecSQL();

        Query->SQL->Text =
            L"CREATE TABLE IF NOT EXISTS Occurrence ("
            L"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
            L"OccurrenceType TEXT NOT NULL, "
            L"Description TEXT NOT NULL, "
            L"EmployeeName TEXT, "
            L"EventDateTime TEXT NOT NULL, "
            L"Terminal TEXT NOT NULL, "
            L"Status TEXT NOT NULL"
            L")";
        Query->ExecSQL();

        Query->SQL->Text =
            L"CREATE INDEX IF NOT EXISTS IX_Occurrence_DateTime "
            L"ON Occurrence(EventDateTime DESC)";
        Query->ExecSQL();

        Query->SQL->Text =
            L"CREATE TABLE IF NOT EXISTS AccessBlock ("
            L"EmployeeID INTEGER PRIMARY KEY, "
            L"EmployeeName TEXT NOT NULL, "
            L"Blocked INTEGER NOT NULL DEFAULT 1, "
            L"Reason TEXT, "
            L"UpdatedAt TEXT NOT NULL, "
            L"Terminal TEXT NOT NULL"
            L")";
        Query->ExecSQL();

        Query->SQL->Text =
            L"CREATE TABLE IF NOT EXISTS RfidTagAssignment ("
            L"EmployeeID INTEGER PRIMARY KEY, "
            L"IdentityKey TEXT, "
            L"EmployeeName TEXT NOT NULL, "
            L"Department TEXT, "
            L"TagUid TEXT NOT NULL UNIQUE, "
            L"TagType TEXT NOT NULL, "
            L"Active INTEGER NOT NULL DEFAULT 1, "
            L"CreatedAt TEXT NOT NULL"
            L")";
        Query->ExecSQL();

        // Migração compatível com access.s3db já existente.
        bool HasIdentityKey = false;

        Query->SQL->Text =
            L"PRAGMA table_info(RfidTagAssignment)";
        Query->Open();

        while (!Query->Eof)
        {
            if (SameText(
                    SafeField(Query.get(), L"name"),
                    L"IdentityKey"))
            {
                HasIdentityKey = true;
                break;
            }

            Query->Next();
        }

        Query->Close();

        if (!HasIdentityKey)
        {
            Query->SQL->Text =
                L"ALTER TABLE RfidTagAssignment "
                L"ADD COLUMN IdentityKey TEXT";
            Query->ExecSQL();
        }

        Query->SQL->Text =
            L"CREATE INDEX IF NOT EXISTS IX_RfidTagAssignment_TagUid "
            L"ON RfidTagAssignment(TagUid)";
        Query->ExecSQL();

        OutputDebugStringW(
            (L"[AccessEventRepository] Ready: " + FDatabasePath +
             L"\r\n").c_str());
        return true;
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] Initialize ERROR: " +
             E.Message + L"\r\n").c_str());
        return false;
    }
}

bool TAccessEventRepository::Add(const TAccessEvent& Event) const
{
    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"INSERT INTO AccessEvent ("
            L"EmployeeID, EmployeeName, Department, EventType, "
            L"IdentificationMethod, EventDateTime, Terminal, Status, "
            L"RecognitionScore) "
            L"VALUES (:EmployeeID, :EmployeeName, :Department, :EventType, "
            L":IdentificationMethod, :EventDateTime, :Terminal, :Status, "
            L":RecognitionScore)";

        Query->ParamByName(L"EmployeeID")->AsInteger = Event.EmployeeId;
        Query->ParamByName(L"EmployeeName")->AsString = Event.EmployeeName;
        Query->ParamByName(L"Department")->AsString = Event.Department;
        Query->ParamByName(L"EventType")->AsString = Event.EventType;
        Query->ParamByName(L"IdentificationMethod")->AsString =
            Event.IdentificationMethod;
        Query->ParamByName(L"EventDateTime")->AsString = Event.EventDateTime;
        Query->ParamByName(L"Terminal")->AsString = Event.Terminal;
        Query->ParamByName(L"Status")->AsString = Event.Status;
        Query->ParamByName(L"RecognitionScore")->AsFloat =
            Event.RecognitionScore;
        Query->ExecSQL();

        return true;
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] Add ERROR: " +
             E.Message + L"\r\n").c_str());
        return false;
    }
}

std::vector<TAccessEvent> TAccessEventRepository::GetRecent(int Limit) const
{
    std::vector<TAccessEvent> Events;

    try
    {
        Limit = std::max(1, std::min(Limit, 200));

        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"SELECT ID, EmployeeID, EmployeeName, Department, EventType, "
            L"IdentificationMethod, EventDateTime, Terminal, Status, "
            L"RecognitionScore "
            L"FROM AccessEvent "
            L"ORDER BY ID DESC "
            L"LIMIT " + IntToStr(Limit);
        Query->Open();

        while (!Query->Eof)
        {
            TAccessEvent Event;
            Event.Id = Query->FieldByName(L"ID")->AsInteger;
            Event.EmployeeId =
                Query->FieldByName(L"EmployeeID")->AsInteger;
            Event.EmployeeName = SafeField(Query.get(), L"EmployeeName");
            Event.Department = SafeField(Query.get(), L"Department");
            Event.EventType = SafeField(Query.get(), L"EventType");
            Event.IdentificationMethod =
                SafeField(Query.get(), L"IdentificationMethod");
            Event.EventDateTime = SafeField(Query.get(), L"EventDateTime");
            Event.Terminal = SafeField(Query.get(), L"Terminal");
            Event.Status = SafeField(Query.get(), L"Status");
            Event.RecognitionScore =
                Query->FieldByName(L"RecognitionScore")->AsFloat;

            Events.push_back(Event);
            Query->Next();
        }
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] GetRecent ERROR: " +
             E.Message + L"\r\n").c_str());
        Events.clear();
    }

    return Events;
}


std::vector<TAccessEvent> TAccessEventRepository::GetPresentToday() const
{
    std::vector<TAccessEvent> Events;

    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();

        // Nesta etapa do projeto só registramos ENTRADA automática.
        // Portanto, "presentes" = um registro por funcionário identificado hoje.
        // Quando adicionarmos SAÍDA, esta consulta será evoluída para considerar
        // o último evento do funcionário.
        Query->SQL->Text =
            L"SELECT A.ID, A.EmployeeID, A.EmployeeName, A.Department, "
            L"A.EventType, A.IdentificationMethod, A.EventDateTime, "
            L"A.Terminal, A.Status, A.RecognitionScore "
            L"FROM AccessEvent A "
            L"INNER JOIN ("
            L"  SELECT EmployeeID, MAX(ID) AS MaxID "
            L"  FROM AccessEvent "
            L"  WHERE EventType = 'ENTRADA' "
            L"    AND substr(EventDateTime, 1, 10) = :Today "
            L"  GROUP BY EmployeeID"
            L") P ON P.MaxID = A.ID "
            L"ORDER BY A.EmployeeName COLLATE NOCASE";

        Query->ParamByName(L"Today")->AsString =
            FormatDateTime(L"yyyy-mm-dd", Now());
        Query->Open();

        while (!Query->Eof)
        {
            TAccessEvent Event;
            Event.Id = Query->FieldByName(L"ID")->AsInteger;
            Event.EmployeeId =
                Query->FieldByName(L"EmployeeID")->AsInteger;
            Event.EmployeeName = SafeField(Query.get(), L"EmployeeName");
            Event.Department = SafeField(Query.get(), L"Department");
            Event.EventType = SafeField(Query.get(), L"EventType");
            Event.IdentificationMethod =
                SafeField(Query.get(), L"IdentificationMethod");
            Event.EventDateTime = SafeField(Query.get(), L"EventDateTime");
            Event.Terminal = SafeField(Query.get(), L"Terminal");
            Event.Status = SafeField(Query.get(), L"Status");
            Event.RecognitionScore =
                Query->FieldByName(L"RecognitionScore")->AsFloat;

            Events.push_back(Event);
            Query->Next();
        }
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] GetPresentToday ERROR: " +
             E.Message + L"\r\n").c_str());
        Events.clear();
    }

    return Events;
}


bool TAccessEventRepository::AddOccurrence(
    const TOccurrence& Occurrence) const
{
    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"INSERT INTO Occurrence ("
            L"OccurrenceType, Description, EmployeeName, "
            L"EventDateTime, Terminal, Status) "
            L"VALUES (:OccurrenceType, :Description, :EmployeeName, "
            L":EventDateTime, :Terminal, :Status)";

        Query->ParamByName(L"OccurrenceType")->AsString =
            Occurrence.OccurrenceType;
        Query->ParamByName(L"Description")->AsString =
            Occurrence.Description;
        Query->ParamByName(L"EmployeeName")->AsString =
            Occurrence.EmployeeName;
        Query->ParamByName(L"EventDateTime")->AsString =
            Occurrence.EventDateTime;
        Query->ParamByName(L"Terminal")->AsString =
            Occurrence.Terminal;
        Query->ParamByName(L"Status")->AsString =
            Occurrence.Status;
        Query->ExecSQL();

        return true;
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] AddOccurrence ERROR: " +
             E.Message + L"\r\n").c_str());
        return false;
    }
}

std::vector<TOccurrence> TAccessEventRepository::GetRecentOccurrences(
    int Limit) const
{
    std::vector<TOccurrence> Occurrences;

    try
    {
        Limit = std::max(1, std::min(Limit, 200));

        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"SELECT ID, OccurrenceType, Description, EmployeeName, "
            L"EventDateTime, Terminal, Status "
            L"FROM Occurrence "
            L"ORDER BY ID DESC "
            L"LIMIT " + IntToStr(Limit);
        Query->Open();

        while (!Query->Eof)
        {
            TOccurrence Occurrence;
            Occurrence.Id = Query->FieldByName(L"ID")->AsInteger;
            Occurrence.OccurrenceType =
                SafeField(Query.get(), L"OccurrenceType");
            Occurrence.Description =
                SafeField(Query.get(), L"Description");
            Occurrence.EmployeeName =
                SafeField(Query.get(), L"EmployeeName");
            Occurrence.EventDateTime =
                SafeField(Query.get(), L"EventDateTime");
            Occurrence.Terminal =
                SafeField(Query.get(), L"Terminal");
            Occurrence.Status =
                SafeField(Query.get(), L"Status");

            Occurrences.push_back(Occurrence);
            Query->Next();
        }
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] GetRecentOccurrences ERROR: " +
             E.Message + L"\r\n").c_str());
        Occurrences.clear();
    }

    return Occurrences;
}


bool TAccessEventRepository::SetEmployeeBlocked(
    int EmployeeId,
    const UnicodeString& EmployeeName,
    bool Blocked,
    const UnicodeString& Reason,
    const UnicodeString& Terminal) const
{
    if (EmployeeId <= 0)
    {
        return false;
    }

    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();

        Query->SQL->Text =
            L"INSERT OR REPLACE INTO AccessBlock ("
            L"EmployeeID, EmployeeName, Blocked, Reason, UpdatedAt, Terminal) "
            L"VALUES (:EmployeeID, :EmployeeName, :Blocked, :Reason, "
            L":UpdatedAt, :Terminal)";

        Query->ParamByName(L"EmployeeID")->AsInteger = EmployeeId;
        Query->ParamByName(L"EmployeeName")->AsString = EmployeeName;
        Query->ParamByName(L"Blocked")->AsInteger = Blocked ? 1 : 0;
        Query->ParamByName(L"Reason")->AsString = Reason;
        Query->ParamByName(L"UpdatedAt")->AsString =
            FormatDateTime(L"yyyy-mm-dd hh:nn:ss", Now());
        Query->ParamByName(L"Terminal")->AsString = Terminal;
        Query->ExecSQL();

        return true;
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] SetEmployeeBlocked ERROR: " +
             E.Message + L"\r\n").c_str());
        return false;
    }
}

bool TAccessEventRepository::IsEmployeeBlocked(
    int EmployeeId,
    UnicodeString& Reason) const
{
    Reason = UnicodeString();

    if (EmployeeId <= 0)
    {
        return false;
    }

    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"SELECT Blocked, Reason "
            L"FROM AccessBlock "
            L"WHERE EmployeeID = :EmployeeID";

        Query->ParamByName(L"EmployeeID")->AsInteger = EmployeeId;
        Query->Open();

        if (Query->Eof)
        {
            return false;
        }

        const bool Blocked =
            Query->FieldByName(L"Blocked")->AsInteger != 0;

        if (Blocked)
        {
            Reason = SafeField(Query.get(), L"Reason");
        }

        return Blocked;
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] IsEmployeeBlocked ERROR: " +
             E.Message + L"\r\n").c_str());
        return false;
    }
}


bool TAccessEventRepository::AssignRfidTag(
    int EmployeeId,
    const UnicodeString& IdentityKey,
    const UnicodeString& EmployeeName,
    const UnicodeString& Department,
    const UnicodeString& TagUid,
    const UnicodeString& TagType) const
{
    if (EmployeeId <= 0 ||
        Trim(IdentityKey).IsEmpty() ||
        Trim(TagUid).IsEmpty())
    {
        return false;
    }

    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();

        // Remove eventual vínculo anterior do mesmo UID para garantir
        // uma TAG física vinculada a somente um funcionário.
        Query->SQL->Text =
            L"DELETE FROM RfidTagAssignment "
            L"WHERE TagUid = :TagUid AND EmployeeID <> :EmployeeID";
        Query->ParamByName(L"TagUid")->AsString = TagUid;
        Query->ParamByName(L"EmployeeID")->AsInteger = EmployeeId;
        Query->ExecSQL();

        Query->SQL->Text =
            L"INSERT OR REPLACE INTO RfidTagAssignment ("
            L"EmployeeID, IdentityKey, EmployeeName, Department, TagUid, "
            L"TagType, Active, CreatedAt) "
            L"VALUES (:EmployeeID, :IdentityKey, :EmployeeName, :Department, "
            L":TagUid, :TagType, 1, :CreatedAt)";

        Query->ParamByName(L"EmployeeID")->AsInteger = EmployeeId;
        Query->ParamByName(L"IdentityKey")->AsString = IdentityKey;
        Query->ParamByName(L"EmployeeName")->AsString = EmployeeName;
        Query->ParamByName(L"Department")->AsString = Department;
        Query->ParamByName(L"TagUid")->AsString = TagUid;
        Query->ParamByName(L"TagType")->AsString = TagType;
        Query->ParamByName(L"CreatedAt")->AsString =
            FormatDateTime(L"yyyy-mm-dd hh:nn:ss", Now());
        Query->ExecSQL();

        return true;
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] AssignRfidTag ERROR: " +
             E.Message + L"\r\n").c_str());
        return false;
    }
}

bool TAccessEventRepository::RemoveRfidTag(int EmployeeId) const
{
    if (EmployeeId <= 0)
    {
        return false;
    }

    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"DELETE FROM RfidTagAssignment WHERE EmployeeID = :EmployeeID";
        Query->ParamByName(L"EmployeeID")->AsInteger = EmployeeId;
        Query->ExecSQL();

        return true;
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] RemoveRfidTag ERROR: " +
             E.Message + L"\r\n").c_str());
        return false;
    }
}

bool TAccessEventRepository::GetRfidTagByEmployee(
    int EmployeeId,
    TRfidTagAssignment& Assignment) const
{
    Assignment = TRfidTagAssignment();

    if (EmployeeId <= 0)
    {
        return false;
    }

    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"SELECT EmployeeID, IdentityKey, EmployeeName, Department, TagUid, "
            L"TagType, Active, CreatedAt "
            L"FROM RfidTagAssignment "
            L"WHERE EmployeeID = :EmployeeID";
        Query->ParamByName(L"EmployeeID")->AsInteger = EmployeeId;
        Query->Open();

        if (Query->Eof)
        {
            return false;
        }

        Assignment.EmployeeId =
            Query->FieldByName(L"EmployeeID")->AsInteger;
        Assignment.IdentityKey =
            SafeField(Query.get(), L"IdentityKey");
        Assignment.EmployeeName =
            SafeField(Query.get(), L"EmployeeName");
        Assignment.Department =
            SafeField(Query.get(), L"Department");
        Assignment.TagUid =
            SafeField(Query.get(), L"TagUid");
        Assignment.TagType =
            SafeField(Query.get(), L"TagType");
        Assignment.Active =
            Query->FieldByName(L"Active")->AsInteger != 0;
        Assignment.CreatedAt =
            SafeField(Query.get(), L"CreatedAt");

        return true;
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] GetRfidTagByEmployee ERROR: " +
             E.Message + L"\r\n").c_str());
        return false;
    }
}

bool TAccessEventRepository::GetRfidTagByUid(
    const UnicodeString& TagUid,
    TRfidTagAssignment& Assignment) const
{
    Assignment = TRfidTagAssignment();

    if (Trim(TagUid).IsEmpty())
    {
        return false;
    }

    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"SELECT EmployeeID, IdentityKey, EmployeeName, Department, TagUid, "
            L"TagType, Active, CreatedAt "
            L"FROM RfidTagAssignment "
            L"WHERE TagUid = :TagUid AND Active = 1";
        Query->ParamByName(L"TagUid")->AsString = TagUid;
        Query->Open();

        if (Query->Eof)
        {
            return false;
        }

        Assignment.EmployeeId =
            Query->FieldByName(L"EmployeeID")->AsInteger;
        Assignment.IdentityKey =
            SafeField(Query.get(), L"IdentityKey");
        Assignment.EmployeeName =
            SafeField(Query.get(), L"EmployeeName");
        Assignment.Department =
            SafeField(Query.get(), L"Department");
        Assignment.TagUid =
            SafeField(Query.get(), L"TagUid");
        Assignment.TagType =
            SafeField(Query.get(), L"TagType");
        Assignment.Active =
            Query->FieldByName(L"Active")->AsInteger != 0;
        Assignment.CreatedAt =
            SafeField(Query.get(), L"CreatedAt");

        return true;
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] GetRfidTagByUid ERROR: " +
             E.Message + L"\r\n").c_str());
        return false;
    }
}

std::vector<TRfidTagAssignment> TAccessEventRepository::GetRfidTags() const
{
    std::vector<TRfidTagAssignment> Tags;

    try
    {
        std::unique_ptr<TFDConnection> Connection =
            CreateAccessConnection(FDatabasePath);

        std::unique_ptr<TFDQuery> Query(new TFDQuery(nullptr));
        Query->Connection = Connection.get();
        Query->SQL->Text =
            L"SELECT EmployeeID, IdentityKey, EmployeeName, Department, TagUid, "
            L"TagType, Active, CreatedAt "
            L"FROM RfidTagAssignment "
            L"ORDER BY EmployeeName COLLATE NOCASE";
        Query->Open();

        while (!Query->Eof)
        {
            TRfidTagAssignment Assignment;
            Assignment.EmployeeId =
                Query->FieldByName(L"EmployeeID")->AsInteger;
            Assignment.IdentityKey =
                SafeField(Query.get(), L"IdentityKey");
            Assignment.EmployeeName =
                SafeField(Query.get(), L"EmployeeName");
            Assignment.Department =
                SafeField(Query.get(), L"Department");
            Assignment.TagUid =
                SafeField(Query.get(), L"TagUid");
            Assignment.TagType =
                SafeField(Query.get(), L"TagType");
            Assignment.Active =
                Query->FieldByName(L"Active")->AsInteger != 0;
            Assignment.CreatedAt =
                SafeField(Query.get(), L"CreatedAt");

            Tags.push_back(Assignment);
            Query->Next();
        }
    }
    catch (const Exception& E)
    {
        OutputDebugStringW(
            (L"[AccessEventRepository] GetRfidTags ERROR: " +
             E.Message + L"\r\n").c_str());
        Tags.clear();
    }

    return Tags;
}
