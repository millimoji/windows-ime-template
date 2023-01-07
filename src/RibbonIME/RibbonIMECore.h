#pragma once

#include "../WindowsImeLib.h"

class RibbonIMECore :
    public WindowsImeLib::ICompositionProcessorEngine,
    public std::enable_shared_from_this<RibbonIMECore>
{
public:
    RibbonIMECore(const std::weak_ptr<WindowsImeLib::ICompositionProcessorEngineOwner>& owner);
    virtual ~RibbonIMECore();

    // Get locale
    BOOL SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode) override;

    BOOL IsKeyEaten(_In_ ITfThreadMgr* pThreadMgr, TfClientId tfClientId, UINT code, _Inout_updates_(1) WCHAR *pwch,
        BOOL isComposing, CANDIDATE_MODE candidateMode, BOOL isCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState) override;

    BOOL AddVirtualKey(WCHAR wch) override;
    void RemoveVirtualKey(DWORD_PTR dwIndex) override;
    void PurgeVirtualKey() override;

    DWORD_PTR GetVirtualKeyLength() override;

    void GetReadingStrings(_Inout_ std::vector<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded) override;
    void GetCandidateList(_Inout_ std::vector<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch) override;
    void GetCandidateStringInConverted(CStringRange &searchString, _In_ std::vector<CCandidateListItem> *pCandidateList) override;

    // Preserved key handler
    void OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId) override;

    // Punctuation
    BOOL IsPunctuation(WCHAR wch) override;
    WCHAR GetPunctuation(WCHAR wch) override;

    BOOL IsDoubleSingleByte(WCHAR wch) override;
    BOOL IsMakePhraseFromText() override;

    // Language bar control
    void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr) override;

    std::vector<DWORD>* GetCandidateListIndexRange() override;
    UINT GetCandidateWindowWidth() override;

    // Compartment
    HRESULT CompartmentCallback(REFGUID guidCompartment) noexcept override;
    void ClearCompartment(ITfThreadMgr *pThreadMgr, TfClientId tfClientId) override;

private:
    std::weak_ptr<WindowsImeLib::ICompositionProcessorEngineOwner> m_owner;
};

class RibbonTextInputProcessor :
    public WindowsImeLib::ITextInputProcessor,
    public std::enable_shared_from_this<RibbonTextInputProcessor>
{
public:
    RibbonTextInputProcessor(WindowsImeLib::ITextInputFramework* framework);
    virtual ~RibbonTextInputProcessor();

    std::wstring TestMethod(const std::wstring_view src) override;
    void SetFocus(bool isGotten) override;

private:
    WindowsImeLib::ITextInputFramework* m_framework;
};

class RibbonIMEConstants : public WindowsImeLib::IConstantProvider
{
    // GUIDs
    const CLSID& IMECLSID() noexcept override
    {
        // {FF089024-807B-4131-9CDF-3C3B067F081C}
        static const GUID imeClasId = { 0xff089024, 0x807b, 0x4131, { 0x9c, 0xdf, 0x3c, 0x3b, 0x6, 0x7f, 0x8, 0x1c } };
        return imeClasId;
    }
    const GUID& IMEProfileGuid() noexcept override
    {
        // {0F86F935-F13C-4813-9E26-14888E3F128E}
        static const GUID imeProfileId = { 0xf86f935, 0xf13c, 0x4813, { 0x9e, 0x26, 0x14, 0x88, 0x8e, 0x3f, 0x12, 0x8e } };
        return imeProfileId;
    }
    const GUID& DisplayAttributeInput() noexcept override
    {
        // {0437ED84-7948-4E19-B395-8A5E4316DEE3}
        static const GUID dispAttrInput = { 0x437ed84, 0x7948, 0x4e19, { 0xb3, 0x95, 0x8a, 0x5e, 0x43, 0x16, 0xde, 0xe3 } };
        return dispAttrInput;
    }
    const GUID& DisplayAttributeConverted() noexcept override
    {
        // {9B18B8EA-BF09-486C-AD47-9C698789346C}
        static const GUID dispAttrCongverted = { 0x9b18b8ea, 0xbf09, 0x486c, { 0xad, 0x47, 0x9c, 0x69, 0x87, 0x89, 0x34, 0x6c } };
        return dispAttrCongverted;
    }
    const GUID& CandUIElement() noexcept override
    {
        // {E1124AF5-A86E-4F39-8081-6FEEE3C90504}
        static const GUID candUiElementId = { 0xe1124af5, 0xa86e, 0x4f39, { 0x80, 0x81, 0x6f, 0xee, 0xe3, 0xc9, 0x5, 0x4 } };
        return candUiElementId;
    }
    const LCID GetLocale() noexcept override
    {
        return MAKELCID(GetLangID(), SORT_DEFAULT);
    }
    const LANGID GetLangID() noexcept override
    {
        return MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
    }
    const CLSID& ServerCLSID() noexcept override
    {
        // {37D39577-1437-4108-8FEC-6CB1EB730333}
        static const GUID serverCLSID = { 0x37d39577, 0x1437, 0x4108, { 0x8f, 0xec, 0x6c, 0xb1, 0xeb, 0x73, 0x3, 0x33 } };
        return serverCLSID;
    }
    const GUID& ServerAppID() noexcept override
    {
        // {BD659CE0-D6BF-4B15-9EA9-5C936A58FE30}
        static const GUID serverAppID = { 0xbd659ce0, 0xd6bf, 0x4b15, { 0x9e, 0xa9, 0x5c, 0x93, 0x6a, 0x58, 0xfe, 0x30 } };
        return serverAppID;
    }
    const wchar_t* ServerName() noexcept override
    {
        return L"Ribbon IME Singleton Sever";
    }
    void GetPreferredTouchKeyboardLayout(_Out_ TKBLayoutType* layoutType, _Out_ WORD* preferredLayoutId) noexcept override
    {
        *layoutType = TKBLT_OPTIMIZED;
        *preferredLayoutId = TKBL_OPT_JAPANESE_ABC;
    }
};
