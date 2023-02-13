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

void RibbonIMECore::OnKeyEvent(WPARAM wParam, LPARAM lParam, BOOL *pIsEaten,
        wchar_t wch, UINT vkPackSource, bool isKbdDisabled, DWORD modifiers, bool isTest, bool isDown)
{
    *pIsEaten = FALSE;
    if (!m_isImeOpen || !isDown) {
        return;
    }

    std::wstring determined;

    if (m_composition.length() > 0) {
        if (vkPackSource == VK_RETURN) {
            determined = std::move(m_composition);
            *pIsEaten = TRUE;
        }
    }

    if (! *pIsEaten) {
        if (iswprint(wch)) {
            m_composition += wch;
            *pIsEaten = TRUE;
        } else {
            if (m_composition.length() > 0) {
                *pIsEaten = TRUE;
            }
        }
    }

    {
        nlohmann::json composition;
        composition[c_jsonKeyDetermined] = determined;
        composition[c_jsonKeyText] = m_composition;

        nlohmann::json attrEntries = nlohmann::json::array();
        if (m_composition.length() > 0) {
            nlohmann::json attr;
            attr[c_jsonKeyStart] = 0;
            attr[c_jsonKeyEnd] = static_cast<int>(m_composition.length());
            attr[c_jsonKeyLangId] = static_cast<int>(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN));
            attr[c_jsonKeyAttrId] = 0;
            attrEntries.emplace_back(attr);
        }

        composition[c_jsonKeyAttribute] = attrEntries;
        composition[c_jsonKeyIP] = static_cast<int>(m_composition.length());

        nlohmann::json jsonRoot;
        jsonRoot[c_jsonKeyComposition] = composition;

        m_compositionBuffer->SetCompositionState(jsonRoot.dump());
    }

}
