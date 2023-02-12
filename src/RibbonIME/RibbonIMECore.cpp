// Copyright (c) millimoji@gmail.com
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
    const auto customData = json[c_jsonKeyCustomData];
    m_isImeOpen = customData[c_imeOpen].get<bool>();
    m_isSecure = customData[c_imeSecure].get<bool>();
    m_isStoreApp = customData[c_imeStoreApp].get<bool>();
    m_isConsole = customData[c_imeConsole].get<bool>();
}

void RibbonIMECore::GetCandidateList(std::vector<shared_wstring>& pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch)
{
    (void)pCandidateList;
    (void)isIncrementalWordSearch;
    (void)isWildcardSearch;
}



