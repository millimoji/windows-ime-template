#include "pch.h"
#include "../WindowsImeLib.h"
#include "SampleIMEDefine.h"
#include "SampleIMEGlobals.h"
#include "ProcessorEngine.h"

#pragma comment(lib, "RuntimeObject.lib")

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return WindowsImeLib::DllMain(hModule, ul_reason_for_call, lpReserved);
}

_Check_return_
STDAPI  DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ void** ppv)
{
    return WindowsImeLib::DllGetClassObject(rclsid, riid, ppv);
}

__control_entrypoint(DllExport)
STDAPI DllCanUnloadNow(void)
{
    return WindowsImeLib::DllCanUnloadNow();
}

STDAPI DllUnregisterServer(void)
{
    return WindowsImeLib::DllUnregisterServer();
}

STDAPI DllRegisterServer(void)
{
    return WindowsImeLib::DllRegisterServer(TEXTSERVICE_ICON_INDEX);
}

namespace WindowsImeLib
{

    class ProcessorFactory : public std::enable_shared_from_this<ProcessorFactory>, public IProcessorFactory
    {
    public:
        std::shared_ptr<ICompositionProcessorEngine> CreateCompositionProcessorEngine(const std::weak_ptr<ICompositionProcessorEngineOwner>& owner) override
        {
            return std::make_shared<CompositionProcessorEngine>(owner);
        }

        std::shared_ptr<IConstantProvider> GetConstantProvider() override
        {
            static std::shared_ptr<IConstantProvider> constantProvider = std::make_shared<Global::ConstantProvider>();
            return constantProvider;
        }

        std::shared_ptr<ITextInputProcessor> CreateTextInputProcessor(ITextInputFramework* framework) override
        {
            auto textInputProcessor = std::make_shared<SampleIMEProcessor>(framework);
            return std::static_pointer_cast<ITextInputProcessor>(textInputProcessor);
        }

        std::shared_ptr<IWindowsIMEInprocClient> CreateIMEInprocClient(IWindowsIMEInprocFramework* framework) override
        {
            auto inprocClient = std::make_shared<SampleIMEInprocClient>(framework);
            return std::static_pointer_cast<IWindowsIMEInprocClient>(inprocClient);
        }
    };

    std::shared_ptr<IProcessorFactory> g_processorFactory = std::static_pointer_cast<IProcessorFactory>(std::make_shared<ProcessorFactory>());
}
