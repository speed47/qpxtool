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

#ifndef __QSCAN_NEC_H
#define __QSCAN_NEC_H


//#include <stdio.h>
//#include <qpx_mmc.h>
#include "qpx_scan_plugin_api.h"

static const drivedesclist drivelist =
{
	{ "_NEC    ", DEV_NEC, "DVD_RW ND-3520", NEC_3520, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "_NEC    ", DEV_NEC, "DVD_RW ND-353", NEC_3530, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "_NEC    ", DEV_NEC, "DVD_RW ND-354", NEC_3540, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "_NEC    ", DEV_NEC, "DVD_RW ND-355", NEC_4550, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "_NEC    ", DEV_NEC, "DVD_RW ND-357", NEC_4570, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "_NEC    ", DEV_NEC, "DVD_RW ND-365", NEC_4650, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "_NEC    ", DEV_NEC, "DVD_RW ND-455", NEC_4550, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "_NEC    ", DEV_NEC, "DVD_RW ND-457", NEC_4570, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "_NEC    ", DEV_NEC, "DVD_RW ND-465", NEC_4650, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "Optiarc ", DEV_NEC, "DVD RW AD-717", NEC_4650, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "Optiarc ", DEV_NEC, "DVD RW AD-7260", NEC_4650, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "", 0, "", 0}
};

static const char plugin_name[]="NEC";
static const char plugin_desc[]="Scan plugin for NEC devices";

class drive_info;

class scan_nec : public scan_plugin {
public:
//    scan_nec(drive_info* idev=NULL);
    scan_nec(drive_info* idev);
    virtual ~scan_nec();
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

// CD ERRC methods
	int cmd_cd_errc_init();
	int cmd_cd_errc_block(cd_errc *data);

// DVD ERRC methods
	int cmd_dvd_errc_init();
	int cmd_dvd_errc_block(dvd_errc *data);

// BD ERRC methods
	int cmd_bd_errc_init();
	int cmd_bd_errc_block(bd_errc *data);

	int cmd_scan_end();
};

#endif // __QSCAN_NEC_H

