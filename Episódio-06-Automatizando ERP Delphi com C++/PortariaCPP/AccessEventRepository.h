//---------------------------------------------------------------------------
#ifndef AccessEventRepositoryH
#define AccessEventRepositoryH
//---------------------------------------------------------------------------
#include <System.hpp>

#include <vector>
//---------------------------------------------------------------------------

struct TAccessEvent
{
    int Id = 0;
    int EmployeeId = 0;
    UnicodeString EmployeeName;
    UnicodeString Department;
    UnicodeString EventType;
    UnicodeString IdentificationMethod;
    UnicodeString EventDateTime;
    UnicodeString Terminal;
    UnicodeString Status;
    double RecognitionScore = 0.0;
};

struct TOccurrence
{
    int Id = 0;
    UnicodeString OccurrenceType;
    UnicodeString Description;
    UnicodeString EmployeeName;
    UnicodeString EventDateTime;
    UnicodeString Terminal;
    UnicodeString Status;
};

struct TRfidTagAssignment
{
    int EmployeeId = 0;

    // Chave imutável do funcionário.
    // Atualmente é alimentada pelo FaceKey sincronizado do ERP Delphi.
    UnicodeString IdentityKey;

    UnicodeString EmployeeName;
    UnicodeString Department;
    UnicodeString TagUid;
    UnicodeString TagType;
    bool Active = false;
    UnicodeString CreatedAt;
};

class TAccessEventRepository
{
public:
    explicit TAccessEventRepository(const UnicodeString& DatabasePath);

    bool Initialize() const;
    bool Add(const TAccessEvent& Event) const;
    std::vector<TAccessEvent> GetRecent(int Limit = 20) const;
    std::vector<TAccessEvent> GetPresentToday() const;
    bool AddOccurrence(const TOccurrence& Occurrence) const;
    std::vector<TOccurrence> GetRecentOccurrences(int Limit = 30) const;

    bool SetEmployeeBlocked(int EmployeeId,
        const UnicodeString& EmployeeName,
        bool Blocked,
        const UnicodeString& Reason,
        const UnicodeString& Terminal) const;

    bool IsEmployeeBlocked(int EmployeeId,
        UnicodeString& Reason) const;

    bool AssignRfidTag(int EmployeeId,
        const UnicodeString& IdentityKey,
        const UnicodeString& EmployeeName,
        const UnicodeString& Department,
        const UnicodeString& TagUid,
        const UnicodeString& TagType) const;

    bool RemoveRfidTag(int EmployeeId) const;

    bool GetRfidTagByEmployee(int EmployeeId,
        TRfidTagAssignment& Assignment) const;

    bool GetRfidTagByUid(const UnicodeString& TagUid,
        TRfidTagAssignment& Assignment) const;

    std::vector<TRfidTagAssignment> GetRfidTags() const;

private:
    UnicodeString FDatabasePath;
};

//---------------------------------------------------------------------------
#endif
