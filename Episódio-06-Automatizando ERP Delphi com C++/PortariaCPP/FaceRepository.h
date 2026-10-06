//---------------------------------------------------------------------------

#ifndef FaceRepositoryH
#define FaceRepositoryH
//---------------------------------------------------------------------------
#include <System.hpp>

#include "FaceRecognizer.h"

#include <vector>
//---------------------------------------------------------------------------

struct FaceTemplate
{
    int EmployeeId = 0;
    UnicodeString FaceKey;
    FaceFeature Feature;
};

class FaceRepository
{
public:
    explicit FaceRepository(const UnicodeString& RootPath);

    bool SaveTemplate(
        int EmployeeId,
        const UnicodeString& FaceKey,
        const FaceFeature& Feature) const;

    bool LoadTemplate(
        int EmployeeId,
        UnicodeString& FaceKey,
        FaceFeature& Feature) const;
    bool HasTemplate(int EmployeeId) const;
    bool DeleteTemplate(int EmployeeId) const;
    std::vector<FaceTemplate> LoadAllTemplates() const;

    UnicodeString RootPath() const;

private:
    UnicodeString TemplatePath(int EmployeeId) const;
    bool EnsureRootPath() const;

    UnicodeString FRootPath;
};

//---------------------------------------------------------------------------
#endif

