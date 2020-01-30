#pragma once

#ifndef INCLUDED_define_h_
#define INCLUDED_define_h_
#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include <windows.h>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include "Box2D/Box2D.h"
#include "useful.h"
#include <array>
#include <algorithm>
#include <memory>
#include <vector>
#include <cstring>
#include <string_view>
#include <cstdint>
#include <optional>
#include <array>
#include <iostream>
#include <fstream>

#include "DXLib_vec.hpp"
#include "MV1ModelHandle.hpp"
#include "EffekseerEffectHandle.hpp"
#include "SoundHandle.hpp"
#include "GraphHandle.hpp"
#include "FontHandle.hpp"

using std::size_t;
using std::uint8_t;
using std::int8_t;
using std::uint16_t;
inline const int dispx = (GetSystemMetrics(SM_CXSCREEN)); /*描画X*/
inline const int dispy = (GetSystemMetrics(SM_CYSCREEN)); /*描画Y*/
constexpr float M_GR = -9.8f;				  /*重力加速度*/
constexpr size_t waypc = 4;				  /*移動確保数*/
constexpr size_t ammoc = 32;				  /*砲弾確保数*/
#define TEAM 1						  /*味方ID*/
#define ENEMY 2						  /*敵ID*/
constexpr auto EXTEND = 4;				  /*ブルーム用*/
constexpr size_t gunc = 2;				  /*銃、砲の数*/
constexpr auto divi = 2;				  /*人の物理処理*/

/*構造体*/
enum animeid {
	//タイトル用
	ANIME_L1 = 0,
	ANIME_L2 = 1,
	ANIME_L3 = 2,
	ANIME_LtoR = 3,
	ANIME_R = 4,
	ANIME_RtoL = 5,
	ANIME_title = 6,
	//戦闘用
	ANIME_nom = 0,
	ANIME_sit = 1,
	ANIME_eye = 2,
	ANIME_voi = 3,
	ANIME_out = 4,
	//ボイス用
	ANIME_voice = 2 //ボイス数
};
enum Key {
	KEY_GOFLONT = 0x001,
	KEY_GOBACK_ = 0x002,
	KEY_GOLEFT_ = 0x004,
	KEY_GORIGHT = 0x008,
	KEY_TURNLFT = 0x010,
	KEY_TURNRIT = 0x020,
	KEY_TURNUP_ = 0x040,
	KEY_TURNDWN = 0x080,
	KEY_SHOTCAN = 0x100,
	KEY_SHOTGAN = 0x200
};
enum Bone {
	//砲塔内
	bone_hatch = 5,    //カメラ
	bone_in_turret = 6 //人配置フレーム
};
enum Effect {
	ef_fire = 0,
	ef_reco = 1,
	ef_bomb = 2,
	ef_smoke1 = 3,
	ef_smoke2 = 4,
	ef_gndhit = 5,
	ef_gun = 6,
	ef_gndhit2 = 7,
	ef_reco2 = 8,
	effects = 9,   //読み込む
	ef_smoke3 = 9, //読み込まない
	efs_user = 10
};
struct ammos {
	bool hit{ false };
	bool flug{ false };
	int cnt = 0;
	int color = 0;
	float speed = 0.f, pene = 0.f;
	VECTOR_ref pos, repos, vec;
};
struct EffectS {
	bool flug{ false };		 /**/
	Effekseer3DPlayingHandle handle; /**/
	VECTOR_ref pos;			 /**/
	VECTOR_ref nor;			 /**/
};
struct Hit {
	bool flug{ false }; /**/
	int use{ 0 };       /*使用フレーム*/
	MV1ModelHandle pic; /*弾痕モデル*/
};
struct switches {
	bool flug{ false };
	uint8_t cnt{ 0 };
};
struct vehicle {
	std::string name;		  /*名前*/
	int countryc;			  /*国*/
	MV1ModelHandle model;		  /*モデル*/
	MV1ModelHandle colmodel;	  /*コリジョン*/
	MV1ModelHandle inmodel;		  /*内装*/
	std::array<float, 4> speed_flont; /*前進*/
	std::array<float, 4> speed_back;  /*後退*/
	float vehicle_RD = 0.0f;	  /*旋回速度*/
	std::array<float, 4> armer;       /*装甲*/
	bool gun_lim_LR = 0;		  /*砲塔限定旋回の有無*/
	std::array<float, 4> gun_lim_;    /*砲塔旋回制限*/
	float gun_RD = 0.0f;		  /*砲塔旋回速度*/
	std::array<float, 3> gun_speed;   /*弾速*/
	std::array<float, 3> pene;	/*貫通*/
	std::array<int, 3> ammotype;      /*弾種*/
	std::vector<VECTOR_ref> loc;      /*フレームの元座標*/
	VECTOR_ref min;			  /*box2D用フレーム*/
	VECTOR_ref max;			  /*box2D用フレーム*/
	int turretframe;		  /*砲塔フレーム*/
	struct guninfo {
		int gunframe;   /*銃フレーム*/
		int reloadtime; /*リロードタイム*/
		float ammosize; /*砲口径*/
		int accuracy;   /*砲精度*/
	};
	std::array<guninfo, gunc> gun_; /*銃フレーム*/
	std::vector<int> youdoframe;    /*誘導輪*/
	std::vector<int> wheelframe;    /*転輪*/
	std::array<int, 2> kidoframe;   /*起動輪*/
	std::array<int, 2> smokeframe;  /*排煙*/
	std::vector<int> upsizeframe;   /*履帯上*/
	int engineframe;		/*エンジン*/
};
//アニメーション
struct Anime {
	int id;
	float total;
	float time;
	float per;
};

static_assert(std::is_move_constructible_v<vehicle>);
namespace std {
	template <>
	struct default_delete<b2Body> {
		void operator()(b2Body* body) const {
			body->GetWorld()->DestroyBody(body);
		}
	};
};
struct players {
	int camf{ false }; /*撃破カメラ*/

	size_t id;			      /*ID*/
	vehicle* ptr;			      /*vehicle*/
	MV1ModelHandle obj;		      /*モデル*/
	MV1ModelHandle colobj;		      /*コリジョン*/
	uint8_t type{ 0 };		      /*敵味方識別*/
	std::vector<SoundHandle> se;	  /*SE*/
	size_t move{ 0 };		      /*キー操作*/
	MATRIX ps_m, ps_t, ps_n;	      /*車体行列,砲塔行列,法線行列*/
	float yace{ 0.f };		      /*y方向加速度*/
	VECTOR_ref vec;			      /*移動ベクトル*/
	VECTOR_ref nor;			      /*法線ベクトル*/
	VECTOR_ref zvec;		      /*前向ベクトル*/
	float spd{ 0.f };		      /*速度関連*/
	float accel{ 0.f };		      /*加速度*/
	float yrad{ 0.f };		      /*角度*/
	float yadd{ 0.f };		      /*角速度*/
	int recorad{ 0 };		      /*弾き角度*/
	int firerad{ 0 };		      /*反動角度*/
	VECTOR_ref recovec;		      /*弾きベクトル*/
	std::optional<size_t> atkf;	   /*cpu ヘイト*/
	int aim{ 0 };			      /*cpu ヘイトの変更カウント*/
	bool selc;			      /**/
	size_t wayselect{ 0 }, waynow{ 0 };   /*cpu */
	std::array<VECTOR_ref, waypc> waypos; /*cpu ウェイポイント*/
	std::array<int8_t, waypc> wayspd;     /*cpu 速度指定*/
	//int state{ 0 };			       /*cpu ステータス*/
	int lost_sec{ 0 };		       /*cpu 見失いカウント*/
	struct Guns {			       /**/
		std::array<ammos, ammoc> Ammo; /*確保する弾*/
		int loadcnt{ 0 };	      /*装てんカウンター*/
		size_t useammo{};	      /*使用弾*/
		float fired{ 0.f };	    /*駐退*/
	};				       /**/
	std::array<Guns, gunc> Gun;	    /*銃、砲全般*/
	int8_t gear{ 0 };		       /*変速*/
	//unsigned int gearu{ 0 };	       /*キー*/
	//unsigned int geard{ 0 };	       /*キー*/
	float inertiax;			     /*慣性*/
	std::array<float, 3> wheelrad;       /*履帯の送り、転輪旋回*/
	VECTOR_ref gunrad;		     /*砲角度*/
	VECTOR_ref gunrad_rec;		     /*砲角度*/
	float gun_turn{ 0.f };		     /*砲旋回速度*/
	int ammotype{ 0 };		     /*弾種*/
	std::array<size_t, 3> setammo;       /*搭載弾数*/
	bool hitadd{ false };		     /*命中フラグ*/
	int hitid{ 0 };			     /*あてた敵*/
	VECTOR_ref iconpos;		     /*UI用*/
	std::vector<float> Springs;	  /*スプリング*/
	std::vector<int16_t> HP;	    /*ライフ*/
	std::array<uint16_t, 2> footfix;     /*履帯修復*/
	std::vector<pair> hitssort;	  /*当たった順番*/
	int hitbuf;			     /*使用弾痕*/
	std::array<Hit, 3> hit;		     /*弾痕*/
	std::array<EffectS, efs_user> effcs; /*effect*/
	std::vector<EffectS> gndsmkeffcs;    /*effect*/
	std::vector<float> gndsmksize;       /*effect*/

	std::vector<MV1_COLL_RESULT_POLY> hitres; /*確保*/
	bool checkhit;				  /*判定の更新をするか*/
	struct b2Pats {
		std::unique_ptr<b2Body> body; /**/
		b2Fixture* playerfix;	 /**/
		VECTOR_ref pos;		      /**/
	};
	b2Pats mine; /*box2d*/
	struct FootWorld {
		b2World* world;		       /*足world*/
		b2RevoluteJointDef f_jointDef; /*ジョイント*/
		std::vector<b2Pats> Foot;      /**/
		std::vector<b2Pats> Wheel;     /**/
		std::vector<b2Pats> Yudo;      /**/
		int LR;			       /*左右か*/
	};
	std::array<FootWorld, 2> foot; /*足*/
};
/*CLASS*/
class Myclass {
private:
	/*setting*/
	bool usegrab;       /*人の物理演算のオフ、オン*/
	unsigned char ANTI; /*アンチエイリアス倍率*/
	bool YSync;	 /*垂直同期*/
	float f_rate;       /*fps*/
	bool windowmode;    /*ウィンドウor全画面*/
	float drawdist;     /*木の描画距離*/
	int gndx;	   /*地面のクオリティ*/
	int shadex;	 /*影のクオリティ*/
	bool USEHOST;       /**/
	bool USEPIXEL;      /*ピクセルライティングの利用*/
	float se_vol;       /**/
	/*common*/
	std::vector<vehicle> vecs;			     /*車輛情報*/
	VECTOR_ref view, view_r;			     /*通常視点の角度、距離*/
	std::vector<int> fonts;				     /*フォント*/
	std::array<SoundHandle, 13> se_;		     /*効果音*/
	std::array<EffekseerEffectHandle, effects> effHndle; /*エフェクトリソース*/
	EffekseerEffectHandle gndsmkHndle;		     /*エフェクトリソース*/
public:
	Myclass();
	const auto get_usegrab(void) { return usegrab; }
	const auto get_gndx(void) { return gndx; }
	const auto get_shadex(void) { return shadex; }
	const auto get_drawdist(void) { return drawdist; }
	const auto get_f_rate(void) { return f_rate; }
	const auto get_usehost(void) { return USEHOST; }
	const auto get_se_vol(void) { return se_vol; }

	void autoset_option(void);

	void write_option(void);

	template <typename... Args>
	void set_fonts(Args&&... args) {
		SetUseASyncLoadFlag(true);
		(this->fonts.emplace(this->fonts.end(), DxLib::CreateFontToHandle(NULL, x_r(args), y_r(args / 3), DX_FONTTYPE_ANTIALIASING_EDGE)), ...);
		SetUseASyncLoadFlag(false);
	} //(必要なフォント数,サイズ1,サイズ2, ...)

	bool set_veh(void);
	int window_choosev(void); //車両選択
	void set_viewrad(VECTOR_ref vv);
	void set_view_r(int wheel, bool life);
	void Screen_Flip(LONGLONG waits);
	~Myclass();
	void set_se_vol(unsigned char size);
	void play_sound(int p1);
	const auto get_font(int p1) { return fonts[p1]; } //フォントハンドル取り出し
	const auto get_view_r(void) { return view_r; }
	const auto get_in(void) { return view_r.z() == 0.1f; }
	VECTOR_ref get_view_pos(void) { return VScale(VGet(sin(view_r.y()) * cos(view_r.x()), sin(view_r.x()), cos(view_r.y()) * cos(view_r.x())), 15.0f * view_r.z()); }
	EffekseerEffectHandle& get_effHandle(int p1) noexcept { return effHndle[p1]; }
	const EffekseerEffectHandle& get_effHandle(int p1) const noexcept { return effHndle[p1]; }
	EffekseerEffectHandle& get_gndhitHandle() noexcept { return gndsmkHndle; }
	const EffekseerEffectHandle& get_gndhitHandle() const noexcept { return gndsmkHndle; }
	vehicle* get_vehicle(int p1) { return &vecs[p1]; }
};
class HUMANS {
private:
	bool usegrab{ false }; /*人の物理演算のオフ、オン*/
	float f_rate{ 60.f };  /*fps*/
	//リソース
	struct Hmod {
		//
		MV1ModelHandle model;
		std::array<SoundHandle, ANIME_voice> sound;
		//タイトル用モーション
		std::array<Anime, ANIME_title> amine_title;
	};
	std::vector<Hmod> mod;
	struct humans {
		char vflug{ 0 };
		MV1ModelHandle obj;
		std::array<Anime, ANIME_out> amine;
		std::array<Anime, ANIME_voice> voice_anime;
		std::array<SoundHandle, ANIME_voice> voice_sound; //ハンドル
		int neck{ 0 };
		VECTOR_ref nvec;
		float voice_time{ 0.f }; //
	};
	std::vector<humans> hum;	 /**/
	MV1ModelHandle inmodel_handle;   //中モデル
	bool in_f{ false };		 //中描画スイッチ
	size_t inflames;		 //inmodelのフレーム数
	std::vector<VECTOR_ref> locin;   /*inmodelのフレーム*/
	std::vector<VECTOR_ref> pos_old; /*inmodelの前回のフレーム*/
	std::vector<std::string> name;   /**/
	players* pplayer;		 /*playerdata*/
public:
	HUMANS(bool useg, float frates);
	bool set_humans(const MV1ModelHandle& inmod);
	void set_state(players* play); /*使用するポインタの指定*/
	void set_humanvc_vol(unsigned char size);
	void set_humanmove(VECTOR_ref rad, const float frate, const float fps);
	void draw_human(size_t p1);
	void draw_humanall();
	void delete_human(void);
	void start_humanvoice(std::int8_t p1);
	void start_humananime(int p1);
	const auto get_neckpos() { return hum[0].obj.frame(hum[0].neck); }
	const auto get_campos() { return inmodel_handle.frame(bone_hatch); }
};
class SOLDIERS {
	uint8_t count = 0;
	float f_rate{ 60.f };	  /*fps*/
	VECTOR_ref camera, viewv, upv; /*カメラ*/
	float rat;		       /*倍率*/
	//リソース
	std::array<MV1ModelHandle, 2> model;
	struct soldiers {
		uint8_t type{ 0 }; /*敵味方識別*/
		size_t id;
		MV1ModelHandle obj;
		VECTOR_ref pos;
		float yrad;
		std::array<Anime, 12> amine;
		size_t useanime;
		std::array<VECTOR_ref, waypc> waypoint;
		uint8_t waynow;
		int8_t HP;
		std::optional<size_t> atkf; /*cpu ヘイト*/
		std::array<ammos, ammoc> ammo;
		size_t useammo;
	};
	std::vector<soldiers> sol; /**/
	std::vector<pair> sort;    /**/
				   //ammos
public:
	SOLDIERS(float frates);
	void set_camerapos(VECTOR_ref pos, VECTOR_ref vec, VECTOR_ref up, float ratio);
	void set_soldier(const uint8_t type, const VECTOR_ref position, const float rad);
	void set_soldier_vol(unsigned char size);
	void set_soldiermove(int map, std::vector<players>& play);
	void set_hit(VECTOR_ref pos, VECTOR_ref repos);
	void draw_soldiersammo();
	void draw_soldiers();
	void delete_soldiers(void);
};
class MAPS {
private:
	/*setting*/
	int groundx;
	float drawdist;
	int shadowx;
	/**/
	size_t treec = 750; /*木の数*/
	struct treeinfo {
		size_t id;	    /**/
		MV1ModelHandle nears; /**/
		MV1ModelHandle fars;  /**/
		VECTOR_ref pos, rad;  /**/
		bool hit;	     /**/
	};
	struct trees {
		MV1ModelHandle mnear;	/**/
		MV1ModelHandle mfar;	 /**/
		std::vector<pair> treesort;  /**/
		std::vector<treeinfo> tree_; /**/
	} tree;
	std::array<int, 3> shadowmap;   /*シャドウマップ(近中遠)*/
	MV1ModelHandle m_model, minmap; /*mapモデル*/
	GraphHandle dif_gra, nor_gra;   /*mapテクスチャ*/
	GraphHandle dif_tex, nor_tex;   /*実テクスチャ*/
	GraphHandle nor_trk;		/*轍ノーマルテクスチャ*/
	GraphHandle GgHandle;		/*地面画像*/
	MV1ModelHandle sky_model;       /*skyモデル*/
	GraphHandle sky_sun;		/*sunpic*/
	VECTOR_ref lightvec;		/*light方向*/
	int grasss = 50000;		/*grassの数*/
	std::vector<VERTEX3D> grassver; /*grass*/
	std::vector<DWORD> grassind;    /*grass*/
	int VerBuf, IndexBuf;		/*grass*/
	MV1ModelHandle grass;		/*grassモデル*/
	GraphHandle graph;		/*画像ハンドル*/
	int IndexNum, VerNum;		/*grass*/
	int vnum, pnum;			/*grass*/
	MV1_REF_POLYGONLIST RefMesh;    /*grass*/
	VECTOR_ref camera, viewv, upv;  /*カメラ*/
	float rat;			/*倍率*/
	VECTOR_ref map_min, map_max;    /*mapsize*/
public:
	MAPS(int map_size, float draw_dist, int shadow_size);
	void set_map_readyb(size_t set);
	bool set_map_ready(void);
	void set_camerapos(VECTOR_ref pos, VECTOR_ref vec, VECTOR_ref up, float ratio);
	void set_map_shadow_near(float vier_r);
	void draw_map_track(const players& player);
	void draw_map_model(void);
	void set_map_track(void);
	void draw_map_sky(void);
	void delete_map(void);

	void ready_shadow(void);
	void exit_shadow(void);
	void set_normal(VECTOR_ref& nor, MATRIX& ps_n, VECTOR_ref position, const float frate, const float fps); //地面に沿わせる
	auto& get_minmap() & { return dif_tex; }
	const auto& get_minmap() const& noexcept { return dif_tex; }
	auto& get_minsize() & { return map_min; }
	const auto& get_minsize() const& noexcept { return map_min; }
	auto& get_maxsize() & { return map_max; }
	const auto& get_maxsize() const& noexcept { return map_max; }
	auto& get_mapobj() & { return m_model; }
	const auto& get_mapobj() const& noexcept { return m_model; }
	const auto get_map_shadow_near() { return shadowmap[0]; }
	const auto get_map_shadow_seminear() { return shadowmap[1]; }
	const auto get_gnd_hit(VECTOR_ref startpos, VECTOR_ref endpos) { return MV1CollCheck_Line(m_model.get(), 0, startpos.get(), endpos.get()); }
	void set_hitplayer(VECTOR_ref pos);
	void draw_trees(void);
	void draw_grass(void);
};
class UIS {
private:
	struct country {
		std::array<GraphHandle, 8> ui_sight;
	};
	std::array<GraphHandle, 4> ui_reload; /*弾UI*/
	GraphHandle ui_compass;		      /*UI*/
	std::vector<GraphHandle> UI_body;     /*UI*/
	std::vector<GraphHandle> UI_turret;   /*UI*/
	std::vector<country> UI_main;	 /*国別UI*/
	size_t countries = 1;		      /*国数*/
	float gearf = 0.f;		      /*変速*/
	float recs = 0.f;		      /*跳弾表現用*/
	players* pplayer;		      /*playerdata*/
	std::array<float, 3> reload_mov;      /*リロード*/

	/*debug*/
	float deb[60][6 + 1];
	LONGLONG waypoint;
	float waydeb[6];
	size_t seldeb;

public:
	UIS();

	void draw_load(void);	  /*ロード画面*/
	bool draw_title(void);
	void set_state(players* play); /*使用するポインタの指定*/
	void set_reco(void);	   /*反射スイッチ*/
	void draw_drive();
	void draw_icon(players& p, int font, float frate);
	void draw_sight(VECTOR_ref aimpos, float ratio, float dist, int font); /*照準UI*/
	void draw_ui(uint8_t selfammo[], float y_v, int font);		       /*メインUI*/
	/*debug*/
	void put_way(void);
	void end_way(void);
	void debug(float fps, float time);
};
/**/
void setcv(float neard, float fard, VECTOR_ref cam, VECTOR_ref view, VECTOR_ref up, float fov);		  //カメラ情報指定
void getdist(VECTOR_ref& startpos, VECTOR_ref vec, float& dist, float& getdists, float speed, float fps); //startposに測距情報を出力
//effect
void set_effect(EffectS* efh, VECTOR_ref pos, VECTOR_ref nor);
void set_pos_effect(EffectS* efh, const EffekseerEffectHandle& handle);
//play_class予定
bool get_reco(players& play, std::vector<players>& tgts, ammos& c, size_t gun_s);
void set_gunrad(players& play, float rat_r);
bool set_shift(players& play);
//
#endif
