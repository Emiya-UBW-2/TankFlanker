#pragma once
#include <DxLib.h>
#include <string_view>
class MV1Handle {
private:
    int handle_;
    constexpr MV1Handle(int h) noexcept : handle_(h) {}
public:
    constexpr MV1Handle() noexcept : handle_(-1) {}
    MV1Handle(const MV1Handle&) = delete;
    MV1Handle(MV1Handle&&) = default;
    MV1Handle& operator=(const MV1Handle&) = delete;
    MV1Handle& operator=(MV1Handle&&) = default;
    ~MV1Handle() noexcept
    {
        if (-1 != this->handle_) {
            MV1DeleteModel(this->handle_);
        }
    }
    void Dispose() noexcept {
        if (-1 != this->handle_) {
            MV1DeleteModel(this->handle_);
            this->handle_ = -1;
        }
    }
    int get() const noexcept { return handle_; }
    MV1Handle DuplicateModel() const noexcept { return DxLib::MV1DuplicateModel(this->handle_); }
    static MV1Handle LoadModel(std::basic_string_view<TCHAR> FileName) noexcept { return DxLib::MV1LoadModelWithStrLen(FileName.data(), FileName.length()); }
};
