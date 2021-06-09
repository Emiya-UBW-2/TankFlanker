#pragma once
#define NOMINMAX
//
#define _USE_OPENVR_
//VR
#ifdef _USE_OPENVR_
#include <openvr.h>
#define BUTTON_TRIGGER vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Trigger)
#define BUTTON_SIDE vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_Grip)
#define BUTTON_TOUCHPAD vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Touchpad)
#define BUTTON_TOPBUTTON vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_ApplicationMenu)
#define BUTTON_TOPBUTTON_B vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_IndexController_B)
#define DEVICE_HMD vr::TrackedDeviceClass_HMD
#define DEVICE_CONTROLLER vr::TrackedDeviceClass_Controller
#define DEVICE_TRACKER vr::TrackedDeviceClass_GenericTracker
#define DEVICE_BASESTATION vr::TrackedDeviceClass_TrackingReference
#endif // _USE_OPENVR_
//共通
#include <array>
#include <list>
#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <array>
#include <vector>
#include <D3D11.h>
#include <memory>
#include <functional>
//DXLIB
#include "DxLib.h"
#include "EffekseerForDXLib.h"
//追加
#include "DXLib_vec.hpp"
#include "SoundHandle.hpp"
#include "GraphHandle.hpp"
#include "FontHandle.hpp"
#include "MV1ModelHandle.hpp"
#include "EffekseerEffectHandle.hpp"

inline float FPS{ 60.f };					/*FPS(共通)*/
constexpr float M_GR{ -9.8f };				/*重力加速度*/
//デスクトップサイズ
inline const int32_t deskx{ (int32_t)(GetSystemMetrics(SM_CXSCREEN)) };
inline const int32_t desky{ (int32_t)(GetSystemMetrics(SM_CYSCREEN)) };//DPIを反映する
inline const int32_t deskx_real{ 1920 };
inline const int32_t desky_real{ 1080 };//DPIを反映しない
//矩形と点との判定
#define in2_(mx, my, x1, y1, x2, y2) (mx >= x1 && mx <= x2 && my >= y1 && my <= y2)
//その他
 //角度からラジアンに
template <typename T>
static float deg2rad(T p1) {
	return float(p1) * DX_PI_F / 180.f;
}
//ラジアンから角度に
template <typename T>
static float rad2deg(T p1) {
	return float(p1) * 180.f / DX_PI_F;
}
//文字列から数値を取り出す
class getparams {
public:
	static const std::string getright(const char* p1) {
		std::string tempname = p1;
		return tempname.substr(tempname.find('=') + 1);
	}
public:
	static auto _str(int p1) {
		char mstr[256];
		FileRead_gets(mstr, 256, p1);
		return getright(mstr);
	}
	static auto get_str(int p1) {
		char mstr[256];
		FileRead_gets(mstr, 256, p1);
		return std::string(mstr);
	}
	static const long int _int(int p1) {
		char mstr[256];
		FileRead_gets(mstr, 256, p1);
		return std::stoi(getright(mstr));
	}
	static const long int _long(int p1) {
		char mstr[256];
		FileRead_gets(mstr, 256, p1);
		return std::stol(getright(mstr));
	}
	static const unsigned long int _ulong(int p2) {
		char mstr[256];
		FileRead_gets(mstr, 256, p2);
		return std::stoul(getright(mstr));
	}
	static const float _float(int p1) {
		char mstr[256];
		FileRead_gets(mstr, 256, p1);
		return std::stof(getright(mstr));
	}
	static const bool _bool(int p1) {
		char mstr[256];
		FileRead_gets(mstr, 256, p1);
		return (getright(mstr).find("true") != std::string::npos);
	}
};
//イージング
void easing_set(float* first, const float& aim, const float& ratio) {
	if (ratio == 0.f) {
		*first = aim;
	}
	else {
		if (aim != 0.f) {
			*first += (aim - *first) * (1.f - powf(ratio, 60.f / FPS));
		}
		else {
			*first *= powf(ratio, 60.f / FPS);
		}
	}
};
void easing_set(VECTOR_ref* first, const VECTOR_ref& aim, const float& ratio) {
	if (ratio == 0.f) {
		*first = aim;
	}
	else {
		*first += (VECTOR_ref(aim) - *first) * (1.f - powf(ratio, 60.f / FPS));
	}
};
//cosAを出す
float getcos_tri(const float& a, const float& b, const float& c) {
	if (b + c > a && c + a > b && a + b > c) {
		return std::clamp((b * b + c * c - a * a) / (2.f * b*c), -1.f, 1.f);
	}
	return 1.f;
}
//起動
void createProcess(char* szCmd, DWORD flag, bool fWait) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	CreateProcess(NULL, szCmd, NULL, NULL, FALSE, flag, NULL, NULL, &si, &pi);
	if (fWait) WaitForSingleObject(pi.hProcess, INFINITE);	//終了を待つ.
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}
//自身を多重起動
void start_me(void) {
	char Path[MAX_PATH];
	// EXEのあるフォルダのパスを取得
	::GetModuleFileName(NULL, Path, MAX_PATH);
	createProcess(Path, SW_HIDE, false);
}
/*wstringをstringへ変換*/
static std::string WStringToString(std::wstring oWString) {
	// wstring → SJIS
	int iBufferSize = WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, (char *)NULL, 0, NULL, NULL);
	// バッファの取得
	CHAR* cpMultiByte = new CHAR[iBufferSize];
	// wstring → SJIS
	WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, cpMultiByte, iBufferSize, NULL, NULL);
	// stringの生成
	std::string oRet(cpMultiByte, cpMultiByte + iBufferSize - 1);
	// バッファの破棄
	delete[] cpMultiByte;
	// 変換結果を返す
	return(oRet);
}
/*stringをwstringへ変換する*/
static std::wstring StringToWString(std::string& oString) {
	// SJIS → wstring
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, (wchar_t*)NULL, 0);
	// バッファの取得
	wchar_t* cpUCS2 = new wchar_t[iBufferSize];
	// SJIS → wstring
	MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2, iBufferSize);
	// stringの生成
	std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);
	// バッファの破棄
	delete[] cpUCS2;
	// 変換結果を返す
	return(oRet);
}
//エフェクト
class EffectS {
public:
	bool flug{ false };				 /**/
	size_t id{ 0 };					 /**/
	Effekseer3DPlayingHandle handle; /**/
	VECTOR_ref pos;					 /**/
	VECTOR_ref nor;					 /**/
	float scale{ 1.f };				 /**/

	void set(VECTOR_ref pos_, VECTOR_ref nor_, float scale_ = 1.f) {
		this->flug = true;
		this->pos = pos_;
		this->nor = nor_;
		this->scale = scale_;
	}
	void put(const EffekseerEffectHandle& handle_) {
		if (this->flug) {
			if (this->handle.IsPlaying()) {
				this->handle.Stop();
			}
			this->handle = handle_.Play3D();
			this->handle.SetPos(this->pos);
			this->handle.SetRotation(atan2(this->nor.y(), std::hypot(this->nor.x(), this->nor.z())), atan2(-this->nor.x(), -this->nor.z()), 0);
			this->handle.SetScale(this->scale);
			this->flug = false;
		}
	}
	void set_loop(const EffekseerEffectHandle& handle_) {
		this->handle = handle_.Play3D();
	}
	void put_loop(VECTOR_ref pos_, VECTOR_ref nor_, float scale_ = 1.f) {
		this->flug = true;
		this->pos = pos_;
		this->nor = nor_;
		this->scale = scale_;
		this->handle.SetPos(this->pos);
		//this->handle.SetRotation(atan2(this->nor.y(), std::hypot(this->nor.x(), this->nor.z())), atan2(-this->nor.x(), -this->nor.z()), 0);
		this->handle.SetScale(this->scale);
	}
};
//
struct cam_info {
	VECTOR_ref campos, camvec, camup;	//カメラ
	float fov{ deg2rad(90) };			//カメラ
	float near_{ 0.1f }, far_{ 10.f };	//ニアファー

	void set_cam_pos(const VECTOR_ref& cam_pos, const VECTOR_ref& cam_vec, const VECTOR_ref& cam_up) {
		campos = cam_pos;
		camvec = cam_vec;
		camup = cam_up;	//カメラ
	}

	void set_cam_info(const float& cam_fov_, const float& cam_near_, const float& cam_far_) {
		fov = cam_fov_;			//カメラ
		near_ = cam_near_, far_ = cam_far_;	//ニアファー
	}
};

class DXDraw {
public:
	int disp_x{ deskx };
	int disp_y{ desky };
	bool use_vr{ true };
private:
	bool use_shadow{ true };			/*影描画*/
	int shadow_near{ 0 };				/*近影*/
	int shadow_nearfar{ 0 };			/*近影*/
	int shadow_far{ 0 };				/*遠影*/
	size_t shadow_size{ 10 };			/*影サイズ*/
	bool use_pixellighting{ true };	    /**/
	bool use_vsync{ false };		    /*垂直同期*/
	float frate{ 60.f };			    /*フレームレート*/
#ifdef _USE_OPENVR_
public:
	struct system_VR {
		int id{ 0 };
		VECTOR_ref pos;
		VECTOR_ref xvec{ VECTOR_ref::vget(1, 0, 0) };
		VECTOR_ref yvec{ VECTOR_ref::vget(0, 1, 0) };
		VECTOR_ref zvec{ VECTOR_ref::vget(0, 0, 1) };
		std::array<uint64_t, 2> on{ 0 };
		VECTOR_ref touch;
		char num{ 0 };
		vr::ETrackedDeviceClass type{ vr::TrackedDeviceClass_Invalid };
		bool turn{ false }, now{ false };
	};
	std::vector<char> tracker_num;
private:
	//VR
	vr::IVRSystem* m_pHMD{ nullptr };
	vr::EVRInitError eError{ vr::VRInitError_None };
	std::vector<system_VR> ctrl;							/*HMD,controller*/
	char deviceall{ 0 };
	char hmd_num{ -1 };
	char hand1_num{ -1 };
	char hand2_num{ -1 };
	bool oldv{ false };
	bool start_c{ true };
	VECTOR_ref rec_HMD;
	MATRIX_ref rec_HMDmat;
#endif // _USE_OPENVR_
public:
	std::array<GraphHandle, 3> outScreen;	//スクリーンバッファ
private:
	std::vector<EffekseerEffectHandle> effHndle; /*エフェクトリソース*/
public:
	size_t get_eff_size() { return effHndle.size(); }
	EffekseerEffectHandle& get_effHandle(int p1) noexcept { return effHndle[p1]; }
	const EffekseerEffectHandle& get_effHandle(int p1) const noexcept { return effHndle[p1]; }

	DXDraw(const char* title, const float& fps = 60.f, const bool& use_VR = false, const bool& use_SHADOW = true, const bool& use_VSYNC = false) {
		use_vr = false;
#ifdef _USE_OPENVR_
		if (use_VR) {
			use_vr = true;
			eError = vr::VRInitError_None;
			m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
			if (eError != vr::VRInitError_None) {
				m_pHMD = nullptr;
				use_vr = false;
			}
		}
#endif // _USE_OPENVR_

		this->use_shadow = use_SHADOW;
		this->shadow_size = 13;
		if (use_vr) {
			uint32_t t_x = 1080;
			uint32_t t_y = 1200;
			//m_pHMD->GetRecommendedRenderTargetSize(&t_x,&t_y);
			this->disp_x = int(t_x) * 2;
			this->disp_y = int(t_y) * 2;
		}
		else {
			this->disp_x = deskx;
			this->disp_y = desky;
		}

		this->frate = fps;
		SetOutApplicationLogValidFlag(false ? TRUE : FALSE);				/*log*/
		SetMainWindowText(title);											/*タイトル*/
		ChangeWindowMode(TRUE);												/*窓表示*/
		SetUseDirect3DVersion(DX_DIRECT3D_11);								/*directX ver*/
		if (use_vr) {
			SetGraphMode(this->disp_x, this->disp_y, 32);						/*解像度*/
		}
		else {
			SetGraphMode(this->disp_x, this->disp_y, 32);						/*解像度*/
		}
		if (use_vr) {
			use_vsync = false;
		}
		else {
			use_vsync = use_VSYNC;
		}
		SetUseDirectInputFlag(TRUE);										/**/
		SetDirectInputMouseMode(TRUE);										/**/
		SetWindowSizeChangeEnableFlag(FALSE, FALSE);						/*ウインドウサイズを手動不可、ウインドウサイズに合わせて拡大もしないようにする*/
		SetUsePixelLighting(use_pixellighting ? TRUE : FALSE);				/*ピクセルシェーダの使用*/
		SetFullSceneAntiAliasingMode(4, 2);									/*アンチエイリアス*/
		SetEnableXAudioFlag(TRUE);											/**/
		Set3DSoundOneMetre(1.0f);											/**/
		SetWaitVSyncFlag(use_vsync ? TRUE : FALSE);							/*垂直同期*/
		DxLib_Init();														/**/
		Effekseer_Init(8000);												/*Effekseer*/
		SetSysCommandOffFlag(TRUE);											/**/
		SetChangeScreenModeGraphicsSystemResetFlag(FALSE);					/*Effekseer*/
		Effekseer_SetGraphicsDeviceLostCallbackFunctions();					/*Effekseer*/
		SetAlwaysRunFlag(TRUE);												/*background*/
		SetUseZBuffer3D(TRUE);												/*zbufuse*/
		SetWriteZBuffer3D(TRUE);											/*zbufwrite*/
		MV1SetLoadModelPhysicsWorldGravity(-9.8f);							/*重力*/
		if (use_vr) {
			//SetWindowSize(deskx_real, desky_real);
		}
		else {
			SetWindowSize(deskx_real, desky_real);
		}
		outScreen[0] = GraphHandle::Make(this->disp_x, this->disp_y);	/*左目*/
		outScreen[1] = GraphHandle::Make(this->disp_x, this->disp_y);	/*右目*/
		outScreen[2] = GraphHandle::Make(this->disp_x, this->disp_y);	/*TPS用*/
		//VRのセット
#ifdef _USE_OPENVR_
		if (use_vr) {
			tracker_num.clear();
			deviceall = 0;
			int i = 0;
			for (char k = 0; k < 8; k++) {
				auto old = deviceall;
				auto dev = m_pHMD->GetTrackedDeviceClass(k);
				if (dev == DEVICE_HMD) {
					hmd_num = deviceall;
					deviceall++;
				}
				else if (dev == DEVICE_CONTROLLER) {
					switch (i) {
					case 0:
						hand1_num = deviceall;
						i++;
						break;
					case 1:
						hand2_num = deviceall;
						i++;
						break;
					default:
						break;
					}
					deviceall++;
				}
				else if (dev == DEVICE_TRACKER) {
					tracker_num.emplace_back(deviceall);
					deviceall++;
				}
				else if (dev == DEVICE_BASESTATION) {
					deviceall++;
				}
				if (deviceall != old) {
					ctrl.resize(deviceall);
					ctrl.back().now = false;
					ctrl.back().id = old;
					ctrl.back().num = k;
					ctrl.back().type = dev;
					ctrl.back().turn = true;
				}
			}
		}
#endif // _USE_OPENVR_
		//エフェクト
		{
			std::string p;
			WIN32_FIND_DATA win32fdt;
			HANDLE hFind;
			hFind = FindFirstFile("data/effect/*", &win32fdt);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					{
						p = win32fdt.cFileName;
						if (p.find(".efk") != std::string::npos) {
							effHndle.resize(effHndle.size() + 1);
							effHndle.back() = EffekseerEffectHandle::load("data/effect/" + p);
						}
					}
				} while (FindNextFile(hFind, &win32fdt));
			} //else{ return false; }
			FindClose(hFind);
		}
	}
	~DXDraw(void) {
#ifdef _USE_OPENVR_
		if (use_vr) {
			//vr::VR_Shutdown();
			m_pHMD = nullptr;
		}
#endif // _USE_OPENVR_
		Effkseer_End();
		DxLib_End();
	}
	bool Set_Light_Shadow(const VECTOR_ref& farsize, const VECTOR_ref& nearsize, const VECTOR_ref& Light_dir, std::function<void()> doing) {
		SetGlobalAmbientLight(GetColorF(0.12f, 0.11f, 0.10f, 0.0f));
		SetLightDirection(Light_dir.get());
		if (this->use_shadow) {
			shadow_near = MakeShadowMap(int(pow(2, this->shadow_size)), int(pow(2, this->shadow_size)));
			shadow_nearfar = MakeShadowMap(int(pow(2, this->shadow_size)), int(pow(2, this->shadow_size)));
			shadow_far = MakeShadowMap(int(pow(2, this->shadow_size)), int(pow(2, this->shadow_size)));
			SetShadowMapAdjustDepth(shadow_near, 0.0005f);
			SetShadowMapLightDirection(shadow_near, Light_dir.get());
			SetShadowMapAdjustDepth(shadow_nearfar, 0.0005f);
			SetShadowMapLightDirection(shadow_nearfar, Light_dir.get());
			SetShadowMapAdjustDepth(shadow_far, 0.0003f);
			SetShadowMapLightDirection(shadow_far, Light_dir.get());
			SetShadowMapDrawArea(shadow_far, nearsize.get(), farsize.get());
			ShadowMap_DrawSetup(shadow_far);
			doing();
			ShadowMap_DrawEnd();
		}
		return true;
	}
	bool Delete_Shadow() {
		if (this->use_shadow) {
			DeleteShadowMap(shadow_near);
			DeleteShadowMap(shadow_nearfar);
			DeleteShadowMap(shadow_far);
		}
		return true;
	}
	bool Update_far_Shadow(std::function<void()> doing) {
		if (this->use_shadow) {
			ShadowMap_DrawSetup(shadow_far);
			doing();
			ShadowMap_DrawEnd();
			return true;
		}
		return false;
	}
	bool Ready_Shadow(const VECTOR_ref& pos_t, std::function<void()> doing, std::function<void()> doing_nearfar, const VECTOR_ref& nearsize, const VECTOR_ref& nearfarsize) {
		if (this->use_shadow) {
			SetShadowMapDrawArea(shadow_near, (nearsize*-1.f + pos_t).get(), (VECTOR_ref(nearsize) + pos_t).get());
			ShadowMap_DrawSetup(shadow_near);
			doing();
			ShadowMap_DrawEnd();

			SetShadowMapDrawArea(shadow_nearfar, (nearfarsize*-1.f + pos_t).get(), (VECTOR_ref(nearfarsize) + pos_t).get());
			ShadowMap_DrawSetup(shadow_nearfar);
			doing_nearfar();
			ShadowMap_DrawEnd();
			return true;
		}
		return false;
	}
	bool Draw_by_Shadow(std::function<void()> doing) {
		if (this->use_shadow) {
			SetUseShadowMap(0, shadow_far);
			SetUseShadowMap(1, shadow_nearfar);
			SetUseShadowMap(2, shadow_near);
		}
		doing();
		if (this->use_shadow) {
			SetUseShadowMap(0, -1);
			SetUseShadowMap(1, -1);
			SetUseShadowMap(2, -1);
		}
		return true;
	}
	bool Screen_Flip() {
		ScreenFlip();
#ifdef _USE_OPENVR_
		if (use_vr) {
			vr::TrackedDevicePose_t tmp;
			vr::VRCompositor()->WaitGetPoses(&tmp, 1, NULL, 1);
		}
#endif // _USE_OPENVR_
		return true;
	}
	static bool Capsule3D(const VECTOR_ref& p1, const VECTOR_ref& p2, const float& range, const unsigned int& color, const unsigned int& speccolor) {
		return DxLib::DrawCapsule3D(p1.get(), p2.get(), range, 8, color, speccolor, TRUE) == TRUE;
	}
	static bool Sphere3D(const VECTOR_ref& p1, const float& range, const unsigned int& color, const unsigned int& speccolor) {
		return DxLib::DrawSphere3D(p1.get(), range, 8, color, speccolor, TRUE) == TRUE;
	}
	static bool Line2D(const int& p1x, const int& p1y, const int& p2x, const int& p2y, const unsigned int& color, const int& thickness = 1) {
		return DxLib::DrawLine(p1x, p1y, p2x, p2y, color, thickness) == TRUE;
	}
	//VR
#ifdef _USE_OPENVR_
	const auto& get_hmd_num(void) { return hmd_num; }
	const auto& get_hand1_num(void) { return hand1_num; }
	const auto& get_hand2_num(void) { return hand2_num; }
	auto* get_device(void) { return &ctrl; }
	auto* get_device_hmd(void) {
		if (hmd_num >= 0) {
			return &ctrl[std::max<char>(hmd_num, 0)];
		}
		return (system_VR*)nullptr;
	}
	auto* get_device_hand1(void) {
		if (hand1_num >= 0) {
			return &ctrl[hand1_num];
		}
		return (system_VR*)nullptr;
	}
	auto* get_device_hand2(void) {
		if (hand2_num >= 0) {
			return &ctrl[hand2_num];
		}
		return (system_VR*)nullptr;
	}
#endif // _USE_OPENVR_
	/**/
	void Move_Player(void) {
#ifdef _USE_OPENVR_
		if (use_vr) {
			vr::TrackedDevicePose_t tmp;
			vr::VRControllerState_t night;
			for (auto& c : ctrl) {
				if (c.type == DEVICE_HMD) {
					m_pHMD->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0.0f, &tmp, 1);
					c.on[0] = 0;
					c.on[1] = 0;
					c.touch = VECTOR_ref::vget(0, 0, 0);
					c.now = tmp.bPoseIsValid;
					c.pos = VECTOR_ref::vget(tmp.mDeviceToAbsoluteTracking.m[0][3], tmp.mDeviceToAbsoluteTracking.m[1][3], -tmp.mDeviceToAbsoluteTracking.m[2][3]);
					c.xvec = VECTOR_ref::vget(-tmp.mDeviceToAbsoluteTracking.m[0][0], -tmp.mDeviceToAbsoluteTracking.m[1][0], tmp.mDeviceToAbsoluteTracking.m[2][0]);
					c.yvec = VECTOR_ref::vget(tmp.mDeviceToAbsoluteTracking.m[0][1], tmp.mDeviceToAbsoluteTracking.m[1][1], -tmp.mDeviceToAbsoluteTracking.m[2][1]);
					c.zvec = VECTOR_ref::vget(tmp.mDeviceToAbsoluteTracking.m[0][2], tmp.mDeviceToAbsoluteTracking.m[1][2], -tmp.mDeviceToAbsoluteTracking.m[2][2]);
				}
				else if (c.type == DEVICE_CONTROLLER || c.type == DEVICE_BASESTATION || c.type == DEVICE_TRACKER) {
					m_pHMD->GetControllerStateWithPose(vr::TrackingUniverseStanding, c.num, &night, sizeof(night), &tmp);
					c.on[0] = night.ulButtonPressed;
					c.on[1] = night.ulButtonTouched;
					c.touch = VECTOR_ref::vget(night.rAxis[0].x, night.rAxis[0].y, 0);
					c.now = tmp.bPoseIsValid;
					c.pos = VECTOR_ref::vget(tmp.mDeviceToAbsoluteTracking.m[0][3], tmp.mDeviceToAbsoluteTracking.m[1][3], -tmp.mDeviceToAbsoluteTracking.m[2][3]);
					c.xvec = VECTOR_ref::vget(-tmp.mDeviceToAbsoluteTracking.m[0][0], -tmp.mDeviceToAbsoluteTracking.m[1][0], tmp.mDeviceToAbsoluteTracking.m[2][0]);
					c.yvec = VECTOR_ref::vget(tmp.mDeviceToAbsoluteTracking.m[0][1], tmp.mDeviceToAbsoluteTracking.m[1][1], -tmp.mDeviceToAbsoluteTracking.m[2][1]);
					c.zvec = VECTOR_ref::vget(tmp.mDeviceToAbsoluteTracking.m[0][2], tmp.mDeviceToAbsoluteTracking.m[1][2], -tmp.mDeviceToAbsoluteTracking.m[2][2]);
				}
			}
		}
		else {
			for (auto& c : ctrl) {
				c.on[0] = 0;
				c.on[1] = 0;
				c.touch = VECTOR_ref::vget(0, 0, 0);
				c.pos = VECTOR_ref::vget(0, 0, 0);
				c.xvec = VECTOR_ref::vget(1, 0, 0);
				c.yvec = VECTOR_ref::vget(0, 1, 0);
				c.zvec = VECTOR_ref::vget(0, 0, 1);
			}
		}
#endif // _USE_OPENVR_
	}
	/**/
	inline VECTOR_ref SetEyePositionVR(const char& eye_type) {
#ifdef _USE_OPENVR_
		if (use_vr) {
			const vr::HmdMatrix34_t tmpmat = vr::VRSystem()->GetEyeToHeadTransform((vr::EVREye)eye_type);
			return ctrl[hmd_num].pos + ctrl[hmd_num].xvec*(tmpmat.m[0][3]) + ctrl[hmd_num].yvec*(tmpmat.m[1][3]) + ctrl[hmd_num].zvec*(-tmpmat.m[2][3]);
		}
#endif // _USE_OPENVR_
		return VECTOR_ref::vget(0, 0, 0);
	}
	/**/
	inline void GetDevicePositionVR(const char& handle_, VECTOR_ref* pos_, MATRIX_ref*mat) {
#ifdef _USE_OPENVR_
		if (use_vr) {
			if (handle_ != -1) {
				*pos_ = ctrl[handle_].pos;
				*mat = MATRIX_ref::Axis1(ctrl[handle_].xvec*-1.f, ctrl[handle_].yvec, ctrl[handle_].zvec*-1.f);
				return;
			}
		}
#endif // _USE_OPENVR_
		*pos_ = VECTOR_ref::vget(0, 0, 0);
		*mat = MATRIX_ref::Axis1(VECTOR_ref::vget(1, 0, 0), VECTOR_ref::vget(0, 1, 0), VECTOR_ref::vget(0, 0, 1));
	}
	/**/
	inline void reset_HMD() {
#ifdef _USE_OPENVR_
		oldv = false;
		start_c = true;
#endif // _USE_OPENVR_
	}
	/**/
	inline void GetHMDPositionVR(VECTOR_ref* pos_, MATRIX_ref*mat) {
#ifdef _USE_OPENVR_
		if (use_vr) {
			if (hmd_num != -1) {
				*pos_ = ctrl[hmd_num].pos;
				*mat = MATRIX_ref::Axis1(ctrl[hmd_num].xvec*-1.f, ctrl[hmd_num].yvec, ctrl[hmd_num].zvec*-1.f);
			}
		}
		else
#endif // _USE_OPENVR_
		{
			*pos_ = VECTOR_ref::vget(0, 0, 0);
			*mat = MATRIX_ref::Axis1(VECTOR_ref::vget(1, 0, 0), VECTOR_ref::vget(0, 1, 0), VECTOR_ref::vget(0, 0, 1));
		}
		*mat = MATRIX_ref::Axis1((*mat).xvec()*-1.f, (*mat).yvec(), (*mat).zvec()*-1.f);
#ifdef _USE_OPENVR_
		{
			auto& ptr_ = *get_device_hmd();
			if (start_c && (ptr_.turn && ptr_.now) != oldv) {
				rec_HMD = VECTOR_ref::vget((*pos_).x(), 0.f, (*pos_).z());

				VECTOR_ref tmp = (*mat).zvec();
				tmp = VECTOR_ref::vget(tmp.x(), 0.f, tmp.z());
				tmp = tmp.Norm();
				rec_HMDmat = MATRIX_ref::RotY(DX_PI_F + std::atan2f(tmp.x(), -tmp.z()));

				start_c = false;
			}
			if (!start_c && !(ptr_.turn && ptr_.now)) {
				start_c = true;
			}
			oldv = ptr_.turn && ptr_.now;
			(*pos_) = (*pos_) - rec_HMD;
			(*mat) = (*mat) * rec_HMDmat;
		}
#endif // _USE_OPENVR_
	}
	/**/
	inline VECTOR_ref GetEyePosition_minVR(const char& eye_type) {
#ifdef _USE_OPENVR_
		if (use_vr) {
			const vr::HmdMatrix34_t tmpmat = vr::VRSystem()->GetEyeToHeadTransform((vr::EVREye)eye_type);
			return (ctrl[hmd_num].xvec*-1.f*(tmpmat.m[0][3])) + (ctrl[hmd_num].yvec*(tmpmat.m[1][3])) + (ctrl[hmd_num].zvec*-1.f*(tmpmat.m[2][3]));
		}
		else
#endif // _USE_OPENVR_
		{
			return VECTOR_ref::vget(0, 0, 0);
		}
	}
	/**/
	inline void PutEye(ID3D11Texture2D* texte, const char& i) {
#ifdef _USE_OPENVR_
		if (use_vr) {
			vr::Texture_t tex = { (void*)texte, vr::ETextureType::TextureType_DirectX,vr::EColorSpace::ColorSpace_Auto };
			vr::VRCompositor()->Submit((vr::EVREye)i, &tex, NULL, vr::Submit_Default);
		}
#endif // _USE_OPENVR_
	}
	/**/
	void draw_VR(std::function<void()> doing, const cam_info& cams) {
#ifdef _USE_OPENVR_
		if (this->use_vr) {
			for (char i = 0; i < 2; i++) {
				outScreen[i].SetDraw_Screen(VECTOR_ref(cams.campos) + this->GetEyePosition_minVR(i), VECTOR_ref(cams.camvec) + this->GetEyePosition_minVR(i), cams.camup, cams.fov, cams.near_, cams.far_);

				doing();

				GraphHandle::SetDraw_Screen((int)DX_SCREEN_BACK);
				{
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
					outScreen[i].DrawGraph(0, 0, false);
					this->PutEye((ID3D11Texture2D*)GetUseDirect3D11BackBufferTexture2D(), i);
				}
			}
		}
		else
#endif // _USE_OPENVR_
		{
			outScreen[0].SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, cams.near_, cams.far_);
			doing();
		}
		GraphHandle::SetDraw_Screen((int)DX_SCREEN_BACK);
		{
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
			outScreen[0].DrawExtendGraph(0, 0, deskx, desky, true);
		}
	}
	/**/
	inline void Haptic(const char&id_, unsigned short times) {
#ifdef _USE_OPENVR_
		if (use_vr) {
			if (id_ != -1) {
				if (m_pHMD) {
					m_pHMD->TriggerHapticPulse(ctrl[id_].id, 2, times);
				}
			}
		}
#endif // _USE_OPENVR_
	}
};

class frames {
public:
	int first{ 0 };
	VECTOR_ref second;

	void set(const int&i, const VECTOR_ref&j) {
		first = i;
		second = j;
	}
};

class switchs {
public:
	bool first{ false };
	uint8_t second{ 0 };

	switchs() {
		first = false;
		second = 0;
	};

	void ready(bool on) {
		first = on;
		second = 0;
	}

	void get_in(bool key) {
		second = std::clamp<uint8_t>(second + 1, 0, (key ? 2 : 0));
		if (push()) {
			first ^= 1;
		}
	}

	const bool on() {
		return first;
	}

	const bool push() {
		return second == 1;
	}

	const bool press() {
		return second != 0;
	}
};

#include "HostPass.hpp"
//ついで
#include "debug.hpp"

struct moves {
	VECTOR_ref pos;
	MATRIX_ref mat;
};