// (C) 2023 millimoji@gmail.com
#include "pch.h"
#include "OnnxRuntimeHelper.h"
#include "MemAlignedTensor.h"

namespace winrt {
    using namespace ::winrt::Windows::AI::MachineLearning;
    using namespace ::winrt::Windows::Foundation::Collections;
    using namespace ::winrt::Windows::Storage;
}

namespace Ribbon::Prediction
{

struct OnnxRuntimeHelperImpl : public OnnxRuntimeHelper, public std::enable_shared_from_this<OnnxRuntimeHelperImpl>
{
private:
    const wchar_t* inInputIds = L"input_ids";
    const wchar_t* inAttentionMask = L"attention_mask";
    const wchar_t* outLogits = L"logits";

private:
    void Initialize(const std::wstring_view modelFile) noexcept override try {
        m_modelFileName = modelFile;
    }
    catch (...) {}

    std::tuple<int64_t, float> GetPrediction(const std::vector<int64_t>& tokens) noexcept override try {
        EnsureInitialized();

        const auto& deviceKind = winrt::LearningModelDeviceKind::Cpu;
        const auto& session = winrt::LearningModelSession{ m_model, winrt::LearningModelDevice(deviceKind) };
        const auto& binding = winrt::LearningModelBinding{ session };

        const auto tokenSize = static_cast<int64_t>(tokens.size());
        std::vector<int64_t> attensionMask(tokens.size(), 1LL);

        const auto& inputIdsTensor = winrt::TensorInt64Bit::CreateFromArray({ 1, tokenSize }, tokens);
        binding.Bind(inInputIds, inputIdsTensor);

        const auto& attenstionTensor = winrt::TensorInt64Bit::CreateFromArray({ 1, tokenSize }, attensionMask);
        binding.Bind(inAttentionMask, attenstionTensor);

        const auto& results = session.Evaluate(binding, L"correlationId");

        const auto& resultOutput = results.Outputs().Lookup(outLogits).as<winrt::TensorFloat>();
        const auto& outputShape = resultOutput.Shape();
        if (outputShape.Size() != 3) throw std::runtime_error("unexpected shape");

        MemAlignedTensor logits(1, outputShape.GetAt(2), nullptr);
        const auto targetVectorOffset = (outputShape.GetAt(1) - 1) * outputShape.GetAt(2);
        resultOutput.GetAsVectorView().GetMany(static_cast<uint32_t>(targetVectorOffset), logits.GetArrayView());

        return logits.GetIndexFromLogits();
    }
    catch (...) { return std::make_tuple(-1LL, 0.0f); }

private:
    void EnsureInitialized() {
        m_model = winrt::LearningModel::LoadFromFilePath(m_modelFileName);
    }

private:
    std::wstring m_modelFileName;
    winrt::LearningModel m_model{ nullptr };
};

} // namespace Ribbon::Prediction

/* static */
std::shared_ptr<OnnxRuntimeHelper> OnnxRuntimeHelper::CreateInstance() noexcept try
{
    return std::make_shared<Ribbon::Prediction::OnnxRuntimeHelperImpl>();
}
catch (...) { return std::shared_ptr<OnnxRuntimeHelper>(); }
