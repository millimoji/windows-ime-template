// Copyright (c) millimoji@gmail.com
#include "pch.h"
#include "PlatformService.h"

struct ConsoleConnection : public AutoDestructor, public std::enable_shared_from_this<ConsoleConnection> {
    ConsoleConnection() {
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();
        }

        freopen_s(&m_pfIn, "CONIN$", "r", stdin);
        freopen_s(&m_pfOut, "CONOUT$", "w", stdout);
        freopen_s(&m_pfErr, "CONOUT$", "w", stderr);
    }

    ~ConsoleConnection() {
        fclose(m_pfIn);
        fclose(m_pfOut);
        fclose(m_pfErr);
        FreeConsole();
    }

    FILE* m_pfIn = {};
    FILE* m_pfOut = {};
    FILE* m_pfErr = {};
};

struct PlatformServiceImpl : public PlatformService, std::enable_shared_from_this<PlatformServiceImpl> {
    
    // Windows dedicated IF
    void ComlessCoCreateInstance(const wchar_t* fileName, HMODULE* moduleHandle, const CLSID& clsid, const IID& iid, void** ppv) override {
        const HMODULE h = LoadLibraryEx(fileName, nullptr, 0);
        THROW_LAST_ERROR_IF(h == nullptr);

        void* rawProcAddr = GetProcAddress(h, "DllGetClassObject");
        THROW_LAST_ERROR_IF(rawProcAddr == nullptr);

        wil::com_ptr<IClassFactory> classFactory;
        THROW_IF_FAILED(reinterpret_cast<decltype(DllGetClassObject)*>(rawProcAddr)(clsid, IID_PPV_ARGS(&classFactory)));
        THROW_IF_FAILED(classFactory->CreateInstance(nullptr, iid, ppv));
        *moduleHandle = h;
    }

    // Pure C++ services
    std::wstring GetThisModulePath() override {
        HMODULE hModule = {};
        THROW_IF_WIN32_BOOL_FALSE(GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(PlatformService::GetInstance), &hModule));
        wchar_t moduleName[MAX_PATH] = {};
        THROW_LAST_ERROR_IF(GetModuleFileName(hModule, moduleName, ARRAYSIZE(moduleName)) == 0);
        return std::wstring(moduleName);
    }
    std::tuple<std::wstring, std::wstring> SplitPathAndFile(const wchar_t* fileName) override {
        wchar_t fullPathName[MAX_PATH];
        wchar_t* filePart;
        THROW_LAST_ERROR_IF(GetFullPathName(fileName, ARRAYSIZE(fullPathName), fullPathName, &filePart) == 0);
        return std::make_tuple(std::wstring(fullPathName, filePart - fullPathName), std::wstring(filePart));
    }
    std::shared_ptr<AutoDestructor> StartConsole() override {
        return std::make_shared<ConsoleConnection>();
    }
    std::vector<std::wstring> CommandLine() override {
        int argc = 0;
        const auto rawArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
        const auto releaseRawArgv = wil::scope_exit([&] { LocalFree(rawArgv); });

        std::vector<std::wstring> argv;
        argv.reserve(argc);
        for (int i = 0; i < argc; ++i) {
            argv.emplace_back(rawArgv[i]);
        }
        return argv;
    }
};

std::shared_ptr<PlatformService> PlatformService::GetInstance() {
    static std::weak_ptr<PlatformServiceImpl> s_singleton;
    if (const auto existingPtr= s_singleton.lock()) {
        return existingPtr;
    }
    const auto newPtr = std::make_shared<PlatformServiceImpl>();
    s_singleton = newPtr;
    return newPtr;
}
