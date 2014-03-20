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

#include <qscan_plugin.h>

#include <qpx_transport.h>
//#include <qpx_mmc.h>
#include <common_functions.h>

//#define _PIO_ERRC_DEBUG

int scan_pioneer::cmd_cd_errc_read(unsigned char sects)
{
/*
CDB:
00000000  3B 02 E1 00 00 00 00 00 20 00                     ;.á..... .      

Data Sent:
00000000  FF 01 00 00  00 60 00 00  00 00 4B 00  00 00 4B 00   ÿ...............
00000010  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00   ................
*/

//	lba += 0x006000;

	int i;
	for (i=0; i<32; i++) dev->rd_buf[i]=0;

	dev->rd_buf[0] = 0xFF;
	dev->rd_buf[1] = 0x01;

	dev->rd_buf[4] = ((lba+0x006000) >> 16) & 0xFF;
	dev->rd_buf[5] = ((lba+0x006000) >> 8) & 0xFF;
	dev->rd_buf[6] = (lba) & 0xFF;
//	dev->rd_buf[6] = 0x01;

	dev->rd_buf[8] = (sects >> 16) & 0xFF;
	dev->rd_buf[9] = (sects >> 8) & 0xFF;
	dev->rd_buf[10] = sects & 0xFF;
	dev->rd_buf[12] = (sects >> 16) & 0xFF;
	dev->rd_buf[13] = (sects >> 8) & 0xFF;
	dev->rd_buf[14] = sects & 0xFF;
/*
	printf("Pioneer scan Cx CMD data:");
	for (i=0; i<16; i++) {
	    if (!(i%0x20)) printf("\n");
	    printf(" %02X",dev->rd_buf[i] & 0xFF);
	}
	printf("\n");
*/
	dev->cmd[0] = 0x3B;
	dev->cmd[1] = 0x02;
	dev->cmd[2] = 0xE1;
	dev->cmd[8] = 0x20;
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,32))) {
		if(!dev->silent) sperror ("pioneer_send_scan_cx",dev->err); return dev->err;
	}
	return 0;
}


int scan_pioneer::cmd_dvd_errc_read(unsigned char nECC)
{
/*
CDB:
00000000  3B 02 E1 00 00 00 00 00 20 00                     ;.á..... .      

Data Sent:
00000000  FF 01 00 00 03 00 01 00 00 00 80 00 00 00 08 00   ÿ...............
00000010  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   ................
*/
	int i;
	int sects = nECC*16;
//	lba += 0x030000;

	for (i=0; i<32; i++) dev->rd_buf[i]=0;
	dev->rd_buf[0] = 0xFF;
	dev->rd_buf[1] = 0x01;
	dev->rd_buf[4] = ((lba >> 16) + 3) & 0xFF;
	dev->rd_buf[5] = (lba >> 8) & 0xFF;
	dev->rd_buf[6] = (lba) & 0xFF;
//	dev->rd_buf[6] = 0x01;

//	dev->rd_buf[9] = 0x01;
//	dev->rd_buf[9] =  (nECC >> 4 ) & 0xFF;
//	dev->rd_buf[10] = (nECC << 4 ) & 0xFF;
//	dev->rd_buf[14] = nECC;
//	dev->rd_buf[14] = 0x08;


//	dev->rd_buf[9]  = (sects >> 8) & 0xFF;
	dev->rd_buf[10] = (sects) & 0xFF;
//	dev->rd_buf[13] = (sects >> 12) & 0xFF;
//	dev->rd_buf[14] = nECC;
	dev->rd_buf[14] = 1;
/*
	printf("Pioneer scan Pi CMD data:");
	for (i=0; i<16; i++) {
	    if (!(i%0x20)) printf("\n");
	    printf(" %02X",dev->rd_buf[i] & 0xFF);
	}
	printf("\n");
*/
	dev->cmd[0] = 0x3B;
	dev->cmd[1] = 0x02;
	dev->cmd[2] = 0xE1;
	dev->cmd[8] = 0x20;
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,32))) {
		if(!dev->silent) sperror ("pioneer_send_scan_pi",dev->err); return dev->err;
	}
	return 0;
}


int scan_pioneer::cmd_cd_errc_getdata(cd_errc *data)
{
	int i;
	for (i=0; i<32; i++) dev->rd_buf[i]=0;
	dev->cmd[0] = 0x3C;
	dev->cmd[1] = 0x02;
	dev->cmd[2] = 0xE1;
	dev->cmd[8] = 0x20;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,32))) {
		sperror ("pioneer_read_error_info",dev->err); return dev->err;
	}
#ifdef _PIO_ERRC_DEBUG
	printf("Read error info RSP data:");
	for (i=0; i<32; i++) {
	    if (!(i%0x20)) printf("\n");
	    printf(" %02X",dev->rd_buf[i] & 0xFF);
	}
	printf("\n");
#endif
	if ((ntoh16(dev->rd_buf+13) > 300) || (ntoh16(dev->rd_buf+5) > 300)) {
		data->bler = 0;
		data->e22  = 0;
	} else {
		//data->bler = ntoh16(dev->rd_buf+13) - ntoh16(dev->rd_buf+5);
		data->bler = ntoh16(dev->rd_buf+13);
		data->e22  = ntoh16(dev->rd_buf+5);
	}
	data->e11 = 0;
	data->e21 = 0;
	data->e31 = 0;
	data->e12 = 0;
	data->e32 = 0;
	data->uncr = 0;

	return 0;
}

int scan_pioneer::cmd_dvd_errc_getdata(dvd_errc *data)
{
	int i;
	for (i=0; i<32; i++) dev->rd_buf[i]=0;
	dev->cmd[0] = 0x3C;
	dev->cmd[1] = 0x02;
	dev->cmd[2] = 0xE1;
	dev->cmd[8] = 0x20;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,32))) {
		sperror ("pioneer_read_error_info",dev->err); return dev->err;
	}
#ifdef _PIO_ERRC_DEBUG
	printf("Read error info RSP data:");
	for (i=0; i<32; i++) {
	    if (!(i%0x20)) printf("\n");
	    printf(" %02X",dev->rd_buf[i] & 0xFF);
	}
	printf("\n");
#endif

	if (dev->dev_ID < PIO_DVR_111) {
		data->pie = max( 0, (ntoh16(dev->rd_buf+13) - ntoh16(dev->rd_buf+5)) / 10);
		data->pif = ntoh16(dev->rd_buf+13) / 200;
	} else {
		data->pie = max( 0, (ntoh16(dev->rd_buf+13) - ntoh16(dev->rd_buf+5)));
		data->pif = ntoh16(dev->rd_buf+13) / 20;
	}
//	data->pi8=data->pie;
	data->poe=0;
//	data->po8=0;
	data->pof=0;
	return 0;
}

int scan_pioneer::cmd_cd_errc_block(cd_errc *data)
{
	int r;
	unsigned char interval = 75;
	r = cmd_cd_errc_read(interval);
	if (!r) r = cmd_cd_errc_getdata(data);
	lba += interval;
	return r;
}

int scan_pioneer::cmd_dvd_errc_block(dvd_errc *data)
{
	int r;
	const char interval = 1;
	r = cmd_dvd_errc_read(interval);
	if (!r) r = cmd_dvd_errc_getdata(data);
//	*pie = max( 0, (ntoh16(dev->rd_buf+13) - 1.5 * ntoh16(dev->rd_buf+5)) / 40);
	lba+= 16*interval;
	return r;
}

int scan_pioneer::cmd_cd_errc_init()
{
	int r;
	cd_errc data;
//	spinup(dev, 2);
	seek(dev, 0);
	r = cmd_cd_errc_read(75);
	if (!r)  r = cmd_cd_errc_getdata(&data);
	return r;
}


int scan_pioneer::cmd_dvd_errc_init()
{
	int r;
	dvd_errc data;
//	spinup(dev, 2);
	seek(dev, 0);
	r = cmd_dvd_errc_read(1);
	if (!r) r = cmd_dvd_errc_getdata(&data);
	return r;
}

/*
int scan_pioneer dummy(dev_info* dev)
{
	dev->err = 0;
	return 0;
}
*/

