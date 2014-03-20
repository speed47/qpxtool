/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 *
 * Some Plextor commands got from PxScan and CDVDlib (C) Alexander Noe`
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
#include <math.h>
#include <unistd.h>
#include <common_functions.h>

#include <qpx_transport.h>
#include <qpx_mmc.h>

#include <qscan_plugin.h>

#define FETE_START_LBA

#define DEBUG 1
//#define _debug_cx
//#define _debug_pi
//#define _debug_jb
//#define _debug_fete

//*********************************//
//
//  commands to start tests
//
//*********************************//

const char PLEX_QCHECK_START	= 0x15;
const char PLEX_QCHECK_READOUT	= 0x16;
const char PLEX_QCHECK_END	= 0x17;

int scan_plextor::cmd_cd_errc_init()
{
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_START; //0x15;
	dev->cmd[2] = 0x00;
	dev->cmd[3] = 0x01;
	
//	dev->cmd[8] = 0x0F;
	dev->cmd[9] = 0x02;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport() ))
		{ if (!dev->silent) sperror ("PLEXTOR_START_CX", dev->err); return dev->err;}
#ifdef _debug_cx
	printf("00 18 01 01 00 4B |      LBA    |  BLER   E31   E21   E11   E32   ???   E22   E12\n");
#endif
	printf("scan init OK!\n");
	return 0;
}

int scan_plextor::cmd_dvd_errc_init()
{
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_START; //0x15;
	dev->cmd[2] = 0x00;
//	dev->cmd[3] = 0x09;
	dev->cmd[3] = 0x01;
	
	dev->cmd[8] = 0x01;
//	dev->cmd[9] = 0x13;
	dev->cmd[9] = 0x12;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport() ))
		{ if(!dev->silent) sperror ("PLEXTOR_START_PIPO",dev->err); return dev->err;}
	printf("scan init OK!\n");
	return 0;
}

#if 0
int plextor_start_pie(drive_info* dev) {
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_START; //0x15;
	dev->cmd[2] = 0x00;
	dev->cmd[3] = 0x00;
	dev->cmd[8] = 0x08; // scan interval (ECC blocks)
	dev->cmd[9] = 0x10;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport() ))
		{sperror ("PLEXTOR_START_PISUM8",dev->err);return dev->err;}
	return 0;
}

int plextor_start_pie_poe(drive_info* dev) {
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_START; //0x15;
	dev->cmd[2] = 0x00;
	dev->cmd[3] = 0x00;
	dev->cmd[8] = 0x08; // scan interval (ECC blocks)
	dev->cmd[9] = 0x11;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport() ))
		{sperror ("PLEXTOR_START_PISUM8_POE",dev->err);return dev->err;}
	return 0;
}

int plextor_start_pif(drive_info* dev) {
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_START; //0x15;
	dev->cmd[2] = 0x00;
	dev->cmd[3] = 0x00;
	dev->cmd[8] = 0x01; // scan interval (ECC blocks)
	dev->cmd[9] = 0x12;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport() ))
		{sperror ("PLEXTOR_START_PIF",dev->err); return dev->err;}
	return 0;
}
#endif

int scan_plextor::cmd_cd_jb_init()
{
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_START; //0x15;
	dev->cmd[2] = 0x10;
	dev->cmd[3] = 0x01; // CD

//	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport() ))
		{sperror ("PLEXTOR_START_JB_CD",dev->err);return dev->err;}
	printf("scan init OK!\n");
	return 0;
}

int scan_plextor::cmd_dvd_jb_init()
{
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_START; //0x15;
	dev->cmd[2] = 0x10;
	dev->cmd[3] = 0x00; // DVD
	dev->cmd[8] = 0x10; // scan interval (ECC blocks)

//	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport() ))
		{sperror ("PLEXTOR_START_JB_DVD",dev->err);return dev->err;}
	printf("scan init OK!\n");
	return 0;
}

int scan_plextor::cmd_fete_init()
{
	fete_idx=-1;
	dev->cmd[0] = PLEXTOR_SCAN_TA_FETE; // 0xF3;
	dev->cmd[1] = 0x1F;
	dev->cmd[2] = 0x03;
	dev->cmd[3] = 0x01;
#ifdef _debug_fete
#ifdef FETE_START_LBA
	printf("scan_plextor::cmd_fete_init: lba %d - %d\n", lba, dev->media.capacity_total-1);
#else
	printf("scan_plextor::cmd_fete_init: lba %d - %d\n", 0, dev->media.capacity_total-1);
#endif
#endif
	if (dev->media.type & DISC_CD) {
		fete_rsize = 4500; // one FE/TE info block per 60 seconds

		int sect;
		msf sect_msf;
		lba2msf(lba,&sect_msf);
//	start address
#ifdef FETE_START_LBA
		dev->cmd[4] = sect_msf.m;
		dev->cmd[5] = sect_msf.s;
		dev->cmd[6] = sect_msf.f;
#else
		dev->cmd[4] = 0x00;
		dev->cmd[5] = 0x00;
		dev->cmd[6] = 0x00;
#endif
//	end address
		sect = dev->media.capacity_total-1;
		lba2msf(sect,&sect_msf);
		dev->cmd[7] = sect_msf.m;
		dev->cmd[8] = sect_msf.s;
		dev->cmd[9] = sect_msf.f;
	} else if (dev->media.type & DISC_DVD) {
		fete_rsize = 25600; // one FE/TE info block per 1600 (?) ECC blocks 

//	start address
#ifdef FETE_START_LBA
		dev->cmd[4] = (lba >> 16) & 0xFF;
		dev->cmd[5] = (lba >> 8) & 0xFF;
		dev->cmd[6] =  lba & 0xFF;
#else
		dev->cmd[4] = 0x00;
		dev->cmd[5] = 0x00;
		dev->cmd[6] = 0x00;
#endif
//	end address
		dev->cmd[7] = ((dev->media.capacity_total-1) >> 16) & 0xFF;
		dev->cmd[8] = ((dev->media.capacity_total-1) >> 8) & 0xFF;
		dev->cmd[9] = (dev->media.capacity_total-1) & 0xFF;
	} else {
		return -1;
	}
	if ((dev->err=dev->cmd.transport() ))
		{sperror ("PLEXTOR_START_FETE",dev->err);return dev->err;}
	return 0;
}

//*********************************//
//
//  end scan commands
//
//*********************************//

int scan_plextor::cmd_scan_end()
{
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_END; //0x17;
	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport() ))
		{sperror ("PLEXTOR_END_SCAN",dev->err); return dev->err;}
	return 0;
}

int scan_plextor::cmd_fete_end()
{
	dev->cmd[0] = PLEXTOR_SCAN_TA_FETE; // 0xF3;
	dev->cmd[1] = 0x1F;
	dev->cmd[2] = 0x04;
	dev->cmd[9] = 0x00;
	if ((dev->err=dev->cmd.transport() ))
		{sperror ("PLEXTOR_END_FETE",dev->err); return dev->err;}
	return 0;
}

//*********************************//
//
//  test data readout commands 
//
//*********************************//

int scan_plextor::cmd_cd_errc_getdata(cd_errc *data)
{
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_READOUT; // 0x16;
	dev->cmd[2] = 0x01;
	dev->cmd[10]= 0x1A;

	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,0x1A)))
		{sperror ("PLEXTOR_READ_CD_ERROR_INFO", dev->err);return dev->err;}

	data->bler = ntoh16(dev->rd_buf+10);
	data->e31 = ntoh16(dev->rd_buf+12);
	data->e21 = ntoh16(dev->rd_buf+14);
	data->e11 = ntoh16(dev->rd_buf+16);

	data->uncr = ntoh16(dev->rd_buf+18); // check where drive returns E32
	data->e32 = ntoh16(dev->rd_buf+20);  // and where is UNCR
	data->e22 = ntoh16(dev->rd_buf+22);
	data->e12 = ntoh16(dev->rd_buf+24);

#ifdef _debug_cx
	int i;
	for (i=0x00; i<0x06; i++) printf("%02X ", dev->rd_buf[i] & 0xFF); printf("| ");
	for (i=0x06; i<0x0A; i++) printf("%02X ", dev->rd_buf[i] & 0xFF); printf("| ");
	for (i=0x0A; i<0x1A; i+=2) { if (ntoh16(dev->rd_buf+i)) printf("%5d ", ntoh16(dev->rd_buf+i)); else printf("_____ "); }
	printf("|\n");
#endif
	return 0;
}

int scan_plextor::cmd_dvd_errc_getdata(dvd_errc *data)
{
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_READOUT; // 0x16;
	dev->cmd[2] = 0x00;
	dev->cmd[10]= 0x34;

	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,0x34)))
		{sperror ("PLEXTOR_READ_DVD_ERROR_INFO",dev->err); return dev->err;}
/*
test 0x12:
	10 UNCR
	14 PIE
	18 POE
	1C
	20
	24 PIF
	28 POF
	2C
	30
*/

	data->pie = ntoh32(dev->rd_buf+0x14);
//	data->pi8 = data->pie;
	data->pif = ntoh32(dev->rd_buf+0x24);

	data->poe = ntoh32(dev->rd_buf+0x18)>>4;
//	data->po8 = data->poe;
	data->pof = ntoh32(dev->rd_buf+0x28);

/*
	data->pie = ntoh32(dev->rd_buf+0x14);
	data->pi8 = ntoh32(dev->rd_buf+0x18);
	data->pif = ntoh32(dev->rd_buf+0x1C);

	data->poe = ntoh32(dev->rd_buf+0x20);
	data->po8 = ntoh32(dev->rd_buf+0x24);
	data->pof = ntoh32(dev->rd_buf+0x28);
	data->uncr = ntoh32(dev->rd_buf+0x10);
*/
	return 0;
}

#if 0
int plextor_read_pi_info(drive_info* dev) {
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_READOUT; // 0x16;
	dev->cmd[2] = 0x00;
	dev->cmd[10]= 0x34;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,0x34)))
		{sperror ("PLEXTOR_READ_PI",dev->err); return dev->err;}
/*
	printf("READ PI:");
	for (int i=0; i<0x34; i++) {
		if (!(i%0x10)) printf("\n");
		printf(" %02X",dev->rd_buf[i] & 0xFF);
	}
	printf("\n");

*/
#ifdef _debug_pi
	int i;
/*
//	printf("\n| ");
	for (i=0x00; i<0x34; i++) {
		if (!(i%0x20))printf("\n| ");
		printf("%02X ", dev->rd_buf[i] & 0xFF);
	}
*/
	for (i=0x00; i<0x06; i++) printf("%02X ", dev->rd_buf[i] & 0xFF); printf("|");
	printf(" %6X |", ntoh32(dev->rd_buf+0x06) - 0x030000);
	printf(" %4X |", ntoh16(dev->rd_buf+0x0A));
	printf(" %4X",   ntoh16(dev->rd_buf+0x0C));

	for (i=0x20; i<0x34; i+=4) {
		if (!(i%0x10))printf(" |");
		if (ntoh32(dev->rd_buf+i))
			printf(" %8d", ntoh32(dev->rd_buf+i));
		else
			printf(" ________");
	}
//	printf("\n");
#endif
	return 0;
}
#endif

int scan_plextor::cmd_jb_getdata(cdvd_jb *data)
{
	dev->cmd[0] = PLEXTOR_QCHECK; //0xEA;
	dev->cmd[1] = PLEX_QCHECK_READOUT; // 0x16;
	dev->cmd[2] = 0x10;
	dev->cmd[10]= 0x10;

	dev->cmd[11]= 0x00;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,0x10)))
		{sperror ("PLEXTOR_READ_JB",dev->err); return dev->err;}
#ifdef _debug_jb
	int i;
	printf("\n| J/B data: | ");
	for (i=0x00; i<0x10; i++) printf("%02X ", dev->rd_buf[i] & 0xFF);
	printf("|\n");
#endif

//	if (dev->dev_ID > PLEXTOR_716) {
		data->asymm  = ntoh16(dev->rd_buf+10);
		data->jitter = ntoh16(dev->rd_buf+12);
//	} else {
//		data->asymm  = ntoh16(dev->rd_buf+10);
//		data->jitter = ntoh16(dev->rd_buf+12);
//	}
	return 0;
}

int scan_plextor::cmd_cd_errc_block(cd_errc *data)
{
	int rsize=15;
	data->uncr=0;
//	if (!lba) return -1;
	for (int i=0; (i<5) && lba<dev->media.capacity; i++) {
		if (lba + 15 > dev->media.capacity)
			rsize = dev->media.capacity - lba;
//		else
//			rsize = 15;

		if (read_cd(dev, dev->rd_buf, lba, rsize, 0xFA)) data->uncr++;
		lba+=rsize;
	}
	cmd_cd_errc_getdata(data);
//	*lba = 
	return 0;
}

int scan_plextor::cmd_dvd_errc_block(dvd_errc *data)
{
	if (read_one_ecc_block(dev, dev->rd_buf, lba)) data->uncr++;
	read_one_ecc_block(dev, dev->rd_buf, lba);
	lba+= 0x10;
	cmd_dvd_errc_getdata(data);
	return 0;
}

#if 0
int plextor_pif_do_one_ecc_block(drive_info* dev, int* lba, int* pif) {
	read_one_ecc_block(dev, *lba);
	*lba+= 0x10;
	plextor_read_pi_info(dev);
//	*lba = ntoh32(dev->rd_buf+0x06) - 0x00030000;
	*pif = ntoh32(dev->rd_buf+0x24);
#ifdef _debug_pi
	printf(" * %4d\n", *pif);
#endif
	return 0;
}

int plextor_pisum8_do_eight_ecc_blocks(drive_info* dev, int* lba, int* pie, int* pof) {
	for (int i=0;i<8;i++) {
		if ((dev->err = read_one_ecc_block(dev, *lba)));// i = 8;
		*lba+= 0x10;
	}
	plextor_read_pi_info(dev);
//	*lba = ntoh32(dev->rd_buf+0x06) - 0x00030000;
	*pie = ntoh32(dev->rd_buf+0x24);
	*pof = ntoh32(dev->rd_buf+0x10);
//	*poe = ntoh32(dev->rd_buf+0x28);
//	*pif = 0;
#ifdef _debug_pi
	printf(" * %4d\n", *pie);
#endif
	return 0;
}

int plextor_burst_do_eight_ecc_blocks(drive_info* dev, int* lba, int* pie, int* poe, int* pof) {
	for (int i=0;i<8;i++) {
		if ((dev->err = read_one_ecc_block(dev, *lba)));// i = 8;
		*lba+= 0x10;
	}
	plextor_read_pi_info(dev);
//	*lba = ntoh32(dev->rd_buf+0x06) - 0x00030000;
	*pie = ntoh32(dev->rd_buf+0x24);
//
	if (ntoh16(dev->rd_buf+0x0C) > ntoh16(dev->rd_buf+0x0A))
		*poe = ntoh32(dev->rd_buf+0x28) >> 1;
	else
		*poe = 0;
	*pof = ntoh32(dev->rd_buf+0x10);

#ifdef _debug_pi
	printf(" * %4d * %4d\n", *pie, *poe);
#endif
	return 0;
}
#endif

int scan_plextor::cmd_dvd_jb_block(cdvd_jb *data)
{
	for (int i=0;i<16;i++) {
		int j = read_one_ecc_block(dev, dev->rd_buf, lba);
		if (j == COMMAND_FAILED) i=16;
		lba+= 0x10;
	}

	cmd_jb_getdata(data);
	if (dev->dev_ID > PLEXTOR_716) {
		data->jitter = 3200 - 2*data->jitter;
	} else {
		data->jitter = 3200 - (int)(2.4*data->jitter);
	}
	return 0;
//	return (!(dev->rd_buf[2]));
}

int scan_plextor::cmd_cd_jb_block(cdvd_jb *data)
{
	int rsize=15;
	for (int i=0;i<5;i++) {
		if (lba + 15 > dev->media.capacity)
			rsize = dev->media.capacity - lba;

		int j = read_cd(dev, dev->rd_buf, lba, rsize, 0xFA);
		lba+= rsize;
		if (j == COMMAND_FAILED) i++;
	}

	cmd_jb_getdata(data);
	if (dev->dev_ID > PLEXTOR_716) {
		data->jitter = 4800 - 2*(data->jitter);
	} else {
		data->jitter = 3600 - (int)(2.4*(data->jitter));
	}
	return 0;
//	return (!(dev->rd_buf[2]));
}

int scan_plextor::cmd_fete_getdata()
{
	dev->cmd[0] = PLEXTOR_FETE_READOUT; // 0xF5;
	dev->cmd[3] = 0x0C;
	dev->cmd[9] = 0xCE;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,0xCE)))
		{sperror ("PLEXTOR_FETE_READOUT",dev->err); return dev->err;}
#ifdef _debug_fete
	for (int i=0; i<0xCE; i++) {
		if (!(i % 0x20)) printf("\n");
		printf("%02X ",dev->rd_buf[i] & 0xFF);
	}
	printf("\n");
#endif
	return 0;
}

int scan_plextor::cmd_fete_block(cdvd_ft* data)
{
	int rdy = test_unit_ready(dev);
	int offs=((++fete_idx) << 1) + 8;
	if (rdy != 0x20408) {
		printf("test unit ready = %05X, return\n", rdy);
		return -1;
	}
//	if (cmd_fete_getdata()) return -1;
//	data->te=dev->rd_buf[offs];
//	data->fe=dev->rd_buf[offs+1];
	data->te=0;
	data->fe=0;
	if (fete_idx>99) return 1;
	lba = (fete_idx+1) * fete_rsize;
	if (lba > (dev->media.capacity_total-1)) lba = dev->media.capacity_total-1;

	while ((data->fe==0 || data->te==0) && (fete_idx<=99) && (rdy == 0x20408)) {
		if (cmd_fete_getdata()) return -1;
		data->te=dev->rd_buf[offs];
		data->fe=dev->rd_buf[offs+1];
		if (data->fe==0 || data->te==0) {
			rdy = test_unit_ready(dev);
#ifdef _debug_fete
			printf("test unit ready = %05X\n", rdy);
#endif
			msleep(10);
		}
	}
	return 0;
}

int scan_plextor::build_TA_histogram_px716(unsigned char* response_data, int* dest_pit, int* dest_land, int len) {
	int* dest[] = { dest_land, dest_pit };
	int count = ntoh16(response_data+2);
//	printf("PX-716 Histogram... %d\n",count);
	int idx=28;
	int v, pit;
	for (int i=0;i<count;i++) {
		v = ntoh16u(response_data+idx);
		pit = !!(v & 0x8000);
		v &=~0x8000;
		dest[pit][min(v, len-1)]++;
		idx+=2;
	}
	return 0;
}

int scan_plextor::build_TA_histogram_px755(unsigned char* response_data, int* dest_pit, int* dest_land, int len, int dt) { //, int spd) {
	int* dest[] = { dest_land, dest_pit };
	int count = ntoh16(response_data+2);
//	printf("PX-755/760 Histogram... %d\n", count);
	int idx=28;
	int v, pit;
	for (int i=0;i<count;i++) {
		v = ntoh16u(response_data+idx);
		pit = !!(v & 0x8000);
		v &=~0x8000;
		if (dt & DISC_DVDplus)
			dest[pit][min( (int)(v*1.452), len-1)]++; // DVD+R(W), speed 2.4x
		else
			dest[pit][min( (int)(v*1.21), len-1)]++; // DVD-R(W), speed 2x
		idx+=2;
	}
	return 0;
}

int scan_plextor::evaluate_histogramme(cdvd_ta* data, int** peaks, int** mins) {
	int i, j1, j2;
	int local_max = 0;
	int next_peak = 0;
	int peak_found= 0;
	int* src[] = { data->pit, data->land };

	for (int k=0;k<2;k++) {
		j1=0; j2=0; local_max = 0;
		for (i=40;i<330;i++) {
			if (src[k][i-1] <= src[k][i] && src[k][i+1] <= src[k][i] && src[k][i] > 20 && src[k][i] > local_max) {
				peaks[k][j1] = i;
				local_max = src[k][i];
				next_peak = 1;
			} else if (peak_found)
			if (/*src[k][i-3] >= src[k][i-1] && src[k][i-2] > src[k][i-1] && */
			    src[k][i-1] > src[k][i] && src[k][i+1] >= src[k][i]) {
				mins[k][j2] = i;
				if (j2<13) j2++;
				peak_found = 0;
			}

			if (local_max > 2*src[k][i]) {
				local_max = 2*src[k][i];
				if (next_peak) {
					next_peak = 0;
					if (j1<13) {
						j1++;
						peak_found=1;
					//	printf("%4d",i);
					}
				}
			}
		}

		int min_count = j2;
		for (i=0;i<min_count;i++) {
			int start = (i?mins[k][i-1]:0);
			int end   = mins[k][i];
			int sum   = 0;
			int partsum=0;
			int j;
			for (j=start;j<end;sum+=src[k][j++]);
			for (j=start;partsum<sum/2;partsum+=src[k][j++]);
			peaks[k][i] = (peaks[k][i] + j-1)/2;
		}
	}
	return 0;
}

int	scan_plextor::cmd_dvd_ta_block (cdvd_ta* data) {
//	printf("Start PLEXTOR TA test on %d layers\n",dev->media.layers);
	int i, r, m,j;
	float sum;
	unsigned char scan_cmd[6][2] = {
		{ 0x04, 0x00 }, { 0x10, 0x00 }, { 0x20, 0x00 },
		{ 0xFA, 0x28 }, { 0xEA, 0x28 }, { 0xDE, 0x28 }};
	const char* scan_txt[] = {
		"Running TA on L0 inner zone ", "Running TA on L0 middle zone", "Running TA on L0 outer zone",
		"Running TA on L1 inner zone ", "Running TA on L1 middle zone", "Running TA on L1 outer zone"
	};
//	int ta_response_pit[6][512]; int ta_response_land[6][512];
	int peaks_lands[15], peaks_pits[15]; int mins_lands[15], mins_pits[15];
	int* peaks[] = { peaks_pits, peaks_lands };
	int* mins[]  = { mins_pits+1, mins_lands+1 };

	if (data->pass<0 || data->pass>6)
		return -1;

	wait_unit_ready(dev, 6);
//	for (int pass=0;(pass<3*dev->media.layers) && (!ctl->skip());pass++) {
	printf("%s\n",scan_txt[data->pass]);

	data->clear();
	for (i=0;i<9;i++) {
		dev->cmd[0] = PLEXTOR_SCAN_TA_FETE; // 0xF3;
		dev->cmd[1] = 0x1F;
		dev->cmd[2] = 0x23;
		dev->cmd[3] = 0x00;
		dev->cmd[4] = 0x00;
		dev->cmd[5] = scan_cmd[data->pass][0];
		dev->cmd[6] = scan_cmd[data->pass][1];
		dev->cmd[7] = i<<4;
		dev->cmd[8] = 0xFF;
		dev->cmd[9] = 0xFE;
		dev->cmd[10] = 0x04*!i;
		dev->cmd[11] = 0x00;
		dev->cmd.transport(READ, dev->rd_buf, 65534);
		printf(".\n");
		if (!strncmp(dev->dev,"DVDR   PX-714A",14)  ||
		    !strncmp(dev->dev,"DVDR   PX-716A",14))
			build_TA_histogram_px716(dev->rd_buf, data->pit, data->land, 512);
		else
			build_TA_histogram_px755(dev->rd_buf, data->pit, data->land, 512, dev->media.type);
			// dev->parms.scan_speed_dvd);
	}  //////
	mins_lands[0] = 0; mins_pits[0]=0;

	int p0,p1;
	int l0,l1;
	for  (j=1;j<400;j++) {
//		if ((j>40) && (j<360)) {
			if ( data->pit[j] == 0 ) {
				p0 = data->pit[j-1];  p1 = data->pit[j+1];
				if ((p0>0) && (p1>0)) data->pit[j] = (p0+p1)/2;
			}
			if ( data->land[j] == 0 ) {
				l0 = data->land[j-1]; l1 = data->land[j+1];
				if ((l0>0) && (l1>0)) data->land[j] = (l0+l1)/2;
			}
//		}

//		block.pit=ta_response_pit[pass][j];
//		block.land=ta_response_land[pass][j];
//		ctl->event_block_done(event_block_done_ta, block);
	}

	evaluate_histogramme(data, peaks, mins);

	printf("peak shift pits : ");
	sum=0;
	for (m=0;m<10;m++) {
		r = (int)((float)peaks_pits[m] - 21.5454 * ((m<9)?m:11) - 64); sum+=sqrt(abs(r));
		printf("%4d", r);
	}
	printf("  sum %f \n",sum);
	printf("peak shift lands: ");

	sum=0;
	for (m=0;m<10;m++) {
		r = (int)((float)peaks_lands[m] - 21.5454 * ((m<9)?m:11) - 64); sum+=sqrt(abs(r));
		printf("%4d", r);
	}
	printf("  sum %f \n",sum);


//	printf("TA test finished\n");
//        cmd_scan_end();
	return 0;
}

