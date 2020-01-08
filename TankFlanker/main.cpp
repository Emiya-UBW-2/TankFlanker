#define NOMINMAX
#include "define.h"
#include "useful.h"
#include <algorithm>
#include <memory>

/*main*/
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR , int) {
	//temp----------------------------------------------------------------------------------//
	int mousex, mousey;									/*mouse*/
	float tmpf;
	VECTOR tempvec[2];
	std::array<bool, 19>keyget;								/*キー用*/
	//変数----------------------------------------------------------------------------------//
	bool out{ false };									/*終了フラグ*/
	std::vector<pair> pssort;								/*playerソート*/
	std::vector<players> player;								/*player*/
	VECTOR aims = VGet(0, 0, 0);								/*照準器座標*/
	float aimm=0.f;										/*照準距離*/
	float fps;										/*fps*/
	std::uint8_t selfammo;									/*UI用*/
	switches aim, map;									/*視点変更*/
	float ratio, rat_r, aim_r;								/*照準視点　倍率、実倍率、距離*/
	size_t waysel, choose = (std::numeric_limits<size_t>::max)();				/*指揮視点　指揮車両、マウス選択*/
	std::uint8_t way = 0;									//マウストリガー
	LONGLONG old_time, waits;								/*時間取得*/
	VECTOR campos, viewpos, uppos;								/*カメラ*/
	MV1_COLL_RESULT_POLY HitPoly;								/*あたり判定*/
	float cpu_move;
	char mstr[64];										/*tank*/
	int mdata;										/*tank*/
	//init----------------------------------------------------------------------------------//
	auto parts = std::make_unique<Myclass>();						/*汎用クラス*/
	auto humanparts = std::make_unique<HUMANS>(parts->get_usegrab(), parts->get_f_rate());	/*車内関係*/
	auto mapparts = std::make_unique<MAPS>(parts->get_gndx(), parts->get_drawdist(),parts->get_shadex());	/*地形、ステージ関係*/
	auto uiparts = std::make_unique<UIS>();
	float f_rates = parts->get_f_rate();
	//load----------------------------------------------------------------------------------//
	parts->set_fonts(18);
	SetUseASyncLoadFlag(TRUE);
		//hit---------------------------------------------------------------------------//
		const auto hit_mod = MV1ModelHandle::Load("data/hit/hit.mv1");
		//screen------------------------------------------------------------------------//
		int minimap = MakeScreen(dispx, dispy, FALSE);					/*ミニマップ*/
		int skyscreen = MakeScreen(dispx, dispy, FALSE);				/*空*/
		int mainscreen = MakeScreen(dispx, dispy, FALSE);				/*遠景*/
		int HighBrightScreen = MakeScreen(dispx, dispy, FALSE);				/*エフェクト*/
		int GaussScreen = MakeScreen(dispx / EXTEND, dispy / EXTEND, FALSE);		/*エフェクト*/
	SetUseASyncLoadFlag(FALSE);
	uiparts->draw_load();//
	if (!parts->set_veh()) { return -1; }
	/*物理開始*/
	auto world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f));				/* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
	//これ以降繰り返しロード----------------------------------------------------------------//
	do {
		int m = parts->window_choosev(); if (m == -1) { return 0; }			/*player指定*/

		mdata = FileRead_open("stage/data_0/main.txt", FALSE);				/*ステージ情報*/
		FileRead_gets(mstr, 64, mdata);
			const int mapc = std::stoi(getright(mstr));
		FileRead_close(mdata);

		const size_t teamc = count_team("data_0");					/*味方数*/
		const size_t enemyc = count_enemy("data_0");					/*敵数*/
		const size_t playerc = teamc + enemyc;						/*車両数*/

		player.resize(playerc);
		for (int p_cnt = 0; p_cnt < playerc; ++p_cnt) { player[p_cnt].id = p_cnt; }
		pssort.resize(playerc);
		//設定
		for (size_t p_cnt = 0; p_cnt < teamc; ++p_cnt) {
			mdata = FileRead_open(("stage/data_0/team/" + std::to_string(p_cnt) + ".txt").c_str(), FALSE);
			FileRead_close(mdata);

			player[p_cnt].use = (player[p_cnt].id == 0) ? m : 2;
			player[p_cnt].pos = VGet(20.0f * p_cnt, 0.0f, -400.0f);
			player[p_cnt].type = TEAM;
		}
		for (size_t p_cnt = teamc; p_cnt < playerc; ++p_cnt) {
			mdata = FileRead_open(("stage/data_0/enemy/" + std::to_string(p_cnt) + ".txt").c_str(), FALSE);
			FileRead_close(mdata);

			player[p_cnt].use = 1;// p_cnt % parts->get_vehc();
			player[p_cnt].pos = VGet(20.0f * (p_cnt - teamc), 0.0f, 400.0f);
			player[p_cnt].type = ENEMY;
		}
		for (auto&& p : player) {
			p.yrad = DX_PI_F * p.type;
			std::fill(std::begin(p.waypos), std::end(p.waypos), p.pos);
			std::fill(std::begin(p.wayspd), std::end(p.wayspd), 2);
			/*vehsから引き継ぎ*/
			p.ptr = parts->get_vehicle(p.use);
		}
		/*UI*/
		uiparts->set_state(&player[0]);
		/*load*/
		SetUseASyncLoadFlag(TRUE);
			/*players*/
			SetCreate3DSoundFlag(TRUE);
			for (auto&& p : player) {
				p.obj = p.ptr->model.Duplicate();
				p.farobj = p.ptr->model_far.Duplicate();
				p.colobj = p.ptr->colmodel.Duplicate();
				for (auto&& h : p.hit) { h.pic = hit_mod.Duplicate(); }
				p.se.emplace_back(SoundHandle::Load("data/audio/se/engine/0.wav"));
				p.se.emplace_back(SoundHandle::Load("data/audio/se/fire/gun.wav"));
				size_t i = 2;
				for (; i < 10; ++i) {
					p.se.emplace_back(SoundHandle::Load("data/audio/se/fire/" + std::to_string(i - 2) + ".wav"));
				}
				for (; i < 27; ++i) {
					p.se.emplace_back(SoundHandle::Load("data/audio/se/ricochet/" + std::to_string(i - 10) + ".wav"));
				}
				for (; i < 29; ++i) {
					p.se.emplace_back(SoundHandle::Load("data/audio/se/engine/o" + std::to_string(i - 27) + ".wav"));
				}
				for (; i < 31; ++i) {
					p.se.emplace_back(SoundHandle::Load("data/audio/se/battle/hit_enemy/" + std::to_string(i - 29) + ".wav"));
				}
			}
		SetCreate3DSoundFlag(FALSE);
		SetUseASyncLoadFlag(FALSE);
		mapparts->set_map_readyb(mapc);
		uiparts->draw_load();//
		/*human*/
		humanparts->set_humans(player[0].ptr->inmodel);
		/*map*/
		if (!mapparts->set_map_ready()) { break; }
		//players
		const auto c_ffff96 = GetColor(255, 255, 150);
		const auto c_ffc896 = GetColor(255, 200, 150);
		for (auto&& p : player) {
			//色調
			for (int i = 0; i < MV1GetMaterialNum(p.obj.get()); ++i) {
				MV1SetMaterialSpcColor(p.obj.get(), i, GetColorF(0.85f, 0.82f, 0.78f, 0.5f));
				MV1SetMaterialSpcPower(p.obj.get(), i, 5.0f);
				MV1SetMaterialSpcColor(p.farobj.get(), i, GetColorF(0.85f, 0.82f, 0.78f, 0.5f));
				MV1SetMaterialSpcPower(p.farobj.get(), i, 5.0f);
			}
			MV1SetMaterialDrawAlphaTestAll(p.obj.get(), TRUE, DX_CMP_GREATER, 128);
			MV1SetMaterialDrawAlphaTestAll(p.farobj.get(), TRUE, DX_CMP_GREATER, 128);
			//リセット
			p.gear = 0;
			//cpu
			p.atkf = std::nullopt;
			p.aim = -1;
			//hit
			for (size_t i = 0; i < p.ptr->colmeshes; ++i) { MV1SetupCollInfo(p.colobj.get(), -1, 5, 5, 5,int(i)); }
			p.hitssort.resize(p.ptr->colmeshes);
			p.hitbuf = 0;
			for (int i = 0; i < p.hit.size(); ++i) { p.hit[i].flug = false; }
			//ammo
			p.Ammo.resize(ammoc*gunc);
			for (size_t i = 0; i < ammoc * gunc; ++i) {
				p.Ammo[i].color = (p.type == TEAM) ? c_ffff96 : c_ffc896;
			}
			//HP
			p.HP.resize(p.ptr->colmeshes);
			/*0123は装甲部分なので詰め込む*/
			p.HP[0] = 1;								//life
			for (size_t i = 4; i < p.ptr->colmeshes; ++i) { p.HP[i] = 100; }	//spaceARMER
			//wheel
			p.Springs.resize(p.ptr->frames);
			//0初期化いる
			//
			MV1SetMatrix(p.colobj.get(), MGetTranslate(VGet(0, 0, 0)));
			//装てん
			p.loadcnt[0] = 1;
			//ypos反映
			HitPoly = mapparts->get_gnd_hit(VAdd(p.pos, VGet(0.0f, (float)map_x, 0.0f)), VAdd(p.pos, VGet(0.0f, -(float)map_x, 0.0f)));
			if (HitPoly.HitFlag) { p.pos.y = HitPoly.HitPosition.y; }
			for (auto&& w : p.waypos) {
				HitPoly = mapparts->get_gnd_hit(VAdd(w, VGet(0.0f, (float)map_x, 0.0f)), VAdd(w, VGet(0.0f, -(float)map_x, 0.0f)));
				if (HitPoly.HitFlag) { w.y = HitPoly.HitPosition.y; }
			}
			//
		}
		//物理set
		for (auto&& p : player) {
			p.dynamicBox.SetAsBox(
				(p.ptr->coloc[0].x - p.ptr->coloc[2].x) / 2,
				(p.ptr->coloc[0].z - p.ptr->coloc[2].z) / 2,
				b2Vec2(
					(p.ptr->coloc[2].x + p.ptr->coloc[0].x) / 2,
					(p.ptr->coloc[2].z + p.ptr->coloc[0].z) / 2
				),
				0.f
			);
			p.fixtureDef.shape = &(p.dynamicBox);
			p.fixtureDef.density = 1.0f;					// ボックス密度をゼロ以外に設定すると、動的になります。
			p.fixtureDef.friction = 0.3f;					// デフォルトの摩擦をオーバーライドします。
			p.bodyDef.type = b2_dynamicBody;
			p.bodyDef.position.Set(p.pos.x, p.pos.z);
			p.bodyDef.angle = -p.yrad;
			p.body.reset(world->CreateBody(&(p.bodyDef)));
			p.playerfix = p.body->CreateFixture(&(p.fixtureDef));		// シェイプをボディに追加します。
		}
		/*音量調整*/
		humanparts->set_humanvc_vol(255);
		parts->set_se_vol(128);
		for (auto&& p : player) {
			for (auto& s : p.se) { ChangeVolumeSoundMem(128, s.get()); }
		}
		/*メインループ*/
		aim.flug = false;		/*照準*/
		map.flug = false;		/*マップ*/
		selfammo = 0;			/*選択弾種*/
		ratio = 1.0f;			/*照準視点　倍率*/
		rat_r = ratio;			/*照準視点　実倍率*/
		aim_r = 100.0f;			/*照準視点　距離*/
		waysel = 1;			/*指揮視点　指揮車両*/
		parts->set_viewrad(VGet(0.f, player[0].yrad, 1.f));
		SetCursorPos(x_r(960), y_r(540));
		old_time = GetNowHiPerformanceCount() + (LONGLONG)(1000000.0f / f_rates);
		for (auto& p : player) {
			p.effcs[ef_smoke2].efhandle = parts->get_effHandle(ef_smoke2).Play3D();
			p.effcs[ef_smoke3].efhandle = parts->get_effHandle(ef_smoke2).Play3D();
			PlaySoundMem(p.se[0].get(), DX_PLAYTYPE_LOOP, TRUE);
			PlaySoundMem(p.se[27].get(), DX_PLAYTYPE_LOOP, TRUE);
			PlaySoundMem(p.se[28].get(), DX_PLAYTYPE_LOOP, TRUE);
			Set3DRadiusSoundMem(200.0f, p.se[0].get());
			Set3DRadiusSoundMem(200.0f, p.se[1].get());
			size_t i = 2;
			for (; i < 10; ++i) { Set3DRadiusSoundMem(300.0f, p.se[i].get()); }
			for (; i < 27; ++i) { Set3DRadiusSoundMem(100.0f, p.se[i].get()); }
			for (; i < 29; ++i) { Set3DRadiusSoundMem(200.0f, p.se[i].get()); }
			for (; i < 31; ++i) { Set3DRadiusSoundMem(300.0f, p.se[i].get()); }
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
				if (player[0].HP[0] > 0) {
					keyget[0] = false;
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
				else {
					for (auto& tt : keyget) { tt = false; }
				}
				/*指揮*/
				if (keyget[1]) {
					map.cnt = std::min<std::uint8_t>(map.cnt+1, 2);
					if (map.cnt == 1) { map.flug ^= 1; SetCursorPos(x_r(960), y_r(540)); }
				}
				else { map.cnt = 0; }
				/*照準*/
				if (keyget[2]) {
					aim.cnt = std::min<std::uint8_t>(aim.cnt+1, 2);
					if (aim.cnt == 1) { aim.flug ^= 1; if (aim.flug) { ratio = 3.0f; } map.flug = false; }
				}
				else { aim.cnt = 0; }
				/*死んだときは無効*/
				if (player[0].HP[0] == 0) { aim.flug = false; map.flug = false; }
				/*弾種交換*/
				if (keyget[7]) {
					selfammo = std::min<std::uint8_t>(selfammo+1, 2);
					if (selfammo == 1) { ++player[0].ammotype; player[0].ammotype %= 3; player[0].loadcnt[0] = 1; }
				}
				else { if (player[0].loadcnt[0] == 0) { selfammo = 0; } }
				/*指揮*/
				if (map.flug) {
					GetMousePoint(&mousex, &mousey); SetMouseDispFlag(TRUE);
					choose = (std::numeric_limits<size_t>::max)();
					for (size_t p_cnt = 1; p_cnt < teamc; ++p_cnt) {
						if (player[p_cnt].HP[0] > 0) { if (inm(x_r(132), y_r(162 + p_cnt * 24), x_r(324), y_r(180 + p_cnt * 24))) { choose = p_cnt; if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) { waysel = p_cnt; } } }
					}
					if (player[waysel].HP[0] > 0) {
						if (player[waysel].wayselect <= waypc - 1) {
							if (inm(x_r(420), y_r(0), x_r(1500), y_r(1080))) {
								if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) {
									way = std::min<std::uint8_t>(way +1, 2);
									if (way == 1) {
										if (player[waysel].wayselect == 0) { player[waysel].waynow = 0; }
										player[waysel].waypos[player[waysel].wayselect] = VGet(_2x(mousex), 0, _2y(mousey));
										for (size_t i = player[waysel].wayselect; i < waypc; ++i) { player[waysel].waypos[i] = player[waysel].waypos[player[waysel].wayselect]; }
										++player[waysel].wayselect;
									}
								}
								else { way = 0; }
							}
						}
					}
				}
				/*視界見回し*/
				else {
					if (aim.flug) {
						SetMousePoint(x_r(960), y_r(540)); GetMouseWheelRotVol();
						if (keyget[3]) { ratio += 2.0f / fps; if (ratio >= 10.0f) { ratio = 10.0f; } }
						if (keyget[4]) { ratio -= 2.0f / fps; if (ratio <= 2.0f) { ratio = 2.0f; } }
						if (keyget[5]) { aim_r += 10.0f; }
						if (keyget[6]) { aim_r -= 10.0f; }
					}
					else { parts->set_view_r(); ratio = 1.0f; }
				}
				differential(rat_r, ratio, 0.1f);/*倍率、測距*/
			}
			else {
				SetMouseDispFlag(TRUE);
			}
			if (true) {
				/*操作、座標系*/
				for (auto&& p : player) {
					if (!map.flug) { p.wayselect = 0; }
					if (p.HP[0] > 0) {
						p.move = 0;
						if (p.id == 0) {
							/*操作*/
							if (keyget[9]) { p.move |= KEY_GOFLONT; }
							if (keyget[10]) { p.move |= KEY_GOBACK_; }
							if (keyget[11]) { p.move |= KEY_GOLEFT_; }
							if (keyget[12]) { p.move |= KEY_GORIGHT; }
							if (keyget[13]) { p.move |= KEY_TURNLFT; }
							if (keyget[14]) { p.move |= KEY_TURNRIT; }
							if (keyget[15]) { p.move |= KEY_TURNUP_; }
							if (keyget[16]) { p.move |= KEY_TURNDWN; }
							if (keyget[17]) { p.move |= KEY_SHOTCAN; }
							if (keyget[18]) { p.move |= KEY_SHOTGAN; }
							/*変速*/
							if (set_shift(&p)) { parts->play_sound(0); }
						}
						else {
							//*CPU操作
							//*移動
							if (VSize(VSub(p.pos, p.waypos[p.waynow])) >= 10.0) {
								p.move |= KEY_GOFLONT;
								tempvec[0] = VNorm(VSub(p.waypos[p.waynow], p.pos));
								cpu_move = -cos(p.yrad) * tempvec[0].x + sin(p.yrad) * tempvec[0].z;
								if (cpu_move < 0) { p.move |= KEY_GOLEFT_; }
								if (cpu_move > 0) { p.move |= KEY_GORIGHT; }
							}
							else { ++p.waynow; if (p.waynow >= waypc - 1) { p.waynow = waypc - 1; } }
							/*
							for (i = p.waynow; i < waypc; i++) { if (VSize(VSub(p.pos, p.waypos[i])) < VSize(VSub(p.pos, p.waypos[p.waynow]))) { p.waynow = i; break; } }
							*/
							//*戦闘
							if (!p.atkf) {
								for (auto& t : player) {
									if (p.type != t.type) {
										if (VSize(VSub(t.pos, p.pos)) <= 500.0f && t.HP[0] > 0) {
											if (p.aim != p.atkf) {
												p.aim = 0;
												p.atkf = t.id;
												break;
											}
										}
									}
								}
								p.gear = p.wayspd[p.waynow];											//*変速
							}
							else {
								p.gear = 1;																	//*変速
								tempvec[1] = p.obj.frame(bone_gun1);												//*元のベクトル
								tempvec[0] = VNorm(VSub(player[p.atkf.value()].obj.frame( bone_gun1), tempvec[1]));							//*向くベクトル
								tmpf = VSize(VSub(player[p.atkf.value()].obj.frame(bone_gun1), tempvec[1]));
								getdist(&tempvec[1], VNorm(VSub(p.obj.frame(bone_gun2), tempvec[1])), &tmpf, p.ptr->gun_speed[p.ammotype], f_rates);

								tempvec[1] = VNorm(VSub(tempvec[1], p.obj.frame(bone_gun1)));
								cpu_move = tempvec[1].y * sqrtf(powf(tempvec[0].x, 2) + powf(tempvec[0].z, 2)) - sqrtf(powf(tempvec[1].x, 2) + powf(tempvec[1].z, 2)) * tempvec[0].y;	//*砲
								if (cpu_move <= 0) { p.move |= KEY_TURNUP_; }
								if (cpu_move > 0) { p.move |= KEY_TURNDWN; }
								cpu_move = tempvec[1].z * tempvec[0].x - tempvec[1].x * tempvec[0].z;											//*車体
								if (cpu_move < 0) { p.move |= KEY_TURNLFT; }
								if (cpu_move > 0) { p.move |= KEY_TURNRIT; }
								if (cpu_move < 0) { p.move |= KEY_GOLEFT_; }
								if (cpu_move > 0) { p.move |= KEY_GORIGHT; }
								if (VSize(VCross(tempvec[1], tempvec[0])) < sin(deg2rad(1))) {
									HitPoly = mapparts->get_gnd_hit(p.obj.frame(bone_gun1), player[p.atkf.value()].obj.frame(bone_gun1));
									if (!HitPoly.HitFlag) {
										if (p.loadcnt[0] == 0) {
											if ((p.move & KEY_GOFLONT) != 0) { p.move -= KEY_GOFLONT; }
											p.gear = 0;															//*変速
											if (p.speed < 5.f / 3.6f / f_rates) {
												p.move |= KEY_SHOTCAN;
												p.aim++;
											}
										}
										if (GetRand(100) <= 2) { p.move |= KEY_SHOTGAN; }
									}
								}
								if (player[p.atkf.value()].HP[0] == 0 || p.aim > 5) { p.aim = int(p.atkf.value()); p.atkf = std::nullopt; }
							}
							//ぶつかり防止
							for (auto& t : player) {
								if (p.id != t.id) {
									if (VSize(VSub(t.pos, p.pos)) <= 10.0 && t.HP[0] > 0) {
										tempvec[0] = VNorm(VSub(t.pos, p.pos));
										cpu_move = -cos(p.yrad) * tempvec[0].x + sin(p.yrad) * tempvec[0].z;
										if (cpu_move > 0) { p.move |= KEY_GOLEFT_; if ((p.move & KEY_GORIGHT) != 0) { p.move -= KEY_GORIGHT; } }
										if (cpu_move < 0) { p.move |= KEY_GORIGHT; if ((p.move & KEY_GOLEFT_) != 0) { p.move -= KEY_GOLEFT_; } }
									}
								}
								/*
								if (p.state == CPU_NOMAL) {
									if (VSize(VSub(t.pos, p.pos)) <= 250.0 &&  p_cnt != tgt_p && p.type != t.type &&  p.waynow != waypc - 1) {
										if (p.waynow != 0) {
											p.waynow = waypc - 1;
											for (j = 0; j < waypc; ++j) {
												p.waypos[j] = p.pos;
											}
											player[waysel].wayselect = 0;
										}
										else {
											if (VSize(VSub(p.waypos[p.waynow], t.pos)) > 225.f) {
												p.waynow = waypc - 1;
												p.waypos[p.waynow] = p.pos;
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
					else { p.move = KEY_TURNUP_; }
				}
				/*共通動作*/
				for (auto& p : player) {
					if (p.id == 0) {
						set_gunrad(&p, rat_r*((keyget[8]) ? 3.f : 1.f));//左CTRLを押すと精密エイム
					}
					else {
						set_gunrad(&p, 1.f);
					}
					p.bodyDef.awake = true;
					p.yrad = p.body->GetAngle();
					//
					if ((p.move & KEY_GOFLONT) != 0) {
						if (p.HP[5] == 0 || p.HP[6] == 0) { p.move -= KEY_GOFLONT; }
						else {
							if (p.gear > 0) {
								if (p.flont >= p.ptr->spdflont[p.gear - 1]) { p.flont -= p.ptr->spdflont[p.gear - 1] / (5.0f * fps); }
								else { p.flont += p.ptr->spdflont[p.gear - 1] / (10.0f * fps); }
							}
						}
					}
					if ((p.move & KEY_GOBACK_) != 0) {
						if (p.HP[5] == 0 || p.HP[6] == 0) { p.move -= KEY_GOBACK_; }
						else {
							if (p.gear < 0) {
								if (p.back <= p.ptr->spdback[-p.gear - 1]) { p.back -= p.ptr->spdback[-p.gear - 1] / (2.0f * fps); }
								else { p.back += p.ptr->spdback[-p.gear - 1] / (5.0f * fps); }
							}
						}
					}
					{
						float turn_bias = 1.0f;
						if (p.HP[5] > 0 || p.HP[6] > 0) {
							turn_bias = 1.0f;
							if (p.HP[5] > 0 && p.HP[6] > 0) {
								if ((p.move & KEY_GOFLONT) != 0 && p.gear > 0) {
									turn_bias = p.flont / (p.ptr->spdflont[p.gear - 1]);
								}
								if ((p.move & KEY_GOBACK_) != 0 && p.gear < 0) {
									turn_bias = p.back / (p.ptr->spdback[-p.gear - 1]);
								}
								turn_bias = abs(turn_bias);
							}
							turn_bias *= ((p.HP[5] > 0) + (p.HP[6] > 0)) / 2.0f;
						}
						if (p.yace == 0.0f) {
							if ((p.move & KEY_GOLEFT_) != 0) {
								if (p.HP[5] == 0 && p.HP[6] == 0) {
									p.move -= KEY_GOLEFT_;
								}
								else {
									differential(p.yadd, p.ptr->vehicle_RD * turn_bias, 0.1f);
								}
							}
							if ((p.move & KEY_GORIGHT) != 0) {
								if (p.HP[5] == 0 && p.HP[6] == 0) {
									p.move -= KEY_GORIGHT;
								}
								else {
									differential(p.yadd, -p.ptr->vehicle_RD * turn_bias, 0.1f);
								}
							}
						}
					}
					p.speed = p.flont + p.back;
					differential(p.inertia.x, (p.speed - p.speedrec), 0.02f); p.speedrec = p.speed;
					differential(p.inertia.z, -(p.znor - p.znorrec) / 2.0f, 0.1f); p.znorrec = p.znor;
					//vec
					p.vec.x = p.speed * sin(p.yrad);
					p.vec.z = -p.speed * cos(p.yrad);
					if (p.HP[5] == 0) {
						p.vec.x = p.ptr->loc[bone_wheel].x * sin(p.yadd) * sin(p.yrad);
						p.vec.z = -p.ptr->loc[bone_wheel].x * sin(p.yadd) * cos(p.yrad);
					}
					if (p.HP[6] == 0) {
						p.vec.x = -p.ptr->loc[bone_wheel].x * sin(p.yadd) * sin(p.yrad);
						p.vec.z = p.ptr->loc[bone_wheel].x * sin(p.yadd) * cos(p.yrad);
					}
					//
					p.wheelrad[0] += p.speed / f_rates;//
					p.wheelrad[1] = -p.wheelrad[0] * 2 + p.yrad * 5;
					p.wheelrad[2] = -p.wheelrad[0] * 2 - p.yrad * 5;
					//
					p.body->SetLinearVelocity(b2Vec2(p.vec.x, p.vec.z));
					p.body->SetAngularVelocity(p.yadd);
				}
				/*物理演算*/
				world->Step(1.0f / f_rates, 1, 1);	// シミュレーションの単一ステップを実行するように世界に指示します。 一般に、タイムステップと反復を固定しておくのが最善です。
				for (auto& p : player) {
					p.pos.x = p.body->GetPosition().x;
					p.pos.z = p.body->GetPosition().y;
					p.yrad = -p.body->GetAngle();
				}
				/*砲撃その他*/
				for (auto& p : player) {
					//地形判定
					HitPoly = mapparts->get_gnd_hit(VAdd(p.pos, VGet(0.0f, 2.0f, 0.0f)), VAdd(p.pos, VGet(0.0f, -0.05f, 0.0f)));//0.3ms
					if (HitPoly.HitFlag) {
						p.yace = 0.0f;
						p.pos.y = HitPoly.HitPosition.y;
						mapparts->set_normal(&p.xnor, &p.znor, p.pos);
						p.nor = VTransform(VGet(0, 1.f, 0), MMult(MGetRotX(p.xnor), MGetRotZ(p.znor)));//p.nor = VAdd(p.nor, VScale(VSub(HitPoly.Normal, p.nor), 0.1f));
						/*speed*/
						if ((p.move & KEY_GOLEFT_) == 0 && (p.move & KEY_GORIGHT) == 0) { p.yadd *= 0.9f; }
						if (p.gear >= 0 || (p.move & KEY_GOBACK_) == 0) { p.back *= 0.95f; }
						if (p.gear <= 0 || (p.move & KEY_GOFLONT) == 0) { p.flont *= 0.95f; }
						/*track*/
						mapparts->draw_map_track(p);//0.1ms
					}
					else {
						p.yadd *= 0.95f;
						p.pos.y += p.yace;
						p.yace += M_GR / 2.0f / fps / fps;
					}
					//地形判定全体=3.8ms
					//サウンド
					ChangeVolumeSoundMem(std::min<int>(int(64.f*abs(p.speed / p.ptr->spdflont[0])), 64), p.se[0].get());
					for (size_t i = 27; i < 29; ++i) { ChangeVolumeSoundMem((int)(32.f + 32.f*abs(p.speed / p.ptr->spdflont[3])), p.se[i].get()); }
					for (auto& s : p.se) { if (CheckSoundMem(s.get()) == 1) { Set3DPositionSoundMem(p.pos, s.get()); } }
					//サウンド全体=1.7ms
					//tree判定
					mapparts->set_hitplayer(p.pos);
					//0.1ms
					/*車体行列*/
					p.ps_m = MMult(MMult(MMult(MMult(MGetRotAxis(VGet(cos(p.gunrad.x), 0, -sin(p.gunrad.x)), sin(deg2rad(p.firerad)) * deg2rad(5)), MGetRotAxis(VGet(cos(p.recorad), 0, -sin(p.recorad)), sin(deg2rad(p.recoall)) * deg2rad(5))), MGetRotX(atan(p.inertia.x))), MMult(MGetRotY(p.yrad), MGetRotVec2(VGet(0, 1.f, 0), p.nor))), MGetTranslate(p.pos));//MMult(MGetRotX(p.xnor), MGetRotZ(p.znor))
					/*砲塔行列*/
					p.ps_t = MMult(MGetRotY(p.gunrad.x), MGetTranslate(p.ptr->loc[bone_trt]));
					//all
					MV1SetMatrix(p.colobj.get(), p.ps_m);
					MV1SetMatrix(p.obj.get(), p.ps_m);
					MV1SetMatrix(p.farobj.get(), p.ps_m);
					//common
					for (int i = bone_trt; i < p.ptr->frames; ++i) {
						if (i == bone_trt) {
							MV1SetFrameUserLocalMatrix(p.obj.get(), i, p.ps_t);
							MV1SetFrameUserLocalMatrix(p.farobj.get(), i, p.ps_t);
							MV1SetFrameUserLocalMatrix(p.colobj.get(), i, p.ps_t);
						}
						else if (i == bone_gun1) {
							const auto mtemp = MMult(MMult(MGetRotX(p.gunrad.y), MGetTranslate(VSub(p.ptr->loc[bone_gun1], p.ptr->loc[bone_trt]))), p.ps_t);
							MV1SetFrameUserLocalMatrix(p.obj.get(), i, mtemp);
							MV1SetFrameUserLocalMatrix(p.farobj.get(), i, mtemp);
							MV1SetFrameUserLocalMatrix(p.colobj.get(), i, mtemp);
						}
						else if (i == bone_gun2) {
							const auto mtemp = MGetTranslate(VAdd(VSub(p.ptr->loc[bone_gun2], p.ptr->loc[bone_gun1]), VGet(0, 0, p.fired)));
							MV1SetFrameUserLocalMatrix(p.obj.get(), i, mtemp);
							MV1SetFrameUserLocalMatrix(p.farobj.get(), i, mtemp);
							MV1SetFrameUserLocalMatrix(p.colobj.get(), i, mtemp);
						}
						else if (i == bone_gun) {
							const auto mtemp = MMult(MMult(MGetRotX(p.gunrad.y), MGetTranslate(VSub(p.ptr->loc[bone_gun], p.ptr->loc[bone_trt]))), p.ps_t);
							MV1SetFrameUserLocalMatrix(p.obj.get(), i, mtemp);
							MV1SetFrameUserLocalMatrix(p.farobj.get(), i, mtemp);
						}
						else if (i == bone_gun_) {
							const auto mtemp = MGetTranslate(VSub(p.ptr->loc[bone_gun_], p.ptr->loc[bone_gun]));
							MV1SetFrameUserLocalMatrix(p.obj.get(), i, mtemp);
							MV1SetFrameUserLocalMatrix(p.farobj.get(), i, mtemp);
						}
						else if (i >= bone_wheel && i < p.ptr->frames - 4) {
							if ((i - bone_wheel) % 2 == 1) {
								const auto mtemp = MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[i].x) + 1]), MGetTranslate(VSub(p.ptr->loc[i], p.ptr->loc[i - 1])));
								MV1SetFrameUserLocalMatrix(p.obj.get(), i, mtemp);
								MV1SetFrameUserLocalMatrix(p.farobj.get(), i, mtemp);
							}
							else {
								MV1ResetFrameUserLocalMatrix(p.obj.get(), i);
								HitPoly = mapparts->get_gnd_hit(VAdd(p.obj.frame(i), VScale(p.nor, 1.0f)), VAdd(p.obj.frame(i), VScale(p.nor, -0.2f)));
								if (HitPoly.HitFlag) {
									p.Springs[i] = std::min<float>(p.Springs[i] + 1.0f / fps, 1.0f - VSize(VSub(HitPoly.HitPosition, VAdd(p.obj.frame(i), VScale(p.nor, 1.0f)))));
								}
								else {
									p.Springs[i] = std::max<float>(p.Springs[i] - 0.2f / fps, -0.2f);
								}
								const auto mtemp = MGetTranslate(VAdd(p.ptr->loc[i], VScale(p.nor, p.Springs[i])));
								MV1SetFrameUserLocalMatrix(p.obj.get(), i, mtemp);
								MV1SetFrameUserLocalMatrix(p.farobj.get(), i, mtemp);
								//2ms
							}
						}
						else if (i >= p.ptr->frames - 4) {
							const auto mtemp = MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[i].x) + 1]), MGetTranslate(p.ptr->loc[i]));
							MV1SetFrameUserLocalMatrix(p.obj.get(), i, mtemp);
							MV1SetFrameUserLocalMatrix(p.farobj.get(), i, mtemp);
						}
					}
					/*collition*/
					for (size_t i = 0; i < p.ptr->colmeshes; ++i) { MV1RefreshCollInfo(p.colobj.get(), -1,int(i)); }
					/**/
					if (p.fired >= 0.01f) { p.fired *= 0.95f; }
					if (p.loadcnt[0] > 0) {
						if (p.firerad < 180) { if (p.firerad <= 90) { p.firerad += 900 / (int)fps; } else { p.firerad += 180 / (int)fps; } }
						else { p.firerad = 180; }
					}

					for (size_t guns = 0; guns < gunc; ++guns) {
						if (p.loadcnt[guns] == 0) {
							if ((p.move & (KEY_SHOTCAN << guns)) != 0) {
								const auto j = p.useammo[guns] + guns * ammoc;
								p.Ammo[j].flug = 1;
								p.Ammo[j].speed = p.ptr->gun_speed[p.ammotype] / fps;
								p.Ammo[j].pene = p.ptr->pene[p.ammotype];
								if (guns == 0) { p.Ammo[j].pos = p.obj.frame(bone_gun1); }
								else { p.Ammo[j].pos = p.obj.frame(bone_gun); }
								tempvec[0] = VSub(p.obj.frame(bone_gun2), p.obj.frame(bone_gun1));

								p.Ammo[j].rad.y = atan2(tempvec[0].x, (tempvec[0].z)) + deg2rad((float)(-1000 + GetRand(2000)) / 10000.f);
								p.Ammo[j].rad.x = atan2(-tempvec[0].y, sqrt(pow(tempvec[0].x, 2) + pow(tempvec[0].z, 2))) - deg2rad((float)(-1000 + GetRand(2000)) / 10000.f);
								p.Ammo[j].vec = VGet(cos(p.Ammo[j].rad.x) * sin(p.Ammo[j].rad.y), -sin(p.Ammo[j].rad.x), cos(p.Ammo[j].rad.x) * cos(p.Ammo[j].rad.y));
								p.Ammo[j].repos = p.Ammo[j].pos;
								p.Ammo[j].cnt = 0;
								//					       
								p.useammo[guns] ++; p.useammo[guns] %= ammoc;
								++p.loadcnt[guns];
								if (guns == 0) {
									set_effect(&(p.effcs[ef_fire]), p.obj.frame(bone_gun2), VSub(p.obj.frame(bone_gun2), p.obj.frame(bone_gun1)));
									p.fired = 0.5f;
									p.firerad = 0;
									if (p.id == 0) { humanparts->start_humananime(2); parts->play_sound(1 + GetRand(6)); }
									PlaySoundMem(p.se[size_t(2) + GetRand(7)].get(), DX_PLAYTYPE_BACK, TRUE);
								}
								else {
									set_effect(&(p.effcs[ef_gun]), p.obj.frame(bone_gun_), VGet(0, 0, 0));
									PlaySoundMem(p.se[1].get(), DX_PLAYTYPE_BACK, TRUE);
								}
							}
						}
						else {
							++p.loadcnt[guns];
							if (p.loadcnt[guns] >= p.ptr->reloadtime[guns]) { p.loadcnt[guns] = 0; if (p.id == 0 && guns == 0) { parts->play_sound(8 + GetRand(4)); } }//装てん完了
						}
						for (size_t i = guns * ammoc; i < ammoc + guns * ammoc; ++i) {
							if (p.Ammo[i].flug != 0) {
								p.Ammo[i].repos = p.Ammo[i].pos;
								p.Ammo[i].pos = VAdd(p.Ammo[i].pos, VScale(p.Ammo[i].vec, p.Ammo[i].speed));
								HitPoly = mapparts->get_gnd_hit(p.Ammo[i].repos, p.Ammo[i].pos);
								if (HitPoly.HitFlag) { p.Ammo[i].pos = HitPoly.HitPosition; }

								bool btmp = false;
								for (auto& t : player) {
									if (p.id == t.id) { continue; }
									btmp = get_reco(&p, &t, i, guns);
									if (btmp) { break; }
								}
								if (!btmp) {
									if (HitPoly.HitFlag) {
										set_effect(&p.effcs[ef_gndhit + guns * (ef_gndhit2 - ef_gndhit)], HitPoly.HitPosition, HitPoly.Normal);
										p.Ammo[i].vec = VAdd(p.Ammo[i].vec, VScale(HitPoly.Normal, VDot(p.Ammo[i].vec, HitPoly.Normal) * -2.0f));
										p.Ammo[i].pos = VAdd(HitPoly.HitPosition, VScale(p.Ammo[i].vec, 0.01f));
										//p.Ammo[i].pene /= 2.0f;
										p.Ammo[i].speed /= 2.f;
									}
								}

								tempvec[0] = VSub(p.Ammo[i].pos, p.Ammo[i].repos);
								p.Ammo[i].rad.y = atan2(tempvec[0].x, tempvec[0].z);
								p.Ammo[i].rad.x = atan2(-tempvec[0].y, sqrt(pow(tempvec[0].x, 2) + pow(tempvec[0].z, 2)));
								p.Ammo[i].vec.y += M_GR / 2.0f / fps / fps;
								p.Ammo[i].pene -= 1.0f / fps;
								p.Ammo[i].speed -= 5.f / fps;
								p.Ammo[i].cnt++;
								if (p.Ammo[i].cnt > (fps*3.f) || p.Ammo[i].speed <= 0.f) { p.Ammo[i].flug = 0; }//3秒で消える
							}
						}
					}
					if (p.recoadd) {
						if (p.recoall < 180) {
							if (p.id == 0 && p.recoall == 0) { uiparts->set_reco(); }
							if (p.recoall <= 90) { p.recoall += 900 / (int)fps; }
							else { p.recoall += 180 / (int)fps; }
						}
						else { p.recoall = 0; p.recoadd = false; }
					}
					if (p.hitadd) {
						if (p.id == 0) { humanparts->start_humanvoice(0); }
						p.hitadd = false;
					}
				}
				/*轍更新*/
				mapparts->set_map_track();
				/*human*/
				humanparts->set_humanmove(player[0], parts->get_view_r(), fps);
				/*effect*/
				for (auto& p : player) {
					for (int i = 0; i < efs_user; ++i) {
						if (i != ef_smoke2 && i != ef_smoke3) { set_pos_effect(&p.effcs[i], parts->get_effHandle(i)); }
					}
					p.effcs[ef_smoke2].efhandle.SetPos(p.obj.frame(bone_smoke1));
					//SetTargetLocation(p.effcs[ef_smoke2].efhandle, p.obj.frame(bone_smoke1));
					p.effcs[ef_smoke3].efhandle.SetPos(p.obj.frame(bone_smoke2));
					//SetTargetLocation(p.effcs[ef_smoke3].efhandle, p.obj.frame(bone_smoke2));
				}
				UpdateEffekseer3D();
			}
			/*視点*/
			if (aim.flug) {
				campos = player[0].obj.frame(bone_gun1);

				viewpos = player[0].obj.frame(bone_gun1);
				getdist(&viewpos, VNorm(VSub(player[0].obj.frame(bone_gun2), player[0].obj.frame(bone_gun1))), &aim_r, player[0].ptr->gun_speed[player[0].ammotype], f_rates);

				uppos = player[0].nor;
			}
			else {
				if (parts->get_in()) {
					campos = VAdd(player[0].pos, VAdd(parts->get_view_pos(), VGet(0, 2, 0)));
					viewpos = VAdd(player[0].pos, VGet(0, 4, 0));
					HitPoly = mapparts->get_gnd_hit(campos, viewpos); if (HitPoly.HitFlag) { campos = HitPoly.HitPosition; }
					campos = VAdd(campos, VGet(0, 2, 0));
					uppos = VGet(0, 1, 0);
				}
				else {
					campos = humanparts->get_campos();
					viewpos = humanparts->get_neckpos();
					uppos = player[0].nor;
				}
			}
			mapparts->set_camerapos(campos, viewpos, uppos, rat_r);
			/*shadow*/
			mapparts->set_map_shadow_near(parts->get_view_r().z);
			/*draw*/
			/*map*/
			if (map.flug) {
				SetDrawScreen(minimap);
				ClearDrawScreen();
				DrawExtendGraph(x_r(420), y_r(0), x_r(1500), y_r(1080), mapparts->get_minmap().get(), FALSE);
				for (auto& p : player) {
					DrawCircle(x_(p.pos.x), y_(p.pos.z), 5, (p.type == TEAM) ? (p.HP[0] == 0) ? c_008000 : c_00ff00 : (p.HP[0] == 0) ? c_800000 : c_ff0000, TRUE);
				}
				//進軍
				for (size_t p_cnt = 1; p_cnt < teamc; ++p_cnt) {
					DrawLine(x_(player[p_cnt].pos.x), y_(player[p_cnt].pos.z), x_(player[p_cnt].waypos[player[p_cnt].waynow].x), y_(player[p_cnt].waypos[player[p_cnt].waynow].z), c_ff0000, 3);
					for (int i = int(player[p_cnt].waynow); i < waypc - 1; ++i) {
						DrawLine(x_(player[p_cnt].waypos[i].x), y_(player[p_cnt].waypos[i].z), x_(player[p_cnt].waypos[i + 1].x), y_(player[p_cnt].waypos[i + 1].z), GetColor(255, 64 * i, 0), 3);
					}
				}
				for (size_t p_cnt = 0; p_cnt < teamc; ++p_cnt) {
					//ステータス
					const auto c = (player[p_cnt].id == waysel)
						? (player[p_cnt].HP[0] == 0) ? c_c8c800 : c_ffff00
						: (p_cnt == choose)
							? c_c0ff00
							: (player[p_cnt].HP[0] == 0) ? c_808080 : c_00ff00;
					DrawBox(x_r(132), y_r(162 + player[p_cnt].id * 24), x_r(324), y_r(180 + player[p_cnt].id * 24), c, TRUE);
					DrawFormatStringToHandle(x_r(132), y_r(162 + player[p_cnt].id * 24), c_ffffff, parts->get_font(0), " %s", player[p_cnt].ptr->name.c_str());
					//進軍パラメータ
					for (size_t i = 0; i < player[p_cnt].wayselect; i++) { DrawBox(x_r(348 + i * 12), y_r(162 + player[p_cnt].id * 24), x_r(356 + i * 12), y_r(180 + player[p_cnt].id * 24), c_3232ff, TRUE); }
				}
				//敵
				for (size_t p_cnt = teamc; p_cnt < playerc; ++p_cnt) {
					DrawBox(x_r(1500), y_r(162 + (player[p_cnt].id - teamc) * 24), x_r(1692), y_r(180 + (player[p_cnt].id - teamc) * 24), (player[p_cnt].HP[0] == 0) ? c_808080 : c_ff0000, TRUE);
					DrawFormatStringToHandle(x_r(1500), y_r(162 + (player[p_cnt].id - teamc) * 24), c_ffffff, parts->get_font(0), " %s", player[p_cnt].ptr->name.c_str());
				}
			}
			/*main*/
			else {
				uiparts->end_way();//debug0//0
				uiparts->end_way();//debug1//0
				uiparts->end_way();//debug2//0

				/*sky*/
				if (parts->get_in() || aim.flug) {
					SetDrawScreen(skyscreen);
					mapparts->draw_map_sky();
				}
				/*near*/
				SetDrawScreen(mainscreen);
				ClearDrawScreen();
				if (aim.flug) { setcv(0.06f + rat_r / 2, 2000.0f, campos, viewpos, uppos, 45.0f / rat_r); }
				else { setcv(0.16f + parts->get_view_r().z, 2000.0f, campos, viewpos, uppos, 45.0f / rat_r); }
				//----------------------------------------------------------
				if (aim.flug) {
					tempvec[0] = player[0].obj.frame(bone_gun1);
					getdist(&tempvec[0], VNorm(VSub(player[0].obj.frame(bone_gun2), player[0].obj.frame(bone_gun1))), &aim_r, player[0].ptr->gun_speed[player[0].ammotype], f_rates);
					aims = ConvWorldPosToScreenPos(tempvec[0]);
					aimm = aim_r / 1000.0f * player[0].ptr->gun_speed[player[0].ammotype];
				}
				//pos

				for (auto& p : player) {
					p.iconpos = ConvWorldPosToScreenPos(VAdd(p.pos, VGet(0, VSize(VSub(p.pos, player[0].pos)) / 40 + 6, 0)));

					if (CheckCameraViewClip_Box(VAdd(p.pos, VGet(-5, 0, -5)), VAdd(p.pos, VGet(5, 3, 5))) == TRUE) {
						pssort[p.id] = pair(p.id, (float)map_x);
					}
					else {
						pssort[p.id] = pair(p.id, VSize(VSub(p.pos, campos)));
					}
				}
				std::sort(pssort.begin(), pssort.end(), [](const pair& x, const pair& y) {return x.second > y.second; });

				//effect
				Effekseer_Sync3DSetting();
				//---------------------------------------------------------------
				if (parts->get_in() || aim.flug) {
					DrawGraph(0, 0, skyscreen, FALSE);//sky
					//main
					ShadowMap_DrawSetup(mapparts->get_map_shadow_seminear());
					for (auto& tt : pssort) {
						if (tt.second == (float)map_x) { continue; }
						if (tt.second < (10.0f *float(parts->get_shadex())* parts->get_view_r().z + 20.0f)) { break; }
						const auto j = tt.first;
						MV1DrawMesh(player[j].farobj.get(), 0);
						for (int i = 1; i < player[j].ptr->meshes; ++i) {
							if (player[j].HP[i + 4] > 0) { MV1DrawMesh(player[j].farobj.get(), i); }
						}
					}
					ShadowMap_DrawEnd();
					ShadowMap_DrawSetup(mapparts->get_map_shadow_near());
					humanparts->draw_human(0);
					for (auto& tt : pssort) {
						if (tt.second > (10.0f *float(parts->get_shadex())* parts->get_view_r().z + 20.0f)) { continue; }
						const auto j = tt.first;
						MV1DrawMesh(player[j].obj.get(), 0);
						for (int i = 1; i < player[j].ptr->meshes; ++i) {
							if (player[j].HP[i + 4] > 0) { MV1DrawMesh(player[j].obj.get(), i); }
						}
					}
					ShadowMap_DrawEnd();

					humanparts->draw_human(0);

					mapparts->ready_shadow();
						mapparts->draw_map_model();
						for (auto& tt : pssort) {
							if (tt.second == (float)map_x) { continue; }
							const auto j = tt.first;
							if (j != 0 || (j == 0 && !aim.flug)) {
								if (tt.second < 200.f || aim.flug) {
									MV1DrawMesh(player[j].obj.get(), 0);
									for (int i = 1; i < player[j].ptr->meshes; ++i) {
										if (i < 3) { MV1SetFrameTextureAddressTransform(player[j].obj.get(), 0, 0.0, player[j].wheelrad[i], 1.0, 1.0, 0.5, 0.5, 0.0); }
										if (i == 3) { MV1ResetFrameTextureAddressTransform(player[j].obj.get(), 0); }
										if (player[j].HP[i + 4] > 0) { MV1DrawMesh(player[j].obj.get(), i); }
									}
									//MV1DrawModel(player[j].colobj.get());/*コリジョンモデルを出すとき*/
									for (int i = 0; i < player[j].hit.size(); ++i) {
										if (player[j].hit[i].flug) {
											MV1SetRotationZYAxis(player[j].hit[i].pic.get(), VSub(player[j].colobj.frame( 11 + 3 * i), player[j].colobj.frame(9 + 3 * i)), VSub(player[j].colobj.frame(10 + 3 * i), player[j].colobj.frame(9 + 3 * i)), 0.f);
											MV1SetPosition(player[j].hit[i].pic.get(), VAdd(player[j].colobj.frame(9 + 3 * i), VScale(VSub(player[j].colobj.frame(10 + 3 * i), player[j].colobj.frame(9 + 3 * i)), 0.005f)));
											MV1DrawFrame(player[j].hit[i].pic.get(), player[j].hit[i].use);
										}
									}
								}
								else {
									MV1DrawMesh(player[j].farobj.get(), 0);
									for (int i = 1; i < player[j].ptr->meshes; ++i) {
										if (i >= 1 && i < 3) { MV1SetFrameTextureAddressTransform(player[j].farobj.get(), 0, 0.0, player[j].wheelrad[i], 1.0, 1.0, 0.5, 0.5, 0.0); }
										if (i == 3) { MV1ResetFrameTextureAddressTransform(player[j].farobj.get(), 0); }
										if (player[j].HP[i + 4] > 0) { MV1DrawMesh(player[j].farobj.get(), i); }
									}
								}
							}
						}
						//grass
						if (!aim.flug) { mapparts->draw_grass(); }
						//effect
						DrawEffekseer3D();
						//ammo
						uiparts->end_way();//debug3//0
						SetUseLighting(FALSE);
						SetFogEnable(FALSE);
						for (auto& p : player) {
							for (size_t i = 0; i < ammoc; ++i) {
								if (p.Ammo[i].flug != 0) {
									SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f*std::min(1.f, 4.f*p.Ammo[i].speed / (p.ptr->gun_speed[p.ammotype] / fps))));
									DrawCapsule3D(p.Ammo[i].pos, p.Ammo[i].repos, p.ptr->ammosize*(VSize(VSub(p.Ammo[i].pos, campos)) / 65.f), 4, p.Ammo[i].color, c_ffffff, TRUE);
								}
							}
							for (size_t i = ammoc; i < ammoc * gunc; ++i) {
								if (p.Ammo[i].flug != 0) {
									SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f*std::min(1.f, 4.f*p.Ammo[i].speed / (p.ptr->gun_speed[p.ammotype] / fps))));
									DrawCapsule3D(p.Ammo[i].pos, p.Ammo[i].repos, 0.0075f*(VSize(VSub(p.Ammo[i].pos, campos)) / 30.f), 4, p.Ammo[i].color, c_ffffff, TRUE);
								}
							}
						}
						SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
						SetFogEnable(TRUE);
						SetUseLighting(TRUE);

						//tree
						mapparts->draw_trees();
						DrawEffekseer3D();
					mapparts->exit_shadow();

					uiparts->end_way();//debug4//0
				}
				else {
					humanparts->draw_humanall();
				}
			}
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
				if (aim.flug) { uiparts->draw_sight(aims.x, aims.y, rat_r, aimm, parts->get_font(0)); }/*照準器*/
				else {
					for (auto& tt : pssort) {
						if (tt.first == 0) { continue; }
						if (tt.second == (float)map_x) { continue; }
						const auto i = tt.first;
						if (player[i].HP[0] != 0) {
							if (player[i].iconpos.z > 0.0f && player[i].iconpos.z < 1.0f) {
								DrawFormatStringToHandle(
									(int)player[i].iconpos.x, (int)player[i].iconpos.y, (player[i].type == TEAM) ? c_00ff00 : c_ff0000, parts->get_font(0),
									"%dm", (int)VSize(VSub(player[i].pos, player[0].pos))
								);
							}
						}
					}
				}/*アイコン*/
				uiparts->draw_ui(selfammo, parts->get_view_r().y);/*main*/
			}
			/*debug*/
			//DrawFormatStringToHandle(x_r(18), y_r(1062), c_ffffff, parts->get_font(0), "start-stop(%.2fms)", (float)stop_w / 1000.f);
			uiparts->debug(fps, (float)(GetNowHiPerformanceCount() - waits) / 1000.0f);

			parts->Screen_Flip(waits);
		}
		//delete
		mapparts->delete_map();
		humanparts->delete_human();
		for (auto& p : player) {
			/*エフェクト*/
			for (auto&& e : p.effcs) e.efhandle.Dispose();
			/*Box2D*/
			delete p.playerfix->GetUserData();
			p.playerfix->SetUserData(NULL);
			/**/
			p.obj.Dispose();
			p.farobj.Dispose();
			p.colobj.Dispose();
			for (auto& h : p.hit) h.pic.Dispose();
			for (auto& s : p.se) s.Dispose();
			p.Ammo.clear();
			p.Springs.clear();
			p.HP.clear();
			p.hitssort.clear();
		}
		pssort.clear();
		player.clear();
	} while (!out);
	/*終了*/
	return 0;
}
