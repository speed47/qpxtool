/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2007-2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef __QSCAN_BENQDVDROM_H
#define __QSCAN_BENQDVDROM_H

#include "qpx_scan_plugin_api.h"

#define cnt_max_cd  4
#define cnt_max_dvd 4

static const drivedesclist drivelist =
//static drivedesclist drivelist =
{
	{ "PLEXTOR ", DEV_BENQ_RD,    "DVD-ROM PX-130", BENQ_DV1650V, CHK_ERRC_CD | CHK_JB_CD | CHK_ERRC_DVD | CHK_JB_DVD },

	{ "", 0, "", 0}
};

static const char plugin_name[]="BENQ_DVDROM";
static const char plugin_desc[]="Scan plugin for BENQ DVD-ROM devices";

class drive_info;

class scan_benqrom : public scan_plugin {
public:
//    scan_benqrom(drive_info* idev=NULL);
    scan_benqrom(drive_info* idev);
    virtual ~scan_benqrom();
//    virtual int  check_drive();
    virtual int  probe_drive();
    virtual int  errc_data();
    virtual int  check_test(unsigned int test);
    virtual int  start_test(unsigned int test, long slba, int &speed);
    virtual int  scan_block(void* data,long* ilba);
    virtual int  end_test();

    virtual const char* name() { return plugin_name; };
    virtual const char* desc() { return plugin_desc; };
private:
	long lba;

	int cmd_scan_init();

// CD ERRC methods
	int cmd_cd_errc_block(cd_errc *data);
	int cmd_cd_jb_block(cdvd_jb *data);

// DVD ERRC methods
	int cmd_dvd_errc_block(dvd_errc *data);
	int cmd_dvd_jb_block(cdvd_jb *data);

// end scan
	int cmd_cd_end();
	int cmd_dvd_end();



	int cnt;
	int _c1[cnt_max_cd];
	int _c2[cnt_max_cd];
	//int _cu[cnt_max_cd];
	int _lba;
	int _pie[cnt_max_dvd];
	int _pif[cnt_max_dvd];
};

#endif

