// PredictionEngine.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "../../PredictionEngine.h"
#include "../PlatformService.h"
#include "OnnxRuntimeHelper.h"
#include "SentencePieceHelper.h"

namespace Ribbon::Prediction {

struct PredictionEngineImpl : public PredictionEngine, public std::enable_shared_from_this<PredictionEngineImpl>
{
private:
    std::wstring GetPredictionText(int tokenCount, const wchar_t* preceeding) noexcept override
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
        if (!m_spiece || !m_onnx) {
            const auto platformSvc = PlatformService::GetInstance();
            const auto config = platformSvc->GetConfig();
            const auto configPrediction = config->at("prediction");
            const auto spieceModel = platformSvc->DecodePath(platformSvc->ToUtf16(configPrediction["spieceModel"].get<std::string>()).c_str());
            const auto onnxModel = platformSvc->DecodePath(platformSvc->ToUtf16(configPrediction["onnxModel"].get<std::string>()).c_str());

            if (!m_spiece) {
                m_spiece = SentencePieceHelper::CreateInstance();
                m_spiece->Initialize(spieceModel.c_str());
            }
            if (!m_onnx) {
                m_onnx = OnnxRuntimeHelper::CreateInstance();
                m_onnx->Initialize(onnxModel.c_str());
            }
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
    // const auto sampleText = L"今年の冬はとても寒い";
    // const auto sampleText = L"吾輩は猫である。名前はまだない。";
    // const auto sampleText = L"今日の東京はとても寒かったけど、そちらの天気はどうですか？";
    const auto sampleText = L"今日の東京はとても寒かったけど、そちらの天気はどうですか？";
    const auto predictedText = predictionEngine->GetPredictionText(10, sampleText);
    wprintf(L"Predicted: %s\n", predictedText.c_str());
    return 0;
}
