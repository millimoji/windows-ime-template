#pragma once

#include "../WindowsImeLib.h"

class RibbonIMECore : public WindowsImeLib::ICompositionProcessorEngine, public std::enable_shared_from_this<RibbonIMECore>
{
public:
    RibbonIMECore(const std::weak_ptr<WindowsImeLib::ICompositionProcessorEngineOwner>& owner);
    virtual ~RibbonIMECore();

    // Get language profile.
    const GUID& GetLanguageProfile(LANGID *plangid) override;

    // Get locale
    BOOL SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode) override;
    LCID GetLocale() override;

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

class RibbonIMEConstants : public WindowsImeLib::IConstantProvider
{
    // GUIDs
    const CLSID& IMECLSID() noexcept override;
    const GUID& IMEProfileGuid() noexcept override;
    const GUID& DisplayAttributeInput() noexcept override;
    const GUID& DisplayAttributeConverted() noexcept override;
    const GUID& CandUIElement() noexcept override;
    const LANGID GetLangID() noexcept override;
};
