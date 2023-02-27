// (C) 2023 millimoji@gmail.com
#pragma once

struct SentencePieceHelper
{
    virtual void Initialize(const std::wstring_view modelFile) noexcept = 0;
    virtual std::vector<int64_t> Encode64(const std::wstring_view sourceText) noexcept = 0;
    virtual std::wstring Decode64(const std::vector<int64_t>& tokens) noexcept = 0;

    virtual int unk_id() noexcept = 0;
	virtual int bos_id() noexcept = 0;
	virtual int eos_id() noexcept = 0;
    virtual int pad_id() noexcept = 0;

    virtual ~SentencePieceHelper() {};
    static std::shared_ptr<SentencePieceHelper> CreateInstance() noexcept;
};
