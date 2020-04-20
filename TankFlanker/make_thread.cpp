#include "make_thread.hpp"

ThreadClass::ThreadClass() {
}
ThreadClass::~ThreadClass() {
	if (thread_1.joinable())
		thread_1.detach();
}

void ThreadClass::thread_start(input& p_in, output& p_out) {
	thread_1 = std::thread([&] { calc(p_in, p_out); });
}

void ThreadClass::thead_stop() {
	thread_1.detach();
}

void ThreadClass::calc(input& p_in, output& p_out) {
	const auto c_ffff96 = GetColor(255, 255, 150);
	const auto c_ffc896 = GetColor(255, 200, 150);
	//初期化
	do {
		p_out.starts = false;
		while (!p_out.starts) {
			auto start = std::chrono::system_clock::now(); // 計測スタート時刻を保存
			if (p_out.starts) {
				break;
			}
			while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count() < 1000000 / 60) {}
		}
		//メイン
		p_out.aim.flug = false; /*照準*/
		p_out.map.flug = false; /*マップ*/
		p_out.vch.flug = false; /**/
		while (!p_out.respawn) {
			auto start = std::chrono::system_clock::now(); // 計測スタート時刻を保存
			if (p_in.keyget2[2]) {
				p_out.ends = true;
				break;
			}
			if (p_in.keyget2[3]) {
				p_out.respawn = true;
				break;
			}

			/*指揮*/
			p_out.map.cnt = std::min<uint8_t>(p_out.map.cnt + 1, p_in.keyget[1] ? 2 : 0);
			if (p_out.map.cnt == 1) {
				p_out.map.flug ^= 1;
				mouse_event(MOUSE_MOVED | MOUSEEVENTF_ABSOLUTE, 65536 / 2, 65536 / 2, 0, 0);
			}

			p_out.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count();
			while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count() < 1000000 / 60) {}
		}
		p_in.keyget2.fill(false);
		p_in.keyget.fill(false);
		while (p_in.next) {}
		p_out.respawn = false;
	} while (!p_out.ends);
	return;
}
