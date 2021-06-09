#pragma once
#include "DXLib_ref/DXLib_ref.h"

// SSAO�t�B���^�[����
	// �ϊ����Ƃ��Ė@���o�b�t�@���w��
	// �o�͐�o�b�t�@�̎w��
	//
	// �J���[�o�b�t�@���w��
	// �Օ����𒲂ׂ�͈�
	// �Օ������肷��ŏ��[�x�l
	// �Օ������肷��ő�[�x�l
	// �Օ����̉e���̋���
	// �I�N�����[�W�����J���[
	// �I�N�����[�W�����J���[�̋���

class HostPassEffect {
private:
	GraphHandle FarScreen_;		//�`��X�N���[��
	GraphHandle NearFarScreen_;	//�`��X�N���[��
	GraphHandle NearScreen_;	//�`��X�N���[��
	GraphHandle GaussScreen_;	//�`��X�N���[��
	GraphHandle BufScreen;		//�`��X�N���[��
	GraphHandle BufScreen_;		//�`��X�N���[��
	GraphHandle BufScreen_no;	//�`��X�N���[��
	GraphHandle SkyScreen;		//��`��
	GraphHandle MAIN_Screen;	//�`��X�N���[��
	GraphHandle bkScreen;

	GraphHandle ColorScreen_;		//�`��X�N���[��
	GraphHandle NormalScreen_;	//�`��X�N���[��
	GraphHandle DummyScreen_;	//�`��X�N���[��

	GraphHandle UI_Screen;		//UI

	int EXTEND = 4;
	bool dof_flag = true;
	bool bloom_flag = true;
	bool ssao_flag = true;
	int disp_x = deskx;
	int disp_y = desky;

	//�u���[���G�t�F�N�g
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
	//�u���[���G�t�F�N�g
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
	//���x���␳
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

	//��ʑ̐[�x�`��
	void near_dof(std::function<void()> sky_doing, std::function<void()> doing, cam_info& cams, bool update_effekseer = true) {
		if (dof_flag) {
			//��
			SkyScreen.SetDraw_Screen(cams.campos - cams.camvec, VECTOR_ref::vget(0, 0, 0), cams.camup, cams.fov, 1000.0f, 5000.0f);
			{
				sky_doing();
			}
			//������
			FarScreen_.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, cams.far_, 1000000.f);
			{
				SkyScreen.DrawGraph(0, 0, FALSE);
				doing();
			}
			//����
			if (ssao_flag) {
				// �J���[�o�b�t�@��`��Ώ�0�ɁA�@���o�b�t�@��`��Ώ�1�ɐݒ�
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
			//����
			NearScreen_.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, 0.1f, 0.1f + cams.near_);
			{
				NearFarScreen_.DrawGraph(0, 0, false);
				doing();
			}
		}
	}
	//���ɉ��������`��
	void near_nomal(std::function<void()> sky_doing, std::function<void()> doing, cam_info& cams, bool update_effekseer = true) {
		//��
		SkyScreen.SetDraw_Screen(cams.campos - cams.camvec, VECTOR_ref::vget(0, 0, 0), cams.camup, cams.fov, 1000.0f, 5000.0f);
		{
			sky_doing();
		}
		if (ssao_flag) {
			// �J���[�o�b�t�@��`��Ώ�0�ɁA�@���o�b�t�@��`��Ώ�1�ɐݒ�
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
		//DoF�p
		dof_flag = dof_;
		SkyScreen = GraphHandle::Make(disp_x, disp_y, false);						//��`��
		FarScreen_ = GraphHandle::Make(disp_x, disp_y, true);						//�`��X�N���[��
		NearFarScreen_ = GraphHandle::Make(disp_x, disp_y, true);					//�`��X�N���[��
		NearScreen_ = GraphHandle::Make(disp_x, disp_y, true);						//�`��X�N���[��
		//�u���[���p
		bloom_flag = bloom_;
		GaussScreen_ = GraphHandle::Make(disp_x / EXTEND, disp_y / EXTEND, true);	//�`��X�N���[��
		BufScreen_ = GraphHandle::Make(disp_x, disp_y, true);						//�`��X�N���[��
		BufScreen_no = GraphHandle::Make(disp_x, disp_y, false);						//�`��X�N���[��
		//�ŏI�`��p
		BufScreen = GraphHandle::Make(disp_x, disp_y, true);						//�`��X�N���[��
		MAIN_Screen = GraphHandle::Make(disp_x, disp_y, false);						//�`��X�N���[��

		bkScreen = GraphHandle::Make(disp_x, disp_y, false);						//�ӂ���

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
		//near�ɕ`��
		if (dof_flag) {
			near_dof(sky_doing, doing, cams, update_effekseer);
		}
		else {
			near_nomal(sky_doing, doing, cams, update_effekseer);
		}
		//���ʕ`��
		BufScreen.SetDraw_Screen();
		{
			NearScreen_.DrawGraph(0, 0, false);
			//NormalScreen_.DrawGraph(0, 0, false);
		}
	}
	//
	void Set_MAIN_draw() {
		//buf�ɕ`��
		buf_bloom();//�u���[��
		buf_ray();
		buf_levelcorrect();

		//���ʕ`��
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
