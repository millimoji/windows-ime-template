#include "Private.h"
#include "Globals.h"
#include "SingletonProcessor.h"
#include "ThreadTaskRunner.h"

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

    IFACEMETHODIMP TestMethod(_In_ BSTR src, _Outptr_ BSTR* result) override
    {
        EnsureInitialized();
        HRESULT hr = S_OK;
        m_threadTaskRunner->RunOnThread([&]() { hr = m_engine->TestMethod(src, result); });
        return hr;
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
    public WindowsImeLib::ITextInputFramework
{
public:
    SingletonProcessorHost()
    {
        m_processor = WindowsImeLib::g_processorFactory->CreateTextInputProcessor(this);
    }

    ~SingletonProcessorHost() {}

private:
    IFACEMETHODIMP TestMethod(_In_ BSTR src, _Outptr_ BSTR* result)
    {
        const auto resultVal = m_processor->TestMethod(src);
        *result = SysAllocStringLen(resultVal.c_str(), static_cast<UINT>(resultVal.length()));
        return S_OK;
    }

    IFACEMETHODIMP Acivate(_In_ ITextInputEnvironment* /*environment*/) override
    {
        return S_OK;
    }

    IFACEMETHODIMP Deacivate(void) override
    {
        return S_OK;
    }

    IFACEMETHODIMP SetFocus(BOOL isGotten) override
    {
        m_processor->SetFocus(!!isGotten);
        return S_OK;
    }

    IFACEMETHODIMP UpdateCustomState(LPCSTR customStateJson) override
    {
        m_processor->UpdateCustomState(customStateJson);
        return S_OK;
    }

    // WindowsImeLib::ITextInputFramework
    void Test() override
    {
    }

private:
    std::shared_ptr<WindowsImeLib::ITextInputProcessor> m_processor;
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
