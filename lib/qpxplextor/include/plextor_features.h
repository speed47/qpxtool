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
 * Some Plextor commands got from PxScan and CDVDlib (C) Alexander Noe`
 *
 */


#ifndef __PLEXTOR_FEATURES_H
#define __PLEXTOR_FEATURES_H

const unsigned char PLEX_GET_MODE	= 0x00;
const unsigned char PLEX_SET_MODE	= 0x10;

const unsigned char PLEX_MODE_SS_HIDE	= 0x01;
const unsigned char PLEX_MODE_VARIREC	= 0x02;
const unsigned char PLEX_MODE_GIGAREC	= 0x04;
const unsigned char PLEX_MODE_SILENT_DISC = 0x06;
const unsigned char PLEX_MODE_SILENT_TRAY = 0x07;
const unsigned char PLEX_MODE_SILENT	= 0x08;
const unsigned char PLEX_MODE_TESTWRITE_DVDPLUS	= 0x21;
const unsigned char PLEX_MODE_BITSET	= 0x22;
const unsigned char PLEX_MODE_SECUREC	= 0xD5;

const unsigned char PLEX_BITSET_R	= 0x0A;
const unsigned char PLEX_BITSET_RDL	= 0x0E;
const unsigned char PLEX_MODE_SPDREAD	= 0xBB;

typedef struct {
	const unsigned char val;
	char	name[4];
} REC;

// GigaRec definitions
const unsigned char GIGAREC_06  = 0x83;
const unsigned char GIGAREC_07  = 0x82;
const unsigned char GIGAREC_08  = 0x81;
const unsigned char GIGAREC_09  = 0x84;
const unsigned char GIGAREC_10  = 0x00;
const unsigned char GIGAREC_11  = 0x04;
const unsigned char GIGAREC_12  = 0x01;
const unsigned char GIGAREC_13  = 0x02;
const unsigned char GIGAREC_14  = 0x03;
const unsigned char GIGAREC_OFF = GIGAREC_10;

const REC gigarec_tbl[]={
	{ GIGAREC_06, "0.6" },
	{ GIGAREC_07, "0.7" },
	{ GIGAREC_08, "0.8" },
	{ GIGAREC_09, "0.9" },
	{ GIGAREC_10, "OFF" },
	{ GIGAREC_11, "1.1" },
	{ GIGAREC_12, "1.2" },
	{ GIGAREC_13, "1.3" },
	{ GIGAREC_14, "1.4" },
	{ 0xFF,"N/A" }
};

// VariRec definitions
const unsigned char VARIREC_CD		= 0x00;
const unsigned char VARIREC_DVD		= 0x10;

const unsigned char VARIREC_CD_STRATEGY	= 0x03;
const unsigned char VARIREC_DVD_STRATEGY	= 0x04;


const unsigned char VARIREC_PLUS_1	= 0x01;
const unsigned char VARIREC_PLUS_2	= 0x02;
const unsigned char VARIREC_PLUS_3	= 0x03;
const unsigned char VARIREC_PLUS_4	= 0x04;
const unsigned char VARIREC_NULL		= 0x00;
const unsigned char VARIREC_MINUS_1	= 0x81;
const unsigned char VARIREC_MINUS_2	= 0x82;
const unsigned char VARIREC_MINUS_3	= 0x83;
const unsigned char VARIREC_MINUS_4	= 0x84;

const REC varirec_pwr_tbl[]={
	{ VARIREC_MINUS_4,"-4" },
	{ VARIREC_MINUS_3,"-3" },
	{ VARIREC_MINUS_2,"-2" },
	{ VARIREC_MINUS_1,"-1" },
	{ VARIREC_NULL,   " 0" },
	{ VARIREC_PLUS_1, "+1" },
	{ VARIREC_PLUS_2, "+2" },
	{ VARIREC_PLUS_3, "+3" },
	{ VARIREC_PLUS_4, "+4" },
	{ 0xFF,"N/A" }
};

typedef char str16[16];

const unsigned char varirec_max_str_cd=7;
const str16 varirec_str_cd_tbl[]={
	"Default",
	"Azo",
	"Cyanine",
	"PhtaloCyanine A",
	"PhtaloCyanine B",
	"PhtaloCyanine C",
	"PhtaloCyanine D"
};

const unsigned char varirec_max_str_dvd=9;
const str16 varirec_str_dvd_tbl[]={
	"Default",
	"Strategy0",
	"Strategy1",
	"Strategy2",
	"Strategy3",
	"Strategy4",
	"Strategy5",
	"Strategy6",
	"Strategy7"
};

// Silent Mode definitions
const unsigned char SILENT_CD_WR_48X = 0x08;
const unsigned char SILENT_CD_WR_32X = 0x06;
const unsigned char SILENT_CD_WR_24X = 0x05;
const unsigned char SILENT_CD_WR_16X = 0x03;
const unsigned char SILENT_CD_WR_8X  = 0x01;
const unsigned char SILENT_CD_WR_4X  = 0x00;

const REC silent_cd_wr_tbl[]={
	{ SILENT_CD_WR_48X, "48X" },
	{ SILENT_CD_WR_32X, "32X" },
	{ SILENT_CD_WR_24X, "24X" },
	{ SILENT_CD_WR_16X, "16X" },
	{ SILENT_CD_WR_8X,   "8X" },
	{ SILENT_CD_WR_4X,   "4X" },
	{ 0xFF, "max" }
};

const unsigned char SILENT_CD_RD_48X = 0x05;
const unsigned char SILENT_CD_RD_40X = 0x04;
const unsigned char SILENT_CD_RD_32X = 0x03;
const unsigned char SILENT_CD_RD_24X = 0x02;
const unsigned char SILENT_CD_RD_8X  = 0x01;
const unsigned char SILENT_CD_RD_4X  = 0x00;

const REC silent_cd_rd_tbl[]={
	{ SILENT_CD_RD_48X, "48X" },
	{ SILENT_CD_RD_40X, "40X" },
	{ SILENT_CD_RD_32X, "32X" },
	{ SILENT_CD_RD_24X, "24X" },
	{ SILENT_CD_RD_8X,   "8X" },
	{ SILENT_CD_RD_4X,   "4X" },
	{ 0xFF, "max" }
};

/*
const unsigned char SILENT_DVD_WR_18X = 0x08;
const unsigned char SILENT_DVD_WR_16X = 0x06;
const unsigned char SILENT_DVD_WR_12X = 0x05;
const unsigned char SILENT_DVD_WR_8X  = 0x03;
const unsigned char SILENT_DVD_WR_6X  = 0x01;
const unsigned char SILENT_DVD_WR_4X  = 0x00;

const REC silent_dvd_wr_tbl[]={
	{ SILENT_DVD_WR_18X, "18X" },
	{ SILENT_DVD_WR_16X, "16X" },
	{ SILENT_DVD_WR_12X, "12X" },
	{ SILENT_DVD_WR_8X,   "8X" },
	{ SILENT_DVD_WR_6X,   "6X" },
	{ SILENT_DVD_WR_4X,   "4X" },
	{ 0xFF, "max" }
};
*/

const unsigned char SILENT_DVD_RD_16X = 0x04;
const unsigned char SILENT_DVD_RD_12X = 0x03;
const unsigned char SILENT_DVD_RD_8X  = 0x02;
const unsigned char SILENT_DVD_RD_5X  = 0x01;
const unsigned char SILENT_DVD_RD_2X  = 0x00;

const REC silent_dvd_rd_tbl[]={
	{ SILENT_DVD_RD_16X, "16X" },
	{ SILENT_DVD_RD_12X, "12X" },
	{ SILENT_DVD_RD_8X,   "8X" },
	{ SILENT_DVD_RD_5X,   "5X" },
	{ SILENT_DVD_RD_2X,   "2X" },
	{ 0xFF, "max" }
};

const unsigned char SILENT_CD          = 0x01;
const unsigned char SILENT_DVD         = 0x05;

const unsigned char SILENT_ACCESS_FAST = 0x00;
const unsigned char SILENT_ACCESS_SLOW = 0x02;

// PX-716 Autostrategy definitions
const unsigned char AS_OFF		= 0x00;
const unsigned char AS_AUTO		= 0x01;
// PX-755/PX-760 AS extentions
const unsigned char AS_FORCED	= 0x04;
const unsigned char AS_ON		= 0x08;

const unsigned char ASDB_ENABLE		= 0x01;
const unsigned char ASDB_DISABLE	= 0x00;
const unsigned char ASDB_DELETE		= 0xFF;

const unsigned char ASDB_CRE_QUICK	= 0x01;
const unsigned char ASDB_CRE_FULL	= 0x05;
const unsigned char ASDB_REPLACE	= 0x00;
const unsigned char ASDB_ADD		= 0x02;

const unsigned char AS_MEDIACK_QUICK	= 0x11;
const unsigned char AS_MEDIACK_ADV	= 0x31;

const unsigned char PLEXERASER_QUICK	= 0x11;
const unsigned char PLEXERASER_FULL	= 0x31;

extern int plextor_reboot(drive_info* drive);
extern int plextor_get_TLA(drive_info* drive);
//
extern int plextor_read_eeprom(drive_info* drive, int* len=NULL);
extern int plextor_get_life(drive_info* drive);

// PoweRec
extern int plextor_set_powerec(drive_info* drive);
extern int plextor_get_powerec(drive_info* drive);
extern int plextor_get_speeds(drive_info* drive);
// silent mode
extern void plextor_print_silentmode_state(drive_info* drive);
extern int plextor_get_silentmode(drive_info* drive);
extern int plextor_set_silentmode_tray(drive_info* drive, int disc_type, int permanent);
extern int plextor_set_silentmode_disc(drive_info* drive, int disc_type, int permanent);
extern int plextor_set_silentmode_disable(drive_info* drive, int permanent);
// GigaRec
extern void print_gigarec_value(drive_info* drive);
extern int plextor_set_gigarec(drive_info* drive);
extern int plextor_get_gigarec(drive_info* drive);
// VariRec
extern void print_varirec(drive_info* drive, int disc_type);
extern int plextor_set_varirec(drive_info* drive, int disc_type);
extern int plextor_get_varirec(drive_info* drive, int disc_type);
// SecuRec
extern void print_securec_state(drive_info* drive);
extern int plextor_get_securec_state(drive_info* drive);
extern int plextor_set_securec(drive_info* drive, char len, char* passwd);
// SpeedRead
extern void print_speedread_state(drive_info* drive);
extern int plextor_set_speedread(drive_info* drive, int state);
extern int plextor_get_speedread(drive_info* drive);
// Hide-CDR / SingleSession
extern void print_hcdr_state(drive_info* drive);
extern void print_sss_state(drive_info* drive);
extern int plextor_get_hidecdr_singlesession(drive_info* drive);
extern int plextor_set_hidecdr(drive_info* drive, int state);
extern int plextor_set_singlesession(drive_info* drive, int state);
// Bitsetting
extern int plextor_get_bitset(drive_info* drive, int disc_type);
extern int plextor_set_bitset(drive_info* drive, int disc_type);
// Simulation on DVD+
extern int plextor_get_testwrite_dvdplus(drive_info* drive);
extern int plextor_set_testwrite_dvdplus(drive_info* drive);
// PlexEraser
extern int plextor_plexeraser(drive_info* drive);
// AUTOSTRATEGY
// get/set autostrategy mode
extern int plextor_print_autostrategy_state(drive_info* drive);
extern int plextor_get_autostrategy(drive_info* drive);
extern int plextor_set_autostrategy(drive_info* drive);
// read autostrategy database
extern int plextor_get_autostrategy_db_entry_count(drive_info* drive);
extern int plextor_get_autostrategy_db(drive_info* drive);

// returns detailed strategies data
extern int plextor_get_strategy(drive_info* drive);
// loads strategy to drive
extern int plextor_add_strategy(drive_info* drive);
// clears autostrategy database
extern int plextor_clear_autostrategy_db(drive_info* drive);

// use action = ASDB_ENABLE/DISABLE/DELETE
extern int plextor_modify_autostrategy_db(drive_info* drive, int index, int action);
extern int plextor_create_strategy(drive_info* drive, int mode);
extern int plextor_media_check(drive_info* drive, int mode);

#if 0
// PX-755/760 Auth
extern int px755_do_auth(drive_info* drive);
extern int px755_get_auth_code(drive_info* drive,unsigned char* auth_code);
extern int px755_send_auth_code(drive_info* drive,unsigned char* auth_code);
extern int px755_clear_auth_status(drive_info* drive);
extern int px755_calc_auth_code(unsigned char* auth_code);
#endif

#endif //__PLEXTOR_FEATURES

