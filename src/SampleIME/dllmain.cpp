#include <windows.h>
#include "../WindowsImeLib.h"
#include "SampleIMEDefine.h"
#include "ProcessorEngine.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
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
    return WindowsImeLib::DllUnregisterServer(TEXTSERVICE_LANGID);
}

STDAPI DllRegisterServer(void)
{
    return WindowsImeLib::DllRegisterServer(TEXTSERVICE_LANGID, TEXTSERVICE_ICON_INDEX);
}

namespace WindowsImeLib
{

    class ProcessorFactory : public std::enable_shared_from_this<ProcessorFactory>, public IProcessorFactory
    {
    public:
        std::shared_ptr<ICompositionProcessorEngine> CreateCompositionProcessorEngine() override
        {
            return std::make_shared<CompositionProcessorEngine>();
        }
    };

    std::shared_ptr<IProcessorFactory> g_processorFactory = std::static_pointer_cast<IProcessorFactory>(std::make_shared<ProcessorFactory>());
}
