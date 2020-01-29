#define NOMINMAX
#include "define.h"
//#include "FontHandle.hpp"
//#include <fstream>
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
	FileRead_gets(mstr, 64, mdata);
	USEHOST = bool(std::stoul(getright(mstr)));
	FileRead_gets(mstr, 64, mdata);
	USEPIXEL = bool(std::stoul(getright(mstr)));
	FileRead_gets(mstr, 64, mdata);
	se_vol = float(std::stoul(getright(mstr))) / 100.f;
	FileRead_close(mdata);

	if (ANTI >= 2)
		SetFullSceneAntiAliasingMode(ANTI, 3); /*アンチエイリアス*/
	//SetWindowStyleMode(4);			    /**/
	//SetWindowUserCloseEnableFlag(FALSE);		    /*alt+F4対処*/
	SetMainWindowText("Tank Flanker");		    /*name*/
	SetAeroDisableFlag(TRUE);			    /**/
	SetUsePixelLighting(USEPIXEL);			    /*ピクセルライティング*/
	SetWaitVSyncFlag(YSync);			    /*垂直同期*/
	ChangeWindowMode(windowmode);			    /*窓表示*/
	SetUseDirect3DVersion(DX_DIRECT3D_11);		    /*directX ver*/
	Set3DSoundOneMetre(1.0f);			    /*3Dsound*/
	SetGraphMode(dispx, dispy, 32);			    /*解像度*/
	DxLib_Init();					    /*init*/
	Effekseer_Init(8000);				    /*Effekseer*/
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);  /*Effekseer*/
	Effekseer_SetGraphicsDeviceLostCallbackFunctions(); /*Effekseer*/
	SetAlwaysRunFlag(TRUE);				    /*background*/
	SetUseZBuffer3D(TRUE);				    /*zbufuse*/
	SetWriteZBuffer3D(TRUE);			    /*zbufwrite*/
	MV1SetLoadModelReMakeNormal(TRUE);		    /*法線*/
	MV1SetLoadModelPhysicsWorldGravity(M_GR);	   /*重力*/
	//SetSysCommandOffFlag(TRUE)//強制ポーズ対策()
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
		v.gun_[0].reloadtime = int(std::stoi(getright(mstr)) * f_rate);
		v.gun_[1].reloadtime = 10;
		FileRead_gets(mstr, 64, mdata);
		v.gun_[0].ammosize = std::stof(getright(mstr)) / 1000.f;
		v.gun_[1].ammosize = 0.0075f;
		FileRead_gets(mstr, 64, mdata);
		v.gun_[0].accuracy = int(std::stof(getright(mstr)));
		v.gun_[1].accuracy = int(std::stof(getright(mstr))); //
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
	SetUseASyncLoadFlag(FALSE);
}
void Myclass::autoset_option(void) {
	using namespace std::literals;
	WIN32_FIND_DATA win32fdt;
	HANDLE hFind;
	char mstr[64]; /*tank*/
	int mdata;     /*tank*/

	const auto c_00ff00 = GetColor(0, 255, 0);
	const auto c_ffff00 = GetColor(255, 255, 0);
	const auto c_ff0000 = GetColor(255, 0, 0);
	const auto c_ffffff = GetColor(255, 255, 255);

	char CPUName[256];
	char GPUName[256];
	double TotalMemorySize;
	GetPcInfo(NULL, NULL, CPUName, NULL, NULL, &TotalMemorySize, NULL, GPUName, NULL, NULL);

	int k = 0;
	const auto waits = GetNowHiPerformanceCount();
	SetDrawScreen(DX_SCREEN_BACK);
	DrawFormatString(0, 18 * (k++), c_ffffff, "CPU    : %s", CPUName); //
	DrawFormatString(0, 18 * (k++), c_ffffff, "GPU    : %s", GPUName); //

	DrawFormatString(0, 18 * (k++), c_ffffff, "メモリ : %fGB", TotalMemorySize / 1000); //
	if (TotalMemorySize <= 5000) {							    //5GB以内
	}

	bool find;
	std::string tempname;
	//CPU
	DrawString(0, 18 * (k++), "CPU構成", c_ffff00);
	tempname = CPUName;
	hFind = FindFirstFile("data/setdata/CPU/*", &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (win32fdt.cFileName[0] != '.') {
				DrawFormatString(0, 18 * (k++), c_ffffff, "┠%s", win32fdt.cFileName);
				if (tempname.find(win32fdt.cFileName) != std::string::npos) {
					DrawString(0, 18 * (k++), "　　find!", c_00ff00);

					mdata = FileRead_open(("data/setdata/CPU/"s + win32fdt.cFileName).c_str(), FALSE);
					FileRead_gets(mstr, 64, mdata);
					usegrab = bool(std::stoul(getright(mstr)));
					FileRead_gets(mstr, 64, mdata);
					FileRead_gets(mstr, 64, mdata); //
					FileRead_gets(mstr, 64, mdata); //
					FileRead_gets(mstr, 64, mdata); //
					FileRead_gets(mstr, 64, mdata);
					drawdist = std::stof(getright(mstr));
					FileRead_gets(mstr, 64, mdata);
					FileRead_gets(mstr, 64, mdata);
					FileRead_gets(mstr, 64, mdata);
					FileRead_gets(mstr, 64, mdata); //se
					FileRead_gets(mstr, 64, mdata); //se
					FileRead_close(mdata);

					find = true;
					break;
				}
				else {
					DrawString(0, 18 * (k++), "　　wrong", c_ff0000);
				}
			}
		} while (FindNextFile(hFind, &win32fdt));
	}
	FindClose(hFind);
	//GPU
	DrawString(0, 18 * (k++), "GPU構成", c_ffff00);
	tempname = GPUName;
	hFind = FindFirstFile("data/setdata/GPU/*", &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (win32fdt.cFileName[0] != '.') {
				DrawFormatString(0, 18 * (k++), c_ffffff, "┠%s", win32fdt.cFileName);
				if (tempname.find(win32fdt.cFileName) != std::string::npos) {
					DrawString(0, 18 * (k++), "　　find!", c_00ff00);

					mdata = FileRead_open(("data/setdata/GPU/"s + win32fdt.cFileName).c_str(), FALSE);
					FileRead_gets(mstr, 64, mdata);
					FileRead_gets(mstr, 64, mdata);
					ANTI = unsigned char(std::stoul(getright(mstr)));
					FileRead_gets(mstr, 64, mdata); //
					FileRead_gets(mstr, 64, mdata); //
					FileRead_gets(mstr, 64, mdata); //
					FileRead_gets(mstr, 64, mdata);
					FileRead_gets(mstr, 64, mdata);
					gndx = std::stoi(getright(mstr));
					FileRead_gets(mstr, 64, mdata);
					shadex = std::stoi(getright(mstr));
					FileRead_gets(mstr, 64, mdata);
					USEHOST = bool(std::stoul(getright(mstr)));
					FileRead_gets(mstr, 64, mdata); //se
					FileRead_gets(mstr, 64, mdata); //se
					FileRead_close(mdata);

					find = true;
					break;
				}
				else {
					DrawString(0, 18 * (k++), "　　wrong", c_ff0000);
				}
			}
		} while (FindNextFile(hFind, &win32fdt));
	}
	FindClose(hFind);

	ScreenFlip();

	DxLib::WaitKey();

	write_option();
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
	outputfile << "hostpass(1or0)=" + std::to_string(USEHOST) + "\n";
	outputfile << "pixellighting(1or0)=" + std::to_string(USEPIXEL) + "\n";
	outputfile << "se_vol(100~0)=" + std::to_string(se_vol * 100.f) + "\n"; //
	outputfile.close();
}
bool Myclass::set_veh(void) {
	for (auto& v : vecs) {
		//
		for (int i = 0; i < v.colmodel.frame_num(); ++i) {
			std::string tempname = MV1GetFrameName(v.colmodel.get(), i);
			if (tempname == "min")
				v.min = v.colmodel.frame(i);
			if (tempname == "max")
				v.max = v.colmodel.frame(i);
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
				if (l < v.smokeframe.size()) {
					if (tempname.find("smoke") != std::string::npos)
						v.smokeframe[l++] = i;
				}
				//
				if (tempname == "turret")
					v.turretframe = i;
				if (j < v.gun_.size()) {
					if (tempname.find("gun") != std::string::npos && tempname.back() != '_') //gun
						v.gun_[j++].gunframe = i;
				}
				//ホイール
				if (k < v.kidoframe.size()) {
					if (tempname[0] == 'K') //起動輪
						v.kidoframe[k++] = i;
				}
				if (tempname[0] == 'Y') //誘導輪
					v.youdoframe.emplace_back(i);
				if (tempname[0] == 'F') //ホイール
					v.wheelframe.emplace_back(i);
				if (tempname[0] == 'U') //ホイール
					v.upsizeframe.emplace_back(i);
			}
		}
	}
	//エフェクト	//読み込みミス現状なさそう？
	{
		size_t j = 0;
		for (auto& e : effHndle)
			e = EffekseerEffectHandle::load("data/effect/" + std::to_string(j++) + ".efk");
		gndsmkHndle = EffekseerEffectHandle::load("data/effect/gndsmk.efk");
	}
	return true;
}
int Myclass::window_choosev(void) {
	SetMousePoint(x_r(960), y_r(969));
	SetMouseDispFlag(TRUE);
	const auto font18 = FontHandle::Create(x_r(18), y_r(18 / 3), DX_FONTTYPE_ANTIALIASING);
	const auto font72 = FontHandle::Create(x_r(72), y_r(72 / 3), DX_FONTTYPE_ANTIALIASING);
	int i = 0, l = 0, x = 0, y = 0;
	int xp = 0, yp = 0;
	unsigned int m;
	float pert;
	int mousex, mousey; /*mouse*/
	float real = 0.f, r = 5.f;
	LONGLONG waits;
	const auto c_00ff00 = GetColor(0, 255, 0);
	const auto c_ffff00 = GetColor(255, 255, 0);
	const auto c_ff0000 = GetColor(255, 0, 0);
	const auto c_ffffff = GetColor(255, 255, 255);
	const auto c_808080 = GetColor(128, 128, 128);
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
		font18.DrawStringFormat(x_r(xp), y_r(850), c_00ff00, "SPEED : %5.2f～%5.2f km/h", vecs[i].speed_flont[3] * 3.6f, vecs[i].speed_back[3] * 3.6f);

		xp = 1140;
		yp = 810;
		DrawBox(x_r(xp - 1), y_r(yp + 18), x_r(xp + 1 + 200), y_r(yp + 19), c_808080, FALSE);
		DrawBox(x_r(xp + 1), y_r(yp + 17), x_r(xp - 1 + 200 * rad2deg(vecs[i].vehicle_RD) / 100.f * pert), y_r(yp + 20), c_00ff00, TRUE);
		font18.DrawStringFormat(x_r(xp), y_r(yp), c_00ff00, "TURN SPEED : %5.2f deg/s", rad2deg(vecs[i].vehicle_RD));


		xp = 1120;
		yp = 580;
		DrawBox(x_r(xp - 1), y_r(yp + 18), x_r(xp + 1 + 200), y_r(yp + 19), c_808080, FALSE);
		DrawBox(x_r(xp + 1), y_r(yp + 17), x_r(xp - 1 + 200 * vecs[i].armer[0] / 150.f * pert), y_r(yp + 20), c_00ff00, TRUE);
		font18.DrawStringFormat(x_r(xp), y_r(yp), c_00ff00, "MAX ARMER : %5.2f mm", vecs[i].armer[0]);

		xp = 450;
		yp = 510;
		DrawBox(x_r(xp - 1), y_r(yp + 18), x_r(xp + 1 + 200), y_r(yp + 19), c_808080, FALSE);
		DrawBox(x_r(xp + 1 + 100), y_r(yp + 17), x_r(xp - 1 + 100 + 100 * rad2deg(vecs[i].gun_lim_[2]) / 40.f * pert), y_r(yp + 20), c_00ff00, TRUE);
		DrawBox(x_r(xp + 1 + 100), y_r(yp + 17), x_r(xp - 1 + 100 - 100 * rad2deg(vecs[i].gun_lim_[3]) / -20.f * pert), y_r(yp + 20), c_ff0000, TRUE);
		font18.DrawStringFormat(x_r(xp), y_r(yp), c_00ff00, "GUN RAD     : %5.2f°～%5.2f°", rad2deg(vecs[i].gun_lim_[2]), rad2deg(vecs[i].gun_lim_[3]));

		yp = 530;
		DrawBox(x_r(xp + 10), y_r(yp + 10), x_r(xp + 90), y_r(yp + 90), c_808080, FALSE);
		DrawBox(
		    x_r(xp + 50 - int(float(vecs[i].gun_[0].accuracy) / 100.f * pert)),
		    y_r(yp + 50 - int(float(vecs[i].gun_[0].accuracy) / 100.f * pert)),
		    x_r(xp + 50 + int(float(vecs[i].gun_[0].accuracy) / 100.f * pert)),
		    y_r(yp + 50 + int(float(vecs[i].gun_[0].accuracy) / 100.f * pert)),
		    c_ffff00, FALSE);
		DrawLine(x_r(xp + 50), y_r(yp + 50), x_r(xp + 140), y_r(yp + 28), c_00ff00);

		DrawCircle(x_r(xp + 120), y_r(yp + 70), y_r(200 / 2 / 5), c_808080, FALSE);
		DrawCircle(x_r(xp + 120), y_r(yp + 70), y_r(int(vecs[i].gun_[0].ammosize * 1000.f * pert) / 2 / 5), c_00ff00, FALSE);
		DrawLine(x_r(xp + 120), y_r(yp + 70), x_r(xp + 140), y_r(yp + 46), c_00ff00);

		font18.DrawStringFormat(x_r(xp + 140), y_r(yp + 10), c_00ff00, "ACCURACY    : ±%05.1f°", float(vecs[i].gun_[0].accuracy) / 10000.f);
		font18.DrawStringFormat(x_r(xp + 140), y_r(yp + 28), c_00ff00, "GUN CALIBER : %03.1fmm", vecs[i].gun_[0].ammosize * 1000.f);

		//
		font18.DrawString(x_r(0), y_r(18 * 1), "SETTING", c_00ff00);
		font18.DrawStringFormat(x_r(0), y_r(18 * 2), c_00ff00, " 人の物理演算         : %s", usegrab ? "TRUE" : "FALSE");
		font18.DrawStringFormat(x_r(0), y_r(18 * 3), c_00ff00, " アンチエイリアス倍率 : x%d", ANTI);
		font18.DrawStringFormat(x_r(0), y_r(18 * 4), c_00ff00, " 垂直同期             : %s", YSync ? "TRUE" : "FALSE");
		font18.DrawStringFormat(x_r(0), y_r(18 * 5), c_00ff00, " ウィンドウor全画面   : %s", windowmode ? "TRUE" : "FALSE");
		font18.DrawStringFormat(x_r(0), y_r(18 * 6), c_00ff00, " 木の描画距離         : %5.2f m", drawdist);
		font18.DrawStringFormat(x_r(0), y_r(18 * 7), c_00ff00, " 地面のクォリティ     : x%d", gndx);
		font18.DrawStringFormat(x_r(0), y_r(18 * 8), c_00ff00, " 影のクォリティ       : x%d", shadex);
		font18.DrawStringFormat(x_r(0), y_r(18 * 9), c_00ff00, " ホストパスエフェクト : %s", USEHOST ? "TRUE" : "FALSE");
		font18.DrawStringFormat(x_r(0), y_r(18 * 10), c_00ff00, " ピクセルライティング : %s", USEPIXEL ? "TRUE" : "FALSE");
		//
		GetMousePoint(&mousex, &mousey);
		if (inm(x_r(360), y_r(340), x_r(400), y_r(740))) {
			m = c_ffff00;
			if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) {
				m = c_ff0000;
				++x;
				if (x == 1) {
					l++;
					++i %= vecs.size();
				}
			}
			else
				x = 0;
		}
		else
			m = c_00ff00;
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
					if (--i < 0)
						i = int(vecs.size() - 1);
				}
			}
			else
				y = 0;
		}
		else
			m = c_00ff00;
		DrawBox(x_r(1520), y_r(340), x_r(1560), y_r(740), m, FALSE);
		font18.DrawString(x_r(1524), y_r(531), ">", c_ffffff);
		//
		if (inm(x_r(760), y_r(960), x_r(1160), y_r(996))) {
			m = c_ffc800;
			if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0)
				break;
		}
		else
			m = c_ff6400;
		DrawBox(x_r(760), y_r(960), x_r(1160), y_r(996), m, FALSE);
		font18.DrawString(x_r(960) - font18.GetDrawWidth("次へ") / 2, y_r(969), "次へ", c_ffffff);
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
void Myclass::set_viewrad(VECTOR_ref vv) {
	view = vv;
	view_r = vv;
}
void Myclass::set_view_r(int wheel, bool life) {
	int mousex, mousey; /*mouse*/
	GetMousePoint(&mousex, &mousey);
	view = DxLib::VGet(
	    std::clamp(view.x() + (float)(mousey - dispy / 2) / dispy * 1.0f, deg2rad(-35), deg2rad(35)),
	    view.y() + (float)(mousex - dispx / 2) / dispx * 1.0f,
	    std::clamp(view.z() + (float)wheel / 10.0f, life ? 0.1f : 0.11f, 2.f));
	view_r += (view - view_r).Scale(0.1f);
	if (view.z() == 0.1f)
		view_r = view;
	SetCursorPos(dispx / 2, dispy / 2);
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
	using namespace std::literals;
	usegrab = useg;
	f_rate = frates;

	WIN32_FIND_DATA win32fdt;
	HANDLE hFind;
	if (usegrab)
		MV1SetLoadModelUsePhysicsMode(DX_LOADMODEL_PHYSICS_REALTIME);
	else
		MV1SetLoadModelUsePhysicsMode(DX_LOADMODEL_PHYSICS_LOADCALC);

	hFind = FindFirstFile("data/chara/*", &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((win32fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (win32fdt.cFileName[0] != '.')) {
				name.emplace_back(win32fdt.cFileName);
				mod.resize(mod.size() + 1);
				mod.back().model = MV1ModelHandle::Load("data/chara/"s + name.back() + "/model.mv1");
				{
					//タイトル用アニメーション追加
					int j = 0;
					for (auto& s : mod.back().amine_title) {
						s.id = MV1AttachAnim(mod.back().model.get(), j++, -1, TRUE);
						s.total = MV1GetAttachAnimTotalTime(mod.back().model.get(), s.id);
					}
					//車長ボイス
					SetUseASyncLoadFlag(TRUE);
					int k = 0;
					for (auto& s : mod.back().sound)
						s = SoundHandle::Load("data/chara/"s + name.back() + "/voice/" + std::to_string(k++) + ".wav");
					SetUseASyncLoadFlag(FALSE);
				}
			}
		} while (FindNextFile(hFind, &win32fdt));
	} //else{ return false; }
	FindClose(hFind);
}
bool HUMANS::set_humans(const MV1ModelHandle& inmod) {
	using namespace std::literals;
	inmodel_handle = inmod.Duplicate();
	inflames = inmodel_handle.frame_num();
	pos_old.resize(inflames);
	hum.resize((inflames - bone_in_turret >= 1) ? inflames - bone_in_turret : 1);
	//読み込み
	//todo : ここでキャラ選択
	int sel = 0;
	{
		SetMousePoint(x_r(960), y_r(969));
		SetMouseDispFlag(TRUE);
		const auto font18 = FontHandle::Create(x_r(18), y_r(18 / 3), DX_FONTTYPE_ANTIALIASING);
		const auto font72 = FontHandle::Create(x_r(72), y_r(72 / 3), DX_FONTTYPE_ANTIALIASING);
		uint8_t x = 0, y = 0;
		//int xp = 0, yp = 0;
		int mousex, mousey; /*mouse*/
		LONGLONG waits;
		unsigned int m;
		const auto c_00ff00 = GetColor(0, 255, 0);
		const auto c_ffff00 = GetColor(255, 255, 0);
		const auto c_ff0000 = GetColor(255, 0, 0);
		const auto c_ffffff = GetColor(255, 255, 255);
		const auto c_808080 = GetColor(128, 128, 128);
		const auto c_ffc800 = GetColor(255, 200, 0);
		const auto c_ff6400 = GetColor(255, 100, 0);

		int j = 0;
		float time = 0.f;

		while (ProcessMessage() == 0) {
			waits = GetNowHiPerformanceCount();
			if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
				sel = -1;
				break;
			} //end
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();
			setcv(1.0f, 100.0f, VGet(0, 2.f, -2.f), VGet(0, 1.0f, 0), VGet(0, 1.0f, 0), 45.0f);
			SetLightDirection(VSub(VGet(0, 2.f, -2.f), VGet(0, 1.0f, 0)));

			MV1SetPosition(mod[sel].model.get(), VGet(0, 0, 0));
			MV1SetRotationXYZ(mod[sel].model.get(), VGet(0, 0, 0));
			MV1DrawModel(mod[sel].model.get());

			for (size_t k = 0; k < ANIME_title; k++) {
				if (k != j)
					MV1SetAttachAnimBlendRate(mod[sel].model.get(), mod[sel].amine_title[k].id, 0.0f);
				else
					MV1SetAttachAnimBlendRate(mod[sel].model.get(), mod[sel].amine_title[k].id, 1.0f);
			}
			MV1SetAttachAnimTime(mod[sel].model.get(), mod[sel].amine_title[j].id, time);

			time += 30.f / f_rate;
			if (time >= mod[sel].amine_title[j].total) {
				time = 0.f;
				++j %= ANIME_title;
			}
			MV1PhysicsCalculation(mod[sel].model.get(), 1000.0f / f_rate);

			font72.DrawString(x_r(960) - font72.GetDrawWidth(name[sel]) / 2, y_r(154), name[sel], c_00ff00);
			/*
			xp = 850;
			yp = 850;
			DrawBox(x_r(xp - 1), y_r(yp + 18), x_r(xp + 1 + 200), y_r(yp + 19), c_808080, FALSE);
			DrawBox(x_r(xp + 1 + 100), y_r(yp + 17), x_r(xp - 1 + 100 + 100 * (vecs[sel].speed_flont[3] * 3.6f) / 100.f), y_r(yp + 20), c_00ff00, TRUE);
			DrawBox(x_r(xp + 1 + 100), y_r(yp + 17), x_r(xp - 1 + 100 - 100 * (vecs[sel].speed_back[3] * -3.6f) / 50.f), y_r(yp + 20), c_ff0000, TRUE);
			font18.DrawStringFormat(x_r(xp), y_r(850), c_00ff00, "SPEED : %5.2f～%5.2f km/h", vecs[sel].speed_flont[3] * 3.6f, vecs[sel].speed_back[3] * 3.6f);
			*/

			GetMousePoint(&mousex, &mousey);
			if (inm(x_r(360), y_r(340), x_r(400), y_r(740))) {
				m = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? c_ff0000 : c_ffff00;
				x = std::min<uint8_t>(x + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
				if (x == 1) {
					++sel %= mod.size();
					MV1PhysicsResetState(mod[sel].model.get());
				}
			}
			else {
				m = c_00ff00;
			}
			DrawBox(x_r(360), y_r(340), x_r(400), y_r(740), m, FALSE);
			font18.DrawString(x_r(396) - font18.GetDrawWidth("<"), y_r(531), "<", c_ffffff);
			//
			if (inm(x_r(1520), y_r(340), x_r(1560), y_r(740))) {
				m = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? c_ff0000 : c_ffff00;
				y = std::min<uint8_t>(y + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
				if (y == 1) {
					if (--sel < 0)
						sel = int(mod.size() - 1);
					MV1PhysicsResetState(mod[sel].model.get());
				}
			}
			else
				m = c_00ff00;
			DrawBox(x_r(1520), y_r(340), x_r(1560), y_r(740), m, FALSE);
			font18.DrawString(x_r(1524), y_r(531), ">", c_ffffff);
			//
			if (inm(x_r(760), y_r(960), x_r(1160), y_r(996))) {
				m = c_ffc800;
				if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0)
					break;
			}
			else
				m = c_ff6400;
			DrawBox(x_r(760), y_r(960), x_r(1160), y_r(996), m, FALSE);
			font18.DrawString(x_r(960) - font18.GetDrawWidth("戦闘開始") / 2, y_r(969), "戦闘開始", c_ffffff);
			ScreenFlip();
			do {
			} while (GetNowHiPerformanceCount() - waits < 1000000.0f / f_rate);
			//Myclass::Screen_Flip(waits);
		}
		if (sel != -1) {
			const auto c_000000 = GetColor(0, 0, 0);
			float unt = 0;
			while (ProcessMessage() == 0 && unt <= 0.9f) {
				waits = GetNowHiPerformanceCount();
				SetDrawScreen(DX_SCREEN_BACK);
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * unt));
				DrawBox(0, 0, dispx, dispy, c_000000, TRUE);
				differential(unt, 1.f, 0.05f);
				ScreenFlip();
				do {
				} while (GetNowHiPerformanceCount() - waits < 1000000.0f / f_rate);
				//Myclass::Screen_Flip(waits);
			}
		}

		if (sel == -1)
			return false;
	}

	for (auto&& h : hum) {
		h.obj = mod[sel].model.Duplicate();
		//色調
		for (int j = 0; j < h.obj.material_num(); ++j) {
			MV1SetMaterialDifColor(h.obj.get(), j, GetColorF(0.5f, 0.5f, 0.5f, 1.0f));
			MV1SetMaterialSpcColor(h.obj.get(), j, GetColorF(0.35f, 0.32f, 0.28f, 0.5f));
			MV1SetMaterialSpcPower(h.obj.get(), j, 1.0f);
		}
		//アニメーション
		for (size_t j = 0; j < ANIME_out; ++j) {
			h.amine[j].id = MV1AttachAnim(h.obj.get(), int(ANIME_title + j), -1, TRUE);
			h.amine[j].total = MV1GetAttachAnimTotalTime(h.obj.get(), h.amine[j].id);
			h.amine[j].time = 0.0f;
		}
		h.amine[ANIME_nom].per = 1.f;
		h.amine[ANIME_sit].per = 1.f;
		h.amine[ANIME_eye].per = 1.f;
		h.amine[ANIME_voi].per = 0.f;
	}
	//車長には口パクアニメーション
	hum[0].amine[ANIME_sit].per = 0.f;
	for (size_t j = 0; j < ANIME_voice; ++j) {
		hum[0].voice_anime[j].id = MV1AttachAnim(hum[0].obj.get(), int(ANIME_title + ANIME_out + j), -1, TRUE);
		hum[0].voice_anime[j].total = MV1GetAttachAnimTotalTime(hum[0].obj.get(), hum[0].voice_anime[j].id);
		hum[0].voice_sound[j] = mod[sel].sound[j].Duplicate();
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
	hum[0].nvec = hum[0].obj.frame(hum[0].neck) - hum[0].obj.frame(hum[0].neck - 1);
	hum[0].vflug = -1;
	return true;
}
void HUMANS::set_state(players* play) {
	pplayer = play;
	MV1SetMatrix(inmodel_handle.get(), pplayer->ps_m);
	int fnum = bone_in_turret;
	for (auto& h : hum) {
		MV1SetMatrix(h.obj.get(), MMult(MMult(MGetRotY(pplayer->gunrad.x() - pplayer->yrad), MGetRotVec2(VGet(0, 1.f, 0), pplayer->nor.get())), inmodel_handle.frame(fnum++).Mtrans()));
		for (size_t j = 0; j < ANIME_out; ++j) {
			MV1SetAttachAnimBlendRate(h.obj.get(), h.amine[j].id, h.amine[j].per);
			MV1SetAttachAnimTime(h.obj.get(), h.amine[j].id, h.amine[j].time);
		}
		if (usegrab)
			MV1PhysicsResetState(h.obj.get());
	}
}
void HUMANS::set_humanvc_vol(unsigned char size) {
	for (auto&& v : hum[0].voice_sound)
		ChangeVolumeSoundMem(size, v.get());
}
void HUMANS::set_humanmove(VECTOR_ref rad, const float frate, const float fps) {
	for (int i = 0; i < inflames; ++i)
		pos_old[i] = inmodel_handle.frame(i);

	MV1SetMatrix(inmodel_handle.get(), pplayer->ps_m);
	MV1SetFrameUserLocalMatrix(inmodel_handle.get(), pplayer->ptr->turretframe, pplayer->ps_t);
	MV1SetFrameUserLocalMatrix(inmodel_handle.get(), pplayer->ptr->gun_[0].gunframe, MMult(MMult(MGetRotX(pplayer->gunrad.y()), (locin[pplayer->ptr->gun_[0].gunframe] - locin[pplayer->ptr->turretframe]).Mtrans()), pplayer->ps_t));
	MV1SetFrameUserLocalMatrix(inmodel_handle.get(), pplayer->ptr->gun_[0].gunframe + 1, (pplayer->ptr->loc[pplayer->ptr->gun_[0].gunframe + 1] - pplayer->ptr->loc[pplayer->ptr->gun_[0].gunframe] + VGet(0, 0, pplayer->Gun[0].fired)).Mtrans());
	//7,6だけ車体乗人
	for (int i = bone_hatch; i < inflames; ++i) {
		if (i == 7 || i == 8)
			continue;
		MV1SetFrameUserLocalMatrix(inmodel_handle.get(), i, MMult(MMult(MGetRotY(pplayer->gunrad.x()), (locin[i] - locin[pplayer->ptr->turretframe]).Mtrans()), pplayer->ps_t));
	}

	bool physicsReset = false;

	if (rad.z() > 0.1f)
		in_f = false;
	if (rad.z() == 0.1f && !in_f) {
		in_f = true;
		physicsReset = true;
	}
	{
		auto& h = hum.front();
		/*座る*/
		if (pplayer->spd >= 30.f / 3.6f)
			fpsdiff(h.amine[ANIME_sit].per, 1.f, 0.075f);
		else
			h.amine[ANIME_sit].per *= pow(0.925f, frate / fps);
		//voice
		h.amine[ANIME_eye].per = 1.f;
		if (h.vflug == -1)
			h.amine[ANIME_voi].per *= pow(0.9f, frate / fps);
		else
			fpsdiff(h.amine[ANIME_voi].per, 1.f, 0.1f);
		h.amine[ANIME_nom].per = 1.0f - h.amine[ANIME_voi].per;
	}
	//反映
	for (int k = 0; k < divi; ++k) {
		int fnum = bone_in_turret;
		for (auto& h : hum) {
			MV1SetMatrix(
			    h.obj.get(),
			    MMult(
				MMult(
				    MGetRotY(pplayer->gunrad.x() - pplayer->yrad), MGetRotVec2(VGet(0, 1.f, 0), pplayer->nor.get())),
				(pos_old[fnum] + (inmodel_handle.frame(fnum) - pos_old[fnum]).Scale((float)(1 + k) / divi)).Mtrans()));
			if (fnum == bone_in_turret) {
				/*首振り*/
				MV1SetFrameUserLocalMatrix(
				    h.obj.get(), h.neck,
				    MMult(
					h.nvec.Mtrans(),
					MMult(
					    MGetRotX(std::clamp(-rad.x(), deg2rad(-20), deg2rad(20))),
					    MGetRotY(std::clamp(atanf(sin(rad.y() - (pplayer->gunrad.x() - pplayer->yrad))), deg2rad(-40), deg2rad(40))))));
				//voice
				if (h.vflug != -1) {
					if (h.voice_time < h.voice_anime[h.vflug].total) {
						if (h.voice_time == 0.0f)
							PlaySoundMem(h.voice_sound[h.vflug].get(), DX_PLAYTYPE_BACK, TRUE);
						MV1SetAttachAnimTime(h.obj.get(), h.voice_anime[h.vflug].id, h.voice_time);
						h.voice_time += 30.0f / divi / f_rate; //
					}
					else {
						h.vflug = -1;
						h.voice_time = 0.0f;
					}
				}
			}
			for (size_t j = 0; j < ANIME_out; ++j) {
				MV1SetAttachAnimBlendRate(h.obj.get(), h.amine[j].id, h.amine[j].per);
				MV1SetAttachAnimTime(h.obj.get(), h.amine[j].id, h.amine[j].time);
				h.amine[j].time += 30.0f / divi / f_rate; //
				if (j != ANIME_eye && h.amine[j].time >= h.amine[j].total)
					h.amine[j].time = 0.0f;
			}
			//
			if (usegrab) {
				if (physicsReset)
					MV1PhysicsResetState(h.obj.get());
				else
					MV1PhysicsCalculation(h.obj.get(), 1000.0f / divi / f_rate);
			}
			//
			if (fnum == bone_in_turret && !in_f)
				break;
			fnum++;
		}
	}
}
void HUMANS::draw_human(size_t p1) {
	if (hum[p1].amine[ANIME_sit].per <= 0.5f)
		MV1DrawModel(hum[p1].obj.get());
}
void HUMANS::draw_humanall() {
	MV1DrawModel(inmodel_handle.get());
	for (const auto& h : hum)
		MV1DrawModel(h.obj.get());
}
void HUMANS::delete_human(void) {
	inmodel_handle.Dispose();
	for (auto&& h : hum) {
		h.obj.Dispose();
		for (auto& s : h.voice_sound)
			s.Dispose();
	}
	hum.clear();
	locin.clear();
	pos_old.clear();
}
void HUMANS::start_humanvoice(std::int8_t p1) {
	hum[0].vflug = p1;
}
void HUMANS::start_humananime(int p1) {
	for (auto&& h : hum)
		h.amine[p1].time = 0.0f;
}
//
MAPS::MAPS(int map_size, float draw_dist, int shadow_size) {
	groundx = map_size * 1024; /*ノーマルマップのサイズ*/
	drawdist = draw_dist;      /*木の遠近*/
	shadowx = shadow_size;
	int shadowsize = (1 << (10 + shadowx));
	//shadow
	for (auto& s : shadowmap)
		s = MakeShadowMap(shadowsize, shadowsize); /*近影*/
	SetShadowMapAdjustDepth(shadowmap[0], 0.0005f);    /*ずれを小さくするため*/
	//map
	SetUseASyncLoadFlag(TRUE);
	sky_sun = GraphHandle::Load("data/sun.png");	  /*太陽*/
	nor_trk = GraphHandle::Load("data/nm.png");	   /*轍*/
	dif_tex = GraphHandle::Make(groundx, groundx, FALSE); /*ノーマルマップ*/
	nor_tex = GraphHandle::Make(groundx, groundx, FALSE); /*実マップ*/
	SetUseASyncLoadFlag(FALSE);
}
void MAPS::set_map_readyb(size_t set) {
	using namespace std::literals;
	lightvec = VGet(0.5f, -0.2f, -0.5f);
	std::array<const char*, 2> mapper{ "map", "map" }; // TODO: 書き換える
	SetUseASyncLoadFlag(TRUE);
	tree.mnear = MV1ModelHandle::Load("data/"s + mapper.at(set) + "/tree/model.mv1");	/*近木*/
	tree.mfar = MV1ModelHandle::Load("data/"s + mapper.at(set) + "/tree/model2.mv1");	/*遠木*/
	dif_gra = GraphHandle::Load("data/"s + mapper.at(set) + "/SandDesert_04_00344_FWD.png"); /*nor*/
	nor_gra = GraphHandle::Load("data/"s + mapper.at(set) + "/SandDesert_04_00344_NM.png");  /*nor*/
	m_model = MV1ModelHandle::Load("data/"s + mapper.at(set) + "/map.mv1");			 /*map*/
	sky_model = MV1ModelHandle::Load("data/"s + mapper.at(set) + "/sky/model_sky.mv1");      /*sky*/
	graph = GraphHandle::Load("data/"s + mapper.at(set) + "/grass/grass.png");		 /*grass*/
	grass = MV1ModelHandle::Load("data/"s + mapper.at(set) + "/grass/grass.mv1");		 /*grass*/
	GgHandle = GraphHandle::Load("data/"s + mapper.at(set) + "/grass/gg.png");		 /*地面草*/
	SetUseASyncLoadFlag(FALSE);
	return;
}
bool MAPS::set_map_ready() {
	tree.tree_.resize(treec);
	tree.treesort.resize(treec);
	MV1SetScale(sky_model.get(), VGet(0.2f, 0.2f, 0.2f));
	SetUseASyncLoadFlag(TRUE);
	{
		auto i = 0;
		for (auto& t : tree.tree_) {
			t.id = i++;
			t.nears = tree.mnear.Duplicate();
			t.fars = tree.mfar.Duplicate();
			t.hit = true; //
		}
	}
	SetUseASyncLoadFlag(FALSE);

	map_min = MV1GetMeshMinPosition(m_model.get(), 0);
	map_max = MV1GetMeshMaxPosition(m_model.get(), 0);


	MV1SetupCollInfo(m_model.get(), 0, int((map_max - map_min).x()) / 5, int((map_max - map_min).y()) / 5, int((map_max - map_min).z()) / 5);
	SetFogStartEnd(10.0f, 1400.0f); /*fog*/
	SetFogColor(150, 150, 175);     /*fog*/
	SetLightDirection(lightvec.get());
	for (auto& s : shadowmap)
		SetShadowMapLightDirection(s, lightvec.get());
	constexpr uint8_t rate = 96;

	SetDrawScreen(dif_tex.get());
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	for (uint8_t x = 0; x < rate; x++)
		for (uint8_t y = 0; y < rate; y++)
			DrawExtendGraph(groundx * x / rate, groundx * y / rate, groundx * (x + 1) / rate, groundx * (y + 1) / rate, dif_gra.get(), FALSE);
	MV1SetTextureGraphHandle(m_model.get(), 0, dif_tex.get(), FALSE);
	SetDrawScreen(nor_tex.get());
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(0, 0, groundx, groundx, GetColor(121, 121, 255), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 64);
	for (uint8_t x = 0; x < rate; x++)
		for (uint8_t y = 0; y < rate; y++)
			DrawExtendGraph(groundx * x / rate, groundx * y / rate, groundx * (x + 1) / rate, groundx * (y + 1) / rate, nor_gra.get(), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	MV1SetTextureGraphHandle(m_model.get(), 1, nor_tex.get(), FALSE);
	/*grass*/
	vnum = 0;
	pnum = 0;
	MV1SetupReferenceMesh(grass.get(), -1, TRUE); /*参照用メッシュの作成*/

	RefMesh = MV1GetReferenceMesh(grass.get(), -1, TRUE); /*参照用メッシュの取得*/

	IndexNum = RefMesh.PolygonNum * 3 * grasss; /*インデックスの数を取得*/
	VerNum = RefMesh.VertexNum * grasss;	/*頂点の数を取得*/

	grassver.resize(VerNum);   /*頂点データとインデックスデータを格納するメモリ領域の確保*/
	grassind.resize(IndexNum); /*頂点データとインデックスデータを格納するメモリ領域の確保*/

	for (int i = 0; i < grasss; ++i) {
		VECTOR_ref tmpvect = VGet((float)(-(map_max - map_min).x() * 5 + GetRand(int((map_max - map_min).x()) * 10)) / 10.0f, 0.0f, (float)(-(map_max - map_min).z() * 5 + GetRand(int((map_max - map_min).z()) * 10)) / 10.0f);
		//
		SetDrawScreen(dif_tex.get());
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 64);
		DrawRotaGraph((int)(groundx * (0.5f + tmpvect.x() / (float)(map_max - map_min).x())), (int)(groundx * (0.5f - tmpvect.z() / (map_max - map_min).z())), 8.f * groundx / 1024 / 128.0f, 0, GgHandle.get(), TRUE);
		const auto HitPoly = get_gnd_hit(tmpvect + VGet(0.0f, (float)(map_max - map_min).x(), 0.0f), tmpvect + VGet(0.0f, -(float)(map_max - map_min).x(), 0.0f));
		if (HitPoly.HitFlag)
			MV1SetMatrix(grass.get(), MMult(MGetScale(VGet((float)(200 + GetRand(400)) / 100.0f, (float)(25 + GetRand(100)) / 100.0f, (float)(200 + GetRand(400)) / 100.0f)), MMult(MMult(MGetRotY(deg2rad(GetRand(360))), MGetRotVec2(VGet(0, 1.f, 0), HitPoly.Normal)), MGetTranslate(HitPoly.HitPosition))));
		//上省
		MV1RefreshReferenceMesh(grass.get(), -1, TRUE);       /*参照用メッシュの更新*/
		RefMesh = MV1GetReferenceMesh(grass.get(), -1, TRUE); /*参照用メッシュの取得*/
		for (int j = 0; j < RefMesh.VertexNum; ++j) {
			auto& g = grassver[j + vnum];
			g.pos = RefMesh.Vertexs[j].Position;
			g.norm = RefMesh.Vertexs[j].Normal;
			g.dif = RefMesh.Vertexs[j].DiffuseColor;
			g.spc = RefMesh.Vertexs[j].SpecularColor;
			g.u = RefMesh.Vertexs[j].TexCoord[0].u;
			g.v = RefMesh.Vertexs[j].TexCoord[0].v;
			g.su = RefMesh.Vertexs[j].TexCoord[1].u;
			g.sv = RefMesh.Vertexs[j].TexCoord[1].v;
		}
		for (size_t j = 0; j < size_t(RefMesh.PolygonNum); ++j) {
			for (size_t k = 0; k < std::size(RefMesh.Polygons[j].VIndex); ++k)
				grassind[j * 3 + k + pnum] = WORD(RefMesh.Polygons[j].VIndex[k] + vnum);
		}
		vnum += RefMesh.VertexNum;
		pnum += RefMesh.PolygonNum * 3;
	}
	VerBuf = CreateVertexBuffer(VerNum, DX_VERTEX_TYPE_NORMAL_3D);
	IndexBuf = CreateIndexBuffer(IndexNum, DX_INDEX_TYPE_32BIT);
	SetVertexBufferData(0, grassver.data(), VerNum, VerBuf);
	SetIndexBufferData(0, grassind.data(), IndexNum, IndexBuf);
	MV1SetTextureGraphHandle(m_model.get(), 0, dif_tex.get(), FALSE);
	/*tree,shadow*/
	SetShadowMapDrawArea(shadowmap[2], map_min.get(), (map_max + VGet(0, 10.f, 0)).get());
	ShadowMap_DrawSetup(shadowmap[2]);
	for (auto& t : tree.tree_) {
		VECTOR_ref tmpvect = VGet((float)(-(map_max - map_min).x() * 5 + GetRand(int((map_max - map_min).x()) * 10)) / 10.0f, 0.0f, (float)(-(map_max - map_min).z() * 5 + GetRand(int((map_max - map_min).z()) * 10)) / 10.0f);
		const auto HitPoly = get_gnd_hit(tmpvect + VGet(0.0f, map_max.y() + 10.f, 0.0f), tmpvect + VGet(0.0f, map_min.y() - 10.f, 0.0f));
		t.pos = (HitPoly.HitFlag) ? HitPoly.HitPosition : tmpvect;
		t.rad = VGet(0.0f, deg2rad(GetRand(360)), 0.0f);
		MV1SetPosition(t.nears.get(), t.pos.get());
		MV1SetPosition(t.fars.get(), t.pos.get());
		MV1SetRotationXYZ(t.nears.get(), t.rad.get());
		MV1DrawModel(t.nears.get());
		MV1SetMaterialDrawAlphaTestAll(t.nears.get(), TRUE, DX_CMP_GREATER, 128);
	}
	MV1DrawModel(m_model.get());
	ShadowMap_DrawEnd();
	return true;
}
void MAPS::set_camerapos(VECTOR_ref pos, VECTOR_ref vec, VECTOR_ref up, float ratio) {
	camera = pos;
	viewv = vec;
	upv = up;
	rat = ratio;
}
void MAPS::set_map_shadow_near(float vier_r) {
	float shadow_dist = std::max(20.f, 10.0f * float(shadowx) * vier_r + 20.0f);
	SetShadowMapDrawArea(shadowmap[0], (camera - VScale(VGet(1.0f, 1.0f, 1.0f), shadow_dist)).get(), (camera + VScale(VGet(1.0f, 1.0f, 1.0f), shadow_dist)).get());
	SetShadowMapDrawArea(shadowmap[1], (camera - VScale(VGet(1.0f, 1.0f, 1.0f), shadow_dist * 2)).get(), (camera + VScale(VGet(1.0f, 1.0f, 1.0f), shadow_dist * 2)).get());
}
void MAPS::draw_map_track(const players& player) {
	SetDrawScreen(nor_tex.get());
	for (auto& w : player.ptr->wheelframe)
		if (player.Springs[w] >= -0.15f)
			DrawRotaGraph((int)(groundx * (0.5f + player.obj.frame(w).x() / (map_max - map_min).x())), (int)(groundx * (0.5f - player.obj.frame(w).z() / (map_max - map_min).z())), 1.f * groundx / 1024 / 195.0f, -player.yrad, nor_trk.get(), TRUE);
}
void MAPS::draw_map_model() {
	MV1DrawModel(m_model.get());
}
void MAPS::set_map_track() {
	MV1SetTextureGraphHandle(m_model.get(), 1, nor_tex.get(), FALSE);
}
void MAPS::draw_map_sky(void) {
	ClearDrawScreen();
	setcv(25.0f, 200.0f, camera, viewv, upv, 45.0f / rat);
	SetUseLighting(FALSE);
	SetFogEnable(FALSE);

	MV1SetPosition(sky_model.get(), camera.get());
	MV1DrawModel(sky_model.get());
	DrawBillboard3D((camera + lightvec.Norm().Scale(-80.0f)).get(), 0.5f, 0.5f, 3.0f, 0.0f, sky_sun.get(), TRUE);

	SetFogEnable(TRUE);
	SetUseLighting(TRUE);
}
void MAPS::set_hitplayer(VECTOR_ref pos) {
	for (auto& t : tree.tree_) {
		if (t.hit) {
			if ((t.pos - pos).size() <= 3.f) {
				t.hit = false;
				t.rad = VGet(t.rad.x(), atan2((t.pos - pos).x(), (t.pos - pos).z()), t.rad.z());
			}
		}
		else {
			if (t.rad.x() <= deg2rad(85)) {
				MV1SetRotationXYZ(t.nears.get(), t.rad.get());
				t.rad = VGet(t.rad.x() + (deg2rad(90) - t.rad.x()) * 0.002f, t.rad.y(), t.rad.z());
			}
		}
	}
}
void MAPS::draw_trees() {
	for (auto& t : tree.tree_) {
		if (CheckCameraViewClip_Box((t.pos + VGet(-10, 0, -10)).get(), (t.pos + VGet(10, 10, 10)).get()))
			tree.treesort[t.id] = pair(t.id, (map_max - map_min).x());
		else
			tree.treesort[t.id] = pair(t.id, (t.pos - camera).size());
	}
	std::sort(tree.treesort.begin(), tree.treesort.end(), [](const pair& x, const pair& y) { return x.second > y.second; });

	for (auto& tt : tree.treesort) {
		if (tt.second == (map_max - map_min).x())
			continue;
		auto& t = tree.tree_[tt.first];
		float per;
		if ((tt.second - drawdist) > 0) {
			per = std::clamp((tt.second - drawdist) / 100.0f, 0.f, 1.f);
			if (per > 0) {
				MV1SetOpacityRate(t.fars.get(), per);
				VECTOR_ref vect = t.pos - camera;
				MV1SetRotationXYZ(t.fars.get(), VGet(0.0f, atan2(vect.x(), vect.z()), 0.0f));
				MV1DrawModel(t.fars.get());
			}
		}
		if ((tt.second - drawdist) < 100) {
			per = (tt.second <= 20) ? tt.second / 20.0f : std::clamp(1.0f - (tt.second - drawdist) / 100.0f, 0.f, 1.f);
			if (per > 0) {
				MV1SetOpacityRate(t.nears.get(), per);
				MV1DrawModel(t.nears.get());
			}
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
	for (auto& t : tree.tree_) {
		t.nears.Dispose();
		t.fars.Dispose();
	}
	tree.treesort.clear();
	tree.tree_.clear();
	graph.Dispose();
	grass.Dispose();
	dif_gra.Dispose();
	nor_gra.Dispose();

	grassver.clear();
	grassind.clear();
}
void MAPS::ready_shadow(void) {
	for (int i = 0; i < shadowmap.size(); ++i)
		SetUseShadowMap(i, shadowmap[i]);
}
void MAPS::exit_shadow(void) {
	for (int i = 0; i < shadowmap.size(); ++i)
		SetUseShadowMap(i, -1);
}
void MAPS::set_normal(VECTOR_ref& nor, MATRIX& ps_n, VECTOR_ref position, const float frate, const float fps) {
	float x_nor = atan2f(nor.z(), nor.y());
	float z_nor = atan2f(-nor.x(), nor.y());
	//*
	const auto hitp = get_gnd_hit(position + VGet(0.0f, 2.0f, 0.0f), position + VGet(0.0f, -2.0f, 0.0f));
	fpsdiff(x_nor, atan2f(hitp.Normal.z, hitp.Normal.y), 0.05f);
	fpsdiff(z_nor, atan2f(-hitp.Normal.x, hitp.Normal.y), 0.05f);
	//0.20ms
	//*/

	/*
	//X
	const auto r0_0 = get_gnd_hit(position + VGet(0.0f, 2.0f, -0.5f), position + VGet(0.0f, -2.0f, -0.5f));
	if (r0_0.HitFlag) {
		const auto r0_1 = get_gnd_hit(position + VGet(0.0f, 2.0f, 0.5f), position + VGet(0.0f, -2.0f, 0.5f));
		if (r0_1.HitFlag)
			fpsdiff(x_nor, atan2(r0_0.HitPosition.y - r0_1.HitPosition.y, 1.0f), 0.05f);
	}
	//Z
	const auto r1_0 = get_gnd_hit(position + VGet(0.5f, 2.0f, 0.0f), position + VGet(0.5f, -2.0f, 0.0f));
	if (r1_0.HitFlag) {
		const auto r1_1 = get_gnd_hit(position + VGet(-0.5f, 2.0f, 0.0f), position + VGet(-0.5f, -2.0f, 0.0f));
		if (r1_1.HitFlag)
			fpsdiff(z_nor, atan2(r1_0.HitPosition.y - r1_1.HitPosition.y, 1.0f), 0.05f);
	}
	//0.38ms
	//*/

	ps_n = MMult(MGetRotX(x_nor), MGetRotZ(z_nor));
	nor = VTransform(VGet(0, 1.f, 0), ps_n);
}
//
UIS::UIS() {
	using namespace std::literals;
	WIN32_FIND_DATA win32fdt;

	countries = 1;					//国の数
	std::array<const char*, 1> country{ "German" }; // TODO: 書き換える		// TODO: Germanの部分は可変になる

	UI_main.resize(countries); /*改善*/
	SetUseASyncLoadFlag(TRUE);
	for (size_t j = 0; j < ui_reload.size(); ++j)
		ui_reload[j] = GraphHandle::Load("data/ui/ammo_" + std::to_string(j) + ".bmp"); /*弾0,弾1,弾2,空弾*/
	//
	ui_compass = GraphHandle::Load("data/ui/compass.png");
	const auto hFind = FindFirstFile("data/ui/body/*.png", &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (win32fdt.cFileName[0] == 'B')
				UI_body.emplace_back(GraphHandle::Load("data/ui/body/"s + win32fdt.cFileName));
			if (win32fdt.cFileName[0] == 'T')
				UI_turret.emplace_back(GraphHandle::Load("data/ui/body/"s + win32fdt.cFileName));
		} while (FindNextFile(hFind, &win32fdt));
	} //else{ return false; }
	FindClose(hFind);
	//
	for (size_t j = 0; j < countries; ++j) {
		for (size_t i = 0; i < 8; ++i)
			UI_main[j].ui_sight[i] = GraphHandle::Load("data/ui/"s + country[j] + "/" + std::to_string(i) + ".png");
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
		font18.DrawStringFormat(x_r(0), y_r(1080 - 24), c_00ff00, "LOADING : %06.2f%%", pers * 100.f);
		//
		DrawExtendGraph(x_r(552), y_r(401), x_r(1367), y_r(679), pad.get(), TRUE);
		int i = 0;
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "W : 前進", c_ff0000);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "S : 後退", c_ff0000);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "A : 左転", c_ff0000);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "D : 右転", c_ff0000);
		//font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "R : シフトアップ", c_ff0000);
		//font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "F : シフトダウン", c_ff0000);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "↑ : 砲昇", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "↓ : 砲降", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "← : 砲左", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "→ : 砲右", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "左CTRL : 大きく砲操作", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "左shift : 照準", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "右CTRL : ドライバー視点", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "Z : レティクル上昇", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "X : レティクル下降", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "C : ズームアウト", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "V : ズームイン", c_ff6400);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "Q : 再装填1", c_00c800);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "E : 再装填2", c_00c800);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "space : 射撃", c_00c800);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "右shift : 指揮", c_3264ff);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "マウス : 見回し/指揮", c_3264ff);
		font18.DrawString(x_r(1367), y_r(401 + 18 * i++), "ESC : 終了", c_3264ff);
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
void UIS::draw_drive() {
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawExtendGraph(0, 0, dispx, dispy, UI_main[pplayer->ptr->countryc].ui_sight[7].get(), TRUE);
}
void UIS::draw_icon(players& p, int font, float frate) {
	const auto c_00ff00 = GetColor(0, 255, 0);
	const auto c_ff0000 = GetColor(255, 0, 0);
	//font18.DrawStringFormat
	if (p.HP[0] != 0 && ((p.lost_sec != -1 && p.type == ENEMY) || (p.type == TEAM)))
		if (p.iconpos.z() > 0.0f && p.iconpos.z() < 1.0f) {
			if (p.type == ENEMY) {
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, int(255.f * (1.f - float(p.lost_sec) / (5.f * frate))));
				DrawFormatStringToHandle((int)p.iconpos.x() - GetDrawFormatStringWidthToHandle(font, "%dm", (int)(p.mine.pos - pplayer->mine.pos).size()) / 2, (int)p.iconpos.y(), c_ff0000, font, "%dm", (int)(p.mine.pos - pplayer->mine.pos).size());
				DrawFormatStringToHandle((int)p.iconpos.x() - GetDrawFormatStringWidthToHandle(font, "%s", p.ptr->name.c_str()) / 2, (int)p.iconpos.y() + y_r(20), c_ff0000, font, "%s", p.ptr->name.c_str());
			}
			else {
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
				DrawFormatStringToHandle((int)p.iconpos.x() - GetDrawFormatStringWidthToHandle(font, "%dm", (int)(p.mine.pos - pplayer->mine.pos).size()) / 2, (int)p.iconpos.y(), c_00ff00, font, "%dm", (int)(p.mine.pos - pplayer->mine.pos).size());
				DrawFormatStringToHandle((int)p.iconpos.x() - GetDrawFormatStringWidthToHandle(font, "%s", p.ptr->name.c_str()) / 2, (int)p.iconpos.y() + y_r(20), c_00ff00, font, "%s", p.ptr->name.c_str());
			}
		}
}
void UIS::draw_sight(VECTOR_ref aimpos, float ratio, float dist, int font) {
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawRotaGraph(x_r(960), y_r(540), (float)y_r(2), deg2rad(-dist / 20), UI_main[pplayer->ptr->countryc].ui_sight[1].get(), TRUE);
	DrawRotaGraph(int(aimpos.x()), int(aimpos.y()), (float)y_r(2) * ratio / 4.0f, 0, UI_main[pplayer->ptr->countryc].ui_sight[2].get(), TRUE);
	DrawRotaGraph(x_r(960), y_r(540), (float)y_r(2), 0, UI_main[pplayer->ptr->countryc].ui_sight[0].get(), TRUE);
	DrawExtendGraph(0, 0, dispx, dispy, UI_main[pplayer->ptr->countryc].ui_sight[7].get(), TRUE);
	DrawFormatStringToHandle(x_r(1056), y_r(594), GetColor(255, 255, 255), font, "[%03d]", (int)dist);
	DrawFormatStringToHandle(x_r(1056), y_r(648), GetColor(255, 255, 255), font, "[x%02.1f]", ratio);
}
void UIS::draw_ui(uint8_t selfammo[], float y_v, int font) {
	/*跳弾*/
	if (recs >= 0.01f) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(recs * 128.0f));
		DrawBox(0, 0, dispx, dispy, GetColor(255, 255, 255), TRUE);
		recs *= 0.9f;
		//recs *= pow(0.9f, frate / fps);
	}
	/*弾*/
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	if (pplayer->Gun[0].loadcnt > 0) {
		DrawRotaGraph(x_r(2112 - (int)(384 * pplayer->Gun[0].loadcnt / pplayer->ptr->gun_[0].reloadtime)), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[pplayer->ammotype].get(), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(128.0f * pow(1.0f - (float)pplayer->Gun[0].loadcnt / (float)pplayer->ptr->gun_[0].reloadtime, 10)));
		if (selfammo[0] == 0 && selfammo[1] == 0) {
			DrawRotaGraph(x_r(1536), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[3].get(), TRUE);
		}
		else {
			if (selfammo[0] > 0)
				DrawRotaGraph(x_r(1536), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[(pplayer->ammotype - 1 == -1) ? 2 : pplayer->ammotype - 1].get(), TRUE);
			else if (selfammo[1] > 0)
				DrawRotaGraph(x_r(1536), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[(pplayer->ammotype + 1 == 3) ? 0 : pplayer->ammotype + 1].get(), TRUE);
		}
	}

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	if (pplayer->Gun[0].loadcnt == 0)
		DrawRotaGraph(x_r(1536), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[pplayer->ammotype].get(), TRUE);

	for (int i = 0; i < 3; i++) {
		float tp = float((i - pplayer->ammotype >= 0) ? (i - pplayer->ammotype) : (i - pplayer->ammotype + 3));
		differential(reload_mov[i], 32.f * tp, 0.1f);
		if (i - pplayer->ammotype == 0)
			DrawRotaGraph(x_r(1728 - (int)(192 * pplayer->Gun[0].loadcnt / pplayer->ptr->gun_[0].reloadtime)), y_r(64), (double)x_r(40) / 40.0, 0.0, ui_reload[i].get(), TRUE);
		else
			DrawRotaGraph(x_r(1728 + reload_mov[i]), y_r(64 + 2 * reload_mov[i]), (double)x_r(40) / 40.0, 0.0, ui_reload[i].get(), TRUE);
		DrawFormatStringToHandle(x_r(1728 + reload_mov[i]), y_r(64 + 2 * reload_mov[i]), GetColor(255, 255, 255), font, "[x%d]", pplayer->setammo[i]);
	}

	//DrawFormatStringToHandle(x_r(960), y_r(540), GetColor(255, 255, 255), font, "[x%d]", pplayer->ammotype);


	/*速度計*/
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawExtendGraph(x_r(0), y_r(888), x_r(192), y_r(1080), UI_main[pplayer->ptr->countryc].ui_sight[3].get(), TRUE);
	DrawRotaGraph(x_r(96), y_r(984), x_r(192) / 152, deg2rad(120.0f * pplayer->spd / pplayer->ptr->speed_flont[3] - 60.f), UI_main[pplayer->ptr->countryc].ui_sight[4].get(), TRUE);

	SetDrawArea(x_r(192), y_r(892), x_r(192 + 40), y_r(892 + 54));
	DrawRotaGraph(x_r(192 + 40 / 2), y_r(892 + 54 / 2 + (int)(54.0f * gearf)), (double)x_r(40) / 40.0, 0.f, UI_main[pplayer->ptr->countryc].ui_sight[5].get(), TRUE);
	SetDrawArea(x_r(0), y_r(0), x_r(1920), y_r(1080));

	DrawExtendGraph(x_r(192), y_r(892 - 4), x_r(232), y_r(950), UI_main[pplayer->ptr->countryc].ui_sight[6].get(), TRUE);
	differential(gearf, (float)pplayer->gear, 0.1f);



	DrawRotaGraph(x_r(392), y_r(980), (double)x_r(40) / 40.0, double(-y_v), ui_compass.get(), TRUE);

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
		DrawRotaGraph(x_r(392), y_r(980), (double)x_r(40) / 40.0, double(-y_v - pplayer->yrad), UI_body[i].get(), TRUE);
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
		DrawRotaGraph(x_r(392), y_r(980), (double)x_r(40) / 40.0, double(-y_v - pplayer->yrad + pplayer->gunrad.x()), UI_turret[i].get(), TRUE);
	}
	DrawFormatStringToHandle(x_r(0), y_r(1080 - 200), GetColor(255, 255, 255), font, "[LIFE : %d]", pplayer->HP[0]);
}
/*debug*/
void UIS::put_way(void) {
	waypoint = GetNowHiPerformanceCount();
	seldeb = 0;
}
void UIS::end_way(void) {
	if (seldeb < 6)
		waydeb[seldeb++] = (float)(GetNowHiPerformanceCount() - waypoint) / 1000.0f;
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
	DrawFormatString(100, 100 + 0, c_ffffff, "%05.2ffps ( %.2fms)(total %.2fms)", fps, time, 1000.0f / fps);

	DrawFormatString(100, 100 + 18, c_ffffff, "%d(%.2fms)", 0, waydeb[0]);
	for (size_t j = 1; j < std::size(waydeb); ++j) {
		DrawFormatString(100, int(100 + 18 + j * 18), c_ffffff, "%d(%.2fms)", j, waydeb[j] - waydeb[j - 1u]);
	}
}
//
void setcv(float neard, float fard, VECTOR_ref cam, VECTOR_ref view, VECTOR_ref up, float fov) {
	SetCameraNearFar(neard, fard);
	SetCameraPositionAndTargetAndUpVec(cam.get(), view.get(), up.get());
	SetupCamera_Perspective(deg2rad(fov));
	Set3DSoundListenerPosAndFrontPosAndUpVec(cam.get(), view.get(), up.get());
}
void getdist(VECTOR_ref& startpos, VECTOR_ref vec, float& dist, float& getdists, float speed, float fps) {
	dist = std::clamp(dist, 100.f, 2000.f);
	speed /= fps;
	auto endpos = startpos;
	for (int z = 0; z < (int)(fps / 1000.0f * dist); ++z) {
		startpos += vec.Scale(speed);
		vec = VGet(vec.x(), vec.y() + m_ac(fps), vec.z());
	}
	getdists = (endpos - startpos).size();
}
//
void set_effect(EffectS* efh, VECTOR_ref pos, VECTOR_ref nor) {
	efh->flug = true;
	efh->pos = pos;
	efh->nor = nor;
}
void set_pos_effect(EffectS* efh, const EffekseerEffectHandle& handle) {
	if (efh->flug) {
		efh->handle = handle.Play3D();
		efh->handle.SetPos(efh->pos);
		efh->handle.SetRotation(atan2(efh->nor.y(), std::hypot(efh->nor.x(), efh->nor.z())), atan2(-efh->nor.x(), -efh->nor.z()), 0);
		efh->flug = false;
	}
	//IsEffekseer3DEffectPlaying(player[0].effcs[i].handle)
}
//
bool get_reco(players& play, std::vector<players>& tgts, ammos& c, size_t gun_s) {
	bool is_hit;
	std::optional<size_t> hitnear;

	for (auto& t : tgts) {
		if (play.id == t.id)
			continue;

		//とりあえず当たったかどうか探してソート
		is_hit = false;
		for (size_t colmesh = 0; colmesh < t.hitssort.size(); ++colmesh) {
			if (colmesh >= 5 && t.HP[colmesh] == 0) {
				t.hitssort[colmesh] = pair(colmesh, (std::numeric_limits<float>::max)());
				continue;
			}
			t.hitres[colmesh] = MV1CollCheck_Line(t.colobj.get(), -1, c.repos.get(), (c.pos + (c.pos - c.repos).Scale(0.1f)).get(), int(colmesh));
			if (t.hitres[colmesh].HitFlag) {
				t.hitssort[colmesh] = pair(colmesh, (c.repos - t.hitres[colmesh].HitPosition).size());
				is_hit = true;
			}
			else
				t.hitssort[colmesh] = pair(colmesh, (std::numeric_limits<float>::max)());
		}
		if (!is_hit)
			continue;
		std::sort(t.hitssort.begin(), t.hitssort.end(), [](const pair& x, const pair& y) { return x.second < y.second; });

		//主砲
		if (gun_s == 0) {
			//近い順に、はじく操作のいらないメッシュに対しダメージ面に届くまで判定
			for (auto& tt : t.hitssort) {
				if (tt.second == (std::numeric_limits<float>::max)())
					break; //装甲面に当たらなかったならスルー
				const auto k = tt.first;
				if (k <= 3) {
					hitnear = k;
					break;
				}
				else if (t.HP[k] > 0) {
					if (k == 4)
						continue; //砲身だけ処理を別にしたいので分けます
					//空間装甲、モジュール
					if (t.hitres[k].HitFlag) {
						set_effect(&play.effcs[ef_reco], t.hitres[k].HitPosition, t.hitres[k].Normal);
						t.HP[k] = std::max<short>(t.HP[k] - 30, 0); //
						c.pene /= 2.0f;
						c.speed /= 2.f;
					}
				}
			}
			//ダメージ面に当たった時に装甲値に勝てるかどうか
			if (hitnear.has_value()) {
				const auto k = hitnear.value();
				MV1SetFrameUserLocalMatrix(t.colobj.get(), 9 + 0 + 3 * t.hitbuf, MMult(MGetTranslate(t.hitres[k].HitPosition), MInverse(t.ps_m)));
				MV1SetFrameUserLocalMatrix(t.colobj.get(), 9 + 1 + 3 * t.hitbuf, MMult(MGetTranslate(VAdd(t.hitres[k].Normal, t.hitres[k].HitPosition)), MInverse(t.ps_m)));
				MV1SetFrameUserLocalMatrix(t.colobj.get(), 9 + 2 + 3 * t.hitbuf, MMult(((c.vec * t.hitres[k].Normal).Scale(-1.f) + t.hitres[k].HitPosition).Mtrans(), MInverse(t.ps_m)));
				//
				set_effect(&play.effcs[ef_reco], t.hitres[k].HitPosition, t.hitres[k].Normal);
				if (c.pene > t.ptr->armer[k] * (1.0f / abs(c.vec.Norm() % t.hitres[k].Normal))) {
					if (t.HP[0] != 0) {
						if (t.HP[0] == 1) {
							PlaySoundMem(t.se[29 + GetRand(1)].get(), DX_PLAYTYPE_BACK, TRUE);
							set_effect(&t.effcs[ef_bomb], t.obj.frame(t.ptr->engineframe), VGet(0, 0, 0));
							if (play.hitadd == false) {
								play.hitadd = true;
								play.hitid = int(t.id);
							}
						}
					}
					c.flug = false;
					t.HP[0] = std::max<short>(t.HP[0] - 1, 0); //
					t.hit[t.hitbuf].use = 0;
				}
				else {
					PlaySoundMem(t.se[10 + GetRand(16)].get(), DX_PLAYTYPE_BACK, TRUE);
					if (t.recorad == 180) {
						float rad = atan2(t.hitres[k].HitPosition.x - t.mine.pos.x(), t.hitres[k].HitPosition.z - t.mine.pos.z());
						t.recovec = VGet(cos(rad), 0, -sin(rad));
						t.recorad = 0;
					}
					c.vec += VScale(t.hitres[k].Normal, (c.vec % t.hitres[k].Normal) * -2.0f);
					c.pos = c.vec.Scale(0.1f) + t.hitres[k].HitPosition;
					c.pene /= 2.0f;
					c.speed /= 2.f;
					t.hit[t.hitbuf].use = 1;
				}
				{
					float asize = play.ptr->gun_[gun_s].ammosize * 100.f;
					MV1SetScale(t.hit[t.hitbuf].pic.get(), VGet(asize / abs(c.vec.Norm() % t.hitres[k].Normal), asize, asize)); //
				}
				t.hit[t.hitbuf].flug = true;
				++t.hitbuf %= 3;
			}
		}
		//同軸機銃
		else {
			//至近で弾かせる
			if (t.hitssort.begin()->second == (std::numeric_limits<float>::max)())
				continue;
			hitnear = t.hitssort.begin()->first;
			if (hitnear.has_value()) {
				set_effect(&play.effcs[ef_reco2], t.hitres[hitnear.value()].HitPosition, t.hitres[hitnear.value()].Normal);
				PlaySoundMem(t.se[10 + GetRand(16)].get(), DX_PLAYTYPE_BACK, TRUE);
				c.vec = c.vec + VScale(t.hitres[hitnear.value()].Normal, (c.vec % t.hitres[hitnear.value()].Normal) * -2.0f);
				c.pos = c.vec.Scale(0.1f) + t.hitres[hitnear.value()].HitPosition;

				if (hitnear.value() >= 5 && hitnear.value() < t.HP.size()) {
					t.HP[hitnear.value()] = std::max<short>(t.HP[hitnear.value()] - 13, 0); //
				}
			}
		}
		if (hitnear.has_value())
			break;
	}
	return (hitnear.has_value());
}
void set_gunrad(players& play, float rat_r) {
	for (int i = 0; i < 4; ++i) {
		if ((play.move & (KEY_TURNLFT << i)) != 0) {
			switch (i) {
			case 0:
				play.gunrad = VGet(play.gunrad.x() - play.ptr->gun_RD / rat_r, play.gunrad.y(), play.gunrad.z());
				if (!play.ptr->gun_lim_LR)
					play.gunrad = VGet(std::max<float>(play.gunrad.x(), play.ptr->gun_lim_[0]), play.gunrad.y(), play.gunrad.z());
				break;
			case 1:
				play.gunrad = VGet(play.gunrad.x() + play.ptr->gun_RD / rat_r, play.gunrad.y(), play.gunrad.z());
				if (!play.ptr->gun_lim_LR)
					play.gunrad = VGet(std::min<float>(play.gunrad.x(), play.ptr->gun_lim_[1]), play.gunrad.y(), play.gunrad.z());
				break;
			case 2:
				play.gunrad = VGet(play.gunrad.x(), std::min<float>(play.gunrad.y() + (play.ptr->gun_RD / 2.f) / rat_r, play.ptr->gun_lim_[2]), play.gunrad.z());
				break;
			case 3:
				play.gunrad = VGet(play.gunrad.x(), std::max<float>(play.gunrad.y() - (play.ptr->gun_RD / 2.f) / rat_r, play.ptr->gun_lim_[3]), play.gunrad.z());
				break;
			}
		}
	}
}
bool set_shift(players& play) {
	const auto gearrec = play.gear;
	/*自動変速機*/
	if (play.gear > 0 && play.gear <= 3)
		if (play.spd >= play.ptr->speed_flont[play.gear - 1] * 0.9)
			++play.gear;
	if (play.gear < 0 && play.gear >= -3)
		if (play.spd <= play.ptr->speed_back[-play.gear - 1] * 0.9)
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

SOLDIERS::SOLDIERS(float frates) {
	using namespace std::literals;
	f_rate = frates;
	SetUseASyncLoadFlag(TRUE);
	for (size_t i = 0; i < model.size(); i++)
		model[i] = MV1ModelHandle::Load("data/soldier/model"s + std::to_string(i) + ".mv1"); /*兵士*/
	SetUseASyncLoadFlag(FALSE);
}
void SOLDIERS::set_camerapos(VECTOR_ref pos, VECTOR_ref vec, VECTOR_ref up, float ratio) {
	camera = pos;
	viewv = vec;
	upv = up;
	rat = ratio;
}
void SOLDIERS::set_soldier(const uint8_t type, const VECTOR_ref position, const float rad) {
	sort.resize(sort.size() + 1);
	sol.resize(sol.size() + 1);
	sol.back().type = type;
	sol.back().atkf.reset();
	sol.back().id = sol.size() - 1;
	sol.back().HP = 1;
	sol.back().useammo = 0;
	sol.back().waynow = 0;
	sol.back().obj = model[GetRand(1)].Duplicate();
	sol.back().pos = position;
	std::fill(std::begin(sol.back().waypoint), std::end(sol.back().waypoint), sol.back().pos + VGet(0, 0, (sol.back().type == TEAM) ? 100.f : -100.f));
	sol.back().yrad = rad;
	for (int j = 0; j < sol.back().obj.material_num(); ++j) {
		MV1SetMaterialDifColor(sol.back().obj.get(), j, GetColorF(0.5f, 0.5f, 0.5f, 1.0f));
		MV1SetMaterialSpcColor(sol.back().obj.get(), j, GetColorF(0.35f, 0.32f, 0.28f, 0.5f));
		MV1SetMaterialSpcPower(sol.back().obj.get(), j, 1.0f);
	}
	//アニメーション
	for (int j = 0; j < sol.back().amine.size(); ++j) {
		sol.back().amine[j].id = MV1AttachAnim(sol.back().obj.get(), j, -1, TRUE);
		sol.back().amine[j].total = MV1GetAttachAnimTotalTime(sol.back().obj.get(), sol.back().amine[j].id);
		sol.back().amine[j].time = 0.0f;
		sol.back().amine[j].per = 0.f;
		MV1SetAttachAnimBlendRate(sol.back().obj.get(), sol.back().amine[j].id, sol.back().amine[j].per);
	}
}
void SOLDIERS::set_soldier_vol(unsigned char size) {
}
void SOLDIERS::set_soldiermove(int map, std::vector<players>& play) {
	//CPU
	for (auto& s : sol) {
		if (s.HP > 0) {
			if (s.atkf.has_value()) {
				VECTOR_ref tempv = (sol[s.atkf.value()].pos - s.pos).Norm();
				if (((-cos(s.yrad)) * tempv.x() - (-sin(s.yrad)) * tempv.z()) < 0)
					s.yrad -= deg2rad(120.f) / f_rate;
				else
					s.yrad += deg2rad(120.f) / f_rate;

				if (s.amine[7].time >= s.amine[7].total) {
					s.useanime = 8;
					if (s.amine[s.useanime].time >= s.amine[s.useanime].total) {
						s.amine[s.useanime].time = 0.0f;
					}
					if (s.amine[s.useanime].time == 0.0f) {
						s.ammo[s.useammo].flug = true;
						s.ammo[s.useammo].speed = 600 / f_rate;
						s.ammo[s.useammo].pos = s.pos + VGet(0, 1.3f, 0);
						s.ammo[s.useammo].repos = s.ammo[s.useammo].pos;
						s.ammo[s.useammo].cnt = 0;
						s.ammo[s.useammo].color = GetColor(255, 255, 0);


						const auto v = sol[s.atkf.value()].pos + VGet(0, 1.3f, 0) - s.ammo[s.useammo].pos;
						const auto y = atan2(v.x(), v.z()) + deg2rad((float)(GetRand(10000 * 2) - 10000) / 10000.f);
						const auto x = atan2(-v.y(), std::hypot(v.x(), v.z())) - deg2rad((float)(GetRand(10000 * 2) - 10000) / 10000.f);
						s.ammo[s.useammo].vec = VGet(cos(x) * sin(y), -sin(x), cos(x) * cos(y));

						++s.useammo %= ammoc;
					}
					if (sol[s.atkf.value()].HP == 0) {
						s.atkf.reset();
					}
				}
				else {
					if (s.amine[6].time >= s.amine[6].total) {
						s.amine[6].time = 0.0f;
						s.useanime = 7;
					}
					else {
						s.useanime = 6;
					}
				}
			}
			else {
				{
					float distp = 9999.f;
					for (auto& p : play) {
						if (p.type != s.type || p.HP[0] == 0)
							continue;
						if ((p.mine.pos - s.pos).size() < distp) {
							distp = (p.mine.pos - s.pos).size();
							std::fill(std::begin(s.waypoint), std::end(s.waypoint), p.mine.pos - p.zvec.Scale(5.f));
						}
					}
				}
				if ((s.waypoint[s.waynow] - s.pos).size() > 6.f) {
					VECTOR_ref tempv = (s.waypoint[s.waynow] - s.pos).Norm();

					for (auto& t : sol) {
						if (t.id == s.id)
							continue;
						if ((t.pos - s.pos).size() < 1.f) {
							tempv = (s.pos - t.pos).Norm();
							break;
						}
					}

					if (((-cos(s.yrad)) * tempv.x() - (-sin(s.yrad)) * tempv.z()) < 0)
						s.yrad -= deg2rad(120.f) / f_rate;
					else
						s.yrad += deg2rad(120.f) / f_rate;

					float spd = ((float(500 + GetRand(1000)) / 100) / 3.6f) / f_rate;
					s.pos += VGet(-sin(s.yrad) * spd, 0, -cos(s.yrad) * spd);
					s.useanime = 5;
					if (s.amine[s.useanime].time >= s.amine[s.useanime].total)
						s.amine[s.useanime].time = 0.0f;

					if (count % 30 == s.id % 30) {
						for (auto& t : sol) {
							if (s.type == t.type || s.id == t.id || t.HP == 0 || (s.pos - t.pos).size() > 550)
								continue;
							const auto hit = MV1CollCheck_Line(map, 0, (s.pos + VGet(0, 1.f, 0)).get(), (t.pos + VGet(0, 1.f, 0)).get());
							if (!hit.HitFlag) {
								s.atkf = t.id;
								break;
							}
						}
					}
				}
				else {
					s.waynow = std::min<uint8_t>(s.waynow + 1, waypc - 1);
					s.pos += VGet(0, 0, 0);
					s.useanime = 3;
					if (s.amine[s.useanime].time >= s.amine[s.useanime].total)
						s.amine[s.useanime].time = 0.0f;
				}
			}
		}
		else {
			s.useanime = 11;
		}
		for (auto& c : s.ammo)
			if (c.flug) {
				c.repos = c.pos;
				c.pos += c.vec.Scale(c.speed);
				const auto hit = MV1CollCheck_Line(map, 0, c.repos.get(), c.pos.get());
				if (hit.HitFlag)
					c.pos = hit.HitPosition;

				for (auto& p : play) {
					if (Segment_Point_MinLength(c.pos.get(), c.repos.get(), (p.mine.pos + VGet(0, 1.f, 0)).get()) < 3.f) {
						c.flug = false;
						continue;
					}
				}

				if (c.cnt > 0)
					set_hit(c.pos, c.repos);

				if (hit.HitFlag) {
					//set_effect(&p.effcs[ef_gndhit2], hit.HitPosition, hit.Normal);
					c.vec += VScale(hit.Normal, (c.vec % hit.Normal) * -2.0f);
					c.pos = c.vec.Scale(0.01f) + hit.HitPosition;
					c.speed /= 2.f;
				}

				c.vec = VGet(c.vec.x(), c.vec.y() + m_ac(f_rate / 2), c.vec.z());
				c.speed -= 5.f / f_rate;
				c.cnt++;
				if (c.cnt > (f_rate * 3.f) || c.speed <= 0.f)
					c.flug = false; //3秒で消える
			}
	}
	//
	if (count % 30 == 0) {
		for (auto& s : sol) {
			const auto hit = MV1CollCheck_Line(map, 0, (s.pos + VGet(0, 500, 0)).get(), (s.pos + VGet(0, -500, 0)).get());
			s.pos = hit.HitPosition;
		}
	}
	if (count % 60 == 0) {
		count = 0;
	}
	count++;
	for (auto& s : sol) {
		MV1SetMatrix(s.obj.get(), MMult(MGetRotY(s.yrad), s.pos.Mtrans()));
		for (size_t j = 0; j < s.amine.size(); ++j) {
			if (j == s.useanime)
				differential(s.amine[j].per, 1.f, 0.1f);
			else
				s.amine[j].per *= 0.9f;
			MV1SetAttachAnimBlendRate(s.obj.get(), s.amine[j].id, s.amine[j].per);
			MV1SetAttachAnimTime(s.obj.get(), s.amine[j].id, s.amine[j].time);
			s.amine[j].time += 30.f / f_rate; //
		}
	}
}
void SOLDIERS::set_hit(VECTOR_ref pos, VECTOR_ref repos) {
	for (auto& s : sol) {
		if (s.HP > 0 && Segment_Point_MinLength(pos.get(), repos.get(), (s.pos + VGet(0, 1.f, 0)).get()) < 1.f) {
			s.HP = 0;
			s.yrad += deg2rad(-90 + GetRand(180));
		}
	}
}
void SOLDIERS::draw_soldiersammo() {
	for (auto& s : sol)
		for (size_t i = 0; i < ammoc; i+=2)
			if (s.ammo[i].flug) {
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * std::min<float>(1.f, 4.f * s.ammo[i].speed / (600 / f_rate))));
				DrawCapsule3D(s.ammo[i].pos.get(), s.ammo[i].repos.get(), 0.0075f * ((s.ammo[i].pos - camera).size() / 60.f), 4, s.ammo[i].color, GetColor(255, 255, 255), TRUE);
			}
}
void SOLDIERS::draw_soldiers() {
	size_t cnt = 0;
	for (auto& t : sol) {
		if (CheckCameraViewClip_Box((t.pos - VGet(-0.5f, 0, -0.5f)).get(), (t.pos - VGet(0.5f, 2.0f, 0.5f)).get()) == TRUE || (camera - t.pos).size() > 550)
			sort[t.id] = pair(t.id, 9999.f);
		else
			sort[t.id] = pair(t.id, (t.pos - camera).size());
	}
	std::sort(sort.begin(), sort.end(), [](const pair& x, const pair& y) { return x.second < y.second; });

	for (auto& tt : sort) {
		if (tt.second == 9999.f)
			break;
		if (++cnt > 30)
			break;
		MV1DrawModel(sol[tt.first].obj.get());
	}
}
void SOLDIERS::delete_soldiers(void) {
	for (auto& s : sol)
		s.obj.Dispose();
	sol.clear();
}
