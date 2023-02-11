// 
#include "pch.h"

#include "RibbonIMECore.h"
#include "RibbonIMEInProcClient.h"

RibbonIMECore::RibbonIMECore(
    const std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer>& compositionBuffer,
    const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& candidateListView)
    : m_compositionBuffer(compositionBuffer), m_candidateListView(candidateListView)
{
}

RibbonIMECore::~RibbonIMECore()
{
}

void RibbonIMECore::UpdateCustomState(const std::string_view customStateJson)
{
    const auto json = nlohmann::json::parse(customStateJson);
    m_isImeOpen = json[c_imeOpen].get<bool>();
    m_isSecure = json[c_imeSecure].get<bool>();
    m_isStoreApp = json[c_imeStoreApp].get<bool>();
    m_isConsole = json[c_imeConsole].get<bool>();
}

void RibbonIMECore::GetCandidateList(std::vector<shared_wstring>& pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch)
{
    (void)pCandidateList;
    (void)isIncrementalWordSearch;
    (void)isWildcardSearch;
}



