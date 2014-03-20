/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2006 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
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

// ************* Scan init commands *********
int scan_tsst::cmd_cd_errc_init() {
	return seek(dev,0);
}

// DVD part is same as Lite-On

int scan_tsst::cmd_dvd_errc_init() {
// 2B 00 00 00 00 00 00 00 00 00
	dev->cmd[0] = 0x2B;
	if ((dev->err=dev->cmd.transport(NONE,NULL,0))){
		sperror ("TSST INIT DVD ERRC",dev->err); return 1;
	}
	printf("TSST INIT DVD ERRC: OK\n");
	return 0;
}

// **********************
int scan_tsst::cmd_cd_errc_block(cd_errc *data)
{
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x0E;
	dev->cmd[8] = 0x4B;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,8))){
		sperror ("TSST Cx SCAN INTERVAL",dev->err); return 1;
	}
#if 0
	for (int i=0; i<8; i++) {
	    printf(" %02X",dev->rd_buf[i]);
	}
	printf("\n");
#endif
	lba = ntoh32(dev->rd_buf);

	data->bler = ntoh16(dev->rd_buf+4);
	data->e11 = 0;
	data->e21 = 0;
	data->e31 = 0;
	data->e12 = 0;
	data->e22 = ntoh16(dev->rd_buf+6);
	data->e32 = 0;
	data->uncr = 0;
	return 0;
}


int scan_tsst::cmd_dvd_errc_block(dvd_errc *data)
{
//	int i;
//	*pie = 0;

	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x0E;
	dev->cmd[8] = 0x10;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,10))){
		sperror ("TSST PI SCAN INTERVAL",dev->err); return 1;
	}
#if 0
	for (int i=0; i<8; i++) {
	    printf(" %02X",dev->rd_buf[i]);
	}
	printf("\n");
#endif

// Data Received:
// 00000000  00 00 00 8E 00 00 00 00                           ...Ž....        
//	lba+=16;

//	lba = ((dev->rd_buf[1] << 16 )& 0xFF0000) + ((dev->rd_buf[2] << 8)&0xFF00 ) + (dev->rd_buf[3] & 0xFF);
	lba = ntoh32(dev->rd_buf);

	data->pie = ntoh16(dev->rd_buf+4);
//	data->pi8 = 0;
	data->pif = ntoh16(dev->rd_buf+6);
	data->poe = 0;
//	data->po8 = 0;
	data->pof = 0;
	return 0;
}

// ************* END SCAN COMMANDS *********
int scan_tsst::cmd_cd_errc_end() {
	dev->err = 0;
	return 0;
}

int scan_tsst::cmd_dvd_errc_end() {
	dev->err = 0;
	return 0;
}
