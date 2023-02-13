// Copyright (c) millimoji@gmail.com

#include "pch.h"
#include "resource.h"
#include "../WindowsImeLib.h"
#include "../Compartment.h"
#include "../LanguageBar.h"
#include "../DisplayAttributeInfo.h"
#include "RibbonIMEInProcClient.h"

inline HMODULE GetCurrentModuleHandle() {
    static HMODULE currentModule = ([]() {
        HMODULE moduleHandle = {};
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          reinterpret_cast<LPCWSTR>(GetCurrentModuleHandle), &moduleHandle);
        return moduleHandle;
    })();
    return currentModule;
}

inline shared_wstring GetStringFromResource(int resId) {
    const wchar_t* textInResource = {};
    auto textLength = LoadString(GetCurrentModuleHandle(), resId, reinterpret_cast<LPWSTR>(&textInResource), 0);
    return std::make_shared<std::wstring>(textInResource, textLength);
}

#define TF_MOD_ALLALT     (TF_MOD_RALT | TF_MOD_LALT | TF_MOD_ALT)
#define TF_MOD_ALLCONTROL (TF_MOD_RCONTROL | TF_MOD_LCONTROL | TF_MOD_CONTROL)
#define TF_MOD_ALLSHIFT   (TF_MOD_RSHIFT | TF_MOD_LSHIFT | TF_MOD_SHIFT)

class RibbonIMEInProcClient :
    public WindowsImeLib::IWindowsIMEInProcClient,
    public std::enable_shared_from_this<RibbonIMEInProcClient>
{
public:
    RibbonIMEInProcClient(WindowsImeLib::IWindowsIMEInProcFramework* framework) : m_framework(framework) {}
    ~RibbonIMEInProcClient() {}

private:
    void Initialize(_In_ ITfThreadMgr* threadMgr, TfClientId tfClientId, _In_ ITfCategoryMgr* categoryMgr) override try {
        m_threadMgr = threadMgr;
        m_tfClientId = tfClientId;

        BOOL isSecureMode = FALSE;
        {   wil::com_ptr<ITfThreadMgrEx> threadMgrEx;
            if (SUCCEEDED_LOG(threadMgr->QueryInterface(IID_PPV_ARGS(&threadMgrEx)))) {
                if (SUCCEEDED_LOG(threadMgrEx->GetActiveFlags(&m_tfTmfActiveFlags))) {
                    isSecureMode = (m_tfTmfActiveFlags & TF_TMF_SECUREMODE) ? TRUE : FALSE;
                }
            }
        }

        InitializeCompartments();
        InitializeLanguageBar(isSecureMode);
        InitializePreservedKeys();
        InitializeDisplayAttributeInfo(categoryMgr);
    }
    CATCH_LOG()

    void Uninitialize() override {
        UninitializeDisplayAttributeInfo();
        UninitializePreservedKeys();
        UninitializeLanguageBar();
        UnitializeCompartments();

        m_threadMgr.reset();
        m_tfClientId = TF_CLIENTID_NULL;
        m_tfTmfActiveFlags = 0;
    }

    std::string EncodeCustomState() override {
        nlohmann::json customData;
        customData[c_imeOpen] = !!m_compartmentKeyboardOpenClose->GetCompartmentBOOL();
        customData[c_imeSecure] = (m_tfTmfActiveFlags & TF_TMF_SECUREMODE) ? true : false;
        customData[c_imeStoreApp] = (m_tfTmfActiveFlags & TF_TMF_IMMERSIVEMODE) ? true : false;
        customData[c_imeConsole] = (m_tfTmfActiveFlags & TF_TMF_CONSOLE) ? true : false;
        nlohmann::json json;
        json[c_jsonKeyCustomData] = customData;
        return json.dump();
    }

    // TODO: re-consider parameter
    // wch: converted character from VK and keyboard state
    // vkPackSource: estimated VK from wch for VK_PACKET
    void OnKeyEvent(WPARAM wParam, LPARAM lParam, BOOL *pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled, DWORD modifiers, bool isTest, bool isDown) override
    {
        if (!isDown) { return; }

        // copy modifier state
        bool isShiftKeyDownOnly   = !!(modifiers & TF_MOD_ALLSHIFT)   && !(modifiers & ~TF_MOD_ALLSHIFT);
        bool isControlKeyDownOnly = !!(modifiers & TF_MOD_ALLCONTROL) && !(modifiers & ~TF_MOD_ALLCONTROL);
        bool isAltKeyDownOnly     = !!(modifiers & TF_MOD_ALLALT)     && !(modifiers & ~TF_MOD_ALLALT);
        DWORD uniqueModifiers = (isShiftKeyDownOnly ? TF_MOD_SHIFT : 0) |
                                (isControlKeyDownOnly ? TF_MOD_CONTROL : 0) |
                                (isAltKeyDownOnly ? TF_MOD_ALT : 0);

        if (vkPackSource == VK_KANJI && !!(uniqueModifiers & TF_MOD_ALT)) {
            const auto isOpen = m_compartmentKeyboardOpenClose->GetCompartmentBOOL();
            m_compartmentKeyboardOpenClose->_SetCompartmentBOOL(isOpen ? FALSE : TRUE);
            // *pIsEaten = TRUE;
        }
    }

    // {864AAE3D-D116-4A20-9B16-81B7F4EFE124}
    static inline const GUID c_imeModeVkKanjiGuid = { 0x864aae3d, 0xd116, 0x4a20, { 0x9b, 0x16, 0x81, 0xb7, 0xf4, 0xef, 0xe1, 0x24 } };
    // {4789C3ED-35A5-45AE-A726-EC4AA2A12D4F}
    static inline const GUID c_imeModeAltTildaGuid = { 0x4789c3ed, 0x35a5, 0x45ae, { 0xa7, 0x26, 0xec, 0x4a, 0xa2, 0xa1, 0x2d, 0x4f } };

    void OnPreservedKey(REFGUID rguid, _Out_ BOOL* pIsEaten, _In_ ITfThreadMgr* /*pThreadMgr*/, TfClientId /*tfClientId*/) override {
        *pIsEaten = FALSE;
#if 0 // TODO: investigate how to use preserved key for IME On/Off
        if (IsEqualGUID(rguid, c_imeModeVkKanjiGuid) ||
            IsEqualGUID(rguid, c_imeModeAltTildaGuid)) {
        }
#endif
    }

    void SetLanguageBarStatus(DWORD status, BOOL isSet) override {
        (void)status; (void)isSet;
    }
    void ConversionModeCompartmentUpdated() override {
    }
    std::shared_ptr<std::vector<std::pair<TfGuidAtom, wil::com_ptr<ITfDisplayAttributeInfo>>>> GetDisplayAttributeInfoList() override {
        return m_listDisplayAttributeInfo;
    }

private:
    static inline const GUID c_compartmentGuids[] = {
        GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
        GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
        GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE,
    };

    void InitializeCompartments() {
        for (const auto& compartmentGuid: c_compartmentGuids) {
            const auto compartment = std::make_shared<CCompartment>(m_threadMgr.get(), m_tfClientId, compartmentGuid);
            wil::com_ptr<CCompartmentEventSink> eventCallback;
            THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CCompartmentEventSink>(&eventCallback, CompartmentCallback, this));
            THROW_IF_FAILED(eventCallback->_Advise(m_threadMgr.get(), compartmentGuid));
            m_listCompartment.emplace_back(compartment);
            m_listCompartmentEventSink.emplace_back(eventCallback);
        }

        // map to named variable
        m_compartmentKeyboardOpenClose = m_listCompartment[0];
        m_compartmentKeyboardInputModeConversion = m_listCompartment[1];
        m_compartmentKeyboardInputModeSentence = m_listCompartment[2];
    }
    void UnitializeCompartments() {
        for (auto&& compartment: m_listCompartment) {
            LOG_IF_FAILED(compartment->_ClearCompartment());
        }
        m_listCompartment.clear();

        for (auto&& compartmentEventSink: m_listCompartmentEventSink) {
            LOG_IF_FAILED(compartmentEventSink->_Unadvise());
        }
        m_listCompartmentEventSink.clear();

        m_compartmentKeyboardOpenClose.reset();
        m_compartmentKeyboardInputModeConversion.reset();
        m_compartmentKeyboardInputModeSentence.reset();
    }

    static inline const struct LANGBARITEMDATA {
        GUID langBarItemGuid;
        GUID compartmentGuid;
        int descriptionResId;
        int toolTipResId;
        int onIconResourceId;
        int offIconResourceId;
    } c_langBarItems[] = {
        { GUID_LBI_INPUTMODE, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, IDS_IME_MODE, IDS_IME_MODE, IDI_IME_MODE_ON, IDI_IME_MODE_OFF },
    };

    void InitializeLanguageBar(BOOL isSecureMode) {
        for (const auto& langBarItemData: c_langBarItems) {
            wil::com_ptr<CLangBarItemButton> langBarItemButton;
            THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CLangBarItemButton>(&langBarItemButton, langBarItemData.langBarItemGuid,
                GetStringFromResource(langBarItemData.descriptionResId)->c_str(), GetStringFromResource(langBarItemData.toolTipResId)->c_str(),
                langBarItemData.onIconResourceId, langBarItemData.offIconResourceId, isSecureMode));

            m_listLanguageBarItem.emplace_back(langBarItemButton);
            THROW_IF_FAILED(m_listLanguageBarItem.back()->_AddItem(m_threadMgr.get()));
            THROW_HR_IF(E_FAIL, !m_listLanguageBarItem.back()->_RegisterCompartment(m_threadMgr.get(), m_tfClientId, langBarItemData.compartmentGuid));
        }
    }
    void UninitializeLanguageBar() {
        for (auto&& langBarItem: m_listLanguageBarItem) {
            langBarItem->_UnregisterCompartment(m_threadMgr.get());
            langBarItem->CleanUp();
        }
        m_listLanguageBarItem.clear();
    }

    static inline const struct PRESERVEDKEYDATA {
        TF_PRESERVEDKEY tfPreservedKey;
        GUID preservedKeyGuid;
        int descriptionResId;
    } c_preservedKeys[] = {
        { TF_PRESERVEDKEY { VK_KANJI, 0             }, c_imeModeVkKanjiGuid, IDS_IME_MODE },
        { TF_PRESERVEDKEY { VK_KANJI, TF_MOD_ALT    }, c_imeModeVkKanjiGuid, IDS_IME_MODE },
#if 0 // TODO: investigate how to set preserved key to IME On/Off
        { TF_PRESERVEDKEY { VK_OEM_3, TF_MOD_ALT    }, c_imeModeAltTildaGuid, IDS_IME_MODE },
        { TF_PRESERVEDKEY { VK_IME_ON, 0    }, c_imeModeAltTildaGuid, IDS_IME_MODE },
        { TF_PRESERVEDKEY { VK_IME_ON, TF_MOD_ALT    }, c_imeModeAltTildaGuid, IDS_IME_MODE },
        { TF_PRESERVEDKEY { VK_IME_OFF, 0    }, c_imeModeAltTildaGuid, IDS_IME_MODE },
        { TF_PRESERVEDKEY { VK_IME_OFF, TF_MOD_ALT    }, c_imeModeAltTildaGuid, IDS_IME_MODE },
#endif
    };

    void InitializePreservedKeys() {
        const auto tfKeystrokeMgr = m_threadMgr.query<ITfKeystrokeMgr>();
        for (const auto& preservedKey: c_preservedKeys) {
            TF_PRESERVEDKEY tfPreservedKey = {};
            tfPreservedKey.uVKey = preservedKey.tfPreservedKey.uVKey;
            tfPreservedKey.uModifiers = preservedKey.tfPreservedKey.uModifiers & 0xffff; // clear extended bits
            const auto description = GetStringFromResource(preservedKey.descriptionResId);
            THROW_IF_FAILED(tfKeystrokeMgr->PreserveKey(m_tfClientId,
                preservedKey.preservedKeyGuid, &tfPreservedKey, description->c_str(), static_cast<ULONG>(description->length())));
        }
    }

    void UninitializePreservedKeys() {
        const auto tfKeystrokeMgr = m_threadMgr.query<ITfKeystrokeMgr>();
        for (const auto& preservedKey: c_preservedKeys) {
            TF_PRESERVEDKEY tfPreservedKey = {};
            tfPreservedKey.uVKey = preservedKey.tfPreservedKey.uVKey;
            tfPreservedKey.uModifiers = preservedKey.tfPreservedKey.uModifiers & 0xffff; // clear extended bits
            LOG_IF_FAILED(tfKeystrokeMgr->UnpreserveKey(preservedKey.preservedKeyGuid, &tfPreservedKey));
        }
    }

    static const inline struct DISPLAYATTRIBUTEDATA {
        GUID guid;
        TF_DISPLAYATTRIBUTE tfDisplayAttribute;
        int valueNameResId;
        int descriptionResId;
    } c_listDisplayAttributeData[] = {
        {
            GUID { 0x4192edb6, 0x90cc, 0x44bc, { 0xbc, 0x2d, 0x4c, 0x51, 0xe0, 0x27, 0x1a, 0x80 } }, // {4192EDB6-90CC-44BC-BC2D-4C51E0271A80}
            TF_DISPLAYATTRIBUTE {
                { TF_CT_COLORREF, RGB(0, 103,206) },    // text color
                { TF_CT_NONE, 0 },                      // background color (TF_CT_NONE => app default)
                TF_LS_DOT,                              // underline style
                FALSE,                                  // underline boldness
                { TF_CT_COLORREF, RGB(0, 103,206) },    // underline color
                TF_ATTR_INPUT                           // attribute info
            },
            IDS_DISPATTRINFO_INPUT_NAME, IDS_DISPATTRINFO_INPUT_DESC
        },
        {
            GUID { 0xa01dad16, 0x7aee, 0x47e8, { 0xaa, 0x1d, 0x72, 0x7d, 0xe5, 0xc4, 0x5f, 0xf4 } }, // {A01DAD16-7AEE-47E8-AA1D-727DE5C45FF4}
            TF_DISPLAYATTRIBUTE {
                { TF_CT_COLORREF, RGB(255, 255, 255) }, // text color
                { TF_CT_COLORREF, RGB(  0, 255, 255) }, // background color (TF_CT_NONE => app default)
                TF_LS_NONE,                             // underline style
                FALSE,                                  // underline boldness
                { TF_CT_NONE, 0 },                      // underline color
                TF_ATTR_TARGET_CONVERTED                // attribute info
            },
            IDS_DISPATTRINFO_CONVERTED_NAME, IDS_DISPATTRINFO_CONVERTED_DESC
        },
        {
            GUID { 0x7c87af9c, 0xc10d, 0x4a86, { 0xbc, 0x9b, 0x30, 0xc0, 0x13, 0x3b, 0x7d, 0x44 } }, // {7C87AF9C-C10D-4A86-BC9B-30C0133B7D44}
            TF_DISPLAYATTRIBUTE {
                { TF_CT_COLORREF, RGB(0, 103,206) },    // text color
                { TF_CT_NONE, 0 },                      // background color (TF_CT_NONE => app default)
                TF_LS_DOT,                              // underline style
                FALSE,                                  // underline boldness
                { TF_CT_COLORREF, RGB(0, 103,206) },    // underline color
                TF_ATTR_CONVERTED                       // attribute info
            },
            IDS_DISPATTRINFO_INPUT_NAME, IDS_DISPATTRINFO_INPUT_DESC
        },
        {
            GUID { 0x8993fdc6, 0x1e39, 0x40d6, { 0x81, 0xd8, 0xb, 0xf0, 0xad, 0xf5, 0x11, 0xa } }, // {8993FDC6-1E39-40D6-81D8-0BF0ADF5110A}
            TF_DISPLAYATTRIBUTE {
                { TF_CT_COLORREF, RGB(255, 255, 255) }, // text color
                { TF_CT_COLORREF, RGB(  0, 255, 255) }, // background color (TF_CT_NONE => app default)
                TF_LS_NONE,                             // underline style
                FALSE,                                  // underline boldness
                { TF_CT_NONE, 0 },                      // underline color
                TF_ATTR_TARGET_NOTCONVERTED             // attribute info
            },
            IDS_DISPATTRINFO_CONVERTED_NAME, IDS_DISPATTRINFO_CONVERTED_DESC
        },
    };

    void InitializeDisplayAttributeInfo(_In_ ITfCategoryMgr* categoryMgr)
    {
        m_listDisplayAttributeInfo = std::make_shared<std::vector<std::pair<TfGuidAtom, wil::com_ptr<ITfDisplayAttributeInfo>>>>();

        for (const auto& displayAttributeData: c_listDisplayAttributeData) {
            TfGuidAtom tfGuidAtom;
            LOG_IF_FAILED(categoryMgr->RegisterGUID(displayAttributeData.guid, &tfGuidAtom));

            const auto description = GetStringFromResource(displayAttributeData.descriptionResId);
            wil::com_ptr<CDisplayAttributeInfo> displayAttributeInfo;
            LOG_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CDisplayAttributeInfo>(&displayAttributeInfo,
                    displayAttributeData.guid, displayAttributeData.tfDisplayAttribute, description, tfGuidAtom));

            m_listDisplayAttributeInfo->emplace_back(std::make_pair(tfGuidAtom, displayAttributeInfo.query<ITfDisplayAttributeInfo>()));
        }
    }

    void UninitializeDisplayAttributeInfo()
    {
        m_listDisplayAttributeInfo.reset();
    }


    static HRESULT CompartmentCallback(void* pv, REFGUID guidCompartment) {
        auto* _this = reinterpret_cast<RibbonIMEInProcClient*>(pv);

        if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE)) {
            _this->KeyboardOpenCompartmentUpdated();
        }

        _this->UpdateCustomStateIfRequired();
        return S_OK;
    }

    void KeyboardOpenCompartmentUpdated()
    {
        const auto isOpen = m_compartmentKeyboardOpenClose->GetCompartmentBOOL();
        DWORD conversionMode = isOpen ? TF_CONVERSIONMODE_NATIVE : TF_CONVERSIONMODE_ALPHANUMERIC;

        const auto conversionModePrev = m_compartmentKeyboardInputModeConversion->GetCompartmentDWORD();
        if (conversionMode != conversionModePrev) {
            m_compartmentKeyboardInputModeConversion->_SetCompartmentDWORD(conversionMode);
        }
    }

    void UpdateCustomStateIfRequired() {
        const auto isOpen = !!m_compartmentKeyboardOpenClose->GetCompartmentBOOL();

        if (m_lastCustomState.isOpen != isOpen) {
            m_framework->UpdateCustomState();

            m_lastCustomState.isOpen = isOpen;
        }
    }

private:
    WindowsImeLib::IWindowsIMEInProcFramework* m_framework;
    wil::com_ptr<ITfThreadMgr> m_threadMgr;
    TfClientId m_tfClientId = TF_CLIENTID_NULL;
    DWORD m_tfTmfActiveFlags = {};

    std::vector<std::shared_ptr<CCompartment>> m_listCompartment;
    std::vector<wil::com_ptr<CCompartmentEventSink>> m_listCompartmentEventSink;
    std::shared_ptr<CCompartment> m_compartmentKeyboardOpenClose;
    std::shared_ptr<CCompartment> m_compartmentKeyboardInputModeConversion;
    std::shared_ptr<CCompartment> m_compartmentKeyboardInputModeSentence;

    std::vector<wil::com_ptr<CLangBarItemButton>> m_listLanguageBarItem;

    std::shared_ptr<std::vector<std::pair<TfGuidAtom, wil::com_ptr<ITfDisplayAttributeInfo>>>> m_listDisplayAttributeInfo;

    struct CustomState {
        bool isOpen = false;
    } m_lastCustomState = {};
};

std::shared_ptr<WindowsImeLib::IWindowsIMEInProcClient> RibbonIMEInProcClient_CreateInstance(WindowsImeLib::IWindowsIMEInProcFramework* framework)
{
    return std::make_shared<RibbonIMEInProcClient>(framework);
}