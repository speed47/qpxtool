/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2006 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 *
 * Thanks to Artur Kalimullin (Kaliy) <kalimullin@gmail.com> for assistance in LiteOn Cx scan implementation
 *
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
#include <colors.h>

#include <qscan_plugin.h>

// ************* Scan init commands *********
int scan_liteon::cmd_cd_errc_init_old() {
	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0xA3;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,256))){
		sperror ("LiteOn_init_A_cx_scan",dev->err); return 1;
	}
//	printf("LiteOn_init_A_cx_scan: OK\n");

	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0xA0;
	dev->cmd[4] = 0x02;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,256))){
		sperror ("LiteOn_init_B_cx_scan",dev->err); return 1;
	}
//	printf("LiteOn_init_B_cx_scan: OK\n");

	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0xA0;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,256))){
		sperror ("LiteOn_init_C_cx_scan",dev->err); return 1;
	}
//	printf("LiteOn_init_C_cx_scan: OK\n");

	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0xA0;
	dev->cmd[4] = 0x04;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,256))){
		sperror ("LiteOn_init_D_cx_scan",dev->err); return 1;
	}
//	printf("LiteOn_init_D_scan: OK\n");

	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0xA0;
	dev->cmd[4] = 0x02;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,256))){
		sperror ("LiteOn_init_E_cx_scan",dev->err); return 1;
	}
	printf(COL_YEL "LiteOn: using OLD CD ERRC commands" COL_NORM "\n");
	return 0;
}

int scan_liteon::cmd_cd_errc_init_new() {
// seek to first sector
	if (seek(dev, 0))
		return 1;
// check if ERRC command works
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x0E;
	dev->cmd[11] = 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,0x10))) {
		sperror ("LiteOn_errc_cd_probe NEW",dev->err);
		cd_errc_new = false;
		return 1;
	}
	return 0;
}

int scan_liteon::cmd_cd_errc_init() {
	cd_errc_new = true;
	if (cmd_cd_errc_init_new())
		return cmd_cd_errc_init_old();
	printf(COL_GRN "LiteOn: using new CD ERRC commands" COL_NORM "\n");
	return 0;
}

int scan_liteon::cmd_dvd_errc_init() {
// seek to first sector
	if (seek(dev, 0))
		return 1;
// check if ERRC command works
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x0E;
	dev->cmd[8] = 0x10;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,0x10))){
		sperror ("LiteOn_errc_dvd_probe",dev->err); return 1;
	}
	return 0;
}

int scan_liteon::cmd_bd_errc_init() {
// seek to first sector
	if (seek(dev, 0))
		return 1;
// check if ERRC command works
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x0E;
	dev->cmd[11] = 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,0x10))){
		sperror ("LiteOn_errc_bd_probe",dev->err); return 1;
	}
	return 0;
}

// fe/te data redout
int scan_liteon::cmd_fete_get_data(cdvd_ft *data) {
	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0x08;
	dev->cmd[2] = 0x02;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,65536))){
		sperror ("LiteOn_FETE get data",dev->err); return 1;
	}
	data->te=dev->rd_buf[0];
	data->fe=dev->rd_buf[1];
	return 0;
}

// current position readout for fe/te
int scan_liteon::cmd_fete_get_position() {
	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0x02;
	dev->cmd[2] = 0x09;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,65536))){
		sperror ("LiteOn_FETE get LBA",dev->err); return 1;
	}
	return 0;
}

int scan_liteon::cmd_cd_fete_init(int& speed) {
// start scan command
	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0x08;
	dev->cmd[2] = 0x01;
	dev->cmd[4] = 0x02; // CD

	dev->cmd[7] = speed & 0xFF;
	dev->cmd[11]= 0x00;

	memset(dev->rd_buf,0,16);
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,16))){
		sperror ("LiteOn_FETE_init scan",dev->err); return 1;
	}
	return 0;
}

int scan_liteon::cmd_dvd_fete_init(int& speed) {
// start scan command
	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0x08;
	dev->cmd[2] = 0x01;
	dev->cmd[3] = 0x03; // DVD

	dev->cmd[7] = speed & 0xFF;
	dev->cmd[11]= 0x00;

	memset(dev->rd_buf,0,16);

	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,16))){
		sperror ("LiteOn_FETE_init scan",dev->err); return 1;
	}
	return 0;
}

int scan_liteon::cmd_bd_fete_init(int& speed) {
// start scan command
	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0x08;
	dev->cmd[2] = 0x01;

	dev->cmd[7] = speed & 0xFF;
	dev->cmd[11]= 0x00;

	memset(dev->rd_buf,0,16);

	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,16))){
		sperror ("LiteOn_FETE_init scan",dev->err); return 1;
	}
	return 0;
}

int scan_liteon::cmd_fete_init(int& speed) {
	if (dev->media.type & (DISC_CD & ~DISC_CDROM)) {
		return cmd_cd_fete_init(speed);
	} else if (dev->media.type & (DISC_DVD & ~DISC_DVDROM)) {
		return cmd_dvd_fete_init(speed);
	} else if (dev->media.type & (DISC_BD & ~DISC_BD_ROM)) {
		return cmd_bd_fete_init(speed);
	}
	return -1;
}

// ********************** CD ERRC commands


int scan_liteon::cmd_cd_errc_read() {
	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0x82;
	dev->cmd[2] = 0x09;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,256))){
		sperror ("LiteOn_cx_do_one_interval_scan",dev->err); return 1;
	}
//	printf("LiteOn_cx_do_one_interval_scan: OK");
	return 0;
}

int scan_liteon::cmd_cd_errc_getdata(cd_errc* data) {
	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0x82;
	dev->cmd[2] = 0x05;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,256))){
		sperror ("LiteOn_cx_do_one_interval_readout",dev->err); return 1;
	}
//	printf("LiteOn_cx_do_one_interval_readout: OK");
#if 0
	for (int i=0; i<32; i++) {
	    printf(" %02X",dev->rd_buf[i]);
	}
	printf("\n");
#endif
	data->bler = ntoh16(dev->rd_buf);
	data->e11 = 0;
	data->e21 = 0;
	data->e31 = 0;
	data->e12 = 0;
	data->e22 = ntoh16(dev->rd_buf+2);
//	data->e32 = ntoh16(dev->rd_buf+4);
	data->e32 = dev->rd_buf[4];
	data->uncr = 0;
	return 0;
}

int scan_liteon::cmd_cd_errc_inteval_rst() {
	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0x97;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,256))){
		sperror ("LiteOn_cx_do_one_interval_rst",dev->err); return 1;
	}
	return 0;
}

int scan_liteon::cmd_cd_errc_block_old(cd_errc *data)
{
	if (cmd_cd_errc_read()) return dev->err;
	if (cmd_cd_errc_getdata(data)) return dev->err;
	lba+=75;
	if (cmd_cd_errc_inteval_rst()) return dev->err;
//	printf("LiteOn_cx_do_one_interval_rst: OK");
	return 0;
}

int scan_liteon::cmd_cd_errc_block_new(cd_errc *data)
{
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x0E;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,10))){
		sperror ("LiteOn_errc_cd_read_block",dev->err); return 1;
	}
	lba = dev->rd_buf[1] * 60 * 75 +
		  dev->rd_buf[2] * 75 +
		  dev->rd_buf[3];

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

int scan_liteon::cmd_cd_errc_block(cd_errc *data)
{
	return cd_errc_new ? cmd_cd_errc_block_new(data) : cmd_cd_errc_block_old(data);
}

// ********************** DVD ERRC commands
int scan_liteon::cmd_dvd_errc_block(dvd_errc *data)
{
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x0E;
	dev->cmd[8] = 0x10;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,10))){
		sperror ("LiteOn_errc_dvd_read_block",dev->err); return 1;
	}
#if 0
	for (int i=0; i<10; i++) {
	    printf(" %02X",dev->rd_buf[i]);
	}
	printf("\n");
#endif

// Data Received:
// 00000000  00 00 00 8E 00 00 00 00                           ...Ž....        

//	lba = ((dev->rd_buf[1] << 16 )& 0xFF0000) + ((dev->rd_buf[2] << 8)&0xFF00 ) + (dev->rd_buf[3] & 0xFF);
	lba = ntoh32(dev->rd_buf);

	data->pie = ntoh16(dev->rd_buf+4);
	data->pif = ntoh16(dev->rd_buf+6);
	data->poe = 0;
	data->pof = 0;
	return 0;
}

// ********************** BD ERRC commands
int scan_liteon::cmd_bd_errc_block(bd_errc *data)
{
	bool retry = false;
	if (!lba) {
		retry = true;
		// if first sector scan requested
		// we have to seek to first sector
		dev->cmd[0] = MMC_SEEK;
		if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,2048))){
			sperror ("READ",dev->err); return 1;
		}
	}

bd_errc_repeat:
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x0E;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,0x10))) {
		sperror ("LiteOn_errc_bd_read_block",dev->err); return 1;
	}
	lba = ntoh32(dev->rd_buf);

	if (!lba && retry) {
		retry = false;
		goto bd_errc_repeat;
	}

	data->ldc = ntoh16(dev->rd_buf+4);
	data->bis = ntoh16(dev->rd_buf+6);
	data->uncr = 0;

// workaround to skip strange LDC/BIS data for last block
	if (data->ldc > 9700 || data->bis >= 768) {
		data->ldc = 0;
		data->bis = 0;
	}
	return 0;
}

// *************** FE/TE    scan block

int scan_liteon::cmd_cd_fete_block(cdvd_ft *data) {
// fe/te data redout
	if (cmd_fete_get_data(data)) return 1;

// current lba redout
	if (cmd_fete_get_position()) return 1;
	int tlba = (((dev->rd_buf[0] >> 4) & 0x0F) * 10 + (dev->rd_buf[0] & 0x0F)) * 60 * 75 +
		  (((dev->rd_buf[1] >> 4) & 0x0F) * 10 + (dev->rd_buf[1] & 0x0F)) * 75 +
		  (((dev->rd_buf[2] >> 4) & 0x0F) * 10 + (dev->rd_buf[2] & 0x0F));

	if (tlba < lba)
		return -1;

	lba = tlba;
	return 0;
}

int scan_liteon::cmd_dvd_fete_block(cdvd_ft *data) {
// fe/te data redout
	if (cmd_fete_get_data(data)) return 1;

// current lba redout
	if (cmd_fete_get_position()) return 1;
	lba = ntoh32(dev->rd_buf) >> 8;
	return 0;
}

int scan_liteon::cmd_bd_fete_block(cdvd_ft *data) {
// fe/te data redout
	if (cmd_fete_get_data(data)) return 1;

// current lba redout
	if (cmd_fete_get_position()) return 1;
	lba = ntoh32(dev->rd_buf);
	return 0;
}

// ************* END SCAN COMMANDS *********
int scan_liteon::cmd_cd_errc_end() {
	if (cd_errc_new)
		return 0;

	dev->cmd[0] = 0xDF;
	dev->cmd[1] = 0xA3;
	dev->cmd[2] = 0x01;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,256))){
		sperror ("LiteOn_end_scan",dev->err); return 1;
	}
	return 0;
}

int scan_liteon::cmd_dvd_errc_end() {
	dev->err = 0;
	return 0;
}

int scan_liteon::cmd_bd_errc_end() {
	dev->err = 0;
	return 0;
}

