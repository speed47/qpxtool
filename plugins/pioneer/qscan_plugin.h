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

#ifndef __QSCAN_PIONEER_H
#define __QSCAN_PIONEER_H

#include "qpx_scan_plugin_api.h"

static const drivedesclist drivelist =
//static drivedesclist drivelist =
{
/*
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-102", PIO_OLD, 0 },
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-103", PIO_OLD, 0 },
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-104", PIO_OLD, 0 },
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-105", PIO_OLD, 0 },
*/
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-106", PIO_DVR_106, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-107", PIO_DVR_107, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-108", PIO_DVR_108, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-109", PIO_DVR_109, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-110", PIO_DVR_110, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-111", PIO_DVR_111, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "PIONEER ", DEV_PIONEER, "DVD-RW  DVR-112", PIO_DVR_112, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "PIONEER ", DEV_PIONEER, "",               PIO_DVR_112, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "ASUS    ", DEV_PIONEER, "DRW-0402P",      PIO_DVR_106, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "ASUS    ", DEV_PIONEER, "DRW-0804P",      PIO_DVR_107, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "ASUS    ", DEV_PIONEER, "DRW-1604P",      PIO_DVR_108, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "ASUS    ", DEV_PIONEER, "DRW-1608P",      PIO_DVR_109, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "ASUS    ", DEV_PIONEER, "DRW-1608P2",     PIO_DVR_110, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "ASUS    ", DEV_PIONEER, "DRW-1608P3",     PIO_DVR_111, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "TEAC    ", DEV_PIONEER, "DV-W50D",        PIO_DVR_106, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "TEAC    ", DEV_PIONEER, "DV-W58D",        PIO_DVR_107, CHK_ERRC_CD | CHK_ERRC_DVD },
	{ "TEAC    ", DEV_PIONEER, "DV-W516D",       PIO_DVR_108, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "PLEXTOR ", DEV_PIONEER, "DVR   PX-810",   PIO_DVR_112, CHK_ERRC_CD | CHK_ERRC_DVD },

	{ "", 0, "", 0}
};

static const char plugin_name[]="PIONEER";
static const char plugin_desc[]="Scan plugin for PIONEER and PIONEER-based devices";

class drive_info;

class scan_pioneer : public scan_plugin {
public:
//    scan_pioneer(drive_info* idev=NULL);
    scan_pioneer(drive_info* idev);
    virtual ~scan_pioneer();
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
	int cmd_cd_errc_read(unsigned char sects);
	int cmd_cd_errc_getdata(cd_errc *data);

// DVD ERRC methods
	int cmd_dvd_errc_init();
	int cmd_dvd_errc_block(dvd_errc *data);
	int cmd_dvd_errc_read(unsigned char nECC);
	int cmd_dvd_errc_getdata(dvd_errc *data);
// end scan
//	int cmd_scan_end();
};

#endif

