#include "useful.h"
//
size_t count_impl(std::basic_string_view<TCHAR> pattern) {
	WIN32_FIND_DATA win32fdt;
	size_t cnt = 0;
	const auto hFind = FindFirstFile(pattern.data(), &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (win32fdt.cFileName[0] != '.') {
				++cnt;
			}
		} while (FindNextFile(hFind, &win32fdt));
	}
	FindClose(hFind);
	return cnt;
}
size_t count_team(std::string stage) { return count_impl("stage/" + stage + "/team/*.txt"); }
size_t count_enemy(std::string stage) { return count_impl("stage/" + stage + "/enemy/*.txt"); }


std::string getright(char* p1) {
	std::string tempname = p1;
	return tempname.substr(tempname.find('=') + 1);
}
