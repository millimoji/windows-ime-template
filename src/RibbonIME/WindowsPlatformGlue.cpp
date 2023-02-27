// (C) 2023 millimoji@gmaill.com
#include "pch.h"
#include "../PredictionEngine.h"


namespace Ribbon::Windows
{
    class ConsoleConnection
    {
        std::vector<std::wstring> m_argv;
        bool m_isForce = {};
        bool m_isConsoleAttached = {};
        FILE* m_pfIn = {};
        FILE* m_pfOut = {};
        FILE* m_pfErr = {};
    public:
        ConsoleConnection(bool _isForce) : m_isForce(_isForce)
        {
            if (AttachConsole(ATTACH_PARENT_PROCESS)) {
                m_isConsoleAttached = true;
            }
            else if (m_isForce) {
                AllocConsole();
                m_isConsoleAttached = true;
            }
            if (m_isConsoleAttached) {
                freopen_s(&m_pfIn, "CONIN$", "r", stdin);
                freopen_s(&m_pfOut, "CONOUT$", "w", stdout);
                freopen_s(&m_pfErr, "CONOUT$", "w", stderr);
                printf("\n"); // force output line break
            }

            int argc = 0;
            const auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
            const auto releaseArgv = wil::scope_exit([&] { LocalFree(argv); });

            m_argv.clear();
            m_argv.reserve(argc);
            for (int i = 0; i < argc; ++i) {
                m_argv.emplace_back(argv[i]);
            }
        }
        ~ConsoleConnection()
        {
            if (m_isConsoleAttached) {
                fclose(m_pfIn);
                fclose(m_pfOut);
                fclose(m_pfErr);
                FreeConsole();
            }
        }
        const std::vector<std::wstring>& CommandLine() {
            return m_argv;
        }
    private:
    };

} // namespace Ribbon::Windows

extern "C" {
    STDAPI_(void) TestPrediction(HWND, HINSTANCE, LPSTR, int)
    {
        try {
            Ribbon::Windows::ConsoleConnection conCon(true);
            PredictionEngine::TestMain(conCon.CommandLine());
        }
        CATCH_LOG();
    }

} // extern "C"
