#include "define.h"
#include <memory>
size_t count_impl(const TCHAR* pattern) {
	WIN32_FIND_DATA win32fdt;
	size_t cnt = 0;
	const auto hFind = FindFirstFile(pattern, &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do { if (win32fdt.cFileName[0] != '.') { ++cnt; } } while (FindNextFile(hFind, &win32fdt));
	}
	FindClose(hFind);
	return cnt;
}
size_t count_team() {
	return count_impl("stage/data_0/team/*.txt");
}
size_t count_enemy() {
	return count_impl("stage/data_0/enemy/*.txt");
}
/*main*/
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	//temp------------------------------------------------------------------//
	int i, j, k, tgt_p;
	int mousex, mousey;							/*mouse*/
	float tmpf, tempfx, tempfy,turn_bias;
	VECTOR tempvec[2];
	bool btmp;
	//int lookplayerc;							/*視認しているplayer人数*/
	bool keyget[19]{ false };						/*キー用*/
	//変数------------------------------------------------------------------//
	bool out{ false };							/*終了フラグ*/
	std::vector<pair> pssort;						/*playerソート*/
	std::vector<players> player;						/*player*/
	VECTOR aims;								/*照準器座標*/
	float aimm;								/*照準距離*/
	float fps;								/*fps*/
	int selfammo;								/*UI用*/
	switches aim, map;							/*視点変更*/
	float ratio, rat_r, aim_r;						/*照準視点　倍率、実倍率、距離*/
	size_t waysel, choose = (std::numeric_limits<size_t>::max)();						/*指揮視点　指揮車両、マウス選択*/
	std::uint8_t way = 0; //マウストリガー
	LONGLONG old_time, waits;						/*時間取得*/
	VECTOR campos, viewpos, uppos;						/*カメラ*/
	MV1_COLL_RESULT_POLY HitPoly;						/*あたり判定*/
	float cpu_move;
	char mstr[64];								/*tank*/
	int mdata;								/*tank*/
	//init------------------------------------------------------------------//
	Myclass	parts;
	HUMANS	humanparts(parts.get_usegrab(), parts.get_f_rate());		/*車内関係*/
	MAPS	mapparts(parts.get_gndx(), parts.get_drawdist());		/*地形、ステージ関係*/
	auto uiparts = std::make_unique<UIS>();
	float f_rates = parts.get_f_rate();
	//load------------------------------------------------------------------//
	parts.set_fonts(18);
	SetUseASyncLoadFlag(TRUE);
		//hit-------------------------------------------------------------------//
		const auto hit_mod = MV1ModelHandle::Load("data/hit/hit.mv1");
		//screen----------------------------------------------------------------//
		int minimap = MakeScreen(dispx, dispy, FALSE);				/*ミニマップ*/
		int skyscreen = MakeScreen(dispx, dispy, FALSE);			/*空*/
		int mainscreen = MakeScreen(dispx, dispy, FALSE);			/*遠景*/
		int HighBrightScreen = MakeScreen(dispx, dispy, FALSE);			/*エフェクト*/
		int GaussScreen = MakeScreen(dispx / EXTEND, dispy / EXTEND, FALSE);	/*エフェクト*/
	SetUseASyncLoadFlag(FALSE);
	uiparts->draw_load();//
	if (parts.set_veh() != true) { return -1; }
	/*物理開始*/
	auto world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f));					// 剛体を保持およびシミュレートするワールドオブジェクトを構築
	//これ以降繰り返しロード------------------------------------------------//
	do {
		k = parts.window_choosev(); if (k == -1) { return 0; }		/*player指定*/

		mdata = FileRead_open("stage/data_0/main.txt", FALSE);
		FileRead_gets(mstr, 64, mdata);


		const int mapc = int(std::stof(getright(mstr)));

//		const bool mapc = bool(std::stoul(getright(mstr)));
		FileRead_close(mdata);

		const size_t teamc = count_team();
		const size_t enemyc = count_enemy();
		const size_t playerc = teamc + enemyc;

		player.resize(playerc);
		pssort.resize(playerc);
		//設定
		for (size_t p_cnt = 0; p_cnt < teamc; ++p_cnt) {
			mdata = FileRead_open(("stage/data_0/team/" + std::to_string(p_cnt) + ".txt").c_str(), FALSE);
			FileRead_close(mdata);

			if (p_cnt == 0) { player[p_cnt].use = k; }
			else { player[p_cnt].use = 2; }//p_cnt % parts.get_vehc(); }
			player[p_cnt].pos = VGet(20.0f * p_cnt, 0.0f, -400.0f);
			player[p_cnt].type = TEAM;
			player[p_cnt].yrad = DX_PI_F * player[p_cnt].type;
			for (i = 0; i < waypc; ++i) {
				player[p_cnt].waypos[i] = player[p_cnt].pos;
				player[p_cnt].wayspd[i] = 2;
			}
		}
		for (size_t p_cnt = teamc; p_cnt < playerc; ++p_cnt) {
			mdata = FileRead_open(("stage/data_0/enemy/" + std::to_string(p_cnt) + ".txt").c_str(), FALSE);
			FileRead_close(mdata);

			player[p_cnt].use = 1;// p_cnt % parts.get_vehc();
			player[p_cnt].pos = VGet(20.0f * (p_cnt - teamc), 0.0f, 400.0f);
			player[p_cnt].type = ENEMY;
			player[p_cnt].yrad = DX_PI_F * player[p_cnt].type;
			for (i = 0; i < waypc; ++i) {
				player[p_cnt].waypos[i] = player[p_cnt].pos;
				player[p_cnt].wayspd[i] = 2;
			}
		}
		/*vehsから引き継ぎ*/
		for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
			player[p_cnt].ptr = parts.get_vehicle(player[p_cnt].use);
		}
		/*UI*/
		uiparts->set_state(&player[0]);
		/*load*/
		SetUseASyncLoadFlag(TRUE);
			/*players*/
			SetCreate3DSoundFlag(TRUE);
			for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
				player[p_cnt].obj = player[p_cnt].ptr->model.Duplicate();
				player[p_cnt].colobj = player[p_cnt].ptr->colmodel.Duplicate();
				for (i = 0; i < 3; i++) { player[p_cnt].hitpic[i] = hit_mod.Duplicate(); }
				player[p_cnt].se[0] = SoundHandle::Load("data/audio/se/engine/0.wav");
				player[p_cnt].se[1] = SoundHandle::Load("data/audio/se/fire/gun.wav");
				for (i = 2; i < 10; ++i) {
					player[p_cnt].se[i] = SoundHandle::Load("data/audio/se/fire/" + std::to_string(i - 2) + ".wav");
				}
				for (i = 10; i < 27; ++i) {
					player[p_cnt].se[i] = SoundHandle::Load("data/audio/se/ricochet/" + std::to_string(i - 10) + ".wav");
				}
				for (i = 27; i < 29; ++i) {
					player[p_cnt].se[i] = SoundHandle::Load("data/audio/se/engine/o" + std::to_string(i - 27) + ".wav");
				}
				for (i = 29; i < 31; ++i) {
					player[p_cnt].se[i] = SoundHandle::Load("data/audio/se/battle/hit_enemy/" + std::to_string(i - 29) + ".wav");
				}
		}
		SetCreate3DSoundFlag(FALSE);
		SetUseASyncLoadFlag(FALSE);
		mapparts.set_map_readyb(mapc);
		uiparts->draw_load();//
		/*human*/
		humanparts.set_humans(player[0].ptr->inmodel);
		/*map*/
		if (mapparts.set_map_ready() != true) { break; }
		//players
		const auto c_ffff96 = GetColor(255, 255, 150);
		const auto c_ffc896 = GetColor(255, 200, 150);
		for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
			//色調
			for (i = 0; i < MV1GetMaterialNum(player[p_cnt].obj.get()); ++i) {
				MV1SetMaterialSpcColor(player[p_cnt].obj.get(), i, GetColorF(0.85f, 0.82f, 0.78f, 0.5f));
				MV1SetMaterialSpcPower(player[p_cnt].obj.get(), i, 5.0f);
			}
			MV1SetMaterialDrawAlphaTestAll(player[p_cnt].obj.get(), TRUE, DX_CMP_GREATER, 128);
			//リセット
			player[p_cnt].hitbuf = 0;
			player[p_cnt].gear = 0;
			//cpu
			player[p_cnt].atkf = std::nullopt;
			player[p_cnt].aim = -2;
			//hit
			for (i = 0; i < player[p_cnt].ptr->colmeshes; ++i) { MV1SetupCollInfo(player[p_cnt].colobj.get(), -1, 5, 5, 5); }
			player[p_cnt].hitssort.resize(player[p_cnt].ptr->colmeshes);
			//ammo
			player[p_cnt].Ammo.resize(ammoc*gunc);
			for (size_t i = 0; i < ammoc * gunc; ++i) {
				player[p_cnt].Ammo[i].color = (player[p_cnt].type == TEAM) ? c_ffff96 : c_ffc896;
			}

			//HP
			player[p_cnt].HP.resize(player[p_cnt].ptr->colmeshes);
			/*3456は装甲部分なので詰め込む*/
			player[p_cnt].HP[0] = 1;								//life
			for (i = 4; i < player[p_cnt].ptr->colmeshes; ++i) { player[p_cnt].HP[i] = 100; }	//spaceARMER
			//wheel
			player[p_cnt].Springs.resize(player[p_cnt].ptr->frames);
			//0初期化いる
		//
			MV1SetMatrix(player[p_cnt].colobj.get(), MGetTranslate(VGet(0, 0, 0)));
			//装てん
			player[p_cnt].loadcnt[0] = 1;
			//ypos反映
			HitPoly = MV1CollCheck_Line(mapparts.get_map_handle().get(), 0, VAdd(player[p_cnt].pos, VGet(0.0f, (float)map_x, 0.0f)), VAdd(player[p_cnt].pos, VGet(0.0f, -(float)map_x, 0.0f)));
			if (HitPoly.HitFlag) { player[p_cnt].pos.y = HitPoly.HitPosition.y; }
			for (i = 0; i < waypc; ++i) {
				HitPoly = MV1CollCheck_Line(mapparts.get_map_handle().get(), 0, VAdd(player[p_cnt].waypos[i], VGet(0.0f, (float)map_x, 0.0f)), VAdd(player[p_cnt].waypos[i], VGet(0.0f, -(float)map_x, 0.0f)));
				if (HitPoly.HitFlag) { player[p_cnt].waypos[i].y = HitPoly.HitPosition.y; }
			}
			//
		}
		//物理set
		for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
			player[p_cnt].dynamicBox.SetAsBox(
				(player[p_cnt].ptr->coloc[0].x - player[p_cnt].ptr->coloc[2].x) / 2,
				(player[p_cnt].ptr->coloc[0].z - player[p_cnt].ptr->coloc[2].z) / 2,
				b2Vec2(
				(player[p_cnt].ptr->coloc[2].x + player[p_cnt].ptr->coloc[0].x) / 2,
					(player[p_cnt].ptr->coloc[2].z + player[p_cnt].ptr->coloc[0].z) / 2
				),
				0.f
			);
			player[p_cnt].fixtureDef.shape = &(player[p_cnt].dynamicBox);
			player[p_cnt].fixtureDef.density = 1.0f;								// ボックス密度をゼロ以外に設定すると、動的になります。
			player[p_cnt].fixtureDef.friction = 0.3f;								// デフォルトの摩擦をオーバーライドします。
			player[p_cnt].bodyDef.type = b2_dynamicBody;
			player[p_cnt].bodyDef.position.Set(player[p_cnt].pos.x, player[p_cnt].pos.z);
			player[p_cnt].bodyDef.angle = -player[p_cnt].yrad;
			player[p_cnt].body.reset(world->CreateBody(&(player[p_cnt].bodyDef)));
			player[p_cnt].playerfix = player[p_cnt].body->CreateFixture(&(player[p_cnt].fixtureDef));		// シェイプをボディに追加します。
		}
		/*音量調整*/
		humanparts.set_humanvc_vol(255);
		parts.set_se_vol(128);
		for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
			for (i = 1; i < 27; ++i) { ChangeVolumeSoundMem(128, player[p_cnt].se[i].get()); }
			for (i = 29; i < 31; ++i) { ChangeVolumeSoundMem(128, player[p_cnt].se[i].get()); }
		}
		/*メインループ*/
		aim.flug = false;
		map.flug = false;
		selfammo = 0;
		ratio = 1.0f;			/*照準視点　倍率*/
		rat_r = ratio;			/*照準視点　実倍率*/
		aim_r = 100.0f;			/*照準視点　距離*/
		waysel = 1;			/*指揮視点　指揮車両*/
		parts.set_viewrad(VGet(0.f, player[0].yrad, 1.f));
		SetCursorPos(x_r(960), y_r(540));
		old_time = GetNowHiPerformanceCount() + (LONGLONG)(1000000.0f / f_rates);
		for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
			player[p_cnt].effcs[ef_smoke2].efhandle = parts.get_effHandle(ef_smoke2).Play3D();
			player[p_cnt].effcs[ef_smoke3].efhandle = parts.get_effHandle(ef_smoke2).Play3D();
			PlaySoundMem(player[p_cnt].se[0].get(), DX_PLAYTYPE_LOOP, TRUE);
			PlaySoundMem(player[p_cnt].se[27].get(), DX_PLAYTYPE_LOOP, TRUE);
			PlaySoundMem(player[p_cnt].se[28].get(), DX_PLAYTYPE_LOOP, TRUE);
			Set3DRadiusSoundMem(200.0f, player[p_cnt].se[0].get());
			Set3DRadiusSoundMem(200.0f, player[p_cnt].se[1].get());
			for (i = 2; i < 10; ++i) { Set3DRadiusSoundMem(300.0f, player[p_cnt].se[i].get()); }
			for (i = 10; i < 27; ++i) { Set3DRadiusSoundMem(100.0f, player[p_cnt].se[i].get()); }
			for (i = 27; i < 29; ++i) { Set3DRadiusSoundMem(200.0f, player[p_cnt].se[i].get()); }
			for (i = 29; i < 31; ++i) { Set3DRadiusSoundMem(300.0f, player[p_cnt].se[i].get()); }
		}
		const auto c_000000 = GetColor(0, 0, 0);
		const auto c_00ff00 = GetColor(0, 255, 0);
		const auto c_ff0000 = GetColor(255, 0, 0);
		const auto c_008000 = GetColor(0, 128, 0);
		const auto c_800000 = GetColor(128, 0, 0);
		const auto c_ffff00 = GetColor(255, 255, 0);
		const auto c_c8c800 = GetColor(200, 200, 0);
		const auto c_c0ff00 = GetColor(192, 255, 0);
		const auto c_808080 = GetColor(128, 128, 128);
		const auto c_ffffff = GetColor(255, 255, 255);
		const auto c_3232ff = GetColor(50, 50, 255);
		while (ProcessMessage() == 0) {
			/*fps*/
			waits = GetNowHiPerformanceCount();
			fps = 1000000.0f / (float)(waits - old_time);
			old_time = GetNowHiPerformanceCount();
			uiparts->put_way();//debug
			if (GetActiveFlag() == TRUE) {
				SetMouseDispFlag(FALSE);
				if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) { out = true; break; }											/*終了*/
				if (CheckHitKey(KEY_INPUT_P) != 0) { break; }													/*リスタート*/
				keyget[0] = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
				if (player[0].HP[0] > 0) {
					keyget[1] = CheckHitKey(KEY_INPUT_RSHIFT) != 0;
					keyget[2] = CheckHitKey(KEY_INPUT_LSHIFT) != 0;
					keyget[3] = CheckHitKey(KEY_INPUT_V) != 0;
					keyget[4] = CheckHitKey(KEY_INPUT_C) != 0;
					keyget[5] = CheckHitKey(KEY_INPUT_X) != 0;
					keyget[6] = CheckHitKey(KEY_INPUT_Z) != 0;
					keyget[7] = CheckHitKey(KEY_INPUT_Q) != 0;
					keyget[8] = CheckHitKey(KEY_INPUT_LCONTROL) != 0;
					keyget[9] = CheckHitKey(KEY_INPUT_W) != 0;
					keyget[10] = CheckHitKey(KEY_INPUT_S) != 0;
					keyget[11] = CheckHitKey(KEY_INPUT_A) != 0;
					keyget[12] = CheckHitKey(KEY_INPUT_D) != 0;
					keyget[13] = CheckHitKey(KEY_INPUT_LEFT) != 0;
					keyget[14] = CheckHitKey(KEY_INPUT_RIGHT) != 0;
					keyget[15] = CheckHitKey(KEY_INPUT_UP) != 0;
					keyget[16] = CheckHitKey(KEY_INPUT_DOWN) != 0;
					keyget[17] = CheckHitKey(KEY_INPUT_SPACE) != 0;
					keyget[18] = CheckHitKey(KEY_INPUT_B) != 0;
				}
				else { for (i = 1; i < 19; ++i) { keyget[i] = false; } }

				if (keyget[1]) { ++map.cnt; if (map.cnt == 1) { map.flug ^= 1; SetCursorPos(x_r(960), y_r(540)); } }
				else { map.cnt = 0; }			/*指揮*/
				if (keyget[2]) { ++aim.cnt; if (aim.cnt == 1) { aim.flug ^= 1; if (aim.flug) { ratio = 3.0f; } map.flug = false; } }
				else { aim.cnt = 0; }	/*照準*/
				if (player[0].HP[0] == 0) { aim.flug = false; map.flug = false; }
				if (keyget[7]) { ++selfammo; if (selfammo == 1) { ++player[0].ammotype; player[0].ammotype %= 3; player[0].loadcnt[0] = 1; } }
				else { if (player[0].loadcnt[0] == 0) { selfammo = 0; } }											/*弾種変更*/
				if (map.flug) {
					GetMousePoint(&mousex, &mousey); SetMouseDispFlag(TRUE);
					choose = (std::numeric_limits<size_t>::max)();
					for (size_t p_cnt = 1; p_cnt < teamc; ++p_cnt) {
						if (player[p_cnt].HP[0] > 0) { if (inm(x_r(132), y_r(162 + p_cnt * 24), x_r(324), y_r(180 + p_cnt * 24))) { choose = p_cnt; if (keyget[0]) { waysel = p_cnt; } } }
					}
					if (player[waysel].HP[0] > 0) {
						if (player[waysel].wayselect <= waypc - 1) {
							if (inm(x_r(420), y_r(0), x_r(1500), y_r(1080))) {
								if (keyget[0]) {
									way = std::min<std::uint8_t>(way + 1, 2);
									if (way == 1) {
										if (player[waysel].wayselect == 0) { player[waysel].waynow = 0; }
										player[waysel].waypos[player[waysel].wayselect] = VGet(_2x(mousex), 0, _2y(mousey));
										for (i = player[waysel].wayselect; i < waypc; ++i) { player[waysel].waypos[i] = player[waysel].waypos[player[waysel].wayselect]; }
										++player[waysel].wayselect;
									}
								}
								else { way = 0; }
							}
						}
					}
				}/*視界見回し*/
				else {
					if (aim.flug) {
						SetMousePoint(x_r(960), y_r(540)); GetMouseWheelRotVol();
						if (keyget[3]) { ratio += 2.0f / fps; if (ratio >= 10.0f) { ratio = 10.0f; } }
						if (keyget[4]) { ratio -= 2.0f / fps; if (ratio <= 2.0f) { ratio = 2.0f; } }
						if (keyget[5]) { aim_r += 10.0f; }
						if (keyget[6]) { aim_r -= 10.0f; }
					}
					else { parts.set_view_r(); ratio = 1.0f; }
				}
				differential(rat_r, ratio, 0.1f);														/*倍率、測距*/
			}
			else {
				SetMouseDispFlag(TRUE);
			}
			if (true) {
				/*操作、座標系*/
				for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
					if (!map.flug) { player[p_cnt].wayselect = 0; }
					if (player[p_cnt].HP[0] > 0) {
						player[p_cnt].move = 0;
						if (p_cnt == 0) {
							/*操作*/
							if (keyget[9]) { player[p_cnt].move |= KEY_GOFLONT; }
							if (keyget[10]) { player[p_cnt].move |= KEY_GOBACK_; }
							if (keyget[11]) { player[p_cnt].move |= KEY_GOLEFT_; }
							if (keyget[12]) { player[p_cnt].move |= KEY_GORIGHT; }
							if (keyget[13]) { player[p_cnt].move |= KEY_TURNLFT; }
							if (keyget[14]) { player[p_cnt].move |= KEY_TURNRIT; }
							if (keyget[15]) { player[p_cnt].move |= KEY_TURNUP_; }
							if (keyget[16]) { player[p_cnt].move |= KEY_TURNDWN; }
							if (keyget[17]) { player[p_cnt].move |= KEY_SHOTCAN; }
							if (keyget[18]) { player[p_cnt].move |= KEY_SHOTGAN; }
							/*変速*/
							if (set_shift(&player[p_cnt])) { parts.play_sound(0); }
						}
						else {
							//*CPU操作
							//*移動
							if (VSize(VSub(player[p_cnt].pos, player[p_cnt].waypos[player[p_cnt].waynow])) >= 10.0) {
								player[p_cnt].move |= KEY_GOFLONT;
								tempvec[0] = VNorm(VSub(player[p_cnt].waypos[player[p_cnt].waynow], player[p_cnt].pos));
								cpu_move = -cos(player[p_cnt].yrad) * tempvec[0].x + sin(player[p_cnt].yrad) * tempvec[0].z;
								if (cpu_move < 0) { player[p_cnt].move |= KEY_GOLEFT_; }
								if (cpu_move > 0) { player[p_cnt].move |= KEY_GORIGHT; }
							}
							else { ++player[p_cnt].waynow; if (player[p_cnt].waynow >= waypc - 1) { player[p_cnt].waynow = waypc - 1; } }
							/*
							for (i = player[p_cnt].waynow; i < waypc; i++) { if (VSize(VSub(player[p_cnt].pos, player[p_cnt].waypos[i])) < VSize(VSub(player[p_cnt].pos, player[p_cnt].waypos[player[p_cnt].waynow]))) { player[p_cnt].waynow = i; break; } }
							*/
							//*戦闘
							if (!player[p_cnt].atkf) {
								for (size_t tgt_p = 0; tgt_p < playerc; ++tgt_p) {
									if (player[p_cnt].type != player[tgt_p].type) {
										if (VSize(VSub(player[tgt_p].pos, player[p_cnt].pos)) <= 500.0f && player[tgt_p].HP[0] > 0) {
											if (player[p_cnt].aim != player[p_cnt].atkf) {
												player[p_cnt].aim = 0;
												player[p_cnt].atkf = tgt_p;
												break;
											}
										}
									}
								}
								player[p_cnt].gear = player[p_cnt].wayspd[player[p_cnt].waynow];											//*変速
							}
							else {
								player[p_cnt].gear = 1;																	//*変速
								tempvec[1] = MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun1);												//*元のベクトル
								tempvec[0] = VNorm(VSub(MV1GetFramePosition(player[player[p_cnt].atkf.value()].obj.get(), bone_gun1), tempvec[1]));							//*向くベクトル
								tmpf = VSize(VSub(MV1GetFramePosition(player[player[p_cnt].atkf.value()].obj.get(), bone_gun1), tempvec[1]));
								getdist(&tempvec[1], VNorm(VSub(MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun2), tempvec[1])), &tmpf, player[p_cnt].ptr->gun_speed[player[p_cnt].ammotype], f_rates);

								tempvec[1] = VNorm(VSub(tempvec[1], MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun1)));
								cpu_move = tempvec[1].y * sqrtf(powf(tempvec[0].x, 2) + powf(tempvec[0].z, 2)) - sqrtf(powf(tempvec[1].x, 2) + powf(tempvec[1].z, 2)) * tempvec[0].y;	//*砲
								if (cpu_move <= 0) { player[p_cnt].move |= KEY_TURNUP_; }
								if (cpu_move > 0) { player[p_cnt].move |= KEY_TURNDWN; }
								cpu_move = tempvec[1].z * tempvec[0].x - tempvec[1].x * tempvec[0].z;											//*車体
								if (cpu_move < 0) { player[p_cnt].move |= KEY_TURNLFT; }
								if (cpu_move > 0) { player[p_cnt].move |= KEY_TURNRIT; }
								if (cpu_move < 0) { player[p_cnt].move |= KEY_GOLEFT_; }
								if (cpu_move > 0) { player[p_cnt].move |= KEY_GORIGHT; }
								if (VSize(VCross(tempvec[1], tempvec[0])) < sin(deg2rad(1))) {
									HitPoly = MV1CollCheck_Line(mapparts.get_map_handle().get(), 0, MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun1), MV1GetFramePosition(player[player[p_cnt].atkf.value()].obj.get(), bone_gun1));
									if (!HitPoly.HitFlag) {
										if (player[p_cnt].loadcnt[0] == 0) {
											if ((player[p_cnt].move & KEY_GOFLONT) != 0) { player[p_cnt].move -= KEY_GOFLONT; }
											player[p_cnt].gear = 0;															//*変速
											if (player[p_cnt].speed < 5.f / 3.6f / f_rates) {
												player[p_cnt].move |= KEY_SHOTCAN;
												player[p_cnt].aim++;
											}
										}
										if (GetRand(100) <= 2) { player[p_cnt].move |= KEY_SHOTGAN; }
									}
								}
								if (player[player[p_cnt].atkf.value()].HP[0] == 0 || player[p_cnt].aim > 5) { player[p_cnt].aim = int(player[p_cnt].atkf.value()); player[p_cnt].atkf = std::nullopt; }
							}
							//ぶつかり防止
							for (tgt_p = 0; tgt_p < playerc; ++tgt_p) {
								if (p_cnt != tgt_p) {
									if (VSize(VSub(player[tgt_p].pos, player[p_cnt].pos)) <= 10.0 && player[tgt_p].HP[0] > 0) {
										tempvec[0] = VNorm(VSub(player[tgt_p].pos, player[p_cnt].pos));
										cpu_move = -cos(player[p_cnt].yrad) * tempvec[0].x + sin(player[p_cnt].yrad) * tempvec[0].z;
										if (cpu_move > 0) { player[p_cnt].move |= KEY_GOLEFT_; if ((player[p_cnt].move & KEY_GORIGHT) != 0) { player[p_cnt].move -= KEY_GORIGHT; } }
										if (cpu_move < 0) { player[p_cnt].move |= KEY_GORIGHT; if ((player[p_cnt].move & KEY_GOLEFT_) != 0) { player[p_cnt].move -= KEY_GOLEFT_; } }
									}
								}
								/*
								if (player[p_cnt].state == CPU_NOMAL) {
									if (VSize(VSub(player[tgt_p].pos, player[p_cnt].pos)) <= 250.0 &&  p_cnt != tgt_p && player[p_cnt].type != player[tgt_p].type &&  player[p_cnt].waynow != waypc - 1) {
										if (player[p_cnt].waynow != 0) {
											player[p_cnt].waynow = waypc - 1;
											for (j = 0; j < waypc; ++j) {
												player[p_cnt].waypos[j] = player[p_cnt].pos;
											}
											player[waysel].wayselect = 0;
										}
										else {
											if (VSize(VSub(player[p_cnt].waypos[player[p_cnt].waynow], player[tgt_p].pos)) > 225.f) {
												player[p_cnt].waynow = waypc - 1;
												player[p_cnt].waypos[player[p_cnt].waynow] = player[p_cnt].pos;
												player[waysel].wayselect = 0;
											}
										}
									}
								}
								*/
							}
							//*CPU操作exit
						}
					}
					else { player[p_cnt].move = KEY_TURNUP_; }
				}
				//0.2ms~5ms
				/*共通動作*/
				for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
					if (p_cnt == 0) {
						set_gunrad(&player[0], rat_r*((keyget[8]) ? 3.f : 1.f));//左CTRLを押すと精密エイム
					}
					else {
						set_gunrad(&player[p_cnt], 1.f);
					}
					player[p_cnt].bodyDef.awake = true;
					player[p_cnt].yrad = player[p_cnt].body->GetAngle();
					//
					if ((player[p_cnt].move & KEY_GOFLONT) != 0) {
						if (player[p_cnt].HP[5] == 0 || player[p_cnt].HP[6] == 0) { player[p_cnt].move -= KEY_GOFLONT; }
						else {
							if (player[p_cnt].gear > 0) {
								if (player[p_cnt].flont >= player[p_cnt].ptr->spdflont[player[p_cnt].gear - 1]) { player[p_cnt].flont -= player[p_cnt].ptr->spdflont[player[p_cnt].gear - 1] / (5.0f * fps); }
								else { player[p_cnt].flont += player[p_cnt].ptr->spdflont[player[p_cnt].gear - 1] / (10.0f * fps); }
							}
						}
					}
					if ((player[p_cnt].move & KEY_GOBACK_) != 0) {
						if (player[p_cnt].HP[5] == 0 || player[p_cnt].HP[6] == 0) { player[p_cnt].move -= KEY_GOBACK_; }
						else {
							if (player[p_cnt].gear < 0) {
								if (player[p_cnt].back <= player[p_cnt].ptr->spdback[-player[p_cnt].gear - 1]) { player[p_cnt].back -= player[p_cnt].ptr->spdback[-player[p_cnt].gear - 1] / (2.0f * fps); }
								else { player[p_cnt].back += player[p_cnt].ptr->spdback[-player[p_cnt].gear - 1] / (5.0f * fps); }
							}
						}
					}
					if (player[p_cnt].HP[5] > 0 || player[p_cnt].HP[6] > 0) {
						turn_bias = 1.0f;
						if (player[p_cnt].HP[5] > 0 && player[p_cnt].HP[6] > 0) {
							if ((player[p_cnt].move & KEY_GOFLONT) != 0 && player[p_cnt].gear > 0) {
								turn_bias = player[p_cnt].flont / (player[p_cnt].ptr->spdflont[player[p_cnt].gear - 1]);
							}
							if ((player[p_cnt].move & KEY_GOBACK_) != 0 && player[p_cnt].gear < 0) {
								turn_bias = player[p_cnt].back / (player[p_cnt].ptr->spdback[-player[p_cnt].gear - 1]);
							}
							turn_bias = abs(turn_bias);
						}
						turn_bias *= ((player[p_cnt].HP[5] > 0) + (player[p_cnt].HP[6] > 0)) / 2.0f;
					}
					if ((player[p_cnt].move & KEY_GOLEFT_) != 0) {
						if (player[p_cnt].HP[5] == 0 && player[p_cnt].HP[6] == 0) {
							player[p_cnt].move -= KEY_GOLEFT_;
						}
						else {
							differential(player[p_cnt].yadd, player[p_cnt].ptr->vehicle_RD * turn_bias, 0.1f);
						}
					}
					if ((player[p_cnt].move & KEY_GORIGHT) != 0) {
						if (player[p_cnt].HP[5] == 0 && player[p_cnt].HP[6] == 0) {
							player[p_cnt].move -= KEY_GORIGHT;
						}
						else {
							differential(player[p_cnt].yadd, -player[p_cnt].ptr->vehicle_RD * turn_bias, 0.1f);
						}
					}

					if (player[p_cnt].gear != 0) { player[p_cnt].speed = player[p_cnt].flont + player[p_cnt].back; }


					differential(player[p_cnt].inertia.x, (player[p_cnt].speed - player[p_cnt].speedrec), 0.02f); player[p_cnt].speedrec = player[p_cnt].speed;
					differential(player[p_cnt].inertia.z, -(player[p_cnt].znor - player[p_cnt].znorrec) / 2.0f, 0.1f); player[p_cnt].znorrec = player[p_cnt].znor;

					//vec
					player[p_cnt].vec.x = player[p_cnt].speed * sin(player[p_cnt].yrad);
					player[p_cnt].vec.z = -player[p_cnt].speed * cos(player[p_cnt].yrad);
					if (player[p_cnt].HP[5] == 0) {
						player[p_cnt].vec.x = player[p_cnt].ptr->loc[bone_wheel].x * sin(player[p_cnt].yadd) * sin(player[p_cnt].yrad);
						player[p_cnt].vec.z = -player[p_cnt].ptr->loc[bone_wheel].x * sin(player[p_cnt].yadd) * cos(player[p_cnt].yrad);
					}
					if (player[p_cnt].HP[6] == 0) {
						player[p_cnt].vec.x = -player[p_cnt].ptr->loc[bone_wheel].x * sin(player[p_cnt].yadd) * sin(player[p_cnt].yrad);
						player[p_cnt].vec.z = player[p_cnt].ptr->loc[bone_wheel].x * sin(player[p_cnt].yadd) * cos(player[p_cnt].yrad);
					}
					//
					player[p_cnt].wheelrad[0] += player[p_cnt].speed / f_rates;//
					player[p_cnt].wheelrad[1] = -player[p_cnt].wheelrad[0] * 2 + player[p_cnt].yrad * 5;
					player[p_cnt].wheelrad[2] = -player[p_cnt].wheelrad[0] * 2 - player[p_cnt].yrad * 5;
					//
					player[p_cnt].body->SetLinearVelocity(b2Vec2(player[p_cnt].vec.x, player[p_cnt].vec.z));
					player[p_cnt].body->SetAngularVelocity(player[p_cnt].yadd);
				}//0.1ms
				//0.0ms
				/*物理演算*/
				world->Step(1.0f / f_rates, 1, 1);						// シミュレーションの単一ステップを実行するように世界に指示します。 一般に、タイムステップと反復を固定しておくのが最善です。
				for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
					player[p_cnt].pos.x = player[p_cnt].body->GetPosition().x;
					player[p_cnt].pos.z = player[p_cnt].body->GetPosition().y;
					player[p_cnt].yrad = -player[p_cnt].body->GetAngle();
				}//0ms
				//0.0ms
				/*砲撃その他*/
				for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
					//地形判定
					HitPoly = MV1CollCheck_Line(mapparts.get_map_handle().get(), 0, VAdd(player[p_cnt].pos, VGet(0.0f, 2.0f, 0.0f)), VAdd(player[p_cnt].pos, VGet(0.0f, -0.05f, 0.0f)));//0.3ms
					if (HitPoly.HitFlag) {
						player[p_cnt].pos.y = HitPoly.HitPosition.y;
						player[p_cnt].yace = 0.0f;
						set_normal(&(player[p_cnt].xnor), &(player[p_cnt].znor), mapparts.get_map_handle().get(), player[p_cnt].pos);//0.6ms
						player[p_cnt].nor = VTransform(VGet(0, 1.f, 0), MMult(MGetRotX(player[p_cnt].xnor), MGetRotZ(player[p_cnt].znor)));
						//player[p_cnt].nor = VAdd(player[p_cnt].nor, VScale(VSub(HitPoly.Normal, player[p_cnt].nor), 0.1f));
						/*speed*/
						if ((player[p_cnt].move & KEY_GOLEFT_) == 0 && (player[p_cnt].move & KEY_GORIGHT) == 0) { player[p_cnt].yadd *= 0.9f; }
						if (player[p_cnt].gear > 0 || (player[p_cnt].move & KEY_GOBACK_) == 0) { player[p_cnt].back *= 0.95f; }
						if (player[p_cnt].gear == 0) { player[p_cnt].speed *= 0.95f; }
						if (player[p_cnt].gear < 0 || (player[p_cnt].move & KEY_GOFLONT) == 0) { player[p_cnt].flont *= 0.95f; }
						/*track*/
						mapparts.draw_map_track(&player[p_cnt]);//0.1ms
					}
					else {
						player[p_cnt].pos.y += player[p_cnt].yace; player[p_cnt].yace += M_GR / 2.0f / fps / fps;
					}
					//地形判定全体=3.8ms
					//サウンド
					if (abs(player[p_cnt].speed) >= player[p_cnt].ptr->spdflont[0]) { ChangeVolumeSoundMem(64, player[p_cnt].se[0].get()); }
					else { ChangeVolumeSoundMem((int)(64.f*abs(player[p_cnt].speed) / player[p_cnt].ptr->spdflont[0]), player[p_cnt].se[0].get()); }//0.1ms
					for (i = 27; i < 29; ++i) { ChangeVolumeSoundMem((int)(32.f + 32.f*abs(player[p_cnt].speed / player[p_cnt].ptr->spdflont[3])), player[p_cnt].se[i].get()); }//0.1ms
					for (i = 0; i < 31; ++i) { if (CheckSoundMem(player[p_cnt].se[i].get()) == 1) { Set3DPositionSoundMem(player[p_cnt].pos, player[p_cnt].se[i].get()); } }//1.5ms
					//サウンド全体=1.7ms
					//tree判定
					mapparts.set_hitplayer(player[p_cnt].pos);
					//0.1ms
					/*車体行列*/
					player[p_cnt].ps_m = MMult(MMult(MMult(MMult(MGetRotAxis(VGet(cos(player[p_cnt].gunrad.x), 0, -sin(player[p_cnt].gunrad.x)), sin(deg2rad(player[p_cnt].firerad)) * deg2rad(5)), MGetRotAxis(VGet(cos(player[p_cnt].recorad), 0, -sin(player[p_cnt].recorad)), sin(deg2rad(player[p_cnt].recoall)) * deg2rad(5))), MGetRotX(atan(player[p_cnt].inertia.x))), MMult(MGetRotY(player[p_cnt].yrad), MGetRotVec2(VGet(0, 1.f, 0), player[p_cnt].nor))), MGetTranslate(player[p_cnt].pos));//MMult(MGetRotX(player[p_cnt].xnor), MGetRotZ(player[p_cnt].znor))
					/*砲塔行列*/
					player[p_cnt].ps_t = MMult(MGetRotY(player[p_cnt].gunrad.x), MGetTranslate(player[p_cnt].ptr->loc[bone_trt]));
					//all
					MV1SetMatrix(player[p_cnt].colobj.get(), player[p_cnt].ps_m);
					MV1SetMatrix(player[p_cnt].obj.get(), player[p_cnt].ps_m);
					//common
					for (i = bone_trt; i < player[p_cnt].ptr->frames; ++i) {
						if (i == bone_trt) {
							MV1SetFrameUserLocalMatrix(player[p_cnt].obj.get(), i, player[p_cnt].ps_t);
							MV1SetFrameUserLocalMatrix(player[p_cnt].colobj.get(), i, player[p_cnt].ps_t);
						}
						else if (i == bone_gun1) {
							const auto mtemp = MMult(MMult(MGetRotX(player[p_cnt].gunrad.y), MGetTranslate(VSub(player[p_cnt].ptr->loc[i], player[p_cnt].ptr->loc[bone_trt]))), player[p_cnt].ps_t);
							MV1SetFrameUserLocalMatrix(player[p_cnt].obj.get(), i, mtemp);
							MV1SetFrameUserLocalMatrix(player[p_cnt].colobj.get(), i, mtemp);
						}
						else if (i == bone_gun2) {
							const auto mtemp = MGetTranslate(VAdd(VSub(player[p_cnt].ptr->loc[i], player[p_cnt].ptr->loc[bone_gun1]), VGet(0, 0, player[p_cnt].fired)));
							MV1SetFrameUserLocalMatrix(player[p_cnt].obj.get(), i, mtemp);
							MV1SetFrameUserLocalMatrix(player[p_cnt].colobj.get(), i, mtemp);
						}
						else if (i == bone_gun) {
							MV1SetFrameUserLocalMatrix(player[p_cnt].obj.get(), i, MMult(MMult(MGetRotX(player[p_cnt].gunrad.y), MGetTranslate(VSub(player[p_cnt].ptr->loc[i], player[p_cnt].ptr->loc[bone_trt]))), player[p_cnt].ps_t));
						}
						else if (i == bone_gun_) {
							MV1SetFrameUserLocalMatrix(player[p_cnt].obj.get(), i, MGetTranslate(VSub(player[p_cnt].ptr->loc[i], player[p_cnt].ptr->loc[bone_gun])));
						}
						else if (i >= bone_wheel && i < player[p_cnt].ptr->frames - 4) {
							if ((i - bone_wheel) % 2 == 1) {
								MV1SetFrameUserLocalMatrix(
									player[p_cnt].obj.get(), i, 
									MMult(
										MGetRotX(player[p_cnt].wheelrad[signbit(player[p_cnt].ptr->loc[i].x)+1]),
										MGetTranslate(VSub(player[p_cnt].ptr->loc[i], player[p_cnt].ptr->loc[size_t(i) - 1]))
									)
								);
							}
							else {
								MV1ResetFrameUserLocalMatrix(player[p_cnt].obj.get(), i);
								tempvec[1] = VAdd(MV1GetFramePosition(player[p_cnt].obj.get(), i), VScale(player[p_cnt].nor, 1.0f));
								tempvec[0] = VAdd(MV1GetFramePosition(player[p_cnt].obj.get(), i), VScale(player[p_cnt].nor, -0.2f));
								HitPoly = MV1CollCheck_Line(mapparts.get_map_handle().get(), 0, tempvec[1], tempvec[0]);
								if (HitPoly.HitFlag) {
									tmpf = 1.0f - VSize(VSub(HitPoly.HitPosition, tempvec[1]));
									player[p_cnt].Springs[i] += 1.0f / fps; if (player[p_cnt].Springs[i] >= tmpf) { player[p_cnt].Springs[i] = tmpf; }
								}
								else {
									if (player[p_cnt].Springs[i] > -0.2f) { player[p_cnt].Springs[i] += -0.2f / fps; }
								}
								MV1SetFrameUserLocalMatrix(player[p_cnt].obj.get(), i, MGetTranslate(VAdd(player[p_cnt].ptr->loc[i], VScale(player[p_cnt].nor, player[p_cnt].Springs[i]))));
								//2ms
							}
						}
						else if (i >= player[p_cnt].ptr->frames - 4) {
							MV1SetFrameUserLocalMatrix(player[p_cnt].obj.get(), i, MMult(MGetRotX(player[p_cnt].wheelrad[signbit(player[p_cnt].ptr->loc[i].x)+1]), MGetTranslate(player[p_cnt].ptr->loc[i])));
						}
					}
					/*collition*/
					for (i = 0; i < player[p_cnt].ptr->colmeshes; ++i) { MV1RefreshCollInfo(player[p_cnt].colobj.get(), -1); }
					/**/
					if (player[p_cnt].fired >= 0.01f) { player[p_cnt].fired *= 0.95f; }
					if (player[p_cnt].loadcnt[0] > 0) {
						if (player[p_cnt].firerad < 180) { if (player[p_cnt].firerad <= 90) { player[p_cnt].firerad += 900 / (int)fps; } else { player[p_cnt].firerad += 180 / (int)fps; } }
						else { player[p_cnt].firerad = 180; }
					}
					for (size_t guns = 0; guns < gunc; ++guns) {
						size_t k = guns * ammoc;
						if (player[p_cnt].loadcnt[guns] == 0) {
							if ((player[p_cnt].move & (KEY_SHOTCAN << guns)) != 0) {
								const auto j = player[p_cnt].useammo[guns] + k;
								player[p_cnt].Ammo[j].flug = 1;
								player[p_cnt].Ammo[j].speed = player[p_cnt].ptr->gun_speed[player[p_cnt].ammotype] / fps;
								player[p_cnt].Ammo[j].pene = player[p_cnt].ptr->pene[player[p_cnt].ammotype];
								if (guns == 0) { player[p_cnt].Ammo[j].pos = MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun1); }
								else { player[p_cnt].Ammo[j].pos = MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun); }
								tempvec[0] = VSub(MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun2), MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun1));

								player[p_cnt].Ammo[j].rad.y = atan2(tempvec[0].x, (tempvec[0].z));
								player[p_cnt].Ammo[j].rad.x = atan2(-tempvec[0].y, sqrt(pow(tempvec[0].x, 2) + pow(tempvec[0].z, 2)));

								tempfx = player[p_cnt].Ammo[j].rad.x - deg2rad((float)(-1000 + GetRand(2000)) / 10000.f);
								tempfy = player[p_cnt].Ammo[j].rad.y + deg2rad((float)(-1000 + GetRand(2000)) / 10000.f);
								player[p_cnt].Ammo[j].vec = VGet(cos(tempfx) * sin(tempfy), -sin(tempfx), cos(tempfx) * cos(tempfy));
								player[p_cnt].Ammo[j].repos = player[p_cnt].Ammo[j].pos;
								player[p_cnt].Ammo[j].cnt = 0;
								//					       
								player[p_cnt].useammo[guns] = (std::min)(player[p_cnt].useammo[guns] + 1, ammoc - 1);
								//繰り返す必要があるのですが余剰でよろしいでしょうか
								/*
								player[p_cnt].useammo[guns] ++;
								player[p_cnt].useammo[guns] %= ammoc;
								*/

								++player[p_cnt].loadcnt[guns];
								if (guns == 0) {
									set_effect(&(player[p_cnt].effcs[ef_fire]), MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun2), VSub(MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun2), MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun1)));
									player[p_cnt].fired = 0.5f;
									player[p_cnt].firerad = 0;
									if (p_cnt == 0) { humanparts.start_humananime(2); parts.play_sound(1 + GetRand(6)); }
									PlaySoundMem(player[p_cnt].se[size_t(2) + GetRand(7)].get(), DX_PLAYTYPE_BACK, TRUE);
								}
								else {
									set_effect(&(player[p_cnt].effcs[ef_gun]), MV1GetFramePosition(player[p_cnt].obj.get(), bone_gun_), VGet(0, 0, 0));
									PlaySoundMem(player[p_cnt].se[1].get(), DX_PLAYTYPE_BACK, TRUE);
								}
							}
						}
						else {
							++player[p_cnt].loadcnt[guns];
							if (player[p_cnt].loadcnt[guns] >= player[p_cnt].ptr->reloadtime[guns]) { player[p_cnt].loadcnt[guns] = 0; if (p_cnt == 0 && guns == 0) { parts.play_sound(8 + GetRand(4)); } }//装てん完了
						}
						for (size_t i = k; i < ammoc + k; ++i) {
							if (player[p_cnt].Ammo[i].flug != 0) {
								player[p_cnt].Ammo[i].repos = player[p_cnt].Ammo[i].pos;
								player[p_cnt].Ammo[i].pos = VAdd(player[p_cnt].Ammo[i].pos, VScale(player[p_cnt].Ammo[i].vec, player[p_cnt].Ammo[i].speed));
								HitPoly = MV1CollCheck_Line(mapparts.get_map_handle().get(), 0, player[p_cnt].Ammo[i].repos, player[p_cnt].Ammo[i].pos);
								if (HitPoly.HitFlag) { player[p_cnt].Ammo[i].pos = HitPoly.HitPosition; }
								for (tgt_p = 0; tgt_p < playerc; ++tgt_p) {
									if (p_cnt == tgt_p) { continue; }
									btmp = get_reco(&(player[p_cnt]), &(player[tgt_p]), i, guns);
									if (btmp) { break; }
								}
								if (!btmp) {
									if (HitPoly.HitFlag) {
										set_effect(&(player[p_cnt].effcs[ef_gndhit + guns * (ef_gndhit2 - ef_gndhit)]), HitPoly.HitPosition, HitPoly.Normal);
										player[p_cnt].Ammo[i].vec = VAdd(player[p_cnt].Ammo[i].vec, VScale(HitPoly.Normal, VDot(player[p_cnt].Ammo[i].vec, HitPoly.Normal) * -2.0f));
										player[p_cnt].Ammo[i].pos = VAdd(HitPoly.HitPosition, VScale(player[p_cnt].Ammo[i].vec, 0.01f));
										//player[p_cnt].Ammo[i].pene /= 2.0f;
										player[p_cnt].Ammo[i].speed /= 2.f;
									}
								}

								tempvec[0] = VSub(player[p_cnt].Ammo[i].pos, player[p_cnt].Ammo[i].repos);
								player[p_cnt].Ammo[i].rad.y = atan2(tempvec[0].x, tempvec[0].z);
								player[p_cnt].Ammo[i].rad.x = atan2(-tempvec[0].y, sqrt(pow(tempvec[0].x, 2) + pow(tempvec[0].z, 2)));
								player[p_cnt].Ammo[i].vec.y += M_GR / 2.0f / fps / fps;
								player[p_cnt].Ammo[i].pene -= 1.0f / fps;
								player[p_cnt].Ammo[i].speed -= 5.f / fps;
								player[p_cnt].Ammo[i].cnt++;
								if (player[p_cnt].Ammo[i].cnt > (fps*3.f) || player[p_cnt].Ammo[i].speed <= 0.f) { player[p_cnt].Ammo[i].flug = 0; }//3秒で消える
							}
						}
					}
					if (player[p_cnt].recoadd) {
						if (player[p_cnt].recoall < 180) {
							if (p_cnt == 0 && player[p_cnt].recoall == 0) { uiparts->set_reco(); }
							if (player[p_cnt].recoall <= 90) { player[p_cnt].recoall += 900 / (int)fps; }
							else { player[p_cnt].recoall += 180 / (int)fps; }
						}
						else { player[p_cnt].recoall = 0; player[p_cnt].recoadd = false; }
					}
					if (player[p_cnt].hitadd) {
						if (p_cnt == 0) { humanparts.start_humanvoice(0); }
						player[p_cnt].hitadd = false;
					}
				}
				//5.3ms
				/*轍更新*/
				mapparts.set_map_track();
				//0ms
				/*human*/
				humanparts.set_humanmove(player[0], parts.get_view_r(), fps);
				//(usegrab=TRUE)0.9~1.0ms(FALSE)0.1ms
				/*effect*/
				for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
					for (i = 0; i < efs_user; ++i) {
						if (i != ef_smoke2 && i != ef_smoke3) { set_pos_effect(&player[p_cnt].effcs[i], parts.get_effHandle(i)); }
					}
						tempvec[0] = MV1GetFramePosition(player[p_cnt].obj.get(), bone_smoke1);
						player[p_cnt].effcs[ef_smoke2].efhandle.SetPos(tempvec[0].x, tempvec[0].y, tempvec[0].z);
						//SetTargetLocation(player[p_cnt].effcs[ef_smoke2].efhandle, tempvec[0].x, tempvec[0].y, tempvec[0].z);
						tempvec[0] = MV1GetFramePosition(player[p_cnt].obj.get(), bone_smoke2);
						player[p_cnt].effcs[ef_smoke3].efhandle.SetPos(tempvec[0].x, tempvec[0].y, tempvec[0].z);
						//SetTargetLocation(player[p_cnt].effcs[ef_smoke3].efhandle, tempvec[0].x, tempvec[0].y, tempvec[0].z);
				}
				UpdateEffekseer3D();
				//0.6ms
			}
			/*視点*/
			if (aim.flug) {
				campos = MV1GetFramePosition(player[0].obj.get(), bone_gun1);
				viewpos = MV1GetFramePosition(player[0].obj.get(), bone_gun2);
				uppos = player[0].nor;
			}
			else {
				if (parts.get_view_r().z != 0.1f) {
					campos = VAdd(player[0].pos, VAdd(parts.get_view_pos(), VGet(0, 2, 0)));
					viewpos = VAdd(player[0].pos, VGet(0, 4, 0));
					HitPoly = MV1CollCheck_Line(mapparts.get_map_handle().get(), 0, campos, viewpos); if (HitPoly.HitFlag) { campos = HitPoly.HitPosition; }
					campos = VAdd(campos, VGet(0, 2, 0));
					uppos = VGet(0, 1, 0);
				}
				else {
					campos = humanparts.get_campos();
					viewpos = humanparts.get_neckpos();
					uppos = player[0].nor;
				}
			}
			mapparts.set_camerapos(campos, viewpos, uppos, rat_r);
			/*shadow*/
			mapparts.set_map_shadow_near(parts.get_view_r().z);
			/*draw*/
			/*map*/
			if (map.flug) {
				SetDrawScreen(minimap);
				ClearDrawScreen();
				DrawExtendGraph(x_r(420), y_r(0), x_r(1500), y_r(1080), mapparts.get_minmap(), FALSE);
				for (size_t p_cnt = 0; p_cnt < teamc; ++p_cnt) {
					DrawCircle(x_(player[p_cnt].pos.x), y_(player[p_cnt].pos.z), 5, (player[p_cnt].HP[0] == 0) ? c_008000 : c_00ff00, 1);
				}
				for (size_t p_cnt = teamc; p_cnt < playerc; ++p_cnt) {
					DrawCircle(x_(player[p_cnt].pos.x), y_(player[p_cnt].pos.z), 5, (player[p_cnt].HP[0] == 0) ? c_800000 : c_ff0000, 1);
				}
				//teamc + enemyc
				for (size_t p_cnt = 1; p_cnt < teamc; ++p_cnt) {
					DrawLine(x_(player[p_cnt].pos.x), y_(player[p_cnt].pos.z), x_(player[p_cnt].waypos[player[p_cnt].waynow].x), y_(player[p_cnt].waypos[player[p_cnt].waynow].z), c_ff0000, 3);
					for (j = player[p_cnt].waynow; j < waypc - 1; ++j) {
						DrawLine(x_(player[p_cnt].waypos[j].x), y_(player[p_cnt].waypos[j].z), x_(player[p_cnt].waypos[j + 1].x), y_(player[p_cnt].waypos[j + 1].z), GetColor(255, 64 * j, 0), 3);
					}
				}
				for (size_t p_cnt = 0; p_cnt < teamc; ++p_cnt) {
					//ステータス
					const auto c = (p_cnt == waysel)
						? (player[p_cnt].HP[0] == 0) ? c_c8c800 : c_ffff00
						: (p_cnt == choose)
							? c_c0ff00
							: (player[p_cnt].HP[0] == 0) ? c_808080 : c_00ff00;
					DrawBox(x_r(132), y_r(162 + p_cnt * 24), x_r(324), y_r(180 + p_cnt * 24), c, TRUE);
					DrawFormatStringToHandle(x_r(132), y_r(162 + p_cnt * 24), c_ffffff, parts.get_font(0), " %s", player[p_cnt].ptr->name.c_str());
					//進軍
					for (k = 0; k < player[p_cnt].wayselect; k++) { DrawBox(x_r(348 + k * 12), y_r(162 + p_cnt * 24), x_r(356 + k * 12), y_r(180 + p_cnt * 24), c_3232ff, TRUE); }
				}
				for (size_t p_cnt = teamc; p_cnt < playerc; ++p_cnt) {
					DrawBox(x_r(1500), y_r(162 + (p_cnt - teamc) * 24), x_r(1692), y_r(180 + (p_cnt - teamc) * 24), (player[p_cnt].HP[0] == 0) ? c_808080 : c_ff0000, TRUE);
					DrawFormatStringToHandle(x_r(1500), y_r(162 + (p_cnt - teamc) * 24), c_ffffff, parts.get_font(0), " %s", player[p_cnt].ptr->name.c_str());
				}
			}
			//0.1ms
			/*main*/
			else {
				uiparts->end_way();//debug0//0
				uiparts->end_way();//debug1//0
				uiparts->end_way();//debug2//0

				/*sky*/
				if (parts.get_view_r().z != 0.1f || aim.flug) {
					SetDrawScreen(skyscreen);
					mapparts.draw_map_sky();
				}
				/*near*/
				SetDrawScreen(mainscreen);
				ClearDrawScreen();
				if (aim.flug) { setcv(0.06f + rat_r / 2, 2000.0f, campos, viewpos, uppos, 45.0f / rat_r); }
				else { setcv(0.16f + parts.get_view_r().z, 2000.0f, campos, viewpos, uppos, 45.0f / rat_r); }
				//----------------------------------------------------------
				if (aim.flug) {
					tempvec[0] = MV1GetFramePosition(player[0].obj.get(), bone_gun1);
					tmpf = player[0].ptr->gun_speed[player[0].ammotype];
					getdist(&tempvec[0], VNorm(VSub(MV1GetFramePosition(player[0].obj.get(), bone_gun2), MV1GetFramePosition(player[0].obj.get(), bone_gun1))), &aim_r, tmpf, f_rates);
					aims = ConvWorldPosToScreenPos(tempvec[0]);
					aimm = aim_r / 1000.0f * tmpf;
				}
				//pos
				for (size_t p_cnt = 1; p_cnt < playerc; ++p_cnt) { player[p_cnt].iconpos = ConvWorldPosToScreenPos(VAdd(player[p_cnt].pos, VGet(0, VSize(VSub(player[p_cnt].pos, player[0].pos)) / 40 + 6, 0))); }

				for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
					if (CheckCameraViewClip_Box(VAdd(player[p_cnt].pos, VGet(-5, 0, -5)), VAdd(player[p_cnt].pos, VGet(5, 3, 5))) == TRUE) {
						pssort[p_cnt] = pair(p_cnt, (float)map_x);
					}
					else {
						pssort[p_cnt] = pair(p_cnt, VSize(VSub(player[p_cnt].pos, campos)));
					}
				}
				std::sort(pssort.begin(), pssort.end(), [](const pair& x, const pair& y) {return x.second > y.second; });

				//effect
				Effekseer_Sync3DSetting();
				//---------------------------------------------------------------
				if (parts.get_view_r().z != 0.1f || aim.flug) {
					DrawGraph(0, 0, skyscreen, FALSE);//sky
					//main
					ShadowMap_DrawSetup(mapparts.get_map_shadow_seminear());
					for (auto& tt : pssort) {
						if (tt.second == (float)map_x) { continue; }
						if (tt.second < (15.0f * parts.get_view_r().z + 20.0f)) { break; }
						const auto k = tt.first;
						MV1DrawMesh(player[k].obj.get(), 0);
						for (i = 1; i < player[k].ptr->meshes; ++i) {
							if (player[k].HP[i + 4] > 0) { MV1DrawMesh(player[k].obj.get(), i); }
						}
					}
					ShadowMap_DrawEnd();
					ShadowMap_DrawSetup(mapparts.get_map_shadow_near());
					humanparts.draw_human(0);
					for (auto& tt : pssort) {
						if (tt.second > (15.0f * parts.get_view_r().z + 20.0f)) { continue; }
						const auto k = tt.first;
						MV1DrawMesh(player[k].obj.get(), 0);
						for (i = 1; i < player[k].ptr->meshes; ++i) {
							if (player[k].HP[i + 4] > 0) { MV1DrawMesh(player[k].obj.get(), i); }
						}
					}
					ShadowMap_DrawEnd();

					SetUseShadowMap(0, mapparts.get_map_shadow_near());
					SetUseShadowMap(1, mapparts.get_map_shadow_far());
					SetUseShadowMap(2, mapparts.get_map_shadow_seminear());
					mapparts.draw_map_model();
					humanparts.draw_human(0);
					for (auto& tt : pssort) {
						if (tt.second == (float)map_x) { continue; }
						const auto k = tt.first;
						if (k != 0 || (k == 0 && !aim.flug)) {

							MV1DrawMesh(player[k].obj.get(), 0);
							for (i = 1; i < player[k].ptr->meshes; ++i) {
								if (i >= 1 && i < 3) { MV1SetFrameTextureAddressTransform(player[k].obj.get(), 0, 0.0, player[k].wheelrad[i], 1.0, 1.0, 0.5, 0.5, 0.0); }
								if (i == 3) { MV1ResetFrameTextureAddressTransform(player[k].obj.get(), 0); }
								if (player[k].HP[i + 4] > 0) { MV1DrawMesh(player[k].obj.get(), i); }
							}
							for (i = 0; i < 3; ++i) {
								MV1SetRotationZYAxis(player[k].hitpic[i].get(), VSub(MV1GetFramePosition(player[k].colobj.get(), 11 + 3 * i), MV1GetFramePosition(player[k].colobj.get(), 9 + 3 * i)), VSub(MV1GetFramePosition(player[k].colobj.get(), 10 + 3 * i), MV1GetFramePosition(player[k].colobj.get(), 9 + 3 * i)), 0.f);
								MV1SetPosition(player[k].hitpic[i].get(), VAdd(MV1GetFramePosition(player[k].colobj.get(), 9 + 3 * i), VScale(VSub(MV1GetFramePosition(player[k].colobj.get(), 10 + 3 * i), MV1GetFramePosition(player[k].colobj.get(), 9 + 3 * i)), 0.005f)));
								MV1DrawFrame(player[k].hitpic[i].get(), player[k].usepic[i]);
							}
						}
					}
					//grass
					if (!aim.flug) { mapparts.draw_grass(); }
					//effect
					DrawEffekseer3D();
					//ammo
					uiparts->end_way();//debug3//0
					SetUseLighting(FALSE);
					SetFogEnable(FALSE);
					for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
						for (size_t i = 0; i < ammoc; ++i) {
							if (player[p_cnt].Ammo[i].flug != 0) {
								tmpf = 4.f*player[p_cnt].Ammo[i].speed / (player[p_cnt].ptr->gun_speed[player[p_cnt].ammotype] / fps);
								if (tmpf >= 1.f) { tmpf = 1.f; }
								SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f*tmpf));

								DrawCapsule3D(player[p_cnt].Ammo[i].pos, player[p_cnt].Ammo[i].repos, player[p_cnt].ptr->ammosize*(VSize(VSub(player[p_cnt].Ammo[i].pos, campos)) / 65.f), 4, player[p_cnt].Ammo[i].color, c_ffffff, TRUE);
							}
						}
						for (size_t i = ammoc; i < ammoc * gunc; ++i) {
							if (player[p_cnt].Ammo[i].flug != 0) {
								tmpf = 4.f*player[p_cnt].Ammo[i].speed / (player[p_cnt].ptr->gun_speed[player[p_cnt].ammotype] / fps);
								if (tmpf >= 1.f) { tmpf = 1.f; }
								SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f*tmpf));
								DrawCapsule3D(player[p_cnt].Ammo[i].pos, player[p_cnt].Ammo[i].repos, 0.0075f*(VSize(VSub(player[p_cnt].Ammo[i].pos, campos)) / 30.f), 4, player[p_cnt].Ammo[i].color, c_ffffff, TRUE);
							}
						}
					}
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
					SetFogEnable(TRUE);
					SetUseLighting(TRUE);

					//tree
					mapparts.draw_trees();
					DrawEffekseer3D();

					SetUseShadowMap(0, -1);
					SetUseShadowMap(1, -1);
					SetUseShadowMap(2, -1);
					//カッコ内2~4ms
					uiparts->end_way();//debug4//0
				}
				else {
					humanparts.draw_humanall();
				}
				//1.2ms
			}
			//2ms
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();
			if (map.flug) {
				/*指揮*/
				DrawGraph(0, 0, minimap, FALSE);
			}
			else {
				/*通常*/
				DrawGraph(0, 0, mainscreen, FALSE);
				/*ブルーム*/
				GraphFilterBlt(mainscreen, HighBrightScreen, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS, 210, TRUE, c_000000, 255);
				GraphFilterBlt(HighBrightScreen, GaussScreen, DX_GRAPH_FILTER_DOWN_SCALE, EXTEND);
				GraphFilter(GaussScreen, DX_GRAPH_FILTER_GAUSS, 16, 1000);
				SetDrawMode(DX_DRAWMODE_BILINEAR);
				SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
				DrawExtendGraph(0, 0, dispx, dispy, GaussScreen, FALSE);
				DrawExtendGraph(0, 0, dispx, dispy, GaussScreen, FALSE);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
				/*UI*/
				if (aim.flug) { uiparts->draw_sight(aims.x, aims.y, rat_r, aimm, parts.get_font(0)); }/*照準器*/
				else {
					for (auto& tt : pssort) {
						if (tt.second == (float)map_x) { continue; }
						const auto k = tt.first;
						if (player[k].HP[0] != 0) {
							if (player[k].iconpos.z > 0.0f && player[k].iconpos.z < 1.0f) {
								DrawFormatStringToHandle(
									(int)player[k].iconpos.x, (int)player[k].iconpos.y, (player[k].type == TEAM) ? c_00ff00 : c_ff0000, parts.get_font(0),
									"%dm", (int)VSize(VSub(player[k].pos, player[0].pos))
								);
							}
						}
					}
				}/*アイコン*/
				uiparts->draw_ui(selfammo, parts.get_view_r().y);/*main*/
			}
			/*debug*/
			//DrawFormatStringToHandle(x_r(18), y_r(1062), c_ffffff, parts.get_font(0), "start-stop(%.2fms)", (float)stop_w / 1000.f);
			uiparts->debug(fps, (float)(GetNowHiPerformanceCount() - waits) / 1000.0f);

			parts.Screen_Flip(waits);
		}
		//delete
		mapparts.delete_map();
		humanparts.delete_human();
		for (size_t p_cnt = 0; p_cnt < playerc; ++p_cnt) {
			/*エフェクト*/
			for (auto&& e : player[p_cnt].effcs) e.efhandle.Dispose();
			/*Box2D*/
			delete player[p_cnt].playerfix->GetUserData();
			player[p_cnt].playerfix->SetUserData(NULL);
			/**/
			player[p_cnt].obj.Dispose();
			player[p_cnt].colobj.Dispose();
			for (i = 0; i < 50; ++i) { player[p_cnt].se[i].Dispose(); }
			for (i = 0; i < 3; i++) { player[p_cnt].hitpic[i].Dispose(); }
			player[p_cnt].Ammo.clear();
			player[p_cnt].Springs.clear();
			player[p_cnt].HP.clear();
			player[p_cnt].hitssort.clear();
		}
		pssort.clear();
		player.clear();
	} while (!out);
	/*終了*/
	return 0;
}
