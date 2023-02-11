// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "WindowsIME.h"
#include "CandidateListUIPresenter.h"
#include "CandidateListView.h"
#include "../Compartment.h"
#include "TfInputProcessorProfile.h"
#include "RegKey.h"
#include "SingletonProcessor.h"
#include "CompositionBuffer.h"

namespace wrl
{
    using namespace Microsoft::WRL;
}

//+---------------------------------------------------------------------------
//
// CreateInstance
//
//----------------------------------------------------------------------------

/* static */
HRESULT CWindowsIME::CreateInstance(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj) try
{
    if (pUnkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    wil::com_ptr<CWindowsIME> windowsIme;

    RETURN_IF_FAILED(wrl::MakeAndInitialize<CWindowsIME>(&windowsIme));

    RETURN_IF_FAILED(windowsIme->QueryInterface(riid, ppvObj));

    return S_OK;
}
CATCH_RETURN()

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CWindowsIME::CWindowsIME()
{
    DllAddRef();

    {
        wchar_t fileNameBuf[MAX_PATH];
        GetModuleFileName(nullptr, fileNameBuf, ARRAYSIZE(fileNameBuf));
        m_processName = fileNameBuf;
        m_processNameBstr.reset(SysAllocString(m_processName.c_str()));
    }
    CoCreateGuid(&m_clientGuid);
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CWindowsIME::~CWindowsIME()
{
    m_singletonProcessor.reset();

//    if (_pCandidateListUIPresenter)
//    {
//        delete _pCandidateListUIPresenter;
//        _pCandidateListUIPresenter = nullptr;
//    }

    DllRelease();
}

//+---------------------------------------------------------------------------
//
// ITfTextInputProcessorEx::ActivateEx
//
//----------------------------------------------------------------------------

STDAPI CWindowsIME::ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, DWORD dwFlags)
{
    auto activity = WindowsImeLibTelemetry::ITfTextInputProcessorEx_ActivateEx();

    _pThreadMgr = pThreadMgr;
    // _pThreadMgr->AddRef();

    _tfClientId = tfClientId;
    _dwActivateFlags = dwFlags;

    if (!_InitThreadMgrEventSink())
    {
        goto ExitError;
    }

    {
        wil::com_ptr<ITfDocumentMgr> pDocMgrFocus;
        if (SUCCEEDED_LOG(_pThreadMgr->GetFocus(&pDocMgrFocus)) && pDocMgrFocus)
        {
            _InitTextEditSink(pDocMgrFocus.get());
        }
    }

    if (!_InitKeyEventSink())
    {
        goto ExitError;
    }

    if (!_InitActiveLanguageProfileNotifySink())
    {
        goto ExitError;
    }

    if (!_InitThreadFocusSink())
    {
        goto ExitError;
    }

    if (!_InitDisplayAttributeGuidAtom())
    {
        goto ExitError;
    }

    if (!_InitFunctionProviderSink())
    {
        goto ExitError;
    }

    try
    {
        m_inprocClient = WindowsImeLib::g_processorFactory->CreateIMEInProcClient(this);
        m_inprocClient->Initialize(pThreadMgr, tfClientId, _IsSecureMode());
    }
    catch (...)
    {
        LOG_CAUGHT_EXCEPTION();
        goto ExitError;
    }

//    {
//        auto candidateListView = std::make_shared<CandidateListView>(this);
//        m_candidateListView = std::static_pointer_cast<ICandidateListViewInternal>(candidateListView);
//    }

    m_compositionBuffer = std::make_shared<CompositionBuffer>(
        this,
        nullptr, // nullptm_candidateListView->GetClientInterface(),
        _tfClientId,
        _gaDisplayAttributeInput);

    if (!_AddTextProcessorEngine())
    {
        goto ExitError;
    }

    activity.Stop();
    return S_OK;

ExitError:
    Deactivate();
    return E_FAIL;
}

//+---------------------------------------------------------------------------
//
// ITfTextInputProcessorEx::Deactivate
//
//----------------------------------------------------------------------------

STDAPI CWindowsIME::Deactivate()
{
    auto activity = WindowsImeLibTelemetry::ITfTextInputProcessorEx_Deactivate();

    auto pContext = m_compositionBuffer->GetCompositionContext();
    if (pContext)
    {
        // pContext->AddRef();
        LOG_IF_FAILED(m_compositionBuffer->_TerminateCompositionInternal());
    }

    if (m_singletonProcessor)
    {
//        _pCompositionProcessorEngine->ClearCompartment(_pThreadMgr, _tfClientId);
        m_singletonProcessor->CandidateListViewInternal_EndCandidateList();
        m_singletonProcessor.reset();
    }

//    if (_pCandidateListUIPresenter)
    {
//        delete _pCandidateListUIPresenter;
//        _pCandidateListUIPresenter = nullptr;
//        _pCandidateListUIPresenter.reset();
//
//        if (pContext)
//        {
//            pContext->Release();
//        }

//        _candidateMode = CANDIDATE_NONE;
//        _isCandidateWithWildcard = FALSE;
//        m_compositionBuffer->ResetCandidateState();

//        m_compositionBuffer->DestroyCandidateView();
//        m_candidateListView->_EndCandidateList();
    }

    if (m_inprocClient)
    {
        m_inprocClient->Deinitialize();
        m_inprocClient.reset();
    }

    _UninitFunctionProviderSink();

    _UninitThreadFocusSink();

    _UninitActiveLanguageProfileNotifySink();

    _UninitKeyEventSink();

    _UninitThreadMgrEventSink();

//    CCompartment CompartmentKeyboardOpen(_pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//    CompartmentKeyboardOpen._ClearCompartment();
//
//    CCompartment CompartmentDoubleSingleByte(_pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//    CompartmentDoubleSingleByte._ClearCompartment();
//
//    CCompartment CompartmentPunctuation(_pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
//    CompartmentDoubleSingleByte._ClearCompartment();

//    if (_pThreadMgr != nullptr)
//    {
//        _pThreadMgr->Release();
//    }

    _pThreadMgr.reset();
    _tfClientId = TF_CLIENTID_NULL;

//    if (_pDocMgrLastFocused)
//    {
//        _pDocMgrLastFocused->Release();
//        _pDocMgrLastFocused = nullptr;
//    }
    _pDocMgrLastFocused.reset();

    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfFunctionProvider::GetType
//
//----------------------------------------------------------------------------
HRESULT CWindowsIME::GetType(__RPC__out GUID *pguid)
{
    auto activity = WindowsImeLibTelemetry::ITfFunctionProvider_GetType();

    HRESULT hr = E_INVALIDARG;
    if (pguid)
    {
        *pguid = WindowsImeLib::g_processorFactory->GetConstantProvider()->IMECLSID();
        hr = S_OK;
    }

    activity.Stop();
    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFunctionProvider::GetDescription
//
//----------------------------------------------------------------------------
HRESULT CWindowsIME::GetDescription(__RPC__deref_out_opt BSTR *pbstrDesc)
{
    auto activity = WindowsImeLibTelemetry::ITfFunctionProvider_GetDescription();

    HRESULT hr = E_INVALIDARG;
    if (pbstrDesc != nullptr)
    {
        *pbstrDesc = nullptr;
        hr = E_NOTIMPL;
    }

    activity.Stop();
    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFunctionProvider::GetFunction
//
//----------------------------------------------------------------------------
HRESULT CWindowsIME::GetFunction(__RPC__in REFGUID rguid, __RPC__in REFIID riid, __RPC__deref_out_opt IUnknown **ppunk)
{
    auto activity = WindowsImeLibTelemetry::ITfFunctionProvider_GetFunction();

    HRESULT hr = E_NOINTERFACE;
    if ((IsEqualGUID(rguid, GUID_NULL)) 
        && (IsEqualGUID(riid, __uuidof(ITfFnSearchCandidateProvider))))
    {
        hr = _pITfFnSearchCandidateProvider->QueryInterface(riid, (void**)ppunk);
    }
    else if (IsEqualGUID(rguid, GUID_NULL))
    {
        hr = QueryInterface(riid, (void **)ppunk);
    }

    activity.Stop();
    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFunction::GetDisplayName
//
//----------------------------------------------------------------------------
HRESULT CWindowsIME::GetDisplayName(_Out_ BSTR *pbstrDisplayName)
{
    auto activity = WindowsImeLibTelemetry::ITfFunction_GetDisplayName();

    HRESULT hr = E_INVALIDARG;
    if (pbstrDisplayName != nullptr)
    {
        *pbstrDisplayName = nullptr;
        hr = E_NOTIMPL;
    }

    activity.Stop();
    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFnGetPreferredTouchKeyboardLayout::GetLayout
// The tkblayout will be Optimized layout.
//----------------------------------------------------------------------------
HRESULT CWindowsIME::GetLayout(_Out_ TKBLayoutType *ptkblayoutType, _Out_ WORD *pwPreferredLayoutId)
{
    auto activity = WindowsImeLibTelemetry::ITfFnGetPreferredTouchKeyboardLayout_GetLayout();

    HRESULT hr = E_INVALIDARG;
    if ((ptkblayoutType != nullptr) && (pwPreferredLayoutId != nullptr))
    {
        WindowsImeLib::g_processorFactory->GetConstantProvider()->GetPreferredTouchKeyboardLayout(ptkblayoutType, pwPreferredLayoutId);
        hr = S_OK;
    }

    activity.Stop();
    return hr;
}

//////////////////////////////////////////////////////////////////////
//
// CWindowsIME implementation.
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _AddTextProcessorEngine
//
//----------------------------------------------------------------------------

BOOL CWindowsIME::_AddTextProcessorEngine()
{
    LANGID langid = 0;
    CLSID clsid = GUID_NULL;
    GUID guidProfile = GUID_NULL;

    // Get default profile.
    CTfInputProcessorProfile profile;

    if (FAILED(profile.CreateInstance()))
    {
        return FALSE;
    }

    if (FAILED(profile.GetCurrentLanguage(&langid)))
    {
        return FALSE;
    }

    if (FAILED(profile.GetDefaultLanguageProfile(langid, GUID_TFCAT_TIP_KEYBOARD, &clsid, &guidProfile)))
    {
        return FALSE;
    }

    // Is this already added?
#if 0
    if (m_singletonProcessor)
    {
        LANGID langidProfile = WindowsImeLib::g_processorFactory->GetConstantProvider()->GetLangID();
        GUID guidLanguageProfile = WindowsImeLib::g_processorFactory->GetConstantProvider()->IMEProfileGuid();

        if ((langid == langidProfile) && IsEqualGUID(guidProfile, guidLanguageProfile))
        {
            return TRUE;
        }
    }
#endif

    // Create composition processor engine
    if (!m_singletonProcessor)
    {
        m_singletonProcessor = CreateSingletonProcessorBridge();
        UpdateCustomState();
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// CWindowsIME::CreateInstance 
//
//----------------------------------------------------------------------------

HRESULT CWindowsIME::CreateInstance(REFCLSID rclsid, REFIID riid, _Outptr_result_maybenull_ LPVOID* ppv, _Out_opt_ HINSTANCE* phInst, BOOL isComLessMode)
{
    HRESULT hr = S_OK;
    if (phInst == nullptr)
    {
        return E_INVALIDARG;
    }

    *phInst = nullptr;

    if (!isComLessMode)
    {
        hr = ::CoCreateInstance(rclsid, 
            NULL, 
            CLSCTX_INPROC_SERVER,
            riid,
            ppv);
    }
    else
    {
        hr = CWindowsIME::ComLessCreateInstance(rclsid, riid, ppv, phInst);
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// CWindowsIME::ComLessCreateInstance
//
//----------------------------------------------------------------------------

HRESULT CWindowsIME::ComLessCreateInstance(REFGUID rclsid, REFIID riid, _Outptr_result_maybenull_ void **ppv, _Out_opt_ HINSTANCE *phInst)
{
    HRESULT hr = S_OK;
    HINSTANCE sampleIMEDllHandle = nullptr;
    WCHAR wchPath[MAX_PATH] = {'\0'};
    WCHAR szExpandedPath[MAX_PATH] = {'\0'};
    DWORD dwCnt = 0;
    *ppv = nullptr;

    hr = phInst ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        *phInst = nullptr;
        hr = CWindowsIME::GetComModuleName(rclsid, wchPath, ARRAYSIZE(wchPath));
        if (SUCCEEDED(hr))
        {
            dwCnt = ExpandEnvironmentStringsW(wchPath, szExpandedPath, ARRAYSIZE(szExpandedPath));
            hr = (0 < dwCnt && dwCnt <= ARRAYSIZE(szExpandedPath)) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                sampleIMEDllHandle = LoadLibraryEx(szExpandedPath, NULL, 0);
                hr = sampleIMEDllHandle ? S_OK : E_FAIL;
                if (SUCCEEDED(hr))
                {
                    *phInst = sampleIMEDllHandle;
                    FARPROC pfn = GetProcAddress(sampleIMEDllHandle, "DllGetClassObject");
                    hr = pfn ? S_OK : E_FAIL;
                    if (SUCCEEDED(hr))
                    {
                        IClassFactory *pClassFactory = nullptr;
                        hr = ((HRESULT (STDAPICALLTYPE *)(REFCLSID rclsid, REFIID riid, LPVOID *ppv))(pfn))(rclsid, IID_IClassFactory, (void **)&pClassFactory);
                        if (SUCCEEDED(hr) && pClassFactory)
                        {
                            hr = pClassFactory->CreateInstance(NULL, riid, ppv);
                            pClassFactory->Release();
                        }
                    }
                }
            }
        }
    }

    if (!SUCCEEDED(hr) && phInst && *phInst)
    {
        FreeLibrary(*phInst);
        *phInst = 0;
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// CWindowsIME::GetComModuleName
//
//----------------------------------------------------------------------------

HRESULT CWindowsIME::GetComModuleName(REFGUID rclsid, _Out_writes_(cchPath)WCHAR* wchPath, DWORD cchPath)
{
    HRESULT hr = S_OK;

    CRegKey key;
    WCHAR wchClsid[CLSID_STRLEN + 1];
    hr = CLSIDToString(rclsid, wchClsid) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        WCHAR wchKey[MAX_PATH];
        hr = StringCchPrintfW(wchKey, ARRAYSIZE(wchKey), L"CLSID\\%s\\InProcServer32", wchClsid);
        if (SUCCEEDED(hr))
        {
            hr = (key.Open(HKEY_CLASSES_ROOT, wchKey, KEY_READ) == ERROR_SUCCESS) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                WCHAR wszModel[MAX_PATH];
                ULONG cch = ARRAYSIZE(wszModel);
                hr = (key.QueryStringValue(L"ThreadingModel", wszModel, &cch) == ERROR_SUCCESS) ? S_OK : E_FAIL;
                if (SUCCEEDED(hr))
                {
                    if (CompareStringOrdinal(wszModel, 
                        -1, 
                        L"Apartment", 
                        -1,
                        TRUE) == CSTR_EQUAL)
                    {
                        hr = (key.QueryStringValue(NULL, wchPath, &cchPath) == ERROR_SUCCESS) ? S_OK : E_FAIL;
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }
        }
    }

    return hr;
}

void CCandidateWindow::SetDefaultCandidateTextFont()
{
    // Candidate Text Font
    if (Global::defaultlFontHandle == nullptr)
    {
        const auto fontNameResourceId = WindowsImeLib::g_processorFactory->GetConstantProvider()->GetDefaultCandidateTextFontResourceID();

        WCHAR fontName[50] = {'\0'}; 
        LoadString(Global::dllInstanceHandle, fontNameResourceId, fontName, 50);
        Global::defaultlFontHandle = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, fontName);
        if (!Global::defaultlFontHandle)
        {
            LOGFONT lf = {};
            SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
            // Fall back to the default GUI font on failure.
            Global::defaultlFontHandle = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, lf.lfFaceName);
        }
    }
}
