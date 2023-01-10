#include "pch.h"

#include "RibbonIMECore.h"

RibbonIMECore::RibbonIMECore(const std::weak_ptr<WindowsImeLib::ICompositionProcessorEngineOwner>& owner)
    : m_owner(owner)
{
}

RibbonIMECore::~RibbonIMECore()
{
}

// Get locale
BOOL RibbonIMECore::Initialize()
{
    return TRUE;
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

// // Preserved key handler
// void RibbonIMECore::OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
// {
//     (void)rguid;
//     (void)pThreadMgr;
//     (void)tfClientId;
//     *pIsEaten = FALSE;
// }

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

// // Language bar control
// void RibbonIMECore::ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr)
// {
//     (void)pThreadMgr;
// }

std::vector<DWORD>* RibbonIMECore::GetCandidateListIndexRange()
{
    return nullptr;
}

UINT RibbonIMECore::GetCandidateWindowWidth()
{
    return 0;
}

// // Compartment
// HRESULT RibbonIMECore::CompartmentCallback(REFGUID guidCompartment) noexcept
// {
//     return S_OK;
// }
// 
// void RibbonIMECore::ClearCompartment(ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
// {
//     (void)pThreadMgr;
//     (void)tfClientId;
// }


RibbonTextInputProcessor::RibbonTextInputProcessor(WindowsImeLib::ITextInputFramework* framework) :
	m_framework(framework)
{
}

RibbonTextInputProcessor::~RibbonTextInputProcessor()
{
}

std::wstring RibbonTextInputProcessor::TestMethod(const std::wstring& src)
{
    return src + L"-ribbon-suffix";
}

void RibbonTextInputProcessor::SetFocus(bool isGotten)
{
    WindowsImeLib::TraceLog("RibbonTextInputProcessor::::SetFocus:%d", isGotten ? 1 : 0);
}
