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

#ifndef __QSCAN_GENERIC_H
#define __QSCAN_GENERIC_H

#include "qpx_scan_plugin_api.h"

static const drivedesclist drivelist =
//static drivedesclist drivelist =
{
	{ "", 0, "", 0}
};

static const char plugin_name[]="C2P";
static const char plugin_desc[]="Generic scan plugin for devices supported C2 pointers";

class drive_info;

class scan_generic : public scan_plugin {
public:
//    scan_generic(drive_info* idev=NULL);
    scan_generic(drive_info* idev);
    virtual ~scan_generic();
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

	int cmd_cd_errc_block(cd_errc *data);
//	int c2calc(cd_errc *data);
	int c2calc(unsigned char* buf, unsigned int lba, unsigned char sects);
};

#endif

