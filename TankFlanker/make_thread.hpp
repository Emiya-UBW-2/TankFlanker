#pragma once
#ifndef INCLUDED_thd_h_
#define INCLUDED_thd_h_

#include "define.h"
#include "useful.h"
#include <thread> 
#include <chrono>
#include <array> 
//����
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
	std::array<bool, 20> keyget; /*�L�[�p(�ꎞ�Ď�)*/
	std::array<bool, 4> keyget2; /*�L�[�p(�펞�Ď�)*/
	int m_x, m_y;
	bool next;
};
struct output {
	bool starts{ false };    /*�J�n�t���O*/
	bool ends{ false };    /*�I���t���O*/
	bool respawn{ false }; /*�Q�[���I�[�o�[�t���O*/
	LONGLONG time;/*��������*/

	switches aim, map, vch; /*���_�ύX*/
	std::vector<pair> pssort;    /*player�\�[�g*/
	std::vector<players> player; /*player*/
};
//60fps���ێ�����������Z(box2D����)
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