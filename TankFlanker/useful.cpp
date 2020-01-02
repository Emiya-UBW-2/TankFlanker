#include "useful.h"
//
void quick_sort(sorts sort[], int left, int right) {
	int i, j, pivot;
	if (left < right) {
		i = left;
		j = right + 1;
		pivot = left;   // 先頭要素をpivotとする
		do {
			do { ++i; } while (sort[i].distance < sort[pivot].distance);
			do { j--; } while (sort[pivot].distance < sort[j].distance);
			if (i < j) {
				swap_t(float, sort[i].distance, sort[j].distance);
				swap_t(int, sort[i].turn, sort[j].turn);
			}
		} while (i < j);
		swap_t(float, sort[pivot].distance, sort[j].distance);
		swap_t(int, sort[pivot].turn, sort[j].turn);
		pivot = j;
		quick_sort(sort, left, pivot - 1);   // pivotを境に再帰的にクイックソート
		quick_sort(sort, pivot + 1, right);
	}
}
const char* getstr(const char * p1, int p2, const char * p3) {
	std::string tempname = p1 + std::to_string(p2) + p3;
	return tempname.c_str();
}
const char* getstr_2(const char* p1, std::string p2, const char* p3) {
	std::string tempname = p1 + p2 + p3;
	return tempname.c_str();
}
std::string getright(char* p1) {
	std::string tempname = p1;
	return tempname.substr(tempname.find('=') + 1);
}
//