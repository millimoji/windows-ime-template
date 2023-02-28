// (C) 2023 millimoji@gmail.com
#pragma once

struct OnnxRuntimeHelper
{
    virtual void Initialize(const std::wstring_view modelFile) noexcept = 0;
    virtual std::tuple<int64_t, float> GetPrediction(const std::vector<int64_t>& tokens) = 0;

    virtual ~OnnxRuntimeHelper() {};
    static std::shared_ptr<OnnxRuntimeHelper> CreateInstance() noexcept;
};
