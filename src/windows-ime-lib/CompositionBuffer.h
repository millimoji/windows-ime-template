// Copyright (c) millimoji@gmail.com
#pragma once

#include "Private.h"
#include "Globals.h"
#include "../WindowsImeLib.h"
#include "CandidateListUIPresenter.h"

class CompositionBuffer :
    public WindowsImeLib::IWindowsIMECompositionBuffer,
    public std::enable_shared_from_this<CompositionBuffer>
{
public:
    CompositionBuffer(
        WindowsImeLib::ICompositionProcessorEngineOwner* textService,
        const std::shared_ptr<WindowsImeLib::ICompositionProcessorEngine>& pCompositionProcessorEngine,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& candidateListView,
        const TfClientId& tfClientId,
        const TfGuidAtom& gaDisplayAttributeInput
    ) :
        _textService(textService),
        _pCompositionProcessorEngine(pCompositionProcessorEngine),
        _pCandidateListUIPresenter(candidateListView),
        _tfClientId(tfClientId),
        _gaDisplayAttributeInput(gaDisplayAttributeInput)
    {}
    virtual ~CompositionBuffer() {}

private:
    // functions for the composition object.
    void _TerminateComposition(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCalledFromDeactivate = FALSE) override;

    // key event handlers for composition/candidate/phrase common objects.
    HRESULT _HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext) override;
    HRESULT _HandleCancel(TfEditCookie ec, _In_ ITfContext* pContext) override;
    // key event handlers for composition object.
    HRESULT _HandleCompositionInput(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) override;
    HRESULT _HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext* pContext, BOOL fCandidateList) override;

    HRESULT _HandleCompositionConvert(TfEditCookie ec, _In_ ITfContext* pContext, BOOL isWildcardSearch) override;
    HRESULT _HandleCompositionBackspace(TfEditCookie ec, _In_ ITfContext* pContext) override;
    HRESULT _HandleCompositionArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, KEYSTROKE_FUNCTION keyFunction) override;
    HRESULT _HandleCompositionPunctuation(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) override;
    HRESULT _HandleCompositionDoubleSingleByte(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) override;
    // key event handlers for candidate object.
    HRESULT _HandleCandidateFinalize(TfEditCookie ec, _In_ ITfContext* pContext) override;
    HRESULT _HandleCandidateConvert(TfEditCookie ec, _In_ ITfContext* pContext) override;
    HRESULT _HandleCandidateArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, _In_ KEYSTROKE_FUNCTION keyFunction) override;
    HRESULT _HandleCandidateSelectByNumber(TfEditCookie ec, _In_ ITfContext* pContext, _In_ UINT uCode) override;
    // key event handlers for phrase object.
    HRESULT _HandlePhraseFinalize(TfEditCookie ec, _In_ ITfContext* pContext) override;
    HRESULT _HandlePhraseArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, _In_ KEYSTROKE_FUNCTION keyFunction) override;
    HRESULT _HandlePhraseSelectByNumber(TfEditCookie ec, _In_ ITfContext* pContext, _In_ UINT uCode) override;

    // functions for the composition object.
    HRESULT _HandleCompositionInputWorker(_In_ WindowsImeLib::ICompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext) override;
    HRESULT _CreateAndStartCandidate(_In_ WindowsImeLib::ICompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext) override;
    HRESULT _HandleCandidateWorker(TfEditCookie ec, _In_ ITfContext *pContext) override;

    HRESULT _StartComposition(TfEditCookie ec, _In_ ITfContext *_pContext);

    HRESULT _AddComposingAndChar(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString) override;
    HRESULT _AddCharAndFinalize(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString) override;

    BOOL _FindComposingRange(TfEditCookie ec, _In_ ITfContext *pContext, _In_ ITfRange *pSelection, _Outptr_result_maybenull_ ITfRange **ppRange) override;
    HRESULT _SetInputString(TfEditCookie ec, _In_ ITfContext *pContext, _Out_opt_ ITfRange *pRange, _In_ CStringRange *pstrAddString, BOOL exist_composing) override;
    HRESULT _InsertAtSelection(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString, _Outptr_ ITfRange **ppCompRange) override;

    HRESULT _RemoveDummyCompositionForComposing(TfEditCookie ec, _In_ ITfComposition *pComposition) override;

    // function for the language property
    BOOL _SetCompositionLanguage(TfEditCookie ec, _In_ ITfContext *pContext) override;

    // function for the display attribute
    void _ClearCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext) override;
    BOOL _SetCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext, TfGuidAtom gaDisplayAttribute) override;

    BOOL _IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover) override;
    VOID _DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext) override;
    BOOL _IsComposing() override;

    std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> GetCandidateList() override { return _pCandidateListUIPresenter; }
    wil::com_ptr<ITfContext> GetContext() override { return _pContext; }
    wil::com_ptr<ITfComposition> GetComposition() override { return _pComposition; }
    CANDIDATE_MODE CandidateMode() override { return _candidateMode; }
    bool IsCandidateWithWildcard() override  { return !!_isCandidateWithWildcard; }
    void ResetCandidateState() override
    {
        _candidateMode  = CANDIDATE_NONE;
        _isCandidateWithWildcard = FALSE;
    }
    void DestroyCandidateView() override
    {
        _pCandidateListUIPresenter->_EndCandidateList();
        _pCandidateListUIPresenter->DestroyView();
    }

private:
    void _SetComposition(_In_ ITfComposition *pComposition);
    void _SaveCompositionContext(_In_ ITfContext *pContext);

private:
    WindowsImeLib::ICompositionProcessorEngineOwner* _textService = nullptr;
    std::shared_ptr<WindowsImeLib::ICompositionProcessorEngine> _pCompositionProcessorEngine;
    std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> _pCandidateListUIPresenter;

    TfClientId _tfClientId = TF_CLIENTID_NULL;
    TfGuidAtom _gaDisplayAttributeInput = {};

    // need to sync with m_textService
    wil::com_ptr<ITfContext> _pContext;
    wil::com_ptr<ITfComposition> _pComposition;
    CANDIDATE_MODE _candidateMode = CANDIDATE_NONE;
    BOOL _isCandidateWithWildcard = FALSE;
};