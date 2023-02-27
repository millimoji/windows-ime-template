// Copyright (c) millimoji@gmail.com
#pragma once

struct SentencePieceHelper
{
    virtual void Initialize(const wchar_t* modelFile) = 0;
    virtual std::vector<int64_t> Encode64(const wchar_t* sourceText) = 0;
    virtual std::wstring Decode64(const std::vector<int64_t>& tokens) = 0;

    virtual int unk_id() = 0;
    virtual int bos_id() = 0;
    virtual int eos_id() = 0;
    virtual int pad_id() = 0;

    virtual ~SentencePieceHelper() {};
};


struct PlatformService
{
    virtual std::shared_ptr<SentencePieceHelper> CreateSentencePieceHelper() = 0;


    static std::shared_ptr<PlatformService> GetInstance();
    virtual ~PlatformService() {}
};
