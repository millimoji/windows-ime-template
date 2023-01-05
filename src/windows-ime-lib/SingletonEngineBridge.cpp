#include "Private.h"
#include "Globals.h"
#include "SingletonEngineBridge.h"
#include "SingletonEngineHost.h"
#include "ThreadTaskRunner.h"

namespace wrl
{
    using namespace Microsoft::WRL;
}

class SingletonEngineOwner :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IDispatch,
    Microsoft::WRL::FtmBase>
{
public:
    SingletonEngineOwner() {}
    virtual ~SingletonEngineOwner() {}

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


struct SingletonEngineBridgeImpl : public std::enable_shared_from_this<SingletonEngineBridgeImpl>, public SingletonEngineBridge
{
    SingletonEngineBridgeImpl()
    {
    }
    ~SingletonEngineBridgeImpl()
    {
        if (m_engine && m_threadTaskRunner)
        {
            m_threadTaskRunner->RunOnThread([this]() { m_engine.reset(); });
        }
    }

    std::shared_ptr<const std::wstring> CallTestMethod(const std::wstring_view param) override
    {
        EnsureInitialized();
        std::shared_ptr<const std::wstring> result;
        m_threadTaskRunner->RunOnThread([&]()
            {
                result = InvokeIDispatch_String_String(m_engine.get(), DISPID_TEST_METHOD, param);
            });
        return result;
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
            THROW_IF_FAILED(wrl::MakeAndInitialize<SingletonEngineOwner>(&m_owner, m_notification));
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

std::shared_ptr<SingletonEngineBridge> SingletonEngineBridge::CreateSingletonEngineBridge()
{
    const auto singletonEngineBridge = std::make_shared<SingletonEngineBridgeImpl>();
    return std::static_pointer_cast<SingletonEngineBridge>(singletonEngineBridge);
}
