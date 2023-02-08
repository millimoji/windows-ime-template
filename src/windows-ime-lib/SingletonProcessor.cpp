#include "Private.h"
#include "Globals.h"
#include "SingletonProcessor.h"
#include "ThreadTaskRunner.h"
#include "CandidateListView.h"
#include "CompositionBuffer.h"

namespace wrl
{
    using namespace Microsoft::WRL;
}

class SingletonProcessorEnvironment :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        ITextInputEnvironment,
                                        Microsoft::WRL::FtmBase>
{
public:
    SingletonProcessorEnvironment() {}
    virtual ~SingletonProcessorEnvironment() {}

    HRESULT RuntimeClassInitialize(const std::shared_ptr<TaskRunner>& taskRunner)
    {
        m_taskRunner = taskRunner;
        return S_OK;
    }

private:
    IFACEMETHODIMP TestMethod(_In_ BSTR src, _Outptr_ BSTR* result)
    {
        std::wstring work(src);
        work += L"-suffix";
        *result = SysAllocStringLen(work.c_str(), static_cast<UINT>(work.length()));
        return S_OK;
    }

private:
    std::shared_ptr<TaskRunner> m_taskRunner;
};


struct SingletonProcessorBridge :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        ITextInputProcessor,
                                        Microsoft::WRL::FtmBase>
{
    SingletonProcessorBridge()
    {
    }
    ~SingletonProcessorBridge()
    {
        if (m_engine && m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([this]() { m_engine.reset(); });
        }
    }

    IFACEMETHODIMP Acivate(_In_ ITextInputEnvironment* /*environment*/) override
    {
        EnsureInitialized();
        HRESULT hr = S_OK;
        m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->Acivate(nullptr); });
        return hr;
    }

    IFACEMETHODIMP Deacivate(void) override
    {
        EnsureInitialized();
        HRESULT hr = S_OK;
        m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->Deacivate(); });
        return hr;
    }

    IFACEMETHODIMP SetFocus(BOOL isGotten) override
    {
        EnsureInitialized();
        HRESULT hr = S_OK;
        m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->SetFocus(isGotten); });
        return hr;
    }

    IFACEMETHODIMP UpdateCustomState(LPCSTR customStateJson) override
    {
        HRESULT hr = S_OK;
        if (m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->UpdateCustomState(customStateJson); });
        }
        return hr;
    }

    // ICompositionProcessorEngine
    IFACEMETHODIMP OnKeyEvent(DWORD wParam, DWORD lParam, WCHAR wch, UINT vkPackSource,
                        BOOL isKbdDisabled, DWORD modifiers, DWORD uniqueModifiers, BOOL isTest, BOOL isDown,
                        _Outptr_ BSTR* result, _Out_ BOOL *pIsEaten) override
    {
        EnsureInitialized();

        HRESULT hr = S_OK;
        if (m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([&]() {
                hr = m_engine->OnKeyEvent(wParam, lParam, wch, vkPackSource, isKbdDisabled, modifiers, uniqueModifiers, isTest, isDown, result, pIsEaten); });
        }
        return hr;
    }

    IFACEMETHODIMP GetCandidateList(BOOL isIncrementalWordSearch, BOOL isWildcardSearch, BSTR* candidateList) override
    {
        HRESULT hr = S_OK;
        if (m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->GetCandidateList(isIncrementalWordSearch, isWildcardSearch, candidateList); });
        }
        return hr;
    }

    IFACEMETHODIMP FinalizeCandidateList() override
    {
        HRESULT hr = S_OK;
        if (m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->FinalizeCandidateList(); });
        }
        return hr;
    }

    IFACEMETHODIMP _DeleteCandidateList() override
    {
        HRESULT hr = S_OK;
        if (m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->_DeleteCandidateList(); });
        }
        return hr;
    }

    // ICandidateListViewInternal
    IFACEMETHODIMP CandidateListViewInternal_LayoutChangeNotification(HWND hwnd, RECT *lpRect)
    {
        HRESULT hr = S_OK;
        if (m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->CandidateListViewInternal_LayoutChangeNotification(hwnd, lpRect); });
        }
        return hr;
    }

    IFACEMETHODIMP CandidateListViewInternal_LayoutDestroyNotification()
    {
        HRESULT hr = S_OK;
        if (m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->CandidateListViewInternal_LayoutDestroyNotification(); });
        }
        return hr;
    }

    IFACEMETHODIMP CandidateListViewInternal_OnSetThreadFocus()
    {
        HRESULT hr = S_OK;
        if (m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->CandidateListViewInternal_OnSetThreadFocus(); });
        }
        return hr;
    }

    IFACEMETHODIMP CandidateListViewInternal_OnKillThreadFocus()
    {
        HRESULT hr = S_OK;
        if (m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->CandidateListViewInternal_OnKillThreadFocus(); });
        }
        return hr;
    }

    IFACEMETHODIMP CandidateListViewInternal_EndCandidateList()
    {
        HRESULT hr = S_OK;
        if (m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->CandidateListViewInternal_EndCandidateList(); });
        }
        return hr;
    }

private:
    void EnsureInitialized()
    {
        if (!m_threadTaskRunner)
        {
            m_threadTaskRunner = std::make_unique<ThreadTaskRunner>();
        }
        if (!m_environment)
        {
            m_notification = std::make_shared<TaskRunner>();
            THROW_IF_FAILED(wrl::MakeAndInitialize<SingletonProcessorEnvironment>(&m_environment, m_notification));
        }
        if (!m_engine)
        {
            m_threadTaskRunner->RunOnThread([this]() {
                m_engine = wil::CoCreateInstance<ITextInputProcessor>(WindowsImeLib::g_processorFactory->GetConstantProvider()->ServerCLSID(), CLSCTX_LOCAL_SERVER);
            });
        }
    }

    bool CallbackFromMesageWindow(WPARAM /*wParam*/, LPARAM /*lParam*/)
    {
        return false;
    }

private:
    std::unique_ptr<ThreadTaskRunner> m_threadTaskRunner;
    std::shared_ptr<TaskRunner> m_notification;

    wil::com_ptr<ITextInputProcessor> m_engine;
    wil::com_ptr<ITextInputEnvironment> m_environment;
};

wil::com_ptr<ITextInputProcessor> CreateSingletonProcessorBridge()
{
    wil::com_ptr<SingletonProcessorBridge> bridgeImp;
    THROW_IF_FAILED(wrl::MakeAndInitialize<SingletonProcessorBridge>(&bridgeImp));
    return bridgeImp.query<ITextInputProcessor>();
}


class SingletonProcessorHost :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        ITextInputProcessor,
                                        Microsoft::WRL::FtmBase>,
    public WindowsImeLib::ITextInputFramework,
    public ICandidateListViewOwner
{
public:
    SingletonProcessorHost()
    {
        m_candidateListView = s_candidateListView.lock();
        if (!m_candidateListView)
        {
            m_candidateListView = std::make_shared<CandidateListView>(this);
            s_candidateListView = m_candidateListView;
        }
        m_candidateListViewInternal = m_candidateListView;

        m_compositionBufferProxy = s_compositionBufferProxy.lock();
        if (!m_compositionBufferProxy)
        {
            m_compositionBufferProxy = std::make_shared<CompositionBufferProxy>();
            s_compositionBufferProxy = m_compositionBufferProxy;
        }

        m_processor = s_processor.lock();
        if (!m_processor)
        {
            m_processor = WindowsImeLib::g_processorFactory->CreateCompositionProcessorEngine(m_compositionBufferProxy, m_candidateListView);
            s_processor = m_processor;
            m_processor->Initialize();
        }
    }

    ~SingletonProcessorHost() {}

private:
    IFACEMETHODIMP Acivate(_In_ ITextInputEnvironment* /*environment*/) override
    {
        return S_OK;
    }

    IFACEMETHODIMP Deacivate(void) override
    {
        return S_OK;
    }

    IFACEMETHODIMP SetFocus(BOOL /*isGotten*/) override
    {
        // m_processor->SetFocus(!!isGotten);
        return S_OK;
    }

    IFACEMETHODIMP UpdateCustomState(LPCSTR customStateJson) override
    {
        m_processor->UpdateCustomState(std::string(customStateJson));
        return S_OK;
    }

    // ICompositionProcessorEngine
    IFACEMETHODIMP OnKeyEvent(DWORD wParam, DWORD lParam, WCHAR wch, UINT vkPackSource,
                        BOOL isKbdDisabled, DWORD modifiers, DWORD uniqueModifiers, BOOL isTest, BOOL isDown,
                        _Outptr_ BSTR* result, _Out_ BOOL *pIsEaten) override
    {
        m_compositionBufferProxy->m_jsonCmdArray.clear();

        m_processor->OnKeyEvent(static_cast<WPARAM>(wParam),  static_cast<LPARAM>(lParam), pIsEaten, wch, vkPackSource,
                                                !!isKbdDisabled, modifiers, uniqueModifiers, !!isTest, !!isDown);

        nlohmann::json jsonComposition;
        jsonComposition["cmds"] = m_compositionBufferProxy->m_jsonCmdArray;
        nlohmann::json jsonResult;
        jsonResult["composition"] = jsonComposition;
        const auto resultText = jsonResult.dump();

        *result = SysAllocStringLen(reinterpret_cast<const wchar_t*>(resultText.c_str()), static_cast<UINT>((resultText.length() + 1) / 2));
        return S_OK;
    }

    IFACEMETHODIMP GetCandidateList(BOOL isIncrementalWordSearch, BOOL isWildcardSearch, BSTR* /*encodedCandidateList*/) override
    {
        std::vector<shared_wstring> candidateList;
        m_processor->GetCandidateList(candidateList, isIncrementalWordSearch, isWildcardSearch);
        return S_OK;
    }

    IFACEMETHODIMP FinalizeCandidateList() override
    {
        m_processor->FinalizeCandidateList();
        return S_OK;
    }

    IFACEMETHODIMP _DeleteCandidateList() override
    {
        m_processor->_DeleteCandidateList();
        return S_OK;
    }

    // ICandidateListViewInternal
    IFACEMETHODIMP CandidateListViewInternal_LayoutChangeNotification(HWND hwnd, RECT *lpRect) override
    {
        m_cachedDocumentWindow = hwnd;
        m_cachedCompositionRect = *lpRect;
        m_candidateListViewInternal->_LayoutChangeNotification(hwnd, lpRect);
        return S_OK;
    }

    IFACEMETHODIMP CandidateListViewInternal_LayoutDestroyNotification() override
    {
        m_candidateListViewInternal->_LayoutDestroyNotification();
        return S_OK;
    }

    IFACEMETHODIMP CandidateListViewInternal_OnSetThreadFocus() override
    {
        return m_candidateListViewInternal->OnSetThreadFocus();
    }

    IFACEMETHODIMP CandidateListViewInternal_OnKillThreadFocus() override
    {
        return m_candidateListViewInternal->OnKillThreadFocus();
    }

    IFACEMETHODIMP CandidateListViewInternal_EndCandidateList() override
    {
        m_candidateListViewInternal->_EndCandidateList();
        return S_OK;
    }

    // WindowsImeLib::ITextInputFramework
    void Test() override
    {
    }

    // ICandidateListViewOwner
    HRESULT _GetLastTextExt(_Out_ HWND* documentWindow, _Out_ RECT *lpRect) override
    {
        *documentWindow = m_cachedDocumentWindow;
        *lpRect = m_cachedCompositionRect;
        return S_OK;
    }
    BOOL _IsStoreAppMode() { return FALSE; } // TODO
    void NotifyFinalizeCandidateList() { } // TODO
    wil::com_ptr<ITfThreadMgr> _GetThreadMgr() { return wil::com_ptr<ITfThreadMgr>(); } // do not support

private:
    static inline std::weak_ptr<WindowsImeLib::ICompositionProcessorEngine> s_processor;
    static inline std::weak_ptr<CandidateListView> s_candidateListView;
    static inline std::weak_ptr<CompositionBufferProxy> s_compositionBufferProxy;

    std::shared_ptr<WindowsImeLib::ICompositionProcessorEngine> m_processor;
    std::shared_ptr<ICandidateListViewInternal> m_candidateListViewInternal;
    std::shared_ptr<CandidateListView> m_candidateListView;
    std::shared_ptr<CompositionBufferProxy> m_compositionBufferProxy;

    HWND m_cachedDocumentWindow;
    RECT m_cachedCompositionRect;
};

HRESULT CreateSingletonProcessorHost(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj) try
{
    RETURN_HR_IF(CLASS_E_NOAGGREGATION, pUnkOuter != nullptr);
    wil::com_ptr<SingletonProcessorHost> engineHost;
    RETURN_IF_FAILED(wrl::MakeAndInitialize<SingletonProcessorHost>(&engineHost));
    RETURN_IF_FAILED(engineHost->QueryInterface(riid, ppvObj));
    return S_OK;
}
CATCH_RETURN()
