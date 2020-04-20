#pragma once
#ifndef INCLUDED_thd_h_
#define INCLUDED_thd_h_

#include "define.h"
#include "useful.h"
#include <thread> 
#include <chrono>
#include <array> 
//入力
enum Keyx {
	KEY_ESCAPE = 0,
	KEY_PAUSE = 1,
	KEY_M_LEFT = 2,
	KEY_M_RIGHT = 3,
	KEY_NO_1 = 4,
	KEY_NO_2 = 5,
	KEY_NO_3 = 6,
	ACTIVE = 7,
	ON_PAD = 8,
	KEY_UP = 9,
	KEY_DOWN = 10,
	KEY_LEFT = 11,
	KEY_RIGHT = 12
};
struct input {
	std::array<bool, 20> keyget; /*キー用(一時監視)*/
	std::array<bool, 4> keyget2; /*キー用(常時監視)*/
	int m_x, m_y;
	bool next;
};
struct output {
	bool starts{ false };    /*開始フラグ*/
	bool ends{ false };    /*終了フラグ*/
	bool respawn{ false }; /*ゲームオーバーフラグ*/
	LONGLONG time;/*処理時間*/

	switches aim, map, vch; /*視点変更*/
	std::vector<pair> pssort;    /*playerソート*/
	std::vector<players> player; /*player*/
};
//60fpsを維持しつつ操作を演算(box2D込み)
class ThreadClass {
private:
	std::thread thread_1;
	void calc(input& p_in, output& p_out);
public:
	ThreadClass();
	~ThreadClass();
	void thread_start(input& p_in, output& p_out);
	void thead_stop();

};

#endif 