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

void RibbonIMECore::GetCandidateList(std::vector<shared_wstring>& pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch)
{
    (void)pCandidateList;
    (void)isIncrementalWordSearch;
    (void)isWildcardSearch;
}
