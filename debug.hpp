#pragma once

#define trees  6

class DeBuG {
private:
	int frate{ 60 };
	std::vector<std::array<float, trees + 2>> deb;
	LONGLONG waypoint = 0;
	std::array<float, trees> waydeb{ 0.f };
	size_t seldeb;
	FontHandle font;
	const int fontsize = 12;
public:
	DeBuG(const float& fps_rate = 60.f) {
		frate = int(fps_rate);
		font = FontHandle::Create(fontsize, DX_FONTTYPE_EDGE);
		deb.resize(frate);
	}
	void put_way(void) {
		waypoint = GetNowHiPerformanceCount();
		seldeb = 0;
	}
	void end_way(void) {
		if (seldeb < waydeb.size()) {
			waydeb[seldeb++] = (float)(GetNowHiPerformanceCount() - waypoint) / 1000.0f;
		}
	}
	void debug(int xpos, int ypos, float time) {
		int wide = 180;
		deb[0][0] = time;
		deb[0][1] = 1000.f / GetFPS();
		for (size_t j = deb.size() - 1; j >= 1; --j) {
			deb[j][0] = deb[j - 1][0];
			deb[j][1] = deb[j - 1][1];
		}
		for (size_t i = 0; i < waydeb.size(); ++i) {
			if (seldeb - 1 <= i) {
				waydeb[i] = waydeb[seldeb - 1];
			}
			deb[0][i + 2] = waydeb[i];
			for (size_t j = deb.size() - 1; j >= 1; --j) {
				deb[j][i + 2] = deb[j - 1][i + 2];
			}
		}

		float xs = float(wide) / float(frate);
		float ys = float(int(waydeb.size() + 1) * fontsize) / float(frate);

		DrawBox(xpos, ypos, xpos + wide, ypos + int(100.f*ys), GetColor(0, 0, 0), TRUE);
		DrawBox(xpos, ypos, xpos + wide, ypos + int(100.f*ys), GetColor(255, 0, 0), FALSE);

		for (size_t j = 0; j < deb.size() - 1; ++j) {
			size_t jt = j + 1;
			for (size_t i = 0; i < waydeb.size(); ++i) {
				DXDraw::Line2D(xpos + int(float(j) * xs), ypos + int(100.f*ys) - int(deb[j][i + 2] * 5.f), xpos + int(float(jt) * xs), ypos + int(100.f*ys) - int(deb[jt][i + 2] * 5.f), GetColor(50, 128 + 127 * int(i) / 6, 50));
			}
			DXDraw::Line2D(xpos + int(float(j) * xs), ypos + int(100.f*ys) - int(deb[j][0] * 5.f), xpos + int(float(jt) * xs), ypos + int(100.f*ys) - int(deb[jt][0] * 5.f), GetColor(255, 255, 0));
			DXDraw::Line2D(xpos + int(float(j) * xs), ypos + int(100.f*ys) - int(deb[j][1] * 5.f), xpos + int(float(jt) * xs), ypos + int(100.f*ys) - int(deb[jt][1] * 5.f), GetColor(128, 255, 128));
		}
		DXDraw::Line2D(xpos, ypos + int(50.f*ys), xpos + wide, ypos + int(50.f*ys), GetColor(0, 255, 0));
		font.DrawStringFormat(xpos, ypos, GetColor(255, 255, 255), "%05.2ffps ( %.2fms)", GetFPS(), time);
		font.DrawStringFormat(xpos, ypos + fontsize, GetColor(255, 255, 255), "%d(%.2fms)", 0, waydeb[0]);
		for (size_t j = 1; j < waydeb.size(); ++j) {
			font.DrawStringFormat(xpos, ypos + int(j + 1) * fontsize, GetColor(255, 255, 255), "%d(%.2fms)", j, waydeb[j] - waydeb[j - 1u]);
		}
	}
};