/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef __qpxtool_mmc_defs_h
#define __qpxtool_mmc_defs_h

//#ifdef HAVE_LIMITS_H
#include <limits.h>
//#endif
#include <inttypes.h>

#include "common_functions.h"
#include "qpx_opcodes.h"

typedef char	str_vendor[9];
typedef char	str_dev[16];

struct dev_desc {
	str_dev name;
	int len;
};

typedef char	str_if[16];
typedef char	str_lo[64];
typedef char	str8[8];
typedef char	str_32[32];

typedef struct {
	uint8_t id;
	str_lo	name;
} desc8;

typedef struct {
	uint16_t id;
	str_lo	name;
} desc16;

typedef struct {
	uint32_t id;
	str_lo	name;
} desc32;

typedef struct {
	uint64_t id;
	str_lo	name;
} desc64;

typedef struct{
	msf	lin;
	str_lo	name;
} manuf;

static const uint32_t	rpc_phase_max=2;
static const str8	rpc_phase[rpc_phase_max+1]={
	"n/a",
	"RPC-I",
	"RPC-II"
};

static const uint32_t spindowns=16;
static const str8     spindown_tbl[spindowns+1]={
	"vendor","125ms","250ms","500ms","1s","2s","4s","8s",
	"16s","32s","1min","2min","4min","8min","16min","32min",
	"unknown"
};

static const uint32_t	iface_id_max=8;
static const str_if	iface_list[iface_id_max+2]={
	"Unspecified\0",
	"SCSI\0",
	"ATAPI\0",
	"IEEE1394-1995\0",
	"IEEE1394A\0",
	"Fibre Channel\0",
	"IEEE1394B\0",
	"Serial ATAPI\0",
	"USB (1.1/2.0)\0",
	"<Unknown>\0",
};

static const	str_if	disc_status_list[4]= {
	"Blank",
	"Incomplete",
	"Finalized",
	"Random Access",
//	"[READ_DISK_INFO error]",
};

static const	str_if	session_status_list[4]= {
	"Empty",
	"Incomplete",
	"Reserved",
	"Complete",
//	"[READ_DISK_INFO error]",
};

static const	str_lo	loader_list[8]={
	"Caddy/Slot",
	"Tray",
	"Pop-up",
	"<reserved>",
	"Changer (individual discs)",
	"Changer (cartridge mechanism)",
	"<reserved>",
	"<reserved>"
};

static const   str8 track_mode[16]={
	"Blank","<1>","Audio","<3>",
	"Data","<5>","<6>","Packet",
	"<8>","<9>","<10>","<11>",
	"<12>","<13>","<14>","<15>"
};

static const   str8 data_mode[16]={
	"<0>","Mode1","Mode2","<3>",
	"<4>","<5>","<6>","<7>",
	"<8>","<9>","<10>","<11>",
	"<12>","<13>","<14>","<15>"
};

#define EVENT_NO_CHANGE		0x00
#define EVENT_EJECT			0x01
#define EVENT_MEDIA_NEW		0x02
#define EVENT_MEDIA_REMOVED	0x03
#define EVENT_MEDIA_CHANGED	0x04
#define EVENT_MEDIA_BG_DONE	0x05
#define EVENT_MEDIA_BG_REST	0x06

static const	str_32 media_event[16]={
	"No Change", "Eject requested", "New Media", "Media Removal",
	"Media Changed", "BGformat done", "BGformat restart", "<res 7>",
	"<res 8>", "<res 9>", "<res A>", "<res B>",
	"<res C>", "<res D>", "<res E>", "<res F>"
};

static const	str_32 cdrw_subtype_tbl[8]={
	"Normal Speed",
	"High Speed",
	"Ultra Speed",
	"Ultra Speed+",
	"<res 4>",
	"<res 5>",
	"<res 6>",
	"<res 7>"
};

static const	str_32 cdr_subtype_tbl[8]={
	"<res 0>",
	"<res 1>",
	"Type A, low beta (A-)",
	"Type A, high beta (A+)",
	"Type B, low beta (B-)",
	"Type B, high beta (B+)",
	"Type C, low beta (C-)",
	"Type C, high beta (C+)"
};


static const	manuf	mi[]={
	{ { 97, 22, 60 }, "Acer" },
	{ { 97, 45, 20 }, "Acer" },
	{ { 97, 22, 20 }, "Advanced Digital Media" },
	{ { 97, 42, 20 }, "Advanced Digital Media" },
	{ { 97, 25, 50 }, "AMS" },
	{ { 97, 23, 30 }, "AUDIO DISTRIBUTORS" },
	{ { 97, 28, 30 }, "Auvistar" },
	{ { 97, 46, 50 }, "Auvistar" },

	{ { 97, 22, 40 }, "CIS Technology" },
	{ { 97, 45, 40 }, "CIS Technology" },
	{ { 97, 26, 60 }, "CMC Magnetics" },
	{ { 97, 46, 60 }, "CMC Magnetics" },
	{ { 97, 23, 60 }, "Customer Pressing Oosterhout" },

	{ { 97, 28, 50 }, "DELPHI" },
	{ { 97, 27,  0 }, "DIGITAL STORAGE" },
	{ { 97, 48, 40 }, "DIGITAL STORAGE" },
	{ { 97, 23, 10 }, "Doremi" },
	{ { 97, 13, 30 }, "DST" },

	{ { 97, 22, 30 }, "EXIMPO" },

	{ { 97, 26,  0 }, "FORNET" },
	{ { 97, 45,  0 }, "FORNET" },
	{ { 97, 26, 40 }, "FUJI" },
	{ { 97, 46, 40 }, "FUJI" },

	{ { 97, 29, 50 }, "General Magnetics" },
	{ { 97, 28, 10 }, "GIGASTORAGE" },
	{ { 97, 49, 10 }, "GIGASTORAGE" },
	{ { 97, 16, 30 }, "Grand Advance" },
	{ { 97, 31, 30 }, "Grand Advance" },
	{ { 97, 51, 10 }, "Grand Advance" },
	{ { 97, 45, 50 }, "Guann Yinn" },
	{ { 97, 24, 50 }, "Guann Yinn" },

	{ { 97, 24, 60 }, "Harmonic Hall" },
	{ { 97, 29,  0 }, "Harmonic Hall" },
	{ { 97, 29, 30 }, "Hile Optical Disc" },
	{ { 97, 51, 50 }, "Hile Optical Disc" },
	{ { 97, 25, 20 }, "Hitachi Maxell" },
	{ { 97, 47, 10 }, "Hitachi Maxell" },

	{ { 97, 25, 30 }, "INFODISC" },
	{ { 97, 51, 20 }, "INFODISC" },

	{ { 97, 24, 40 }, "kdg mediatech" },
	{ { 97, 28, 40 }, "King Pro" },
	{ { 97, 49, 20 }, "King Pro" },
	{ { 97, 27, 40 }, "Kodak" },
	{ { 97, 48, 10 }, "Kodak" },

	{ { 97, 26, 50 }, "Lead Data" },
	{ { 97, 48, 60 }, "Lead Data" },

	{ { 97, 23,  0 }, "Matsushita" },
	{ { 97, 49, 60 }, "Matsushita" },
	{ { 97, 15, 20 }, "Mitsubishi" },
	{ { 97, 34, 20 }, "Mitsubishi" },
	{ { 97, 50, 20 }, "Mitsubishi" },
	{ { 97, 27, 50 }, "Mitsui" },
	{ { 97, 48, 50 }, "Mitsui" },
	{ { 97, 17,  0 }, "Moser Baer India" },
	{ { 97, 25,  0 }, "MPO" },
	{ { 97, 28, 20 }, "MultiMediaMasters & Machinery" },
	{ { 97, 46, 20 }, "MultiMediaMasters & Machinery" },

	{ { 97, 23, 20 }, "Nacar Media srl" },
	{ { 97, 15, 30 }, "NAN-YA" },

	{ { 97, 28,  0 }, "Opti.Me.S. S.p.A." },
	{ { 97, 49, 30 }, "Opti.Me.S. S.p.A." },
	{ { 97, 21, 40 }, "Optical Disc Manuf." },
	{ { 97, 26, 30 }, "OPTICAL DISC CORPRATION" },
	{ { 97, 23, 50 }, "OPTROM.INC." },

	{ { 97, 27, 30 }, "Pioneer" },
	{ { 97, 48, 30 }, "Pioneer" },
	{ { 97, 27, 10 }, "Plasmon" },
	{ { 97, 48, 20 }, "Plasmon" },
	{ { 97, 26, 10 }, "POSTECH" },
	{ { 97, 47, 40 }, "POSTECH" },
	{ { 97, 27, 20 }, "Princo" },
	{ { 97, 47, 20 }, "Princo" },
	{ { 96, 43, 20 }, "Prodisc" },
	{ { 96, 43, 30 }, "Prodisc" }, // Ritek ?
	{ { 97, 32, 10 }, "Prodisc" },
	{ { 97, 47, 60 }, "Prodisc" },

	{ { 97, 27, 60 }, "Ricoh" },
	{ { 97, 48,  0 }, "Ricoh" },
	{ { 97, 15, 10 }, "Ritek" },
	{ { 97, 31,  0 }, "Ritek" },
	{ { 97, 47, 50 }, "Ritek" },

	{ { 97, 22, 10 }, "Seantram" },
	{ { 97, 16, 20 }, "SHENZEN SG&GAST" },
	{ { 97, 26, 20 }, "SKC" },
	{ { 97, 24, 10 }, "SONY" },
	{ { 97, 46, 10 }, "SONY" },

	{ { 97, 29,  0 }, "Taeil Media" },
	{ { 97, 24,  0 }, "Taiyo Yuden" },
	{ { 97, 46,  0 }, "Taiyo Yuden" },
	{ { 97, 18, 60 }, "TAROKO" },
	{ { 97, 15,  0 }, "TDK Corporation" },
	{ { 97, 32,  0 }, "TDK Corporation" },
	{ { 97, 49,  0 }, "TDK Corporation" },

	{ { 97, 29, 20 }, "UNIDISC" },
	{ { 97, 24, 30 }, "UNITECH" },
	{ { 97, 45, 10 }, "UNITECH" },

	{ { 97, 29, 10 }, "Vanguard" },
	{ { 97, 50, 10 }, "Vanguard" },
	{ { 97, 49, 40 }, "VICTOR" },
	{ { 97, 23, 40 }, "VICTOR" },
	{ { 97, 29, 40 }, "VIVA MAGNETICS" },
	{ { 97, 25, 40 }, "VIVASTAR" },

	{ { 97, 25, 60 }, "Xcitec" },
	{ { 97, 45, 60 }, "Xcitec" },

	{ {  0,  0,  0 }, "UNKNOWN" }
};

#define MODE_PAGE_RW_ERR_CTL			0x01
#define MODE_PAGE_MTRAINIER				0x02
#define MODE_PAGE_WRITE_PARAMETERS		0x05
#define MODE_PAGE_VERIFY_ERR_RECOVERY	0x07
#define MODE_PAGE_CACHING				0x08
#define MODE_PAGE_MEDIUM_SUPPORTED		0x0B
#define MODE_PAGE_CD_DEV_PARANETERS		0x0D
#define MODE_PAGE_CD_AUDIO_CTL			0x0E
#define MODE_PAGE_POWER_CONDITION		0x1A
#define MODE_PAGE_FAULT_REPORTING		0x1C
#define MODE_PAGE_TIMEOUT_PROTECT		0x1D
#define MODE_PAGE_MM_CAP_STATUS			0x2A
#define MODE_PAGE_MTRAINIER2			0x2C
#define MODE_PAGE_YAMAHA_TATTOO			0x31

static const desc8 MODE_PAGES[]={
//	{ ""				, 0x00},

	{ 0x00, "vendor"},
	{ 0x01, "R/W error control"},
	{ 0x02, "Mt.Rainier"},
	{ 0x05, "Write parameters"},
	{ 0x07, "Verify error recovery"},
	{ 0x08, "Caching"},
	{ 0x0B, "Medium types supported"},
	{ 0x0D, "CD device parameters"},
	{ 0x0E, "CD Audio control"},
	{ 0x1A, "Power condition"},
	{ 0x1C, "Fault/Failure reporting"},
	{ 0x1D, "Time-out & Protect"},
	{ 0x2A, "MM Capabilities & Mechanical status"},
	{ 0x2C, "Mt.Rainier"},
	{ 0x31, "Yamaha DiscT@2"},

	{ 0x3F, "???"}
};

#define FEATURE_PROFILE_LIST			0x000
#define FEATURE_CORE					0x001
#define FEATURE_MORPHING				0x002
#define FEATURE_REMOVABLE_MEDIA			0x003
#define FEATURE_WRITE_PROTECT			0x004
#define FEATURE_RANDOM_READABLE			0x010
#define FEATURE_MULTI_READ				0x01D
#define FEATURE_CD_READ					0x01E
#define FEATURE_DVD_READ				0x01F
#define FEATURE_RANDOM_WRITABLE			0x020
#define FEATURE_INCREMENTAL_STREAMING_WRITABLE 0x021
#define FEATURE_SECTOR_ERASABLE			0x022
#define FEATURE_FORMATTABLE				0x023
#define FEATURE_DEFECT_MANAGEMENT		0x024
#define FEATURE_WRITE_ONCE				0x025
#define FEATURE_RESTRICTED_OVERWRITE	0x026
#define FEATURE_CD_RW_CAV_WRITE			0x027
#define FEATURE_MRW						0x028
#define FEATURE_ENHANCED_DEFECT_REPORTING 0x029
#define FEATURE_DVD_PLUS_RW				0x02A
#define FEATURE_DVD_PLUS_R				0x02B
#define FEATURE_RIGID_RESTRICTED_OVERWRITE 0x02C
#define FEATURE_CD_TRACK_AT_ONCE		0x02D
#define FEATURE_CD_MASTERING			0x02E
#define FEATURE_DVD_R_RW_WRITE			0x02F
#define FEATURE_DDCD_READ				0x030
#define FEATURE_DDCD_R_WRITE			0x031
#define FEATURE_DDCD_RW_WRITE			0x032
#define FEATURE_LAYER_JUMP_RECORDING	0x033
#define FEATURE_CD_RW_MEDIA_WRITE_SUPPORT 0x037
#define FEATURE_BD_R_POW				0x038
#define FEATURE_DVD_PLUS_RW_DOUBLE_LAYER 0x03A
#define FEATURE_DVD_PLUS_R_DOUBLE_LAYER	0x03B
#define FEATURE_BD_READ					0x040
#define FEATURE_BD_WRITE				0x041
#define FEATURE_HDDVD_READ				0x050
#define FEATURE_HDDVD_WRITE				0x051
#define FEATURE_HDDVD_RW_FRAGREC		0x052
#define FEATURE_POWER_MANAGEMENT		0x100
#define FEATURE_SMART					0x101
#define FEATURE_EMBEDDED_CHANGER		0x102
#define FEATURE_CD_AUDIO_ANALOG_PLAY	0x103
#define FEATURE_MICROCODE_UPGRADE		0x104
#define FEATURE_TIMEOUT					0x105
#define FEATURE_DVD_CSS					0x106
#define FEATURE_REAL_TIME_STREAMING		0x107
#define FEATURE_LOGICAL_UNIT_SERIAL_NUMBER	0x108
#define FEATURE_MEDIA_SERIAL_NUMBER		0x109
#define FEATURE_DISC_CONTROL_BLOCKS		0x10A
#define FEATURE_DVD_CPRM				0x10B
#define FEATURE_FIRMWARE_DATE			0x10C

static const desc16 FEATURES[]={
	{ 0x0000, "PROFILE_LIST"},
	{ 0x0001, "CORE"},
	{ 0x0002, "MORPHING"},
	{ 0x0003, "REMOVABLE MEDIA"},
	{ 0x0004, "WRITE PROTECT"},
	{ 0x0010, "RANDOM READABLE"},
	{ 0x001D, "MULTI READ"},
	{ 0x001E, "CD READ"},
	{ 0x001F, "DVD READ"},
	{ 0x0020, "RANDOM WRITABLE"},
	{ 0x0021, "INCREMENTAL STREAMING WRITABLE"},
	{ 0x0022, "SECTOR ERASABLE"},
	{ 0x0023, "FORMATTABLE"},
	{ 0x0024, "DEFECT MANAGEMENT"},
	{ 0x0025, "WRITE ONCE"},
	{ 0x0026, "RESTRICTED OVERWRITE"},
	{ 0x0027, "CD-RW CAV WRITE"},
	{ 0x0028, "MRW"},
	{ 0x0029, "ENHANCED DEFECT REPORTING"},
	{ 0x002A, "DVD+RW"},
	{ 0x002B, "DVD+R"},
	{ 0x002C, "RIGID RESTRICTED OVERWRITE"},
	{ 0x002D, "CD TRACK AT ONCE"},
	{ 0x002E, "CD MASTERING (SESSION AT ONCE)"},
	{ 0x002F, "DVD+R(W) WRITE"},
	{ 0x0030, "DDCD READ"},
	{ 0x0031, "DDCD-R WRITE"},
	{ 0x0032, "DDCD-RW WRITE"},
	{ 0x0033, "LAYER JUMP RECORDING"},
	{ 0x0037, "CD-RW MEDIA WRITE SUPPORT"},
	{ 0x0038, "BD-R PSEUDO-OVERWRITE (POW)"},
	{ 0x003A, "DVD+RW DOUBLE LAYER"},
	{ 0x003B, "DVD+R DOUBLE LAYER"},
	{ 0x0040, "BD READ"},
	{ 0x0041, "BD WRITE"},
	{ 0x0042, "TIME SAFE RECORDING (TSR)"},
	{ 0x0050, "HD-DVD READ"},
	{ 0x0051, "HD-DVD WRITE"},
	{ 0x0080, "HYBRID DISC READ"},
	{ 0x0100, "POWER MANAGEMENT"},
	{ 0x0101, "S.M.A.R.T."},
	{ 0x0102, "EMBEDDED CHANGER"},
	{ 0x0103, "CD AUDIO ANALOG PLAY"},
	{ 0x0104, "MICROCODE UPGRADE"},
	{ 0x0105, "TIME-OUT"},
	{ 0x0106, "DVD CSS"},
	{ 0x0107, "REAL TIME STREAMING"},
	{ 0x0108, "LOGICAL UNIT SERIAL NUMBER"},
	{ 0x0109, "MEDIA SERIAL NUMBER"},
	{ 0x010A, "DISC CONTROL BLOCKS"},
	{ 0x010B, "DVD CPRM"},
	{ 0x010C, "FIRMWARE DATE"},
	{ 0x010D, "AACS"},
	{ 0x0110, "VCPS"},

	{ 0xFF00, "PLEXTOR  Autostrategy"},
	{ 0xFFFF, "???"}
};

#define PROFILE_NONE			0x0000
#define PROFILE_NON_REMOVABLE	0x0001
#define PROFILE_REMOVABLE		0x0002
#define PROFILE_MOPTIC_E		0x0003
#define PROFILE_OPTIC_WO		0x0004
#define PROFILE_AS_MO			0x0005
#define PROFILE_CD_ROM			0x0008
#define PROFILE_CD_R			0x0009
#define PROFILE_CD_RW			0x000A
#define PROFILE_DVD_ROM			0x0010
#define PROFILE_DVD_R_SEQ		0x0011
#define PROFILE_DVD_RAM			0x0012
#define PROFILE_DVD_RW_RESTOV	0x0013
#define PROFILE_DVD_RW_SEQ		0x0014
#define PROFILE_DVD_R_DL_SEQ	0x0015
#define PROFILE_DVD_R_DL_JUMP	0x0016
#define PROFILE_DVD_RW_DL		0x0017
#define PROFILE_DVD_PLUS_RW		0x001A
#define PROFILE_DVD_PLUS_R		0x001B
#define PROFILE_DVD_PLUS_RW_DL	0x002A
#define PROFILE_DVD_PLUS_R_DL	0x002B
#define PROFILE_DDCD_ROM		0x0020
#define PROFILE_DDCD_R			0x0021
#define PROFILE_DDCD_RW			0x0022

#define PROFILE_BD_ROM			0x040
#define PROFILE_BD_R_SEQ		0x041
#define PROFILE_BD_R_RND		0x042
#define PROFILE_BD_RE			0x043

#define PROFILE_HDDVD_ROM		0x050
#define PROFILE_HDDVD_R			0x051
#define PROFILE_HDDVD_RAM		0x052
#define PROFILE_HDDVD_RW		0x053
#define PROFILE_HDDVD_R_DL		0x058
#define PROFILE_HDDVD_RW_DL		0x05A

#define PROFILE_NO_PROFILE		0xFFFF

static const desc16 PROFILES[]={
	{ 0x0000, "NONE"},
	{ 0x0001, "NON_REMOVABLE"},
	{ 0x0002, "REMOVABLE"},
	{ 0x0003, "MO-ERASABLE"},
	{ 0x0004, "MO-WO"},
	{ 0x0005, "AS-MO"},
	{ 0x0008, "CD-ROM"},
	{ 0x0009, "CD-R"},
	{ 0x000A, "CD-RW"},

	{ 0x0010, "DVD-ROM"},
	{ 0x0011, "DVD-R Sequential"},
	{ 0x0012, "DVD-RAM"},
	{ 0x0013, "DVD-RW Restricted Overwrite"},
	{ 0x0014, "DVD-RW Sequential"},
	{ 0x0015, "DVD-R/DL Sequential"},
	{ 0x0016, "DVD-R/DL Layer Jump"},
	{ 0x0017, "DVD-RW/DL"},
	{ 0x001A, "DVD+RW"},
	{ 0x001B, "DVD+R"},

	{ 0x0020, "DD CD-ROM"},
	{ 0x0021, "DD CD-R"},
	{ 0x0022, "DD CD-RW"},
	{ 0x002A, "DVD+RW/DL"},
	{ 0x002B, "DVD+R/DL"},

	{ 0x0040, "BD-ROM"},
	{ 0x0041, "BD-R Sequential"},
	{ 0x0042, "BD-R Random"},
	{ 0x0043, "BD-RE"},

	{ 0x0050, "HD DVD-ROM"},
	{ 0x0051, "HD DVD-R"},
	{ 0x0052, "HD DVD-RAM"},
	{ 0x0053, "HD DVD-RW"},
	{ 0x0058, "HD DVD-R/DL"},
	{ 0x005A, "HD DVD-RW/DL"},

	{ 0xFFFF, "NO PROFILE"}
};

static const	dev_desc _devtbl[]= {
#if defined(__linux)
		{ "hd\0", 3},  // IDE devices
		{ "scd\0", 0}, // SCSI devices, kernel 2.4
		{ "sr\0", 0 },  // SCSI devices, kernel 2.6
#elif defined(__OpenBSD__) || defined(__NetBSD__) 
		{ "rcd\0", 0 },
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
		{ "acd\0", 0 },
		{ "cd\0", 0 },
#elif (defined (__APPLE__) && defined(__MACH__))
		{ "disk\0", 0 },
#elif defined(_WIN32)
		{ "C:", 0 },
		{ "D:", 0 },
		{ "E:", 0 },
		{ "F:", 0 },
		{ "G:", 0 },
		{ "H:", 0 },
		{ "I:", 0 },
		{ "J:", 0 },
		{ "K:", 0 },
		{ "L:", 0 },
		{ "M:", 0 },
		{ "N:", 0 },
		{ "O:", 0 },
		{ "P:", 0 },
		{ "Q:", 0 },
		{ "R:", 0 },
		{ "S:", 0 },
		{ "T:", 0 },
		{ "U:", 0 },
		{ "V:", 0 },
		{ "W:", 0 },
		{ "X:", 0 },
		{ "Y:", 0 },
		{ "Z:", 0 },
#endif
		{ "", -1 }
};

#if defined (_WIN32)
#define DEVTBL_DIRECT
#endif

#define ERR_NO_DEV   0x0002
#define ERR_NO_SCSI  0x0003
#define ERR_NO_MMC   0x0004


#define WTYPE_PACKET	0
#define WTYPE_TAO		1
#define WTYPE_SAO		2
#define WTYPE_RAW		3
#define WTYPE_LJ		4

typedef struct {
	signed char wtype;
	signed char dtype;
	uint32_t	id;
	char		name[32];
} wr_mode;

// write modes
static const wr_mode wr_modes[] =
{
	{ 0, 8,  0x00000001, "Packet"},
	{ 1, 8,  0x00000002, "TAO"},
	{ 2, 8,  0x00000004, "SAO"},

	{ 2, 0,  0x00000010, "SAO/RAW"},
	{ 2, 1,  0x00000020, "SAO/16"},
	{ 2, 2,  0x00000040, "SAO/96R"},
	{ 2, 3,  0x00000080, "SAO/96P"},

	{ 3, 1,  0x00000100, "RAW/16"},
	{ 3, 2,  0x00000200, "RAW/96R"},
	{ 3, 3,  0x00000400, "RAW/96P"},

	{ 4, 8,  0x00000800, "LayerJump"},

	{ -1, -1, 0x00, ""}
};

// track/session close modes
#define CLOSE_NONE     0x00
#define CLOSE_TRACK    0x01
#define CLOSE_SESSION  0x02
#define CLOSE_FINALIZE 0x06


#define DISC_NODISC		  0

#define DISC_CDROM        1ULL
#define DISC_CDR          (1ULL <<  1)
#define DISC_CDRW         (1ULL <<  2)
#define DISC_CD			 (DISC_CDROM | DISC_CDR | DISC_CDRW)

#define DISC_CDRWSUBT     (7ULL <<  3)
#define DISC_CDRWNS       (0ULL <<  3)
#define DISC_CDRWHS       (1ULL <<  3)
#define DISC_CDRWUS       (2ULL <<  3)
#define DISC_CDRWUSP      (3ULL <<  3)

#define DISC_DVDROM       (1ULL <<  6)
#define DISC_DVDRAM       (1ULL <<  7)
#define DISC_DVDmR		  (1ULL <<  8)
#define DISC_DVDmRW		  (1ULL <<  9)
#define DISC_DVDmRWS	  (1ULL << 10)
#define DISC_DVDmRWR	  (1ULL << 11)
#define DISC_DVDmRDL	  (1ULL << 12)
#define DISC_DVDmRDLJ	  (1ULL << 13)

#define DISC_DVDpRW		  (1ULL << 14)
#define DISC_DVDpRWDL	  (1ULL << 15)
#define DISC_DVDpR		  (1ULL << 16)
#define DISC_DVDpRDL	  (1ULL << 17)

#define DISC_DVDmRWDL	  (1ULL << 31)

#define DISC_DVDminus    (DISC_DVDmR | DISC_DVDmRW | DISC_DVDmRWS | DISC_DVDmRWR | DISC_DVDmRWDL | DISC_DVDmRDL | DISC_DVDmRDLJ)
#define DISC_DVDplus     (DISC_DVDpR | DISC_DVDpRW | DISC_DVDpRDL | DISC_DVDpRWDL)
#define DISC_DVD	     (DISC_DVDROM | DISC_DVDRAM | DISC_DVDminus | DISC_DVDplus)

#define DISC_DDCD_ROM     (1ULL << 18)
#define DISC_DDCD_R       (1ULL << 19)
#define DISC_DDCD_RW      (1ULL << 20)
#define DISC_DDCD	     (DISC_DDCD_ROM | DISC_DDCD_R | DISC_DDCD_RW)

#define DISC_BD_ROM	      (1ULL << 21)
#define DISC_BD_R_SEQ	  (1ULL << 22)
#define DISC_BD_R_RND	  (1ULL << 23)
#define DISC_BD_RE	      (1ULL << 24)
#define DISC_BD          (DISC_BD_ROM | DISC_BD_R_SEQ | DISC_BD_R_RND | DISC_BD_RE)

#define DISC_HDDVD_ROM	  (1ULL << 25)
#define DISC_HDDVD_R	  (1ULL << 26)
#define DISC_HDDVD_RAM    (1ULL << 27)
#define DISC_HDDVD_RW	  (1ULL << 28)
#define DISC_HDDVD_RDL	  (1ULL << 29)
#define DISC_HDDVD_RWDL	  (1ULL << 30)
#define DISC_HDDVD       (DISC_HDDVD_ROM | DISC_HDDVD_R | DISC_HDDVD_RAM | DISC_HDDVD_RW | DISC_HDDVD_RDL | DISC_HDDVD_RWDL)

#define DISC_UN           (1ULL << 63)

static const desc64 MEDIA[]={
	{ DISC_NODISC, "No Media"},
	{ DISC_CDROM,  "CD-ROM" },
	{ DISC_CDR,    "CD-R" },
	{ DISC_CDRW,   "CD-RW" },
/*
#define DISC_CDRWSUBT   = { 0x70000000, "" },
#define DISC_CDRWMS     = { 0x10000000, "" },
#define DISC_CDRWHS     = { 0x20000000, "" },
#define DISC_CDRWUS     = { 0x30000000, "" },
#define DISC_CDRWUSP    = { 0x40000000, "" },
*/
	{ DISC_DVDROM,   "DVD-ROM" },
	{ DISC_DVDRAM,   "DVD-RAM" },
	{ DISC_DVDmR,    "DVD-R" },
	{ DISC_DVDmRW,   "DVD-RW (unknown subtype)" },
	{ DISC_DVDmRWS,  "DVD-RW (Sequential)" },
	{ DISC_DVDmRWR,  "DVD-RW (Restricted overwrite)" },
	{ DISC_DVDmRDL,  "DVD-R DL (Sequential)" },
	{ DISC_DVDmRDLJ, "DVD-R DL (Layer Jump)" },

	{ DISC_DVDpRW,   "DVD+RW" },
	{ DISC_DVDpRWDL, "DVD+RW DL" },
	{ DISC_DVDpR,    "DVD+R" },
	{ DISC_DVDpRDL,  "DVD+R DL" },

	{ DISC_DDCD_ROM, "DDCD-ROM" },
	{ DISC_DDCD_R,   "DDCD-R" },
	{ DISC_DDCD_RW,  "DDCD-RW" },

	{ DISC_BD_ROM,   "BD-ROM" },
	{ DISC_BD_R_SEQ, "BD-R (Sequential)" },
	{ DISC_BD_R_RND, "BD-R (Random)" },
	{ DISC_BD_RE,    "BD-RE" },

	{ DISC_HDDVD_ROM, "HDDVD-ROM" },
	{ DISC_HDDVD_RAM, "HDDVD-RAM" },
	{ DISC_HDDVD_R,   "HDDVD-R" },
	{ DISC_HDDVD_RW,  "HDDVD-RW" },
	{ DISC_HDDVD_RDL, "HDDVD-R DL" },
	{ DISC_HDDVD_RWDL,"HDDVD-RW DL" },

	{ DISC_UN,		 "unknown" },
	{ 0xFFFFFFFFULL, "???" }
};

#define BOOK_DVD_ROM	0x00
#define BOOK_DVD_RAM    0x01
#define BOOK_DVD_R      0x02
#define BOOK_DVD_RW     0x03
#define BOOK_HDDVD_ROM	0x04
#define BOOK_HDDVD_RAM  0x05
#define BOOK_HDDVD_R    0x06
#define BOOK_DVD_PRW    0x09
#define BOOK_DVD_PR     0x0A
#define BOOK_DVD_PRW_DL 0x0D
#define BOOK_DVD_PR_DL  0x0E

static const	str_dev book_type_tbl[16]={
	"DVD-ROM",		"DVD-RAM",		"DVD-R",	"DVD-RW",
	"HD DVD-ROM",	"HD DVD-RAM",	"HD DVD-R ","unknown",
	"unknown",		"DVD+RW",		"DVD+R",	"unknown",
	"unknown",		"DVD+RW DL",	"DVD+R DL",	"unknown"
};

static const	str_dev max_rate_tbl[16]={
	"2.52 Mbps",	"5.04 Mbps",	"10.08 Mbps",	"20.16 Mbps",
	"30.24 Mbps",	"Reserved", 	"Reserved", 	"Reserved",
	"Reserved", 	"Reserved", 	"Reserved", 	"Reserved",
	"Reserved", 	"Reserved", 	"Reserved", 	"Not specified"
};

#define COMMAND_FAILED  -0x01

#define Media_BLANK     0x80
#define Media_NOTBLANK  0x7F
#define Media_NoMedia   0x00
#define Media_CD        0x01
#define Media_DVD       0x02

#define CAP_REMOVABLE_MEDIA		1ULL
#define CAP_MORPHING			(1ULL << 1)
#define CAP_EMBEDDED_CHANGER	(1ULL << 2)
#define CAP_MICROCODE_UPGRADE	(1ULL << 3)
#define CAP_SMART				(1ULL << 4)
#define CAP_REAL_TIME_STREAMING	(1ULL << 5)
#define CAP_POWER_MANAGEMENT	(1ULL << 6)
#define CAP_DEFECT_MANAGEMENT	(1ULL << 7)
#define CAP_DVD_CSS				(1ULL << 8)
#define CAP_DVD_CPRM			(1ULL << 9)
#define CAP_C2					(1ULL << 10)
#define CAP_CD_TEXT				(1ULL << 11)
#define CAP_CD_AUDIO			(1ULL << 12)
#define CAP_DAE					(1ULL << 13)
#define CAP_ACCURATE_STREAM		(1ULL << 14)

#define CAP_COMPOSITE			(1ULL << 15)
#define CAP_DIGITAL_PORT_1		(1ULL << 16)
#define CAP_DIGITAL_PORT_2		(1ULL << 17)
#define CAP_MULTISESSION		(1ULL << 18)
#define CAP_MODE2_FORM1			(1ULL << 19)
#define CAP_MODE2_FORM2			(1ULL << 20)
#define CAP_TEST_WRITE_CD		(1ULL << 21)
#define CAP_READ_BAR_CODE		(1ULL << 22)
#define CAP_UPC					(1ULL << 23)
#define CAP_ISRC				(1ULL << 24)
#define CAP_SIDE_CHANGE			(1ULL << 25)
#define CAP_LOCK				(1ULL << 26)
#define CAP_EJECT				(1ULL << 27)
#define CAP_TEST_WRITE_DVD		(1ULL << 28)
#define CAP_SSA					(1ULL << 29)

//#define CAP_		= 0x00000000;

#define CAP_SET_CD_SPEED		(1ULL << 30)
#define NCAP_SET_CD_SPEED		(~CAP_SET_CD_SPEED);

#define CAP_TEST_WRITE_DVD_PLUS	(1ULL << 31)
#define CAP_BURN_FREE			(1ULL << 32)

static const desc64 capabilities[] = {
	{ CAP_REMOVABLE_MEDIA,		"Removable media" },
	{ CAP_COMPOSITE, 			"Composite out" },
	{ CAP_DIGITAL_PORT_1,		"Digital port 1" },
	{ CAP_DIGITAL_PORT_2,		"Digital port 2" },
	{ CAP_MULTISESSION,			"Multisession" },
	{ CAP_MODE2_FORM1,			"Mode 2 Form 1" },
	{ CAP_MODE2_FORM2,			"Mode 2 Form 2" },
	{ CAP_READ_BAR_CODE,		"Bar Code Read" },
	{ CAP_BURN_FREE,			"BURN-free" },
	{ CAP_TEST_WRITE_CD,		"Test Write CD" },
	{ CAP_TEST_WRITE_DVD,		"Test Write DVD-" },
	{ CAP_TEST_WRITE_DVD_PLUS,	"Test Write DVD+" },
	{ CAP_UPC,					"UPC" },
	{ CAP_ISRC,					"ISRC" },
	{ CAP_SIDE_CHANGE,			"Side Change" },
	{ CAP_EJECT,				"Media Eject" },
	{ CAP_LOCK,					"Media Lock" },
	{ CAP_SMART,				"S.M.A.R.T." },
	{ CAP_MICROCODE_UPGRADE,	"Firmware upgrade" },
	{ CAP_MORPHING,				"Morphing" },
	{ CAP_POWER_MANAGEMENT,		"Power Management" },
	{ CAP_EMBEDDED_CHANGER,		"Embedded Changer" },
	{ CAP_C2,					"C2 Pointers" },
	{ CAP_CD_AUDIO,				"Audio-CD" },
	{ CAP_DAE,					"DAE" },
	{ CAP_CD_TEXT,				"CD-text" },
	{ CAP_ACCURATE_STREAM,		"Accurate Stream" },
	{ CAP_DEFECT_MANAGEMENT,	"Defect Management" },
	{ CAP_REAL_TIME_STREAMING,	"Realtime Streaming" },
	{ CAP_SSA,					"Spare Area Info" },
	{ CAP_DVD_CSS,				"DVD CSS" },
	{ CAP_DVD_CPRM,				"DVD CPRM" },

	{ 0, "" }
};

//						                   RW
#define DEVICE_CD_ROM		  1ULL         // +
#define DEVICE_CD_R		      (1ULL <<  1) // ++
#define DEVICE_CD_RW		  (1ULL <<  2) // ++
#define DEVICE_DVD_ROM		  (1ULL <<  3) // +
#define DEVICE_DVD_RAM		  (1ULL <<  4) // ++
#define DEVICE_DVD_R		  (1ULL <<  5) // ++
#define DEVICE_DVD_RW		  (1ULL <<  6) // ++

//#define DEVICE_DVD_R_DL_SEQ	  (1ULL <<  7) // ++
//#define DEVICE_DVD_R_DL_LJ	  (1ULL <<  8) // ++
//#define DEVICE_DVD_R_DL		  (DEVICE_DVD_R_DL_SEQ | DEVICE_DVD_R_DL_LJ) // ++
#define DEVICE_DVD_R_DL		  (1ULL <<  7) // ++

#define DEVICE_DVD_RW_DL	  (1ULL <<  9) // ++
#define DEVICE_DVD_PLUS_R	  (1ULL << 10) // ++
#define DEVICE_DVD_PLUS_RW	  (1ULL << 11) // ++
#define DEVICE_DVD_PLUS_R_DL  (1ULL << 12) // ++
#define DEVICE_DVD_PLUS_RW_DL (1ULL << 13) // ++
#define DEVICE_DVD			 (DEVICE_DVD_ROM | DEVICE_DVD_RAM \
								| DEVICE_DVD_R | DEVICE_DVD_RW | DEVICE_DVD_R_DL | DEVICE_DVD_RW_DL \
								| DEVICE_DVD_PLUS_R | DEVICE_DVD_PLUS_RW | DEVICE_DVD_PLUS_R_DL | DEVICE_DVD_PLUS_RW_DL)
#define DEVICE_MRW			  (1ULL << 14) // ++
#define DEVICE_MRW_DVD		  (1ULL << 15) // ++
#define DEVICE_DDCD_R		  (1ULL << 16) // ++
#define DEVICE_DDCD_RW		  (1ULL << 17) // ++
#define DEVICE_BD_ROM		  (1ULL << 18) // +
#define DEVICE_BD_R			  (1ULL << 19) // ++
#define DEVICE_BD_RE		  (1ULL << 20) // ++
#define DEVICE_BD			 (DEVICE_BD_ROM | DEVICE_BD_R | DEVICE_BD_RE)

#define DEVICE_HDDVD_ROM	  (1ULL << 21) // +
#define DEVICE_HDDVD_R		  (1ULL << 22) // ++
#define DEVICE_HDDVD_RAM	  (1ULL << 23) // ++
#define DEVICE_HDDVD_RW		  (1ULL << 24) //
#define DEVICE_HDDVD_R_DL	  (1ULL << 25) //
#define DEVICE_HDDVD_RW_DL	  (1ULL << 26) //
#define DEVICE_HDDVD         (DEVICE_HDDVD_ROM \
								| DEVICE_HDDVD_RAM | DEVICE_HDDVD_R | DEVICE_HDDVD_RW \
								| DEVICE_HDDVD_R_DL | DEVICE_HDDVD_RW_DL)

#define CD_SPEED_MULT  177
#define DVD_SPEED_MULT 1385

static const desc64 rw_capabilities[] = {
//	{ DEVICE_CD_ROM,	    "CD-ROM" },
	{ DEVICE_CD_R,			"CD-R" },
	{ DEVICE_CD_RW,			"CD-RW" },
	{ DEVICE_MRW,			"CD-MRW" },
	{ DEVICE_DDCD_R,		"DDCD-R" },
	{ DEVICE_DDCD_RW,		"DDCD-RW" },
	{ 0,					"-" },

	{ DEVICE_DVD_ROM,		"DVD-ROM" },
	{ DEVICE_DVD_RAM,		"DVD-RAM" },
	{ DEVICE_DVD_R,			"DVD-R" },
	{ DEVICE_DVD_RW,		"DVD-RW" },

//	{ DEVICE_DVD_R_DL_SEQ	  (1ULL <<  7) // ++
//	{ DEVICE_DVD_R_DL_LJ	  (1ULL <<  8) // ++
//	{ DEVICE_DVD_R_DL		  (DEVICE_DVD_R_DL_SEQ | DEVICE_DVD_R_DL_LJ) // ++
	{ DEVICE_DVD_R_DL,		"DVD-R/DL" },
	{ DEVICE_DVD_RW_DL,		"DVD-RW/DL" },
	{ DEVICE_DVD_PLUS_R,	"DVD+R" },
	{ DEVICE_DVD_PLUS_RW,	"DVD+RW" },
	{ DEVICE_DVD_PLUS_R_DL,	"DVD+R/DL" },
	{ DEVICE_DVD_PLUS_RW_DL,"DVD+RW/DL" },
	{ DEVICE_MRW_DVD,		"DVD+MRW" },
	{ 0,					"-" },

	{ DEVICE_BD_ROM,		"BD-ROM" },
	{ DEVICE_BD_R,			"BD-R" },
	{ DEVICE_BD_RE,			"BD-RE" },

	{ DEVICE_HDDVD_ROM,		"HDDVD-ROM" },
	{ DEVICE_HDDVD_R,		"HDDVD-R" },
	{ DEVICE_HDDVD_RAM,		"HDDVD-RAM" },
//	{ DEVICE_HDDVD_RW,		"HDDVD-RW" },
//	{ DEVICE_HDDVD_R_DL,	"HDDVD-R/DL" },
//	{ DEVICE_HDDVD_RW_DL,   "HDDVD-RW/DL"},

	{ 0, "" }	
};

#define ERR_INVALID_OPCODE  0x00052000
#define ERR_NO_MEDIUM		0x00023A00
#define ERR_NO_ERROR		0x00000000

// **** Vendors definition 
#define DEV_GENERIC      0x00000001
#define DEV_PLEXTOR      0x00000002
#define DEV_PIONEER      0x00000004
#define DEV_NEC          0x00000008
#define DEV_LITEON       0x00000010
#define DEV_BENQ_RD      0x00000020
#define DEV_BENQ_WR      0x00000040
#define DEV_YAMAHA       0x00000080
#define DEV_ASUS         0x00000100
#define DEV_TSST         0x00000200
#define DEV_LG           0x00000400
#define DEV_TEAC         0x00000800

// Vendor-specififc features
#define PX_POWEREC		 0x00000001
#define PX_GIGAREC		 0x00000002
#define PX_VARIREC_CD	 0x00000004
#define PX_VARIREC_DVD	 0x00000008
#define PX_HCDRSS		 0x00000010
#define PX_SPDREAD		 0x00000020
#define PX_SECUREC		 0x00000040
#define PX_SILENT		 0x00000080
#define PX_ASTRATEGY	 0x00000100
#define PX_BITSET_R		 0x00000200
#define PX_BITSET_RDL	 0x00000400
#define PX_SIMUL_PLUS    0x00000800
#define PX_ERASER        0x00001000
#define PIO_QUIET	     0x00002000
#define YMH_AMQR         0x00004000
#define YMH_FORCESPEED   0x00008000
#define YMH_TATTOO		 0x00010000
#define LTN_ERASER		 0x00020000

// Yamaha devices
#define YAMAHA_OLD       0x00000001
#define YAMAHA_F1        0x00000002

// Plextor devices
#define PLEXTOR_OLD      0x00000001

#define PLEXTOR_4824     0x00000100
#define PLEXTOR_5224     0x00000200
#define PLEXTOR_PREMIUM  0x00000400
#define PLEXTOR_708      0x00000800
#define PLEXTOR_708A2    0x00001000
#define PLEXTOR_712      0x00002000
#define PLEXTOR_714      0x00004000
#define PLEXTOR_716      0x00008000
#define PLEXTOR_716AL    0x00010000
#define PLEXTOR_755      0x00020000
#define PLEXTOR_760      0x00040000
#define PLEXTOR_PREMIUM2 0x00080000

//Pioneer devices
#define PIO_OLD			 0x00000001
#define PIO_DVR_106		 0x00000002
#define PIO_DVR_107		 0x00000004
#define PIO_DVR_108		 0x00000008
#define PIO_DVR_109		 0x00000010
#define PIO_DVR_110		 0x00000020
#define PIO_DVR_111		 0x00000040
#define PIO_DVR_112		 0x00000080
#define PIO_BDR			 0x00000100

//Asus devices
#define ASUS_1612		 0x00000001
#define ASUS_2014		 0x00000002

//Nec devices
#define NEC_OLD			 0x00000001
#define NEC_3520		 0x00000002
#define NEC_3530		 0x00000004
#define NEC_3540		 0x00000008
#define NEC_4550		 0x00000010
#define NEC_4570		 0x00000020
#define NEC_4650		 0x00000040
#define NEC_7170		 0x00000080
#define NEC_7200		 0x00000100

//LiteOn devices
#define LTN_OLD			 0x00000001
#define LTN_CDR_G7		 0x00000002
#define LTN_SDVDR_G1	 0x00000004
#define LTN_SDVDR_G2	 0x00000008
#define LTN_SDVDR_G3	 0x00000010
#define LTN_DVDR_G1		 0x00000020
#define LTN_DVDR_G2		 0x00000040
#define LTN_DVDR_G3		 0x00000080
#define LTN_DVDR_G4		 0x00000100
#define LTN_DVDR_G5		 0x00000200
#define LTN_DVDR_G6		 0x00000400
#define LTN_DVDR_G7		 0x00000800
#define LTN_DVDR_G8		 0x00001000
#define LTN_BDR			 0x00002000

#define LTN_iHAx1		 0x00002000
#define LTN_iHAx2		 0x00004000
#define LTN_iHAx3		 0x00008000
#define LTN_iHAx4		 0x00010000

//BenQ devices
#define BENQ_OLD		 0x00000001
#define BENQ_DV1650V	 0x00000002
#define BENQ_DW1620		 0x00000004
#define BENQ_DW1625		 0x00000008
#define BENQ_DW1640		 0x00000010
#define BENQ_DW1650		 0x00000020
#define BENQ_DW1655		 0x00000040

#define TSST_H2		 0x00000002

// **** end of devices list

#endif

