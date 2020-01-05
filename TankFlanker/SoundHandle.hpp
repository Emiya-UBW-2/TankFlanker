#pragma once
#include <DxLib.h>
#include <string_view>
class SoundHandle {
private:
    int handle_;
    constexpr SoundHandle(int h) noexcept : handle_(h) {}
public:
    constexpr SoundHandle() noexcept : handle_(-1) {}
    SoundHandle(const SoundHandle&) = delete;
    SoundHandle(SoundHandle&&) = default;
    SoundHandle& operator=(const SoundHandle&) = delete;
    SoundHandle& operator=(SoundHandle&&) = default;
    ~SoundHandle() noexcept
    {
        if (-1 != this->handle_) {
            DeleteSoundMem(this->handle_);
        }
    }
    void Dispose() noexcept {
        if (-1 != this->handle_) {
            DeleteSoundMem(this->handle_);
            this->handle_ = -1;
        }
    }
    int get() const noexcept { return handle_; }
    static SoundHandle Load(std::basic_string_view<TCHAR> FileName, int BufferNum = 3) noexcept {
        return DxLib::LoadSoundMemWithStrLen(FileName.data(), FileName.length(), BufferNum);
    }
};
