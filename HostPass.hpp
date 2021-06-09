#pragma once
#include "DXLib_ref/DXLib_ref.h"

// SSAOフィルター処理
	// 変換元として法線バッファを指定
	// 出力先バッファの指定
	//
	// カラーバッファを指定
	// 遮蔽物を調べる範囲
	// 遮蔽物判定する最小深度値
	// 遮蔽物判定する最大深度値
	// 遮蔽物の影響の強さ
	// オクリュージョンカラー
	// オクリュージョンカラーの強さ

class HostPassEffect {
private:
	GraphHandle FarScreen_;		//描画スクリーン
	GraphHandle NearFarScreen_;	//描画スクリーン
	GraphHandle NearScreen_;	//描画スクリーン
	GraphHandle GaussScreen_;	//描画スクリーン
	GraphHandle BufScreen;		//描画スクリーン
	GraphHandle BufScreen_;		//描画スクリーン
	GraphHandle BufScreen_no;	//描画スクリーン
	GraphHandle SkyScreen;		//空描画
	GraphHandle MAIN_Screen;	//描画スクリーン
	GraphHandle bkScreen;

	GraphHandle ColorScreen_;		//描画スクリーン
	GraphHandle NormalScreen_;	//描画スクリーン
	GraphHandle DummyScreen_;	//描画スクリーン

	GraphHandle UI_Screen;		//UI

	int EXTEND = 4;
	bool dof_flag = true;
	bool bloom_flag = true;
	bool ssao_flag = true;
	int disp_x = deskx;
	int disp_y = desky;

	//ブルームエフェクト
	void buf_ray() {
		if (bloom_flag) {
			GraphFilterBlt(BufScreen.get(), GaussScreen_.get(), DX_GRAPH_FILTER_DOWN_SCALE, EXTEND);
			GraphFilter(GaussScreen_.get(), DX_GRAPH_FILTER_GAUSS, 16, 1000);
		}
		BufScreen.SetDraw_Screen(false);
		if (bloom_flag) {
			SetDrawMode(DX_DRAWMODE_BILINEAR);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 92);
			GaussScreen_.DrawExtendGraph(0, 0, disp_x, disp_y, true);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		}
	}
	//ブルームエフェクト
	void buf_bloom() {
		if (bloom_flag) {
			GraphFilterBlt(BufScreen.get(), BufScreen_.get(), DX_GRAPH_FILTER_TWO_COLOR, 250, GetColor(0, 0, 0), 255, GetColor(128, 128, 128), 255);
			GraphFilterBlt(BufScreen_.get(), GaussScreen_.get(), DX_GRAPH_FILTER_DOWN_SCALE, EXTEND);
			GraphFilter(GaussScreen_.get(), DX_GRAPH_FILTER_GAUSS, 16, 1000);
		}
		BufScreen.SetDraw_Screen(false);
		if (bloom_flag) {
			SetDrawMode(DX_DRAWMODE_BILINEAR);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
			GaussScreen_.DrawExtendGraph(0, 0, disp_x, disp_y, true);
			GaussScreen_.DrawExtendGraph(0, 0, disp_x, disp_y, true);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		}
	}
	//レベル補正
	void buf_levelcorrect() {
		if (true) {
			int input_low = 0;
			int input_high = 255;
			float gamma = 1.25f;
			int output_low = 0;
			int output_high = 216;
			if (ssao_flag) {
				output_high = 255;
			}
			GraphFilterBlt(BufScreen.get(), BufScreen_.get(), DX_GRAPH_FILTER_LEVEL, input_low, input_high, int(gamma * 100), output_low, output_high);
		}
		BufScreen.SetDraw_Screen(false);
		if (true) {
			BufScreen_.DrawExtendGraph(0, 0, disp_x, disp_y, true);
		}
	}

	//被写体深度描画
	void near_dof(std::function<void()> sky_doing, std::function<void()> doing, cam_info& cams, bool update_effekseer = true) {
		if (dof_flag) {
			//空
			SkyScreen.SetDraw_Screen(cams.campos - cams.camvec, VECTOR_ref::vget(0, 0, 0), cams.camup, cams.fov, 1000.0f, 5000.0f);
			{
				sky_doing();
			}
			//遠距離
			FarScreen_.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, cams.far_, 1000000.f);
			{
				SkyScreen.DrawGraph(0, 0, FALSE);
				doing();
			}
			//中間
			if (ssao_flag) {
				// カラーバッファを描画対象0に、法線バッファを描画対象1に設定
				SetRenderTargetToShader(1, NormalScreen_.get());
				SetRenderTargetToShader(0, ColorScreen_.get());
				SetCameraNearFar(cams.near_, cams.far_);
				SetupCamera_Perspective(cams.fov);
				SetCameraPositionAndTargetAndUpVec(cams.campos.get(), cams.camvec.get(), cams.camup.get());
				ClearDrawScreen();
			}
			else {
				NearFarScreen_.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, cams.near_, cams.far_);
			}
			{
				Effekseer_Sync3DSetting();
				GraphFilter(FarScreen_.get(), DX_GRAPH_FILTER_GAUSS, 16, 200);
				FarScreen_.DrawGraph(0, 0, false);
				if (update_effekseer) {
					UpdateEffekseer3D();
				}
				doing();
				DrawEffekseer3D();
			}
			if (ssao_flag) {
				SetRenderTargetToShader(0, BufScreen.get());
				SetRenderTargetToShader(1, -1);
				GraphFilterBlt(NormalScreen_.get(), NearFarScreen_.get(), DX_GRAPH_FILTER_SSAO, ColorScreen_.get(), 450.f, 0.001f, 0.042f, 0.4f, GetColor(0, 0, 0), 6.5f);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
				SetRenderTargetToShader(0, DummyScreen_.get());
				SetRenderTargetToShader(1, -1);
			}
			//至近
			NearScreen_.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, 0.1f, 0.1f + cams.near_);
			{
				NearFarScreen_.DrawGraph(0, 0, false);
				doing();
			}
		}
	}
	//特に何もせず描画
	void near_nomal(std::function<void()> sky_doing, std::function<void()> doing, cam_info& cams, bool update_effekseer = true) {
		//空
		SkyScreen.SetDraw_Screen(cams.campos - cams.camvec, VECTOR_ref::vget(0, 0, 0), cams.camup, cams.fov, 1000.0f, 5000.0f);
		{
			sky_doing();
		}
		if (ssao_flag) {
			// カラーバッファを描画対象0に、法線バッファを描画対象1に設定
			SetRenderTargetToShader(1, NormalScreen_.get());
			SetRenderTargetToShader(0, ColorScreen_.get());
			SetCameraNearFar(cams.near_, cams.far_);
			SetupCamera_Perspective(cams.fov);
			SetCameraPositionAndTargetAndUpVec(cams.campos.get(), cams.camvec.get(), cams.camup.get());
			ClearDrawScreen();
		}
		else {
			NearScreen_.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, 0.1f, cams.far_);
		}
		{
			Effekseer_Sync3DSetting();
			SkyScreen.DrawGraph(0, 0, FALSE);
			if (update_effekseer) {
				UpdateEffekseer3D();
			}
			doing();
			DrawEffekseer3D();
		}
		if (ssao_flag) {
			SetRenderTargetToShader(0, BufScreen.get());
			SetRenderTargetToShader(1, -1);
			GraphFilterBlt(NormalScreen_.get(), NearScreen_.get(), DX_GRAPH_FILTER_SSAO, ColorScreen_.get(), 250.f, 0.002f, 0.01f, 0.1f, GetColor(0, 0, 0), 20.f);
			SetRenderTargetToShader(0, DummyScreen_.get());
			SetRenderTargetToShader(1, -1);
		}
	}
public:
	HostPassEffect(const bool& dof_, const bool& bloom_, const bool& SSAO_, const int& xd, const int& yd) {
		disp_x = xd;
		disp_y = yd;
		//DoF用
		dof_flag = dof_;
		SkyScreen = GraphHandle::Make(disp_x, disp_y, false);						//空描画
		FarScreen_ = GraphHandle::Make(disp_x, disp_y, true);						//描画スクリーン
		NearFarScreen_ = GraphHandle::Make(disp_x, disp_y, true);					//描画スクリーン
		NearScreen_ = GraphHandle::Make(disp_x, disp_y, true);						//描画スクリーン
		//ブルーム用
		bloom_flag = bloom_;
		GaussScreen_ = GraphHandle::Make(disp_x / EXTEND, disp_y / EXTEND, true);	//描画スクリーン
		BufScreen_ = GraphHandle::Make(disp_x, disp_y, true);						//描画スクリーン
		BufScreen_no = GraphHandle::Make(disp_x, disp_y, false);						//描画スクリーン
		//最終描画用
		BufScreen = GraphHandle::Make(disp_x, disp_y, true);						//描画スクリーン
		MAIN_Screen = GraphHandle::Make(disp_x, disp_y, false);						//描画スクリーン

		bkScreen = GraphHandle::Make(disp_x, disp_y, false);						//ふち黒

		ssao_flag = SSAO_;
		ColorScreen_ = GraphHandle::Make(disp_x, disp_y, false);
		NormalScreen_ = GraphHandle::Make(disp_x, disp_y, true);
		DummyScreen_ = GraphHandle::Make(disp_x, disp_y, true);
		UI_Screen = GraphHandle::Make(disp_x, disp_y, true);

		bkScreen.SetDraw_Screen(true);
		{
			int y = 0, c = 0, p = 2;
			DrawBox(0, 0, disp_x, disp_y, GetColor(255, 255, 255), TRUE);
			p = 1;
			for (y = 0; y < 255; y += p) {
				c = 255 - int(powf(float(255 - y) / 255.f, 1.5f)*64.f);
				DXDraw::Line2D(0, y / p, disp_x, y / p, GetColor(c, c, c));
			}
			p = 2;
			for (y = 0; y < 255; y += p) {
				c = 255 - int(powf(float(255 - y) / 255.f, 1.5f)*128.f);
				DXDraw::Line2D(0, disp_y - y / p, disp_x, disp_y - y / p, GetColor(c, c, c));
			}
		}
	}

	~HostPassEffect() {
	}

	void BUF_draw(std::function<void()> sky_doing, std::function<void()> doing, cam_info& cams, bool update_effekseer = true) {
		//nearに描画
		if (dof_flag) {
			near_dof(sky_doing, doing, cams, update_effekseer);
		}
		else {
			near_nomal(sky_doing, doing, cams, update_effekseer);
		}
		//結果描画
		BufScreen.SetDraw_Screen();
		{
			NearScreen_.DrawGraph(0, 0, false);
			//NormalScreen_.DrawGraph(0, 0, false);
		}
	}
	//
	void Set_MAIN_draw() {
		//bufに描画
		buf_bloom();//ブルーム
		buf_ray();
		buf_levelcorrect();

		//結果描画
		MAIN_Screen.SetDraw_Screen();
		{
			GraphBlend(BufScreen.get(), bkScreen.get(), 255, DX_GRAPH_BLEND_RGBA_SELECT_MIX,
				DX_RGBA_SELECT_SRC_R, DX_RGBA_SELECT_SRC_G, DX_RGBA_SELECT_SRC_B, DX_RGBA_SELECT_BLEND_R);
			DrawBox(0, 0, disp_x, disp_y, GetColor(0, 0, 0), TRUE);

			BufScreen.DrawGraph(0, 0, true);

		}
	}
	//
	const auto& Get_MAIN_Screen() { return MAIN_Screen; }
	void MAIN_draw() {
		MAIN_Screen.DrawGraph(0, 0, true);
	}
	void UI_draw() {
		UI_Screen.DrawGraph(0, 0, TRUE);
	}

	void Set_UI_draw(std::function<void()> doing) {
		UI_Screen.SetDraw_Screen();
		{
			doing();
		}
	}
	void Draw(cam_info* cam_t,bool use_vr) {
		//UI
		if (use_vr) {
			SetCameraNearFar(0.01f, 2.f);
			SetUseZBuffer3D(FALSE);												//zbufuse
			SetWriteZBuffer3D(FALSE);											//zbufwrite
			{
				DrawBillboard3D((cam_t->campos + (cam_t->camvec - cam_t->campos).Norm()*1.0f).get(), 0.5f, 0.5f, 1.8f, 0.f, UI_Screen.get(), TRUE);
			}
			SetUseZBuffer3D(TRUE);												//zbufuse
			SetWriteZBuffer3D(TRUE);											//zbufwrite
		}
		else {
			UI_draw();
		}
	}
};
