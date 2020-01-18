#pragma once
#include "DxLib.h"
#include <EffekseerForDXLib.h>
#include <string_view>
class EffekseerEffectHandle;
/*
class Effekseer2DPlayingHandle {
private:
	int handle_;
	constexpr Effekseer2DPlayingHandle(int h) noexcept : handle_(h) {}
	friend EffekseerEffectHandle;
	static constexpr int invalid_handle = -1;

public:
	constexpr Effekseer2DPlayingHandle() noexcept : handle_(invalid_handle) {}
	Effekseer2DPlayingHandle(const Effekseer2DPlayingHandle&) = delete;
	Effekseer2DPlayingHandle(Effekseer2DPlayingHandle&& o) noexcept : handle_(o.handle_) {
		o.handle_ = invalid_handle;
	}
	Effekseer2DPlayingHandle& operator=(const Effekseer2DPlayingHandle&) = delete;
	Effekseer2DPlayingHandle& operator=(Effekseer2DPlayingHandle&& o) noexcept {
		this->handle_ = o.handle_;
		o.handle_ = invalid_handle;
		return *this;
	}
	~Effekseer2DPlayingHandle() noexcept {
		if (-1 != this->handle_) {
			this->Stop();
		}
	}
	//    @brief	2D表示のエフェクトが再生中か取得する。
	//    @return	再生中かどうか?
	//    @return	true:再生中、false:再生されていない、もしくは再生終了
	bool IsPlaying() const noexcept {
		return 0 == IsEffekseer2DEffectPlaying(this->handle_);
	}
	// @brief 2D表示のエフェクトを停止する。
	void Stop() const noexcept {
		StopEffekseer2DEffect(this->handle_);
	}
	//    @brief	再生中の2D表示のエフェクトの位置を設定する。
	//    @param	x	X座標
	//    @param	y	Y座標
	void SetPos(VECTOR_ref pos) const noexcept {
		SetPosPlayingEffekseer2DEffect(this->handle_, pos.x(), pos.y(), pos.z());
	}
	//    @brief	再生中の2D表示のエフェクトの角度を設定する。
	//    @param	x	X軸角度(ラジアン)
	//    @param	y	Y軸角度(ラジアン)
	//    @param	z	Z軸角度(ラジアン)
	//    @note
	//    回転の方向は時計回りである。
	//    回転の順番は Z軸回転 → X軸回転 → Y軸回転である。
	void SetRotation(float x, float y, float z) const noexcept {
		SetRotationPlayingEffekseer2DEffect(this->handle_, x, y, z);
	}
	//    @brief	再生中の2D表示のエフェクトの拡大率を設定する。
	//    @param	x	X方向拡大率
	//    @param	y	Y方向拡大率
	//    @param	z	Z方向拡大率
	void SetScale(float x, float y, float z) const noexcept {
		SetScalePlayingEffekseer2DEffect(this->handle_, x, y, z);
	}
	//    @brief	再生中の2D表示のエフェクトの再生速度を取得する。
	//    @return	再生速度
	float GetSpeed() const noexcept {
		return GetSpeedPlayingEffekseer2DEffect(this->handle_);
	}
	//    @brief	再生中の2D表示のエフェクトの再生速度を設定する。
	//    @param	speed	再生速度
	void SetSpeed(float speed) const noexcept {
		SetSpeedPlayingEffekseer2DEffect(this->handle_, speed);
	}
	// * @brief 再生中の2D表示のエフェクトの色を設定する。
	void SetColor(int r, int g, int b, int a) const noexcept {
		SetColorPlayingEffekseer2DEffect(this->handle_, r, g, b, a);
	}
	// * @brief ハンドルを破棄する
	void Dispose() noexcept {
		if (-1 != this->handle_) {
			this->Stop();
			this->handle_ = -1;
		}
	}
	//    @brief	指定されたEffekseerにより再生中の2Dエフェクトを描画する。
	//    @note
	//    特定のエフェクトのみを描画したい時に使用する。
	//    特定のエフェクトを描画する。
	//    DrawEffekseer2Dとは併用できない。
	void Draw() noexcept {
		DrawEffekseer2D_Draw(this->handle_);
	}
	[[deprecated]] int get() const noexcept { return this->handle_; }
};
*/
class Effekseer3DPlayingHandle {
private:
	int handle_;
	constexpr Effekseer3DPlayingHandle(int h) noexcept : handle_(h) {}
	friend EffekseerEffectHandle;
	static constexpr int invalid_handle = -1;

public:
	constexpr Effekseer3DPlayingHandle() noexcept : handle_(invalid_handle) {}
	Effekseer3DPlayingHandle(const Effekseer3DPlayingHandle&) = delete;
	Effekseer3DPlayingHandle(Effekseer3DPlayingHandle&& o) noexcept : handle_(o.handle_) {
		o.handle_ = invalid_handle;
	}
	Effekseer3DPlayingHandle& operator=(const Effekseer3DPlayingHandle&) = delete;
	Effekseer3DPlayingHandle& operator=(Effekseer3DPlayingHandle&& o) noexcept {
		this->handle_ = o.handle_;
		o.handle_ = invalid_handle;
		return *this;
	}
	~Effekseer3DPlayingHandle() noexcept {
		if (-1 != this->handle_) {
			this->Stop();
		}
	}
	/**
	    @brief	3D表示のエフェクトが再生中か取得する。
	    @return	再生中かどうか?
	    @return	true:再生中、false:再生されていない、もしくは再生終了
	*/
	bool IsPlaying() const noexcept {
		return 0 == IsEffekseer3DEffectPlaying(this->handle_);
	}
	/**
	 * @brief 3D表示のエフェクトを停止する。
	 */
	void Stop() const noexcept {
		StopEffekseer3DEffect(this->handle_);
	}
	/**
	    @brief	再生中の3D表示のエフェクトの位置を設定する。
	    @param	x	X座標
	    @param	y	Y座標
	*/
	void SetPos(VECTOR_ref pos) const noexcept {
		SetPosPlayingEffekseer3DEffect(this->handle_, pos.x(), pos.y(), pos.z());
	}
	/**
	    @brief	再生中の3D表示のエフェクトの角度を設定する。
	    @param	x	X軸角度(ラジアン)
	    @param	y	Y軸角度(ラジアン)
	    @param	z	Z軸角度(ラジアン)
	    @note
	    回転の方向は時計回りである。
	    回転の順番は Z軸回転 → X軸回転 → Y軸回転である。
	*/
	void SetRotation(float x, float y, float z) const noexcept {
		SetRotationPlayingEffekseer3DEffect(this->handle_, x, y, z);
	}
	/**
	    @brief	再生中の3D表示のエフェクトの拡大率を設定する。
	    @param	x	X方向拡大率
	    @param	y	Y方向拡大率
	    @param	z	Z方向拡大率
	*/
	void SetScale(float size) const noexcept {
		SetScalePlayingEffekseer3DEffect(this->handle_, size, size, size);
	}
	/**
	    @brief	再生中の3D表示のエフェクトの再生速度を取得する。
	    @return	再生速度
	*/
	float GetSpeed() const noexcept {
		return GetSpeedPlayingEffekseer3DEffect(this->handle_);
	}
	/**
	    @brief	再生中の3D表示のエフェクトの再生速度を設定する。
	    @param	speed	再生速度
	*/
	void SetSpeed(float speed) const noexcept {
		SetSpeedPlayingEffekseer3DEffect(this->handle_, speed);
	}
	/**
	 * @brief 再生中の3D表示のエフェクトの色を設定する。
	 */
	void SetColor(int r, int g, int b, int a) const noexcept {
		SetColorPlayingEffekseer3DEffect(this->handle_, r, g, b, a);
	}
	/**
	 * @brief ハンドルを破棄する
	 */
	void Dispose() noexcept {
		if (-1 != this->handle_) {
			this->Stop();
			this->handle_ = -1;
		}
	}
	/**
	    @brief	指定されたEffekseerにより再生中の3Dエフェクトを描画する。
	    @note
	    特定のエフェクトのみを描画したい時に使用する。
	    特定のエフェクトを描画する。
	    DrawEffekseer3Dとは併用できない。
	*/
	void Draw() noexcept {
		DrawEffekseer3D_Draw(this->handle_);
	}
	[[deprecated]] int get() const noexcept { return this->handle_; }
};

class EffekseerEffectHandle {
private:
	int handle_;
	constexpr EffekseerEffectHandle(int h) noexcept : handle_(h) {}
	static constexpr int invalid_handle = -1;

public:
	constexpr EffekseerEffectHandle() noexcept : handle_(invalid_handle) {}
	EffekseerEffectHandle(const EffekseerEffectHandle&) = delete;
	EffekseerEffectHandle(EffekseerEffectHandle&& o) noexcept : handle_(o.handle_) {
		o.handle_ = invalid_handle;
	}
	EffekseerEffectHandle& operator=(const EffekseerEffectHandle&) = delete;
	EffekseerEffectHandle& operator=(EffekseerEffectHandle&& o) noexcept {
		this->handle_ = o.handle_;
		o.handle_ = invalid_handle;
		return *this;
	}
	~EffekseerEffectHandle() noexcept {
		if (-1 != this->handle_) {
			DeleteEffekseerEffect(this->handle_);
		}
	}
	void Dispose() noexcept {
		if (-1 != this->handle_) {
			DeleteEffekseerEffect(this->handle_);
			this->handle_ = -1;
		}
	}
	/*
	//    @brief	メモリ上のEffekseerのエフェクトリソースを2D表示で再生する。
	//    @return	エフェクトのハンドル
	Effekseer2DPlayingHandle Play2D() const noexcept {
		return { PlayEffekseer2DEffect(this->handle_) };
	}
	*/
	/**
	    @brief	メモリ上のEffekseerのエフェクトリソースを3D表示で再生する。
	    @return	エフェクトのハンドル
	*/
	Effekseer3DPlayingHandle Play3D() const noexcept {
		return { PlayEffekseer3DEffect(this->handle_) };
	}
	[[deprecated]] int get() const noexcept { return this->handle_; }
	constexpr explicit operator bool() { return -1 != this->handle_; }
	// LoadEffekseerEffectはfileNameをstd::wstringに必ず格納する、オーバーロード必要
	static EffekseerEffectHandle load(const char* fileName, float magnification = 1.0f) noexcept { return { LoadEffekseerEffect(fileName, magnification) }; }
	static EffekseerEffectHandle load(const wchar_t* fileName, float magnification = 1.0f) noexcept { return { LoadEffekseerEffect(fileName, magnification) }; }
	static EffekseerEffectHandle load(const std::string& fileName, float magnification = 1.0f) noexcept { return load(fileName.c_str(), magnification); }
	static EffekseerEffectHandle load(const std::wstring& fileName, float magnification = 1.0f) noexcept { return load(fileName.c_str(), magnification); }
};
