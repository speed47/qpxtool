/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef _defs_h
#define _defs_h


#if defined (_WIN32)

#include <stdlib.h>

#if defined(__MINGW32__)
#define msleep(t) _sleep(t)
#define sleep(t)  _sleep((t) << 10)
#else
#define msleep(t) Sleep(t)
#define sleep(t)  Sleep((t) << 10)
#endif

#else

#ifndef msleep
#define msleep(t) usleep((t) << 10)
#endif
extern int min(int a, int b);
extern int max(int a, int b);

#endif


#endif

