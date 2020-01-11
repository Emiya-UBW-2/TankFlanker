#pragma once

#include <sstream>
#include <memory>
#include "DxLib.h"

//pos->map
#define x_(p1) (dispx / 2 + dispy * (int)(p1) / map_x)
#define y_(p1) (dispy / 2 - dispy * (int)(p1) / map_y)
//mouse->pos
#define _2x(p1) (float)(map_x * (int)(p1 - dispx / 2) / dispy)
#define _2y(p1) (float)(map_y * (int)(dispy / 2 - p1) / dispy)
//リサイズ
#define x_r(p1) ((int)(p1)*dispx / 1920)
#define y_r(p1) ((int)(p1)*dispy / 1080)
//マウス判定
#define inm(x1, y1, x2, y2) (mousex >= x1 && mousex <= x2 && mousey >= y1 && mousey <= y2)
//その他
#define deg2rad(p1) ((p1)*DX_PI_F / 180)		//角度からラジアンに
#define rad2deg(p1) ((p1)*180 / DX_PI_F)		//ラジアンから角度に
#define differential(p1, p2, p3) p1 += ((p2)-p1) * (p3) //p2の値に近づいていく
#define cross2D(px, py, qx, qy) (qy * px - qx * py)	//2D外積


#ifndef INCLUDED_useful_h_
#define INCLUDED_useful_h_

typedef std::pair<size_t, float> pair;

std::string getright(char* p1);

size_t count_impl(std::basic_string_view<TCHAR> pattern);
size_t count_team(std::string stage);
size_t count_enemy(std::string stage);

#endif
