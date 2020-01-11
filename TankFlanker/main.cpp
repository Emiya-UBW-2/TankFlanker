#define NOMINMAX
#include "define.h"
#include "useful.h"
//#include <algorithm>
//#include <memory>

/*main*/
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//temp----------------------------------------------------------------------------------//
	int mousex, mousey; /*mouse*/
	VECTOR tempvec[2];
	std::array<bool, 20> keyget; /*キー用*/
	//変数----------------------------------------------------------------------------------//
	bool out{ false };					      /*終了フラグ*/
	std::vector<pair> pssort;				      /*playerソート*/
	std::vector<players> player;				      /*player*/
	VECTOR aims = VGet(0, 0, 0);				      /*照準器座標*/
	float aimm = 0.f;					      /*照準距離*/
	float fps;						      /*fps*/
	uint8_t selfammo;					      /*UI用*/
	switches aim, map;					      /*視点変更*/
	float ratio, rat_r, aim_r, rat_aim;			      /*カメラ　倍率、実倍率、距離、照準視点倍率*/
	size_t waysel, choose = (std::numeric_limits<size_t>::max)(); /*指揮視点　指揮車両、マウス選択*/
	uint8_t way = 0, bak = 0;				      //マウストリガー
	LONGLONG old_time, waits;				      /*時間取得*/
	VECTOR campos, viewpos, uppos;				      /*カメラ*/
	int mapc;
	//init----------------------------------------------------------------------------------//
	auto parts = std::make_unique<Myclass>();							       /*汎用クラス*/
	auto humanparts = std::make_unique<HUMANS>(parts->get_usegrab(), parts->get_f_rate());		       /*車内関係*/
	auto mapparts = std::make_unique<MAPS>(parts->get_gndx(), parts->get_drawdist(), parts->get_shadex()); /*地形、ステージ関係*/
	auto uiparts = std::make_unique<UIS>();
	float f_rates = parts->get_f_rate();
	//load----------------------------------------------------------------------------------//

	parts->write_option(); //オプション書き込み

	parts->set_fonts(18);
	SetUseASyncLoadFlag(TRUE);
	//hit---------------------------------------------------------------------------//
	const auto hit_mod = MV1ModelHandle::Load("data/hit/hit.mv1");
	//screen------------------------------------------------------------------------//
	int minimap = MakeScreen(dispx, dispy, FALSE);			     /*ミニマップ*/
	int skyscreen = MakeScreen(dispx, dispy, FALSE);		     /*空*/
	int mainscreen = MakeScreen(dispx, dispy, FALSE);		     /*遠景*/
	int HighBrightScreen = MakeScreen(dispx, dispy, FALSE);		     /*エフェクト*/
	int GaussScreen = MakeScreen(dispx / EXTEND, dispy / EXTEND, FALSE); /*エフェクト*/
	SetUseASyncLoadFlag(FALSE);
	uiparts->draw_load(); //
	if (!parts->set_veh())
		return -1;
	/*物理開始*/
	auto world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f)); /* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
	//これ以降繰り返しロード----------------------------------------------------------------//
	do {
		std::string stage = "data_0";
		{
			const auto mdata = FileRead_open(("stage/" + stage + "/main.txt").c_str(), FALSE); /*ステージ情報*/
			char mstr[64];									   /*tank*/
			FileRead_gets(mstr, 64, mdata);
			mapc = std::stoi(getright(mstr));
			FileRead_close(mdata);
		}
		const size_t teamc = count_team(stage);	  /*味方数*/
		const size_t enemyc = count_enemy(stage); /*敵数*/
		player.resize(teamc + enemyc);
		pssort.resize(teamc + enemyc);
		for (int p_cnt = 0; p_cnt < teamc + enemyc; ++p_cnt)
			player[p_cnt].id = p_cnt;
		{
			const int m = parts->window_choosev(); /*player指定*/
			if (m == -1)
				return 0;
			//設定
			for (auto&& p : player) {
				if (p.id < teamc) {
					p.type = TEAM;
					{
						const auto mdata = FileRead_open(("stage/" + stage + "/team/" + std::to_string(p.id) + ".txt").c_str(), FALSE);
						//char mstr[64]; /*tank*/
						//
						p.use = (p.id == 0) ? m : 0; //2;
						p.pos = VGet(20.0f * p.id, 0.0f, -400.0f);
						FileRead_close(mdata);
					}
				}
				else {
					p.type = ENEMY;
					{
						const auto mdata = FileRead_open(("stage/" + stage + "/enemy/" + std::to_string(p.id - teamc) + ".txt").c_str(), FALSE);
						//char mstr[64]; /*tank*/
						//
						p.use = 0; //1; // p.id % parts->get_vehc();
						p.pos = VGet(20.0f * (p.id - teamc), 0.0f, 400.0f);
						FileRead_close(mdata);
					}
				}
			}
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
			p.colobj = p.ptr->colmodel.Duplicate();
			for (auto&& h : p.hit)
				h.pic = hit_mod.Duplicate();
			p.se.emplace_back(SoundHandle::Load("data/audio/se/engine/0.wav"));
			p.se.emplace_back(SoundHandle::Load("data/audio/se/fire/gun.wav"));
			size_t i = 2;
			for (; i < 10; ++i)
				p.se.emplace_back(SoundHandle::Load("data/audio/se/fire/" + std::to_string(i - 2) + ".wav"));
			for (; i < 27; ++i)
				p.se.emplace_back(SoundHandle::Load("data/audio/se/ricochet/" + std::to_string(i - 10) + ".wav"));
			for (; i < 29; ++i)
				p.se.emplace_back(SoundHandle::Load("data/audio/se/engine/o" + std::to_string(i - 27) + ".wav"));
			for (; i < 31; ++i)
				p.se.emplace_back(SoundHandle::Load("data/audio/se/battle/hit_enemy/" + std::to_string(i - 29) + ".wav"));
		}
		SetCreate3DSoundFlag(FALSE);
		SetUseASyncLoadFlag(FALSE);
		mapparts->set_map_readyb(mapc);
		uiparts->draw_load(); //
		/*human*/
		humanparts->set_humans(player[0].ptr->inmodel);
		/*map*/
		if (!mapparts->set_map_ready()) {
			break;
		}
		//players
		const auto c_ffff96 = GetColor(255, 255, 150);
		const auto c_ffc896 = GetColor(255, 200, 150);
		for (auto&& p : player) {
			//色調
			for (int i = 0; i < p.obj.material_num(); ++i) {
				MV1SetMaterialSpcColor(p.obj.get(), i, GetColorF(0.85f, 0.82f, 0.78f, 0.5f));
				MV1SetMaterialSpcPower(p.obj.get(), i, 5.0f);
			}
			MV1SetMaterialDrawAlphaTestAll(p.obj.get(), TRUE, DX_CMP_GREATER, 128);
			//リセット
			p.gear = 0;
			//cpu
			p.atkf = std::nullopt;
			p.aim = -1;
			//hit
			for (int i = 0; i < p.ptr->colmodel.mesh_num(); ++i)
				MV1SetupCollInfo(p.colobj.get(), -1, 5, 5, 5, i);
			p.hitssort.resize(p.ptr->colmodel.mesh_num());
			p.hitbuf = 0;
			for (int i = 0; i < p.hit.size(); ++i)
				p.hit[i].flug = false;
			//ammo
			for (size_t guns = 0; guns < gunc; ++guns) {
				p.Gun[guns].Ammo.resize(ammoc);
				for (auto& c : p.Gun[guns].Ammo)
					c.color = (p.type == TEAM) ? c_ffff96 : c_ffc896;
			}
			//HP
			p.HP.resize(p.ptr->colmodel.mesh_num());
			/*0123は装甲部分なので詰め込む*/
			p.HP[0] = 1; //life
			for (size_t i = 4; i < p.HP.size(); ++i) {
				p.HP[i] = 100;
			} //spaceARMER
			//wheel
			p.Springs.resize(p.obj.frame_num());
			//0初期化いる
			//
			MV1SetMatrix(p.colobj.get(), MGetTranslate(VGet(0, 0, 0)));
			//装てん
			p.Gun[0].loadcnt = 1;
			//ypos反映
			{
				auto HitPoly = mapparts->get_gnd_hit(VAdd(p.pos, VGet(0.0f, (float)map_x, 0.0f)), VAdd(p.pos, VGet(0.0f, -(float)map_x, 0.0f)));
				if (HitPoly.HitFlag)
					p.pos.y = HitPoly.HitPosition.y;
				for (auto&& w : p.waypos) {
					HitPoly = mapparts->get_gnd_hit(VAdd(w, VGet(0.0f, (float)map_x, 0.0f)), VAdd(w, VGet(0.0f, -(float)map_x, 0.0f)));
					if (HitPoly.HitFlag)
						w.y = HitPoly.HitPosition.y;
				}
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
				(p.ptr->coloc[2].z + p.ptr->coloc[0].z) / 2),
			    0.f);
			p.fixtureDef.shape = &p.dynamicBox;
			p.fixtureDef.density = 1.0f;  // ボックス密度をゼロ以外に設定すると、動的になります。
			p.fixtureDef.friction = 0.3f; // デフォルトの摩擦をオーバーライドします。
			p.bodyDef.type = b2_dynamicBody;
			p.bodyDef.position.Set(p.pos.x, p.pos.z);
			p.bodyDef.angle = -p.yrad;
			p.body.reset(world->CreateBody(&p.bodyDef));
			p.playerfix = p.body->CreateFixture(&p.fixtureDef); // シェイプをボディに追加します。
			/* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/

			for (size_t LR = 0; LR < 2; ++LR) {
				p.foot[LR] = new b2World(b2Vec2(0.0f, 0.0f));
				{
					b2Body* ground = NULL;
					{
						b2BodyDef bd;
						ground = p.foot[LR]->CreateBody(&bd);
						b2EdgeShape shape;
						shape.Set(b2Vec2(-40.0f, -10.0f), b2Vec2(40.0f, -10.0f));
						ground->CreateFixture(&shape, 0.0f);
					}
					b2Body* prevBody = ground;
					int i = 0;
					VECTOR vects = VGet(0, 0, 0);
					for (auto& w : p.ptr->upsizeframe) {
						vects = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w));
						if (vects.x * ((LR == 0) ? 1.f : -1.f) > 0) {
							p.Foot[LR].resize(i + 1);
							p.f_dynamicBox[LR].SetAsBox(0.1f, 0.125f);
							p.f_fixtureDef[LR].shape = &p.f_dynamicBox[LR];
							p.f_fixtureDef[LR].density = 20.0f;
							p.f_fixtureDef[LR].friction = 0.2f;
							p.f_bodyDef[LR].type = b2_dynamicBody;
							p.f_bodyDef[LR].position.Set(vects.z, vects.y);
							p.Foot[LR][i].f_body.reset(p.foot[LR]->CreateBody(&p.f_bodyDef[LR]));
							p.Foot[LR][i].f_playerfix = p.Foot[LR][i].f_body->CreateFixture(&p.f_fixtureDef[LR]); // シェイプをボディに追加します。
							p.f_jointDef[LR].Initialize(prevBody, p.Foot[LR][i].f_body.get(), b2Vec2(vects.z, vects.y));
							p.foot[LR]->CreateJoint(&p.f_jointDef[LR]);
							prevBody = p.Foot[LR][i].f_body.get();
							++i;
						}
					}
					p.f_jointDef[LR].Initialize(prevBody, ground, b2Vec2(vects.z, vects.y));
					p.foot[LR]->CreateJoint(&p.f_jointDef[LR]);
					i = 0;
					for (auto& w : p.ptr->wheelframe) {
						vects = VAdd(
						    VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(player[0].obj.get(), w + 1)),
						    VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(player[0].obj.get(), w)));
						if (vects.x * ((LR == 0) ? 1.f : -1.f) > 0) {
							p.Fwheel[LR].resize(i + 1);
							b2CircleShape shape;
							shape.m_radius = VSize(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w + 1))) - 0.1f;
							p.fw_fixtureDef[LR].shape = &shape;
							p.fw_fixtureDef[LR].density = 1.0f;
							p.fw_bodyDef[LR].type = b2_kinematicBody;
							p.fw_bodyDef[LR].position.Set(vects.z, vects.y);
							p.Fwheel[LR][i].fw_body.reset(p.foot[LR]->CreateBody(&p.fw_bodyDef[LR]));
							p.playerfix = p.Fwheel[LR][i].fw_body->CreateFixture(&p.fw_fixtureDef[LR]);
							++i;
						}
					}
				}
			}
		}
		/*音量調整*/
		humanparts->set_humanvc_vol(255);
		parts->set_se_vol(128);
		for (auto&& p : player)
			for (auto& s : p.se)
				ChangeVolumeSoundMem(128, s.get());
		/*メインループ*/
		aim.flug = false; /*照準*/
		map.flug = false; /*マップ*/
		selfammo = 0;	  /*選択弾種*/
		rat_aim = 3.f;	  /*照準視点　倍率*/
		ratio = 1.0f;	  /*カメラ　　倍率*/
		rat_r = ratio;	  /*カメラ　　実倍率*/
		aim_r = 100.0f;	  /*照準視点　距離*/
		waysel = 1;	  /*指揮視点　指揮車両*/
		parts->set_viewrad(VGet(0.f, player[0].yrad, 1.f));
		SetCursorPos(x_r(960), y_r(540));
		old_time = GetNowHiPerformanceCount() + (LONGLONG)(1000000.0f / f_rates);
		for (auto& p : player) {
			p.effcs[ef_smoke2].handle = parts->get_effHandle(ef_smoke2).Play3D();
			p.effcs[ef_smoke3].handle = parts->get_effHandle(ef_smoke2).Play3D();
			PlaySoundMem(p.se[0].get(), DX_PLAYTYPE_LOOP, TRUE);
			PlaySoundMem(p.se[27].get(), DX_PLAYTYPE_LOOP, TRUE);
			PlaySoundMem(p.se[28].get(), DX_PLAYTYPE_LOOP, TRUE);
			size_t i = 0;
			for (; i < 2; ++i)
				Set3DRadiusSoundMem(200.0f, p.se[i].get());
			for (; i < 10; ++i)
				Set3DRadiusSoundMem(300.0f, p.se[i].get());
			for (; i < 27; ++i)
				Set3DRadiusSoundMem(100.0f, p.se[i].get());
			for (; i < 29; ++i)
				Set3DRadiusSoundMem(200.0f, p.se[i].get());
			for (; i < 31; ++i)
				Set3DRadiusSoundMem(300.0f, p.se[i].get());
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
			uiparts->put_way(); //debug
			if (GetActiveFlag() == TRUE) {
				SetMouseDispFlag(FALSE);
				if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
					out = true;
					break;
				} /*終了*/
				if (CheckHitKey(KEY_INPUT_P) != 0) {
					break;
				} /*リスタート*/
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
					keyget[19] = CheckHitKey(KEY_INPUT_RCONTROL) != 0;
				}
				else
					for (auto& tt : keyget)
						tt = false;
				/*指揮*/
				if (keyget[1]) {
					map.cnt = std::min<uint8_t>(map.cnt + 1, 2);
					if (map.cnt == 1) {
						map.flug ^= 1;
						SetCursorPos(x_r(960), y_r(540));
					}
				}
				else {
					map.cnt = 0;
				}
				/*照準*/
				if (keyget[2]) {
					aim.cnt = std::min<uint8_t>(aim.cnt + 1, 2);
					if (aim.cnt == 1) {
						aim.flug ^= 1;
						//一回だけ進めたいものはここに
						if (aim.flug)
							ratio = rat_aim;
						else {
							rat_aim = ratio;
							ratio = 1.0f;
						}
						map.flug = false;
					}
				}
				else {
					aim.cnt = 0;
				}
				/*死んだときは無効*/
				if (player[0].HP[0] == 0) {
					aim.flug = false;
					map.flug = false;
				}
				/*弾種交換*/
				if (keyget[7]) {
					selfammo = std::min<uint8_t>(selfammo + 1, 2);
					if (selfammo == 1) {
						++player[0].ammotype;
						player[0].ammotype %= 3;
						player[0].Gun[0].loadcnt = 1;
					}
				}
				else {
					if (player[0].Gun[0].loadcnt == 0) {
						selfammo = 0;
					}
				}
				/*指揮*/
				if (map.flug) {
					GetMousePoint(&mousex, &mousey);
					SetMouseDispFlag(TRUE);
					choose = (std::numeric_limits<size_t>::max)();
					for (auto&& p : player) {
						if (p.id == 0)
							continue;
						if (p.HP[0] > 0) {
							if (inm(x_r(132), y_r(162 + p.id * 24), x_r(324), y_r(180 + p.id * 24))) {
								choose = p.id;
								if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0)
									waysel = choose;
							}
						}
					}
					if (player[waysel].HP[0] > 0) {
						if (inm(x_r(420), y_r(0), x_r(1500), y_r(1080))) {
							if (player[waysel].wayselect <= waypc - 1) {
								if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) {
									way = std::min<uint8_t>(way + 1, 2);
									if (way == 1) {
										if (player[waysel].wayselect == 0) {
											player[waysel].waynow = 0;
										}
										player[waysel].waypos[player[waysel].wayselect] = VGet(_2x(mousex), 0, _2y(mousey));
										for (size_t i = player[waysel].wayselect; i < waypc; ++i) {
											player[waysel].waypos[i] = player[waysel].waypos[player[waysel].wayselect];
										}
										++player[waysel].wayselect;
									}
								}
								else {
									way = 0;
								}
							}
							if (player[waysel].wayselect > 0) {
								if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
									bak = std::min<uint8_t>(bak + 1, 2);
									if (bak == 1) {
										player[waysel].waynow = std::max<size_t>(player[waysel].waynow - 1, 0);
										--player[waysel].wayselect;
										if (player[waysel].wayselect >= 1) {
											for (size_t i = player[waysel].wayselect; i < waypc; ++i) {
												player[waysel].waypos[i] = player[waysel].waypos[player[waysel].wayselect - 1];
											}
										}
										else {
											for (size_t i = 0; i < waypc; ++i) {
												player[waysel].waypos[i] = player[waysel].pos;
											}
										}
									}
								}
								else {
									bak = 0;
								}
							}
						}
					}
				}
				/*視界見回し*/
				else {
					if (aim.flug) {
						SetMousePoint(x_r(960), y_r(540));
						GetMouseWheelRotVol();
						if (keyget[3])
							ratio = std::min<float>(ratio + 2.0f / f_rates, 10.f);
						if (keyget[4])
							ratio = std::max<float>(ratio - 2.0f / f_rates, 2.f);
						if (keyget[5])
							aim_r += 10.0f;
						if (keyget[6])
							aim_r -= 10.0f;
					}
					else
						parts->set_view_r();
				}
				differential(rat_r, ratio, 0.1f); /*倍率、測距*/
			}
			else {
				SetMouseDispFlag(TRUE);
			}
			if (true) {
				/*操作、座標系*/
				for (auto&& p : player) {
					if (!map.flug) {
						p.wayselect = 0;
					}
					if (p.HP[0] > 0) {
						p.move = 0;
						if (p.id == 0) {
							/*操作*/
							for (auto i = 0; i < 10; i++)
								p.move |= keyget[i + 9] << i;
							/*変速*/
							if (set_shift(p))
								parts->play_sound(0);
						}
						else {
							//*CPU操作
							//*移動
							if (VSize(VSub(p.pos, p.waypos[p.waynow])) >= 10.0) {
								p.move |= KEY_GOFLONT;
								tempvec[0] = VNorm(VSub(p.waypos[p.waynow], p.pos));
								if ((-cos(p.yrad) * tempvec[0].x + sin(p.yrad) * tempvec[0].z) < 0)
									p.move |= KEY_GOLEFT_;
								else
									p.move |= KEY_GORIGHT;
							}
							else
								p.waynow = std::min<size_t>(p.waynow + 1, waypc - 1);
							/*
							for (i = p.waynow; i < waypc; i++) { if (VSize(VSub(p.pos, p.waypos[i])) < VSize(VSub(p.pos, p.waypos[p.waynow]))) { p.waynow = i; break; } }
							*/
							//*戦闘
							if (!p.atkf) {
								p.gear = p.wayspd[p.waynow]; //*変速
								for (auto& t : player) {
									if (p.type == t.type || t.HP[0] == 0)
										continue;
									if (VSize(VSub(t.pos, p.pos)) <= 500.0f) { //見つける
										if (p.aim != p.atkf) {		   //前狙った敵でないか
											p.aim = 0;
											p.atkf = t.id;
											break;
										}
									}
								}
							}
							else {
								p.gear = 1; //*変速
								{
									tempvec[1] = p.obj.frame(p.ptr->gunframe[0]);									 //*元のベクトル
									tempvec[0] = VNorm(VSub(player[p.atkf.value()].obj.frame(player[p.atkf.value()].ptr->gunframe[0]), tempvec[1])); //*向くベクトル
									float tmpf = VSize(VSub(player[p.atkf.value()].obj.frame(player[p.atkf.value()].ptr->gunframe[0]), tempvec[1]));
									getdist(&tempvec[1], VNorm(VSub(p.obj.frame(p.ptr->gunframe[0] + 1), tempvec[1])), &tmpf, p.ptr->gun_speed[p.ammotype], f_rates);
									tempvec[1] = VNorm(VSub(tempvec[1], p.obj.frame(p.ptr->gunframe[0])));
								}
								if (cross2D(
									std::hypot(tempvec[0].x, tempvec[0].z), tempvec[0].y,
									std::hypot(tempvec[1].x, tempvec[1].z), tempvec[1].y) <= 0)
									p.move |= KEY_TURNUP_;
								else
									p.move |= KEY_TURNDWN;

								if (cross2D(tempvec[0].x, tempvec[0].z, tempvec[1].x, tempvec[1].z) < 0) {
									p.move |= KEY_TURNLFT;
									p.move |= KEY_GOLEFT_; //
								}
								else {
									p.move |= KEY_TURNRIT;
									p.move |= KEY_GORIGHT; //
								}
								if (VSize(VCross(tempvec[1], tempvec[0])) < sin(deg2rad(1))) {
									const auto HitPoly = mapparts->get_gnd_hit(p.obj.frame(p.ptr->gunframe[0]), player[p.atkf.value()].obj.frame(player[p.atkf.value()].ptr->gunframe[0]));
									if (!HitPoly.HitFlag) {
										if (p.Gun[0].loadcnt == 0) {
											if ((p.move & KEY_GOFLONT) != 0)
												p.move -= KEY_GOFLONT;
											p.gear = 0; //*変速
											if (p.speed < 5.f / 3.6f / f_rates) {
												p.move |= KEY_SHOTCAN;
												p.aim++;
											}
										}
										if (GetRand(100) <= 2)
											p.move |= KEY_SHOTGAN;
									}
								}
								if (player[p.atkf.value()].HP[0] == 0 || p.aim > 5) {
									p.aim = int(p.atkf.value());
									p.atkf = std::nullopt;
								}
							}
							//ぶつかり防止
							for (auto& t : player) {
								if (p.id != t.id && t.HP[0] > 0) {
									if (VSize(VSub(t.pos, p.pos)) <= 10.0) {
										tempvec[0] = VNorm(VSub(t.pos, p.pos));
										if ((-cos(p.yrad) * tempvec[0].x + sin(p.yrad) * tempvec[0].z) > 0) {
											p.move |= KEY_GOLEFT_;
											if ((p.move & KEY_GORIGHT) != 0)
												p.move -= KEY_GORIGHT;
										}
										else {
											p.move |= KEY_GORIGHT;
											if ((p.move & KEY_GOLEFT_) != 0)
												p.move -= KEY_GOLEFT_;
										}
									}
								}
								/*
								if (p.state == CPU_NOMAL) {
									if (VSize(VSub(t.pos, p.pos)) <= 250.0 &&  p.id != tgt_p && p.type != t.type &&  p.waynow != waypc - 1) {
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
					else {
						p.move = KEY_TURNUP_;
					}
				}
				/*共通動作*/
				for (auto& p : player) {
					if (p.id == 0)
						set_gunrad(p, rat_r * ((keyget[8]) ? 3.f : 1.f)); //左CTRLを押すと精密エイム
					else
						set_gunrad(p, 1.f);
					p.bodyDef.awake = true;
					p.yrad = p.body->GetAngle();
					//
					if ((p.move & KEY_GOFLONT) != 0) {
						if (p.HP[5] == 0 || p.HP[6] == 0)
							p.move -= KEY_GOFLONT;
						else if (p.gear > 0)
							p.flont = std::min<float>(p.flont + p.ptr->speed_flont[p.gear - 1] / (5.0f * f_rates), p.ptr->speed_flont[p.gear - 1]);
					}
					if ((p.move & KEY_GOBACK_) != 0) {
						if (p.HP[5] == 0 || p.HP[6] == 0)
							p.move -= KEY_GOBACK_;
						else if (p.gear < 0)
							p.back = std::max<float>(p.back + p.ptr->speed_back[-p.gear - 1] / (5.0f * f_rates), p.ptr->speed_back[-p.gear - 1]);
					}
					{
						float turn_bias = 0.f;
						if (p.HP[5] > 0 || p.HP[6] > 0) {
							turn_bias = 1.0f;
							if (p.HP[5] > 0 && p.HP[6] > 0) {
								if ((p.move & KEY_GOFLONT) != 0 && p.gear > 0)
									turn_bias = abs(p.flont / p.ptr->speed_flont[p.gear - 1]);
								if ((p.move & KEY_GOBACK_) != 0 && p.gear < 0)
									turn_bias = abs(p.back / p.ptr->speed_back[-p.gear - 1]);
							}
							turn_bias *= ((p.HP[5] > 0) + (p.HP[6] > 0)) / 2.0f; //履帯が切れていると
						}
						if (p.yace == 0.0f) {
							if ((p.move & KEY_GOLEFT_) != 0) {
								if (p.HP[5] == 0 && p.HP[6] == 0)
									p.move -= KEY_GOLEFT_;
								else
									differential(p.yadd, p.ptr->vehicle_RD * turn_bias, 0.1f);
							}
							if ((p.move & KEY_GORIGHT) != 0) {
								if (p.HP[5] == 0 && p.HP[6] == 0)
									p.move -= KEY_GORIGHT;
								else
									differential(p.yadd, -p.ptr->vehicle_RD * turn_bias, 0.1f);
							}
						}
					}
					p.speed = p.flont + p.back;
					differential(p.inertia.x, (p.speed - p.speedrec), 0.02f);
					p.speedrec = p.speed;
					differential(p.inertia.z, -(p.znor - p.znorrec) / 2.0f, 0.1f);
					p.znorrec = p.znor;
					//vec
					p.vec.x = p.speed * sin(p.yrad);
					p.vec.z = -p.speed * cos(p.yrad);
					if (p.HP[5] == 0 || p.HP[6] == 0) {
						p.vec.x = p.ptr->loc[p.ptr->wheelframe[0]].x * sin(p.yadd) * sin(p.yrad) * ((p.HP[5] == 0) - (p.HP[6] == 0));
						p.vec.z = p.ptr->loc[p.ptr->wheelframe[0]].x * sin(p.yadd) * cos(p.yrad) * ((p.HP[6] == 0) - (p.HP[5] == 0));
					}
					//
					p.wheelrad[0] += p.speed / f_rates; //
					p.wheelrad[1] = -p.wheelrad[0] * 2 + p.yrad * 5;
					p.wheelrad[2] = -p.wheelrad[0] * 2 - p.yrad * 5;
					//
					p.body->SetLinearVelocity(b2Vec2(p.vec.x, p.vec.z));
					p.body->SetAngularVelocity(p.yadd);


					for (size_t LR = 0; LR < 2; ++LR) {
						int i = 0;
						for (auto& w : p.ptr->wheelframe) {
							VECTOR vects = VAdd(
							    VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w + 1)),
							    VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w)));
							if (vects.x * ((LR == 0) ? 1.f : -1.f) > 0) {
								p.Fwheel[LR][i].fw_body->SetTransform(b2Vec2(vects.z, vects.y), 0.f);
								i++;
							}
						}
						for (auto& f : p.Foot[LR])
							f.f_body->SetLinearVelocity(b2Vec2(0.f, -0.98f));
						p.foot[LR]->Step(1.0f / f_rates, 3, 3);
						for (auto& f : p.Foot[LR]) {
							f.f_p.y = f.f_body->GetPosition().y;
							f.f_p.z = f.f_body->GetPosition().x;
						}
					}
				}
				/*物理演算*/
				world->Step(1.0f / f_rates, 1, 1); // シミュレーションの単一ステップを実行するように世界に指示します。 一般に、タイムステップと反復を固定しておくのが最善です。

				for (auto& p : player) {
					p.pos.x = p.body->GetPosition().x;
					p.pos.z = p.body->GetPosition().y;
					p.yrad = -p.body->GetAngle();

					p.vec.x = p.body->GetLinearVelocity().x;
					p.vec.z = p.body->GetLinearVelocity().y;
				}
				/*砲撃その他*/
				for (auto& p : player) {
					//地形判定
					{
						const auto HitPoly = mapparts->get_gnd_hit(VAdd(p.pos, VGet(0.0f, 2.0f, 0.0f)), VAdd(p.pos, VGet(0.0f, -0.05f, 0.0f))); //0.3ms
						if (HitPoly.HitFlag) {
							p.yace = 0.0f;
							p.pos.y = HitPoly.HitPosition.y;
							mapparts->set_normal(&p.xnor, &p.znor, p.pos);
							p.nor = VTransform(VGet(0, 1.f, 0), MMult(MGetRotX(p.xnor), MGetRotZ(p.znor))); //p.nor = VAdd(p.nor, VScale(VSub(HitPoly.Normal, p.nor), 0.1f));
							/*speed*/
							if ((p.move & KEY_GOLEFT_) == 0 && (p.move & KEY_GORIGHT) == 0) {
								p.yadd *= 0.9f;
							}
							if (p.gear >= 0 || (p.move & KEY_GOBACK_) == 0) {
								p.back *= 0.95f;
							}
							if (p.gear <= 0 || (p.move & KEY_GOFLONT) == 0) {
								p.flont *= 0.95f;
							}
							/*track*/
							mapparts->draw_map_track(p); //0.1ms
						}
						else {
							p.yadd *= 0.95f;
							p.pos.y += p.yace;
							p.yace += M_GR / 2.0f / f_rates / f_rates;
						}
					}
					//サウンド
					ChangeVolumeSoundMem(std::min<int>(int(64.f * abs(p.speed / p.ptr->speed_flont[0])), 64), p.se[0].get());
					for (size_t i = 27; i < 29; ++i)
						ChangeVolumeSoundMem((int)(32.f + 32.f * abs(p.speed / p.ptr->speed_flont[3])), p.se[i].get());
					for (auto& s : p.se)
						if (CheckSoundMem(s.get()) == 1)
							Set3DPositionSoundMem(p.pos, s.get());
					//サウンド全体=1.7ms
					//tree判定
					mapparts->set_hitplayer(p.pos);
					//0.1ms
					/*車体行列*/
					p.ps_m = MMult(MMult(MMult(MMult(MGetRotAxis(VGet(cos(p.gunrad.x), 0, -sin(p.gunrad.x)), sin(deg2rad(p.firerad)) * deg2rad(5)), MGetRotAxis(VGet(cos(p.recorad), 0, -sin(p.recorad)), sin(deg2rad(p.recoall)) * deg2rad(5))), MGetRotX(atan(p.inertia.x))), MMult(MGetRotY(p.yrad), MGetRotVec2(VGet(0, 1.f, 0), p.nor))), MGetTranslate(p.pos)); //MMult(MGetRotX(p.xnor), MGetRotZ(p.znor))
					/*砲塔行列*/
					p.ps_t = MMult(MGetRotY(p.gunrad.x), MGetTranslate(p.ptr->loc[p.ptr->turretframe]));
					//all
					MV1SetMatrix(p.colobj.get(), p.ps_m);
					MV1SetMatrix(p.obj.get(), p.ps_m);
					//common
					MV1SetFrameUserLocalMatrix(p.obj.get(), p.ptr->turretframe, p.ps_t);
					MV1SetFrameUserLocalMatrix(p.colobj.get(), 2, p.ps_t);

					for (int guns = 0; guns < gunc; ++guns) {
						auto mtemp = MMult(MMult(MGetRotX(p.gunrad.y), MGetTranslate(VSub(p.ptr->loc[p.ptr->gunframe[guns]], p.ptr->loc[p.ptr->turretframe]))), p.ps_t);
						MV1SetFrameUserLocalMatrix(p.obj.get(), p.ptr->gunframe[guns], mtemp);
						if (guns == 0)
							MV1SetFrameUserLocalMatrix(p.colobj.get(), 3, mtemp);
						mtemp = MGetTranslate(VAdd(VSub(p.ptr->loc[p.ptr->gunframe[guns] + 1], p.ptr->loc[p.ptr->gunframe[guns]]), VGet(0, 0, p.Gun[guns].fired)));
						MV1SetFrameUserLocalMatrix(p.obj.get(), p.ptr->gunframe[guns] + 1, mtemp);
						if (guns == 0)
							MV1SetFrameUserLocalMatrix(p.colobj.get(), 3 + 1, mtemp);
					}
					for (auto& w : p.ptr->wheelframe) {
						MV1ResetFrameUserLocalMatrix(p.obj.get(), w);
						const auto HitPoly = mapparts->get_gnd_hit(VAdd(p.obj.frame(w), VScale(p.nor, 1.0f)), VAdd(p.obj.frame(w), VScale(p.nor, -0.2f)));
						if (HitPoly.HitFlag)
							p.Springs[w] = std::min<float>(p.Springs[w] + 1.0f / f_rates, 1.0f - VSize(VSub(HitPoly.HitPosition, VAdd(p.obj.frame(w), VScale(p.nor, 1.0f)))));
						else
							p.Springs[w] = std::max<float>(p.Springs[w] - 0.2f / f_rates, -0.2f);
						MV1SetFrameUserLocalMatrix(p.obj.get(), w, MGetTranslate(VAdd(p.ptr->loc[w], VScale(p.nor, p.Springs[w]))));
						MV1SetFrameUserLocalMatrix(p.obj.get(), w + 1, MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[w + 1].x) + 1]), MGetTranslate(VSub(p.ptr->loc[w + 1], p.ptr->loc[w]))));
					}
					for (auto& w : p.ptr->youdoframe)
						MV1SetFrameUserLocalMatrix(p.obj.get(), w, MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[w].x) + 1]), MGetTranslate(p.ptr->loc[w])));
					for (auto& w : p.ptr->kidoframe)
						MV1SetFrameUserLocalMatrix(p.obj.get(), w, MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[w].x) + 1]), MGetTranslate(p.ptr->loc[w])));
					{
						size_t i = 0, j = 0;
						for (auto& w : p.ptr->upsizeframe) {
							float xw = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w)).x;
							if (xw > 0) {
								p.Foot[0][i].f_p.x = xw;
								MV1SetFrameUserLocalMatrix(p.obj.get(), w, MGetTranslate(p.Foot[0][i].f_p));
								++i;
							}
							else {
								p.Foot[1][j].f_p.x = xw;
								MV1SetFrameUserLocalMatrix(p.obj.get(), w, MGetTranslate(p.Foot[1][j].f_p));
								++j;
							}
						}
					}
					/*collition*/
					for (int i = 0; i < p.ptr->colmodel.mesh_num(); ++i)
						MV1RefreshCollInfo(p.colobj.get(), -1, i);
					/*反動*/
					if (p.Gun[0].loadcnt > 0) {
						if (p.firerad < 180)
							if (p.firerad <= 90)
								p.firerad += 900 / (int)f_rates;
							else
								p.firerad += 180 / (int)f_rates;
						else
							p.firerad = 180;
					}

					for (size_t guns = 0; guns < gunc; ++guns) {
						if (p.Gun[guns].fired >= 0.01f)
							p.Gun[guns].fired *= 0.95f;
						if (p.Gun[guns].loadcnt == 0) {
							if ((p.move & (KEY_SHOTCAN << guns)) != 0) {
								auto& a = p.Gun[guns].Ammo[p.Gun[guns].useammo];
								a.flug = true;
								a.speed = p.ptr->gun_speed[p.ammotype] / f_rates;
								a.pene = p.ptr->pene[p.ammotype];
								a.pos = p.obj.frame(p.ptr->gunframe[guns]);
								a.repos = a.pos;
								a.cnt = 0;

								tempvec[0] = VSub(p.obj.frame(p.ptr->gunframe[guns] + 1), a.pos);
//								const auto y = atan2(tempvec[0].x, tempvec[0].z) + deg2rad((float)(-1000 + GetRand(2000)) / 10000.f);				 //ばらつき±0.1°
//								const auto x = atan2(-tempvec[0].y, std::hypot(tempvec[0].x, tempvec[0].z)) - deg2rad((float)(-1000 + GetRand(2000)) / 10000.f); //ばらつき±0.1°

								const auto y = atan2(tempvec[0].x, tempvec[0].z);
								const auto x = atan2(-tempvec[0].y, std::hypot(tempvec[0].x, tempvec[0].z));
								a.vec = VGet(cos(x) * sin(y), -sin(x), cos(x) * cos(y));
								//
								p.Gun[guns].useammo++;
								p.Gun[guns].useammo %= ammoc;
								++p.Gun[guns].loadcnt;
								if (guns == 0) {
									set_effect(&p.effcs[ef_fire], p.obj.frame(p.ptr->gunframe[guns] + 1), VSub(p.obj.frame(p.ptr->gunframe[guns] + 1), p.obj.frame(p.ptr->gunframe[guns])));
									p.Gun[guns].fired = 0.5f;
									p.firerad = 0;
									if (p.id == 0) {
										humanparts->start_humananime(2);
										parts->play_sound(1 + GetRand(6));
									}
									PlaySoundMem(p.se[size_t(2) + GetRand(7)].get(), DX_PLAYTYPE_BACK, TRUE);
								}
								else {
									set_effect(&(p.effcs[ef_gun]), p.obj.frame(p.ptr->gunframe[guns] + 1), VGet(0, 0, 0));
									p.Gun[guns].fired = 0.0f;
									PlaySoundMem(p.se[1].get(), DX_PLAYTYPE_BACK, TRUE);
								}
							}
						}
						else {
							++p.Gun[guns].loadcnt;
							if (p.Gun[guns].loadcnt >= p.ptr->reloadtime[guns]) {
								p.Gun[guns].loadcnt = 0;
								if (p.id == 0 && guns == 0)
									parts->play_sound(8 + GetRand(4));
							} //装てん完了
						}
						for (auto& c : p.Gun[guns].Ammo) {
							if (c.flug) {
								c.repos = c.pos;
								c.pos = VAdd(c.pos, VScale(c.vec, c.speed));
								const auto HitPoly = mapparts->get_gnd_hit(c.repos, c.pos);
								if (HitPoly.HitFlag)
									c.pos = HitPoly.HitPosition;
								if (!get_reco(p, player, c, guns)) {
									if (HitPoly.HitFlag) {
										set_effect(&p.effcs[ef_gndhit + guns * (ef_gndhit2 - ef_gndhit)], HitPoly.HitPosition, HitPoly.Normal);
										c.vec = VAdd(c.vec, VScale(HitPoly.Normal, VDot(c.vec, HitPoly.Normal) * -2.0f));
										c.pos = VAdd(HitPoly.HitPosition, VScale(c.vec, 0.01f));
										//c.pene /= 2.0f;
										c.speed /= 2.f;
									}
								}

								c.vec.y += M_GR / 2.0f / f_rates / f_rates;
								c.pene -= 1.0f / f_rates;
								c.speed -= 5.f / f_rates;
								c.cnt++;
								if (c.cnt > (f_rates * 3.f) || c.speed <= 0.f)
									c.flug = false; //3秒で消える
							}
						}
					}
					if (p.recoadd) {
						if (p.recoall < 180) {
							if (p.id == 0 && p.recoall == 0)
								uiparts->set_reco();
							if (p.recoall <= 90)
								p.recoall += 900 / (int)f_rates;
							else
								p.recoall += 180 / (int)f_rates;
						}
						else {
							p.recoall = 0;
							p.recoadd = false;
						}
					}
					if (p.hitadd) {
						if (p.id == 0)
							humanparts->start_humanvoice(0);
						p.hitadd = false;
					}
				}
				/*轍更新*/
				mapparts->set_map_track();
				/*human*/
				humanparts->set_humanmove(player[0], parts->get_view_r());
				/*effect*/
				for (auto& p : player) {
					for (int i = 0; i < efs_user; ++i)
						if (i != ef_smoke2 && i != ef_smoke3)
							set_pos_effect(&p.effcs[i], parts->get_effHandle(i));
					p.effcs[ef_smoke2].handle.SetPos(p.obj.frame(p.ptr->smokeframe[0]));
					//SetTargetLocation(p.effcs[ef_smoke2].handle, p.obj.frame(p.ptr->smokeframe[0]));
					p.effcs[ef_smoke3].handle.SetPos(p.obj.frame(p.ptr->smokeframe[1]));
					//SetTargetLocation(p.effcs[ef_smoke3].handle, p.obj.frame(p.ptr->smokeframe[1]));
				}
				UpdateEffekseer3D();
			}
			/*視点*/
			if (aim.flug) {
				campos = player[0].obj.frame(player[0].ptr->gunframe[0]);
				viewpos = player[0].obj.frame(player[0].ptr->gunframe[0]);
				getdist(&viewpos, VNorm(VSub(player[0].obj.frame(player[0].ptr->gunframe[0] + 1), player[0].obj.frame(player[0].ptr->gunframe[0]))), &aim_r, player[0].ptr->gun_speed[player[0].ammotype], f_rates);
				uppos = player[0].nor;
			}
			else {
				if (!parts->get_in()) {
					if (keyget[19]) {
						campos = player[0].obj.frame(7);
						viewpos = player[0].obj.frame(7 + 1);
						uppos = player[0].nor;
					}
					else {
						campos = VAdd(player[0].pos, VAdd(parts->get_view_pos(), VGet(0, 2, 0)));
						viewpos = VAdd(player[0].pos, VGet(0, 4, 0));
						const auto HitPoly = mapparts->get_gnd_hit(campos, viewpos);
						if (HitPoly.HitFlag)
							campos = HitPoly.HitPosition;
						campos = VAdd(campos, VGet(0, 2, 0));
						uppos = VGet(0, 1, 0);
					}
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
				//進軍
				for (auto& p : player) {
					if (p.id == 0)
						continue;
					DrawLine(x_(p.pos.x), y_(p.pos.z), x_(p.waypos[p.waynow].x), y_(p.waypos[p.waynow].z), c_ff0000, 3);
					for (int i = int(p.waynow); i < waypc - 1; ++i)
						DrawLine(x_(p.waypos[i].x), y_(p.waypos[i].z), x_(p.waypos[i + 1].x), y_(p.waypos[i + 1].z), GetColor(255, 255 * i / waypc, 0), 3);
				}
				for (auto& p : player)
					DrawCircle(x_(p.pos.x), y_(p.pos.z), 5, (p.type == TEAM) ? (p.HP[0] == 0) ? c_008000 : c_00ff00 : (p.HP[0] == 0) ? c_800000 : c_ff0000, TRUE);

				for (auto& p : player) {
					//味方
					if (p.id < teamc) {
						//ステータス
						const auto c = (p.id == waysel)
								   ? (p.HP[0] == 0) ? c_c8c800 : c_ffff00
								   : (p.id == choose)
									 ? c_c0ff00
									 : (p.HP[0] == 0) ? c_808080 : c_00ff00;
						DrawBox(x_r(132), y_r(162 + p.id * 24), x_r(324), y_r(180 + p.id * 24), c, TRUE);
						DrawFormatStringToHandle(x_r(132), y_r(162 + p.id * 24), c_ffffff, parts->get_font(0), " %s", p.ptr->name.c_str());
						//進軍パラメータ
						for (size_t i = 0; i < p.wayselect; i++)
							DrawBox(x_r(348 + i * 12), y_r(162 + p.id * 24), x_r(356 + i * 12), y_r(180 + p.id * 24), c_3232ff, TRUE);
					}
					//敵
					else {
						//ステータス
						DrawBox(x_r(1500), y_r(162 + (p.id - teamc) * 24), x_r(1692), y_r(180 + (p.id - teamc) * 24), (p.HP[0] == 0) ? c_808080 : c_ff0000, TRUE);
						DrawFormatStringToHandle(x_r(1500), y_r(162 + (p.id - teamc) * 24), c_ffffff, parts->get_font(0), " %s", p.ptr->name.c_str());
					}
				}
			}
			/*main*/
			else {
				uiparts->end_way(); //debug0//0
				uiparts->end_way(); //debug1//0
				uiparts->end_way(); //debug2//0

				/*sky*/
				if (!parts->get_in() || aim.flug) {
					SetDrawScreen(skyscreen);
					mapparts->draw_map_sky();
				}
				/*near*/
				SetDrawScreen(mainscreen);
				ClearDrawScreen();
				if (aim.flug)
					setcv(0.06f + rat_r / 2, 2000.0f, campos, viewpos, uppos, 45.0f / rat_r);
				else {
					if (keyget[19])
						setcv(0.1f, 2000.0f, campos, viewpos, uppos, 45.0f);
					else
						setcv(0.16f + parts->get_view_r().z, 2000.0f, campos, viewpos, uppos, 45.0f / rat_r);
				}
				//----------------------------------------------------------
				if (aim.flug) {
					tempvec[0] = player[0].obj.frame(player[0].ptr->gunframe[0]);
					getdist(&tempvec[0], VNorm(VSub(player[0].obj.frame(player[0].ptr->gunframe[0] + 1), tempvec[0])), &aim_r, player[0].ptr->gun_speed[player[0].ammotype], f_rates);
					aims = ConvWorldPosToScreenPos(tempvec[0]);
					aimm = aim_r / 1000.0f * player[0].ptr->gun_speed[player[0].ammotype];
				}
				//pos

				for (auto& p : player) {
					p.iconpos = ConvWorldPosToScreenPos(VAdd(p.pos, VGet(0, VSize(VSub(p.pos, player[0].pos)) / 40 + 6, 0)));

					if (CheckCameraViewClip_Box(VAdd(p.pos, VGet(-5, 0, -5)), VAdd(p.pos, VGet(5, 3, 5))) == TRUE)
						pssort[p.id] = pair(p.id, (float)map_x);
					else
						pssort[p.id] = pair(p.id, VSize(VSub(p.pos, campos)));
				}
				std::sort(pssort.begin(), pssort.end(), [](const pair& x, const pair& y) { return x.second > y.second; });

				//effect
				Effekseer_Sync3DSetting();
				//---------------------------------------------------------------
				if (!parts->get_in() || aim.flug) {
					DrawGraph(0, 0, skyscreen, FALSE); //sky
					//main
					ShadowMap_DrawSetup(mapparts->get_map_shadow_seminear());
					for (auto& tt : pssort) {
						if (tt.second == (float)map_x)
							continue;
						if (tt.second < (10.0f * float(parts->get_shadex()) * parts->get_view_r().z + 20.0f))
							break;
						MV1DrawMesh(player[tt.first].obj.get(), 0);
						for (int i = 1; i < player[tt.first].obj.mesh_num(); ++i)
							if (player[tt.first].HP[i + 4] > 0)
								MV1DrawMesh(player[tt.first].obj.get(), i);
					}
					ShadowMap_DrawEnd();

					ShadowMap_DrawSetup(mapparts->get_map_shadow_near());
					humanparts->draw_human(0);
					for (auto& tt : pssort) {
						if (tt.second > (10.0f * float(parts->get_shadex()) * parts->get_view_r().z + 20.0f))
							continue;
						MV1DrawMesh(player[tt.first].obj.get(), 0);
						for (int i = 1; i < player[tt.first].obj.mesh_num(); ++i) {
							if (i < 3)
								MV1SetFrameTextureAddressTransform(player[tt.first].obj.get(), 0, 0.0, player[tt.first].wheelrad[i], 1.0, 1.0, 0.5, 0.5, 0.0);
							if (i == 3)
								MV1ResetFrameTextureAddressTransform(player[tt.first].obj.get(), 0);
							if (player[tt.first].HP[i + 4] > 0)
								MV1DrawMesh(player[tt.first].obj.get(), i);
						}
					}
					ShadowMap_DrawEnd();
					humanparts->draw_human(0);

					mapparts->ready_shadow();
					mapparts->draw_map_model();
					for (auto& tt : pssort) {
						if (tt.second == (float)map_x)
							continue;
						if (tt.first != 0 || (tt.first == 0 && !aim.flug)) {
							auto& p = player[tt.first];
							MV1DrawMesh(p.obj.get(), 0);
							for (int i = 1; i < p.obj.mesh_num(); ++i) {
								if (i < 3)
									MV1SetFrameTextureAddressTransform(p.obj.get(), 0, 0.0, p.wheelrad[i], 1.0, 1.0, 0.5, 0.5, 0.0);
								if (i == 3)
									MV1ResetFrameTextureAddressTransform(p.obj.get(), 0);
								if (p.HP[i + 4] > 0)
									MV1DrawMesh(p.obj.get(), i);
							}
							//MV1DrawModel(p.colobj.get());/*コリジョンモデルを出すとき*/
							for (int i = 0; i < p.hit.size(); ++i) {
								if (p.hit[i].flug) {
									MV1SetRotationZYAxis(p.hit[i].pic.get(), VSub(p.colobj.frame(11 + 3 * i), p.colobj.frame(9 + 3 * i)), VSub(p.colobj.frame(10 + 3 * i), p.colobj.frame(9 + 3 * i)), 0.f);
									MV1SetPosition(p.hit[i].pic.get(), VAdd(p.colobj.frame(9 + 3 * i), VScale(VSub(p.colobj.frame(10 + 3 * i), p.colobj.frame(9 + 3 * i)), 0.005f)));
									MV1DrawFrame(p.hit[i].pic.get(), p.hit[i].use);
								}
							}
						}
					}
					//grass
					//					if (!aim.flug)
					mapparts->draw_grass();
					//effect
					DrawEffekseer3D();
					//ammo
					uiparts->end_way(); //debug3//0
					SetUseLighting(FALSE);
					SetFogEnable(FALSE);
					for (auto& p : player) {
						for (size_t guns = 0; guns < gunc; ++guns) {
							for (size_t i = 0; i < ammoc; ++i) {
								if (p.Gun[guns].Ammo[i].flug) {
									SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * std::min<float>(1.f, 4.f * p.Gun[guns].Ammo[i].speed / (p.ptr->gun_speed[p.ammotype] / f_rates))));
									DrawCapsule3D(p.Gun[guns].Ammo[i].pos, p.Gun[guns].Ammo[i].repos, p.ptr->ammosize[guns] * (VSize(VSub(p.Gun[guns].Ammo[i].pos, campos)) / 60.f), 4, p.Gun[guns].Ammo[i].color, c_ffffff, TRUE);
								}
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

					uiparts->end_way(); //debug4//0
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
				if (aim.flug)
					uiparts->draw_sight(aims.x, aims.y, rat_r, aimm, parts->get_font(0)); /*照準器*/
				else
					/*アイコン*/
					for (auto& tt : pssort) {
						if (tt.first == 0 || tt.second == (float)map_x)
							continue;
						uiparts->draw_icon(player[tt.first], parts->get_font(0));
					}
				if (keyget[19])
					uiparts->draw_drive(); /*ドライバー視点*/

				uiparts->draw_ui(selfammo, parts->get_view_r().y); /*main*/
			}
			/*debug*/
			//DrawFormatStringToHandle(x_r(18), y_r(1062), c_ffffff, parts->get_font(0), "start-stop(%.2fms)", (float)stop_w / 1000.f);
			uiparts->debug(fps, (float)(GetNowHiPerformanceCount() - waits) / 1000.0f);
			//
			parts->Screen_Flip(waits);
		}
		//delete
		mapparts->delete_map();
		humanparts->delete_human();
		for (auto& p : player) {
			/*エフェクト*/
			for (auto&& e : p.effcs)
				e.handle.Dispose();
			/*Box2D*/
			delete p.playerfix->GetUserData();
			p.playerfix->SetUserData(NULL);
			/**/
			p.obj.Dispose();
			p.colobj.Dispose();
			for (auto& h : p.hit)
				h.pic.Dispose();
			for (auto& s : p.se)
				s.Dispose();
			for (size_t guns = 0; guns < gunc; ++guns)
				p.Gun[guns].Ammo.clear();
			p.Springs.clear();
			p.HP.clear();
			p.hitssort.clear();

			for (size_t LR = 0; LR < 2; ++LR) {
				for (int32 i = 0; i < p.Foot[LR].size(); ++i) {
					delete p.Foot[LR][i].f_playerfix->GetUserData();
					p.Foot[LR][i].f_playerfix->SetUserData(NULL);
				}
				//delete p.foot[LR];
			}
		}
		pssort.clear();
		player.clear();
	} while (!out);
	/*終了*/
	return 0;
}
