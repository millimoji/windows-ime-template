// (C) 2023 millimoji@gmail.com
#include "pch.h"
#include "OnnxRuntimeHelper.h"
#include "OnnxProto/onnx.proto3.pb.h"
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
    const wchar_t* lastHiddenState = L"last_hidden_state";

private:
    void Initialize(const std::wstring_view modelFile) noexcept override try {
        m_modelFileName = modelFile;
    }
    catch (...) {}

    std::tuple<int64_t, float> GetPrediction(const std::vector<int64_t>& tokens) noexcept override try {
        const auto startTime = std::chrono::system_clock::now();
        EnsureInitialized();

        const auto tokenSize = static_cast<int64_t>(tokens.size());
        std::vector<int64_t> attensionMask(tokens.size(), 1LL);

        m_binding.Clear();
        const auto& inputIdsTensor = winrt::TensorInt64Bit::CreateFromArray({ 1, tokenSize }, tokens);
        m_binding.Bind(inInputIds, inputIdsTensor);

        const auto& attenstionTensor = winrt::TensorInt64Bit::CreateFromArray({ 1, tokenSize }, attensionMask);
        m_binding.Bind(inAttentionMask, attenstionTensor);

        const auto& results = m_session.Evaluate(m_binding, L"correlationId");
        const auto evaluateEndTime = std::chrono::system_clock::now();

        const auto& resultOutput = results.Outputs().Lookup(lastHiddenState).as<winrt::TensorFloat>();
        const auto& outputShape = resultOutput.Shape();
        if (outputShape.Size() != 3) throw std::runtime_error("unexpected shape");

        m_tokenVector.Reserve(1, outputShape.GetAt(2));
        const auto targetVectorOffset = (outputShape.GetAt(1) - 1) * outputShape.GetAt(2);
        resultOutput.GetAsVectorView().GetMany(static_cast<uint32_t>(targetVectorOffset), m_tokenVector.GetArrayView());

        m_tokenVector.SubstractPosition(m_positionEmbedding, static_cast<int>(outputShape.GetAt(1) - 1));
        const auto foundToken = m_tokenVector.FindToken(m_wordEmbedding);

        const auto foundTokenTime = std::chrono::system_clock::now();
        printf("Time: %lld, %lld\n",
            std::chrono::duration_cast<std::chrono::milliseconds>(evaluateEndTime - startTime).count(),
            std::chrono::duration_cast<std::chrono::milliseconds>(foundTokenTime - evaluateEndTime).count());

        return foundToken;
    }
    catch (...) { return std::make_tuple(-1LL, 0.0f); }

private:
    void EnsureInitialized() {
        if (!m_model) {
            const auto requiredSize = WideCharToMultiByte(CP_UTF8, 0, m_modelFileName.data(), static_cast<int>(m_modelFileName.length()), nullptr, 0, nullptr, nullptr);
            std::string utf8Filename(requiredSize, ' ');
            WideCharToMultiByte(CP_UTF8, 0, m_modelFileName.data(), static_cast<int>(m_modelFileName.length()), utf8Filename.data(), requiredSize, nullptr, nullptr);
            std::tie(m_wordEmbedding, m_positionEmbedding) = GetWeights(utf8Filename.c_str());

            m_model = winrt::LearningModel::LoadFromFilePath(m_modelFileName);
            const auto deviceKind = winrt::LearningModelDeviceKind::Cpu;
            m_session = winrt::LearningModelSession{ m_model, winrt::LearningModelDevice(deviceKind) };
            m_binding = winrt::LearningModelBinding{ m_session };
        }
    }

    MemAlignedTensor GetWeightSub(const ::onnx::TensorProto& initializer)
    {
        if (initializer.dims_size() != 2) {
            throw std::exception("Unsupported weight dims");
        }

        int row = static_cast<int>(initializer.dims(0));
        int column = static_cast<int>(initializer.dims(1));

        const auto rawData = initializer.raw_data();
        const float* floatArray = reinterpret_cast<const float*>(&rawData[0]);

        return MemAlignedTensor(row, column, floatArray);
    }

    std::tuple<MemAlignedTensor, MemAlignedTensor> GetWeights(const char* onnxFileName)
    {
        GOOGLE_PROTOBUF_VERIFY_VERSION;

        onnx::ModelProto model;

        std::fstream in(onnxFileName, std::ios::in | std::ios::binary);
        model.ParseFromIstream(&in);

        // IR_VERSION_2020_5_8 : IR VERSION 7 published on May 8, 2020
        if (model.ir_version() < ::onnx::IR_VERSION_2020_5_8) {
            throw std::exception("unsupported version");
        }

        MemAlignedTensor wte, wpe;

        const auto& graph = model.graph();

        const auto initializerSize = graph.initializer_size();
        for (int i = 0; i < initializerSize; i++) {
            const auto& initializer = graph.initializer(i);
            const auto& initializerName = initializer.name();
            if (initializerName == "wte.weight") {
                wte = GetWeightSub(initializer);
            }
            else if (initializerName == "wpe.weight") {
                wpe = GetWeightSub(initializer);
            }
        }
        return std::make_tuple(std::move(wte), std::move(wpe));
    }



private:
    std::wstring m_modelFileName;
    winrt::LearningModel m_model{ nullptr };
    winrt::LearningModelSession m_session{ nullptr };
    winrt::LearningModelBinding m_binding{ nullptr };

    MemAlignedTensor m_wordEmbedding;
    MemAlignedTensor m_positionEmbedding;
    MemAlignedTensor m_tokenVector;
};

} // namespace Ribbon::Prediction

/* static */
std::shared_ptr<OnnxRuntimeHelper> OnnxRuntimeHelper::CreateInstance() noexcept try
{
    return std::make_shared<Ribbon::Prediction::OnnxRuntimeHelperImpl>();
}
catch (...) { return std::shared_ptr<OnnxRuntimeHelper>(); }
