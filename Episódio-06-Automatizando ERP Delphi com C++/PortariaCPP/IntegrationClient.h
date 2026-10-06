//---------------------------------------------------------------------------

#ifndef IntegrationClientH
#define IntegrationClientH
//---------------------------------------------------------------------------
#include <System.hpp>

#include "EmployeeModel.h"
//---------------------------------------------------------------------------

class TIntegrationClient
{
public:
    explicit TIntegrationClient(const UnicodeString& ApiUrl);

    bool SendEmployeeArrival(const TEmployee& Employee, const UnicodeString& Terminal,
        bool FaceConfirmed, bool BleConfirmed);

    UnicodeString ApiUrl() const;
    UnicodeString LastPayload() const;

private:
    UnicodeString EscapeJson(const UnicodeString& Value) const;

    UnicodeString FApiUrl;
    UnicodeString FLastPayload;
};

//---------------------------------------------------------------------------
#endif
