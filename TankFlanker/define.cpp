#define NOMINMAX
#include "define.h"
#include "FontHandle.hpp"
#include <fstream>
//
Myclass::Myclass() {
	using namespace std::literals;
	WIN32_FIND_DATA win32fdt;
	HANDLE hFind;
	char mstr[64]; /*tank*/
	int mdata;     /*tank*/

	SetOutApplicationLogValidFlag(FALSE); /*log*/

	mdata = FileRead_open("data/setting.txt", FALSE);
	FileRead_gets(mstr, 64, mdata);
		usegrab = bool(std::stoul(getright(mstr)));
		FileRead_gets(mstr, 64, mdata);
		ANTI = unsigned char(std::stoul(getright(mstr)));
		FileRead_gets(mstr, 64, mdata);
		YSync = bool(std::stoul(getright(mstr)));
		FileRead_gets(mstr, 64, mdata);
		f_rate = (YSync) ? 60.f : std::stof(getright(mstr));
		FileRead_gets(mstr, 64, mdata);
		windowmode = bool(std::stoul(getright(mstr)));
		FileRead_gets(mstr, 64, mdata);
		drawdist = std::stof(getright(mstr));
		FileRead_gets(mstr, 64, mdata);
		gndx = std::stoi(getright(mstr));
		FileRead_gets(mstr, 64, mdata);
		shadex = std::stoi(getright(mstr));
	FileRead_close(mdata);

	SetMainWindowText("Tank Flanker"); /*name*/
	//SetWindowStyleMode(4);			    /**/
	//SetWindowUserCloseEnableFlag(FALSE);		    /*alt+F4対処*/
	SetAeroDisableFlag(TRUE);			    /**/
	SetUsePixelLighting(TRUE);			    /*ピクセルライティング*/
	if (ANTI >= 2) {} /*アンチエイリアス*/		    /*動作不全のため一旦削除*/
	SetWaitVSyncFlag(YSync);			    /*垂直同期*/
	ChangeWindowMode(windowmode);			    /*窓表示*/
	SetUseDirect3DVersion(DX_DIRECT3D_11);		    /*directX ver*/
	Set3DSoundOneMetre(1.0f);			    /*3Dsound*/
	SetGraphMode(dispx, dispy, 32);			    /*解像度*/
	DxLib_Init();					    /*init*/
	Effekseer_Init(8000);				    /*Effekseer*/
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);  /*Effekseer*/
	Effekseer_SetGraphicsDeviceLostCallbackFunctions(); /*Effekseer*/
	//Effekseer_InitDistortion(1.0f);		    /*エフェクトの歪み*/
	SetAlwaysRunFlag(TRUE);			  /*background*/
	SetUseZBuffer3D(TRUE);			  /*zbufuse*/
	SetWriteZBuffer3D(TRUE);		  /*zbufwrite*/
	MV1SetLoadModelReMakeNormal(TRUE);	/*法線*/
	MV1SetLoadModelPhysicsWorldGravity(M_GR); /*重力*/
	//車両数取得
	hFind = FindFirstFile("data/tanks/*", &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((win32fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (win32fdt.cFileName[0] != '.')) {
				vecs.resize(vecs.size() + 1);
				vecs.back().name = win32fdt.cFileName;
			}
		} while (FindNextFile(hFind, &win32fdt));
	} //else{ return false; }
	FindClose(hFind);
	//車両データ取得
	for (auto& v : vecs) {
		mdata = FileRead_open(("data/tanks/" + v.name + "/data.txt").c_str(), FALSE);
		FileRead_gets(mstr, 64, mdata);
		v.countryc = std::stoi(getright(mstr));
		for (auto& s : v.speed_flont) {
			FileRead_gets(mstr, 64, mdata);
			s = std::stof(getright(mstr)) / 3.6f;
		}
		for (auto& s : v.speed_back) {
			FileRead_gets(mstr, 64, mdata);
			s = std::stof(getright(mstr)) / 3.6f;
		}
		FileRead_gets(mstr, 64, mdata);
		v.vehicle_RD = deg2rad(std::stof(getright(mstr)));
		for (auto& a : v.armer) {
			FileRead_gets(mstr, 64, mdata);
			a = std::stof(getright(mstr));
		}
		FileRead_gets(mstr, 64, mdata);
		v.gun_lim_LR = bool(std::stoul(getright(mstr)));
		for (auto& g : v.gun_lim_) {
			FileRead_gets(mstr, 64, mdata);
			g = deg2rad(std::stof(getright(mstr)));
		}
		FileRead_gets(mstr, 64, mdata);
		v.gun_RD = deg2rad(std::stof(getright(mstr))) / f_rate;
		FileRead_gets(mstr, 64, mdata);
		v.reloadtime[0] = int(std::stoi(getright(mstr)) * f_rate);
		v.reloadtime[1] = 10;
		FileRead_gets(mstr, 64, mdata);
		v.ammosize[0] = std::stof(getright(mstr)) / 1000.f;
		v.ammosize[1] = 0.0075f;
		for (size_t i = 0; i < std::size(v.ammotype); ++i) {
			FileRead_gets(mstr, 64, mdata);
			v.ammotype[i] = std::stoi(getright(mstr));
			FileRead_gets(mstr, 64, mdata);
			v.gun_speed[i] = std::stof(getright(mstr));
			FileRead_gets(mstr, 64, mdata);
			v.pene[i] = std::stof(getright(mstr));
		}
		FileRead_close(mdata);
	}
	SetUseASyncLoadFlag(TRUE);
	for (auto& v : vecs) {
		v.model = MV1ModelHandle::Load("data/tanks/" + v.name + "/model.mv1");
		v.colmodel = MV1ModelHandle::Load("data/tanks/" + v.name + "/col.mv1");
		v.inmodel = MV1ModelHandle::Load("data/tanks/" + v.name + "/in/model.mv1");
	}
	for (size_t j = 0; j < std::size(se_); ++j) {
		const auto filename = (j < 1) ? "data/audio/se/engine/shift.wav"s
					      : (j < 8) ? "data/audio/se/eject/" + std::to_string(j - 1) + ".wav"
							: "data/audio/se/load/" + std::to_string(j - 8) + ".wav";
		se_[j] = SoundHandle::Load(filename);
	}
	for (size_t j = 0; j < std::size(ui_reload); ++j)
		ui_reload[j] = GraphHandle::Load("data/ui/ammo_" + std::to_string(j) + ".bmp"); /*弾0,弾1,弾2,空弾*/
	SetUseASyncLoadFlag(FALSE);
}
void Myclass::write_option(void) {
	std::ofstream outputfile("data/setting.txt");
	outputfile << "usegrab(1or0)=" + std::to_string(usegrab) + "\n";
	outputfile << "ANTI(1~4)=" + std::to_string(ANTI) + "\n";
	outputfile << "YSync(1or0)=" + std::to_string(YSync) + "\n";
	outputfile << "fps(30or60or120)=" + std::to_string(f_rate) + "\n";
	outputfile << "windowmode(1or0)=" + std::to_string(windowmode) + "\n";
	outputfile << "drawdist(100~400)=" + std::to_string(drawdist) + "\n";
	outputfile << "groundx(1~16)=" + std::to_string(gndx) + "\n";
	outputfile << "shadow(0~3)=" + std::to_string(shadex) + "\n";
	outputfile.close();
}
bool Myclass::set_veh(void) {
	for (auto& v : vecs) {
		//
		for (int i = 0; i < v.colmodel.frame_num(); ++i) {
			std::string tempname = MV1GetFrameName(v.colmodel.get(), i);
			if (tempname == "min")
				v.coloc[0] = v.colmodel.frame(i);
			if (tempname == "max")
				v.coloc[2] = v.colmodel.frame(i);
		}
		//
		{
			int j = 0, k = 0, l = 0;
			for (int i = 0; i < v.model.frame_num(); ++i) {
				v.loc.emplace_back(v.model.frame(i));
				std::string tempname = MV1GetFrameName(v.model.get(), i);
				//エフェクト用
				if (tempname == "engine")
					v.engineframe = i;
				if (l < 2) {
					if (tempname.find("smoke") != std::string::npos)
						v.smokeframe[l++] = i;
				}
				//
				if (tempname == "turret")
					v.turretframe = i;
				if (j < 2) {
					if (tempname.find("gun") != std::string::npos && tempname.back() != '_') //gun
						v.gunframe[j++] = i;
				}
				//ホイール
				if (k < 2) {
					if (tempname[0] == 'K') //起動輪
						v.kidoframe[k++] = i;
				}
				if (tempname[0] == 'Y') //誘導輪
					v.youdoframe.push_back(i);
				if (tempname[0] == 'F') //ホイール
					v.wheelframe.push_back(i);
				if (tempname[0] == 'U') //ホイール
					v.upsizeframe.push_back(i);
			}
		}
	}
	//エフェクト------------------------------------------------------------//
	const auto c_00ff00 = GetColor(0, 255, 0);
	const auto c_ffff00 = GetColor(255, 255, 0);
	const auto c_ff0000 = GetColor(255, 0, 0);
	//読み込みミス現状なさそう？
	for (int j = 0, k = 0; j < effects; ++j, ++k) {
		for (size_t i = 0; i < f_rate && ProcessMessage() == 0; ++i) {
			effHndle[j] = EffekseerEffectHandle::load("data/effect/" + std::to_string(j) + ".efk");
			const auto waits = GetNowHiPerformanceCount();
			SetDrawScreen(DX_SCREEN_BACK);
			DrawFormatString(0, (18 * k), c_00ff00, "エフェクト読み込み中…%d/%d", j, effects); //
			if (effHndle[j]) {
				k++;
				DrawFormatString(0, (18 * k), c_ffff00, "エフェクト読み込み成功…%d", j);
			}
			if (i == f_rate-1) {
				k++;
				DrawFormatString(0, (18 * k), c_ff0000, "エフェクト読み込み失敗…%d", j);
			}
			Screen_Flip(waits);
			if (effHndle[j])
				break;
		}
	}
	for (size_t i = 0; i < f_rate && ProcessMessage() == 0; ++i)
		Screen_Flip(GetNowHiPerformanceCount());

	return true;
}
int Myclass::window_choosev(void) {
	SetMousePoint(x_r(960), y_r(969));
	SetMouseDispFlag(TRUE);
	const auto font18 = FontHandle::Create(x_r(18), y_r(18 / 3), DX_FONTTYPE_ANTIALIASING);
	const auto font72 = FontHandle::Create(NULL, x_r(72), y_r(72 / 3), DX_FONTTYPE_ANTIALIASING);
	int i = 0, l = 0, x = 0, y = 0;
	int xp = 0, yp = 0;
	unsigned int m;
	float pert;
	int mousex, mousey;
	float real = 0.f, r = 5.f;
	LONGLONG waits;
	const auto c_00ff00 = GetColor(0, 255, 0);
	const auto c_ffff00 = GetColor(255, 255, 0);
	const auto c_ff0000 = GetColor(255, 0, 0);
	const auto c_ffffff = GetColor(255, 255, 255);
	const auto c_808080 = GetColor(128,128,128);
	const auto c_ffc800 = GetColor(255, 200, 0);
	const auto c_ff6400 = GetColor(255, 100, 0);
	while (ProcessMessage() == 0) {
		waits = GetNowHiPerformanceCount();
		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
			i = -1;
			break;
		} //end
		SetDrawScreen(DX_SCREEN_BACK);
		ClearDrawScreen();
		differential(real, deg2rad(360 * l / (int)vecs.size()), 0.05f);
		setcv(1.0f, 100.0f, VGet(-sin(real) * (10.f + r), 1, -cos(real) * (10.f + r)), VGet(-sin(real) * r, 2, -cos(real) * r), VGet(0, 1.0f, 0), 45.0f);
		SetLightDirection(VSub(VGet(-sin(real) * r, 2, -cos(real) * r), VGet(-sin(real) * (10.f + r), 4, -cos(real) * (10.f + r))));
		for (size_t k = 0; k < vecs.size(); k++) {
			MV1SetPosition(vecs[k].model.get(), VGet(-sin(deg2rad(360 * k / vecs.size())) * r, 0, -cos(deg2rad(360 * k / vecs.size())) * r));
			MV1SetRotationXYZ(vecs[k].model.get(), VGet(0, deg2rad((360 * k / vecs.size() + 30)), 0));
			MV1DrawModel(vecs[k].model.get());
		}

		pert = abs(1.0f - abs(float(real - deg2rad(360 * l / (int)vecs.size())) / deg2rad(360 / (int)vecs.size())));

		font72.DrawString(x_r(960) - font72.GetDrawWidth(vecs[i].name) / 2, y_r(154), vecs[i].name, c_00ff00);
		
			xp = 850;
			yp = 850;
			DrawBox(x_r(xp - 1), y_r(yp + 18), x_r(xp + 1 + 200), y_r(yp + 19), c_808080, FALSE);
			DrawBox(x_r(xp + 1 + 100), y_r(yp + 17), x_r(xp - 1 + 100 + 100 * (vecs[i].speed_flont[3] * 3.6f) / 100.f * pert), y_r(yp + 20), c_00ff00, TRUE);
			DrawBox(x_r(xp + 1 + 100), y_r(yp + 17), x_r(xp - 1 + 100 - 100 * (vecs[i].speed_back[3] * -3.6f) / 50.f * pert), y_r(yp + 20), c_ff0000, TRUE);
			DrawFormatStringToHandle(x_r(xp), y_r(850), c_00ff00, font18.get(), "SPEED : %5.2f～%5.2f km/h", vecs[i].speed_flont[3] * 3.6f, vecs[i].speed_back[3] * 3.6f);

			xp = 1140;
			yp = 810;
			DrawBox(x_r(xp - 1), y_r(yp + 18), x_r(xp + 1 + 200), y_r(yp + 19), c_808080, FALSE);
			DrawBox(x_r(xp + 1), y_r(yp + 17), x_r(xp - 1 + 200 * rad2deg(vecs[i].vehicle_RD) / 100.f * pert), y_r(yp + 20), c_00ff00, TRUE);
			DrawFormatStringToHandle(x_r(xp), y_r(yp), c_00ff00, font18.get(), "TURN SPEED : %5.2f deg/s", rad2deg(vecs[i].vehicle_RD));

			xp=1120;
			yp = 580;
			DrawBox(x_r(xp - 1), y_r(yp + 18), x_r(xp + 1 + 200), y_r(yp + 19), c_808080, FALSE);
			DrawBox(x_r(xp + 1), y_r(yp + 17), x_r(xp - 1 + 200 * vecs[i].armer[0] / 150.f * pert), y_r(yp + 20), c_00ff00, TRUE);
			DrawFormatStringToHandle(x_r(xp), y_r(yp), c_00ff00, font18.get(), "MAX ARMER : %5.2f mm", vecs[i].armer[0]);

			xp = 650;
			yp = 410;
			DrawBox(x_r(xp - 1), y_r(yp + 18), x_r(xp + 1 + 200), y_r(yp + 19), c_808080, FALSE);
			DrawBox(x_r(xp + 1 + 100), y_r(yp + 17), x_r(xp - 1 + 100 + 100 * rad2deg(vecs[i].gun_lim_[2]) / 40.f * pert), y_r(yp + 20), c_00ff00, TRUE);
			DrawBox(x_r(xp + 1 + 100), y_r(yp + 17), x_r(xp - 1 + 100 - 100 * rad2deg(vecs[i].gun_lim_[3]) / -20.f * pert), y_r(yp + 20), c_ff0000, TRUE);
			DrawFormatStringToHandle(x_r(xp), y_r(yp), c_00ff00, font18.get(), "GUN RAD     : %5.2f°～%5.2f°", rad2deg(vecs[i].gun_lim_[2]), rad2deg(vecs[i].gun_lim_[3]));

			xp = 650;
			yp = 430;
			DrawBox(x_r(xp - 1), y_r(yp + 18), x_r(xp + 1 + 200), y_r(yp + 19), c_808080, FALSE);
			DrawBox(x_r(xp + 1), y_r(yp + 17), x_r(xp - 1 + 200 * (vecs[i].ammosize[0] * 1000.f) / 200.f * pert), y_r(yp + 20), c_00ff00, TRUE);
			DrawFormatStringToHandle(x_r(xp), y_r(yp), c_00ff00, font18.get(), "GUN CALIBER : %05.1fmm", vecs[i].ammosize[0] * 1000.f);
		//
		font18.DrawString(x_r(0), y_r(18 * 1), "SETTING", c_00ff00);
		DrawFormatStringToHandle(x_r(0), y_r(18 * 2), c_00ff00, font18.get(), " 人の物理演算         : %s", usegrab ? "TRUE" : "FALSE");
		DrawFormatStringToHandle(x_r(0), y_r(18 * 3), c_00ff00, font18.get(), " アンチエイリアス倍率 : x%d", ANTI);
		DrawFormatStringToHandle(x_r(0), y_r(18 * 4), c_00ff00, font18.get(), " 垂直同期             : %s", YSync ? "TRUE" : "FALSE");
		DrawFormatStringToHandle(x_r(0), y_r(18 * 5), c_00ff00, font18.get(), " ウィンドウor全画面   : %s", windowmode ? "TRUE" : "FALSE");
		DrawFormatStringToHandle(x_r(0), y_r(18 * 6), c_00ff00, font18.get(), " 木の描画距離         : %5.2f m", drawdist);
		DrawFormatStringToHandle(x_r(0), y_r(18 * 7), c_00ff00, font18.get(), " 地面のクォリティ     : x%d", gndx);
		DrawFormatStringToHandle(x_r(0), y_r(18 * 8), c_00ff00, font18.get(), " 影のクォリティ       : x%d", shadex);
		//
		GetMousePoint(&mousex, &mousey);
		if (inm(x_r(360), y_r(340), x_r(400), y_r(740))) {
			m = c_ffff00;
			if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) {
				m = c_ff0000;
				++x;
				if (x == 1) {
					l++;
					i++;
					if (i > vecs.size() - 1) {
						i = 0;
					}
				}
			}
			else {
				x = 0;
			}
		}
		else {
			m = c_00ff00;
		}
		DrawBox(x_r(360), y_r(340), x_r(400), y_r(740), m, FALSE);
		font18.DrawString(x_r(396) - font18.GetDrawWidth("<"), y_r(531), "<", c_ffffff);
		//
		if (inm(x_r(1520), y_r(340), x_r(1560), y_r(740))) {
			m = c_ffff00;
			if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) {
				m = c_ff0000;
				++y;
				if (y == 1) {
					l--;
					i--;
					if (i < 0) {
						i = int(vecs.size() - 1);
					}
				}
			}
			else {
				y = 0;
			}
		}
		else {
			m = c_00ff00;
		}
		DrawBox(x_r(1520), y_r(340), x_r(1560), y_r(740), m, FALSE);
		font18.DrawString(x_r(1524), y_r(531), ">", c_ffffff);
		//
		if (inm(x_r(760), y_r(960), x_r(1160), y_r(996))) {
			m = c_ffc800;
			if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) {
				break;
			}
		}
		else {
			m = c_ff6400;
		}
		DrawBox(x_r(760), y_r(960), x_r(1160), y_r(996), m, FALSE);
		font18.DrawString(x_r(960) - font18.GetDrawWidth("戦闘開始") / 2, y_r(969), "戦闘開始", c_ffffff);
		Screen_Flip(waits);
	}
	if (i != -1) {
		const auto c_000000 = GetColor(0, 0, 0);
		float unt = 0;
		while (ProcessMessage() == 0 && unt <= 0.9f) {
			waits = GetNowHiPerformanceCount();
			SetDrawScreen(DX_SCREEN_BACK);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * unt));
			DrawBox(0, 0, dispx, dispy, c_000000, TRUE);
			differential(unt, 1.f, 0.05f);
			Screen_Flip(waits);
		}
	}
	return i;
}
void Myclass::set_viewrad(VECTOR vv) {
	view = vv;
	view_r = vv;
}
void Myclass::set_view_r(void) {
	int px, py;
	GetMousePoint(&px, &py);
	view.z += (float)GetMouseWheelRotVol() / 10.0f;
	if (view.z < 0.1f) {
		view.z = 0.1f;
		view_r.z = view.z;
	}
	if (view.z > 2.f) {
		view.z = 2.f;
	}
	//x_r(960), y_r(540)
	view.y += (float)(px - dispx / 2) / dispx * dispx / 640 * 1.0f;
	view.x += (float)(py - dispy / 2) / dispy * dispy / 480 * 1.0f;
	view.x = std::min<float>(deg2rad(35), std::max<float>(deg2rad(-35), view.x));//limit
	view_r = VAdd(view_r, VScale(VSub(view, view_r), 0.1f));
	SetMousePoint(x_r(960), y_r(540));
}
void Myclass::Screen_Flip(LONGLONG waits) {
	ScreenFlip();
	if (!YSync) {
		do { } while (GetNowHiPerformanceCount() - waits < 1000000.0f / f_rate); }
}
Myclass::~Myclass() {
	Effkseer_End();
	DxLib_End();
}
void Myclass::set_se_vol(unsigned char size) {
	for (auto&& s : se_)
		ChangeVolumeSoundMem(size, s.get());
}
void Myclass::play_sound(int p1) {
	PlaySoundMem(se_[p1].get(), DX_PLAYTYPE_BACK, TRUE);
}
//
HUMANS::HUMANS(bool useg, float frates) {
	usegrab = useg;
	f_rate = frates;

	WIN32_FIND_DATA win32fdt;
	HANDLE hFind;

	hFind = FindFirstFile("data/chara/*", &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((win32fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (win32fdt.cFileName[0] != '.')) {
				name.emplace_back(win32fdt.cFileName);
			}
		} while (FindNextFile(hFind, &win32fdt));
	} //else{ return false; }
	FindClose(hFind);
}
void HUMANS::set_humans(const MV1ModelHandle& inmod) {
	using namespace std::literals;
	//load
	SetUseASyncLoadFlag(FALSE);
	inmodel_handle = inmod.Duplicate();
	inflames = inmodel_handle.frame_num();
	pos_old.resize(inflames);
	hum.resize((inflames - bone_in_turret >= 1) ? inflames - bone_in_turret : 1);
	//読み込み
	//todo : ここでキャラ選択

	for (auto&& h : hum) {
		if (usegrab)
			MV1SetLoadModelUsePhysicsMode(DX_LOADMODEL_PHYSICS_REALTIME);
		else
			MV1SetLoadModelUsePhysicsMode(DX_LOADMODEL_PHYSICS_LOADCALC);
		h.obj = MV1ModelHandle::Load("data/chara/"s + name[0] + "/model.mv1");
	}
	//読み込み後の設定、読み込み
	for (auto&& h : hum) {
		//色調
		for (int j = 0; j < h.obj.material_num(); ++j) {
			MV1SetMaterialDifColor(h.obj.get(), j, GetColorF(0.5f, 0.5f, 0.5f, 1.0f));
			MV1SetMaterialSpcColor(h.obj.get(), j, GetColorF(0.35f, 0.32f, 0.28f, 0.5f));
			MV1SetMaterialSpcPower(h.obj.get(), j, 1.0f);
		}
		//アニメーション
		for (size_t j = 0; j < animes; ++j) {
			h.amine[j] = MV1AttachAnim(h.obj.get(), int(j), -1, TRUE);
			h.time[j] = 0.0f;
			h.alltime[j] = MV1GetAttachAnimTotalTime(h.obj.get(), h.amine[j]);
			h.per[j] = 0.f;
		}
	}
	//車長にはボイスと口パクアニメーション
	for (size_t i = 0; i < voice; ++i) {
		hum[0].voices[i] = MV1AttachAnim(hum[0].obj.get(), int(animes + i), -1, TRUE);
		hum[0].voicealltime[i] = MV1GetAttachAnimTotalTime(hum[0].obj.get(), hum[0].voices[i]);
		hum[0].vsound[i] = SoundHandle::Load("data/chara/"s + name[0] + "/" + std::to_string(i) + ".wav");
	}
	//
	MV1SetMatrix(inmodel_handle.get(), MGetTranslate(VGet(0, 0, 0)));
	for (int i = 0; i < inflames; i++) {
		locin.emplace_back(inmodel_handle.frame(i));
		MV1ResetFrameUserLocalMatrix(inmodel_handle.get(), i);
	}
	//首フレームを探す
	hum[0].neck = 0;
	for (const TCHAR* re; "首"sv != (re = MV1GetFrameName(hum[0].obj.get(), hum[0].neck++));) {}
	//if (hum[0].neck == 0) { hum[0].neck = 121; } //暫定処置
	MV1SetMatrix(hum[0].obj.get(), MGetTranslate(VGet(0, 0, 0)));
	hum[0].nvec = VSub(hum[0].obj.frame(hum[0].neck), hum[0].obj.frame(hum[0].neck - 1));
	first = false;
	hum[0].vflug = -1;
	return;
}
void HUMANS::set_humanvc_vol(unsigned char size) {
	for (auto&& v : hum[0].vsound)
		ChangeVolumeSoundMem(size, v.get());
}
void HUMANS::set_humanmove(const players& player, VECTOR rad) {
	if (!first)
		MV1SetMatrix(inmodel_handle.get(), player.ps_m);
	for (int i = 0; i < inflames; ++i)
		pos_old[i] = inmodel_handle.frame(i);

	MV1SetMatrix(inmodel_handle.get(), player.ps_m);
	MV1SetFrameUserLocalMatrix(inmodel_handle.get(), player.ptr->turretframe, player.ps_t);
	MV1SetFrameUserLocalMatrix(inmodel_handle.get(), player.ptr->gunframe[0], MMult(MMult(MGetRotX(player.gunrad.y), MGetTranslate(VSub(locin[player.ptr->gunframe[0]], locin[player.ptr->turretframe]))), player.ps_t));
	MV1SetFrameUserLocalMatrix(inmodel_handle.get(), player.ptr->gunframe[0], MGetTranslate(VAdd(VSub(player.ptr->loc[player.ptr->gunframe[0] + 1], player.ptr->loc[player.ptr->gunframe[0]]), VGet(0, 0, player.Gun[0].fired))));
	//
	for (int i = bone_hatch; i < inflames; ++i) {
		//警告	C6289	不適切な演算子です : || を使用した相互排除は常に 0 でない定数となります。 && を使用しようとしましたか ?
		if (i != 9 || i != 10)
			MV1SetFrameUserLocalMatrix(inmodel_handle.get(), i, MMult(MMult(MGetRotY(player.gunrad.x), MGetTranslate(VSub(locin[i], locin[player.ptr->turretframe]))), player.ps_t));
	}

	if (rad.z >= 0.1f)
		in_f = false;

	if (rad.z == 0.1f && !in_f) {
		for (size_t j = 1; j < hum.size(); ++j) {
			MV1PhysicsResetState(hum[j].obj.get());
		}
		in_f = true;
	}
	{
		auto& h = hum.front();
		h.per[0] = 0.f;
		if (player.speed >= 30.f / 3.6f) {
			h.per[1] += (1.0f - h.per[1]) * 0.1f;
		}
		else {
			h.per[1] *= 0.9f;
		} /*座る*/
		h.per[2] = 1.f;
		if (h.vflug == -1) {
			h.per[3] *= 0.9f;
		}
		else {
			h.per[3] += (1.0f - h.per[3]) * 0.1f;
		}
		h.per[0] = 1.0f - h.per[3]; /*無線*/
	}
	std::for_each(hum.begin() + 1, hum.end(), [](HUMANS::humans& e) { e.per = { 1, 1, 1, 0 }; });
	//反映
	for (int k = 0; k < divi; ++k) {
		for (size_t i = 0; i < hum.size(); ++i) {
			auto& h = hum[i];
			MV1SetMatrix(
			    h.obj.get(),
			    MMult(
				MMult(
				    MGetRotY(player.yrad + player.gunrad.x),
				    MGetRotVec2(VGet(0, 1.f, 0), player.nor)),
				MGetTranslate(
				    VAdd(
					pos_old[bone_in_turret + i],
					VScale(
					    VSub(
						inmodel_handle.frame(int(bone_in_turret + i)),
						pos_old[bone_in_turret + i]),
					    (float)(1 + k) / divi))))); //MMult(MGetRotX(player.xnor), MGetRotZ(player.znor))
			if (i == 0) {
				/*首振り*/
				MV1SetFrameUserLocalMatrix(
				    h.obj.get(),
				    h.neck,
				    MMult(
					MGetTranslate(h.nvec),
					MMult(
					    MGetRotX(std::clamp(-rad.x, deg2rad(-20), deg2rad(20))),
					    MGetRotY(std::clamp(atanf(sin(rad.y - player.yrad - player.gunrad.x)), deg2rad(-40), deg2rad(40))))));
				//voice
				if (h.vflug != -1) {
					if (h.voicetime < h.voicealltime[h.vflug]) {
						if (h.voicetime == 0.0f) {
							PlaySoundMem(h.vsound[h.vflug].get(), DX_PLAYTYPE_BACK, TRUE);
						}
						MV1SetAttachAnimTime(h.obj.get(), h.voices[h.vflug], h.voicetime);
						h.voicetime += 60.0f / divi / f_rate; //
					}
					else {
						h.vflug = -1;
						h.voicetime = 0.0f;
					}
				}
			}
			for (size_t j = 0; j < animes; ++j) {
				MV1SetAttachAnimBlendRate(h.obj.get(), h.amine[j], h.per[j]);
				MV1SetAttachAnimTime(h.obj.get(), h.amine[j], h.time[j]);
				h.time[j] += 30.0f / divi / f_rate; //
				if (j != 2 && h.time[j] >= h.alltime[j]) {
					h.time[j] = 0.0f;
				}
			}
			//
			if (usegrab) {
				if (!first) {
					MV1PhysicsResetState(h.obj.get());
				}
				else {
					MV1PhysicsCalculation(h.obj.get(), 1000.0f / divi / f_rate);
				}
			}
			//
			if (i == 0 && !in_f) {
				break;
			}
		}
	}
	first = true;
}
void HUMANS::draw_human(size_t p1) {
	MV1DrawModel(hum[p1].obj.get());
}
void HUMANS::draw_humanall() {
	MV1DrawModel(inmodel_handle.get());
	for (size_t i = 0; i < hum.size(); ++i) {
		draw_human(i);
	}
}
void HUMANS::delete_human(void) {
	inmodel_handle.Dispose();
	hum.clear();
	locin.clear();
	pos_old.clear();
}
void HUMANS::start_humanvoice(std::int8_t p1) {
	hum[0].vflug = p1;
}
void HUMANS::start_humananime(int p1) {
	for (auto&& h : hum)
		h.time.at(p1) = 0.0f;
}
//
MAPS::MAPS(int map_size, float draw_dist, int shadow_size) {
	groundx = map_size * 1024; /*ノーマルマップのサイズ*/
	drawdist = draw_dist;      /*木の遠近*/
	shadowx = shadow_size;
	int shadowsize = (1 << (10 + shadowx));
	//shadow----------------------------------------------------------------------------------------//
	shadow_near = MakeShadowMap(shadowsize, shadowsize);     /*近影*/
	SetShadowMapAdjustDepth(shadow_near, 0.0005f);		 /*ずれを小さくするため*/
	shadow_seminear = MakeShadowMap(shadowsize, shadowsize); /*近影*/
	shadow_far = MakeShadowMap(shadowsize, shadowsize);      /*マップ用*/
	//map-------------------------------------------------------------------------------------------//
	SetUseASyncLoadFlag(TRUE);
	sky_sun = GraphHandle::Load("data/sun.png");       /*太陽*/
	texo = GraphHandle::Load("data/nm.png");	   /*轍*/
	texp = GraphHandle::Make(groundx, groundx, FALSE); /*ノーマルマップ*/
	texn = GraphHandle::Make(groundx, groundx, FALSE); /*実マップ*/
	SetUseASyncLoadFlag(FALSE);
}
void MAPS::set_map_readyb(size_t set) {
	using namespace std::literals;
	lightvec = VGet(0.5f, -0.2f, 0.5f);
	std::array<const char*, 2> mapper{ "map", "map" }; // TODO: 書き換える
	SetUseASyncLoadFlag(TRUE);
	tree.mnear = MV1ModelHandle::Load("data/"s + mapper.at(set) + "/tree/model.mv1");     /*近木*/
	tree.mfar = MV1ModelHandle::Load("data/"s + mapper.at(set) + "/tree/model2.mv1");     /*遠木*/
	texl = GraphHandle::Load("data/"s + mapper.at(set) + "/SandDesert_04_00344_FWD.png"); /*nor*/
	texm = GraphHandle::Load("data/"s + mapper.at(set) + "/SandDesert_04_00344_NM.png");  /*nor*/
	m_model = MV1ModelHandle::Load("data/"s + mapper.at(set) + "/map.mv1");		      /*map*/
	sky_model = MV1ModelHandle::Load("data/"s + mapper.at(set) + "/sky/model_sky.mv1");   /*sky*/
	graph = GraphHandle::Load("data/"s + mapper.at(set) + "/grass/grass.png");	    /*grass*/
	grass = MV1ModelHandle::Load("data/"s + mapper.at(set) + "/grass/grass.mv1");	 /*grass*/
	GgHandle = GraphHandle::Load("data/"s + mapper.at(set) + "/grass/gg.png");	    /*地面草*/
	SetUseASyncLoadFlag(FALSE);
	return;
}
bool MAPS::set_map_ready() {
	tree.nears.resize(treec);
	tree.fars.resize(treec);
	tree.treesort.resize(treec);
	tree.pos.resize(treec);
	tree.rad.resize(treec);

	MV1SetScale(sky_model.get(), VGet(0.2f, 0.2f, 0.2f));

	SetUseASyncLoadFlag(TRUE);
	for (size_t j = 0; j < treec; ++j) {
		tree.nears[j] = tree.mnear.Duplicate();
		tree.fars[j] = tree.mfar.Duplicate();
		tree.hit.push_back(true); //
	}
	SetUseASyncLoadFlag(FALSE);

	MV1SetupCollInfo(m_model.get(), 0, map_x / 5, map_x / 5, map_y / 5);
	SetFogStartEnd(10.0f, 1400.0f); /*fog*/
	SetFogColor(150, 150, 175);     /*fog*/
	SetLightDirection(lightvec);
	SetShadowMapLightDirection(shadow_near, lightvec);
	SetShadowMapLightDirection(shadow_seminear, lightvec);
	SetShadowMapLightDirection(shadow_far, lightvec);
	SetShadowMapDrawArea(shadow_far, VGet(-(float)map_x / 2.f, -(float)map_x / 2.f, -(float)map_y / 2.f), VGet((float)map_x / 2.f, (float)map_x / 2.f, (float)map_y / 2.f));

	constexpr uint8_t rate = 96;

	SetDrawScreen(texp.get());
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	for (uint8_t x = 0; x < rate; x++)
		for (uint8_t y = 0; y < rate; y++)
			DrawExtendGraph(groundx * x / rate, groundx * y / rate, groundx * (x + 1) / rate, groundx * (y + 1) / rate, texl.get(), FALSE);
	MV1SetTextureGraphHandle(m_model.get(), 0, texp.get(), FALSE);
	SetDrawScreen(texn.get());
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(0, 0, groundx, groundx, GetColor(121, 121, 255), TRUE);
	for (uint8_t x = 0; x < 32; x++)
		for (uint8_t y = 0; y < 32; y++)
			DrawExtendGraph(groundx * x / rate, groundx * y / rate, groundx * (x + 1) / rate, groundx * (y + 1) / rate, texm.get(), TRUE);
	MV1SetTextureGraphHandle(m_model.get(), 1, texn.get(), FALSE);
	/*grass*/
	vnum = 0;
	pnum = 0;
	MV1SetupReferenceMesh(grass.get(), -1, TRUE);	 /*参照用メッシュの作成*/

	RefMesh = MV1GetReferenceMesh(grass.get(), -1, TRUE); /*参照用メッシュの取得*/
	/*todo : mv1にしとく*/

	IndexNum = RefMesh.PolygonNum * 3 * grasss;	   /*インデックスの数を取得*/
	VerNum = RefMesh.VertexNum * grasss;		      /*頂点の数を取得*/

	grassver.resize(VerNum);   /*頂点データとインデックスデータを格納するメモリ領域の確保*/
	grassind.resize(IndexNum); /*頂点データとインデックスデータを格納するメモリ領域の確保*/

	for (int i = 0; i < grasss; ++i) {
		const auto tmpvect = VGet((float)(-map_x * 5 + GetRand(map_x * 10)) / 10.0f, 0.0f, (float)(-map_y * 5 + GetRand(map_y * 10)) / 10.0f);
		//
		SetDrawScreen(texp.get());
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 64);
		DrawRotaGraph((int)(groundx * (0.5f + tmpvect.x / (float)map_x)), (int)(groundx * (0.5f - tmpvect.z / (float)map_y)), 8.f * groundx / 1024 / 128.0f, 0, GgHandle.get(), TRUE);
		const auto HitPoly = MV1CollCheck_Line(m_model.get(), 0, VAdd(tmpvect, VGet(0.0f, (float)map_x, 0.0f)), VAdd(tmpvect, VGet(0.0f, -(float)map_x, 0.0f)));
		if (HitPoly.HitFlag)
			MV1SetMatrix(grass.get(), MMult(MGetScale(VGet((float)(200 + GetRand(400)) / 100.0f, (float)(25 + GetRand(100)) / 100.0f, (float)(200 + GetRand(400)) / 100.0f)), MMult(MMult(MGetRotY(deg2rad(GetRand(360))), MGetRotVec2(VGet(0, 1.f, 0), HitPoly.Normal)), MGetTranslate(HitPoly.HitPosition))));
		//上省
		MV1RefreshReferenceMesh(grass.get(), -1, TRUE);       /*参照用メッシュの更新*/
		RefMesh = MV1GetReferenceMesh(grass.get(), -1, TRUE); /*参照用メッシュの取得*/
		for (size_t j = 0; j < size_t(RefMesh.VertexNum); ++j) {
			grassver[j + vnum].pos = RefMesh.Vertexs[j].Position;
			grassver[j + vnum].norm = RefMesh.Vertexs[j].Normal;
			grassver[j + vnum].dif = RefMesh.Vertexs[j].DiffuseColor;
			grassver[j + vnum].spc = RefMesh.Vertexs[j].SpecularColor;
			grassver[j + vnum].u = RefMesh.Vertexs[j].TexCoord[0].u;
			grassver[j + vnum].v = RefMesh.Vertexs[j].TexCoord[0].v;
			grassver[j + vnum].su = RefMesh.Vertexs[j].TexCoord[1].u;
			grassver[j + vnum].sv = RefMesh.Vertexs[j].TexCoord[1].v;
		}
		for (size_t j = 0; j < size_t(RefMesh.PolygonNum); ++j) {
			for (size_t k = 0; k < std::size(RefMesh.Polygons[j].VIndex); ++k) {
				grassind[j * 3 + k + pnum] = WORD(RefMesh.Polygons[j].VIndex[k] + vnum);
			}
		}
		vnum += RefMesh.VertexNum;
		pnum += RefMesh.PolygonNum * 3;
	}
	VerBuf = CreateVertexBuffer(VerNum, DX_VERTEX_TYPE_NORMAL_3D);
	IndexBuf = CreateIndexBuffer(IndexNum, DX_INDEX_TYPE_32BIT);
	SetVertexBufferData(0, grassver.data(), VerNum, VerBuf);
	SetIndexBufferData(0, grassind.data(), IndexNum, IndexBuf);
	MV1SetTextureGraphHandle(m_model.get(), 0, texp.get(), FALSE);
	/*tree,shadow*/
	ShadowMap_DrawSetup(shadow_far);
	for (size_t i = 0; i < treec; ++i) {
		const auto tmpvect = VGet((float)(-map_x * 5 + GetRand(map_x * 10)) / 10.0f, 0.0f, (float)(-map_y * 5 + GetRand(map_y * 10)) / 10.0f);
		const auto HitPoly = MV1CollCheck_Line(m_model.get(), 0, VAdd(tmpvect, VGet(0.0f, 100.0f, 0.0f)), VAdd(tmpvect, VGet(0.0f, -100.0f, 0.0f)));
		tree.pos[i] = (HitPoly.HitFlag) ? HitPoly.HitPosition : tmpvect;
		tree.rad[i] = VGet(0.0f, deg2rad(GetRand(360)), 0.0f);
		MV1SetPosition(tree.nears[i].get(), tree.pos[i]);
		MV1SetPosition(tree.fars[i].get(), tree.pos[i]);
		MV1SetRotationXYZ(tree.nears[i].get(), tree.rad[i]);
		MV1DrawModel(tree.nears[i].get());
		MV1SetMaterialDrawAlphaTestAll(tree.nears[i].get(), TRUE, DX_CMP_GREATER, 128);
	}
	MV1DrawModel(m_model.get());
	ShadowMap_DrawEnd();
	return true;
}
void MAPS::set_camerapos(VECTOR pos, VECTOR vec, VECTOR up, float ratio) {
	camera = pos;
	viewv = vec;
	upv = up;
	rat = ratio;
}
void MAPS::set_map_shadow_near(float vier_r) {
	float shadow_dist = std::max(20.f, 10.0f * float(shadowx) * vier_r + 20.0f);
	SetShadowMapDrawArea(shadow_near, VSub(camera, VScale(VGet(1.0f, 1.0f, 1.0f), shadow_dist)), VAdd(camera, VScale(VGet(1.0f, 1.0f, 1.0f), shadow_dist)));
	SetShadowMapDrawArea(shadow_seminear, VSub(camera, VScale(VGet(1.0f, 1.0f, 1.0f), shadow_dist * 2)), VAdd(camera, VScale(VGet(1.0f, 1.0f, 1.0f), shadow_dist * 2)));
}
void MAPS::draw_map_track(const players& player) {
	SetDrawScreen(texn.get());
	for (auto& w : player.ptr->wheelframe) {
		if (player.Springs[w] >= -0.15f) {
			VECTOR tempvec = player.obj.frame(w);
			DrawRotaGraph((int)(groundx * (0.5f + tempvec.x / (float)map_x)), (int)(groundx * (0.5f - tempvec.z / (float)map_y)), 1.f * groundx / 1024 / 195.0f, player.yrad, texo.get(), TRUE);
		}
	}
}
void MAPS::draw_map_model() {
	MV1DrawModel(m_model.get());
}
void MAPS::set_map_track() {
	MV1SetTextureGraphHandle(m_model.get(), 1, texn.get(), FALSE);
}
void MAPS::draw_map_sky(void) {
	ClearDrawScreen();
	setcv(50.0f, 200.0f, camera, viewv, upv, 45.0f / rat);
	SetUseLighting(FALSE);
	SetFogEnable(FALSE);

	MV1SetPosition(sky_model.get(), camera);
	MV1DrawModel(sky_model.get());
	DrawBillboard3D(VAdd(camera, VScale(VNorm(lightvec), -80.0f)), 0.5f, 0.5f, 10.0f, 0.0f, sky_sun.get(), TRUE);

	SetFogEnable(TRUE);
	SetUseLighting(TRUE);
}
void MAPS::set_hitplayer(VECTOR pos) {
	for (size_t j = 0; j < treec; ++j) {
		if (tree.hit[j]) {
			if (VSize(VSub(tree.pos[j], pos)) <= 3.f) {
				tree.hit[j] = false;
				tree.rad[j].y = atan2(VSub(tree.pos[j], pos).x, VSub(tree.pos[j], pos).z);
			}
		}
		else {
			if (tree.rad[j].x <= deg2rad(85)) {
				MV1SetRotationXYZ(tree.nears[j].get(), tree.rad[j]);
				differential(tree.rad[j].x, deg2rad(90), 0.002f);
			}
		}
	}
}
void MAPS::draw_trees() {

	for (size_t j = 0; j < treec; ++j) {
		if (CheckCameraViewClip_Box(VAdd(tree.pos[j], VGet(-10, 0, -10)), VAdd(tree.pos[j], VGet(10, 10, 10))))
			tree.treesort[j] = pair(j, (float)map_x);
		else
			tree.treesort[j] = pair(j, VSize(VSub(tree.pos[j], camera)));
	}
	std::sort(tree.treesort.begin(), tree.treesort.end(), [](const pair& x, const pair& y) { return x.second > y.second; });

	for (auto& tt : tree.treesort) {
		if (tt.second == (float)map_x)
			continue;
		const auto k = tt.first;
		if (tt.second > drawdist) {
			const auto per = (tt.second < drawdist + 100) ? (tt.second - drawdist) / 100.0f : 1.f;
			if (per > 0) {
				MV1SetOpacityRate(tree.fars[k].get(), per);
				const auto vect = VSub(tree.pos[k], camera);
				MV1SetRotationXYZ(tree.fars[k].get(), VGet(0.0f, atan2(vect.x, vect.z), 0.0f));
				MV1DrawModel(tree.fars[k].get());
			}
		}
		if (tt.second < drawdist + 100) {
			auto per = (tt.second < 20) ? -0.5f + tt.second / 20.0f : 1.f;
			if (tt.second > drawdist)
				per = 1.0f - (tt.second - drawdist) / 100.0f;
			if (per > 0)
				MV1DrawModel(tree.nears[k].get());
		}
	}
}
void MAPS::draw_grass(void) {
	DrawPolygonIndexed3D_UseVertexBuffer(VerBuf, IndexBuf, graph.get(), TRUE);
}
void MAPS::delete_map(void) {
	m_model.Dispose();
	sky_model.Dispose();
	tree.mnear.Dispose();
	tree.mfar.Dispose();
	for (size_t j = 0; j < treec; ++j) {
		tree.nears[j].Dispose();
		tree.fars[j].Dispose();
	}
	tree.treesort.clear();
	graph.Dispose();
	grass.Dispose();
	texl.Dispose();
	texm.Dispose();
	tree.nears.clear();
	tree.fars.clear();
	tree.pos.clear();
	tree.rad.clear();
	tree.hit.clear();

	grassver.clear();
	grassind.clear();
}
void MAPS::ready_shadow(void) {
	SetUseShadowMap(0, shadow_near);
	SetUseShadowMap(1, shadow_far);
	SetUseShadowMap(2, shadow_seminear);
}
void MAPS::exit_shadow(void) {
	SetUseShadowMap(0, -1);
	SetUseShadowMap(1, -1);
	SetUseShadowMap(2, -1);
}
void MAPS::set_normal(float* xnor, float* znor, VECTOR position) {
	/*X*/
	const auto r0_0 = get_gnd_hit(VAdd(position, VGet(0.0f, 2.0f, -0.5f)), VAdd(position, VGet(0.0f, -2.0f, -0.5f)));
	if (r0_0.HitFlag) {
		const auto r0_1 = get_gnd_hit(VAdd(position, VGet(0.0f, 2.0f, 0.5f)), VAdd(position, VGet(0.0f, -2.0f, 0.5f)));
		if (r0_1.HitFlag)
			differential(*xnor, atan2(r0_0.HitPosition.y - r0_1.HitPosition.y, 1.0f), 0.05f);
	}
	/*Z*/
	const auto r1_0 = get_gnd_hit(VAdd(position, VGet(0.5f, 2.0f, 0.0f)), VAdd(position, VGet(0.5f, -2.0f, 0.0f)));
	if (r1_0.HitFlag) {
		const auto r1_1 = get_gnd_hit(VAdd(position, VGet(-0.5f, 2.0f, 0.0f)), VAdd(position, VGet(-0.5f, -2.0f, 0.0f)));
		if (r1_1.HitFlag)
			differential(*znor, atan2(r1_0.HitPosition.y - r1_1.HitPosition.y, 1.0f), 0.05f);
	}
}
//
UIS::UIS() {
	using namespace std::literals;
	WIN32_FIND_DATA win32fdt;

	countries = 1;//国の数

	UI_main.resize(countries); /*改善*/
	SetUseASyncLoadFlag(TRUE);
	for (size_t j = 0; j < std::size(ui_reload); ++j) {
		ui_reload[j] = GraphHandle::Load("data/ui/ammo_" + std::to_string(j) + ".bmp");
	} /*弾0,弾1,弾2,空弾*/
	const auto hFind = FindFirstFile("data/ui/body/*.png", &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (win32fdt.cFileName[0] == 'B') {
				UI_body.emplace_back(GraphHandle::Load(("data/ui/body/"s + win32fdt.cFileName).c_str()));
			}
			if (win32fdt.cFileName[0] == 'T') {
				UI_turret.emplace_back(GraphHandle::Load(("data/ui/body/"s + win32fdt.cFileName).c_str()));
			}
		} while (FindNextFile(hFind, &win32fdt));
	} //else{ return false; }
	FindClose(hFind);
	for (size_t j = 0; j < countries; ++j) {
		// TODO: Germanの部分は可変になる
		for (size_t i = 0; i < 8; ++i) {
			UI_main[j].ui_sight[i] = GraphHandle::Load("data/ui/German/" + std::to_string(i) + ".png");
		}
	}
	SetUseASyncLoadFlag(FALSE);
}
void UIS::draw_load(void) {
	const auto font18 = FontHandle::Create(x_r(18), y_r(18 / 3), DX_FONTTYPE_ANTIALIASING);
	SetUseASyncLoadFlag(TRUE);
	auto pad = GraphHandle::Load("data/key.png");
	SetUseASyncLoadFlag(FALSE);
	const int pp = GetASyncLoadNum();
	const auto c_00ff00 = GetColor(0, 255, 0);
	const auto c_ff0000 = GetColor(255, 0, 0);
	const auto c_ff6400 = GetColor(255, 100, 0);
	const auto c_00c800 = GetColor(0, 200, 0);
	const auto c_3264ff = GetColor(50, 100, 255);
	while (ProcessMessage() == 0 && GetASyncLoadNum() != 0) {
		const auto waits = GetNowHiPerformanceCount();
		SetDrawScreen(DX_SCREEN_BACK);
		ClearDrawScreen();
		//
		const float pers = (float)(pp - GetASyncLoadNum()) / pp;
		DrawBox(x_r(0), y_r(1080 - 6), x_r(1920.f * pers), y_r(1080 - 3), c_00ff00, TRUE);
		DrawFormatStringToHandle(x_r(0), y_r(1080 - 24), c_00ff00, font18.get(), "LOADING : %06.2f%%", pers * 100.f);
		//
		DrawExtendGraph(x_r(552), y_r(401), x_r(1367), y_r(679), pad.get(), TRUE);
		int i = 0;
		font18.DrawString(x_r(1367), y_r(401 + i), "W : 前進", c_ff0000);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "S : 後退", c_ff0000);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "A : 左転", c_ff0000);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "D : 右転", c_ff0000);
		i += 18;
		//font18.DrawString(x_r(1367), y_r(401 + i), "R : シフトアップ", c_ff0000); i += 18;
		//font18.DrawString(x_r(1367), y_r(401 + i), "F : シフトダウン", c_ff0000); i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "↑ : 砲昇", c_ff6400);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "↓ : 砲降", c_ff6400);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "← : 砲左", c_ff6400);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "→ : 砲右", c_ff6400);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "左CTRL : 精密砲操作", c_ff6400);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "左shift : 照準", c_ff6400);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "Z : レティクル上昇", c_ff6400);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "X : レティクル下降", c_ff6400);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "C : ズームアウト", c_ff6400);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "V : ズームイン", c_ff6400);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "Q : 再装填", c_00c800);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "space : 射撃", c_00c800);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "右shift : 指揮", c_3264ff);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "マウス : 見回し/指揮", c_3264ff);
		i += 18;
		font18.DrawString(x_r(1367), y_r(401 + i), "ESC : 終了", c_3264ff);
		i += 18;
		ScreenFlip();
		while (GetNowHiPerformanceCount() - waits < 1000000.0f / 60.f) {}
	}
	SetDrawScreen(DX_SCREEN_BACK);
	ClearDrawScreen();
	ScreenFlip();
}
void UIS::set_state(players* play) {
	pplayer = play;
}
void UIS::set_reco(void) {
	recs = 1.f;
}
void UIS::draw_sight(float posx, float posy, float ratio, float dist, int font) {
	DrawRotaGraph(x_r(960), y_r(540), (float)y_r(2), deg2rad(-dist / 20), UI_main[pplayer->ptr->countryc].ui_sight[1].get(), TRUE);
	DrawRotaGraph((int)posx, (int)posy, (float)y_r(2) * ratio / 4.0f, 0, UI_main[pplayer->ptr->countryc].ui_sight[2].get(), TRUE);
	DrawRotaGraph(x_r(960), y_r(540), (float)y_r(2), 0, UI_main[pplayer->ptr->countryc].ui_sight[0].get(), TRUE);
	DrawExtendGraph(0, 0, dispx, dispy, UI_main[pplayer->ptr->countryc].ui_sight[7].get(), TRUE);
	DrawFormatStringToHandle(x_r(1056), y_r(594), GetColor(255, 255, 255), font, "[%03d]", (int)dist);
	DrawFormatStringToHandle(x_r(1056), y_r(648), GetColor(255, 255, 255), font, "[x%02.1f]", ratio);
}
void UIS::draw_ui(int selfammo, float y_v) {
	/*跳弾*/
	if (recs >= 0.01f) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(recs * 128.0f));
		DrawBox(0, 0, dispx, dispy, GetColor(255, 255, 255), TRUE);
		recs *= 0.9f;
	}
	/*弾*/
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	if (pplayer->Gun[0].loadcnt > 0) {
		DrawRotaGraph(x_r(2112 - (int)(384 * pplayer->Gun[0].loadcnt / pplayer->ptr->reloadtime[0])), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[pplayer->ammotype].get(), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(128.0f * pow(1.0f - (float)pplayer->Gun[0].loadcnt / (float)pplayer->ptr->reloadtime[0], 10)));
		if (selfammo == 0)
			DrawRotaGraph(x_r(1536), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[3].get(), TRUE);
		else 
			DrawRotaGraph(x_r(1536), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[(pplayer->ammotype - 1) % 3].get(), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	if (pplayer->Gun[0].loadcnt == 0)
		DrawRotaGraph(x_r(1536), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[pplayer->ammotype].get(), TRUE);
	DrawRotaGraph(x_r(1728 - (int)(192 * pplayer->Gun[0].loadcnt / pplayer->ptr->reloadtime[0])), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[pplayer->ammotype].get(), TRUE);
	DrawRotaGraph(x_r(1760), y_r(128), (double)x_r(40) / 40.0, 0.0, ui_reload[(pplayer->ammotype + 1) % 3].get(), TRUE);
	DrawRotaGraph(x_r(1792), y_r(192), (double)x_r(40) / 40.0, 0.0, ui_reload[(pplayer->ammotype + 2) % 3].get(), TRUE);
	/*速度計*/
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawExtendGraph(x_r(0), y_r(888), x_r(192), y_r(1080), UI_main[pplayer->ptr->countryc].ui_sight[3].get(), TRUE);
	DrawRotaGraph(x_r(96), y_r(984), x_r(192) / 152, deg2rad(120.0f * pplayer->speed / pplayer->ptr->speed_flont[3] - 60.f), UI_main[pplayer->ptr->countryc].ui_sight[4].get(), TRUE);

	SetDrawArea(x_r(192), y_r(892), x_r(192 + 40), y_r(892 + 54));
	DrawRotaGraph(x_r(192 + 40 / 2), y_r(892 + 54 / 2 + (int)(54.0f * gearf)), (double)x_r(40) / 40.0, 0.f, UI_main[pplayer->ptr->countryc].ui_sight[5].get(), TRUE);
	SetDrawArea(x_r(0), y_r(0), x_r(1920), y_r(1080));

	DrawExtendGraph(x_r(192), y_r(892 - 4), x_r(232), y_r(950), UI_main[pplayer->ptr->countryc].ui_sight[6].get(), TRUE);
	differential(gearf, (float)pplayer->gear, 0.1f);

	for (size_t i = 0; i < UI_body.size(); ++i) {
		if (i >= 1 && i <= 2) {
			const auto pers = 1.f - (float)pplayer->HP[4 + i] / 100.f;
			if (pers <= 0.2f)
				SetDrawBright(50, 50, 255);
			else
				SetDrawBright((int)(255.f * sin(pers * DX_PI_F / 2)), (int)(255.f * cos(pers * DX_PI_F / 2)), 0);
		}
		else
			SetDrawBright(255, 255, 255);
		DrawRotaGraph(x_r(392), y_r(980), (double)x_r(40) / 40.0, double(-y_v + pplayer->yrad), UI_body[i].get(), TRUE);
	}
	for (int i = 0; i < UI_turret.size(); ++i) {
		if (i == 0) {
			const auto pers = 1.f - (float)pplayer->HP[4 + i] / 100.f;
			if (pers <= 0.2f)
				SetDrawBright(50, 50, 255);
			else
				SetDrawBright((int)(255.f * sin(pers * DX_PI_F / 2)), (int)(255.f * cos(pers * DX_PI_F / 2)), 0);
		}
		else
			SetDrawBright(255, 255, 255);
		DrawRotaGraph(x_r(392), y_r(980), (double)x_r(40) / 40.0, double(-y_v + pplayer->yrad + pplayer->gunrad.x), UI_turret[i].get(), TRUE);
	}

	const auto c_ff0000 = GetColor(255, 0, 0);
	DrawLine(x_r(392), y_r(980), x_r(392 - int(100.f * sin(y_v))), y_r(980 - int(100.f * cos(y_v))), c_ff0000);

	//DrawFormatString(x_r(1056), y_r(594), GetColor(255, 255, 255), "[%03d][%03d]", UI_body.size(),UI_turret.size());
}
/*debug*/
void UIS::put_way(void) {
	waypoint = GetNowHiPerformanceCount();
	seldeb = 0;
}
void UIS::end_way(void) {
	if (seldeb < 6) {
		waydeb[seldeb] = (float)(GetNowHiPerformanceCount() - waypoint) / 1000.0f;
		seldeb++;
	}
}
void UIS::debug(float fps, float time) {
	deb[0][0] = time;
	for (size_t j = std::size(deb) - 1; j >= 1; --j) {
		deb[j][0] = deb[j - 1][0];
	}
	for (size_t i = 0; i < std::size(waydeb); ++i) {
		if (seldeb - 1 <= i) {
			waydeb[i] = waydeb[seldeb - 1];
		}
		deb[0][i + 1] = waydeb[i];
		for (size_t j = std::size(deb) - 1; j >= 1; --j) {
			deb[j][i + 1] = deb[j - 1][i + 1];
		}
	}
	const auto c_ffff00 = GetColor(255, 255, 0);
	for (int j = 0; j < 60 - 1; ++j) {
		for (int i = 0; i < 6; ++i) {
			DrawLine(100 + j * 5, 100 + (int)(200.f - deb[j][i + 1] * 10.f), 100 + (j + 1) * 5, 100 + (int)(200.f - deb[j + 1][i + 1] * 10.f), GetColor(50, 50, 128 + 127 * i / 6));
		}
		DrawLine(100 + j * 5, 100 + (int)(200.f - deb[j][0] * 10.f), 100 + (j + 1) * 5, 100 + (int)(200.f - deb[j + 1][0] * 10.f), c_ffff00);
	}
	const auto c_ffffff = GetColor(255, 255, 255);
	DrawLine(100, 100 + 200 - 166, 100 + 60 * 5, 100 + 200 - 166, GetColor(0, 255, 0));
	DrawBox(100, 100 + 0, 100 + 60 * 5, 100 + 200, GetColor(255, 0, 0), FALSE);
	DrawFormatString(100, 100 + 0, c_ffffff, "%05.2ffps (%.2fms)", fps, time);
	DrawFormatString(100, 100 + 18, c_ffffff, "%d(%.2fms)", 0, waydeb[0]);
	for (size_t j = 1; j < std::size(waydeb); ++j) {
		DrawFormatString(100, int(100 + 18 + j * 18), c_ffffff, "%d(%.2fms)", j, waydeb[j] - waydeb[j - 1u]);
	}
}
//
void setcv(float neard, float fard, VECTOR cam, VECTOR view, VECTOR up, float fov) {
	SetCameraNearFar(neard, fard);
	SetCameraPositionAndTargetAndUpVec(cam, view, up);
	SetupCamera_Perspective(deg2rad(fov));
	Set3DSoundListenerPosAndFrontPosAndUpVec(cam, view, up);
}
void getdist(VECTOR* startpos, VECTOR vector, float* dist, float speed, float fps) {
	*dist = std::max(100.f, std::min(2000.f, *dist));
	for (int z = 0; z < (int)(fps / 1000.0f * (*dist)); ++z) {
		*startpos = VAdd(*startpos, VScale(vector, speed / fps));
		vector.y += M_GR / 2.0f / fps / fps;
		speed -= 5.f / fps;
	}
}
//
void set_effect(EffectS* efh, VECTOR pos, VECTOR nor) {
	efh->flug = true;
	efh->pos = pos;
	efh->nor = nor;
}
void set_pos_effect(EffectS* efh, const EffekseerEffectHandle& handle) {
	if (efh->flug) {
		efh->handle = handle.Play3D();
		efh->handle.SetPos(efh->pos);
		efh->handle.SetRotation(atan2(efh->nor.y, std::hypot(efh->nor.x, efh->nor.z)), atan2(-efh->nor.x, -efh->nor.z), 0);
		efh->flug = false;
	}
	//IsEffekseer3DEffectPlaying(player[0].effcs[i].handle)
}
//
bool get_reco(players& play, std::vector<players>& tgts, ammos &c, size_t gun_s) {
	bool btmp = false;
	bool is_hit;
	std::optional<size_t> hitnear;

	for (auto& t : tgts) {
		if (play.id == t.id)
			continue;
		hitnear.reset();
		//主砲
		if (gun_s == 0) {
			//とりあえず当たったかどうか

			is_hit = false;
			for (size_t colmesh = 0; colmesh < t.hitssort.size(); ++colmesh) {
				const auto HitPoly = MV1CollCheck_Line(t.colobj.get(), -1, c.repos, c.pos, int(colmesh));
				if (HitPoly.HitFlag) {
					t.hitssort[colmesh] = pair(colmesh, VSize(VSub(HitPoly.HitPosition, c.repos)));
					is_hit = true;
				}
				else
					t.hitssort[colmesh] = pair(colmesh, 9999.f);
			}
			if (!is_hit)
				continue;

			std::sort(t.hitssort.begin(), t.hitssort.end(), [](const pair& x, const pair& y) { return x.second < y.second; });
			//近い順に、はじく操作のいらないメッシュに対しダメージ面に届くまで判定
			for (auto& tt : t.hitssort) {
				if (tt.second == 9999.f)
					break; //装甲面に当たらなかったならスルー
				const auto k = tt.first;
				if (k >= 4) {
					if (t.HP[k] > 0) {
						if (k == 4)
							continue; //砲身だけ処理を別にしたいので分けます
						//空間装甲、モジュール
						const auto HitPoly = MV1CollCheck_Line(t.colobj.get(), -1, c.repos, c.pos, int(k));
						if (HitPoly.HitFlag) {
							set_effect(&play.effcs[ef_reco], HitPoly.HitPosition, HitPoly.Normal);
							t.HP[k] = std::max<short>(t.HP[k] - 50, 0);
							c.pene /= 2.0f;
							c.speed /= 2.f;
						}
					}
				}
				else {
					hitnear = k;
					break;
				}
			}
			//ダメージ面に当たった時に装甲値に勝てるかどうか
			if (hitnear) {
 				const auto HitPoly = MV1CollCheck_Line(t.colobj.get(), -1, c.repos, c.pos, int(hitnear.value())); //当たっているものとして詳しい判定をとる
				MV1SetFrameUserLocalMatrix(t.colobj.get(), 9 + 0 + 3 * t.hitbuf, MMult(MGetTranslate(HitPoly.HitPosition), MInverse(t.ps_m)));
				MV1SetFrameUserLocalMatrix(t.colobj.get(), 9 + 1 + 3 * t.hitbuf, MMult(MGetTranslate(VAdd(HitPoly.Normal, HitPoly.HitPosition)), MInverse(t.ps_m)));
				MV1SetFrameUserLocalMatrix(t.colobj.get(), 9 + 2 + 3 * t.hitbuf, MMult(MGetTranslate(VAdd(VCross(HitPoly.Normal, c.vec), HitPoly.HitPosition)), MInverse(t.ps_m)));
				set_effect(&play.effcs[ef_reco], HitPoly.HitPosition, HitPoly.Normal);
				if (c.pene > t.ptr->armer[hitnear.value()] * (1.0f / abs(VDot(VNorm(c.vec), HitPoly.Normal)))) {
					if (t.HP[0] != 0) {
						PlaySoundMem(t.se[29 + GetRand(1)].get(), DX_PLAYTYPE_BACK, TRUE);
						set_effect(&play.effcs[ef_bomb], t.obj.frame(t.ptr->engineframe), VGet(0, 0, 0));
						set_effect(&play.effcs[ef_smoke1], t.obj.frame(t.ptr->engineframe), VGet(0, 0, 0));
						if (play.hitadd == false)
							play.hitadd = true;
					}
					c.flug = false;
					t.HP[0] = 0;
					t.hit[t.hitbuf].use = 0;
				}
				else {
					PlaySoundMem(t.se[10 + GetRand(16)].get(), DX_PLAYTYPE_BACK, TRUE);
					if (t.recoadd == false) {
						t.recorad = atan2(HitPoly.HitPosition.x - t.pos.x, HitPoly.HitPosition.z - t.pos.z);
						t.recoadd = true;
					}
					c.vec = VAdd(c.vec, VScale(HitPoly.Normal, VDot(c.vec, HitPoly.Normal) * -2.0f));
					c.pos = VAdd(HitPoly.HitPosition, VScale(c.vec, 0.01f));

					c.pene /= 2.0f;
					c.speed /= 2.f;

					t.hit[t.hitbuf].use = 1;
				}
				MV1SetScale(t.hit[t.hitbuf].pic.get(), VGet(play.ptr->ammosize[gun_s] * 100.f * (1.0f / abs(VDot(VNorm(c.vec), HitPoly.Normal))), play.ptr->ammosize[gun_s] * 100.f, play.ptr->ammosize[gun_s] * 100.f)); //
				t.hit[t.hitbuf].flug = true;
				t.hitbuf++;
				t.hitbuf %= 3;
			}
		}
		//同軸機銃
		else {
			//近い面を探す判定
			std::array<float, 2> tmpf;
			tmpf[0] = c.speed;
			for (size_t colmesh = 0; colmesh < t.HP.size(); ++colmesh) {
				if (colmesh >= 5 && t.HP[colmesh] == 0)
					continue;
				const auto HitPoly = MV1CollCheck_Line(t.colobj.get(), -1, c.repos, c.pos, int(colmesh));
				if (HitPoly.HitFlag) {
					tmpf[1] = VSize(VSub(HitPoly.HitPosition, c.repos));
					if (tmpf[1] <= tmpf[0]) {
						tmpf[0] = tmpf[1];
						hitnear = colmesh;
					}
				}
			}
			//至近で弾かせる
			if (hitnear) {
				const auto HitPoly = MV1CollCheck_Line(t.colobj.get(), -1, c.repos, c.pos, int(hitnear.value()));
				set_effect(&play.effcs[ef_reco2], HitPoly.HitPosition, HitPoly.Normal);
				PlaySoundMem(t.se[10 + GetRand(16)].get(), DX_PLAYTYPE_BACK, TRUE);
				c.vec = VAdd(c.vec, VScale(HitPoly.Normal, VDot(c.vec, HitPoly.Normal) * -2.0f));
				c.pos = VAdd(HitPoly.HitPosition, VScale(c.vec, 0.01f));
			}
		}

		btmp = hitnear.has_value();

		if (hitnear.has_value())
			break;
	}
	return btmp;
}
void set_gunrad(players& play, float rat_r) {
	for (int i = 0; i < 4; ++i) {
		if ((play.move & (KEY_TURNLFT << i)) != 0) {
			switch (i) {
			case 0:
				play.gunrad.x -= play.ptr->gun_RD / rat_r; //
				if (!play.ptr->gun_lim_LR)
					play.gunrad.x = std::max<float>(play.gunrad.x, play.ptr->gun_lim_[0]);
				break;
			case 1:
				play.gunrad.x += play.ptr->gun_RD / rat_r; //
				if (!play.ptr->gun_lim_LR)
					play.gunrad.x = std::min<float>(play.gunrad.x, play.ptr->gun_lim_[1]);
				break;
			case 2:
				play.gunrad.y = std::min<float>(play.gunrad.y + (play.ptr->gun_RD / 2.f) / rat_r, play.ptr->gun_lim_[2]);
				break;
			case 3:
				play.gunrad.y = std::max<float>(play.gunrad.y - (play.ptr->gun_RD / 2.f) / rat_r, play.ptr->gun_lim_[3]);
				break;
			}
		}
	}
}
bool set_shift(players& play) {
	int gearrec = play.gear;
	/*自動変速機*/
	if (play.gear > 0 && play.gear <= 3)
		if (play.flont >= play.ptr->speed_flont[play.gear - 1] * 0.9)
			++play.gear;
	if (play.gear < 0 && play.gear >= -3)
		if (play.back <= play.ptr->speed_back[-play.gear - 1] * 0.9)
			--play.gear;

	if ((play.move & KEY_GOFLONT) == 0 && play.gear > 0)
		--play.gear;
	else if ((play.move & KEY_GOBACK_) == 0 && play.gear < 0)
		++play.gear;
	/*変速*/
	/*
	if (play.gear > 0 && play.gear <= 3) {
		if (CheckHitKey(KEY_INPUT_R) != 0) { ++play.gearu; if (play.gearu == 1) { ++play.gear; } }
		else { play.gearu = 0; }
	}
	if (play.gear < 0 && play.gear >= -3) {
		if (CheckHitKey(KEY_INPUT_F) != 0) { ++play.geard; if (play.geard == 1) { --play.gear; } }
		else { play.geard = 0; }
	}
	*/
	/**/
	if (play.gear == 0) {
		if ((play.move & KEY_GOFLONT) != 0) {
			++play.gear;
		}
		if ((play.move & KEY_GOBACK_) != 0) {
			--play.gear;
		}
	}
	if (play.gear != gearrec) {
		return true;
	}
	return false;
}
