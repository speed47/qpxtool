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

#include <stdio.h>
#include <qscan_plugin.h>

static const int SPEEDS_ERRC_CD[] = {
	 8*CD_SPEED_MULT,
	12*CD_SPEED_MULT,
	24*CD_SPEED_MULT,
	32*CD_SPEED_MULT,
	40*CD_SPEED_MULT,
	48*CD_SPEED_MULT,
	0
};

static const int SPEEDS_ERRC_DVD[] = {
	 1*DVD_SPEED_MULT,
	 2*DVD_SPEED_MULT,
	 4*DVD_SPEED_MULT,
	 6*DVD_SPEED_MULT,
	 8*DVD_SPEED_MULT,
	12*DVD_SPEED_MULT,
	16*DVD_SPEED_MULT,
	0
};

scan_plugin*	plugin_create(drive_info* idev)
{
	return new scan_benq(idev);
}

void plugin_destroy(scan_plugin* iplugin)
{
	if (iplugin != NULL) delete iplugin;
}

scan_benq::scan_benq(drive_info* idev)
    : scan_plugin()
{
	dev = idev;
	if (!dev->silent) printf("scan_benq()\n");
	devlist = (drivedesc*) &drivelist; 
	test=0;
}

scan_benq::~scan_benq()
{
	if (!dev->silent) printf("~scan_benq()\n");
}


int  scan_benq::probe_drive() {
	if (dev->media.type & DISC_CD) {
		int spd=8;
		if (cmd_cd_errc_init(spd)) return DEV_FAIL;
		if (cmd_cd_end()) return DEV_FAIL;
	} else if (dev->media.type & DISC_DVD) {
		int spd=4;
		if (cmd_dvd_errc_init(spd)) return DEV_FAIL;
		if (cmd_dvd_end()) return DEV_FAIL;
	} else {
		return DEV_FAIL;
	}
	return DEV_PROBED;
}


int  scan_benq::errc_data()
{
	if (dev->media.type & DISC_CD) {
		return (ERRC_DATA_BLER|ERRC_DATA_E11|ERRC_DATA_E21|ERRC_DATA_E31|ERRC_DATA_E12|ERRC_DATA_E22|ERRC_DATA_E32|ERRC_DATA_UNCR);
	} else if (dev->media.type & DISC_DVD) {
		return (ERRC_DATA_PIE|ERRC_DATA_PIF|ERRC_DATA_POE|ERRC_DATA_POF|ERRC_DATA_UNCR);
	}
	return 0;
}

int  scan_benq::check_test(unsigned int itest)
{
	switch (itest) {
		case CHK_ERRC:
			return 0;
		case CHK_FETE:
//			if (dev->media.dstatus) return -1;
			if (dev->media.type & DISC_DVD & ~DISC_DVDROM)
				return 0;
			break;
		default:
			break;
	}
	return -1;
}

int* scan_benq::get_test_speeds(unsigned int itest)
{
	switch (itest) {
		case CHK_ERRC:
		case CHK_JB:
			if (dev->media.type & DISC_CD)
				return (int*)SPEEDS_ERRC_CD;
			if (dev->media.type & DISC_DVD)
				return (int*)SPEEDS_ERRC_DVD;
			break;
		case CHK_FETE:
			if (dev->media.type & DISC_DVD)
				// just use available write speeds for FE/TE test
				return NULL;
			break;
		default:
			break;
	}
	return NULL;
}

int  scan_benq::start_test(unsigned int itest, long ilba, int &speed)
{
	int r=-1;
	sidx=0;
	switch (itest) {
		case CHK_ERRC_CD:
			lba=ilba;
			r = cmd_cd_errc_init(speed);
			break;
		case CHK_ERRC_DVD:
			lba=ilba;
			r = cmd_dvd_errc_init(speed);
			break;
		case CHK_FETE:
//			if (dev->media.dstatus) return -1;
			if (dev->media.type & DISC_DVD & ~DISC_DVDROM) {
				lba=ilba;
				r = cmd_dvd_fete_init(speed);
				break;
			} else {
				return 0;
			}
		default:
			return -1;
	}
	if (!r) {
		test = itest;
		return r;
	} else {
		test = 0;
		return r;
	}
}

int  scan_benq::scan_block(void *data, long *ilba)
{
	int r=-1;
	switch (test) {
		case CHK_ERRC_CD:
			r = cmd_cd_errc_block((cd_errc*)data);
			if(ilba) *ilba = lba;
			return r;
		case CHK_ERRC_DVD:
			r = cmd_dvd_errc_block((dvd_errc*)data);
			if(ilba) *ilba = lba;
			return r;
		case CHK_FETE:
			if (dev->media.type & DISC_DVD & ~DISC_DVDROM) {
				r = cmd_dvd_fete_block((cdvd_ft*)data);
			} else {
				return -1;
			}
		default:
			return -1;
	}
}

int  scan_benq::end_test()
{
	int r=0;
	switch (test) {
		case CHK_ERRC_CD:
			r = cmd_cd_end();
			break;
		case CHK_ERRC_DVD:
			r = cmd_dvd_end();
			break;
		case CHK_FETE:
			r = cmd_check_mode_exit();
			break;
		default:
			break;
	}
	test=0;
	return r;
}

