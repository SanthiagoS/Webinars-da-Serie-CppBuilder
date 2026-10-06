//---------------------------------------------------------------------------

#ifndef EmployeeRepositoryH
#define EmployeeRepositoryH
//---------------------------------------------------------------------------
#include <System.hpp>

#include <FireDAC.Comp.Client.hpp>

#include <vector>
#include <mutex>

#include "EmployeeModel.h"
//---------------------------------------------------------------------------

class TEmployeeRepository
{
public:
    explicit TEmployeeRepository(const UnicodeString& DatabasePath);

    bool FindById(int Id, TEmployee& Employee) const;
    bool FindByBleId(const UnicodeString& DeviceId, TEmployee& Employee) const;
    std::vector<TEmployee> GetAll() const;

    // v0.6.3.1 - snapshot de funcionários recebido do ERP Delphi.
    void ReplaceRemoteSnapshot(const std::vector<TEmployee>& Employees);
    void ClearRemoteSnapshot();
    bool HasRemoteSnapshot() const;
    int RemoteEmployeeCount() const;

    UnicodeString DatabasePath() const;

private:
    bool PopulateEmployeeFromCurrentRow(TEmployee& Employee,
        Firedac::Comp::Client::TFDQuery* Query) const;

    UnicodeString FDatabasePath;

    mutable std::mutex FRemoteMutex;
    std::vector<TEmployee> FRemoteEmployees;
    bool FRemoteSnapshotReady = false;
};

//---------------------------------------------------------------------------
#endif

