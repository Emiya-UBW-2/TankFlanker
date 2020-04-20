#define NOMINMAX
#include "define.h"
#include "useful.h"
#include "make_thread.hpp"
//#include <algorithm>
//#include <memory>
/*main*/
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	/*変数*/
	input in{ 0 };
	output out{ 0 };

	bool camflug;
	size_t camcnt[2];
	size_t output = 0;

	uint8_t way[2];	     /*マウストリガー*/
	uint8_t selfammo[2]; /*弾種変更キー*/

	VECTOR_ref aimpos;	     /*照準器座標確保用*/
	float aimdist{ 0.f };	     /*照準距離確保用*/
	float ratio, rat_r, aim_r;   /*カメラ倍率、実倍率、距離*/
	float rat_aim;		     /*照準視点倍率*/
	size_t waysel, choose;	     /*指揮視点　指揮車両、マウス選択*/
	VECTOR_ref cam, view, upvec; /*カメラ*/
	size_t mapc;		     /*mapselect*/
	//init
	auto parts = std::make_unique<Myclass>();							       /*汎用クラス*/
	float frate = parts->get_f_rate();								       /*基準フレームレート*/
	auto humanparts = std::make_unique<HUMANS>(parts->get_usegrab(), frate);			       /*車内関係*/
	auto mapparts = std::make_unique<MAPS>(parts->get_gndx(), parts->get_drawdist(), parts->get_shadex()); /*地形、ステージ関係*/
	auto uiparts = std::make_unique<UIS>();								       /*UI*/
	auto soldierparts = std::make_unique<SOLDIERS>(frate);						       /*歩兵*/
	auto threadparts = std::make_unique<ThreadClass>();						       /*演算クラス(todo:移植)*/
	//
	//parts->autoset_option();//オプション自動セット
	//parts->write_option(); //オプション書き込み
	//load
	parts->set_fonts(18);
	SetUseASyncLoadFlag(TRUE);
	//hit
	const auto hit_mod = MV1ModelHandle::Load("data/hit/hit.mv1");
	//screen
	int minimap = MakeScreen(dispx, dispy, FALSE);			     /*ミニマップ*/
	int skyscreen = MakeScreen(dispx, dispy, FALSE);		     /*空*/
	int mainscreen = MakeScreen(dispx, dispy, FALSE);		     /*遠景*/
	int HighBrightScreen = MakeScreen(dispx, dispy, FALSE);		     /*エフェクト*/
	int GaussScreen = MakeScreen(dispx / EXTEND, dispy / EXTEND, FALSE); /*エフェクト*/
	SetUseASyncLoadFlag(FALSE);
	uiparts->draw_load(); //
	if (!uiparts->draw_title())
		return 0;
	if (!parts->set_veh())
		return -1;
	/*物理開始*/
	auto world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f)); /* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
	//color
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
	const auto c_ffff96 = GetColor(255, 255, 150);
	const auto c_ffc896 = GetColor(255, 200, 150);
	//これ以降繰り返しロード
	threadparts->thread_start(in, out);
 	do {
		std::string stage = "data_0";
		{
			auto font18 = FontHandle::Create(x_r(18), y_r(18 / 3), DX_FONTTYPE_ANTIALIASING_EDGE);
			const auto mdata = FileRead_open(("stage/" + stage + "/story.txt").c_str(), FALSE); /*ステージ情報*/
			char mstr[256];									    /*tank*/
			char ostr[256];									    /*tank*/
			int i = 0, j = 0, k, length = 0;
			auto se_type = SoundHandle::Load("data/audio/se/type.wav");
			auto se_next = SoundHandle::Load("data/audio/se/next.wav");
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();
			parts->Screen_Flip(GetNowHiPerformanceCount());
			while (ProcessMessage() == 0) {
				if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0 || FileRead_gets(mstr, 256, mdata) == -1)
					break;
				for (length = 0; mstr[length] != '\0'; length++) {}
				j = 0;
				while (ProcessMessage() == 0 && j <= length) {
					const auto waits = GetNowHiPerformanceCount();

					if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0)
						break;
					for (k = 0; k < length; k++) {
						ostr[k] = mstr[k];
					}
					ostr[j] = '\0';
					SetDrawScreen(DX_SCREEN_BACK);
					font18.DrawString(x_r(960 - (18 / 3) * (80)), y_r(540 - 400 + 18 * i), ostr, GetColor(255, 255, 255));
					if (j % 2 == 0)
						PlaySoundMem(se_type.get(), DX_PLAYTYPE_BACK, TRUE);
					WaitTimer(75);

					font18.DrawString(x_r(960) - font18.GetDrawWidth("クリックしてスキップ") / 2, y_r(969), "クリックしてスキップ", GetColor(255, 255, 255));

					parts->Screen_Flip(waits);
					j++;
				}
				if (j != length)
					break;
				PlaySoundMem(se_next.get(), DX_PLAYTYPE_NORMAL, TRUE);
				i++;
			}

			FileRead_close(mdata);
			font18.Dispose();
			se_type.Dispose();
			se_next.Dispose();
		}
		draw_black();

		{
			const auto mdata = FileRead_open(("stage/" + stage + "/main.txt").c_str(), FALSE); /*ステージ情報*/
			char mstr[64];									   /*tank*/
			FileRead_gets(mstr, 64, mdata);
			mapc = std::stoi(getright(mstr));
			FileRead_close(mdata);
		}
		const size_t teamc = count_team(stage);	  /*味方数*/
		const size_t enemyc = count_enemy(stage); /*敵数*/
		out.player.resize(teamc + enemyc);
		out.pssort.resize(teamc + enemyc);
		for (size_t p_cnt = 0; p_cnt < teamc + enemyc; ++p_cnt)
			out.player[p_cnt].id = p_cnt;
		//設定
		size_t mt = 0;
		for (auto&& p : out.player) {
			int mdata = -1;
			if (p.id < teamc) {
				p.type = TEAM;
				mdata = FileRead_open(("stage/" + stage + "/team/" + std::to_string(p.id) + ".txt").c_str(), FALSE);
			}
			else {
				p.type = ENEMY;
				mdata = FileRead_open(("stage/" + stage + "/enemy/" + std::to_string(p.id - teamc) + ".txt").c_str(), FALSE);
			}
			{
				char mstr[64]; /*tank*/
				FileRead_gets(mstr, 64, mdata);
				p.ptr = parts->get_vehicle(size_t(std::stoi(getright(mstr))));
				if (p.id == 0)
					mt = size_t(std::stoi(getright(mstr)));
				FileRead_gets(mstr, 64, mdata);
				p.yrad = deg2rad(std::stoi(getright(mstr)));
				FileRead_gets(mstr, 64, mdata);
				float xpp = float(std::stoi(getright(mstr)));
				FileRead_gets(mstr, 64, mdata);
				float zpp = float(std::stoi(getright(mstr)));
				p.mine.pos = VGet(xpp, 0.0f, zpp);
				for (size_t i = 0; i < waypc; i++) {
					FileRead_gets(mstr, 64, mdata);
					xpp = float(std::stoi(getright(mstr)));
					FileRead_gets(mstr, 64, mdata);
					zpp = float(std::stoi(getright(mstr)));
					p.waypos[i] = VGet(xpp, 0.0f, zpp);
					FileRead_gets(mstr, 64, mdata);
					p.wayspd[i] = int8_t(std::stoi(getright(mstr)));
				}
				FileRead_close(mdata);
			}

			p.setammo[0] = 40;
			p.setammo[1] = 5;
			p.setammo[2] = 20;
		}
		do {
			out.ends = false;
			{
				const int m = parts->window_choosev(mt); /*out.player指定*/
				if (m == -1)
					return 0;
				out.player[0].ptr = parts->get_vehicle(size_t(m));
				out.player[0].setammo[0] = 40;
				out.player[0].setammo[1] = 5;
				out.player[0].setammo[2] = 20;
			}
			/*UI*/
			uiparts->set_state(&out.player[0]);
			/*load*/
			SetUseASyncLoadFlag(TRUE);
			SetCreate3DSoundFlag(TRUE);
			for (auto&& p : out.player) {
				p.obj = p.ptr->model.Duplicate();
				p.colobj = p.ptr->colmodel.Duplicate();
				for (auto&& h : p.hit)
					h.pic = hit_mod.Duplicate();
				size_t i = 0;
				p.se.emplace_back(SoundHandle::Load("data/audio/se/engine/0.wav"));
				i++;
				p.se.emplace_back(SoundHandle::Load("data/audio/se/fire/gun.wav"));
				i++;
				for (; i < 10; ++i)
					p.se.emplace_back(SoundHandle::Load("data/audio/se/fire/" + std::to_string(i - 2) + ".wav"));
				for (; i < 27; ++i)
					p.se.emplace_back(SoundHandle::Load("data/audio/se/ricochet/" + std::to_string(i - 10) + ".wav"));
				for (; i < 29; ++i)
					p.se.emplace_back(SoundHandle::Load("data/audio/se/engine/o" + std::to_string(i - 27) + ".wav"));
				for (; i < 31; ++i)
					p.se.emplace_back(SoundHandle::Load("data/audio/se/hit_enemy/" + std::to_string(i - 29) + ".wav"));

				p.se.emplace_back(SoundHandle::Load("data/audio/se/turret/" + std::to_string(0) + ".wav"));
				i++;
			}
			SetCreate3DSoundFlag(FALSE);
			SetUseASyncLoadFlag(FALSE);
			mapparts->set_map_readyb(mapc);
			/*human*/
			if (!humanparts->set_humans(out.player[0].ptr->inmodel)) { //
				for (auto&& p : out.player) {
					SetASyncLoadFinishDeleteFlag(p.obj.get());
					SetASyncLoadFinishDeleteFlag(p.colobj.get());
					for (auto&& h : p.hit)
						SetASyncLoadFinishDeleteFlag(h.pic.get());
					for (size_t i = 0; i < p.se.size(); ++i)
						SetASyncLoadFinishDeleteFlag(p.se[i].get());
				}
				mapparts->set_map_cancelb();
				//SetASyncLoadFinishDeleteFlag(0);
				out.ends = true;
			}

			uiparts->draw_load(); //
		} while (out.ends);
		out.ends = false;
		/*map*/
		if (!mapparts->set_map_ready())
			break;
		//players
		for (auto&& p : out.player) {
			//色調
			for (int i = 0; i < p.obj.material_num(); ++i) {
				MV1SetMaterialSpcColor(p.obj.get(), i, GetColorF(0.85f, 0.82f, 0.78f, 0.5f));
				MV1SetMaterialSpcPower(p.obj.get(), i, 5.0f);
			}
			MV1SetMaterialDrawAlphaTestAll(p.obj.get(), TRUE, DX_CMP_GREATER, 128);
			//リセット
			p.gear = 0;
			//cpu
			p.atkf.reset();
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
				for (auto& c : p.Gun[guns].Ammo)
					c.color = (p.type == TEAM) ? c_ffff96 : c_ffc896;
			}
			//HP
			p.HP.resize(p.ptr->colmodel.mesh_num());
			/*0123は装甲部分なので詰め込む*/
			p.HP[0] = 2; //life
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
				auto HitPoly = mapparts->get_gnd_hit(p.mine.pos + VGet(0.0f, mapparts->get_minsize().y(), 0.0f), p.mine.pos + VGet(0.0f, mapparts->get_maxsize().y(), 0.0f));
				if (HitPoly.HitFlag)
					p.mine.pos = VGet(p.mine.pos.x(), HitPoly.HitPosition.y, p.mine.pos.z());
				for (auto&& w : p.waypos) {
					HitPoly = mapparts->get_gnd_hit(w + VGet(0.0f, mapparts->get_minsize().y(), 0.0f), w + VGet(0.0f, mapparts->get_maxsize().y(), 0.0f));
					if (HitPoly.HitFlag)
						w = HitPoly.HitPosition;
				}
			}
			//
			p.hitres.resize(p.ptr->colmodel.mesh_num());
			p.gndsmkeffcs.resize(p.ptr->wheelframe.size());
			p.gndsmksize.resize(p.ptr->wheelframe.size());
			p.recorad = 180;
			p.firerad = 180;
			p.hitid = -1;
			p.nor = VGet(0, 1.f, 0);
			const auto mat = MMult(
			    MGetRotY(-p.yrad),
			    MGetRotVec2(VGet(0, 1.f, 0), p.nor.get()));
			p.zvec = VTransform(VGet(0, 0, -1.f), mat);
			p.ps_m = MMult(mat, p.mine.pos.Mtrans());
			p.recovec = VGet(0, 0, 1.f);
			p.lost_sec = -1;
		}
		//
		humanparts->set_state(&out.player[0]);
		//物理set
		for (auto&& p : out.player) {
			b2PolygonShape dynamicBox; /*ダイナミックボディに別のボックスシェイプを定義します。*/
			dynamicBox.SetAsBox(
			    (p.ptr->min.x() - p.ptr->max.x()) / 2,
			    (p.ptr->min.z() - p.ptr->max.z()) / 2,
			    b2Vec2(
				(p.ptr->max.x() + p.ptr->min.x()) / 2,
				(p.ptr->max.z() + p.ptr->min.z()) / 2),
			    0.f);						    /**/
			b2FixtureDef fixtureDef;				    /*動的ボディフィクスチャを定義します*/
			fixtureDef.shape = &dynamicBox;				    /**/
			fixtureDef.density = 1.0f;				    /*ボックス密度をゼロ以外に設定すると、動的になります*/
			fixtureDef.friction = 0.3f;				    /*デフォルトの摩擦をオーバーライドします*/
			b2BodyDef bodyDef;					    /*ダイナミックボディを定義します。その位置を設定し、ボディファクトリを呼び出します*/
			bodyDef.type = b2_dynamicBody;				    /**/
			bodyDef.position.Set(p.mine.pos.x(), p.mine.pos.z());	    /**/
			bodyDef.angle = p.yrad;					    /**/
			p.mine.body.reset(world->CreateBody(&bodyDef));		    /**/
			p.mine.playerfix = p.mine.body->CreateFixture(&fixtureDef); /*シェイプをボディに追加します*/
			/* 剛体を保持およびシミュレートするワールドオブジェクトを構築*/
			for (size_t i = 0; i < 2; ++i)
				p.foot[i].LR = (i == 0) ? 1 : -1;

			for (auto& f : p.foot) {
				f.world = new b2World(b2Vec2(0.0f, 0.0f));
				{
					b2Body* ground = NULL;
					{
						b2BodyDef bd;
						ground = f.world->CreateBody(&bd);
						b2EdgeShape shape;
						shape.Set(b2Vec2(-40.0f, -10.0f), b2Vec2(40.0f, -10.0f));
						ground->CreateFixture(&shape, 0.0f);
					}
					b2Body* prevBody = ground;
					size_t i = 0;
					VECTOR_ref vects;
					for (auto& w : p.ptr->upsizeframe) {
						vects = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w));
						if (vects.x() * f.LR > 0) {
							f.Foot.resize(i + 1);
							b2PolygonShape f_dynamicBox; /*ダイナミックボディに別のボックスシェイプを定義します。*/
							f_dynamicBox.SetAsBox(0.1f, 0.125f);
							b2FixtureDef f_fixtureDef;
							f_fixtureDef.shape = &f_dynamicBox;
							f_fixtureDef.density = 20.0f;
							f_fixtureDef.friction = 0.2f;
							b2BodyDef f_bodyDef;
							f_bodyDef.type = b2_dynamicBody;
							f_bodyDef.position.Set(vects.z(), vects.y());
							f.Foot[i].body.reset(f.world->CreateBody(&f_bodyDef));
							f.Foot[i].playerfix = f.Foot[i].body->CreateFixture(&f_fixtureDef); // シェイプをボディに追加します。
							f.f_jointDef.Initialize(prevBody, f.Foot[i].body.get(), b2Vec2(vects.z(), vects.y()));
							f.world->CreateJoint(&f.f_jointDef);
							prevBody = f.Foot[i].body.get();
							++i;
						}
					}
					f.f_jointDef.Initialize(prevBody, ground, b2Vec2(vects.z(), vects.y()));
					f.world->CreateJoint(&f.f_jointDef);
					i = 0;
					for (auto& w : p.ptr->wheelframe) {
						vects = VECTOR_ref(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w + 1))) +
							VECTOR_ref(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w)));
						if (vects.x() * f.LR > 0) {
							f.Wheel.resize(i + 1);
							b2CircleShape shape;
							shape.m_radius = VSize(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w + 1))) - 0.1f;
							b2FixtureDef fw_fixtureDef;
							fw_fixtureDef.shape = &shape;
							fw_fixtureDef.density = 1.0f;
							b2BodyDef fw_bodyDef;
							fw_bodyDef.type = b2_kinematicBody;
							fw_bodyDef.position.Set(vects.z(), vects.y());
							f.Wheel[i].body.reset(f.world->CreateBody(&fw_bodyDef));
							f.Wheel[i].playerfix = f.Wheel[i].body->CreateFixture(&fw_fixtureDef);
							++i;
						}
					}
					i = 0;
					for (auto& w : p.ptr->youdoframe) {
						vects = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w));
						if (vects.x() * f.LR > 0) {
							f.Yudo.resize(i + 1);
							b2CircleShape shape;
							shape.m_radius = 0.05f;
							b2FixtureDef fy_fixtureDef;
							fy_fixtureDef.shape = &shape;
							fy_fixtureDef.density = 1.0f;
							b2BodyDef fy_bodyDef;
							fy_bodyDef.type = b2_kinematicBody;
							fy_bodyDef.position.Set(vects.z(), vects.y());
							f.Yudo[i].body.reset(f.world->CreateBody(&fy_bodyDef));
							f.Yudo[i].playerfix = f.Yudo[i].body->CreateFixture(&fy_fixtureDef);
							++i;
						}
					}
				}
			}
		}
		/*音量調整*/
		humanparts->set_humanvc_vol(unsigned char(255.f * parts->get_se_vol()));
		parts->set_se_vol(unsigned char(128.f * parts->get_se_vol()));
		for (auto&& p : out.player)
			for (auto& s : p.se)
				ChangeVolumeSoundMem(unsigned char(128.f * parts->get_se_vol()), s.get());

		/*メインループ*/
		selfammo[0] = 0;
		selfammo[1] = 0;
		rat_aim = 3.f;	  /*照準視点　倍率*/
		ratio = 1.0f;	  /*カメラ　　倍率*/
		rat_r = ratio;	  /*カメラ　　実倍率*/
		aim_r = 100.0f;	  /*照準視点　距離*/
		waysel = 1;	  /*指揮視点　指揮車両*/
		parts->set_viewrad(VGet(0.f, out.player[0].yrad, 1.f));
		SetCursorPos(x_r(960), y_r(540));
		//
		camflug = false;
		camcnt[0] = 0;
		camcnt[1] = 0;
		choose = (std::numeric_limits<size_t>::max)();
		//sound
		parts->stop_sound();
		PlaySoundMem(out.player[0].se[31].get(), DX_PLAYTYPE_LOOP, TRUE);
		for (auto& p : out.player) {
			p.checkhit = true;
			p.gunrad_rec = VGet(0, 0, 0);
			for (auto& g : p.gndsmkeffcs)
				g.handle = parts->get_gndhitHandle().Play3D();
			p.effcs[ef_smoke2].handle = parts->get_effHandle(ef_smoke2).Play3D();
			p.effcs[ef_smoke3].handle = parts->get_effHandle(ef_smoke2).Play3D();
			PlaySoundMem(p.se[0].get(), DX_PLAYTYPE_LOOP, TRUE);
			PlaySoundMem(p.se[27].get(), DX_PLAYTYPE_LOOP, TRUE);
			PlaySoundMem(p.se[28].get(), DX_PLAYTYPE_LOOP, TRUE);
			size_t i = 0;
			Set3DRadiusSoundMem(50.0f, p.se[i].get()); //engine
			i++;
			Set3DRadiusSoundMem(150.0f, p.se[i].get()); //gun
			i++;
			for (; i < 10; ++i)
				Set3DRadiusSoundMem(600.0f, p.se[i].get()); //shot
			for (; i < 27; ++i)
				Set3DRadiusSoundMem(100.0f, p.se[i].get()); //reco
			for (; i < 29; ++i)
				Set3DRadiusSoundMem(50.0f, p.se[i].get()); //engine2
			for (; i < 31; ++i)
				Set3DRadiusSoundMem(1000.0f, p.se[i].get()); //hit
			for (; i < 32; ++i)
				Set3DRadiusSoundMem(10.0f, p.se[i].get()); //turret
		}
		humanparts->start_humanvoice(1);
		//
		for (size_t i = 0; i < 64; i++) {
			VECTOR_ref tempvec = VGet(-10 * 5 + float(i % 10) * 10, 0, -480.f * ((i < 50) ? 1.f : -1.f) + float(i / 10) * 10 * ((i < 50) ? 1.f : -1.f));
			auto HitPoly = mapparts->get_gnd_hit(tempvec + VGet(0.0f, mapparts->get_minsize().y(), 0.0f), tempvec + VGet(0.0f, mapparts->get_maxsize().y(), 0.0f));
			tempvec = HitPoly.HitPosition;
			soldierparts->set_soldier((i < 50) ? TEAM : ENEMY, tempvec, (i < 50) ? DX_PI_F : 0.f);
		}
		out.starts = true;
		while (ProcessMessage() == 0 && !out.respawn) {
			/*fps*/
			const auto waits = GetNowHiPerformanceCount();
			float fps = GetFPS();
			uiparts->put_way(); //debug
			//メインスレッド依存
			in.keyget2[0] = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
			in.keyget2[1] = (GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;
			in.keyget2[2] = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
			in.keyget2[3] = CheckHitKey(KEY_INPUT_P) != 0;
			if (out.player[0].HP[0] > 0) {
				//指揮
				in.keyget[1] = CheckHitKey(KEY_INPUT_RSHIFT) != 0;
				//照準
				in.keyget[2] = CheckHitKey(KEY_INPUT_LSHIFT) != 0;
				in.keyget[3] = CheckHitKey(KEY_INPUT_V) != 0;
				in.keyget[4] = CheckHitKey(KEY_INPUT_C) != 0;
				in.keyget[5] = false;
				in.keyget[6] = CheckHitKey(KEY_INPUT_Z) != 0;
				//装てん
				in.keyget[7] = CheckHitKey(KEY_INPUT_Q) != 0;
				in.keyget[0] = CheckHitKey(KEY_INPUT_E) != 0;
				//操縦
				in.keyget[9] = CheckHitKey(KEY_INPUT_W) != 0;
				in.keyget[10] = CheckHitKey(KEY_INPUT_S) != 0;
				in.keyget[11] = CheckHitKey(KEY_INPUT_A) != 0;
				in.keyget[12] = CheckHitKey(KEY_INPUT_D) != 0;
				//砲塔
				in.keyget[13] = CheckHitKey(KEY_INPUT_LEFT) != 0;
				in.keyget[14] = CheckHitKey(KEY_INPUT_RIGHT) != 0;
				in.keyget[15] = CheckHitKey(KEY_INPUT_UP) != 0;
				in.keyget[16] = CheckHitKey(KEY_INPUT_DOWN) != 0;
				in.keyget[8] = CheckHitKey(KEY_INPUT_LCONTROL) != 0;
				//射撃
				in.keyget[17] = CheckHitKey(KEY_INPUT_SPACE) != 0;
				in.keyget[18] = CheckHitKey(KEY_INPUT_B) != 0;
				//視点
				in.keyget[19] = CheckHitKey(KEY_INPUT_RCONTROL) != 0;
			}
			if (out.player[0].HP[0] == 0) {
				for (auto& tt : in.keyget)
					tt = false;
			}
			if (GetActiveFlag() == TRUE) {
				SetMouseDispFlag(FALSE);
				/*指揮*/
				if (out.map.flug)
					SetMouseDispFlag(TRUE);
			}
			else {
				SetMouseDispFlag(TRUE);
			}
			//依存しない
			if (GetActiveFlag() == TRUE) {

				/*照準*/
				out.aim.cnt = std::min<uint8_t>(out.aim.cnt + 1, in.keyget[2] ? 2 : 0);
				if (out.aim.cnt == 1) {
					out.aim.flug ^= 1;
					//一回だけ進めたいものはここに
					if (out.aim.flug) {
						ratio = rat_aim;
					}
					else {
						rat_aim = ratio;
						ratio = 1.0f;
					}
					out.map.flug = false;
				}
				/*履帯蘇生*/
				for (auto&& p : out.player) {
					if (p.HP[0] > 0)
						for (size_t i = 5; i <= 6; ++i) {
							if (p.HP[i] == 0) {
								p.footfix[i - 5]++;
								if (p.footfix[i - 5] >= 10 * frate) {
									p.footfix[i - 5] = 0;
									p.HP[i] = 100;
								}
							}
						}
				}
				/*死んだときは無効*/
				if (out.player[0].HP[0] == 0) {
					out.aim.flug = false;
					out.map.flug = false;
				}
				/*弾種交換*/
				if (in.keyget[7]) {
					selfammo[0] = std::min<uint8_t>(selfammo[0] + 1, 2);
					if (selfammo[0] == 1) {
						out.player[0].ammotype = (out.player[0].ammotype == 2) ? 0 : out.player[0].ammotype + 1;
						out.player[0].Gun[0].loadcnt = 1;
					}
				}
				else if (out.player[0].Gun[0].loadcnt == 0) {
					selfammo[0] = 0;
				}
				if (in.keyget[0]) {
					selfammo[1] = std::min<uint8_t>(selfammo[1] + 1, 2);
					if (selfammo[1] == 1) {
						out.player[0].ammotype = (out.player[0].ammotype == 0) ? 2 : out.player[0].ammotype - 1;
						out.player[0].Gun[0].loadcnt = 1;
					}
				}
				else if (out.player[0].Gun[0].loadcnt == 0) {
					selfammo[1] = 0;
				}
				/*指揮*/
				if (out.map.flug) {
					int mousex, mousey; /*mouse*/
					GetMousePoint(&mousex, &mousey);
					choose = (std::numeric_limits<size_t>::max)();
					for (auto&& p : out.player) {
						if (p.id == 0)
							continue;
						if (p.HP[0] > 0)
							if (inm(x_r(132), y_r(162 + p.id * 24), x_r(324), y_r(180 + p.id * 24))) {
								choose = p.id;
								if (in.keyget2[0])
									waysel = choose;
							}
					}
					if (out.player[waysel].HP[0] > 0)
						if (inm(x_r(420), y_r(0), x_r(1500), y_r(1080))) {
							auto& sel = out.player[waysel].wayselect;
							if (sel <= waypc - 1) {
								way[0] = std::min<uint8_t>(way[0] + 1, in.keyget2[0] ? 2 : 0);
								if (way[0] == 1) {
									if (sel == 0)
										out.player[waysel].waynow = 0;
									out.player[waysel].waypos[sel] = VGet(_2x(mousex), 0, _2y(mousey));
									for (size_t i = sel; i < waypc; ++i)
										out.player[waysel].waypos[i] = out.player[waysel].waypos[sel];
									++sel;
								}
							}
							if (sel > 0) {
								way[1] = std::min<uint8_t>(way[1] + 1, in.keyget2[1] ? 2 : 0);
								if (way[1] == 1) {
									out.player[waysel].waynow = std::max<size_t>(out.player[waysel].waynow - 1, 0);
									--sel;
									if (sel >= 1)
										for (size_t i = sel; i < waypc; ++i)
											out.player[waysel].waypos[i] = out.player[waysel].waypos[sel - 1];
									else
										for (size_t i = 0; i < waypc; ++i)
											out.player[waysel].waypos[i] = out.player[waysel].mine.pos;
								}
							}
						}
				}
				/*視界見回し*/
				else {
					if (out.aim.flug) {
						SetCursorPos(x_r(960), y_r(540));
						if (in.keyget[3])
							ratio = std::min<float>(ratio + 2.0f / fps, 10.f);
						if (in.keyget[4])
							ratio = std::max<float>(ratio - 2.0f / fps, 2.f);

						gethitdist(
						    out.player,
						    out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe),
						    (out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe + 1) - out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe)).Norm(),
						    aim_r,
						    out.player[0].ptr->gun_speed[out.player[0].ammotype],
						    frate,
						    mapparts->get_mapobj().get());
					}
					else
						parts->set_view_r(GetMouseWheelRotVol(), out.player[0].HP[0] > 0);
				}
				fpsdiff(rat_r, ratio, 0.1f); /*倍率、測距*/
			}
			//uiparts->end_way(); //debug0//0
			//
			if (true) {
				/*操作、座標系*/
				for (auto&& p : out.player) {
					if (!out.map.flug)
						p.wayselect = 0;
					if (p.HP[0] > 0) {
						p.move = 0;
						/*操作*/
						if (p.id == 0) {
							for (auto i = 0; i < 10; i++)
								p.move |= in.keyget[i + 9] << i;
							/*変速*/
							if (set_shift(p))
								parts->play_sound(0);
						}
						/*CPU操作*/
						else {
							/*移動*/
							if ((p.waypos[p.waynow] - p.mine.pos).size() >= 20.0) {
								p.move |= KEY_GOFLONT;
								VECTOR_ref tempv = (p.waypos[p.waynow] - p.mine.pos).Norm();
								if ((p.zvec.z() * tempv.x() - p.zvec.x() * tempv.z()) < 0)
									p.move |= KEY_GOLEFT_;
								else
									p.move |= KEY_GORIGHT;
							}
							else {
								p.waynow = std::min<size_t>(p.waynow + 1, waypc - 1);
							}
							//*戦闘
							if (!p.atkf.has_value()) {
								if ((p.waypos[p.waynow] - p.mine.pos).size() >= 30.0)
									p.gear = 1;
								else
									p.gear = p.wayspd[p.waynow]; /*変速 */
								/*
								VECTOR_ref tempvec = p.obj.frame(p.ptr->gun_[0].gunframe+1) - p.obj.frame(p.ptr->gun_[0].gunframe); //*元のベクトル
								if (cross2D(p.zvec.x(), p.zvec.z(), tempvec.x(), tempvec.z()) < 0) {
									p.move |= KEY_TURNLFT;
								}
								else {
									p.move |= KEY_TURNRIT;
								}
								*/

								p.selc = false;
								for (auto& t : out.player) {
									if (p.type == t.type || t.HP[0] == 0)
										continue;

									const auto HitPoly = mapparts->get_gnd_hit(p.obj.frame(p.ptr->gun_[0].gunframe), t.obj.frame(GetRand(t.ptr->gun_[0].gunframe)));
									if (!HitPoly.HitFlag && (t.mine.pos - p.mine.pos).size() <= 500.0f) //見つける
									{
										p.selc = true;
										if (p.aim != t.id) { //前狙った敵でないか
											p.atkf = t.id;
											p.aim = 0;
											p.lost_sec = 0;
											break;
										}
									}
								}
								if (p.selc && !p.atkf.has_value()) {
									p.atkf = p.aim;
									p.aim = 0;
									p.lost_sec = 0;
									break;
								}
							}
							else {
								p.gear = 1; /*変速*/
								auto& t = out.player[p.atkf.value()];
								const auto aim_frame = GetRand(int(t.obj.frame_num()) - 1);
								VECTOR_ref tempvec[2];
								tempvec[1] = p.obj.frame(p.ptr->gun_[0].gunframe);	   //*元のベクトル
								tempvec[0] = (t.obj.frame(aim_frame) - tempvec[1]).Norm(); //*向くベクトル
								{
									float tmpf = (t.obj.frame(aim_frame) - tempvec[1]).size();
									float tmpf2;

									getdist(tempvec[1], (p.obj.frame(p.ptr->gun_[0].gunframe + 1) - tempvec[1]).Norm(), tmpf, tmpf2, p.ptr->gun_speed[p.ammotype], fps);
									tempvec[1] = (tempvec[1] - p.obj.frame(p.ptr->gun_[0].gunframe)).Norm();
								}
								if (cross2D(
									std::hypot(tempvec[0].x(), tempvec[0].z()), tempvec[0].y(),
									std::hypot(tempvec[1].x(), tempvec[1].z()), tempvec[1].y()) <= 0)
									p.move |= KEY_TURNUP_;
								else
									p.move |= KEY_TURNDWN;

								if (cross2D(tempvec[0].x(), tempvec[0].z(), tempvec[1].x(), tempvec[1].z()) < 0) {
									p.move |= KEY_TURNLFT;
									//p.move |= KEY_GOLEFT_; //
								}
								else {
									p.move |= KEY_TURNRIT;
									//p.move |= KEY_GORIGHT; //
								}
								const auto HitPoly = mapparts->get_gnd_hit(p.obj.frame(p.ptr->gun_[0].gunframe), t.obj.frame(aim_frame));
								if (!HitPoly.HitFlag) {
									if ((tempvec[1] * tempvec[0]).size() < sin(deg2rad(15))) {
										if (p.Gun[0].loadcnt == 0) {
											p.move &= ~KEY_GOFLONT;
											p.gear = 0; //*変速
											if (p.spd < 5.f / 3.6f / fps) {
												p.move |= KEY_SHOTCAN;
												p.aim++;
											}
										}
										if (GetRand(100) <= 2)
											p.move |= KEY_SHOTGAN;
									}
									p.lost_sec = 0;
								}
								else {
									p.lost_sec++;
								}
								if (p.lost_sec > 5.f * fps) {
									p.lost_sec = -1;
									p.aim = -1;
									p.atkf.reset();
								}

								if (t.HP[0] == 0 || p.aim > 5) {
									p.lost_sec = -1;
									p.aim = int(p.atkf.value());
									p.atkf.reset();
								}
							}
							//ぶつかり防止
							for (auto& t : out.player) {
								if (p.id != t.id && t.HP[0] > 0) {
									if ((t.mine.pos - p.mine.pos).size() <= 10.0) {
										VECTOR_ref tempv = (t.mine.pos - p.mine.pos).Norm();
										if ((p.zvec.z() * tempv.x() - p.zvec.x() * tempv.z()) > 0) {
											p.move |= KEY_GOLEFT_;
											p.move &= ~KEY_GORIGHT;
										}
										else {
											p.move |= KEY_GORIGHT;
											p.move &= ~KEY_GOLEFT_;
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
											out.player[waysel].wayselect = 0;
										}
										else {
											if (VSize(VSub(p.waypos[p.waynow], t.pos)) > 225.f) {
												p.waynow = waypc - 1;
												p.waypos[p.waynow] = p.pos;
												out.player[waysel].wayselect = 0;
											}
										}
									}
								}
								*/
							}
						}
					}
					else
						p.move = KEY_TURNUP_;
					if (p.HP[5] == 0 || p.HP[6] == 0) {
						p.move &= ~KEY_GOFLONT;
						p.move &= ~KEY_GOBACK_;
						if (p.HP[5] == 0 && p.HP[6] == 0) {
							p.move &= ~KEY_GOLEFT_;
							p.move &= ~KEY_GORIGHT;
						}
					}
				} //0.2ms
				/*共通動作*/
				for (auto& p : out.player) {
					if (p.id == 0) {
						set_gunrad(p, rat_r * ((in.keyget[8]) ? 1.f : 3.f));
					}
					else {
						set_gunrad(p, 3.f);
					}
					//
					if ((p.move & KEY_GOFLONT) != 0 && p.gear > 0) {
						if (p.spd >= p.ptr->speed_flont[p.gear - 1]) {
							p.accel *= pow(0.9f, frate / fps);
						}
						else {
							float acu = 0.f;
							if (p.gear > 1) {
								acu = (p.ptr->speed_flont[p.gear - 1] - p.ptr->speed_flont[p.gear - 2]) / frate;
							}
							else {
								acu = p.ptr->speed_flont[0] / frate;
							}
							p.accel = std::min(p.accel + acu / 5.f, acu);
						}
					}
					else if ((p.move & KEY_GOBACK_) != 0 && p.gear < 0) {
						if (p.spd <= p.ptr->speed_back[-p.gear - 1]) {
							p.accel *= pow(0.9f, frate / fps);
						}
						else {
							if (p.gear < 1) {
								const auto acu = (p.ptr->speed_back[-p.gear - 1] - p.ptr->speed_back[-p.gear - 2]) / frate;
								p.accel = std::max(p.accel + acu / 5, acu);
							}
							else {
								const auto acu = p.ptr->speed_back[0] / frate;
								p.accel = std::max(p.accel + acu / 5.f, acu);
							}
						}
					}
					p.spd += p.accel * frate / fps;
					//旋回
					{
						float turn_bias = 0.f;
						if (((p.move & KEY_GORIGHT) != 0 || (p.move & KEY_GOLEFT_) != 0) && (p.HP[5] > 0 || p.HP[6] > 0)) {
							turn_bias = 1.0f;

							if ((p.move & KEY_GOFLONT) != 0 && p.gear > 0)
								turn_bias = abs(p.spd / p.ptr->speed_flont[p.gear - 1]);
							if ((p.move & KEY_GOBACK_) != 0 && p.gear < 0)
								turn_bias = abs(p.spd / p.ptr->speed_back[-p.gear - 1]);
							turn_bias *= ((p.HP[5] > 0) + (p.HP[6] > 0)) / 2.0f; //履帯が切れていると
							if ((p.move & KEY_GORIGHT) != 0)
								turn_bias = -turn_bias;
						}
						if (p.gnd) {
							fpsdiff(p.yadd, p.ptr->vehicle_RD * turn_bias, 0.1f);
						}
					}
					fpsdiff(p.inertiax, p.accel, 0.02f);
					//vec
					{
						float tsize = 0.f;
						if (p.HP[5] == 0 || p.HP[6] == 0) {
							tsize = p.ptr->loc[p.ptr->wheelframe[0]].x() * sin(p.yadd) * ((p.HP[5] == 0) - (p.HP[6] == 0));
						}
						else {
							tsize = p.spd;
						}
						p.vec += (p.zvec.Scale(tsize) - p.vec).Scale(0.05f);
					}
					//
					p.wheelrad[0] += (p.zvec.x() * p.mine.body->GetLinearVelocity().x + p.zvec.z() * p.mine.body->GetLinearVelocity().y) / fps; //
					p.wheelrad[1] = -p.wheelrad[0] * 2 + p.yrad * 5;
					p.wheelrad[2] = -p.wheelrad[0] * 2 - p.yrad * 5;
				} //0.1ms
				/*物理演算*/
				for (auto& p : out.player) {
					p.mine.body->SetLinearVelocity(b2Vec2(p.vec.x(), p.vec.z()));
					p.mine.body->SetAngularVelocity(p.yadd);
					//
					if (p.mine.body->GetPosition().x < mapparts->get_minsize().x() + 50.f)
						p.mine.body->SetTransform(b2Vec2(mapparts->get_minsize().x() + 50.f, p.mine.body->GetPosition().y), p.mine.body->GetAngle());
					if (p.mine.body->GetPosition().x > mapparts->get_maxsize().x() - 50.f)
						p.mine.body->SetTransform(b2Vec2(mapparts->get_maxsize().x() - 50.f, p.mine.body->GetPosition().y), p.mine.body->GetAngle());
					if (p.mine.body->GetPosition().y < mapparts->get_minsize().z() + 50.f)
						p.mine.body->SetTransform(b2Vec2(p.mine.body->GetPosition().x, mapparts->get_minsize().z() + 50.f), p.mine.body->GetAngle());
					if (p.mine.body->GetPosition().y > mapparts->get_maxsize().z() - 50.f)
						p.mine.body->SetTransform(b2Vec2(p.mine.body->GetPosition().x, mapparts->get_maxsize().z() - 50.f), p.mine.body->GetAngle());
					//
				} //1.22ms
				world->Step(1.0f / fps, 1, 1);
				for (auto& p : out.player) {
					p.yrad = p.mine.body->GetAngle();
					float spdrec = p.spd;
					fpsdiff(p.spd, std::hypot(p.mine.body->GetLinearVelocity().x, p.mine.body->GetLinearVelocity().y) * ((p.spd > 0) ? 1.f : -1.f), 0.01f);
					p.accel = p.spd - spdrec;
					b2Vec2 tmpb2 = b2Vec2((M_GR / fps) * ((p.obj.frame(7 + 1) - p.obj.frame(7)).Norm() % VGet(0, -1.f, 0)), (M_GR / fps) * (p.nor % VGet(0, 1.f, 0)));
					for (auto& f : p.foot) {
						size_t i = 0;
						VECTOR_ref vects;
						for (auto& w : p.ptr->wheelframe) {
							vects = VECTOR_ref(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w + 1))) +
								VECTOR_ref(VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w)));
							if (vects.x() * f.LR > 0)
								f.Wheel[i++].body->SetTransform(b2Vec2(vects.z(), vects.y()), 0.f);
						}
						i = 0;
						for (auto& w : p.ptr->youdoframe) {
							vects = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w));
							if (vects.x() * f.LR > 0)
								f.Yudo[i++].body->SetTransform(b2Vec2(vects.z(), vects.y()), 0.f);
						}
						for (auto& t : f.Foot)
							t.body->SetLinearVelocity(tmpb2); //

						f.world->Step(1.0f / fps, 3, 3);
						for (auto& t : f.Foot)
							t.pos = VGet(t.pos.x(), t.body->GetPosition().y, t.body->GetPosition().x);
					}
				} //0.5ms
				/*砲撃その他*/
				//uiparts->end_way(); //debug1//0
				for (auto& p : out.player) {
					//地形判定
					{
						const auto HitPoly = mapparts->get_gnd_hit(p.mine.pos + VGet(0.0f, 2.0f, 0.0f), p.mine.pos + VGet(0.0f, -0.05f, 0.0f));
						p.gnd = HitPoly.HitFlag;
						if (HitPoly.HitFlag) {
							auto rep = p.mine.pos;
							p.mine.pos = VGet(
							    p.mine.body->GetPosition().x,
							    HitPoly.HitPosition.y,
							    p.mine.body->GetPosition().y);

							p.yace = (p.mine.pos - rep).y();

							mapparts->set_normal(p.nor, p.ps_n, p.mine.pos, frate, fps);
							/*0.1km/h以内の時かキーを押していないときに減速*/
							if (((0.1f / 3.6f) / fps) < -p.spd && (p.move & KEY_GOBACK_) == 0 ||					//バック
							    ((0.1f / 3.6f) / fps) < p.spd && (p.move & KEY_GOFLONT) == 0 ||					//前進
							    ((0.1f / 3.6f) / fps) > abs(p.spd) && (p.move & KEY_GOBACK_) == 0 && (p.move & KEY_GOFLONT) == 0) { //停止
								p.accel = -p.spd * (1.f - pow(0.95f, frate / fps));
							}
							/*turn*/
							if ((p.move & KEY_GOLEFT_) == 0 && (p.move & KEY_GORIGHT) == 0)
								p.yadd *= pow(0.9f, frate / fps);
							/*track*/
							mapparts->draw_map_track(p);
						}
						else {
							p.yadd *= pow(0.95f, frate / fps);
							p.mine.pos = VGet(p.mine.body->GetPosition().x, p.mine.pos.y() + p.yace, p.mine.body->GetPosition().y);
							p.yace += m_ac(fps);
						}
					}
					//サウンド
					ChangeVolumeSoundMem(int(std::min<float>(64.f * abs(p.spd / p.ptr->speed_flont[0]), 64.f) * parts->get_se_vol()), p.se[0].get());
					for (size_t i = 27; i < 29; ++i)
						ChangeVolumeSoundMem(int(std::min<float>(32.f + 32.f * abs(p.spd / p.ptr->speed_flont[0]), 64.f) * parts->get_se_vol()), p.se[i].get());
					ChangeVolumeSoundMem(int(128.f * p.gun_turn * parts->get_se_vol()), p.se[31].get());
					fpsdiff(p.gun_turn, (p.gunrad_rec - p.gunrad).size() / p.ptr->gun_RD, 0.05f);
					p.gunrad_rec = p.gunrad;

					for (auto& s : p.se)
						if (CheckSoundMem(s.get()) == 1)
							Set3DPositionSoundMem(p.mine.pos.get(), s.get());
					//tree判定
					mapparts->set_hitplayer(p.mine.pos);
				}
				for (auto& p : out.player) {
					/*車体行列*/
					const auto mat = MMult(
					    MMult(
						MMult(
						    MGetRotAxis(VGet(cos(p.gunrad.x()), 0, -sin(p.gunrad.x())), sin(deg2rad(p.firerad)) * deg2rad(p.ptr->gun_[0].ammosize * 1000 / 75 * 5)),
						    MGetRotAxis(p.recovec.get(), sin(deg2rad(p.recorad)) * deg2rad(5))),
						MGetRotX(atan(p.inertiax))),
					    MMult(MGetRotY(-p.yrad), p.ps_n));
					/*砲塔移動*/
					p.ps_m = MMult(mat, p.mine.pos.Mtrans());
					/*砲塔行列*/
					p.ps_t = MMult(MGetRotY(p.gunrad.x()), p.ptr->loc[p.ptr->turretframe].Mtrans());
					//車体前方
					p.zvec = VTransform(VGet(0, 0, -1.f), mat);
				}
				for (auto& p : out.player) {
					//all
					MV1SetMatrix(p.obj.get(), p.ps_m);
					MV1SetFrameUserLocalMatrix(p.obj.get(), p.ptr->turretframe, p.ps_t);
					MV1SetMatrix(p.colobj.get(), p.ps_m);
					MV1SetFrameUserLocalMatrix(p.colobj.get(), 2, p.ps_t);
					MV1SetFrameUserLocalMatrix(p.colobj.get(), 3, MMult(MMult(MGetRotX(p.gunrad.y()), (p.ptr->loc[p.ptr->gun_[0].gunframe] - p.ptr->loc[p.ptr->turretframe]).Mtrans()), p.ps_t));
					MV1SetFrameUserLocalMatrix(p.colobj.get(), 3 + 1, (p.ptr->loc[p.ptr->gun_[0].gunframe + 1] - p.ptr->loc[p.ptr->gun_[0].gunframe] + VGet(0, 0, p.Gun[0].fired)).Mtrans());
					for (int guns = 0; guns < gunc; ++guns) {
						auto& g = p.ptr->gun_[guns];
						MV1SetFrameUserLocalMatrix(p.obj.get(), g.gunframe, MMult(MMult(MGetRotX(p.gunrad.y()), (p.ptr->loc[g.gunframe] - p.ptr->loc[p.ptr->turretframe]).Mtrans()), p.ps_t));
						MV1SetFrameUserLocalMatrix(p.obj.get(), g.gunframe + 1, (p.ptr->loc[g.gunframe + 1] - p.ptr->loc[g.gunframe] + VGet(0, 0, p.Gun[guns].fired)).Mtrans());
					}
					for (size_t i = 0; i < p.ptr->wheelframe.size(); i++) {
						auto& w = p.ptr->wheelframe[i];
						MV1ResetFrameUserLocalMatrix(p.obj.get(), w);
						const auto HitPoly = mapparts->get_gnd_hit(p.obj.frame(w) + p.nor.Scale(1.0f), p.obj.frame(w) + p.nor.Scale(-0.2f));
						if (HitPoly.HitFlag) {
							p.Springs[w] = std::min<float>(p.Springs[w] + 1.0f / fps, 1.0f - (VECTOR_ref(HitPoly.HitPosition) - p.obj.frame(w) - p.nor.Scale(1.0f)).size());
							fpsdiff(p.gndsmksize[i], 0.01f + abs(p.spd / p.ptr->speed_flont[3]), 0.1f);
						}
						else {
							p.Springs[w] = std::max<float>(p.Springs[w] - 0.2f / fps, -0.2f);
							fpsdiff(p.gndsmksize[i], 0.01f, 0.1f);
						}
						MV1SetFrameUserLocalMatrix(p.obj.get(), w, (p.ptr->loc[w] + p.nor.Scale(p.Springs[w])).Mtrans());
						MV1SetFrameUserLocalMatrix(p.obj.get(), w + 1, MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[w + 1].x()) + 1]), (p.ptr->loc[w + 1] - p.ptr->loc[w]).Mtrans()));
					}
					for (auto& w : p.ptr->youdoframe)
						MV1SetFrameUserLocalMatrix(p.obj.get(), w, MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[w].x()) + 1]), p.ptr->loc[w].Mtrans()));
					for (auto& w : p.ptr->kidoframe)
						MV1SetFrameUserLocalMatrix(p.obj.get(), w, MMult(MGetRotX(p.wheelrad[signbit(p.ptr->loc[w].x()) + 1]), p.ptr->loc[w].Mtrans()));
					for (auto& f : p.foot) {
						size_t i = 0;
						for (auto& w : p.ptr->upsizeframe) {
							float xw = VTransform(VGet(0, 0, 0), MV1GetFrameLocalMatrix(p.obj.get(), w)).x;
							if (xw * f.LR > 0) {
								f.Foot[i].pos = VGet(xw, f.Foot[i].pos.y(), f.Foot[i].pos.z());
								MV1SetFrameUserLocalMatrix(p.obj.get(), w, f.Foot[i].pos.Mtrans());
								++i;
							}
						}
					}
				} //1.5ms
				/*collition*/
				for (auto& p : out.player) {
					if (p.checkhit) {
						MV1SetMatrix(p.colobj.get(), MMult(p.ps_m, MGetTranslate(VGet(0, mapparts->get_minsize().y(), 0))));
						for (int i = 0; i < p.ptr->colmodel.mesh_num(); ++i)
							MV1RefreshCollInfo(p.colobj.get(), -1, i);
						p.checkhit = false;
						MV1SetMatrix(p.colobj.get(), p.ps_m);
					}
				}
				for (auto& p : out.player) {
					/*反動*/
					if (p.firerad < 180) {
						if (p.firerad <= 90)
							p.firerad += 900 / (int)fps;
						else
							p.firerad += 180 / (int)fps;
					}
					else {
						p.firerad = 180;
					}

					for (size_t guns = 0; guns < gunc; ++guns) {
						auto& g = p.ptr->gun_[guns];
						if (p.Gun[guns].fired >= 0.01f)
							p.Gun[guns].fired *= pow(0.95f, frate / fps);

						if (p.setammo[p.ammotype] == 0 && guns == 0) {
							++p.ammotype %= 3;
							p.Gun[0].loadcnt = 1;
						}
						if (p.Gun[guns].loadcnt == 0) {
							if (p.setammo[p.ammotype] == 0 && guns == 0) {
								p.move &= ~KEY_SHOTCAN;
							}
							if ((p.move & (KEY_SHOTCAN << guns)) != 0) {
								auto& a = p.Gun[guns].Ammo[p.Gun[guns].useammo];
								a.flug = true;
								if (guns == 0)
									p.setammo[p.ammotype]--;
								a.speed = p.ptr->gun_speed[p.ammotype] / fps;
								a.pene = p.ptr->pene[p.ammotype];
								a.pos = p.obj.frame(g.gunframe);
								a.cnt = 0;

								const auto v = p.obj.frame(g.gunframe + 1) - a.pos;
								const auto y = atan2(v.x(), v.z()) + deg2rad((float)(GetRand(g.accuracy * 2) - g.accuracy) / 10000.f);
								const auto x = atan2(-v.y(), std::hypot(v.x(), v.z())) - deg2rad((float)(GetRand(g.accuracy * 2) - g.accuracy) / 10000.f); //
								a.vec = VGet(cos(x) * sin(y), -sin(x), cos(x) * cos(y));
								//
								++p.Gun[guns].useammo;
								p.Gun[guns].useammo %= ammoc;
								++p.Gun[guns].loadcnt;
								if (guns == 0) {
									do {
										if (p.id == 0 && out.aim.flug)
											continue;
										set_effect(&p.effcs[ef_fire], p.obj.frame(g.gunframe + 1).get(), (p.obj.frame(g.gunframe + 1) - p.obj.frame(g.gunframe)).get());
									} while (false);

									p.Gun[guns].fired = 0.5f;
									p.firerad = 0;
									if (p.id == 0) {
										humanparts->start_humananime(2);
										parts->play_sound(1 + GetRand(6));
									}
									PlaySoundMem(p.se[size_t(2) + GetRand(7)].get(), DX_PLAYTYPE_BACK, TRUE);
								}
								else {
									set_effect(&(p.effcs[ef_gun]), p.obj.frame(g.gunframe + 1).get(), VGet(0, 0, 0));
									p.Gun[guns].fired = 0.0f;
									PlaySoundMem(p.se[1].get(), DX_PLAYTYPE_BACK, TRUE);
								}
							}
						}
						else {
							++p.Gun[guns].loadcnt;
							if (p.Gun[guns].loadcnt >= g.reloadtime) {
								p.Gun[guns].loadcnt = 0;
								if (p.id == 0 && guns == 0)
									parts->play_sound(8 + GetRand(4));
							} //装てん完了
						}
						for (auto& c : p.Gun[guns].Ammo)
							if (c.flug) {
								c.repos = c.pos;
								c.pos += c.vec.Scale(c.speed);

								for (auto& t : out.player) {
									if (p.id == t.id || (Segment_Point_MinLength(c.pos.get(), c.repos.get(), t.mine.pos.get()) > c.speed) || t.checkhit)
										continue;
									for (int i = 0; i < t.ptr->colmodel.mesh_num(); ++i)
										MV1RefreshCollInfo(t.colobj.get(), -1, i);
									t.checkhit = true;
								}

								const auto HitPoly = mapparts->get_gnd_hit(c.repos, c.pos);
								if (HitPoly.HitFlag)
									c.pos = HitPoly.HitPosition;
								if (!get_reco(p, out.player, c, guns))
									if (HitPoly.HitFlag) {
										soldierparts->set_hit(c.pos, c.repos);
										set_effect(&p.effcs[ef_gndhit + guns * (ef_gndhit2 - ef_gndhit)], HitPoly.HitPosition, HitPoly.Normal);
										c.vec += VScale(HitPoly.Normal, (c.vec % HitPoly.Normal) * -2.0f);
										c.pos = c.vec.Scale(0.01f) + HitPoly.HitPosition;
										//c.pene /= 2.0f;
										c.speed /= 2.f;
									}

								c.vec = VGet(c.vec.x(), c.vec.y() + m_ac(fps), c.vec.z());
								c.pene -= 1.0f / fps;
								c.speed -= 5.f / fps;
								c.cnt++;
								if (c.cnt > (fps * 3.f) || c.speed <= 0.f)
									c.flug = false; //3秒で消える
							}
					}
					if (p.recorad < 180) {
						if (p.id == 0 && p.recorad == 0)
							uiparts->set_reco();
						if (p.recorad <= 90)
							p.recorad += 900 / (int)fps;
						else
							p.recorad += 180 / (int)fps;
					}
					else {
						p.recorad = 180;
					}

					if (p.hitadd) {
						if (p.hitid == 0)
							uiparts->set_damage();
						if (out.player[p.hitid].HP[0] == 0) {
							if (p.id == 0)
								humanparts->start_humanvoice(0);
							out.player[p.hitid].effcs[ef_smoke1].handle = parts->get_effHandle(ef_smoke1).Play3D();
						}
						p.hitadd = false;
					}
				} //0.1~1.0ms
				/*轍更新*/
				mapparts->set_map_track(); //0.0ms
				/*human*/
				humanparts->set_humanmove(parts->get_view_r(), frate, fps); //0.02
				/*人の移動*/
				soldierparts->set_soldiermove(mapparts->get_mapobj().get(), out.player); //1.06ms
				/*effect*/
				for (auto& p : out.player) {
					for (int i = 0; i < efs_user; ++i)
						if (i != ef_smoke1 && i != ef_smoke2 && i != ef_smoke3)
							set_pos_effect(&p.effcs[i], parts->get_effHandle(i));

					for (size_t i = 0; i < p.ptr->wheelframe.size(); i++) {
						p.gndsmkeffcs[i].handle.SetPos(p.obj.frame(p.ptr->wheelframe[i]));
						p.gndsmkeffcs[i].handle.SetScale(p.gndsmksize[i]);
						//p.gndsmkeffcs[i].handle.SetColor
					}
					p.effcs[ef_smoke1].handle.SetPos(p.obj.frame(p.ptr->engineframe));
					p.effcs[ef_smoke2].handle.SetPos(p.obj.frame(p.ptr->smokeframe[0]));
					p.effcs[ef_smoke3].handle.SetPos(p.obj.frame(p.ptr->smokeframe[1]));
				}		     //0.16ms
				UpdateEffekseer3D(); //2.0ms
			}
			/*視点*/
			if (!camflug) {
				if (out.aim.flug) {
					cam = out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe);
					upvec = out.player[0].nor;

					out.vch.cnt = std::min<uint8_t>(out.vch.cnt + 1, in.keyget[6] ? 2 : 0);
					if (out.vch.cnt == 1)
						out.vch.flug ^= 1;
					if (out.vch.flug) {
						float getdists;
						view = out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe);
						getdist(
						    view,
						    (out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe + 1) - out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe)).Norm(),
						    aim_r,
						    getdists,
						    out.player[0].ptr->gun_speed[out.player[0].ammotype],
						    frate);
					}
					else {
						view = out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe + 1);
					}
				}
				else {
					if (!parts->get_in()) {
						if (in.keyget[19]) {
							//ドライバー視点
							cam = out.player[0].obj.frame(7);
							view = out.player[0].obj.frame(7 + 1);
							upvec = out.player[0].nor;
							/*
							//肩越し
							cam = out.player[0].colobj.frame(6) + out.player[0].nor.Scale(1.f) + (out.player[0].nor * out.player[0].zvec).Norm().Scale(0.5f);
							view = out.player[0].colobj.frame(7) + out.player[0].nor.Scale(1.f) + (out.player[0].nor * out.player[0].zvec).Norm().Scale(0.5f);
							upvec = out.player[0].nor;
							//*/
							//*
							//砲身
							cam = out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe + 1);
							view = out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe);
							cam += (out.player[0].nor * (out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe + 1) - out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe))).Norm().Scale(-0.5f);
							view += (out.player[0].nor * (out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe + 1) - out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe))).Norm().Scale(-0.5f);
							upvec = out.player[0].nor;
							ratio = 0.5f;
							rat_r = ratio;
							//*/
						}
						else {
							ratio = 1.f;
							cam = out.player[0].mine.pos + parts->get_view_pos() + VGet(0, 2, 0);
							view = out.player[0].mine.pos + VGet(0, 4, 0);
							const auto HitPoly = mapparts->get_gnd_hit(cam.get(), view.get());
							if (HitPoly.HitFlag)
								cam = HitPoly.HitPosition;
							cam += VGet(0, 2, 0);
							upvec = VGet(0, 1, 0);
						}
					}
					else {
						view = humanparts->get_campos();
						cam = humanparts->get_neckpos();

						//cam = humanparts->get_campos();
						//view = humanparts->get_neckpos();
						upvec = out.player[0].nor;
					}
				}
			}
			/*
			{
				auto& p = out.player[0];
				int i = int(p.Gun[0].useammo) - 1;
				if (i == -1)
					i = ammoc - 1;
				if (p.Gun[0].Ammo[i].flug) {
					//camflug = true;
					if (out.aim.flug)
						rat_aim = ratio;
				}
				if (camflug) {
					out.aim.flug = false;
					if (p.hitid == -1) {
						ratio = 1.0f;
						rat_r = ratio; //*カメラ　　実倍率

						cam = VGet(
						    std::clamp<float>(p.Gun[0].Ammo[i].repos.x(), mapparts->get_minsize().x(), mapparts->get_maxsize().x()),
						    p.Gun[0].Ammo[i].repos.y(),
						    std::clamp<float>(p.Gun[0].Ammo[i].repos.z(), mapparts->get_minsize().x(), mapparts->get_maxsize().x()));

						view = p.Gun[0].Ammo[i].pos;

						upvec = VGet(0, 1, 0);
						camcnt[0]++;
						if (camcnt[0] >= 3.f * fps || !p.Gun[0].Ammo[i].flug) {
							camflug = false;
							camcnt[0] = 0;
							camcnt[1] = 0;
							p.hitid = -1;
						}
					}
					else {
						camcnt[1]++;
						if (camcnt[1] >= 2.f * fps)
							ratio = 5.0f;
						cam += ((out.player[p.hitid].obj.frame(out.player[p.hitid].ptr->gun_[0].gunframe) + p.Gun[0].Ammo[i].vec.Scale(15.f) + VGet(1.f, 3.f, 1.f)) - cam).Scale(0.1f);
						view += (p.obj.frame(p.ptr->gun_[0].gunframe) - view).Scale(0.1f);
						upvec = VGet(0, 1, 0);
						if (camcnt[1] >= 3.f * fps) {
							ratio = 1.0f;
							rat_r = ratio; //*カメラ　　実倍率
							camflug = false;
							camcnt[0] = 0;
							camcnt[1] = 0;
							p.hitid = -1;
						}
					}
				}
			}
			*/
			if (out.player[0].HP[0] == 0) {
				camflug = true;
			}
			if (camflug) {
				if (camcnt[1] == 0) {
					for (auto& p : out.player) {
						if (p.hitid == 0) {
							output = p.id;
							break;
						}
					}
				}
				auto& p = out.player[output];
				int i;
				i = int(p.Gun[0].useammo) - 1;
				if (i == -1)
					i = ammoc - 1;
				camcnt[1]++;
				if (camcnt[1] >= 2.f * fps)
					ratio = 5.0f;
				cam += ((out.player[p.hitid].obj.frame(out.player[p.hitid].ptr->gun_[0].gunframe) + p.Gun[0].Ammo[i].vec.Scale(15.f) + VGet(1.f, 3.f, 1.f)) - cam).Scale(0.1f);
				view += (p.obj.frame(p.ptr->gun_[0].gunframe) - view).Scale(0.1f);
				upvec = VGet(0, 1, 0);
				if (camcnt[1] >= 5.f * fps) {
					break;
				}
			}
			soldierparts->set_camerapos(cam, view, upvec, rat_r);
			mapparts->set_camerapos(cam, view, upvec, rat_r);
			//uiparts->end_way(); //debug3//0
			/*shadow*/
			if (in.keyget[19]) {
				mapparts->set_map_shadow_near(0.01f);
			}
			else {
				mapparts->set_map_shadow_near(parts->get_view_r().z());
			}
			//uiparts->end_way(); //debug4//0
			//uiparts->end_way(); //debug4//0
			/*draw*/
			/*map*/
			if (out.map.flug) {
				SetDrawScreen(minimap);
				ClearDrawScreen();
				DrawExtendGraph(x_r(420), y_r(0), x_r(1500), y_r(1080), mapparts->get_minmap().get(), FALSE);
				//進軍
				for (auto& p : out.player) {
					if (p.id == 0)
						continue;
					if (p.type == TEAM) {
						DrawLine(x2_(p.mine.pos.x()), y2_(p.mine.pos.z()), x2_(p.waypos[p.waynow].x()), y2_(p.waypos[p.waynow].z()), c_ff0000, 3);
						for (int i = int(p.waynow); i < waypc - 1; ++i)
							DrawLine(x2_(p.waypos[i].x()), y2_(p.waypos[i].z()), x2_(p.waypos[i + 1].x()), y2_(p.waypos[i + 1].z()), GetColor(255, 255 * i / waypc, 0), 3);
					}
					else {
						DrawLine(
						    x2_(p.mine.pos.x()),
						    y2_(p.mine.pos.z()),
						    x2_(p.mine.pos.x() + (p.waypos[p.waynow].x() - p.mine.pos.x()) / 5.f),
						    y2_(p.mine.pos.z() + (p.waypos[p.waynow].z() - p.mine.pos.z()) / 5.f),
						    c_ff0000, 3);
					}
				}
				for (auto& p : out.player)
					DrawCircle(x2_(p.mine.pos.x()), y2_(p.mine.pos.z()), 5, (p.type == TEAM) ? (p.HP[0] == 0) ? c_008000 : c_00ff00 : (p.HP[0] == 0) ? c_800000 : c_ff0000, TRUE);

				for (auto& p : out.player) {
					//味方
					if (p.id < teamc) {
						//ステータス
						const auto c = (p.id == waysel) ? ((p.HP[0] == 0) ? c_c8c800 : c_ffff00) : ((p.id == choose) ? c_c0ff00 : ((p.HP[0] == 0) ? c_808080 : c_00ff00));
						DrawBox(x_r(132), y_r(162 + p.id * 24), x_r(324), y_r(180 + p.id * 24), c, TRUE);
						DrawFormatStringToHandle(x_r(132), y_r(162 + p.id * 24), c_ffffff, parts->get_font(0), " %s", p.ptr->name.c_str());
						//進軍パラメータ
						for (size_t i = 0; i < p.wayselect; i++)
							DrawBox(x_r(348 + i * 12), y_r(162 + p.id * 24), x_r(356 + i * 12), y_r(180 + p.id * 24), c_3232ff, TRUE);
					}
					//敵
					else {
						//ステータス
						const auto c = (p.HP[0] == 0) ? c_808080 : c_ff0000;
						DrawBox(x_r(1500), y_r(162 + (p.id - teamc) * 24), x_r(1692), y_r(180 + (p.id - teamc) * 24), c, TRUE);
						DrawFormatStringToHandle(x_r(1500), y_r(162 + (p.id - teamc) * 24), c_ffffff, parts->get_font(0), " %s", p.ptr->name.c_str());
					}
				}
			}
			/*main*/
			else {
				/*sky*/
				if (!parts->get_in() || out.aim.flug) {
					SetDrawScreen(skyscreen);
					mapparts->draw_map_sky();
				}
				/*near*/
				SetDrawScreen(mainscreen);
				ClearDrawScreen();
				if (out.aim.flug)
					setcv(0.06f + rat_r / 2, 2000.0f, cam, view, upvec, 45.0f / rat_r);
				else {
					if (in.keyget[19])
						setcv(0.1f, 2000.0f, cam, view, upvec, 45.0f / rat_r);
					else
						setcv(0.16f + parts->get_view_r().z(), 2000.0f, cam, view, upvec, 45.0f / rat_r);
				}
				if (out.aim.flug) {
					auto v = out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe);
					getdist(
					    v,
					    (out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe + 1) - out.player[0].obj.frame(out.player[0].ptr->gun_[0].gunframe)).Norm(),
					    aim_r,
					    aimdist,
					    out.player[0].ptr->gun_speed[out.player[0].ammotype],
					    frate);
					aimpos = ConvWorldPosToScreenPos(v.get());
				}
				//pos

				for (auto& p : out.player) {
					p.iconpos = ConvWorldPosToScreenPos((p.mine.pos + VGet(0, (p.mine.pos - out.player[0].mine.pos).size() / 40 + ((out.aim.flug) ? 2 : 6), 0)).get());

					if (CheckCameraViewClip_Box((p.mine.pos + VGet(-5, 0, -5)).get(), (p.mine.pos + VGet(5, 3, 5)).get()) == TRUE)
						out.pssort[p.id] = pair(p.id, 9999.f);
					else
						out.pssort[p.id] = pair(p.id, (p.mine.pos - cam).size());
				}
				std::sort(out.pssort.begin(), out.pssort.end(), [](const pair& x, const pair& y) { return x.second > y.second; });

				//effect
				Effekseer_Sync3DSetting();
				if (!parts->get_in() || out.aim.flug) {
					DrawGraph(0, 0, skyscreen, FALSE); //sky
					//main
					ShadowMap_DrawSetup(mapparts->get_map_shadow_seminear());
					for (auto& tt : out.pssort) {
						if (tt.second == 9999.f)
							continue;
						if (tt.second < (10.0f * float(parts->get_shadex()) * parts->get_view_r().z() + 20.0f))
							break;
						MV1DrawMesh(out.player[tt.first].obj.get(), 0);
						for (int i = 1; i < out.player[tt.first].obj.mesh_num(); ++i)
							if (out.player[tt.first].HP[i + 4] > 0)
								MV1DrawMesh(out.player[tt.first].obj.get(), i);
					}
					ShadowMap_DrawEnd();

					ShadowMap_DrawSetup(mapparts->get_map_shadow_near());
					if (out.player[0].HP[0] > 0)
						humanparts->draw_human(0);

					for (auto& tt : out.pssort) {
						if (tt.second > (10.0f * float(parts->get_shadex()) * parts->get_view_r().z() + 20.0f))
							continue;
						MV1ResetFrameTextureAddressTransform(out.player[tt.first].obj.get(), 0);
						MV1DrawMesh(out.player[tt.first].obj.get(), 0);
						for (int i = 1; i < out.player[tt.first].obj.mesh_num(); ++i) {
							if (i < 3)
								MV1SetFrameTextureAddressTransform(out.player[tt.first].obj.get(), 0, 0.0, out.player[tt.first].wheelrad[i], 1.0, 1.0, 0.5, 0.5, 0.0);
							if (i == 3)
								MV1ResetFrameTextureAddressTransform(out.player[tt.first].obj.get(), 0);
							if (out.player[tt.first].HP[i + 4] > 0)
								MV1DrawMesh(out.player[tt.first].obj.get(), i);
						}
					}
					ShadowMap_DrawEnd();

					if (out.player[0].HP[0] > 0)
						humanparts->draw_human(0);

					mapparts->ready_shadow();
					mapparts->draw_map_model();


					for (auto& tt : out.pssort) {
						if (tt.second == 9999.f)
							continue;
						if (tt.first != 0 || (tt.first == 0 && !out.aim.flug)) {
							auto& p = out.player[tt.first];
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
							for (int i = 0; i < p.hit.size(); ++i)
								if (p.hit[i].flug) {
									MV1SetRotationZYAxis(p.hit[i].pic.get(), (p.colobj.frame(11 + 3 * i) - p.colobj.frame(9 + 3 * i)).get(), (p.colobj.frame(10 + 3 * i) - p.colobj.frame(9 + 3 * i)).get(), 0.f);
									MV1SetPosition(p.hit[i].pic.get(), (p.colobj.frame(9 + 3 * i) + (p.colobj.frame(10 + 3 * i) - p.colobj.frame(9 + 3 * i)).Scale(0.005f)).get());
									MV1DrawFrame(p.hit[i].pic.get(), p.hit[i].use);
								}
						}
					}

					soldierparts->draw_soldiers();
					//grass
					mapparts->draw_grass();
					//effect
					DrawEffekseer3D();
					//ammo
					SetUseLighting(FALSE);
					SetFogEnable(FALSE);
					for (auto& p : out.player)
						for (size_t guns = 0; guns < gunc; ++guns)
							for (size_t i = 0; i < ammoc; ++i)
								if (p.Gun[guns].Ammo[i].flug) {
									SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * std::min<float>(1.f, 4.f * p.Gun[guns].Ammo[i].speed / (p.ptr->gun_speed[p.ammotype] / fps))));
									DrawCapsule3D(p.Gun[guns].Ammo[i].pos.get(), p.Gun[guns].Ammo[i].repos.get(), p.ptr->gun_[guns].ammosize * ((p.Gun[guns].Ammo[i].pos - cam).size() / 60.f), 4, p.Gun[guns].Ammo[i].color, c_ffffff, TRUE);
								}
					soldierparts->draw_soldiersammo();
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
					SetFogEnable(TRUE);
					SetUseLighting(TRUE);

					//tree
					mapparts->draw_trees();
					DrawEffekseer3D();
					mapparts->exit_shadow();
				}
				else
					humanparts->draw_humanall();
			}
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();
			if (out.map.flug)
				DrawGraph(0, 0, minimap, FALSE); /*指揮*/
			else {
				/*通常*/
				DrawGraph(0, 0, mainscreen, FALSE);
				/*ブルーム*/
				if (!parts->get_in() && parts->get_usehost()) {
					GraphFilterBlt(mainscreen, HighBrightScreen, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS, 210, TRUE, c_000000, 255);
					GraphFilterBlt(HighBrightScreen, GaussScreen, DX_GRAPH_FILTER_DOWN_SCALE, EXTEND);
					GraphFilter(GaussScreen, DX_GRAPH_FILTER_GAUSS, 16, 1000);
					SetDrawMode(DX_DRAWMODE_BILINEAR);
					SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
					DrawExtendGraph(0, 0, dispx, dispy, GaussScreen, FALSE);
					DrawExtendGraph(0, 0, dispx, dispy, GaussScreen, FALSE);
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
				}
				/*UI*/
				/*アイコン*/
				for (auto& tt : out.pssort) {
					if (tt.first == 0 || tt.second == 9999.f)
						continue;
					uiparts->draw_icon(out.player[tt.first], parts->get_font(0), frate);
				}
				if (out.aim.flug)
					uiparts->draw_sight(aimpos, rat_r, aimdist, parts->get_font(0)); /*照準器*/
				if (in.keyget[19] && !out.aim.flug)
					uiparts->draw_drive();						 /*ドライバー視点*/
				uiparts->draw_ui(selfammo, parts->get_view_r().y(), parts->get_font(0)); /*main*/
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
		soldierparts->delete_soldiers();
		for (auto& p : out.player) {
			/*エフェクト*/
			for (auto& g : p.gndsmkeffcs)
				g.handle.Dispose();
			for (auto&& e : p.effcs)
				e.handle.Dispose();
			p.gndsmkeffcs.clear();
			p.gndsmksize.clear();
			p.hitres.clear();
			/*Box2D*/
			delete p.mine.playerfix->GetUserData();
			p.mine.playerfix->SetUserData(NULL);
			/**/
			p.obj.Dispose();
			p.colobj.Dispose();
			for (auto& h : p.hit)
				h.pic.Dispose();
			for (auto& s : p.se)
				s.Dispose();
			p.Springs.clear();
			p.HP.clear();
			p.hitssort.clear();

			for (auto& t : p.foot) {
				for (auto& f : t.Foot) {
					delete f.playerfix->GetUserData();
					f.playerfix->SetUserData(NULL);
				}
				for (auto& f : t.Wheel) {
					delete f.playerfix->GetUserData();
					f.playerfix->SetUserData(NULL);
				}
				for (auto& f : t.Yudo) {
					delete f.playerfix->GetUserData();
					f.playerfix->SetUserData(NULL);
				}
			}
		}
		out.pssort.clear();
		out.player.clear();
		in.next = true;
		while (out.respawn) {}
		in.next = false;
	} while (!out.ends);
	/*終了*/
	threadparts->thead_stop();
 	return 0;
}
