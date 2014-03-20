/*
 * qscan plugin for ASUS drives:
 *	1612
 *	1814
 *	2014S1
 *	2014L1
 *
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include <stdio.h>
#include <stdlib.h>

#include <qpx_transport.h>
#include <qpx_mmc.h>
#include <common_functions.h>

#include <qscan_plugin.h>

int scan_asus::probe_drive()
{
	dev->cmd[0] = 0x5A;
	dev->cmd[2] = 0x38;
	dev->cmd[3] = 0x41;
	dev->cmd[4] = 0x53;
	dev->cmd[5] = 0x10;
	dev->cmd[9] = 0x04;
	if ((dev->err=dev->cmd.transport(READ, dev->rd_buf, 20))){
		sperror ("asus_probe",dev->err); return DEV_FAIL;
	}
	if (strncmp((char*)dev->rd_buf,"ASUS",4)) return DEV_FAIL;
	return DEV_PROBED;
}

// ************* Scan init commands *********
int scan_asus::cmd_errc_init()
{
	/* initialize scan mode */
	dev->cmd[0] = 0x5A;
	dev->cmd[2] = 0x38;
	dev->cmd[3] = 0x41;
	dev->cmd[4] = 0x53;
	dev->cmd[5] = 0x10;
	dev->cmd[9] = 0x01;
	if ((dev->err=dev->cmd.transport(NONE, NULL, 0))){
		sperror ("asus_errc_init",dev->err); return 1;
	}
	seek(dev,0);
	return 0;
}

int scan_asus::cmd_errc_getdata()
{
	/* initialize scan mode */
	dev->cmd[0] = 0x5A;
	dev->cmd[2] = 0x38;
	dev->cmd[3] = 0x41;
	dev->cmd[4] = 0x53;
	dev->cmd[5] = 0x10;
	dev->cmd[9] = 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,8))){
		sperror ("asus_errc_getdata",dev->err); return 1;
	}
	return 0;
}



// **********************
int scan_asus::cmd_cd_errc_block(cd_errc *data)
{
//	seek(dev,lba);
	data->e11 = 0;
	data->e21 = 0;
	data->e31 = 0;
	data->e12 = 0;
	data->e32 = 0;
	data->uncr = 0;
	
	if (!cmd_errc_getdata()) {
		data->bler = (dev->rd_buf[1] << 8) | dev->rd_buf[0];
		data->e22 = (dev->rd_buf[3] << 8) | dev->rd_buf[2];
	} else {
		data->bler = 0;
		data->e22 = 0;
	}
	lba=((int)dev->rd_buf[5] * 4500 + (int)dev->rd_buf[6] * 75 + (int)dev->rd_buf[7]);
//	lba+=75;
	return 0;
}

int scan_asus::cmd_dvd_errc_block(dvd_errc *data)
{
	seek(dev,lba);
	data->poe = 0;
	data->pof = 0;
	if (!cmd_errc_getdata()) {
		data->pie = (dev->rd_buf[1] << 8) | dev->rd_buf[0];
		data->pif = (dev->rd_buf[3] << 8) | dev->rd_buf[2];
	} else {
		data->pie = 0;
		data->pif = 0;
	}
//	lba = swap4(dev->rd_buf+4);
	lba+=16;
	return 0;
}

// ************* END SCAN COMMAND *********
int scan_asus::cmd_errc_end()
{
	dev->cmd[0] = 0x5A;
	dev->cmd[2] = 0x38;
	dev->cmd[3] = 0x41;
	dev->cmd[4] = 0x53;
	dev->cmd[5] = 0x10;
	dev->cmd[9] = 0x02;
	if ((dev->err=dev->cmd.transport(NONE, NULL, 0))){
		sperror ("asus_errc_end",dev->err); return 1;
	}
	return 0;
}

