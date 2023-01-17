#include "pch.h"

#include "RibbonIMECore.h"

RibbonIMECore::RibbonIMECore(
    const std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer>& compositionBuffer,
    const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& candidateListView)
    : m_compositionBuffer(compositionBuffer), m_candidateListView(candidateListView)
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
