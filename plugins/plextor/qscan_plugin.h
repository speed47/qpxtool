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

#ifndef __QSCAN_PLEXTOR_H
#define __QSCAN_PLEXTOR_H


//#include <stdio.h>
//#include <qpx_mmc.h>
#include "qpx_scan_plugin_api.h"

static const drivedesclist drivelist =
//static drivedesclist drivelist =
{
	{ "PLEXTOR ", DEV_PLEXTOR, "CD-R   PREMIUM ", PLEXTOR_PREMIUM,
			CHK_ERRC_CD | CHK_JB_CD | CHK_FETE },
	{ "PLEXTOR ", DEV_PLEXTOR, "CD-R   PREMIUM2", PLEXTOR_PREMIUM2,
			CHK_ERRC_CD | CHK_JB_CD | CHK_FETE },
	{ "PLEXTOR ", DEV_PLEXTOR, "DVDR   PX-708A2", PLEXTOR_708A2, 
			CHK_ERRC_CD | CHK_JB_CD | CHK_ERRC_DVD | CHK_JB_DVD | CHK_TA_DVD | CHK_FETE },
	{ "PLEXTOR ", DEV_PLEXTOR, "DVDR   PX-712A ", PLEXTOR_712,
			CHK_ERRC_CD | CHK_JB_CD | CHK_ERRC_DVD | CHK_JB_DVD | CHK_TA_DVD | CHK_FETE },
	{ "PLEXTOR ", DEV_PLEXTOR, "DVDR   PX-714A ", PLEXTOR_716,
			CHK_ERRC_CD | CHK_JB_CD | CHK_ERRC_DVD | CHK_JB_DVD | CHK_TA_DVD | CHK_FETE },
	{ "PLEXTOR ", DEV_PLEXTOR, "DVDR   PX-716A ", PLEXTOR_716,
			CHK_ERRC_CD | CHK_JB_CD | CHK_ERRC_DVD | CHK_JB_DVD | CHK_TA_DVD | CHK_FETE },
	{ "PLEXTOR ", DEV_PLEXTOR, "DVDR   PX-716AL", PLEXTOR_716AL,
			CHK_ERRC_CD | CHK_JB_CD | CHK_ERRC_DVD | CHK_JB_DVD | CHK_TA_DVD | CHK_FETE },
	{ "PLEXTOR ", DEV_PLEXTOR, "DVDR   PX-755A ", PLEXTOR_755,
			CHK_ERRC_CD | CHK_JB_CD | CHK_ERRC_DVD | CHK_JB_DVD | CHK_TA_DVD | CHK_FETE },
	{ "PLEXTOR ", DEV_PLEXTOR, "DVDR   PX-760A ", PLEXTOR_760,
			CHK_ERRC_CD | CHK_JB_CD | CHK_ERRC_DVD | CHK_JB_DVD | CHK_TA_DVD | CHK_FETE },
	{ "", 0, "", 0}
};

// CHK_CD_ERRC | CHK_CD_JB | CHK_FETE_CD | CHK_DVD_ERRC | CHK_DVD_JB | CHK_FETE_DVD | CHK_TA_DVD },
// CHK_CD_ERRC | CHK_CD_JB | CHK_DVD_ERRC | CHK_DVD_JB | CHK_TA_DVD | CHK_FETE },

static const char plugin_name[]="PLEXTOR";
static const char plugin_desc[]="Scan plugin for PLEXTOR devices (based on SANYO chips)";

class drive_info;

class scan_plextor : public scan_plugin {
public:
//    scan_plextor(drive_info* idev=NULL);
    scan_plextor(drive_info* idev);
    virtual ~scan_plextor();
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
	int fete_idx;
	int fete_rsize;
/*
	int plextor_init_fete(fete *data);
	int plextor_init_dvd_ta();
*/

// CD ERRC methods
	int cmd_cd_errc_init();
	int cmd_cd_errc_block(cd_errc *data);
	int cmd_cd_errc_getdata(cd_errc *data);

// CD J/B methods
	int cmd_cd_jb_init();
	int cmd_cd_jb_block(cdvd_jb *data);
	int cmd_jb_getdata(cdvd_jb *data);

// DVD ERRC methods
	int cmd_dvd_errc_init();
	int cmd_dvd_errc_block(dvd_errc *data);
	int cmd_dvd_errc_getdata(dvd_errc *data);

// DVD J/B methods
	int cmd_dvd_jb_init();
	int cmd_dvd_jb_block(cdvd_jb *data);
// end scan
	int cmd_scan_end();

// FE/TE commands
	int cmd_fete_init();
	int cmd_fete_block(cdvd_ft *data);
	int cmd_fete_getdata();
	int cmd_fete_end();

// Time Analyser commands
	int cmd_dvd_ta_block(cdvd_ta *data);

	int build_TA_histogram_px716(unsigned char* response_data, int* dest_pit, int* dest_land, int len);
	int build_TA_histogram_px755(unsigned char* response_data, int* dest_pit, int* dest_land, int len, int dt);
	int evaluate_histogramme(cdvd_ta* data, int** peaks, int** mins);
};

#endif // __QSCAN_PLEXTOR_H

