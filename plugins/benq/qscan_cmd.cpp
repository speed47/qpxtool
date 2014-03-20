/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <sys/time.h>

#include <qpx_transport.h>
#include <qpx_mmc.h>
#include <common_functions.h>

#include <qscan_plugin.h>

//#define _BENQ_DEBUG 1
#define _BENQ_DEBUG2 1


int scan_benq::cmd_check_mode_init()
{
	dev->cmd[0] = 0xFD;
	dev->cmd[1] = 0xF1;
	dev->cmd[2] = 0x42;
	dev->cmd[3] = 0x45;
	dev->cmd[4] = 0x4E;
	dev->cmd[5] = 0x51;
#ifdef _BENQ_DEBUG
	printf("benq_check_mode_init\n");
#endif
	if ((dev->err=dev->cmd.transport(NONE,NULL,0))){
		if(!dev->silent) sperror ("benq_check_mode_init",dev->err);
		return dev->err;
	}
	return 0;
}

int scan_benq::cmd_check_mode_exit()
{
	dev->cmd[0] = 0xFD;
	dev->cmd[1] = 0xF2;
	dev->cmd[2] = 0x42;
	dev->cmd[3] = 0x45;
	dev->cmd[4] = 0x4E;
	dev->cmd[5] = 0x51;
#ifdef _BENQ_DEBUG
	printf("benq_check_mode_exit\n");
#endif
	if ((dev->err=dev->cmd.transport(NONE,NULL,0))){
		if(!dev->silent) sperror ("benq_check_mode_exit",dev->err);
		return dev->err;
	}
	return 0;
}

int scan_benq::cmd_get_result()
{
	dev->cmd[0] = 0xF8;
	dev->cmd[8] = 0x02;
#ifdef _BENQ_DEBUG
	printf("BENQ_CHECK_RESULT\n");
#endif
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,2))){
		if(!dev->silent) sperror ("BENQ_CHECK_RESULT",dev->err);
		return dev->err;
	}
	printf("BENQ Check result: %02d %02d\n", dev->rd_buf[0], dev->rd_buf[1]);
	return ( (dev->rd_buf[0] << 8) | dev->rd_buf[1]);
}

int scan_benq::cmd_set_speed(unsigned char sidx)
{
	dev->rd_buf[0] = 0xD2;
	dev->rd_buf[1] = 0x0A;
	dev->rd_buf[2] = sidx;
	dev->rd_buf[3] = 0x00;

	dev->cmd[0] = 0xF9;
	dev->cmd[8] = 0x04;
#ifdef _BENQ_DEBUG
	printf("BENQ_SET_ERRC_SPEED\n");
#endif
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,4))){
		if(!dev->silent) sperror ("BENQ_SET_ERRC_SPEED",dev->err);
		return dev->err;
	}
	return (cmd_get_result());
}

int scan_benq::cmd_start_errc(int lba)
{
	dev->rd_buf[0] = 0xD4;
	dev->rd_buf[1] = 0x91;
	dev->rd_buf[2] = (lba >> 16) & 0xFF;
	dev->rd_buf[3] = (lba >>  8) & 0xFF;
	dev->rd_buf[4] = lba & 0xFF;
	dev->rd_buf[5] = 0x00;

	dev->cmd[0] = 0xF9;
	dev->cmd[8] = 0x06;
#ifdef _BENQ_DEBUG
	printf("BENQ_ERRC_SEEK\n");
#endif
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,6))){
		if(!dev->silent) sperror ("BENQ_ERRC_SEEK",dev->err);
		return dev->err;
	}
	return (cmd_get_result());
}

int scan_benq::cmd_start_fete(int lba)
{
	dev->cmd[0] = 0xFD;
	dev->cmd[1] = 0xFB;
	dev->cmd[2] = 0x42;
	dev->cmd[3] = 0x45;
	dev->cmd[4] = 0x4E;
	dev->cmd[5] = 0x51;

	dev->cmd[6] = 0x05;
	dev->cmd[7] = 0x02;
	dev->cmd[8] = (lba >> 16) & 0xFF;
	dev->cmd[9] = (lba >>  8) & 0xFF;
	dev->cmd[10]= lba & 0xFF;
	dev->cmd[11]= 0x04;
#ifdef _BENQ_DEBUG
	printf("BENQ_START_FETE\n");
#endif
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,8))){
		if(!dev->silent) sperror ("BENQ_START_FETE",dev->err);
		return dev->err;
	}
	return 0;
}

// ************* Scan init commands *********
int scan_benq::cmd_cd_errc_init(int &speed)
{
	int r;
	if ((r = cmd_check_mode_init())) return r;
// ************
// available test speeds for CD
// 0x00 -  8x
// 0x01 - 12x
// 0x06 - 24x
// 0x08 - 32x
// 0x09 - 40x
// 0x0B - 48x
// 0x10 -  8x CLV
// 0x12 - 12x CLV
// 0x13 - 16x CLV
// 0x14 - 24x P-CAV
// 0x15 - 32x P-CAV

	if      (speed>=48) { speed = 48; sidx=0x0B; } // 48x
	else if (speed>=40) { speed = 40; sidx=0x09; } // 40x
	else if (speed>=32) { speed = 32; sidx=0x08; } // 32x
	else if (speed>=24) { speed = 24; sidx=0x06; } // 24x
	else if (speed>=16) { speed = 16; sidx=0x13; } // 16x CLV
	else if (speed>=12) { speed = 12; sidx=0x12; } // 12x CLV
	else				{ speed =  8; sidx=0x10; } //  8x CLV

	if (cmd_set_speed(sidx)) return dev->err;

// ************
	memset(dev->rd_buf, 0, 10);
	dev->rd_buf[0] = 0xC8;
	dev->rd_buf[1] = 0x99;
	dev->rd_buf[2] = 0x79;

	dev->cmd[0] = 0xF9;
	dev->cmd[8] = 0x0A;
#ifdef _BENQ_DEBUG
	printf("benq_init_cx_scan_3\n");
#endif
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,10))){
		if(!dev->silent) sperror ("benq_init_cx_scan_3",dev->err);
		return dev->err;
	}
	cmd_get_result();

// ************
	if (cmd_start_errc(0)) return 1;
	return 0;
}


int scan_benq::cmd_dvd_errc_init(int &speed)
{
	int r;
	if ((r = cmd_check_mode_init())) return r;
// ************
// available test speeds for DVD
// 0x02 -  4x
// 0x04 -  6x
// 0x05 -  8x
// 0x08 - 12x
// 0x0A - 16x
// 0x0C -  1x CLV
// 0x0D -  2x CLV
// 0x0E -  4x CLV
// 0x0F -  6x CLV
// 0x10 -  8x P-CAV
	if      (speed >= 16) { speed = 16; sidx=0x0A; } // 16x CAV
	else if (speed >= 12) { speed = 12; sidx=0x08; } // 12X CAV
	else if (speed >=  8) { speed = 8;  sidx=0x05; } //  8x CAV
	else if (speed >=  6) { speed = 6;  sidx=0x0F; } //  6x CLV
	else if (speed >=  4) { speed = 4;  sidx=0x0E; } //  4x CLV
	else if (speed >=  2) { speed = 2;  sidx=0x0D; } //  2x CLV
	else                  { speed = 1;  sidx=0x0C; } //  1x CLV

	if (cmd_set_speed(sidx)) return dev->err;

// ************
	memset(dev->rd_buf, 0, 10);
	dev->rd_buf[0] = 0xC8;
	dev->rd_buf[1] = 0x99;
	dev->rd_buf[2] = 0x79;

	dev->cmd[0] = 0xF9;
	dev->cmd[8] = 0x0A;
#ifdef _BENQ_DEBUG
	printf("benq_init_pi_scan_3\n");
#endif
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,10))){
		if(!dev->silent) sperror ("benq_init_pi_scan_3",dev->err);
		return dev->err;
	}
	if (cmd_get_result()) return 1;

// ************
	if (cmd_start_errc(0x030000)) return 1;
	return 0;
}

int scan_benq::cmd_dvd_fete_init(int &speed)
{
	int r;

	dev->cmd[0] = 0xFD;
	dev->cmd[1] = 0xF1;
	dev->cmd[2] = 0x42;
	dev->cmd[3] = 0x45;
	dev->cmd[4] = 0x4E;
	dev->cmd[5] = 0x51;
#ifdef _BENQ_DEBUG
	printf("benq_check_mode_init\n");
#endif
	if ((dev->err=dev->cmd.transport(NONE,NULL,0))){
		if(!dev->silent) sperror ("benq_check_mode_init",dev->err);
		return dev->err;
	}

// 0x13 16X
// 0x12 12X
// 0x11 10X ?
// 0x10  8X
// 0x0F  6X ?
// 0x0E  4X
// 0x0D  2.4X / 2X
	if      (speed >= 16) { speed = 16; sidx=0x13; }
	else if (speed >= 12) { speed = 12;	sidx=0x12; }
	else if (speed >= 10) { speed = 10; sidx=0x11; }
	else if (speed >=  8) { speed = 8;  sidx=0x10; }
	else if (speed >=  6) { speed = 6;  sidx=0x0F; }
	else if (speed >=  4) { speed = 4;  sidx=0x0E; }
	else                  { speed = 2;  sidx=0x0D; }

	if ((r = cmd_check_mode_init())) return r;

	dev->cmd[0] = 0xFD;
	dev->cmd[1] = 0xFB;
	dev->cmd[2] = 0x42;
	dev->cmd[3] = 0x45;
	dev->cmd[4] = 0x4E;
	dev->cmd[5] = 0x51;

	dev->cmd[6] = 0x05;
	dev->cmd[7] = 0x02;
	dev->cmd[8] = 0x25;
	dev->cmd[9] = 0xC0;
	dev->cmd[10]= 0x00;
	dev->cmd[11]= 0x03;

	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,8))){
		if(!dev->silent) sperror ("benq_fete_init_1",dev->err);
		return dev->err;
	}
	return (cmd_start_fete(0x030000));
}

// ************ Scan commands ***********
int scan_benq::cmd_read_block()
{
	dev->rd_buf[0] = 0xC1;
	dev->rd_buf[1] = 0x9A;
	dev->rd_buf[2] = 0x00;
	dev->rd_buf[3] = 0x00;

	dev->cmd[0] = 0xF9;
	dev->cmd[8] = 0x04;
#ifdef _BENQ_DEBUG
	printf("benq_scan_block\n");
#endif
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,4))){
		sperror ("benq_scan_block",dev->err);
		return dev->err;
	}
	return 0;
}

int scan_benq::cmd_getdata()
{
	dev->cmd[0] = 0xF8;
	dev->cmd[7] = 0x01;
	dev->cmd[8] = 0x02;
#ifdef _BENQ_DEBUG
	printf("benq_read_err\n");
#endif
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,258))){
		sperror ("benq_read_err",dev->err);
		return dev->err;
	}
	return 0;
}

// ***********
//  CD tests
// ***********

int scan_benq::cmd_cd_errc_block(cd_errc *data)
{
	int i,m,s,f,plba;
#ifdef _BENQ_DEBUG
	printf("benq_cx_do_one_interval. LBA=%d\n",lba);
#endif
	int found=0;
	int cnt=128;
	do {
		cmd_read_block();
		cmd_getdata();
		if ((!dev->rd_buf[0]) && (dev->rd_buf[1] == 0x63) && (dev->rd_buf[2] == 0x64) && (dev->rd_buf[3] == 0x6E)) {
			found = 1;
#ifdef _BENQ_DEBUG2
			printf("\nData block found...\n");
		} else {
//			printf(".");
//			return 0;
#endif
		}
		msleep(20);
		cnt--;
	} while ((!found) && (cnt));
	if (!cnt) return 1;
#ifdef _BENQ_DEBUG2
	for (i=0; i<32; i++) {
		if (!(i%8)) printf("| ");
		printf("%02X ", dev->rd_buf[i] & 0xFF);
	}
	printf("|\n");
#endif

	data->e11 = ntoh16(dev->rd_buf+0x0C);
	data->e21 = ntoh16(dev->rd_buf+0x0E); // +0x0E or +0x30 ??
	data->e31 = ntoh16(dev->rd_buf+0x28);
	data->bler = data->e11 + data->e21 + data->e31;

	data->e12 = ntoh16(dev->rd_buf+0x10);
	data->e22 = ntoh16(dev->rd_buf+0x12);
	data->e32 = ntoh16(dev->rd_buf+0x2A);
//	*lba+=75;
//	cnt++;
	m = ((dev->rd_buf[7] >> 4) & 0x0F)*10 + (dev->rd_buf[7] & 0x0F);
	s = ((dev->rd_buf[8] >> 4) & 0x0F)*10 + (dev->rd_buf[8] & 0x0F);
	f = ((dev->rd_buf[9] >> 4) & 0x0F)*10 + (dev->rd_buf[9] & 0x0F);
	plba = lba;
	lba = (m*60 + s)*75 + f;
//	printf("MSF: %02d:%02d.00 ; LBA: %d; C1:%4d; C2:%4d\n",m,s,*lba,*BLER,*E22);
	if ((lba-plba) > 150) lba = plba+75;
	if (lba<plba) {
	    printf("\nDrive returned invalid LBA, terminating scan!\n");
	    return 1;
	}
	return 0;
}

/*
int benq_jitter_CD_do_one_interval(dev_info* dev, int* lba, int* jitter, short int* beta, int blklen)
{
	benq_scan_block(dev);
	benq_read_err(dev);

	*jitter = 0;
	*beta = 0;
//	*lba=((int)dev->rd_buf[0] * 4500 + (int)dev->rd_buf[3] * 75 + (int)dev->rd_buf[2]);
	*lba = (((dev->rd_buf[7] & 0xF0)*10 + (dev->rd_buf[7] & 0x0F))*60 +  (dev->rd_buf[7] & 0xF0))*10 + (dev->rd_buf[7] & 0x0F);
	return 0;
}
*/

// ***********
// DVD tests
// ***********

int scan_benq::cmd_dvd_errc_block(dvd_errc *data)
{
	int i,plba;
//	int pif;
#ifdef _BENQ_DEBUG
	printf("benq_pie_pif_do_one_interval. LBA=%d\n",lba);
#endif
	int found=0;
	int cnt=256;
	do {
		cmd_read_block();
		cmd_getdata();
		if ((!dev->rd_buf[0]) && (dev->rd_buf[1] == 0x64) && (dev->rd_buf[2] == 0x76) && (dev->rd_buf[3] == 0x64)) {
			found = 1;
#ifdef _BENQ_DEBUG2
			printf("\nData block found...\n");
		} else {
			printf(".");
//			return 0;
#endif
		}
		msleep(20);
		cnt--;
	} while ((!found) && (cnt));
	if (!cnt) return 1;
#ifdef _BENQ_DEBUG2
	for (i=0; i<32; i++) {
		if (!(i%8)) printf("| ");
		printf("%02X ", dev->rd_buf[i] & 0xFF);
	}
	printf("|\n");
#endif
	// 1PIE/ROW + 2PIE/ROW + 3PIE/ROW + 4PIE/ROW + 5PIE/ROW
	data->pie = ntoh16(dev->rd_buf+0x0C)
		+ntoh16(dev->rd_buf+0x0E)
		+ntoh16(dev->rd_buf+0x10)
		+ntoh16(dev->rd_buf+0x12)
		+ntoh16(dev->rd_buf+0x14);
	data->pif = ntoh16(dev->rd_buf+0x16);
	data->poe =
//		+ntoh16(dev->rd_buf+0x18)
		+ntoh16(dev->rd_buf+0x1A)
		+ntoh16(dev->rd_buf+0x1C)
		+ntoh16(dev->rd_buf+0x1E)
		+ntoh16(dev->rd_buf+0x20)
		+ntoh16(dev->rd_buf+0x22);
	data->pof = ntoh16(dev->rd_buf+0x38);
	plba = lba;
	lba = (((dev->rd_buf[7]-3) << 16 )& 0xFF0000) + ((dev->rd_buf[8] << 8)&0xFF00 ) + (dev->rd_buf[9] & 0xFF);
//	printf("LBA: %d; PIE:%4d; PIF:%4d\n",*lba,*pie, *pif);
	if ((lba-plba) > 32) lba = plba+32;
	if (lba<plba) {
	    printf("\nDrive returned invalid LBA, terminating scan!\n");
	    return 1;
	}
	return 0;
}

int scan_benq::cmd_dvd_fete_block(cdvd_ft *data)
{
	dev->cmd[0] = 0xFD;
	dev->cmd[1] = 0xFB;
	dev->cmd[2] = 0x42;
	dev->cmd[3] = 0x45;
	dev->cmd[4] = 0x4E;
	dev->cmd[5] = 0x51;

	dev->cmd[6] = 0x05;
	dev->cmd[7] = sidx;
	dev->cmd[8]  = (lba >> 16) & 0xFF + 0x03;
	dev->cmd[9]  = (lba >>  8) & 0xFF;
	dev->cmd[10] = lba & 0xFF;
	dev->cmd[11] = 0;

	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,8))){
		if(!dev->silent) sperror ("benq_fete_block",dev->err);
		return dev->err;
	}

	data->te = (ntoh16(dev->rd_buf+3) + 5) / 10;
	data->fe = (ntoh16(dev->rd_buf+5) + 5) / 10;
	lba+=0x1000;
	return 0;
}

// ************* END SCAN COMMAND *********
int scan_benq::cmd_cd_end()
{
	int r;
#if 1
	memset(dev->rd_buf, 0, 10);
	dev->rd_buf[0] = 0xD4;
	dev->rd_buf[1] = 0x91;

	dev->cmd[0] = 0xF9;
	dev->cmd[8] = 0x0A;
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,10))){
		sperror ("benq_end_scan_0",dev->err);
		return dev->err;
	}
	cmd_get_result();

// ************
	dev->rd_buf[0] = 0xD4;
	dev->rd_buf[1] = 0x91;
	dev->rd_buf[2] = 0x00;
	dev->rd_buf[3] = 0x02;
	dev->rd_buf[4] = 0x00;
	dev->rd_buf[5] = 0x00;

	dev->cmd[0] = 0xF9;
	dev->cmd[8] = 0x06;
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,6))){
		sperror ("benq_end_scan_2",dev->err);
		return dev->err;
	}

	cmd_get_result();
#endif
// ************
	if ((r = cmd_check_mode_exit())) return r;
	return 0;
}


int scan_benq::cmd_dvd_end()
{
	int r;
#if 1
	memset(dev->rd_buf, 0, 10);
	dev->rd_buf[0] = 0xD4;
	dev->rd_buf[1] = 0x91;

	dev->cmd[0] = 0xF9;
	dev->cmd[8] = 0x0A;
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,10))){
		sperror ("benq_end_scan_0",dev->err);
		return dev->err;
	}
	cmd_get_result();

// ************
	dev->rd_buf[0] = 0xD4;
	dev->rd_buf[1] = 0x91;
	dev->rd_buf[2] = 0x03;
	dev->rd_buf[3] = 0x00;
	dev->rd_buf[4] = 0x00;
	dev->rd_buf[5] = 0x00;

	dev->cmd[0] = 0xF9;
	dev->cmd[8] = 0x06;
	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,6))){
		sperror ("benq_end_scan_2",dev->err);
		return dev->err;
	}
	cmd_get_result();

#endif
// ************
	if ((r = cmd_check_mode_exit())) return r;
	return 0;
}

