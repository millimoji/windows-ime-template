// Copyright (c) millimoji@gmail.com
#pragma once

struct AutoDestructor {
    virtual ~AutoDestructor() {}
};

struct PlatformService
{
    // Windows dedicated IF
    virtual void ComlessCoCreateInstance(const wchar_t* fileName, HMODULE* moduleHandle, const CLSID& clsid, const IID& iid, void** ppv) = 0;

    // Pure C++ services
    virtual std::wstring GetThisModulePath() = 0;
    virtual std::tuple<std::wstring, std::wstring> SplitPathAndFile(const wchar_t* fileName) = 0;
    virtual std::shared_ptr<AutoDestructor> StartConsole() = 0;
    virtual std::vector<std::wstring> CommandLine() = 0;

    static std::shared_ptr<PlatformService> GetInstance();
    virtual ~PlatformService() {}
};
