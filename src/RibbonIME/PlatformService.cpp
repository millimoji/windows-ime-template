// Copyright (c) millimoji@gmail.com
#include "pch.h"
#include <Knownfolders.h>
#include <shobjidl_core.h>
#include <pathcch.h>
#include "PlatformService.h"

#pragma comment(lib, "pathcch.lib")

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

struct ConfigAccessorImpl : public ConfigAccessor, public std::enable_shared_from_this<ConfigAccessorImpl> {
    ConfigAccessorImpl(const std::shared_ptr<nlohmann::json>& json) : m_json(json) {
    }
    std::shared_ptr<ConfigAccessor> get(std::string_view key) override {
        const auto value = m_json->at(key);
        THROW_HR_IF(E_INVALIDARG, !value.is_object());
        const auto jsonPtr = std::make_shared<nlohmann::json>(value);
        return std::make_shared<ConfigAccessorImpl>(jsonPtr);
    }
    std::wstring getText(std::string_view key) override {
        const auto value = m_json->at(key);
        THROW_HR_IF(E_INVALIDARG, !value.is_string());
        const auto utf8 = value.get<std::string>();
        const auto platformService = PlatformService::GetInstance();
        return platformService->ToUtf16(utf8);
    }
    std::string getTextUtf8(std::string_view key) override {
        const auto value = m_json->at(key);
        THROW_HR_IF(E_INVALIDARG, !value.is_string());
        return value.get<std::string>();
    }
    std::wstring GetPath(std::string_view key) override {
        const auto srcData = getText(key);
        if (srcData.starts_with(m_binDirMacro)) {
            const auto platformService = PlatformService::GetInstance();
            const auto [pathPart, filePart] = platformService->SplitPathAndFile(platformService->GetThisModulePath().c_str());
            const auto realPath = platformService->CombinePath(pathPart.c_str(), srcData.c_str() + m_binDirMacro.length());
            return realPath;
        }
        if (srcData.starts_with(m_userDirMacro)) {
            const auto platformService = PlatformService::GetInstance();
            const auto userPath = platformService->GetAndEnsureUserPath();
            const auto realPath = platformService->CombinePath(userPath.c_str(), srcData.c_str() + m_userDirMacro.length());
            return realPath;
        }
        return srcData;
    }
    int getInt(std::string_view key) override {
        THROW_HR_IF(E_INVALIDARG, m_json->contains(key));
        if (m_json->contains(key)) {
            const auto value = m_json->at(key);
            if (value.is_number()) {
                return value.get<int>();
            }
        }
        return -1;
    }

    static inline const auto m_binDirMacro = std::wstring(L"${BINDIR}");
    static inline const auto m_userDirMacro = std::wstring(L"${USERDIR}");

    std::shared_ptr<nlohmann::json> m_json;
};

struct MemoryMappedFileImpl : public MemoryMappedFile, std::enable_shared_from_this<MemoryMappedFileImpl> {
    MemoryMappedFileImpl(const wchar_t* fileName) {
        wil::unique_handle hFile(CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0));
        THROW_LAST_ERROR_IF(hFile.get() == INVALID_HANDLE_VALUE);
        LARGE_INTEGER fileSize;
        THROW_LAST_ERROR_IF(!GetFileSizeEx(hFile.get(), &fileSize));
        wil::unique_handle hMemMap(CreateFileMapping(hFile.get(), 0, PAGE_READONLY, 0, 0, nullptr));
        THROW_LAST_ERROR_IF(hMemMap.get() == NULL);
        const auto mappedAddr = MapViewOfFile(hMemMap.get(), FILE_MAP_READ, 0, 0, 0);
        THROW_LAST_ERROR_IF(mappedAddr == NULL);

        m_hFile = std::move(hFile);
        m_hMemMap = std::move(hMemMap);
        m_mappedAddr = mappedAddr;
        m_fileSize = fileSize.QuadPart;
    }
    ~MemoryMappedFileImpl() {
        UnmapViewOfFile(m_mappedAddr);

    }
    const void* Addr() override { return m_mappedAddr; }
    size_t Size() override { return m_fileSize; }

    wil::unique_handle m_hFile;
    wil::unique_handle m_hMemMap;
    const void* m_mappedAddr = nullptr;
    size_t m_fileSize;
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
        return GetModulePath(GetThisModuleHanlde());
    }
    std::wstring GetAndEnsureUserPath() override {
        const auto knownFolderManager = wil::CoCreateInstance<IKnownFolderManager>(CLSID_KnownFolderManager);
        wil::com_ptr<IKnownFolder> kownFolder;
        THROW_IF_FAILED(knownFolderManager->GetFolder(FOLDERID_RoamingAppData, &kownFolder));
        wil::unique_cotaskmem_string taskMemPath;
        THROW_IF_FAILED(kownFolder->GetPath(0, taskMemPath.addressof()));
        const auto resultPath = CombinePath(taskMemPath.get(), L"RibbonIME");
        // Ensure folder exist
        CreateDirectory(resultPath.c_str(), nullptr);
        return resultPath;
    }
    std::tuple<std::wstring, std::wstring> SplitPathAndFile(const wchar_t* fileName) override {
        wchar_t fullPathName[MAX_PATH];
        wchar_t* filePart;
        THROW_LAST_ERROR_IF(GetFullPathName(fileName, ARRAYSIZE(fullPathName), fullPathName, &filePart) == 0);
        return std::make_tuple(std::wstring(fullPathName, filePart - fullPathName), std::wstring(filePart));
    }
    std::wstring CombinePath(const wchar_t* pathLeft, const wchar_t* pathRight) override {
        wchar_t combined[MAX_PATH * 2];
        THROW_IF_FAILED(PathCchCombine(combined, ARRAYSIZE(combined), pathLeft, pathRight));
        return std::wstring(combined);
    }
    std::wstring ToUtf16(std::string_view src) override {
        const auto requiredSize = MultiByteToWideChar(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), nullptr, 0);
        std::wstring utf16(requiredSize, L' ');
        MultiByteToWideChar(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), utf16.data(), requiredSize);
        return utf16;
    }
    std::string ToUtf8(std::wstring_view src) override {
        const auto requiredSize = WideCharToMultiByte(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), nullptr, 0, nullptr, nullptr);
        std::string utf8(requiredSize, ' ');
        WideCharToMultiByte(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), utf8.data(), requiredSize, nullptr, nullptr);
        return utf8;
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
    std::shared_ptr<MemoryMappedFile> OpenFile(const wchar_t* fileName) override {
        return std::make_shared<MemoryMappedFileImpl>(fileName);
    }
    std::shared_ptr<ConfigAccessor> GetConfig(bool reload) override {
        if (!reload && m_config) {
            return m_config;
        }

        auto root = nlohmann::json::object();
        try {
            const auto inBinRsrc = LoadResource(L"ribbon-config.json", RT_RCDATA);
            const auto binRsrcJson = nlohmann::json::parse(std::string_view(&inBinRsrc[0], inBinRsrc.size()));
            root.merge_patch(binRsrcJson);
        } catch (...) {}
        try {
            const auto userPath = GetAndEnsureUserPath();
            const auto userConfig = CombinePath(userPath.c_str(), L"ribbon-config.json");
            const auto memFile = OpenFile(userConfig.c_str());
            const auto userJson = nlohmann::json::parse(std::string_view(reinterpret_cast<const char*>(memFile->Addr()), memFile->Size()));
            root.merge_patch(userJson);
        } catch (...) {}
        try {
            // current directoy
            const auto memFile = OpenFile(L"ribbon-config.json");
            const auto userJson = nlohmann::json::parse(std::string_view(reinterpret_cast<const char*>(memFile->Addr()), memFile->Size()));
            root.merge_patch(userJson);
        } catch (...) {}

        return std::make_shared<ConfigAccessorImpl>(std::make_shared<nlohmann::json>(std::move(root)));
    }

private:
    HMODULE GetThisModuleHanlde() {
        HMODULE hModule = {};
        THROW_IF_WIN32_BOOL_FALSE(GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(PlatformService::GetInstance), &hModule));
        return hModule;
    }
    std::wstring GetModulePath(HMODULE hModule) {
        wchar_t moduleName[MAX_PATH] = {};
        THROW_LAST_ERROR_IF(GetModuleFileName(hModule, moduleName, ARRAYSIZE(moduleName)) == 0);
        return std::wstring(moduleName);
    }
    std::vector<char> LoadResource(LPCWSTR key, LPCWSTR resType) {
        const auto hModule = GetThisModuleHanlde();
        const auto hrsrc = FindResource(hModule, key, resType);
        THROW_LAST_ERROR_IF(hrsrc == NULL);
        const auto resSize = SizeofResource(hModule, hrsrc);
        const auto hglobal = ::LoadResource(hModule, hrsrc);
        THROW_LAST_ERROR_IF(!hglobal);
        const auto resAddr = LockResource(hglobal);
        auto loadBuf = std::vector<char>(resSize);
        memcpy(&loadBuf[0], resAddr, resSize);
        return loadBuf;
    }

    std::shared_ptr<ConfigAccessor> m_config;
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
