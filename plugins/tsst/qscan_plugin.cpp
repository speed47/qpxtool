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
	return new scan_tsst(idev);
}

void plugin_destroy(scan_plugin* iplugin){
	if (iplugin != NULL) delete iplugin;
}

scan_tsst::scan_tsst(drive_info* idev)
    : scan_plugin()
{
	dev = idev;
	if (!dev->silent) printf("scan_tsst()\n");
	devlist = (drivedesc*) &drivelist; 
	test=0;
}

scan_tsst::~scan_tsst() {
	if (!dev->silent) printf("~scan_tsst()\n");
}

int scan_tsst::probe_drive() {
	cd_errc tmp_errc;
	if (strncmp(dev->ven,"TSSTcorp", 8)) return DEV_FAIL;
	if (dev->media.type & DISC_CD) {
		if (cmd_cd_errc_init()) return DEV_FAIL;
		if (cmd_cd_errc_block(&tmp_errc)) return DEV_FAIL;
		if (cmd_cd_errc_end()) return DEV_FAIL;
	} else if (dev->media.type & DISC_DVD) {
		if (cmd_dvd_errc_init()) return DEV_FAIL;
		if (cmd_dvd_errc_end()) return DEV_FAIL;
	} else {
		return DEV_FAIL;
	}
	printf("TSST probe OK\n");
	return DEV_PROBED;
}

int  scan_tsst::errc_data()
{
	if (dev->media.type & DISC_CD) {
		return (ERRC_DATA_BLER|ERRC_DATA_E32|ERRC_DATA_UNCR);
	} else if (dev->media.type & DISC_DVD) {
		return (ERRC_DATA_PIE|ERRC_DATA_PIF|ERRC_DATA_UNCR);
	}
	return 0;
}

int  scan_tsst::check_test(unsigned int itest)
{
	switch (itest) {
		case CHK_ERRC:
			if (dev->media.type & ~DISC_DVDRAM)
				return 0;
			break;
		default:
			break;
	}
	return -1;
}

int  scan_tsst::start_test(unsigned int itest, long ilba, int &speed){
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
		default:
			return -1;
	}
	if (!r) {
		test = itest;
	} else {
		end_test();
	}
	return r;
}

int  scan_tsst::scan_block(void *data, long *ilba) {
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
		default:
			return -1;
	}
}

int  scan_tsst::end_test() {
	switch (test) {
		case CHK_ERRC_CD:
			cmd_cd_errc_end();
			break;
		case CHK_ERRC_DVD:
			cmd_dvd_errc_end();
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

