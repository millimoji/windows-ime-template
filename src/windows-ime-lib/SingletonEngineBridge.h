#pragma once

struct SingletonEngineBridge
{
    static std::shared_ptr<SingletonEngineBridge> CreateSingletonEngineBridge();

    virtual ~SingletonEngineBridge() {}

    virtual std::shared_ptr<const std::wstring> CallTestMethod(const std::wstring_view param) = 0;
};
