#include "pch.h"

#include "RibbonIMECore.h"

RibbonIMECore::RibbonIMECore(WindowsImeLib::ICompositionProcessorEngineOwner* owner)
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

void RibbonIMECore::PurgeVirtualKey()
{
}

void RibbonIMECore::GetCandidateList(_Inout_ std::vector<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch)
{
    (void)pCandidateList;
    (void)isIncrementalWordSearch;
    (void)isWildcardSearch;
}

// // Preserved key handler
// void RibbonIMECore::OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
// {
//     (void)rguid;
//     (void)pThreadMgr;
//     (void)tfClientId;
//     *pIsEaten = FALSE;
// }

// // Language bar control
// void RibbonIMECore::ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr)
// {
//     (void)pThreadMgr;
// }

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
