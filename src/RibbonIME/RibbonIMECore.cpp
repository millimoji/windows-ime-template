#include "pch.h"

#include "RibbonIMECore.h"

RibbonIMECore::RibbonIMECore(const std::weak_ptr<WindowsImeLib::ICompositionProcessorEngineOwner>& owner)
    : m_owner(owner)
{
}

RibbonIMECore::~RibbonIMECore()
{
}

// Get language profile.
const GUID& RibbonIMECore::GetLanguageProfile(LANGID *plangId)
{
    // {A5A31980-289D-4B12-BE41-E975D2852629}
    *plangId = MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
    static const GUID profileGuid = { 0xa5a31980, 0x289d, 0x4b12, { 0xbe, 0x41, 0xe9, 0x75, 0xd2, 0x85, 0x26, 0x29 } };
    return profileGuid;
}

// Get locale
BOOL RibbonIMECore::SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode)
{
    (void)langid;
    (void)guidLanguageProfile;
    (void)pThreadMgr;
    (void)tfClientId;
    (void)isSecureMode;
    (void)isComLessMode;

    return TRUE;
}

LCID RibbonIMECore::GetLocale()
{
    return MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN), SORT_DEFAULT);
}

BOOL RibbonIMECore::IsKeyEaten(_In_ ITfThreadMgr* pThreadMgr, TfClientId tfClientId, UINT code, _Inout_updates_(1) WCHAR *pwch,
        BOOL isComposing, CANDIDATE_MODE candidateMode, BOOL isCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState)
{
    (void)pThreadMgr;
    (void)tfClientId;
    (void)code;
    (void)pwch;
    (void)isComposing;
    (void)candidateMode;
    (void)isCandidateWithWildcard;

    if (pKeyState) {
        *pKeyState = _KEYSTROKE_STATE();
    }
    return FALSE;
}

BOOL RibbonIMECore::AddVirtualKey(WCHAR wch)
{
    (void)wch;

    return TRUE;
}

void RibbonIMECore::RemoveVirtualKey(DWORD_PTR dwIndex)
{
    (void)dwIndex;
}

void RibbonIMECore::PurgeVirtualKey()
{
}

DWORD_PTR RibbonIMECore::GetVirtualKeyLength()
{
    return 0;
}

void RibbonIMECore::GetReadingStrings(_Inout_ std::vector<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded)
{
    (void)pReadingStrings;
    *pIsWildcardIncluded = FALSE;
}

void RibbonIMECore::GetCandidateList(_Inout_ std::vector<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch)
{
    (void)pCandidateList;
    (void)isIncrementalWordSearch;
    (void)isWildcardSearch;
}

void RibbonIMECore::GetCandidateStringInConverted(CStringRange &searchString, _In_ std::vector<CCandidateListItem> *pCandidateList)
{
    (void)searchString;
    (void)pCandidateList;
}

// Preserved key handler
void RibbonIMECore::OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    (void)rguid;
    (void)pThreadMgr;
    (void)tfClientId;
    *pIsEaten = FALSE;
}

// Punctuation
BOOL RibbonIMECore::IsPunctuation(WCHAR wch)
{
    (void)wch;
    return FALSE;
}

WCHAR RibbonIMECore::GetPunctuation(WCHAR wch)
{
    (void)wch;
    return 0;
}

BOOL RibbonIMECore::IsDoubleSingleByte(WCHAR wch)
{
    (void)wch;
    return FALSE;
}

BOOL RibbonIMECore::IsMakePhraseFromText()
{
    return FALSE;
}

// Language bar control
void RibbonIMECore::ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr)
{
    (void)pThreadMgr;
}

std::vector<DWORD>* RibbonIMECore::GetCandidateListIndexRange()
{
    return nullptr;
}

UINT RibbonIMECore::GetCandidateWindowWidth()
{
    return 0;
}

// Compartment
HRESULT RibbonIMECore::CompartmentCallback(REFGUID guidCompartment) noexcept
{
    return S_OK;
}

void RibbonIMECore::ClearCompartment(ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    (void)pThreadMgr;
    (void)tfClientId;
}

// GUIDs
const CLSID& RibbonIMEConstants::IMECLSID() noexcept
{
    // {FF089024-807B-4131-9CDF-3C3B067F081C}
    static const GUID imeClasId = { 0xff089024, 0x807b, 0x4131, { 0x9c, 0xdf, 0x3c, 0x3b, 0x6, 0x7f, 0x8, 0x1c } };
    return imeClasId;

}

const GUID& RibbonIMEConstants::IMEProfileGuid() noexcept
{
    // {0F86F935-F13C-4813-9E26-14888E3F128E}
    static const GUID imeProfileId = { 0xf86f935, 0xf13c, 0x4813, { 0x9e, 0x26, 0x14, 0x88, 0x8e, 0x3f, 0x12, 0x8e } };
    return imeProfileId;
}

const GUID& RibbonIMEConstants::DisplayAttributeInput() noexcept
{
    // {0437ED84-7948-4E19-B395-8A5E4316DEE3}
    static const GUID dispAttrInput = { 0x437ed84, 0x7948, 0x4e19, { 0xb3, 0x95, 0x8a, 0x5e, 0x43, 0x16, 0xde, 0xe3 } };
    return dispAttrInput;
}

const GUID& RibbonIMEConstants::DisplayAttributeConverted() noexcept
{
    // {9B18B8EA-BF09-486C-AD47-9C698789346C}
    static const GUID dispAttrCongverted = { 0x9b18b8ea, 0xbf09, 0x486c, { 0xad, 0x47, 0x9c, 0x69, 0x87, 0x89, 0x34, 0x6c } };
    return dispAttrCongverted;
}

const GUID& RibbonIMEConstants::CandUIElement() noexcept
{
    // {E1124AF5-A86E-4F39-8081-6FEEE3C90504}
    static const GUID candUiElementId = { 0xe1124af5, 0xa86e, 0x4f39, { 0x80, 0x81, 0x6f, 0xee, 0xe3, 0xc9, 0x5, 0x4 } };
    return candUiElementId;
}

const LANGID RibbonIMEConstants::GetLangID() noexcept
{
    return MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
}
