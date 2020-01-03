#pragma once
#define dispx		(GetSystemMetrics(SM_CXSCREEN)/1)			/*�`��X*/
#define dispy		(GetSystemMetrics(SM_CYSCREEN)/1)			/*�`��Y*/
#define M_GR		-9.8f							/*�d�͉����x*/
#define waypc		4							/*�ړ��m�ې�*/
#define ammoc		16							/*�C�e�m�ې�*/
#define animes		4							/*�l�A�j���[�V����*/
#define voice		1							/*�{�C�X*/
#define map_x		1000							/*�}�b�v�T�C�YX*/
#define map_y		1000							/*�}�b�v�T�C�YY*/
#define TEAM		1							/*����ID*/
#define ENEMY		2							/*�GID*/
#define EXTEND		4							/*�u���[���p*/
#define gunc		2							/*�e�A�C�̐�*/
#define divi		2							/*�l�̕�������*/

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
/*�\����*/
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
	effects		= 9,//�ǂݍ���
	ef_smoke3	= 9,//�ǂݍ��܂Ȃ�
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
	std::string name;						/*���O*/
	int countryc;							/*��*/
	MV1Handle model;							/*���f��*/
	MV1Handle colmodel;						/*�R���W����*/
	MV1Handle inmodel;						/*����*/
	float spdflont[4] = { 0.0f };					/*�O�i*/
	float spdback[4] = { 0.0f };					/*���*/
	float vehicle_RD = 0.0f;					/*���񑬓x*/
	float armer[4] = { 0 };						/*���b*/
	bool gun_lim_LR = 0;						/*�C���������̗L��*/
	float gun_lim_[4] = { 0.f };					/*�C�����񐧌�*/
	float gun_RD = 0.0f;						/*�C�����񑬓x*/
	int reloadtime[gunc]{ 0 };					/*�����[�h�^�C��*/
	float ammosize = 0.0f;						/*�C���a*/
	float gun_speed[3] = { 0.0f };					/*�e��*/
	float pene[3] = { 0.0f };					/*�ђ�*/
	int ammotype[3] = { 0 };					/*�e��*/
	std::vector<VECTOR> loc;					/*�t���[���̌����W*/
	VECTOR coloc[4] = { VGet(0,0,0) };				/*box2D�p�t���[��*/
	int frames{ 0 };
	int colmeshes{ 0 };
	int meshes{ 0 };
};
static_assert(std::is_move_constructible_v<vehicle>);
struct players {
	/*���*/
	int use{ 0 };							/*�g�p�ԗ�*/
	vehicle* ptr;							/*vehicle*/
	MV1Handle obj;							/*���f��*/
	MV1Handle colobj;						/*�R���W����*/
	MV1Handle hitpic[3];							/*�e�����f��*/
	char type{ 0 };							/*�G��������*/
	int se[50]{ 0 };						/*SE*/
	/**/
	int move{ 0 };							/*�L�[����*/
	VECTOR pos{ VGet(0, 0, 0) };					/*���W*/
	MATRIX ps_m;							/*�ԑ̍s��*/
	MATRIX ps_t;							/*�C���s��*/
	//std::vector<MATRIX> ps_all;					/*�s��*/
	float yace{ 0.f };						/*�����x*/
	float speed{ 0.f }, speedrec{ 0.f }, flont{ 0.f }, back{ 0.f };	/*���x�֘A*/
	VECTOR vec{ VGet(0, 0, 0) };					/*�ړ��x�N�g��*/
	float xnor{ 0.f }, znor{ 0.f }, znorrec{ 0.f };			/*�@���p�x*/
	VECTOR nor{ VGet(0, 0, 0) };					/*�@��*/
	float yrad{ 0.f };						/*�p�x*/
	float yadd{ 0.f };						/*�p���x*/
	int recoall{ 0 };						/*�e���p�x*/
	int firerad{ 0 };						/*�����p�x*/
	float recorad{ 0.f };						/*�e������*/
	/*cpu�֘A*/
	int atkf{ -1 };							/*cpu�̃w�C�g*/
	int aim{ 0 };							/*�w�C�g�̕ύX�J�E���g*/
	int wayselect{ 0 }, waynow{ 0 };				/**/
	VECTOR waypos[waypc]{ VGet(0, 0, 0) };				/*�E�F�C�|�C���g*/
	int wayspd[waypc]{ 0 };						/*���x�w��*/
	int state{ 0 };							/*�X�e�[�^�X*/
	/**/
	std::vector<ammos> Ammo;					/*�m�ۂ���e(array�ł������H)*/
	std::vector<float> Springs;					/*�X�v�����O*/
	std::vector<short> HP;						/*���C�t*/
	std::vector<pair> hitssort;					/*������������*/
	/**/
	int gear{ 0 };							/*�ϑ�*/
	unsigned int gearu{ 0 };					/*�L�[*/
	unsigned int geard{ 0 };					/*�L�[*/
	VECTOR inertia{ VGet(0, 0, 0) };				/*����*/
	float wheelrad[3]{ 0.f };					/*���т̐���*/
	VECTOR gunrad{ 0.f };						/*�C�p�x*/
	float fired{ 0.f };						/*����*/
	/*�e�֘A*/
	int ammotype{ 0 };						/*�e��*/
	int loadcnt[gunc]{ 0 };						/*���Ă�J�E���^�[*/
	int useammo[gunc]{ 0 };						/*�g�p�e*/
	bool recoadd{ false };						/*�e���t���O*/
	bool hitadd{ false };						/*�����t���O*/
	VECTOR iconpos{ VGet(0, 0, 0) };				/*UI�p*/
	EffectS effcs[efs_user];					/*effect*/
	int usepic[3];							/*�g�p�t���[��*/
	int hitbuf;							/*�g�p�e��*/
	/*box2d*/
	b2Body* body;							/**/
	b2FixtureDef fixtureDef;					/*���I�{�f�B�t�B�N�X�`�����`���܂��B*/
	b2PolygonShape dynamicBox;					/*�_�C�i�~�b�N�{�f�B�ɕʂ̃{�b�N�X�V�F�C�v���`���܂��B*/
	b2Fixture *playerfix;						/**/
	b2BodyDef bodyDef;						/*�_�C�i�~�b�N�{�f�B���`���܂��B���̈ʒu��ݒ肵�A�{�f�B�t�@�N�g�����Ăяo���܂��B*/
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
	bool usegrab{ false };							/*�l�̕������Z�̃I�t�A�I���A��l�����I��*/
	unsigned char ANTI{ 1 };						/*�A���`�G�C���A�X�{��*/
	bool YSync{ true };							/*��������*/
	float f_rate{ 60.f };							/*fps*/
	bool windowmode{ false };						/*�E�B���h�Eor�S���*/
	float drawdist{ 100.0f };						/*�؂̕`�拗��*/
	int gndx = 8;
	/**/
	std::vector<vehicle> vecs;						/*���p���*/
	VECTOR view, view_r;							/*�ʏ펋�_�̊p�x�A����*/
	std::vector<int> fonts;							/*�t�H���g*/
	int se_[13];								/*���ʉ�*/
	int ui_reload[4] = { 0 };						/*UI�p*/
	int effHndle[effects] = { 0 };						/*�G�t�F�N�g���\�[�X*/
public:
	Myclass();
	bool get_usegrab(void) { return usegrab; }
	int get_gndx(void) { return gndx; }
	float get_drawdist(void) { return drawdist; }
	float get_f_rate(void) { return f_rate; }
	void write_option(void);						//������
	//bool set_fonts(int arg_num, ...);					//(�K�v�ȃt�H���g��,�T�C�Y1,�T�C�Y2, ...)
	template<typename ...Args>
	void set_fonts(Args&& ...args)
	{
		SetUseASyncLoadFlag(true);
		auto create = [](int value) { return DxLib::CreateFontToHandle(NULL, x_r(value), y_r(value / 3), DX_FONTTYPE_ANTIALIASING_EDGE); }
		this->fonts.emplace(this->fonts.end(), create(args)...);
		SetUseASyncLoadFlag(false);
	}

	bool set_veh(void);
	int window_choosev(void);						//�ԗ��I��
	void set_viewrad(VECTOR vv);
	void set_view_r(void);
	void Screen_Flip(LONGLONG waits);
	~Myclass();
	void set_se_vol(unsigned char size);
	void play_sound(int p1);
	int* get_ui2(void) { return ui_reload; }
	int get_font(int p1) { return fonts[p1]; }				//�t�H���g�n���h�����o��
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
	int human{ 0 };								/*����l��*/
	bool usegrab{ false };							/*�l�̕������Z�̃I�t�A�I���A��l�����I��*/
	float f_rate{ 60.f };							/*fps*/

	MV1Handle inmodel_handle;						//�����f��
	bool in_f{ false };							//���`��X�C�b�`
	int inflames;								//inmodel�̃t���[����
	std::vector<humans> hum;						/**/
	std::vector<VECTOR> locin;						/*inmodel�̃t���[��*/
	std::vector<VECTOR> pos_old;						/*inmodel�̑O��̃t���[��*/
	bool first;								//����t���O
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
	int treec = 750;							/*�؂̐�*/
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
	int looktree = 0;							/*tree�`�搔*/
	int shadow_seminear;							/*shadow������*/
	int shadow_near;							/*shadow�ߋ���*/
	int shadow_far;								/*shadow�}�b�v�p*/
	MV1Handle m_model, minmap;							/*map���f��*/
	int texp,texo, texn, texm,texl;						/*map�e�N�X�`��*/
	MV1Handle sky_model;								/*sky���f��*/
	int sky_sun;								/*sunpic*/
	VECTOR lightvec;							/*light����*/
	/*grass*/
	int grasss = 50000;							/*grass�̐�*/
	//std::vector<VERTEX3D> grassver;
	//std::vector<DWORD> grassind;
	VERTEX3D* grassver;							/**/
	DWORD* grassind;							/**/
	int VerBuf, IndexBuf;							/**/
	MV1Handle grass;							/*grass���f��*/
	int graph;/*�摜�n���h��*/
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
	int ui_reload[4] = { 0 };						/*�eUI*/
	std::vector<int> UI_body;						/*�eUI*/
	std::vector<int> UI_turret;						/*�eUI*/
	struct country { int ui_sight[8] = { 0 }; };/*���P*/
	std::vector<country> UI_main;						/*����UI*/
	int countries = 1;							/*����*/
	float gearf = 0.f;							/*�ϑ�*/
	float recs = 0.f;							/*���e�\���p*/
	players *pplayer;							/*playerdata*/
	/*debug*/
	float deb[60][6 + 1]{ 0.f };
	LONGLONG waypoint{ 0 };							/*���Ԏ擾*/
	float waydeb[6]{ 0 };
	int seldeb{ 0 };
public:
	UIS();
	void draw_load(void);								/*���[�h���*/
	void set_state(players* play);							/*�g�p����|�C���^�̎w��*/
	void set_reco(void);								/*���˃X�C�b�`*/
	void draw_sight(float posx, float posy, float ratio, float dist, int font);	/*�Ə�UI*/
	void draw_ui(int selfammo,float y_v);							/*���C��UI*/
	void put_way(void);
	void end_way(void);
	void debug(float fps, float time);
};
/**/
void setcv(float neard, float fard, VECTOR cam, VECTOR view, VECTOR up, float fov);//�J�������w��
void getdist(VECTOR *startpos, VECTOR vector, float *dist, float speed, float fps);//startpos�ɑ��������o��
//effect
void set_effect(EffectS *efh,VECTOR pos,VECTOR nor);
void set_pos_effect(EffectS *efh, int handle);
//play_class�\��
void set_normal(float* xnor, float* znor, int maphandle, VECTOR position);
bool get_reco(players* play, players* tgt, int i,int guns);
void set_gunrad(players *play, float rat_r);
bool set_shift(players *play);
//
#endif
