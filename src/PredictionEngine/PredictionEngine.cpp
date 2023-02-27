// PredictionEngine.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "../PredictionEngine.h"
#include "SentencePieceHelper.h"
#include "OnnxRuntimeHelper.h"

namespace Ribbon::Prediction {

    constexpr auto c_tokenizerModel = L"C:\\TEMP\\rinna-xsmall-optimum\\spiece.model";
    constexpr auto c_onnxModel = L"C:\\TEMP\\rinna-xsmall-optimum\\model.onnx";

struct PredictionEngineImpl : public PredictionEngine, public std::enable_shared_from_this<PredictionEngineImpl>
{
private:
    std::wstring GetPredictionText(int tokenCount, std::wstring_view preceeding) noexcept override
    {
        EnsureInitialized();

        auto tokens = m_spiece->Encode64(preceeding);
        const auto initialTokenCount = tokens.size();
        for (int i = 0; i < tokenCount; ++i) {
            const auto nextTuple = m_onnx->GetPrediction(tokens);
            tokens.emplace_back(std::get<0>(nextTuple));
        }

        std::vector<int64_t> predicted;
        for (auto it = tokens.begin() + initialTokenCount; it != tokens.end(); ++it) {
            predicted.emplace_back(*it);
        }
        return m_spiece->Decode64(predicted);
    }

private:
    void EnsureInitialized() {
        if (!m_spiece) {
            m_spiece = SentencePieceHelper::CreateInstance();
            m_spiece->Initialize(c_tokenizerModel);
        }
        if (!m_onnx) {
            m_onnx = OnnxRuntimeHelper::CreateInstance();
            m_onnx->Initialize(c_onnxModel);
        }
    }


private:
    std::shared_ptr<SentencePieceHelper> m_spiece;
    std::shared_ptr<OnnxRuntimeHelper> m_onnx;
};

} // namespace Ribbon::Prediction

std::shared_ptr<PredictionEngine> PredictionEngine::CreateInstance() noexcept try {
    return std::make_shared<Ribbon::Prediction::PredictionEngineImpl>();
}
catch (...) {
    return std::shared_ptr<Ribbon::Prediction::PredictionEngineImpl>();
}

int PredictionEngine::TestMain(const std::vector<std::wstring>& args) {
    const auto predictionEngine = PredictionEngine::CreateInstance();
    const auto predictedText = predictionEngine->GetPredictionText(20, L"今年の冬はとても寒い");
    wprintf(L"Predicted: %s\n", predictedText.c_str());
    return 0;
}
