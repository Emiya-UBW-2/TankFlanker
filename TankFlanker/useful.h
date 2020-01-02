#pragma once

#include <sstream>

#define swap_t(type, a, b)	do {type t; t = a; a = b; b = t;} while(0)
//pos->map
#define x_(p1)			( dispx / 2 + dispy * (int)(p1) / map_x)
#define y_(p1)			( dispy / 2 - dispy * (int)(p1) / map_y)
//mouse->pos
#define _2x(p1)			(float)( map_x * (int)(p1-dispx/2) / dispy)
#define _2y(p1)			(float)( map_y * (int)(dispy/2-p1) / dispy)
//リサイズ
#define x_r(p1)			( (int)(p1) *dispx/ 1920)
#define y_r(p1)			( (int)(p1) *dispy/ 1080)
//マウス判定
#define inm(x1,y1,x2,y2)	(mousex >= x1 && mousex <= x2 && mousey >= y1 && mousey <= y2)

#define deg2rad(p1)	(DX_PI_F/180*(p1))
#define differential(p1,p2,p3) p1+=((p2)-p1)*(p3)
#ifndef INCLUDED_useful_h_
#define INCLUDED_useful_h_

struct sorts {
	int turn = 0;
	float distance = 0.f;
};

void quick_sort(sorts sort[], int left, int right);
const char* getstr(const char* p1, int p2, const char* p3);
const char* getstr_2(const char* p1, std::string p2, const char* p3);
std::string getright(char* p1);

#endif
