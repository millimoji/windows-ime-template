// (C) 2023 millimoji@gmaill.com
#include "pch.h"
#include "../PlatformService.h"
#include "../PredictionEngine.h"
#include "../SentencePiece/SentencePiece_h.h"

namespace Ribbon::Windows
{
    struct InternalHelpers {
        static std::tuple<std::wstring, std::wstring> SplitPathAndFile(const wchar_t* fileName) {
            wchar_t fullPathName[MAX_PATH];
            wchar_t* filePart;
            THROW_LAST_ERROR_IF(GetFullPathName(fileName, ARRAYSIZE(fullPathName), fullPathName, &filePart) == 0);
            return std::make_tuple(std::wstring(fullPathName, filePart - fullPathName), std::wstring(filePart));
        }

        static std::wstring GetThisModulePath() {
            HMODULE hModule = {};
            THROW_IF_WIN32_BOOL_FALSE(GetModuleHandleEx(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(GetThisModulePath), &hModule));
            wchar_t moduleName[MAX_PATH] = {};
            THROW_LAST_ERROR_IF(GetModuleFileName(hModule, moduleName, ARRAYSIZE(moduleName)) == 0);
            return std::wstring(moduleName);
        }

        static void ComlessCoCreateInstance(const wchar_t* fileName, HMODULE* moduleHandle, const CLSID& clsid, const IID& iid, void** ppv) {
            const HMODULE h = LoadLibraryEx(fileName, nullptr, 0);
            THROW_LAST_ERROR_IF(h == nullptr);

            void* rawProcAddr = GetProcAddress(h, "DllGetClassObject");
            THROW_LAST_ERROR_IF(rawProcAddr == nullptr);

            wil::com_ptr<IClassFactory> classFactory;
            THROW_IF_FAILED(reinterpret_cast<decltype(DllGetClassObject)*>(rawProcAddr)(clsid, IID_PPV_ARGS(&classFactory)));
            THROW_IF_FAILED(classFactory->CreateInstance(nullptr, iid, ppv));
            *moduleHandle = h;
        }
    };

    struct SentencePieceHelperImpl : public SentencePieceHelper, std::enable_shared_from_this<SentencePieceHelperImpl> {
        ~SentencePieceHelperImpl() {
            m_processor.reset();
            FreeLibrary(m_sentencePieceHelper);
        }
        void Initialize(const wchar_t* modelFile) noexcept override {
            const auto myPath = InternalHelpers::GetThisModulePath();
            const auto pathAndFilename = InternalHelpers::SplitPathAndFile(myPath.c_str());
            const auto internalSentencePiece = std::get<0>(pathAndFilename) + L"Internal.SentencePiece.dll";
            InternalHelpers::ComlessCoCreateInstance(internalSentencePiece.c_str(),
                &m_sentencePieceHelper, __uuidof(SentencePieceProcessor), IID_PPV_ARGS(&m_processor));
            THROW_IF_FAILED(m_processor->Load(modelFile));

            THROW_IF_FAILED(m_processor->unk_id(&m_unk_id));
            THROW_IF_FAILED(m_processor->bos_id(&m_bos_id));
            THROW_IF_FAILED(m_processor->eos_id(&m_eos_id));
            THROW_IF_FAILED(m_processor->pad_id(&m_pad_id));
        }
        std::vector<int64_t> Encode64(const wchar_t* sourceText) noexcept override {
            UINT tokenLen = {};
            int64_t* tokenRawPtr = {};
            THROW_IF_FAILED(m_processor->Encode64(sourceText, &tokenLen, &tokenRawPtr));
            wil::unique_cotaskmem_ptr<__int64> tokenPtr(tokenRawPtr);
            auto resultBuf = std::vector<int64_t>(tokenLen, 0LL);
            memcpy(resultBuf.data(), tokenRawPtr, sizeof(int64_t) * tokenLen);
            return resultBuf;
        }
        std::wstring Decode64(const std::vector<int64_t>& tokens) noexcept override {
            wil::unique_bstr bstrText;
            THROW_IF_FAILED(m_processor->Decode64(static_cast<UINT>(tokens.size()), const_cast<int64_t*>(tokens.data()), &bstrText));
            return std::wstring(bstrText.get());
        }
        int unk_id() noexcept override { return m_unk_id; }
        int bos_id() noexcept override { return m_bos_id; }
        int eos_id() noexcept override { return m_eos_id; }
        int pad_id() noexcept override { return m_pad_id; }
    private:
        HMODULE m_sentencePieceHelper = {};
        std::wstring m_modelFileName;
        wil::com_ptr<ISentencePieceProcessor> m_processor;
        int m_unk_id;
        int m_bos_id;
        int m_eos_id;
        int m_pad_id;
    };

    struct PlatformServiceImpl : public PlatformService, std::enable_shared_from_this<PlatformServiceImpl> {
        std::shared_ptr<SentencePieceHelper> CreateSentencePieceHelper() override {
            return std::make_shared<SentencePieceHelperImpl>();
        }
    };


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

std::shared_ptr<PlatformService> PlatformService::GetInstance() {
    static std::weak_ptr<Ribbon::Windows::PlatformServiceImpl> s_singleton;
    if (const auto existingPtr= s_singleton.lock()) {
        return existingPtr;
    }
    const auto newPtr = std::make_shared<Ribbon::Windows::PlatformServiceImpl>();
    s_singleton = newPtr;
    return newPtr;
}


extern "C" {
    STDAPI_(void) TestPrediction(HWND, HINSTANCE, LPSTR, int) {
        try {
            Ribbon::Windows::ConsoleConnection conCon(true);
            PredictionEngine::TestMain(conCon.CommandLine());
        }
        CATCH_LOG();
    }
} // extern "C"
