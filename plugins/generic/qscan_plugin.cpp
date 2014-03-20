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
	return new scan_generic(idev);
}

void plugin_destroy(scan_plugin* iplugin){
	if (iplugin != NULL) delete iplugin;
}

scan_generic::scan_generic(drive_info* idev)
    : scan_plugin()
{
	dev = idev;
	if (!dev->silent) printf("scan_generic()\n");
	devlist = (drivedesc*) &drivelist; 
	test=0;
}

scan_generic::~scan_generic() {
	if (!dev->silent) printf("~scan_generic()\n");
}

int  scan_generic::probe_drive() {
	if (dev->capabilities & CAP_C2)
		return DEV_PROBED;
	else
		return DEV_FAIL;
}

int  scan_generic::errc_data() {
	if (dev->media.type & DISC_CD) {
		return (ERRC_DATA_E22|ERRC_DATA_UNCR);
	}
	return 0;
}

int  scan_generic::check_test(unsigned int itest) {
	switch (itest) {
		case CHK_ERRC:
			if (dev->media.type & DISC_CD)
				return 0;
			break;
		default:
			break;
	}
	return -1;
}

int  scan_generic::start_test(unsigned int itest, long ilba, int &speed) {
	int r=-1;
	switch (itest) {
		case CHK_ERRC_CD:
			lba=ilba;
			set_read_speed(speed);
			r = (dev->capabilities & CAP_C2) ? 0 : 1;
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

int  scan_generic::scan_block(void *data, long *ilba) {
	int r=-1;
	switch (test) {
		case CHK_ERRC_CD:
			r = cmd_cd_errc_block((cd_errc*)data);
			if(ilba) *ilba = lba;
			return r;
		default:
			return -1;
	}
}

int  scan_generic::end_test() {
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

