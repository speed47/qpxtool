/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2012 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef _COLORS_H_INCLUDED
#define _COLORS_H_INCLUDED

extern const char* COL_GR0;
extern const char* COL_RED;
extern const char* COL_GRN;
extern const char* COL_YEL;
extern const char* COL_BLU;
extern const char* COL_VIOL;
extern const char* COL_CYAN;
extern const char* COL_GR1;
extern const char* COL_NO__;
extern const char* COL_WHITE;
extern const char* COL_NORM;

// Call once at startup; disables color output if NO_COLOR is set.
// See https://no-color.org/
void colors_init();

#endif
