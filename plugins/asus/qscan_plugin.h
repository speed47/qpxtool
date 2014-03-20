/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef __QSCAN_ASUS_H
#define __QSCAN_ASUS_H

#include "qpx_scan_plugin_api.h"

static const drivedesclist drivelist =
//static drivedesclist drivelist =
{
	{ "ASUS    ", DEV_ASUS, "DRW-1612",     ASUS_1612, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "ASUS    ", DEV_ASUS, "DRW-1814",     ASUS_1612, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "ASUS    ", DEV_ASUS, "DRW-2014S1",   ASUS_2014, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "ASUS    ", DEV_ASUS, "DRW-2014L1",   ASUS_2014, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "", 0, "", 0}
};

static const char plugin_name[]="ASUS";
static const char plugin_desc[]="Scan plugin for real ASUS devices (based on MediaTek chip)";

class drive_info;

class scan_asus : public scan_plugin {
public:
//    scan_asus(drive_info* idev=NULL);
    scan_asus(drive_info* idev);
    virtual ~scan_asus();
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

	int cmd_errc_init();
	int cmd_errc_getdata();
	int cmd_errc_end();
// CD ERRC methods
	int cmd_cd_errc_block(cd_errc *data);

// DVD ERRC methods
	int cmd_dvd_errc_block(dvd_errc *data);
// end scan
};

#endif

