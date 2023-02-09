// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "../WindowsImeLib.h"
#include "SampleIMEBaseStructure.h"
#include "SampleIMEDefine.h"
#include "SampleIMEGlobals.h"

class SampleIMEInprocClient :
    public WindowsImeLib::IWindowsIMEInProcClient,
    public std::enable_shared_from_this<SampleIMEInprocClient>
{
public:
    SampleIMEInprocClient(WindowsImeLib::IWindowsIMEInProcFramework* framework) : m_framework(framework) {}
    ~SampleIMEInprocClient() {}

private:
    void Initialize(_In_ ITfThreadMgr *threadMgr, TfClientId tfClientId, BOOL isSecureMode) override
    {
        try
        {
            m_threadMgr = threadMgr;
            m_tfClientId = tfClientId;

            InitCompartments();
            InitLanguageBar(isSecureMode);
            InitPreservedKeys();
        }
        catch (...)
        {
            LOG_CAUGHT_EXCEPTION();
            Deinitialize();
            throw;
        }
    }

    void Deinitialize() override try
    {
        DeinitPreservedKeys();
        DeinitLanguageBar();
        DeinitCompartments();

        m_threadMgr.reset();
        m_tfClientId = TF_CLIENTID_NULL;
    }
    CATCH_LOG()

    void OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr* /*pThreadMgr*/, TfClientId /*tfClientId*/) override
    {
        *pIsEaten = FALSE;
        if (IsEqualGUID(rguid, Global::SampleIMEGuidImeModePreserveKey))
        {
            /* const auto preservedKeyData = FindPreservedKey(rguid); */
            if (true /*CheckShiftKeyOnly(preservedKeyData->tfPreservedKey.uModifiers)*/)
            {
                const auto isOpen = m_compartmentKeyboardOpenClose->GetCompartmentBOOL();
                THROW_IF_FAILED(m_compartmentKeyboardOpenClose->_SetCompartmentBOOL(isOpen ? FALSE : TRUE));
                *pIsEaten = TRUE;
            }
        }
        else if (IsEqualGUID(rguid, Global::SampleIMEGuidDoubleSingleBytePreserveKey))
        {
            const auto preservedKeyData = FindPreservedKey(rguid);
            if (CheckShiftKeyOnly(preservedKeyData->tfPreservedKey.uModifiers))
            {
                const auto isDouble = m_compartmentDoubleSingleByte->GetCompartmentBOOL();
                THROW_IF_FAILED(m_compartmentDoubleSingleByte->_SetCompartmentBOOL(isDouble ? FALSE : TRUE));
                *pIsEaten = TRUE;
            }
        }
        else if (IsEqualGUID(rguid, Global::SampleIMEGuidPunctuationPreserveKey))
        {
            const auto preservedKeyData = FindPreservedKey(rguid);
            if (CheckShiftKeyOnly(preservedKeyData->tfPreservedKey.uModifiers))
            {
                const auto isPunctuation = m_compartmentPunctuation->GetCompartmentBOOL();
                THROW_IF_FAILED(m_compartmentPunctuation->_SetCompartmentBOOL(isPunctuation ? FALSE : TRUE));
                *pIsEaten = TRUE;
            }
        }
    }

    void SetLanguageBarStatus(DWORD status, BOOL isSet) override
    {
        for (auto&& item: m_listLanguageBarItem)
        {
            if (item)
            {
                item->SetStatus(status, isSet);
            }
        }
    }

    void ConversionModeCompartmentUpdated() override
    {
        DWORD conversionMode = m_compartmentKeyboardInputModeConversion->GetCompartmentDWORD();

        const auto isOpen = m_compartmentKeyboardOpenClose->GetCompartmentBOOL();
        const auto isDouble = m_compartmentDoubleSingleByte->GetCompartmentBOOL();
        const auto isPunctuation = m_compartmentPunctuation->GetCompartmentBOOL();

        if (!isDouble && (conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
        {
            m_compartmentDoubleSingleByte->_SetCompartmentBOOL(TRUE);
        }
        else if (isDouble && !(conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
        {
            m_compartmentDoubleSingleByte->_SetCompartmentBOOL(FALSE);
        }

        if (!isPunctuation && (conversionMode & TF_CONVERSIONMODE_SYMBOL))
        {
            m_compartmentPunctuation->_SetCompartmentBOOL(TRUE);
        }
        else if (isPunctuation && !(conversionMode & TF_CONVERSIONMODE_SYMBOL))
        {
            m_compartmentPunctuation->_SetCompartmentBOOL(FALSE);
        }

        if (isOpen && !(conversionMode & TF_CONVERSIONMODE_NATIVE))
        {
            m_compartmentKeyboardOpenClose->_SetCompartmentBOOL(FALSE);
        }
        else
        {
            m_compartmentKeyboardOpenClose->_SetCompartmentBOOL(TRUE);
        }
    }

    std::string EncodeCustomState() override
    {
        nlohmann::json json;
        json["isOpen"] = !!m_compartmentKeyboardOpenClose->GetCompartmentBOOL();
        json["isDouble"] = !!m_compartmentDoubleSingleByte->GetCompartmentBOOL();
        json["isPunctuation"] = !!m_compartmentPunctuation->GetCompartmentBOOL();
        return json.dump();
    }

private:
    static const inline GUID s_compartmentGuids[5] = {
        GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
        GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
        GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE,
        Global::SampleIMEGuidCompartmentDoubleSingleByte,
        Global::SampleIMEGuidCompartmentPunctuation
    };

    void InitCompartments()
    {
        for (const auto& compartmentGuid: s_compartmentGuids)
        {
            const auto compartment = std::make_shared<CCompartment>(m_threadMgr.get(), m_tfClientId, compartmentGuid);
            wil::com_ptr<CCompartmentEventSink> eventCallback;
            eventCallback.attach(new CCompartmentEventSink(CompartmentCallback, this));
            THROW_IF_FAILED(eventCallback->_Advise(m_threadMgr.get(), compartmentGuid));

            m_listCompartment.emplace_back(compartment);
            m_listCompartmentEventSink.emplace_back(eventCallback);
        }

        // map to named variable
        m_compartmentKeyboardOpenClose = m_listCompartment[0];
        m_compartmentKeyboardInputModeConversion = m_listCompartment[1];
        m_compartmentKeyboardInputModeSentence = m_listCompartment[2];
        m_compartmentDoubleSingleByte = m_listCompartment[3];
        m_compartmentPunctuation = m_listCompartment[4];

        // set initial value
        m_compartmentKeyboardOpenClose->_SetCompartmentBOOL(TRUE);
        m_compartmentKeyboardInputModeConversion->_SetCompartmentDWORD(0);
        m_compartmentKeyboardInputModeSentence->_SetCompartmentDWORD(0);
        m_compartmentDoubleSingleByte->_SetCompartmentBOOL(FALSE);
        m_compartmentPunctuation->_SetCompartmentBOOL(TRUE);
    }

    void DeinitCompartments()
    {
        for (auto&& compartment: m_listCompartment)
        {
            LOG_IF_FAILED(compartment->_ClearCompartment());
        }
        m_listCompartment.clear();

        m_compartmentKeyboardOpenClose.reset();
        m_compartmentKeyboardInputModeConversion.reset();
        m_compartmentKeyboardInputModeSentence.reset();
        m_compartmentDoubleSingleByte.reset();
        m_compartmentPunctuation.reset();

        for (auto&& eventShink: m_listCompartmentEventSink)
        {
            LOG_IF_FAILED(eventShink->_Unadvise());
        }
        m_listCompartmentEventSink.clear();
    }

    static const inline struct LANGBARITEMDATA {
        GUID langBarItemGuid;
        GUID compartmentGuid;
        const wchar_t* description;
        const wchar_t* toolTip;
        int onIconResourceId;
        int offIconResourceId;
    } s_langBarItems[3] = {
        { GUID_LBI_INPUTMODE, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
            Global::LangbarImeModeDescription, Global::ImeModeDescription, Global::ImeModeOnIcoIndex, Global::ImeModeOffIcoIndex },
        { Global::SampleIMEGuidLangBarDoubleSingleByte, Global::SampleIMEGuidCompartmentDoubleSingleByte,
            Global::LangbarDoubleSingleByteDescription, Global::DoubleSingleByteDescription, Global::DoubleSingleByteOnIcoIndex, Global::DoubleSingleByteOffIcoIndex },
        { Global::SampleIMEGuidLangBarPunctuation, Global::SampleIMEGuidCompartmentPunctuation,
            Global::LangbarPunctuationDescription, Global::PunctuationDescription, Global::PunctuationOnIcoIndex, Global::PunctuationOffIcoIndex }
    };

    void InitLanguageBar(BOOL isSecureMode)
    {
        for (const auto& langBarItemData: s_langBarItems)
        {
            m_listLanguageBarItem.emplace_back(wil::com_ptr<CLangBarItemButton>());
            m_listLanguageBarItem.back().attach(new CLangBarItemButton(langBarItemData.langBarItemGuid,
                    langBarItemData.description, langBarItemData.toolTip, langBarItemData.onIconResourceId, langBarItemData.offIconResourceId, isSecureMode));
            THROW_IF_FAILED(m_listLanguageBarItem.back()->_AddItem(m_threadMgr.get()));
            THROW_HR_IF(E_FAIL, !m_listLanguageBarItem.back()->_RegisterCompartment(m_threadMgr.get(), m_tfClientId, langBarItemData.compartmentGuid));
        }
    }

    void DeinitLanguageBar()
    {
        for (auto&& langBarItem: m_listLanguageBarItem)
        {
            langBarItem->_UnregisterCompartment(m_threadMgr.get());
            langBarItem->CleanUp();
        }
        m_listLanguageBarItem.clear();
    }

    static const inline struct PRESERVEDKEYDATA {
        TF_PRESERVEDKEY tfPreservedKey;
        GUID preservedKeyGuid;
        const wchar_t* description;
    } s_preservedKeys[3] = {
        { TF_PRESERVEDKEY { VK_SHIFT,       _TF_MOD_ON_KEYUP_SHIFT_ONLY },  Global::SampleIMEGuidImeModePreserveKey,            Global::ImeModeDescription},
        { TF_PRESERVEDKEY { VK_SPACE,       TF_MOD_SHIFT },                 Global::SampleIMEGuidDoubleSingleBytePreserveKey,   Global::DoubleSingleByteDescription },
        { TF_PRESERVEDKEY { VK_OEM_PERIOD,  TF_MOD_CONTROL},                Global::SampleIMEGuidPunctuationPreserveKey,        Global::PunctuationDescription }
    };

    void InitPreservedKeys()
    {
        const auto tfKeystrokeMgr = m_threadMgr.query<ITfKeystrokeMgr>();
        for (const auto& preservedKey: s_preservedKeys)
        {
            TF_PRESERVEDKEY tfPreservedKey = {};
            tfPreservedKey.uVKey = preservedKey.tfPreservedKey.uVKey;
            tfPreservedKey.uModifiers = preservedKey.tfPreservedKey.uModifiers & 0xffff; // clear extended bits
            ULONG descriptionLength = static_cast<ULONG>(wcslen(preservedKey.description));
            THROW_IF_FAILED(tfKeystrokeMgr->PreserveKey(m_tfClientId, preservedKey.preservedKeyGuid, &tfPreservedKey, preservedKey.description, descriptionLength));
        }
    }

    void DeinitPreservedKeys()
    {
        const auto tfKeystrokeMgr = m_threadMgr.query<ITfKeystrokeMgr>();
        for (const auto& preservedKey: s_preservedKeys)
        {
            TF_PRESERVEDKEY tfPreservedKey = {};
            tfPreservedKey.uVKey = preservedKey.tfPreservedKey.uVKey;
            tfPreservedKey.uModifiers = preservedKey.tfPreservedKey.uModifiers & 0xffff; // clear extended bits
            THROW_IF_FAILED(tfKeystrokeMgr->UnpreserveKey(preservedKey.preservedKeyGuid, &tfPreservedKey));
        }
    }

    const PRESERVEDKEYDATA* FindPreservedKey(REFGUID guidPreservedKey)
    {
        for (const auto& preservedKey: s_preservedKeys)
        {
            if (preservedKey.preservedKeyGuid == guidPreservedKey)
            {
                return &preservedKey;
            }
        }
        return nullptr;
    }

    BOOL CheckShiftKeyOnly(UINT extendedModifer)
    {

        if (((extendedModifer & (_TF_MOD_ON_KEYUP_SHIFT_ONLY & 0xffff0000)) && !Global::IsShiftKeyDownOnly) ||
            ((extendedModifer & (_TF_MOD_ON_KEYUP_CONTROL_ONLY & 0xffff0000)) && !Global::IsControlKeyDownOnly) ||
            ((extendedModifer & (_TF_MOD_ON_KEYUP_ALT_ONLY & 0xffff0000)) && !Global::IsAltKeyDownOnly))
        {
            return FALSE;
        }
        return TRUE;
    }

    static HRESULT CompartmentCallback(void* pv, REFGUID guidCompartment)
    {
        SampleIMEInprocClient* _this = reinterpret_cast<SampleIMEInprocClient*>(pv);

        if (IsEqualGUID(guidCompartment, Global::SampleIMEGuidCompartmentDoubleSingleByte) ||
            IsEqualGUID(guidCompartment, Global::SampleIMEGuidCompartmentPunctuation))
        {
            _this->PrivateCompartmentsUpdated();
        }
        else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION) ||
            IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE))
        {
            _this->ConversionModeCompartmentUpdated();
        }
        else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
        {
            _this->KeyboardOpenCompartmentUpdated();
        }

        _this->UpdateCustomStateIfRequired();
        return S_OK;
    }

    void PrivateCompartmentsUpdated()
    {
        DWORD conversionMode = m_compartmentKeyboardInputModeConversion->GetCompartmentDWORD();
        DWORD conversionModePrev = conversionMode;

        BOOL isDouble = m_compartmentDoubleSingleByte->GetCompartmentBOOL();
        if (!isDouble && (conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
        {
            conversionMode &= ~TF_CONVERSIONMODE_FULLSHAPE;
        }
        else if (isDouble && !(conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
        {
            conversionMode |= TF_CONVERSIONMODE_FULLSHAPE;
        }

        BOOL isPunctuation = m_compartmentPunctuation->GetCompartmentBOOL();
        if (!isPunctuation && (conversionMode & TF_CONVERSIONMODE_SYMBOL))
        {
            conversionMode &= ~TF_CONVERSIONMODE_SYMBOL;
        }
        else if (isPunctuation && !(conversionMode & TF_CONVERSIONMODE_SYMBOL))
        {
            conversionMode |= TF_CONVERSIONMODE_SYMBOL;
        }

        if (conversionMode != conversionModePrev)
        {
            m_compartmentKeyboardInputModeConversion->_SetCompartmentDWORD(conversionMode);
        }
    }

    void KeyboardOpenCompartmentUpdated()
    {
        DWORD conversionMode = m_compartmentKeyboardInputModeConversion->GetCompartmentDWORD();
        DWORD conversionModePrev = conversionMode;

        const auto isOpen = m_compartmentKeyboardOpenClose->GetCompartmentBOOL();

        if (isOpen && !(conversionMode & TF_CONVERSIONMODE_NATIVE))
        {
            conversionMode |= TF_CONVERSIONMODE_NATIVE;
        }
        else if (!isOpen && (conversionMode & TF_CONVERSIONMODE_NATIVE))
        {
            conversionMode &= ~TF_CONVERSIONMODE_NATIVE;
        }

        if (conversionMode != conversionModePrev)
        {
            m_compartmentKeyboardInputModeConversion->_SetCompartmentDWORD(conversionMode);
        }
    }

    void UpdateCustomStateIfRequired()
    {
        bool isOpen = !!m_compartmentKeyboardOpenClose->GetCompartmentBOOL();
        bool isDouble = !!m_compartmentDoubleSingleByte->GetCompartmentBOOL();
        bool isPunctuation = !!m_compartmentPunctuation->GetCompartmentBOOL();

        if (m_lastCompartmentIsOpen != isOpen ||
            m_lastCompartmentIsDoubleSingleByte != isDouble ||
            m_lastCompartmentIsPunctuation != isPunctuation)
        {
            m_framework->UpdateCustomState();

            m_lastCompartmentIsOpen = isOpen;
            m_lastCompartmentIsDoubleSingleByte = isDouble;
            m_lastCompartmentIsPunctuation = isPunctuation;
        }
    }

private:
    WindowsImeLib::IWindowsIMEInProcFramework* m_framework;
    wil::com_ptr<ITfThreadMgr> m_threadMgr;
    TfClientId m_tfClientId = TF_CLIENTID_NULL;

    bool m_lastCompartmentIsOpen = false;
    bool m_lastCompartmentIsDoubleSingleByte = false;
    bool m_lastCompartmentIsPunctuation = false;

    std::shared_ptr<CCompartment> m_compartmentKeyboardOpenClose;
    std::shared_ptr<CCompartment> m_compartmentKeyboardInputModeConversion;
    std::shared_ptr<CCompartment> m_compartmentKeyboardInputModeSentence;
    std::shared_ptr<CCompartment> m_compartmentDoubleSingleByte;
    std::shared_ptr<CCompartment> m_compartmentPunctuation;
    std::vector<std::shared_ptr<CCompartment>> m_listCompartment;
    std::vector<wil::com_ptr<CCompartmentEventSink>> m_listCompartmentEventSink;

    std::vector<wil::com_ptr<CLangBarItemButton>> m_listLanguageBarItem;
};

namespace WindowsImeLib
{

std::shared_ptr<IWindowsIMEInProcClient> ProcessorFactory_CreateIMEInprocClient(IWindowsIMEInProcFramework* framework)
{
    auto inprocClient = std::make_shared<SampleIMEInprocClient>(framework);
    return std::static_pointer_cast<IWindowsIMEInProcClient>(inprocClient);
}

}
