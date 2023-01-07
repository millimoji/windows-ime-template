#include "Private.h"
#include "Globals.h"
#include "SingletonProcessor.h"
#include "ThreadTaskRunner.h"

namespace wrl
{
    using namespace Microsoft::WRL;
}





class SingletonProcessorOwner :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch,
    Microsoft::WRL::FtmBase>
{
public:
    SingletonProcessorOwner() {}
    virtual ~SingletonProcessorOwner() {}

    HRESULT RuntimeClassInitialize(const std::shared_ptr<TaskRunner>& taskRunner)
    {
        m_taskRunner = taskRunner;
        return S_OK;
    }

private:
    IFACEMETHODIMP GetTypeInfoCount(UINT*) noexcept override { return E_NOTIMPL; }
    IFACEMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo**) noexcept override { return E_NOTIMPL; }
    IFACEMETHODIMP GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) noexcept override { return E_NOTIMPL; }

    IFACEMETHODIMP Invoke(DISPID /*dispIdMember*/, REFIID, LCID, WORD, DISPPARAMS* /*pDispParams*/, VARIANT* /*pVarResult*/, EXCEPINFO*, UINT*) noexcept override
    {
        return S_OK;
    }

    std::shared_ptr<TaskRunner> m_taskRunner;
};


struct SingletonProcessorBridge : public std::enable_shared_from_this<SingletonProcessorBridge>, public WindowsImeLib::ITextInputProcessor
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

    std::wstring TestMethod(const std::wstring_view src) override
    {
        EnsureInitialized();
        std::wstring result;
        m_threadTaskRunner->RunOnThread([&]()
            {
                result = InvokeIDispatch_WString_WString(m_engine.get(), DISPID_TEST_METHOD, src);
            });
        return result;
    }

    void SetFocus(bool isGotten) override
    {
        EnsureInitialized();
        nlohmann::json json;
        json["isGotten"] = isGotten;
        m_threadTaskRunner->RunOnThread([&]()
            {
                InvokeIDispatch_String_Void(m_engine.get(), DISPID_SET_FOCUS, json.dump());
            });
    }

private:
    void EnsureInitialized()
    {
        if (!m_threadTaskRunner)
        {
            m_threadTaskRunner = std::make_unique<ThreadTaskRunner>();
        }
        if (!m_engine)
        {
            m_threadTaskRunner->RunOnThread([this]() {
                    m_engine = wil::CoCreateInstance<IDispatch>(WindowsImeLib::g_processorFactory->GetConstantProvider()->ServerCLSID(), CLSCTX_LOCAL_SERVER); });
        }
        if (!m_owner)
        {
            m_notification = std::make_shared<TaskRunner>();
            THROW_IF_FAILED(wrl::MakeAndInitialize<SingletonProcessorOwner>(&m_owner, m_notification));
        }
    }

    bool CallbackFromMesageWindow(WPARAM /*wParam*/, LPARAM /*lParam*/)
    {
        return false;
    }

private:
    std::unique_ptr<ThreadTaskRunner> m_threadTaskRunner;
    std::shared_ptr<TaskRunner> m_notification;

    wil::com_ptr<IDispatch> m_engine;
    wil::com_ptr<IDispatch> m_owner;
};

std::shared_ptr<WindowsImeLib::ITextInputProcessor> CreateSingletonProcessorBridge()
{
    const auto bridge = std::make_shared<SingletonProcessorBridge>();
    return std::static_pointer_cast<WindowsImeLib::ITextInputProcessor>(bridge);
}






class SingletonProcessorHost :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        IDispatch,
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
    // IDispatch
    IFACEMETHODIMP GetTypeInfoCount(UINT*) noexcept override { return E_NOTIMPL; }
    IFACEMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo**) noexcept override { return E_NOTIMPL; }
    IFACEMETHODIMP GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) noexcept override { return E_NOTIMPL; }

    IFACEMETHODIMP Invoke(DISPID dispIdMember, REFIID, LCID, WORD, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO*, UINT*) noexcept override try
    {
	    switch (dispIdMember)
	    {
	    case DISPID_TEST_METHOD:
	        {
				THROW_HR_IF(E_UNEXPECTED, pDispParams->cArgs != 1);
	            const auto arg = VARIANTtoWString(pDispParams->rgvarg[0]);
	            const auto returnStr = arg + L"-suffix";
	            WStringToVARIANT(returnStr, *pVarResult);
	        }
	        return S_OK;

	    case DISPID_SET_FOCUS:
	        {
	            THROW_HR_IF(E_UNEXPECTED, pDispParams->cArgs != 1);
	            const auto arg = VARIANTtoString(pDispParams->rgvarg[0]);
	            const auto json = nlohmann::json::parse(arg);
                m_processor->SetFocus(json["isGotten"].get<bool>());
	        }
	        return S_OK;
	    }
	    return S_OK;
	}
	CATCH_RETURN()

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
