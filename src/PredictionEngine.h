// (C) 2023 millimoji@gmail.com
#pragma once

struct PredictionEngine
{
    virtual std::wstring GetPredictionText(int minTextLen, const wchar_t* preceeding) noexcept = 0;

    virtual ~PredictionEngine() {};
    static std::shared_ptr<PredictionEngine> CreateInstance() noexcept;
    static int TestMain(const std::vector<std::wstring>& args);
};