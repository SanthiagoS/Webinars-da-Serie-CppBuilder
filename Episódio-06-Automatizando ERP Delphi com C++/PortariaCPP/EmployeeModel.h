//---------------------------------------------------------------------------

#ifndef EmployeeModelH
#define EmployeeModelH
//---------------------------------------------------------------------------
#include <System.hpp>

#include <vector>
//---------------------------------------------------------------------------

struct TEmployee
{
    int Id = 0;
    UnicodeString Name;
    UnicodeString Department;
    UnicodeString Email;
    UnicodeString Phone;
    UnicodeString Seniority;
    UnicodeString FaceKey;
    UnicodeString BleId;
    UnicodeString PhotoPath;
    std::vector<float> FaceEmbedding;
    bool HasFaceEmbedding = false;
};

//---------------------------------------------------------------------------
#endif

