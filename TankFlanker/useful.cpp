#include "useful.h"
//
std::string getright(char* p1) {
	std::string tempname = p1;
	return tempname.substr(tempname.find('=') + 1);
}
