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

#if defined (_WIN32) || defined (_WIN64) 
// get header file with Sleep function
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
#include <winbase.h>  // msys/mingw32 (incompatible with Qt5!) 
#else
#include <synchapi.h> // msys2/mingw64-32 or Win32/64
#endif
// Sleep functions
#ifndef msleep
#define msleep(t) Sleep(t)
#endif
#ifndef sleep
#define sleep(t)  Sleep((t) << 10)
#endif

#else  // not WIN32 or MINGW
#ifndef msleep
#define msleep(t) usleep((t) << 10)
#endif

#endif


#endif

