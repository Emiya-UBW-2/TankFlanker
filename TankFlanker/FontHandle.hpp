#pragma once
#include <DxLib.h>
#include <string_view>
class FontHandle {
private:
	int handle_;
	constexpr FontHandle(int h) noexcept : handle_(h) {}
	static constexpr int invalid_handle = -1;

public:
	constexpr FontHandle() noexcept : handle_(invalid_handle) {}
	FontHandle(const FontHandle&) = delete;
	FontHandle(FontHandle&& o) noexcept : handle_(o.handle_) {
		o.handle_ = invalid_handle;
	}
	FontHandle& operator=(const FontHandle&) = delete;
	FontHandle& operator=(FontHandle&& o) noexcept {
		this->handle_ = o.handle_;
		o.handle_ = invalid_handle;
		return *this;
	}
	~FontHandle() noexcept {
		if (-1 != this->handle_) {
			DeleteFontToHandle(this->handle_);
		}
	}
	void Dispose() noexcept {
		if (-1 != this->handle_) {
			DeleteFontToHandle(this->handle_);
			this->handle_ = -1;
		}
	}
	int get() const noexcept { return handle_; }
	int DrawString(int x, int y, std::basic_string_view<TCHAR> String, unsigned int Color, unsigned int EdgeColor = 0, bool VerticalFlag = false) const noexcept {
		return DxLib::DrawNStringToHandle(x, y, String.data(), String.size(), Color, this->handle_, EdgeColor, VerticalFlag);
	}

	template <typename... Args>
	int DrawStringFormat(int x, int y, unsigned int Color, std::string String, Args&&... args) const noexcept {
		return DxLib::DrawFormatStringToHandle(x, y, Color, this->handle_, String.c_str(), args...);
	}

	int GetDrawWidth(std::basic_string_view<TCHAR> String, bool VerticalFlag = false) const noexcept {
		return DxLib::GetDrawNStringWidthToHandle(String.data(), String.size(), this->handle_, VerticalFlag);
	}
	static FontHandle Create(std::basic_string_view<TCHAR> FontName, int Size, int Thick, int FontType = -1, int CharSet = -1, int EdgeSize = -1, bool Italic = false) noexcept {
		return { DxLib::CreateFontToHandleWithStrLen(FontName.data(), FontName.length(), Size, Thick, FontType, CharSet, EdgeSize, Italic) };
	}
	static FontHandle Create(int Size, int Thick, int FontType = -1, int CharSet = -1, int EdgeSize = -1, bool Italic = false) noexcept {
		return { DxLib::CreateFontToHandle(nullptr, Size, Thick, FontType, CharSet, EdgeSize, Italic) };
	}
};
