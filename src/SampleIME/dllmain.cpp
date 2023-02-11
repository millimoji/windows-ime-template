#include "pch.h"
#include "../WindowsImeLib.h"
#include "SampleIMEDefine.h"
#include "SampleIMEGlobals.h"
#include "CompositionProcessorEngine.h"

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
	extern std::shared_ptr<IWindowsIMEInProcClient> ProcessorFactory_CreateIMEInProcClient(IWindowsIMEInProcFramework* framework);

    class ProcessorFactory : public std::enable_shared_from_this<ProcessorFactory>, public IProcessorFactory
    {
    public:
        std::shared_ptr<ICompositionProcessorEngine> CreateCompositionProcessorEngine(
            const std::shared_ptr<IWindowsIMECompositionBuffer>& compositionBuffer,
            const std::shared_ptr<IWindowsIMECandidateListView>& candidateListView)
        {
            return std::make_shared<CompositionProcessorEngine>(compositionBuffer, candidateListView);
        }

        std::shared_ptr<IConstantProvider> GetConstantProvider() override
        {
            static std::shared_ptr<IConstantProvider> constantProvider = std::make_shared<Global::ConstantProvider>();
            return constantProvider;
        }

        std::shared_ptr<IWindowsIMEInProcClient> CreateIMEInProcClient(IWindowsIMEInProcFramework* framework) override
        {
            return ProcessorFactory_CreateIMEInProcClient(framework);
        }
    };

    std::shared_ptr<IProcessorFactory> g_processorFactory = std::static_pointer_cast<IProcessorFactory>(std::make_shared<ProcessorFactory>());
}
