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

#ifndef __QSCAN_BENQ_H
#define __QSCAN_BENQ_H

#include "qpx_scan_plugin_api.h"

static const drivedesclist drivelist =
//static drivedesclist drivelist =
{
	{ "BENQ    ", DEV_BENQ_WR, "DVD DD DW1620", BENQ_DW1620, CHK_ERRC_CD | CHK_ERRC_DVD | CHK_FETE},
	{ "BENQ    ", DEV_BENQ_WR, "DVD DD DW1625", BENQ_DW1625, CHK_ERRC_CD | CHK_ERRC_DVD | CHK_FETE},
	{ "BENQ    ", DEV_BENQ_WR, "DVD DD DW1640", BENQ_DW1640, CHK_ERRC_CD | CHK_ERRC_DVD | CHK_FETE},
	{ "BENQ    ", DEV_BENQ_WR, "DVD DD DW1650", BENQ_DW1650, CHK_ERRC_CD | CHK_ERRC_DVD | CHK_FETE},
	{ "BENQ    ", DEV_BENQ_WR, "DVD DD DW1655", BENQ_DW1655, CHK_ERRC_CD | CHK_ERRC_DVD | CHK_FETE},

//	// Plextor PX-740 only works if crossflashed to BENQ DW1640
//	{ "PLEXTOR ", DEV_BENQ_WR, "DVDR   PX-740", BENQ_DW1640, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "", 0, "", 0}
};

static const char plugin_name[]="BENQ";
static const char plugin_desc[]="Scan plugin for BENQ and BENQ-based devices";

class drive_info;

class scan_benq : public scan_plugin {
public:
//    scan_benq(drive_info* idev=NULL);
    scan_benq(drive_info* idev);
    virtual ~scan_benq();
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
	int cmd_check_mode_init();
	int cmd_check_mode_exit();
	int cmd_get_result();
	int cmd_set_speed(unsigned char sidx);
	int cmd_start_errc(int lba);
	int cmd_start_fete(int lba);

	int cmd_read_block();
	int cmd_getdata();
// CD ERRC methods
	int cmd_cd_errc_init(int &speed);
	int cmd_cd_errc_block(cd_errc *data);
	int cmd_cd_end();

// DVD ERRC methods
	int cmd_dvd_errc_init(int &speed);
	int cmd_dvd_errc_block(dvd_errc *data);
	int cmd_dvd_end();

	int cmd_dvd_fete_init(int &speed);
	int cmd_dvd_fete_block(cdvd_ft *data);

	unsigned char sidx;
};

#endif
