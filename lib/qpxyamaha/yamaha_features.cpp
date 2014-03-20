/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2007 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/time.h>
#include <unistd.h>

#include <qpx_transport.h>
#include <qpx_mmc.h>
#include "yamaha_features.h"

int yamaha_check_amqr(drive_info* drive) {
//	unsigned char len=0;
	unsigned int  ilen=0;
	int	offs=0;
	int	r;
//	int	i;

	mode_sense(drive, MODE_PAGE_WRITE_PARAMETERS, 00, 256);
	ilen = ntoh16u (drive->rd_buf);
/*
	printf("data len: %4X (%4d), Header:\n", ilen, ilen);
	for (i=0; i<8; i++) printf(" %02X",drive->rd_buf[i] & 0xFF); printf("\n");
*/

	offs=0; while (((drive->rd_buf[offs]) & 0x3F) != MODE_PAGE_WRITE_PARAMETERS) offs++;
/*	len=drive->rd_buf[offs+1];

	printf("Page:\n");
	for (i=0; i<(len+2); i++) { if (!(i%8)) printf("\n"); printf(" %02X",drive->rd_buf[i+8] & 0xFF); }
	printf("\n");
*/

	if (!ilen) return 1;
//	drive->silent++;

/*
	drive->rd_buf[offs+2] &= 0xB0;
	drive->rd_buf[offs+2] |= 0x08;
	drive->rd_buf[offs+3] &= 0xF0;
	drive->rd_buf[offs+4] &= 0xF0;
	drive->rd_buf[offs+8]  = 0x00;
*/

/*
	drive->rd_buf[offs+2] = 0x32;

	drive->rd_buf[offs+2] = 0x88;
	drive->rd_buf[offs+3] = 0x00;
	drive->rd_buf[offs+4] = 0x00;
	drive->rd_buf[offs+8] = 0x00;
*/
	r = mode_select(drive, ilen);
//	drive->silent--;
	return r;
}

int yamaha_set_amqr(drive_info* drive) {
	return 1;
}

int yamaha_check_forcespeed(drive_info* drive) {
	int	offs;
	int	speed;
	int	speed_wr;
	mode_sense(drive, MODE_PAGE_MM_CAP_STATUS, 00, 256);
	offs=0; while (((drive->rd_buf[offs]) & 0x3F) != MODE_PAGE_MM_CAP_STATUS) offs++;
	speed    = ntoh16(drive->rd_buf+offs+14);
	speed_wr = ntoh16(drive->rd_buf+offs+28);

	drive->cmd[0] = MMC_SET_SPEED;
	drive->cmd[1] = 0x01;
	drive->cmd[2] = (speed >> 8) & 0xFF;
	drive->cmd[3] = speed & 0xFF;
//	drive->cmd[4] = 0xFF;
//	drive->cmd[5] = 0xFF;
	drive->cmd[4] = (speed_wr >> 8) & 0xFF;
	drive->cmd[5] = speed_wr & 0xFF;
	drive->cmd[11] = 0x80;
	if ((drive->err=drive->cmd.transport(NONE,NULL,0) )) { return (drive->err); }
	return 0;
}

int yamaha_set_forcespeed(drive_info* drive) {
	int	speed = 0xFFFF;
	int	speed_wr = 0xFFFF;
	if (drive->parms.read_speed_kb) speed = drive->parms.read_speed_kb;
	if (drive->parms.write_speed_kb) speed_wr = drive->parms.write_speed_kb;

	drive->cmd[0] = MMC_SET_SPEED;
	drive->cmd[1] = 0x01;
	drive->cmd[2] = (speed >> 8) & 0xFF;
	drive->cmd[3] = speed & 0xFF;
//	drive->cmd[4] = 0xFF;
//	drive->cmd[5] = 0xFF;
	drive->cmd[4] = (speed_wr >> 8) & 0xFF;
	drive->cmd[5] = speed_wr & 0xFF;
	drive->cmd[11] = 0x80;
	if ((drive->err=drive->cmd.transport(NONE,NULL,0) )) { return (drive->err); }
	return 0;
}

int yamaha_f1_get_tattoo(drive_info* drive) {
	unsigned int	ilen=0;
	int		offs=0;
	drive->yamaha.tattoo_i=0;
	drive->yamaha.tattoo_o=0;
	drive->yamaha.tattoo_rows=0;

	drive->silent++;
	mode_sense(drive, MODE_PAGE_YAMAHA_TATTOO, 00, 256);
	drive->silent--;
	if (drive->err) {
		if (drive->err == 0x52400) return 1;
		return 0;
	}

	ilen = ntoh16u (drive->rd_buf);
	if (ilen < 22) return 1;
	offs=0; while (((drive->rd_buf[offs]) & 0x3F) != MODE_PAGE_YAMAHA_TATTOO) offs++;
/*
	unsigned char len=0;
	int	r;
	int	i;
	printf("data len: %4X (%4d), Header:\n", ilen, ilen);
	for (i=0; i<8; i++) printf(" %02X",drive->rd_buf[i] & 0xFF); printf("\n");

	len=drive->rd_buf[offs+1];	

	printf("Page:");
	for (i=0; i<(len+2); i++) { if (!(i%8)) printf("\n"); printf(" %02X",drive->rd_buf[i+8] & 0xFF); }
	printf("\n");
*/
	drive->yamaha.tattoo_i = (drive->rd_buf[offs+4] << 16) | (drive->rd_buf[offs+5] << 8) | drive->rd_buf[offs+6];
	drive->yamaha.tattoo_o = (drive->rd_buf[offs+7] << 16) | (drive->rd_buf[offs+8] << 8) | drive->rd_buf[offs+9];
	drive->yamaha.tattoo_rows = drive->yamaha.tattoo_o - drive->yamaha.tattoo_i;
	return 0;
}

int yamaha_write(drive_info* drive, char mode, char bufid, int offs, int plen) {
//	printf("Mode: %d, BufID: %d, offs: %d, plen: %d, blen: %d\n", mode, bufid, offs, plen, blen);
//	drive->cmd[0] = SPC_WRITE_BUFFER;
	drive->cmd[0] = 0x3B;
	drive->cmd[1] = mode & 7;
	drive->cmd[2] = bufid;
//	drive->cmd[1] = 0x01;
//	drive->cmd[2] = 0;

	drive->cmd[3] = (offs >> 16) & 0xFF;
	drive->cmd[4] = (offs >> 8) & 0xFF;
	drive->cmd[5] = offs & 0xFF;

	drive->cmd[6] = (plen >> 16) & 0xFF;
	drive->cmd[7] = (plen >> 8) & 0xFF;
	drive->cmd[8] = plen & 0xFF;
	if ((drive->err=drive->cmd.transport(WRITE,drive->rd_buf, plen << 11) ))
		{ if (!drive->silent) sperror ("YAMAHA_WRITE", drive->err); return (drive->err); }
	return 0;
}


int yamaha_set_tattoo_speed(drive_info* drive) {
	int speed    = 0x06E5;
	int speed_wr = 0x02C2;
	drive->cmd[0] = MMC_SET_SPEED;
	drive->cmd[1] = 0x00;
	drive->cmd[2] = (speed >> 8) & 0xFF;
	drive->cmd[3] = speed & 0xFF;
	drive->cmd[4] = (speed_wr >> 8) & 0xFF;
	drive->cmd[5] = speed_wr & 0xFF;
	if ((drive->err=drive->cmd.transport(NONE,NULL,0) ))
		{ if (!drive->silent) sperror ("YAMAHA_SET_SPEED",drive->err); return (drive->err); }
	return 0;
}

int yamaha_f1_do_tattoo(drive_info* drive, unsigned char *iimage, uint32_t bsize){
	uint32_t	maxbuf = 20480;
	char		*crow;
	char		*imagec;
	uint32_t	i,j;
	uint32_t	tattoo_size;
	uint32_t	blen = 2048;

	drive->parms.status |= STATUS_LOCK;
	set_lock(drive);

	yamaha_f1_get_tattoo(drive);

	if (!drive->yamaha.tattoo_rows) {
		printf("No space left on CD! Can't write zero size tattoo!\n");
		drive->parms.status &= (~STATUS_LOCK);
		set_lock(drive);
		return 2;
	}

	tattoo_size = drive->yamaha.tattoo_rows * sizeof (tattoo_row);
	if (iimage) {
//		image = (tattoo_row*) iimage;
		if (bsize != tattoo_size) {
			printf("yamaha_f1_do_tattoo(): RAW image size must be exactly %d bytes (3744x%d)\n",
					tattoo_size,drive->yamaha.tattoo_rows);
			return 1;
		}
		imagec = (char*) iimage;
	} else {
		printf("yamaha_f1_do_tattoo(): got no image buffer! writing test image...\n");
//		image = (tattoo_row*) malloc( tattoo_size );
		imagec = (char*) malloc( tattoo_size );
		for (i=0; i<drive->yamaha.tattoo_rows; i++) {
			for (j=0; j< sizeof(tattoo_row); j++) {
				if (j<20) imagec[i*3744+j]=0xFF;
				else if ((j>1000) && (j<1040)) imagec[i*3744+j]=0xFF;
                else imagec[i*3744+j]=0;
//				image[i][j]=0xFF;
			}
		}
	}

	drive->parms.status |= STATUS_LOCK;
	set_lock(drive);

	yamaha_set_tattoo_speed(drive);

	printf("Sending T@2 data (%d rows)...\n", drive->yamaha.tattoo_rows);

//	int b=0;
	crow = imagec;
	i = 0;

	while (i<tattoo_size) {
		memset(drive->rd_buf, 0, maxbuf);
		blen = min(maxbuf, tattoo_size-i);
		memcpy(drive->rd_buf, crow+i, blen);
//		printf("block #%2d: %5d bytes / %2d sect\n",b,blen,blen/2048);

//		printf("block %d\n",i);
		if (!i) {
			if(yamaha_write(drive, 1, 0, drive->yamaha.tattoo_i, maxbuf/2048)) goto tattoo_err;
		} else {
			//	blen = (blen+2047) / 2048 * 2048;
			//	yamaha_write(drive, 1, 0, 0, 10, blen);
			if(yamaha_write(drive, 1, 0, 0, maxbuf/2048)) goto tattoo_err;
		}
//		b++;
		printf(".");
		i+=maxbuf;
	}
	if (yamaha_write(drive, 1, 0, drive->yamaha.tattoo_o, 0)) goto tattoo_err;
	printf(".\n");

	printf("Burning T@2...\n");
//	printf("\nwait_unit_ready()...\n");
	if (wait_unit_ready(drive, 1000)) printf("Error %05X...\n", drive->err);
//	printf("\nwait_fix()...\n");
	if (wait_fix(drive, 1000)) printf("Error %05X...\n", drive->err);

	printf("\nDone!\n");
	goto tattoo_done;

tattoo_err:
	sperror("Error writing T@2", drive->err);

tattoo_done:
	drive->parms.status &= (~STATUS_LOCK);
	set_lock(drive);

	if (drive->parms.status & STATUS_LOCK)
		{ drive->parms.status &= (~STATUS_LOCK);
		set_lock(drive); }
	load_eject(drive, false, false);

	if (!iimage) free(imagec);
	return 0;
}

