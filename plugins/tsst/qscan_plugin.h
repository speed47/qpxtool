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

#ifndef __QSCAN_TSST_H
#define __QSCAN_TSST_H

#include "qpx_scan_plugin_api.h"

static const drivedesclist drivelist =
{
	{ "TSSTcorp",  DEV_TSST, "CDDVDRW SH-S202N",	TSST_H2, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "", 0, "", 0}
};

static const char plugin_name[]="TSST";
static const char plugin_desc[]="Scan plugin for Toshiba-Samsung devices";

class drive_info;

class scan_tsst : public scan_plugin {
public:
    scan_tsst(drive_info* idev);
    virtual ~scan_tsst();
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

// CD ERRC methods
	int cmd_cd_errc_init();
	int cmd_cd_errc_block(cd_errc *data);
	int cmd_cd_errc_end();

// DVD ERRC methods
	int cmd_dvd_errc_init();
	int cmd_dvd_errc_block(dvd_errc *data);
	int cmd_dvd_errc_end();

};

#endif
