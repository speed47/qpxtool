/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2006 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef __pioneer_spdctl
#define __pioneer_spdctl

#define PIO_SILENT_STD	 0x00
#define PIO_SILENT_PERF	 0x01
#define PIO_SILENT_QUIET 0x02

const str_dev pioneer_silent_tbl[]={
  "Standard",
  "Performance",
  "Quiet"};

#define PIO_SPD_NOLIMIT	0x00
#define PIO_SPD_LIMIT	0x01

#define PIO_PURE_READ_STD     0x00
#define PIO_PURE_READ_MASTER  0x01
#define PIO_PURE_READ_PERFECT 0x02

const str_dev pioneer_pureread_tbl[]={
	"Standard",
	"Master",
	"Perfect"};

extern int pioneer_get_quiet(drive_info* drive);
extern int pioneer_set_quiet(drive_info* drive, char silent, bool limit, bool save);

extern int pioneer_set_silent(drive_info* drive, char silent, bool save);
extern int pioneer_set_spdlim(drive_info* drive, bool limit, bool save);
extern int pioneer_set_peakpower(drive_info* drive, bool en, bool save);
extern int pioneer_set_pureread(drive_info* drive, char pr);

#endif

