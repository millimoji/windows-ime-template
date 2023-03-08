// Copyright (c) millimoji@gmail.com
#pragma once

struct AutoDestructor {
    virtual ~AutoDestructor() {}
};

struct ConfigAccessor {
    virtual std::shared_ptr<ConfigAccessor> get(std::string_view) = 0;
    virtual std::wstring getText(std::string_view) = 0;
    virtual std::string getTextUtf8(std::string_view) = 0;
    virtual std::wstring GetPath(std::string_view key) = 0;
    virtual int getInt(std::string_view) = 0;
};

struct MemoryMappedFile {
    virtual const void* Addr() = 0;
    virtual size_t Size() = 0;
};

struct PlatformService
{
    // Windows dedicated IF
    virtual void ComlessCoCreateInstance(const wchar_t* fileName, HMODULE* moduleHandle, const CLSID& clsid, const IID& iid, void** ppv) = 0;

    // Pure C++ services
    virtual std::wstring GetThisModulePath() = 0;
    virtual std::wstring GetAndEnsureUserPath() = 0;
    virtual std::tuple<std::wstring, std::wstring> SplitPathAndFile(const wchar_t* fileName) = 0;
    virtual std::wstring CombinePath(const wchar_t* pathLeft, const wchar_t* pathRight) = 0;
    virtual std::wstring ToUtf16(std::string_view src) = 0;
    virtual std::string ToUtf8(std::wstring_view src) = 0;
    virtual std::shared_ptr<AutoDestructor> StartConsole() = 0;
    virtual std::vector<std::wstring> CommandLine() = 0;
    virtual std::shared_ptr<MemoryMappedFile> OpenFile(const wchar_t* fileName) = 0;
    virtual std::shared_ptr<ConfigAccessor> GetConfig(bool reload = false) = 0;

    static std::shared_ptr<PlatformService> GetInstance();
    virtual ~PlatformService() {}
};
