// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "SampleIMEDefine.h"
#include "SampleIMEGlobals.h"
#include "../WindowsImeLib.h"
#include "CompositionProcessorEngine.h"
// #include "Globals.h"
// #include "Private.h"
// #include "WindowsIME.h"

class CKeyStateCategory;

class CKeyStateCategoryFactory
{
public:
    static CKeyStateCategoryFactory* Instance();
    CKeyStateCategory* MakeKeyStateCategory(
        KEYSTROKE_CATEGORY keyCategory,
        _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
        const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine);
    void Release();

protected:
    CKeyStateCategoryFactory();

private:
    static CKeyStateCategoryFactory* _instance;

};

typedef struct KeyHandlerEditSessionDTO
{
    KeyHandlerEditSessionDTO(UINT virualCode, WCHAR inputChar, KEYSTROKE_FUNCTION arrowKeyFunction)
    {
        code = virualCode;
        wch = inputChar;
        arrowKey = arrowKeyFunction;
    }

    UINT code;
    WCHAR wch;
    KEYSTROKE_FUNCTION arrowKey;
}KeyHandlerEditSessionDTO;

class CKeyStateCategory
{
public:
    CKeyStateCategory(
        _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
        const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine);

protected:
    ~CKeyStateCategory(void);

public:
    HRESULT KeyStateHandler(KEYSTROKE_FUNCTION function, KeyHandlerEditSessionDTO dto);
    void Release(void);

protected:
    // HandleKeyInput
    virtual HRESULT HandleKeyInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyFinalizeTextStoreAndInput
    virtual HRESULT HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyFinalizeTextStore
    virtual HRESULT HandleKeyFinalizeTextStore();

    // HandleKeyFinalizeCandidatelistAndInput
    virtual HRESULT HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyFinalizeCandidatelist
    virtual HRESULT HandleKeyFinalizeCandidatelist();

    // HandleKeyConvert
    virtual HRESULT HandleKeyConvert(KeyHandlerEditSessionDTO dto);

    // HandleKeyConvertWild
    virtual HRESULT HandleKeyConvertWildCard(KeyHandlerEditSessionDTO dto);

    // HandleKeyCancel
    virtual HRESULT HandleKeyCancel();

    // HandleKeyBackspace
    virtual HRESULT HandleKeyBackspace();

    // HandleKeyArrow
    virtual HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto);

    // HandleKeyDoubleSingleByte
    virtual HRESULT HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto);

    // HandleKeyPunctuation
    virtual HRESULT HandleKeyPunctuation(KeyHandlerEditSessionDTO dto);

    // HandleKeySelectByNumber
    virtual HRESULT HandleKeySelectByNumber(UINT keyCode);

protected:
    HRESULT _HandleCancel();
    // key event handlers for composition object.
    HRESULT _HandleCompositionInput(WCHAR wch);
    HRESULT _HandleCompositionFinalize(BOOL fCandidateList);
    HRESULT _HandleComplete();

    HRESULT _HandleCompositionConvert(const KeyHandlerEditSessionDTO& dto, BOOL isWildcardSearch);
    HRESULT _HandleCompositionBackspace();
    HRESULT _HandleCompositionArrowKey(const KeyHandlerEditSessionDTO& dto);
    HRESULT _HandleCompositionPunctuation(const KeyHandlerEditSessionDTO& dto);
    HRESULT _HandleCompositionDoubleSingleByte(const KeyHandlerEditSessionDTO& dto);
    // key event handlers for candidate object.
    HRESULT _HandleCandidateFinalize();
    HRESULT _HandleCandidateConvert();
    HRESULT _HandleCandidateArrowKey(const KeyHandlerEditSessionDTO& dto);
    HRESULT _HandleCandidateSelectByNumber(UINT keyCode);
    // key event handlers for phrase object.
    HRESULT _HandlePhraseFinalize();
    HRESULT _HandlePhraseArrowKey(const KeyHandlerEditSessionDTO& dto);
    HRESULT _HandlePhraseSelectByNumber(UINT keyCode);

    // worker functions for the composition object.
    HRESULT _HandleCompositionInputWorker();
    HRESULT _CreateAndStartCandidate();

//    BOOL _IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover);
    void RemoveSpecificCandidateFromList(_In_ LCID Locale, _Inout_ std::vector<CCandidateListItem> &candidateList, const shared_wstring& candidateString);

protected:
    WindowsImeLib::IWindowsIMECompositionBuffer* _pTextService = nullptr;
    std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> _pCandidateListUIPresenter;
    std::shared_ptr<CompositionProcessorEngine> _pCompositionProcessorEngine;
};

class CKeyStateComposing : public CKeyStateCategory
{
public:
    CKeyStateComposing(
        _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
        const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine);

protected:
    // _HandleCompositionInput
    HRESULT HandleKeyInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyCompositionFinalizeTextStoreAndInput
    HRESULT HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyFinalizeTextStore
    HRESULT HandleKeyFinalizeTextStore();

    // HandleKeyCompositionFinalizeCandidatelistAndInput
    HRESULT HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyCompositionFinalizeCandidatelist
    HRESULT HandleKeyFinalizeCandidatelist();

    // HandleCompositionConvert
    HRESULT HandleKeyConvert(KeyHandlerEditSessionDTO dto);

    // HandleKeyCompositionConvertWildCard
    HRESULT HandleKeyConvertWildCard(KeyHandlerEditSessionDTO dto);

    // HandleCancel
    HRESULT HandleKeyCancel();

    // HandleCompositionBackspace
    HRESULT HandleKeyBackspace();

    // HandleArrowKey
    HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto);

    // HandleKeyDoubleSingleByte
    HRESULT HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto);

    // HandleKeyCompositionPunctuation
    HRESULT HandleKeyPunctuation(KeyHandlerEditSessionDTO dto);
};

class CKeyStateCandidate : public CKeyStateCategory
{
public:
    CKeyStateCandidate(
        _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
        const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine);

protected:
    // HandleKeyFinalizeCandidatelist
    HRESULT HandleKeyFinalizeCandidatelist();

    // HandleKeyFinalizeCandidatelistAndInput
    HRESULT HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto);

    //_HandleCandidateConvert
    HRESULT HandleKeyConvert();

    //_HandleCancel
    HRESULT HandleKeyCancel();

    //_HandleCandidateArrowKey
    HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto);

    //_HandleCandidateSelectByNumber
    HRESULT HandleKeySelectByNumber(UINT keyCode);
};

class CKeyStatePhrase : public CKeyStateCategory
{
public:
    CKeyStatePhrase(
        _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
        const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine);

protected:
    //_HandleCancel
    HRESULT HandleKeyFinalizeCandidatelist();

    //_HandleCancel
    HRESULT HandleKeyCancel();

    //_HandlePhraseArrowKey
    HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto);

    //_HandlePhraseSelectByNumber
    HRESULT HandleKeySelectByNumber(UINT keyCode);
};

//degeneration class
class CKeyStateNull : public CKeyStateCategory
{
public:
    CKeyStateNull(
        _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
        const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine) :
        CKeyStateCategory(pTextService, pCandidateListUIPresenter, pCompositionProcessorEngine) {}

protected:
    // _HandleNullInput
    HRESULT HandleKeyInput(KeyHandlerEditSessionDTO dto) { return __super::HandleKeyInput(dto); };

    // HandleKeyNullFinalizeTextStoreAndInput
    HRESULT HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto) { return __super::HandleKeyFinalizeTextStoreAndInput(dto); };

    // HandleKeyFinalizeTextStore
    HRESULT HandleKeyFinalizeTextStore() { return __super::HandleKeyFinalizeTextStore(); };

    // HandleKeyNullFinalizeCandidatelistAndInput
    HRESULT HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto) { return __super::HandleKeyFinalizeCandidatelistAndInput(dto); };

    // HandleKeyNullFinalizeCandidatelist
    HRESULT HandleKeyFinalizeCandidatelist() { return __super::HandleKeyFinalizeCandidatelist(); };

    //_HandleNullConvert
    HRESULT HandleKeyConvert(KeyHandlerEditSessionDTO dto) { return __super::HandleKeyConvert(dto); };

    //_HandleNullCancel
    HRESULT HandleKeyCancel() { return __super::HandleKeyCancel(); };

    // HandleKeyNullConvertWild
    HRESULT HandleKeyConvertWildCard(KeyHandlerEditSessionDTO dto) { return __super::HandleKeyConvertWildCard(dto); };

    //_HandleNullBackspace
    HRESULT HandleKeyBackspace() { return __super::HandleKeyBackspace(); };

    //_HandleNullArrowKey
    HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto) { return __super::HandleKeyArrow(dto); };

    // HandleKeyDoubleSingleByte
    HRESULT HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto) { return __super::HandleKeyDoubleSingleByte(dto); };

    // HandleKeyPunctuation
    HRESULT HandleKeyPunctuation(KeyHandlerEditSessionDTO dto) { return __super::HandleKeyPunctuation(dto); };

    //_HandleNullCandidateSelectByNumber
    HRESULT HandleKeySelectByNumber(UINT keyCode) { return __super::HandleKeySelectByNumber(keyCode); };
};