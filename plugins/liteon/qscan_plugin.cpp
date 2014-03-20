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

#include <stdio.h>
#include <qscan_plugin.h>

scan_plugin*	plugin_create(drive_info* idev){
	return new scan_liteon(idev);
}

void plugin_destroy(scan_plugin* iplugin){
	if (iplugin != NULL) delete iplugin;
}

scan_liteon::scan_liteon(drive_info* idev)
	: scan_plugin()
{
	dev = idev;
	if (!dev->silent) printf("scan_liteon()\n");
	devlist = (drivedesc*) &drivelist; 
	blklist = (drivedesc*) &blacklist; 
	test=0;
}

scan_liteon::~scan_liteon() {
	if (!dev->silent) printf("~scan_liteon()\n");
}

int  scan_liteon::probe_drive() {
#ifndef PLUGINS_LITEON_NOPROBE
	if (dev->media.type & DISC_CD) {
		if (cmd_cd_errc_init()) return DEV_FAIL;
		if (cmd_cd_errc_end()) return DEV_FAIL;
	} else if (dev->media.type & DISC_DVD) {
		if (cmd_dvd_errc_init()) return DEV_FAIL;
		if (cmd_dvd_errc_end()) return DEV_FAIL;
	} else if (dev->media.type & DISC_BD) {
		if (cmd_bd_errc_init()) return DEV_FAIL;
		if (cmd_bd_errc_end()) return DEV_FAIL;
	} else {
		return DEV_FAIL;
	}
	return DEV_PROBED;
#else
	return DEV_FAIL;
#endif
}

int  scan_liteon::errc_data()
{
	if (dev->media.type & DISC_CD) {
		return (ERRC_DATA_BLER|ERRC_DATA_E22|ERRC_DATA_E32|ERRC_DATA_UNCR);
	} else if (dev->media.type & DISC_DVD) {
		return (ERRC_DATA_PIE|ERRC_DATA_PIF|ERRC_DATA_UNCR);
	} else if (dev->media.type & DISC_BD) {
		return (ERRC_DATA_LDC|ERRC_DATA_BIS|ERRC_DATA_UNCR);
	}
	return 0;
}

int  scan_liteon::check_test(unsigned int itest)
{
	switch (itest) {
		case CHK_ERRC:
			if (dev->media.type & ~DISC_DVDRAM)
				return 0;
			break;
		case CHK_FETE:
			if (dev->media.type & ~DISC_DVDROM)
				return 0;
			break;
		default:
			return -1;
	}
	return -1;
}

int* scan_liteon::get_test_speeds(unsigned int itest) { return NULL; }

int  scan_liteon::start_test(unsigned int itest, long ilba, int &speed)
{
	int r=-1;
	switch (itest) {
		case CHK_ERRC_CD:
			lba=ilba;
			set_read_speed(speed);
			r = cmd_cd_errc_init();
			break;
		case CHK_ERRC_DVD:
			lba=ilba;
			set_read_speed(speed);
			r = cmd_dvd_errc_init();
			break;
		case CHK_ERRC_BD:
			lba=ilba;
			set_read_speed(speed);
			r = cmd_bd_errc_init();
			break;
		case CHK_FETE:
			start_stop(dev,1);
			seek(dev,0);
			if (r = cmd_fete_init(speed))
				return -1;
			r = cmd_fete_init(speed);
			break;
		default:
			return -1;
	}
	if (!r) {
		test = itest;
		return r;
	} else {
		end_test();
		return r;
	}
}

int  scan_liteon::scan_block(void *data, long *ilba)
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
		case CHK_ERRC_BD:
			r = cmd_bd_errc_block((bd_errc*)data);
			if(ilba) *ilba = lba;
			return r;
		case CHK_FETE:
			if (dev->media.type & DISC_CD) {
				r = cmd_cd_fete_block((cdvd_ft*)data);
			} else if (dev->media.type & DISC_DVD) {
				r = cmd_dvd_fete_block((cdvd_ft*)data);
			} else if (dev->media.type & DISC_BD) {
				r = cmd_bd_fete_block((cdvd_ft*)data);
			}
			if(ilba) *ilba = lba;
			return r;	
		default:
			return -1;
	}
}

int  scan_liteon::end_test()
{
	switch (test) {
		case CHK_ERRC_CD:
			cmd_cd_errc_end();
			break;
		case CHK_ERRC_DVD:
			cmd_dvd_errc_end();
			break;
		case CHK_ERRC_BD:
			cmd_bd_errc_end();
			break;
		case CHK_FETE:
			return 0;
			break;
		default:
			break;
	}
	test=0;
	return 0;
}
