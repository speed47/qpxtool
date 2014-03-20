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
	4*CD_SPEED_MULT,
	8*CD_SPEED_MULT,
	24*CD_SPEED_MULT,
	32*CD_SPEED_MULT,
	40*CD_SPEED_MULT,
	0
};

static const int SPEEDS_ERRC_DVD[] = {
	2*DVD_SPEED_MULT,
	5*DVD_SPEED_MULT,
	8*DVD_SPEED_MULT,
	12*DVD_SPEED_MULT,
	0
};

static const int SPEEDS_JB_CD[] = {
	4*CD_SPEED_MULT,
	0
};

static const int SPEEDS_JB_DVD[] = {
	2*DVD_SPEED_MULT,
	0
};

scan_plugin*	plugin_create(drive_info* idev)
{
	return new scan_plextor(idev);
}

void plugin_destroy(scan_plugin* iplugin)
{
	if (iplugin != NULL) delete iplugin;
}

scan_plextor::scan_plextor(drive_info* idev)
    : scan_plugin()
{
	dev = idev;
	if (!dev->silent) printf("scan_plextor()\n");
	devlist = (drivedesc*) &drivelist;
	test=0;
}

scan_plextor::~scan_plextor()
{
	if (!dev->silent) printf("~scan_plextor()\n");
}


int  scan_plextor::probe_drive() {
	if (isPlextor(dev))
		plextor_px755_do_auth(dev);
	if (dev->media.type & DISC_CD) {
		if (cmd_cd_errc_init()) return DEV_FAIL;
		if (cmd_scan_end()) return DEV_FAIL;
	} else if (dev->media.type & DISC_DVD) {
		if (cmd_dvd_errc_init()) return DEV_FAIL;
		if (cmd_scan_end()) return DEV_FAIL;
	} else {
		return DEV_FAIL;
	}
	return DEV_PROBED;
}

int  scan_plextor::errc_data()
{
	if (dev->media.type & ~DISC_DVDRAM) return 0;

	if (dev->media.type & DISC_CD) {
		return (ERRC_DATA_BLER|ERRC_DATA_E11|ERRC_DATA_E21|ERRC_DATA_E31|ERRC_DATA_E12|ERRC_DATA_E22|ERRC_DATA_E32|ERRC_DATA_UNCR);
	} else if (dev->media.type & DISC_DVD) {
		return (ERRC_DATA_PIE|ERRC_DATA_PIF|ERRC_DATA_POE|ERRC_DATA_POF|ERRC_DATA_UNCR);
	}
	return 0;
}

int  scan_plextor::check_test(unsigned int itest)
{
	switch (itest) {
		case CHK_ERRC:
		case CHK_JB:
			if (dev->media.type & ~DISC_DVDRAM)
				return 0;
			break;
		case CHK_FETE:
			if (dev->media.type & (DISC_CD | DISC_DVD) & ~DISC_CDROM & ~DISC_DVDROM)
				return 0;
			break;
		case CHK_TA:
			if (dev->media.type & (DISC_DVD & ~DISC_DVDROM))
			//if (dev->media.type & DISC_DVD)
				return 0;
			break;
		default:
			break;
	}
	return -1;
}

int* scan_plextor::get_test_speeds(unsigned int itest)
{
	switch (itest) {
		case CHK_ERRC:
			if (dev->media.type & DISC_CD)
				return (int*)SPEEDS_ERRC_CD;
			if (dev->media.type & DISC_DVD)
				return (int*)SPEEDS_ERRC_DVD;
			break;
		case CHK_JB:
			if (dev->media.type & DISC_CD)
				return (int*)SPEEDS_JB_CD;
			if (dev->media.type & DISC_DVD)
				return (int*)SPEEDS_JB_DVD;
			break;
		default:
			break;
	}
	return NULL;
}

int  scan_plextor::start_test(unsigned int itest, long ilba, int &speed)
{
	int r=-1;
	plextor_px755_do_auth(dev);
	switch (itest) {
		case CHK_ERRC_CD:
			lba=ilba;
			set_read_speed(speed);
			r = cmd_cd_errc_init();
			break;
		case CHK_JB_CD:
			lba=ilba;
			set_read_speed(speed);
			r = cmd_cd_jb_init();
			break;
		case CHK_ERRC_DVD:
			lba=ilba;
			set_read_speed(speed);
			r = cmd_dvd_errc_init();
			break;
		case CHK_JB_DVD:
			lba=ilba;
			set_read_speed(speed);
			r = cmd_dvd_jb_init();
			break;
#if 1
		case CHK_FETE:
#else
		case CHK_FETE_CD:
		case CHK_FETE_CDROM:
		case CHK_FETE_DVD:
		case CHK_FETE_DVDROM:
#endif
			lba=ilba;
			set_write_speed(speed);
			r = cmd_fete_init();
			break;
		case CHK_TA:
			if (dev->media.type & (DISC_DVD & ~DISC_DVDROM)) {
				r = 0;
				test = CHK_TA_DVD;
				// TA test should run at minimum available speed (2X for DVD-, 2.4X for DVD+)
				// setting 2X for DVD+ will be 2.4
				int taspd=2;
				set_read_speed(taspd);
				return 0;
			}
			break;
		default:
			return -1;
	}
	if (!r) {
		test = itest;
	} else {
		test = 0;
	}
	return r;
}

int  scan_plextor::scan_block(void *data, long *ilba)
{
	int r=-1;
	switch (test) {
		case CHK_ERRC_CD:
			r = cmd_cd_errc_block((cd_errc*)data);
			if(ilba) *ilba = lba;
			return r;
		case CHK_JB_CD:
			r = cmd_cd_jb_block((cdvd_jb*)data);
			if(ilba) *ilba = lba;
			return r;
		case CHK_ERRC_DVD:
			r = cmd_dvd_errc_block((dvd_errc*)data);
			if(ilba) *ilba = lba;
			return r;
		case CHK_JB_DVD:
			r = cmd_dvd_jb_block((cdvd_jb*)data);
			if(ilba) *ilba = lba;
			return r;
#if 1
		case CHK_FETE:
#else
		case CHK_FETE_CD:
		case CHK_FETE_CDROM:
		case CHK_FETE_DVD:
		case CHK_FETE_DVDROM:
#endif
			r = cmd_fete_block((cdvd_ft*)data);
			if(ilba) *ilba = lba;
			return r;
		case CHK_TA_DVD:
			cmd_dvd_ta_block((cdvd_ta*)data);
			return r;
		default:
			return -1;
	}
}

int  scan_plextor::end_test()
{
	switch (test) {
		case CHK_ERRC_CD:
		case CHK_ERRC_DVD:
		case CHK_JB_CD:
		case CHK_JB_DVD:
			cmd_scan_end();
			break;
#if 1
		case CHK_FETE:
#else
		case CHK_FETE_CD:
		case CHK_FETE_CDROM:
		case CHK_FETE_DVD:
		case CHK_FETE_DVDROM:
#endif
			cmd_fete_end();
			break;
		case CHK_TA:
		case CHK_TA_CD:
		case CHK_TA_CDROM:
		case CHK_TA_DVD:
		case CHK_TA_DVDROM:
			break;
		default:
			break;
	}
	test=0;
	return 0;
}

/*
__attribute__((constructor)) void init() {
    printf("init()\n");
}

__attribute__((destructor)) void exit() {
    printf("exit()\n");
}
*/
