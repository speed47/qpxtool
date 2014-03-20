/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2007-2008, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef __QSCAN_LITEON_H
#define __QSCAN_LITEON_H

#include "qpx_scan_plugin_api.h"

static const drivedesclist drivelist =
//static drivedesclist drivelist =
{
	{ "LITE-ON ",  DEV_LITEON, "LTR-52327S",		LTN_CDR_G7,  CHK_ERRC_CD },

	{ "LITE-ON ",  DEV_LITEON, "DVDRW LDW-811S",	LTN_DVDR_G1, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "LITE-ON ",  DEV_LITEON, "DVDRW LDW-451S",	LTN_DVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW LDW-851S",	LTN_DVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SOHW-812S",	LTN_DVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVD+RW SOHW-802S",	LTN_DVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SOHW-832S",	LTN_DVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVD+RW SOHW-822S",	LTN_DVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "LITE-ON ",  DEV_LITEON, "DVDRW SOHW-1213S",	LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SOHW-1613S",	LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SOHW-1633S",	LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SOHW-1653S",	LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "LITE-ON ",  DEV_LITEON, "DVDRW SOHW-1673S",	LTN_DVDR_G4, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SOHW-1693S",	LTN_DVDR_G4, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "LITE-ON ",  DEV_LITEON, "DVDRW SHW-1635S",	LTN_DVDR_G5, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SHW-16H5S",	LTN_DVDR_G5, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SHW-160P6S",	LTN_DVDR_G6, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SHW-160H6S",	LTN_DVDR_G6, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SHM-165P6S",	LTN_DVDR_G6, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW SHM-165H6S",	LTN_DVDR_G6, CHK_ERRC_CD | CHK_ERRC_DVD },


	{ "LITE-ON ",  DEV_LITEON, "DVDRW SH-16A7",	LTN_DVDR_G7,   CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW LH-16W1",	LTN_DVDR_G7,   CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW LH-16A1",	LTN_DVDR_G7,   CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW LH-18A1",	LTN_DVDR_G7,   CHK_ERRC_CD | CHK_ERRC_DVD },


	{ "LITE-ON ",  DEV_LITEON, "DVDRW LH-20A1",	LTN_DVDR_G8,   CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW DH-20A3",	LTN_DVDR_G8,   CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "LITE-ON ",  DEV_LITEON, "DVDRW DH-20A4",	LTN_DVDR_G8,   CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "ATAPI   ",  DEV_LITEON, "iHBS112",		LTN_BDR,   CHK_ERRC_CD | CHK_ERRC_DVD | CHK_ERRC_BD },
	{ "ATAPI   ",  DEV_LITEON, "iHBS212",		LTN_BDR,   CHK_ERRC_CD | CHK_ERRC_DVD | CHK_ERRC_BD },
	{ "ATAPI   ",  DEV_LITEON, "iHBS312",		LTN_BDR,   CHK_ERRC_CD | CHK_ERRC_DVD | CHK_ERRC_BD },

	{ "SONY    ",  DEV_LITEON, "DVD RW DW-U18A",	LTN_DVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DRU-700A",	LTN_DVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-D18A",	LTN_DVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "SONY    ",  DEV_LITEON, "DVD RW DW-U20A",	LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-U21A",	LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DRU-710A",	LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-D22A",	LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-D23A",	LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "SONY    ",  DEV_LITEON, "DVD RW DRU-720A",	LTN_DVDR_G4, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-D26A",	LTN_DVDR_G4, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DRU-800A",	LTN_DVDR_G4, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-Q28A",	LTN_DVDR_G4, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "SONY    ",  DEV_LITEON, "DVD RW DW-Q30A",	LTN_DVDR_G5, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-Q31A",	LTN_DVDR_G5, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-Q120A",	LTN_DVDR_G6, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-G120A",	LTN_DVDR_G6, CHK_ERRC_CD | CHK_ERRC_DVD },


	{ "TEAC    ",  DEV_LITEON, "DV-W58G",		LTN_DVDR_G1, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "TEAC    ",  DEV_LITEON, "DV-W58G-A",		LTN_DVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "TEAC    ",  DEV_LITEON, "DV-W512G",		LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "TEAC    ",  DEV_LITEON, "DV-W516G",		LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "TEAC    ",  DEV_LITEON, "DV-W516GA",		LTN_DVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "TEAC    ",  DEV_LITEON, "DV-W516GB",		LTN_DVDR_G4, CHK_ERRC_CD | CHK_ERRC_DVD },

//	{ "ASUS    ",  DEV_LITEON, "DRW-20A1",		LTN_DVDR_G7,   CHK_ERRC_CD | CHK_ERRC_DVD },
//	{ "ASUS    ",  DEV_LITEON, "DRW-20B1",		LTN_DVDR_G7,   CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "PLEXTOR ",  DEV_LITEON, "DVR   PX-806",	LTN_iHAx1,      CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "PLEXTOR ",  DEV_LITEON, "DVR   PX-850",	LTN_iHAx3,      CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "Optiarc ",  DEV_LITEON, "DVD RW AD-7191",	LTN_DVDR_G6, 	CHK_ERRC_CD | CHK_ERRC_DVD },

// Slimtype
	{ "Slimtype",  DEV_LITEON, "DVD+RW SDW-421S",	LTN_SDVDR_G1, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "Slimtype",  DEV_LITEON, "DVDRW SDW-431S",	LTN_SDVDR_G1, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "Slimtype",  DEV_LITEON, "DVDRW SOSW-852S",	LTN_SDVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "Slimtype",  DEV_LITEON, "DVD+RW SOSW-862S",	LTN_SDVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "Slimtype",  DEV_LITEON, "DVDRW SOSW-813S",	LTN_SDVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "Slimtype",  DEV_LITEON, "DVDRW SOSW-833S",	LTN_SDVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	
	{ "Slimtype",  DEV_LITEON, "DVD A  DS8A2S   ",	LTN_SDVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-D56A",	LTN_SDVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD+RW DW-R56A",	LTN_SDVDR_G2, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "SONY    ",  DEV_LITEON, "DVD RW DW-Q58A",	LTN_SDVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "SONY    ",  DEV_LITEON, "DVD RW DW-Q60A",	LTN_SDVDR_G3, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "", 0, "", 0}
};

static const drivedesclist blacklist =
{
	{ "TEAC    ",  DEV_TEAC, "CD-W552E", 0 },
	{ "HL-DT-ST",  DEV_LG,   "",		 0 },
	{ "TSSTcorp",  DEV_TSST, "",		 0 },

	{ "", 0, "", 0}
};

static const char plugin_name[]="LITEON";
static const char plugin_desc[]="Scan plugin for LITE-ON and LITE-ON-based devices";

class drive_info;

class scan_liteon : public scan_plugin {
public:
//    scan_liteon(drive_info* idev=NULL);
    scan_liteon(drive_info* idev);
    virtual ~scan_liteon();
//    virtual int  check_drive();
    virtual int  probe_drive();
    virtual int  errc_data();
    virtual int  check_test(unsigned int test);
    virtual int* get_test_speeds(unsigned int test);
    virtual int  start_test(unsigned int test, long slba, int &speed);
    virtual int  scan_block(void* data,long* ilba);
    virtual int  end_test();

    virtual const char* name() { return plugin_name; };
    virtual const char* desc() { return plugin_desc; };
private:
	long lba;
	bool cd_errc_new;

// CD ERRC methods
	int cmd_cd_errc_init_old();
	int cmd_cd_errc_init_new();
	int cmd_cd_errc_init();

	int cmd_cd_errc_read();
	int cmd_cd_errc_getdata(cd_errc *data);
	int cmd_cd_errc_inteval_rst();
	int cmd_cd_errc_block_old(cd_errc *data);
	int cmd_cd_errc_block_new(cd_errc *data);
	int cmd_cd_errc_block(cd_errc *data);

	int cmd_cd_errc_end();

// DVD ERRC methods
	int cmd_dvd_errc_init();
	int cmd_dvd_errc_block(dvd_errc *data);
	int cmd_dvd_errc_end();

// BD ERRC methods
	int cmd_bd_errc_init();
	int cmd_bd_errc_block(bd_errc *data);
	int cmd_bd_errc_end();

// FE/TE test methods
	int cmd_fete_init(int& speed);
	int cmd_fete_get_data(cdvd_ft *data);
	int cmd_fete_get_position();

	int cmd_cd_fete_init(int& speed);
	int cmd_cd_fete_block(cdvd_ft *data);

	int cmd_dvd_fete_init(int& speed);
	int cmd_dvd_fete_block(cdvd_ft *data);

	int cmd_bd_fete_init(int& speed);
	int cmd_bd_fete_block(cdvd_ft *data);
};

#endif

