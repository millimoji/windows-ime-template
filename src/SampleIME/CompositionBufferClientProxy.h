// Copyright (c) millimoji@gmail.com
#pragma once

class CompositionBufferClientProxy
{
public:
    CompositionBufferClientProxy(const std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer>& compositionBuffer) :
        m_compositionBuffer(compositionBuffer)
    {}

    HRESULT _StartComposition()
    {
        m_isComposing = true;
        return S_OK;
    }

    HRESULT _TerminateComposition()
    {
        m_determined = m_composition;
        m_composition.reset();
        m_isComposing = false;
        return S_OK;
    }

    HRESULT _AddComposingAndChar(const shared_wstring& pstrAddString)
    {
        m_composition = pstrAddString;
        return S_OK;
    }

    HRESULT _AddCharAndFinalize(const shared_wstring& pstrAddString)
    {
        m_determined = pstrAddString;
        return S_OK;
    }

    HRESULT _RemoveDummyCompositionForComposing()
    {
        m_composition.reset();
        return S_OK;
    }

    bool _IsComposing() { return m_isComposing; }

    void dumpToJson(nlohmann::json& json)
    {
        nlohmann::json composition;
        composition[c_jsonKeyDetermined] = (m_determined ? *m_determined : std::wstring(L""));
        composition[c_jsonKeyText] = (m_composition ? *m_composition : std::wstring(L""));
        composition[c_jsonKeyIP] = (m_composition ? static_cast<int>(m_composition->length()) : 0);

        nlohmann::json attrEntries = nlohmann::json::array();

        if (m_composition) {
            nlohmann::json attrEntry;
            attrEntry[c_jsonKeyStart] = 0;
            attrEntry[c_jsonKeyEnd] = (m_composition ? static_cast<int>(m_composition->length()) : 0);
            attrEntry[c_jsonKeyLangId] = static_cast<int>(WindowsImeLib::g_processorFactory->GetConstantProvider()->GetLangID());
            attrEntry[c_jsonKeyAttrId] = 0; // index 0

            attrEntries.emplace_back(attrEntry);
        }

        composition[c_jsonKeyAttribute] = attrEntries;

        json[c_jsonKeyComposition] = composition;

        m_determined.reset();
    }

private:
    std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer> m_compositionBuffer;
    shared_wstring m_determined;
    shared_wstring m_composition;
    bool m_isComposing = false;
};
