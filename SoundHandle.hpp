/*=============================================================================
  Copyright (C) 2020 yumetodo <yume-wikijp@live.jp>
  Distributed under the Boost Software License, Version 1.0.
  (See https://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#pragma once
#include <DxLib.h>
#include <string_view>
#include <algorithm>
class SoundHandle {
private:
	int handle_;
	constexpr SoundHandle(int h) noexcept : handle_(h) {}
	static constexpr int invalid_handle = -1;
public:
	constexpr SoundHandle() noexcept : handle_(invalid_handle) {}
	SoundHandle(const SoundHandle&) = delete;
	SoundHandle(SoundHandle&& o) noexcept : handle_(o.handle_) {
		o.handle_ = invalid_handle;
	}
	SoundHandle& operator=(const SoundHandle&) = delete;
	SoundHandle& operator=(SoundHandle&& o) noexcept {
		this->handle_ = o.handle_;
		o.handle_ = invalid_handle;
		return *this;
	}
	~SoundHandle() noexcept {
		Dispose();
	}
	void Dispose() noexcept {
		if (invalid_handle != this->handle_) {
			DeleteSoundMem(this->handle_);
			this->handle_ = invalid_handle;
		}
	}
	int get() const noexcept { return handle_; }
	SoundHandle Duplicate() const noexcept { return DxLib::DuplicateSoundMem(this->handle_); }
	bool check()  const noexcept { return (DxLib::CheckSoundMem(handle_) == TRUE); }
	bool play(const int& type, const int& flag = 1) const noexcept { return (PlaySoundMem(handle_, type, flag) == 0); }
	bool stop() const noexcept { return (StopSoundMem(handle_) == 0); }
	bool vol(const int& vol) const noexcept { return (ChangeVolumeSoundMem(std::clamp<int>(vol, 0, 255), handle_) == 0); }
	bool SetPosition(const VECTOR_ref& pos) const noexcept { return (Set3DPositionSoundMem(pos.get(), handle_) == 0); }
	bool Radius(const float& radius) const noexcept { return (Set3DRadiusSoundMem(radius, handle_) == 0); }
	void play_3D(const VECTOR_ref& pos, const float& radius) {
		SetPosition(pos);
		Radius(radius);
		play(DX_PLAYTYPE_BACK, TRUE);
	}
	static SoundHandle Load(std::basic_string_view<TCHAR> FileName, const int& BufferNum = 3) noexcept {
		return { DxLib::LoadSoundMemWithStrLen(FileName.data(), FileName.length(), BufferNum) };
	}
};
