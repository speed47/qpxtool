/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 * */

#ifndef DPI_METRICS_H
#define DPI_METRICS_H
//--------------------------------------------------------------
#include <QPaintDevice>
//--------------------------------------------------------------
static inline int inchesToPixelsX(float inches, QPaintDevice *device) { return qRound(inches * device->logicalDpiX()); }
static inline int inchesToPixelsY(float inches, QPaintDevice *device) { return qRound(inches * device->logicalDpiY()); }
static inline int inchesToPixelsX(float inches, int lDpiX) { return qRound(inches * lDpiX); }
static inline int inchesToPixelsY(float inches, int lDpiY) { return qRound(inches * lDpiY); }
//--------------------------------------------------------------
static inline float mmToInches(double mm) { return mm*0.039370147; }
//--------------------------------------------------------------
#endif // DPI_METRICS_H

