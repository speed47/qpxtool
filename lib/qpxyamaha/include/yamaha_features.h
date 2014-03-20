/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2007 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 *
 */

#ifndef __YAMAHA_FEATURES_H
#define __YAMAHA_FEATURES_H

typedef char tattoo_row[3744];

// AMQR
extern int yamaha_check_amqr(drive_info* drive);
extern int yamaha_set_amqr(drive_info* drive);

// Force Speed
extern int yamaha_check_forcespeed(drive_info* drive);
extern int yamaha_set_forcespeed(drive_info* drive);

// DiscT@2
extern int yamaha_f1_get_tattoo(drive_info* drive);
extern int yamaha_f1_do_tattoo(drive_info* drive, unsigned char *iimage, uint32_t tsize);
#endif

