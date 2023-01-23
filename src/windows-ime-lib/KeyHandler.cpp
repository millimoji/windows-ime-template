// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "EditSession.h"
#include "WindowsIME.h"
#include "CandidateListUIPresenter.h"

//////////////////////////////////////////////////////////////////////
//
// CWindowsIME class
//
//////////////////////////////////////////////////////////////////////

// //+---------------------------------------------------------------------------
// //
// // _IsRangeCovered
// //
// // Returns TRUE if pRangeTest is entirely contained within pRangeCover.
// //
// //----------------------------------------------------------------------------
// 
// BOOL CompositionBuffer::_IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover)
// {
//     LONG lResult = 0;;
// 
//     if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult)) 
//         || (lResult > 0))
//     {
//         return FALSE;
//     }
// 
//     if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult)) 
//         || (lResult < 0))
//     {
//         return FALSE;
//     }
// 
//     return TRUE;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _DeleteCandidateList
// //
// //----------------------------------------------------------------------------
// 
// VOID CompositionBuffer::_DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext)
// {
//     isForce;pContext;
// 
//     _pCompositionProcessorEngine->PurgeVirtualKey();
// 
//     if (_pCandidateListUIPresenter->IsCreated())
//     {
//         _pCandidateListUIPresenter->_EndCandidateList();
// 
//         // _candidateMode = CANDIDATE_NONE;
//         // _isCandidateWithWildcard = FALSE;
// 
//         ResetCandidateState();
//     }
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _HandleComplete
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CompositionBuffer::_HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext)
// {
//     _DeleteCandidateList(FALSE, pContext);
// 
//     // just terminate the composition
//     _TerminateComposition(ec, pContext);
// 
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _CreateAndStartCandidate
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CompositionBuffer::_CreateAndStartCandidate(_In_ WindowsImeLib::ICompositionProcessorEngine* /*pCompositionProcessorEngine*/, TfEditCookie ec, _In_ ITfContext* pContext)
// {
//     HRESULT hr = S_OK;
// 
//     if (((_candidateMode == CANDIDATE_PHRASE) && _pCandidateListUIPresenter->IsCreated())
//         || ((_candidateMode == CANDIDATE_NONE) && _pCandidateListUIPresenter->IsCreated()))
//     {
//         // Recreate candidate list
//         _pCandidateListUIPresenter->_EndCandidateList();
//         // delete _pCandidateListUIPresenter;
//         // _pCandidateListUIPresenter = nullptr;
//         // _pCandidateListUIPresenter.reset();
//         _pCandidateListUIPresenter->DestroyView();
// 
//         _candidateMode = CANDIDATE_NONE;
//         _isCandidateWithWildcard = FALSE;
//     }
// 
// //    if (_pCandidateListUIPresenter == nullptr)
//     if (!_pCandidateListUIPresenter->IsCreated())
//     {
//         _pCandidateListUIPresenter->CreateView(
//             Global::AtomCandidateWindow,
//             CATEGORY_CANDIDATE,
//             _pCompositionProcessorEngine->GetCandidateListIndexRange(),
//             FALSE);
// 
// //        _pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(
// //            reinterpret_cast<CWindowsIME*>(_textService->GetTextService()),
// //            Global::AtomCandidateWindow,
// //            CATEGORY_CANDIDATE,
// //            pCompositionProcessorEngine->GetCandidateListIndexRange(),
// //            FALSE);
// //        if (!_pCandidateListUIPresenter)
// //        {
// //            return E_OUTOFMEMORY;
// //        }
// 
//         _candidateMode = CANDIDATE_INCREMENTAL;
//         _isCandidateWithWildcard = FALSE;
// 
//         // we don't cache the document manager object. So get it from pContext.
//         ITfDocumentMgr* pDocumentMgr = nullptr;
//         if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
//         {
//             // get the composition range.
//             ITfRange* pRange = nullptr;
//             if (SUCCEEDED(_pComposition->GetRange(&pRange)))
//             {
//                 hr = _pCandidateListUIPresenter->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange,
//                         WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
//                 pRange->Release();
//             }
//             pDocumentMgr->Release();
//         }
//     }
// 
//     return hr;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _HandleCompositionConvert
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CompositionBuffer::_HandleCompositionConvert(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isWildcardSearch)
// {
//     HRESULT hr = S_OK;
// 
//     std::vector<CCandidateListItem> candidateList;
// 
//     //
//     // Get candidate string from composition processor engine
//     //
//     auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();
//     pCompositionProcessorEngine->GetCandidateList(&candidateList, FALSE, isWildcardSearch);
// 
//     // If there is no candlidate listin the current reading string, we don't do anything. Just wait for
//     // next char to be ready for the conversion with it.
//     int nCount = static_cast<int>(candidateList.size());
//     if (nCount)
//     {
// //        if (_pCandidateListUIPresenter)
// //        {
// //            _pCandidateListUIPresenter->_EndCandidateList();
// //            // delete _pCandidateListUIPresenter;
// //            // _pCandidateListUIPresenter = nullptr;
// //            _pCandidateListUIPresenter.reset();
// //
// //            _candidateMode = CANDIDATE_NONE;
// //            _isCandidateWithWildcard = FALSE;
// //        }
//         if (_pCandidateListUIPresenter->IsCreated())
//         {
//             _pCandidateListUIPresenter->_EndCandidateList();
//             _pCandidateListUIPresenter->DestroyView();
//             this->ResetCandidateState();
//         }
// 
//         // 
//         // create an instance of the candidate list class.
//         // 
// //        if (_pCandidateListUIPresenter == nullptr)
// //        {
// //            _pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(
// //                reinterpret_cast<CWindowsIME*>(_textService->GetTextService()),
// //                Global::AtomCandidateWindow,
// //                CATEGORY_CANDIDATE,
// //                pCompositionProcessorEngine->GetCandidateListIndexRange(),
// //                FALSE);
// //            if (!_pCandidateListUIPresenter)
// //            {
// //                return E_OUTOFMEMORY;
// //            }
// //
// //            _candidateMode = CANDIDATE_ORIGINAL;
// //        }
//         if (!_pCandidateListUIPresenter->IsCreated())
//         {
//             _pCandidateListUIPresenter->CreateView(
//                                             Global::AtomCandidateWindow,
//                                             CATEGORY_CANDIDATE,
//                                             pCompositionProcessorEngine->GetCandidateListIndexRange(),
//                                             FALSE);
//             _candidateMode = CANDIDATE_ORIGINAL;
//         }
// 
//         _isCandidateWithWildcard = isWildcardSearch;
// 
//         // we don't cache the document manager object. So get it from pContext.
//         ITfDocumentMgr* pDocumentMgr = nullptr;
//         if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
//         {
//             // get the composition range.
//             ITfRange* pRange = nullptr;
//             if (SUCCEEDED(_pComposition->GetRange(&pRange)))
//             {
//                 hr = _pCandidateListUIPresenter->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange,
//                         WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
//                 pRange->Release();
//             }
//             pDocumentMgr->Release();
//         }
//         if (SUCCEEDED(hr))
//         {
//             _pCandidateListUIPresenter->_SetText(&candidateList, FALSE);
//         }
//     }
// 
//     return hr;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _HandleCompositionArrowKey
// //
// // Update the selection within a composition.
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CompositionBuffer::_HandleCompositionArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, KEYSTROKE_FUNCTION keyFunction)
// {
//     ITfRange* pRangeComposition = nullptr;
//     TF_SELECTION tfSelection;
//     ULONG fetched = 0;
// 
//     // get the selection
//     if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched))
//         || fetched != 1)
//     {
//         // no selection, eat the keystroke
//         return S_OK;
//     }
// 
//     // get the composition range
//     if (FAILED(_pComposition->GetRange(&pRangeComposition)))
//     {
//         goto Exit;
//     }
// 
//     // For incremental candidate list
//     if (_pCandidateListUIPresenter->IsCreated())
//     {
//         _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);
//     }
// 
//     pContext->SetSelection(ec, 1, &tfSelection);
// 
//     pRangeComposition->Release();
// 
// Exit:
//     tfSelection.range->Release();
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _InvokeKeyHandler
// //
// // This text service is interested in handling keystrokes to demonstrate the
// // use the compositions. Some apps will cancel compositions if they receive
// // keystrokes while a compositions is ongoing.
// //
// // param
// //    [in] uCode - virtual key code of WM_KEYDOWN wParam
// //    [in] dwFlags - WM_KEYDOWN lParam
// //    [in] dwKeyFunction - Function regarding virtual key
// //----------------------------------------------------------------------------
// 
// HRESULT CWindowsIME::_InvokeKeyHandler(_In_ ITfContext *pContext, UINT code, WCHAR wch, DWORD flags, _KEYSTROKE_STATE keyState)
// {
//     flags;
// 
//     CKeyHandlerEditSession* pEditSession = nullptr;
//     HRESULT hr = E_FAIL;
// 
//     // we'll insert a char ourselves in place of this keystroke
//     pEditSession = new (std::nothrow) CKeyHandlerEditSession(this, pContext, code, wch, keyState);
//     if (pEditSession == nullptr)
//     {
//         goto Exit;
//     }
// 
//     //
//     // Call CKeyHandlerEditSession::DoEditSession().
//     //
//     // Do not specify TF_ES_SYNC so edit session is not invoked on WinWord
//     //
//     hr = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
// 
//     pEditSession->Release();
// 
// Exit:
//     return hr;
// }
