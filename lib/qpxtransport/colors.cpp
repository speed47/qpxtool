#include <cstdlib>
#include "include/colors.h"

const char* COL_GR0 = "\E[30;60m";
const char* COL_RED = "\E[31;60m";
const char* COL_GRN = "\E[32;60m";
const char* COL_YEL = "\E[33;60m";
const char* COL_BLU = "\E[34;60m";
const char* COL_VIOL = "\E[35;60m";
const char* COL_CYAN = "\E[36;60m";
const char* COL_GR1 = "\E[37;60m";
const char* COL_NO__ = "\E[38;60m";
const char* COL_WHITE = "\E[39;60m";
const char* COL_NORM = "\E[0m";

void colors_init() {
	if (getenv("NO_COLOR")) {
		COL_GR0 = "";
		COL_RED = "";
		COL_GRN = "";
		COL_YEL = "";
		COL_BLU = "";
		COL_VIOL = "";
		COL_CYAN = "";
		COL_GR1 = "";
		COL_NO__ = "";
		COL_WHITE = "";
		COL_NORM = "";
	}
}
