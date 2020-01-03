#pragma once
#define dispx		(GetSystemMetrics(SM_CXSCREEN)/1)			/*描画X*/
#define dispy		(GetSystemMetrics(SM_CYSCREEN)/1)			/*描画Y*/
#define M_GR		-9.8f							/*重力加速度*/
#define waypc		4							/*移動確保数*/
#define ammoc		16							/*砲弾確保数*/
#define animes		4							/*人アニメーション*/
#define voice		1							/*ボイス*/
#define map_x		1000							/*マップサイズX*/
#define map_y		1000							/*マップサイズY*/
#define TEAM		1							/*味方ID*/
#define ENEMY		2							/*敵ID*/
#define EXTEND		4							/*ブルーム用*/
#define gunc		2							/*銃、砲の数*/
#define divi		2							/*人の物理処理*/

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
#include <thread>

#include<algorithm>
#include <vector>
#include <cstring>
#include "MV1Handle.hpp"
#include <string_view>
/*構造体*/
enum cpu {
	CPU_NOMAL = 0,
	CPU_CHARGE = 1
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
enum Bone{
	bone_trt	= 2,
	bone_gun1	= 3,
	bone_gun2	= 4,
	bone_hatch	= 5,
	bone_smoke1	= 6,
	bone_smoke2	= 7,
	bone_engine	= 8,
	bone_gun	= 9,
	bone_gun_	= 10,
	bone_wheel	= 11,
	bone_in_turret	= 6
};
enum Effect {
	ef_fire		= 0,
	ef_reco		= 1,
	ef_bomb		= 2,
	ef_smoke1	= 3,
	ef_smoke2	= 4,
	ef_gndhit	= 5,
	ef_gun		= 6,
	ef_gndhit2	= 7,
	ef_reco2	= 8,
	effects		= 9,//読み込む
	ef_smoke3	= 9,//読み込まない
	efs_user	= 10
};

struct ammos {
	int flug = 0;
	int cnt = 0;
	int color = 0;
	float speed = 0.f, pene = 0.f;
	VECTOR pos{ VGet(0, 0, 0) }, repos{ VGet(0, 0, 0) }, vec{ VGet(0, 0, 0) }, rad{ VGet(0, 0, 0) };
};
struct EffectS {
	int efhandle{ -1 };						/**/
	bool efflug{ 0 };						/**/
	VECTOR effpos = { VGet(0, 0, 0) };				/**/
	VECTOR effnor = { VGet(0, 0, 0) };				/**/
};
struct vehicle {
	std::string name;						/*名前*/
	int countryc;							/*国*/
	MV1Handle model;							/*モデル*/
	MV1Handle colmodel;						/*コリジョン*/
	MV1Handle inmodel;						/*内装*/
	float spdflont[4] = { 0.0f };					/*前進*/
	float spdback[4] = { 0.0f };					/*後退*/
	float vehicle_RD = 0.0f;					/*旋回速度*/
	float armer[4] = { 0 };						/*装甲*/
	bool gun_lim_LR = 0;						/*砲塔限定旋回の有無*/
	float gun_lim_[4] = { 0.f };					/*砲塔旋回制限*/
	float gun_RD = 0.0f;						/*砲塔旋回速度*/
	int reloadtime[gunc]{ 0 };					/*リロードタイム*/
	float ammosize = 0.0f;						/*砲口径*/
	float gun_speed[3] = { 0.0f };					/*弾速*/
	float pene[3] = { 0.0f };					/*貫通*/
	int ammotype[3] = { 0 };					/*弾種*/
	std::vector<VECTOR> loc;					/*フレームの元座標*/
	VECTOR coloc[4] = { VGet(0,0,0) };				/*box2D用フレーム*/
	int frames{ 0 };
	int colmeshes{ 0 };
	int meshes{ 0 };
};
static_assert(std::is_move_constructible_v<vehicle>);
struct players {
	/*情報*/
	int use{ 0 };							/*使用車両*/
	vehicle* ptr;							/*vehicle*/
	MV1Handle obj;							/*モデル*/
	MV1Handle colobj;						/*コリジョン*/
	MV1Handle hitpic[3];							/*弾痕モデル*/
	char type{ 0 };							/*敵味方識別*/
	int se[50]{ 0 };						/*SE*/
	/**/
	int move{ 0 };							/*キー操作*/
	VECTOR pos{ VGet(0, 0, 0) };					/*座標*/
	MATRIX ps_m;							/*車体行列*/
	MATRIX ps_t;							/*砲塔行列*/
	//std::vector<MATRIX> ps_all;					/*行列*/
	float yace{ 0.f };						/*加速度*/
	float speed{ 0.f }, speedrec{ 0.f }, flont{ 0.f }, back{ 0.f };	/*速度関連*/
	VECTOR vec{ VGet(0, 0, 0) };					/*移動ベクトル*/
	float xnor{ 0.f }, znor{ 0.f }, znorrec{ 0.f };			/*法線角度*/
	VECTOR nor{ VGet(0, 0, 0) };					/*法線*/
	float yrad{ 0.f };						/*角度*/
	float yadd{ 0.f };						/*角速度*/
	int recoall{ 0 };						/*弾き角度*/
	int firerad{ 0 };						/*反動角度*/
	float recorad{ 0.f };						/*弾き反動*/
	/*cpu関連*/
	int atkf{ -1 };							/*cpuのヘイト*/
	int aim{ 0 };							/*ヘイトの変更カウント*/
	int wayselect{ 0 }, waynow{ 0 };				/**/
	VECTOR waypos[waypc]{ VGet(0, 0, 0) };				/*ウェイポイント*/
	int wayspd[waypc]{ 0 };						/*速度指定*/
	int state{ 0 };							/*ステータス*/
	/**/
	std::vector<ammos> Ammo;					/*確保する弾(arrayでもいい？)*/
	std::vector<float> Springs;					/*スプリング*/
	std::vector<short> HP;						/*ライフ*/
	std::vector<pair> hitssort;					/*当たった順番*/
	/**/
	int gear{ 0 };							/*変速*/
	unsigned int gearu{ 0 };					/*キー*/
	unsigned int geard{ 0 };					/*キー*/
	VECTOR inertia{ VGet(0, 0, 0) };				/*慣性*/
	float wheelrad[3]{ 0.f };					/*履帯の旋回*/
	VECTOR gunrad{ 0.f };						/*砲角度*/
	float fired{ 0.f };						/*駐退*/
	/*弾関連*/
	int ammotype{ 0 };						/*弾種*/
	int loadcnt[gunc]{ 0 };						/*装てんカウンター*/
	int useammo[gunc]{ 0 };						/*使用弾*/
	bool recoadd{ false };						/*弾きフラグ*/
	bool hitadd{ false };						/*命中フラグ*/
	VECTOR iconpos{ VGet(0, 0, 0) };				/*UI用*/
	EffectS effcs[efs_user];					/*effect*/
	int usepic[3];							/*使用フレーム*/
	int hitbuf;							/*使用弾痕*/
	/*box2d*/
	b2Body* body;							/**/
	b2FixtureDef fixtureDef;					/*動的ボディフィクスチャを定義します。*/
	b2PolygonShape dynamicBox;					/*ダイナミックボディに別のボックスシェイプを定義します。*/
	b2Fixture *playerfix;						/**/
	b2BodyDef bodyDef;						/*ダイナミックボディを定義します。その位置を設定し、ボディファクトリを呼び出します。*/
};
struct switches {
	bool flug{ false };
	int cnt{ 0 };
};
/*CLASS*/
/*
class MV1Handle {
private:
	int handle_;
	constexpr MV1Handle(int h) noexcept : handle_(h) {}
public:
	constexpr MV1Handle() noexcept : handle_(-1) {}
	MV1Handle(const MV1Handle&) = delete;
	MV1Handle(MV1Handle&&) = default;
	MV1Handle& operator=(const MV1Handle&) = delete;
	MV1Handle& operator=(MV1Handle&&) = default;
	~MV1Handle() noexcept {
		if (-1 != this->handle_) { MV1DeleteModel(this->handle_); }
	}
	void Dispose() noexcept {
		if (-1 != this->handle_) { MV1DeleteModel(this->handle_); this->handle_ = -1; }
	}
	int get() const noexcept { return handle_; }
	MV1Handle DuplicateModel() const noexcept { return DxLib::MV1DuplicateModel(this->handle_); }
	static MV1Handle LoadModel(std::basic_string_view<TCHAR> FileName) noexcept { return DxLib::MV1LoadModelWithStrLen(FileName.data(), FileName.length()); }
};
//*/
class Myclass {
private:
	/*setting*/
	bool usegrab{ false };							/*人の物理演算のオフ、オン、一人だけオン*/
	unsigned char ANTI{ 1 };						/*アンチエイリアス倍率*/
	bool YSync{ true };							/*垂直同期*/
	float f_rate{ 60.f };							/*fps*/
	bool windowmode{ false };						/*ウィンドウor全画面*/
	float drawdist{ 100.0f };						/*木の描画距離*/
	int gndx = 8;
	/**/
	std::vector<vehicle> vecs;						/*車輛情報*/
	VECTOR view, view_r;							/*通常視点の角度、距離*/
	std::vector<int> fonts;							/*フォント*/
	int se_[13];								/*効果音*/
	int ui_reload[4] = { 0 };						/*UI用*/
	int effHndle[effects] = { 0 };						/*エフェクトリソース*/
public:
	Myclass();
	bool get_usegrab(void) { return usegrab; }
	int get_gndx(void) { return gndx; }
	float get_drawdist(void) { return drawdist; }
	float get_f_rate(void) { return f_rate; }
	void write_option(void);						//未実装
	//bool set_fonts(int arg_num, ...);					//(必要なフォント数,サイズ1,サイズ2, ...)
	template<typename ...Args>
	void set_fonts(Args&& ...args)
	{
		SetUseASyncLoadFlag(true);
		auto create = [](int value) { return DxLib::CreateFontToHandle(NULL, x_r(value), y_r(value / 3), DX_FONTTYPE_ANTIALIASING_EDGE); }
		this->fonts.emplace(this->fonts.end(), create(args)...);
		SetUseASyncLoadFlag(false);
	}

	bool set_veh(void);
	int window_choosev(void);						//車両選択
	void set_viewrad(VECTOR vv);
	void set_view_r(void);
	void Screen_Flip(LONGLONG waits);
	~Myclass();
	void set_se_vol(unsigned char size);
	void play_sound(int p1);
	int* get_ui2(void) { return ui_reload; }
	int get_font(int p1) { return fonts[p1]; }				//フォントハンドル取り出し
	VECTOR get_view_r(void) { return view_r; }
	VECTOR get_view_pos(void) { return VScale(VGet(sin(view_r.y) * cos(view_r.x), sin(view_r.x), cos(view_r.y) * cos(view_r.x)), 15.0f * view_r.z); }
	int get_effHandle(int p1) { return effHndle[p1]; }
	vehicle* get_vehicle(int p1) { return &vecs[p1]; }
};
class HUMANS {
private:
	struct humans {
		MV1Handle obj;
		int neck{ 0 };
		VECTOR nvec{ VGet(0,0,0) };
		int amine[animes]{ 0 };
		float time[animes]{ 0.f };
		float alltime[animes]{ 0.f };
		float per[animes]{ 0.f };
		char vflug{ 0 };
		float voicetime{ 0.f };
		float voicealltime[voice]{ 0 };
		int voices[voice]{ 0 };
		int vsound[voice]{ 0 };
	};
	int human{ 0 };								/*乗員人数*/
	bool usegrab{ false };							/*人の物理演算のオフ、オン、一人だけオン*/
	float f_rate{ 60.f };							/*fps*/

	MV1Handle inmodel_handle;						//中モデル
	bool in_f{ false };							//中描画スイッチ
	int inflames;								//inmodelのフレーム数
	std::vector<humans> hum;						/**/
	std::vector<VECTOR> locin;						/*inmodelのフレーム*/
	std::vector<VECTOR> pos_old;						/*inmodelの前回のフレーム*/
	bool first;								//初回フラグ
public:
	HUMANS(bool useg, float frates);
	void set_humans(const MV1Handle& inmod);
	void set_humanvc_vol(unsigned char size);
	void set_humanmove(const players& player, VECTOR rad, float fps);
	void draw_human(int p1);
	void draw_humanall();
	void delete_human(void);
	void start_humanvoice(int p1);
	void start_humananime(int p1);
	VECTOR get_neckpos() { return MV1GetFramePosition(hum[0].obj.get(), hum[0].neck);}
	VECTOR get_campos() { return MV1GetFramePosition(inmodel_handle.get(), bone_hatch); }
};
class MAPS {
private:
	/*setting*/
	int groundx;
	float drawdist;
	/**/
	int treec = 750;							/*木の数*/
	struct trees {
		MV1Handle mnear;							/**/
		MV1Handle mfar;							/**/
		std::vector<MV1Handle> nears;							/**/
		std::vector<MV1Handle> fars;							/**/

		std::vector<pair> treesort;
		std::vector<VECTOR> pos;
		std::vector<VECTOR> rad;
		std::vector<bool> hit;
	}tree;
	int looktree = 0;							/*tree描画数*/
	int shadow_seminear;							/*shadow中距離*/
	int shadow_near;							/*shadow近距離*/
	int shadow_far;								/*shadowマップ用*/
	MV1Handle m_model, minmap;							/*mapモデル*/
	int texp,texo, texn, texm,texl;						/*mapテクスチャ*/
	MV1Handle sky_model;								/*skyモデル*/
	int sky_sun;								/*sunpic*/
	VECTOR lightvec;							/*light方向*/
	/*grass*/
	int grasss = 50000;							/*grassの数*/
	//std::vector<VERTEX3D> grassver;
	//std::vector<DWORD> grassind;
	VERTEX3D* grassver;							/**/
	DWORD* grassind;							/**/
	int VerBuf, IndexBuf;							/**/
	MV1Handle grass;							/*grassモデル*/
	int graph;/*画像ハンドル*/
	int IndexNum, VerNum;							/**/
	int GgHandle;								/**/
	int vnum, pnum;								/**/
	MV1_REF_POLYGONLIST RefMesh;						/**/
	//campos
	VECTOR camera, viewv, upv;						/**/
	float rat;								/**/
public:
	MAPS(int map_size,float draw_dist);
	void set_map_readyb(int set);
	bool set_map_ready(void);
	void set_camerapos(VECTOR pos, VECTOR vec, VECTOR up, float ratio);
	void set_map_shadow_near(float vier_r);
	void draw_map_track(players*player);
	void draw_map_model(void);
	void set_map_track(void);
	void draw_map_sky(void);
	void delete_map(void);
	MV1Handle& get_map_handle() & noexcept { return m_model; }
	const MV1Handle& get_map_handle() const & noexcept { return m_model; }
	int get_minmap() { return texp; }
	int get_map_shadow_far() { return shadow_far; }
	int get_map_shadow_near() { return shadow_near; }
	int get_map_shadow_seminear() { return shadow_seminear; }
	void set_hitplayer(VECTOR pos);
	void draw_trees(void);
	void draw_grass(void);
};
class UIS {
private:
	/**/
	int ui_reload[4] = { 0 };						/*弾UI*/
	std::vector<int> UI_body;						/*弾UI*/
	std::vector<int> UI_turret;						/*弾UI*/
	struct country { int ui_sight[8] = { 0 }; };/*改善*/
	std::vector<country> UI_main;						/*国別UI*/
	int countries = 1;							/*国数*/
	float gearf = 0.f;							/*変速*/
	float recs = 0.f;							/*跳弾表現用*/
	players *pplayer;							/*playerdata*/
	/*debug*/
	float deb[60][6 + 1]{ 0.f };
	LONGLONG waypoint{ 0 };							/*時間取得*/
	float waydeb[6]{ 0 };
	int seldeb{ 0 };
public:
	UIS();
	void draw_load(void);								/*ロード画面*/
	void set_state(players* play);							/*使用するポインタの指定*/
	void set_reco(void);								/*反射スイッチ*/
	void draw_sight(float posx, float posy, float ratio, float dist, int font);	/*照準UI*/
	void draw_ui(int selfammo,float y_v);							/*メインUI*/
	void put_way(void);
	void end_way(void);
	void debug(float fps, float time);
};
/**/
void setcv(float neard, float fard, VECTOR cam, VECTOR view, VECTOR up, float fov);//カメラ情報指定
void getdist(VECTOR *startpos, VECTOR vector, float *dist, float speed, float fps);//startposに測距情報を出力
//effect
void set_effect(EffectS *efh,VECTOR pos,VECTOR nor);
void set_pos_effect(EffectS *efh, int handle);
//play_class予定
void set_normal(float* xnor, float* znor, int maphandle, VECTOR position);
bool get_reco(players* play, players* tgt, int i,int guns);
void set_gunrad(players *play, float rat_r);
bool set_shift(players *play);
//
#endif
