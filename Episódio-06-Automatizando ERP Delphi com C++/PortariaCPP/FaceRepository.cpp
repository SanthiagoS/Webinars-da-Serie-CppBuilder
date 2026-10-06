//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "FaceRepository.h"

#include <System.IOUtils.hpp>
#include <System.SysUtils.hpp>

#include <fstream>
#include <string>
//---------------------------------------------------------------------------
#pragma package(smart_init)

namespace
{
    const unsigned int FaceTemplateMagic = 0x31464145; // EAF1
    const unsigned int FaceTemplateVersionLegacy = 1;
    const unsigned int FaceTemplateVersionIdentityBound = 2;
    const UnicodeString FaceTemplatePrefix = L"employee_";

    std::string ToUtf8Path(const UnicodeString& Path)
    {
        return UTF8String(Path).c_str();
    }
}

FaceRepository::FaceRepository(const UnicodeString& RootPath)
    : FRootPath(RootPath)
{
}

bool FaceRepository::SaveTemplate(
    int EmployeeId,
    const UnicodeString& FaceKey,
    const FaceFeature& Feature) const
{
    if (EmployeeId <= 0 ||
        Trim(FaceKey).IsEmpty() ||
        Feature.Empty() ||
        !EnsureRootPath())
    {
        return false;
    }

    std::ofstream Stream(
        ToUtf8Path(TemplatePath(EmployeeId)),
        std::ios::binary | std::ios::trunc);

    if (!Stream)
        return false;

    const unsigned int Magic = FaceTemplateMagic;
    const unsigned int Version =
        FaceTemplateVersionIdentityBound;

    const UTF8String FaceKeyUtf8 =
        UTF8String(Trim(FaceKey));

    const int FaceKeySize =
        FaceKeyUtf8.Length();

    const int FeatureSize =
        static_cast<int>(Feature.Values.size());

    Stream.write(reinterpret_cast<const char*>(&Magic), sizeof(Magic));
    Stream.write(reinterpret_cast<const char*>(&Version), sizeof(Version));
    Stream.write(reinterpret_cast<const char*>(&EmployeeId), sizeof(EmployeeId));
    Stream.write(reinterpret_cast<const char*>(&FaceKeySize), sizeof(FaceKeySize));
    Stream.write(reinterpret_cast<const char*>(&FeatureSize), sizeof(FeatureSize));
    Stream.write(FaceKeyUtf8.c_str(), FaceKeySize);
    Stream.write(
        reinterpret_cast<const char*>(Feature.Values.data()),
        sizeof(float) * Feature.Values.size());

    return Stream.good();
}

bool FaceRepository::LoadTemplate(
    int EmployeeId,
    UnicodeString& FaceKey,
    FaceFeature& Feature) const
{
    FaceKey = L"";
    Feature.Values.clear();

    if (EmployeeId <= 0)
        return false;

    std::ifstream Stream(
        ToUtf8Path(TemplatePath(EmployeeId)),
        std::ios::binary);

    if (!Stream)
        return false;

    unsigned int Magic = 0;
    unsigned int Version = 0;
    int StoredEmployeeId = 0;

    Stream.read(reinterpret_cast<char*>(&Magic), sizeof(Magic));
    Stream.read(reinterpret_cast<char*>(&Version), sizeof(Version));
    Stream.read(reinterpret_cast<char*>(&StoredEmployeeId), sizeof(StoredEmployeeId));

    if (!Stream ||
        Magic != FaceTemplateMagic ||
        StoredEmployeeId != EmployeeId)
    {
        return false;
    }

    if (Version == FaceTemplateVersionLegacy)
    {
        int FeatureSize = 0;
        Stream.read(reinterpret_cast<char*>(&FeatureSize), sizeof(FeatureSize));

        if (!Stream || FeatureSize <= 0 || FeatureSize > 4096)
            return false;

        Feature.Values.resize(FeatureSize);
        Stream.read(
            reinterpret_cast<char*>(Feature.Values.data()),
            sizeof(float) * Feature.Values.size());

        if (!Stream)
        {
            Feature.Values.clear();
            return false;
        }

        // Legado: não existe chave de identidade, portanto será ignorado
        // pelo reconhecimento seguro.
        FaceKey = L"";
        return true;
    }

    if (Version != FaceTemplateVersionIdentityBound)
        return false;

    int FaceKeySize = 0;
    int FeatureSize = 0;

    Stream.read(reinterpret_cast<char*>(&FaceKeySize), sizeof(FaceKeySize));
    Stream.read(reinterpret_cast<char*>(&FeatureSize), sizeof(FeatureSize));

    if (!Stream ||
        FaceKeySize <= 0 || FaceKeySize > 512 ||
        FeatureSize <= 0 || FeatureSize > 4096)
    {
        return false;
    }

    std::string FaceKeyBytes(static_cast<size_t>(FaceKeySize), '\0');
    Stream.read(&FaceKeyBytes[0], FaceKeySize);

    if (!Stream)
        return false;

    const UTF8String FaceKeyUtf8 = FaceKeyBytes.c_str();
    FaceKey = UnicodeString(FaceKeyUtf8);

    Feature.Values.resize(FeatureSize);
    Stream.read(
        reinterpret_cast<char*>(Feature.Values.data()),
        sizeof(float) * Feature.Values.size());

    if (!Stream)
    {
        FaceKey = L"";
        Feature.Values.clear();
        return false;
    }

    return !Trim(FaceKey).IsEmpty();
}

bool FaceRepository::HasTemplate(int EmployeeId) const
{
    if (EmployeeId <= 0)
        return false;

    return TFile::Exists(TemplatePath(EmployeeId));
}

bool FaceRepository::DeleteTemplate(int EmployeeId) const
{
    if (EmployeeId <= 0)
        return false;

    const UnicodeString Path = TemplatePath(EmployeeId);

    if (!TFile::Exists(Path))
        return true;

    try
    {
        TFile::Delete(Path);
        return !TFile::Exists(Path);
    }
    catch (...)
    {
        return false;
    }
}

std::vector<FaceTemplate> FaceRepository::LoadAllTemplates() const
{
    std::vector<FaceTemplate> Templates;

    if (!TDirectory::Exists(FRootPath))
        return Templates;

    const TStringDynArray Files =
        TDirectory::GetFiles(FRootPath, L"employee_*.dat");

    for (int Index = 0; Index < Files.Length; ++Index)
    {
        const UnicodeString FileName =
            TPath::GetFileNameWithoutExtension(Files[Index]);

        UnicodeString EmployeeIdText =
            StringReplace(
                FileName,
                FaceTemplatePrefix,
                L"",
                TReplaceFlags() << rfIgnoreCase);

        const int EmployeeId = StrToIntDef(EmployeeIdText, 0);

        if (EmployeeId <= 0)
            continue;

        FaceTemplate Template;
        Template.EmployeeId = EmployeeId;

        if (LoadTemplate(
                EmployeeId,
                Template.FaceKey,
                Template.Feature))
        {
            Templates.push_back(Template);
        }
    }

    return Templates;
}

UnicodeString FaceRepository::RootPath() const
{
    return FRootPath;
}

UnicodeString FaceRepository::TemplatePath(int EmployeeId) const
{
    return TPath::Combine(
        FRootPath,
        FaceTemplatePrefix + IntToStr(EmployeeId) + L".dat");
}

bool FaceRepository::EnsureRootPath() const
{
    if (TDirectory::Exists(FRootPath))
        return true;

    TDirectory::CreateDirectory(FRootPath);
    return TDirectory::Exists(FRootPath);
}
//---------------------------------------------------------------------------

