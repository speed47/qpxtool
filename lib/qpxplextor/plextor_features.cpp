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
 *
 * Some Plextor commands got from PxScan and CDVDlib (C) Alexander Noe`
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/time.h>
#include <unistd.h>

#include <qpx_transport.h>
#include <qpx_mmc.h>
#include "plextor_features.h"

static const char progress[] = { '-', '\\', '|', '/', 0 };

#define ASDB_SAVE_DEBUG
#define ASDB_LOAD_DEBUG
// #define DEBUG_SECUREC
//#define SHOW_RAW_TIME

int plextor_reboot(drive_info* drive) {
	drive->cmd[0] = 0xEE;
	if ((drive->err=drive->cmd.transport(NONE,NULL,0) ))
		{ sperror ("reset",drive->err); return 1; }
	return 0;
}

int plextor_get_TLA(drive_info* drive) {
	if (strncmp(drive->ven,"PLEXTOR ",8) || !strncmp(drive->dev,"CD-R   PREMIUM2",15)) {
		strcpy(drive->TLA,"N/A\0");
		return 1;
	}
	drive->cmd[0] = PLEXTOR_EEPROM_READ;
	drive->cmd[8] = 0x01;
	drive->cmd[9] = 0x00;
	drive->cmd[10] = 0x00;
	drive->cmd[11] = 0x00;
	// The Plextor PX-716 does not understand this command....
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,0x100)))
	{
		//printf("Possible PX-716...\n");
		drive->cmd[0] = 0xF1;
		drive->cmd[1] = 0x01;
		drive->cmd[8] = 0x01;
		drive->cmd[9] = 0x00;
		drive->cmd[10] = 0x00;
		drive->cmd[11] = 0x00;
		if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,0x100) ))
		{
			sperror ("Plextor get TLA",drive->err);
			strcpy(drive->TLA,"N/A\0");
			return 1;
		}
	}
	memcpy(drive->TLA,drive->rd_buf+0x29,4);
	drive->TLA[4] = 0;
	return 0;
}

int plextor_read_eeprom_CDR(drive_info* drive) {
//	unsigned int	i,j;
	drive->cmd[0] = 0xF1;
	drive->cmd[1] = 0x00;

	drive->cmd[7] = 0x00;
	drive->cmd[8] = 0x01;
	drive->cmd[9] = 0x00;
	drive->cmd[10] = 0x00;
	drive->cmd[11] = 0x00;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,256) ))
		{ sperror ("Plextor read CDR EEPROM",drive->err); return drive->err; }
	return 0;
}

int plextor_read_eeprom_PX712(drive_info* drive) {
//	unsigned int	i,j;
	drive->cmd[0] = 0xF1;
	drive->cmd[1] = 0x00;

	drive->cmd[7] = 0x00;
	drive->cmd[8] = 0x02;
	drive->cmd[9] = 0x00;
	drive->cmd[10] = 0x00;
	drive->cmd[11] = 0x00;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,512) ))
		{ sperror ("Plextor read PX712 EEPROM",drive->err); return drive->err; }
	return 0;
}

int plextor_read_eeprom_block(drive_info* drive, unsigned char idx, unsigned int sz) {
//	unsigned int	i,j;
	int		offs=idx*sz;
//	unsigned char*  buf=drive->rd_buf+offs;
	drive->cmd[0] = 0xF1;
	drive->cmd[1] = 0x01;
	drive->cmd[7] = idx;
	drive->cmd[8] = (sz >> 8) & 0xFF;
	drive->cmd[9] = sz & 0xFF;
	drive->cmd[10] = 0x00;
	drive->cmd[11] = 0x00;
//	if ((drive->err=drive->cmd.transport(READ,buf,sz) ))
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf + offs,sz) ))
		{ sperror ("Plextor read EEPROM",drive->err); return drive->err; }
/*
	printf("EEPROM block #%d:\n",idx);
	for(i=0;i<(sz/0x10);i++) {
	    printf("| %X0 | ", i);
	    for(j=0;j<0x10;j++) printf("%02X ",buf[i*0x10+j]);
	    printf("|");
	    for(j=0;j<0x10;j++) {
		if (buf[i*0x10+j] > 0x20) printf("%c",buf[i*0x10+j]);
		else printf(" ");
	    }
	    printf("|\n");
	};
*/
	return 0;
}

int plextor_read_eeprom(drive_info* drive, int* len) {
	int l=0;
	if (!isPlextor(drive)) {
		if (len) *len=0;
		return 1;
	}
	switch(drive->dev_ID) {
		case PLEXTOR_OLD:
		case PLEXTOR_4824:
		case PLEXTOR_5224:
		case PLEXTOR_PREMIUM:
		case PLEXTOR_PREMIUM2:
			plextor_read_eeprom_CDR(drive);
			if ( drive->err ) {
				if (len) *len=0;
				return 1;
			}
			l = 256;
			break;
		case PLEXTOR_708:
		case PLEXTOR_708A2:
		case PLEXTOR_712:
			plextor_read_eeprom_PX712(drive);
			if ( drive->err ) {
				if (len) *len=0;
				return 1;
			}
			l = 512;
			break;
		case PLEXTOR_714:
		case PLEXTOR_716:
		case PLEXTOR_716AL:
		case PLEXTOR_755:
		case PLEXTOR_760:
			for (unsigned char idx=0; idx<4; idx++) {
				plextor_read_eeprom_block(drive, idx, 256);
				if ( drive->err ) {
					if (len) *len=l;
					return 1;
				}
				l+=256;
			}
			break;
		default:
			printf("Don't know how to read eeprom from this device: %04X:%04X\n", drive->ven_ID, drive->dev_ID);
			if (len) *len=0;
			break;
	}
	if (len) *len=l;
	return 0;
}

int plextor_get_life(drive_info* drive)
{
	uint32_t ucr=0;
	uint32_t ucw=0;
	uint32_t udr=0;
	uint32_t udw=0;
	drive->life.ok=0;
//	printf("plextor_get_life()\n");
	if (drive->ven_ID != DEV_PLEXTOR) return 1;
	if (plextor_read_eeprom(drive)) {
		printf("Error reading eeprom!\n");
		return 1;
	}
	switch (drive->dev_ID) {
		case PLEXTOR_OLD:
		case PLEXTOR_4824:
		case PLEXTOR_5224:
		case PLEXTOR_PREMIUM:
		case PLEXTOR_PREMIUM2:
			drive->life.dn = ntoh16(drive->rd_buf+0x0078);
			ucr = ntoh32(drive->rd_buf+0x006C);
			ucw = ntoh32(drive->rd_buf+0x007A);
			break;
		default:
			drive->life.dn = ntoh16(drive->rd_buf+0x0120);
			ucr = ntoh32(drive->rd_buf+0x0122);
			ucw = ntoh32(drive->rd_buf+0x0126);

			udr = ntoh32(drive->rd_buf+0x012A);
			udw = ntoh32(drive->rd_buf+0x012E);
			break;
	}
	drive->life.ok=1;
	int2hms(ucr, &drive->life.cr);
	int2hms(ucw, &drive->life.cw);
	int2hms(udr, &drive->life.dr);
	int2hms(udw, &drive->life.dw);
	return 0;
}

int plextor_get_speeds(drive_info* drive) {
	int sel, max, last;
	drive->cmd[0]=PLEXTOR_PREC_SPD;
	drive->cmd[9]=0x0A;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,0x0A) ))
		{ if (!drive->silent) sperror ("GET_SPEEDS",drive->err);return drive->err;}
//	sel = (drive->rd_buf[5]<<8) | drive->rd_buf[4];
//	max = (drive->rd_buf[7]<<8) | drive->rd_buf[6];
//	last = (drive->rd_buf[9]<<8) | drive->rd_buf[8];
	sel = ntoh16(drive->rd_buf+4);
	max = ntoh16(drive->rd_buf+6);
	last = ntoh16(drive->rd_buf+8);
	if (!drive->silent) {
		printf("Selected write speed : %5d kB/s (%d X)\n", sel,  sel/177);
		printf("Max for this media   : %5d kB/s (%d X)\n", max,  max/177);
		printf("Last actual speed    : %5d kB/s (%d X)\n", last, last/177);
	}
	return 0;
}

int plextor_get_powerec(drive_info* drive) {
	drive->cmd[0]=PLEXTOR_MODE2;
	drive->cmd[1]=PLEX_GET_MODE;
	drive->cmd[2]=0x00;
	drive->cmd[9]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("GET_POWEREC",drive->err); return drive->err; }
	drive->plextor.powerec_state = drive->rd_buf[2];
//	drive->plextor.powerec_spd = ((drive->rd_buf[4] & 0xFF)<<8) | (drive->rd_buf[5] & 0xFF);
	drive->plextor.powerec_spd = ntoh16(drive->rd_buf+4);
	if (!drive->silent) printf("\tPoweRec %s, Recomended speed: %d kB/s\n",
		drive->plextor.powerec_state ? "ON" : "OFF",drive->plextor.powerec_spd);
//	printf("\t"); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	return 0;
}

int plextor_set_powerec(drive_info* drive) {
	printf("\tTurning PoweRec %s\n",drive->plextor.powerec_state ? "ON" : "OFF");
	drive->cmd[0]=PLEXTOR_MODE2;
	drive->cmd[1]=PLEX_SET_MODE | (drive->plextor.powerec_state?1:0);
	drive->cmd[2]=0x00;
	drive->cmd[9]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("SET_POWEREC",drive->err);return drive->err;}
	drive->plextor.powerec_state = drive->rd_buf[2] & 0xFF;
//	drive->plextor.powerec_spd = ((drive->rd_buf[4] & 0xFF)<<8) | (drive->rd_buf[5] & 0xFF);
	drive->plextor.powerec_spd = ntoh16(drive->rd_buf+4);
//	printf("\t"); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
//	plextor_get_speeds(drive);
	return 0;
}

void plextor_print_silentmode_state(drive_info* drive) {
	int val, idx;
	val=drive->plextor_silent.rd; idx=0;
	printf("Active SilentMode settings:\n");
	printf("\tSM Read speed  : ");
	if ( drive->media.type & DISC_DVD ) {
		while ((silent_dvd_rd_tbl[idx].val!=0xFF) & (silent_dvd_rd_tbl[idx].val!=(val & 0xFF))) idx++;
		printf("%s\n",silent_dvd_rd_tbl[idx].name);
//		printf("%s : %d\n",silent_dvd_rd_tbl[idx].name, val);
	} else {
		while ((silent_cd_rd_tbl[idx].val!=0xFF) & (silent_cd_rd_tbl[idx].val!=(val & 0xFF))) idx++;
		printf("%s\n",silent_cd_rd_tbl[idx].name);
//		printf("%s : %d\n",silent_cd_rd_tbl[idx].name, val);
	}
	val=drive->plextor_silent.wr; idx=0;
	printf("\tSM Write speed : ");
	if ( drive->media.type & DISC_DVD ) {
//		while ((silent_dvd_wr_tbl[idx].val!=0xFF) & (silent_dvd_wr_tbl[idx].val!=(val & 0xFF))) idx++;
//		printf("%s : %d\n",silent_dvd_wr_tbl[idx].name, val);
		printf("MAX\n");
	} else {
		while ((silent_cd_wr_tbl[idx].val!=0xFF) & (silent_cd_wr_tbl[idx].val!=(val & 0xFF))) idx++;
		printf("%s\n",silent_cd_wr_tbl[idx].name);
//		printf("%s : %d\n",silent_cd_wr_tbl[idx].name, val);
	}
	printf("\tSM Access time : %s\n",drive->plextor_silent.access?"SLOW":"FAST");
	printf("\tSM Load speed  : %d\n",drive->plextor_silent.load);
	printf("\tSM Eject speed : %d\n",drive->plextor_silent.eject);

	if (!drive->plextor_silent.psaved) {
		printf("Saved SilentMode settings not found\n");
		return;
	}

	printf("Saved SilentMode settings:\n");

	printf("\tPSM Silent State   : %s\n",drive->plextor_silent.pstate?"ON":"OFF");

	printf("\tPSM CD Read speed  : %dX\n",drive->plextor_silent.prd_cd);
	printf("\tPSM CD Write speed : %dX\n",drive->plextor_silent.pwr_cd);
	printf("\tPSM DVD Read speed : %dX\n",drive->plextor_silent.prd_dvd);
//	printf("\tPSM DVD Write speed: %d\n",drive->plextor_silent.pwr_dvd);

	printf("\tPSM Access time : %s\n",drive->plextor_silent.paccess?"SLOW":"FAST");
	printf("\tPSM Load speed  : %d\n",drive->plextor_silent.pload);
	printf("\tPSM Eject speed : %d\n",drive->plextor_silent.peject);
}

void plextor_get_silentmode_saved(drive_info* drive) {
	int len;
	if (plextor_read_eeprom(drive, &len) || len<0x110) return;

	drive->plextor_silent.psaved  = 1;
	drive->plextor_silent.pstate  = (drive->rd_buf[0x100] == 1);
	drive->plextor_silent.paccess = (drive->rd_buf[0x101] == 2);

	drive->plextor_silent.prd_cd  = drive->rd_buf[0x102];
	drive->plextor_silent.prd_dvd = drive->rd_buf[0x103];
	drive->plextor_silent.pwr_cd  = drive->rd_buf[0x104];
//	drive->plextor_silent.pwr_dvd = 16;

//								= drive->rd_buf[0x106]; // ???

/*
	drive->plextor_silent.pload   = drive->rd_buf[0x108] - 47;
	drive->plextor_silent.peject  = 48 - drive->rd_buf[0x107];
*/
	drive->plextor_silent.pload   = drive->rd_buf[0x108] - 47;
	drive->plextor_silent.peject  = -(drive->rd_buf[0x107]+48);
}

int plextor_get_silentmode(drive_info* drive) {
	drive->plextor_silent.psaved = 0;
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_GET_MODE;
	drive->cmd[2]=PLEX_MODE_SILENT;
	drive->cmd[3]=0x04;
	drive->cmd[10]=0x08;
	if (( drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("GET_SILENT_MODE",drive->err); return drive->err; }
	if (( drive->err=drive->cmd.transport(READ,(void*)&(drive->plextor_silent),8) ))
		{ if (!drive->silent) sperror ("GET_SILENT_MODE",drive->err); return drive->err; }
//	printf("\t"); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	plextor_get_silentmode_saved(drive);
	return 0;
}

int plextor_set_silentmode_tray(drive_info* drive, int disc_type, int permanent) {
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_GET_MODE;
	drive->cmd[2]=PLEX_MODE_SILENT_TRAY;
	drive->cmd[3]=disc_type | 2*!!permanent;
	drive->cmd[4]=drive->plextor_silent.eject;
	drive->cmd[6]=drive->plextor_silent.load;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0)))
		{ if (!drive->silent) sperror ("SET_SILENT_MODE_DISC",drive->err);return drive->err;}
//	printf("\t"); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	return 0;
}

int plextor_set_silentmode_disc(drive_info* drive, int disc_type, int permanent) {
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_GET_MODE;
	drive->cmd[2]=PLEX_MODE_SILENT_DISC;
	drive->cmd[3]=disc_type | 2*!!permanent;
	drive->cmd[4]=drive->plextor_silent.rd;
	if (disc_type == SILENT_CD) {
		drive->cmd[5]=drive->plextor_silent.wr;
	} else {
		drive->cmd[5]=0xFF;
	}
	drive->cmd[6]=drive->plextor_silent.access;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0)))
		{ if (!drive->silent) sperror ("SET_SILENT_MODE_DISC",drive->err);return drive->err;}
//	printf("\t"); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	return 0;
}

int plextor_set_silentmode_disable(drive_info* drive, int permanent) {
	drive->plextor_silent.rd=0x07;
	drive->plextor_silent.wr=0x07;
	drive->plextor_silent.access=0;
	plextor_set_silentmode_disc(drive, 0, permanent);
	drive->plextor_silent.eject=0x50;
	drive->plextor_silent.load=0x50;
	plextor_set_silentmode_tray(drive, 0, permanent);
	return 0;
}

void print_gigarec_value(drive_info* drive) {
//	plextor_get_gigarec(drive);
	int g,i;
	printf("GigaRec state       : ");
	i=drive->plextor.gigarec; g=0;
	while ((gigarec_tbl[g].val!=0xFF) & (gigarec_tbl[g].val!=(i & 0xFF))) g++;
	printf("%s\nDisc GigaRec rate   : ",gigarec_tbl[g].name);	
	i=drive->plextor.gigarec_disc; g=0;
	while ((gigarec_tbl[g].val!=0xFF) & (gigarec_tbl[g].val!=(i & 0xFF))) g++;
	printf("%s\n",gigarec_tbl[g].name);
}

int plextor_set_gigarec(drive_info* drive) {
// 	printf("  applying gigarec setting... ");
// 	print_gigarec_value(i);
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_SET_MODE;
	drive->cmd[2]=PLEX_MODE_GIGAREC;
	drive->cmd[3]=(drive->plextor.gigarec?1:0);
	drive->cmd[4]=drive->plextor.gigarec;
	drive->cmd[10]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("SET_GIGAREC",drive->err); return drive->err;}
	drive->plextor.gigarec = drive->rd_buf[3];
	drive->plextor.gigarec_disc = drive->rd_buf[4];
//	print_gigarec_value(drive);
//	printf("\t"); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	return 0;
}

int plextor_get_gigarec(drive_info* drive) {
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_GET_MODE;
	drive->cmd[2]=PLEX_MODE_GIGAREC;
	drive->cmd[10]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("GET_GIGAREC",drive->err); return drive->err;}
	drive->plextor.gigarec = drive->rd_buf[3];
	drive->plextor.gigarec_disc = drive->rd_buf[4];
//	printf("\t"); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	return 0;
}

void print_varirec(drive_info* drive, int disc_type) {
	int v,i,s;
	if (disc_type == VARIREC_DVD) {
		i=drive->plextor.varirec_pwr_dvd;
		s=drive->plextor.varirec_str_dvd;
	} else {
		i=drive->plextor.varirec_pwr_cd;
		s=drive->plextor.varirec_str_cd;
	}
	v=0;
	while ((varirec_pwr_tbl[v].val!=0xFF) & (varirec_pwr_tbl[v].val!=(i & 0xFF))) v++;
	printf("\tVariRec %s power    : %s\n",
									(disc_type == VARIREC_DVD)? "DVD":"CD ",
									varirec_pwr_tbl[v].name);
	printf("\tVariRec %s strategy : %s [%d]\n",
									(disc_type == VARIREC_DVD)? "DVD":"CD ",
									(disc_type == VARIREC_DVD)? varirec_str_dvd_tbl[s] : varirec_str_cd_tbl[s], s);
}

int plextor_set_varirec(drive_info* drive, int disc_type) {
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_SET_MODE;
	drive->cmd[2]=PLEX_MODE_VARIREC;
	if (disc_type == VARIREC_DVD) {
		drive->cmd[3]=disc_type + 2*!!drive->plextor.varirec_state_dvd;
		drive->cmd[4]=drive->plextor.varirec_pwr_dvd;
		drive->cmd[5]=drive->plextor.varirec_str_dvd;
	}else{
		drive->cmd[3]=disc_type + 2*!!drive->plextor.varirec_state_cd;
		drive->cmd[4]=drive->plextor.varirec_pwr_cd;
		drive->cmd[5]=drive->plextor.varirec_str_cd;
	}
	drive->cmd[10]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("SET_VARIREC",drive->err);return drive->err;}
	if (disc_type == VARIREC_DVD) {
		drive->plextor.varirec_state_dvd = drive->rd_buf[2];
		drive->plextor.varirec_pwr_dvd = drive->rd_buf[3];
		drive->plextor.varirec_str_dvd = drive->rd_buf[5];
	}else{
		drive->plextor.varirec_state_cd = drive->rd_buf[2];
		drive->plextor.varirec_pwr_cd = drive->rd_buf[3];
		drive->plextor.varirec_str_cd = drive->rd_buf[5];
	}
//	printf("\t"); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	return 0;
}

int plextor_get_varirec(drive_info* drive, int disc_type) {
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_GET_MODE;
	drive->cmd[2]=PLEX_MODE_VARIREC;
	drive->cmd[3]=0x02 | disc_type;
	drive->cmd[10]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("GET_VARIREC",drive->err); return drive->err;}
	if (disc_type == VARIREC_DVD) {
		drive->plextor.varirec_state_dvd = drive->rd_buf[2];
		drive->plextor.varirec_pwr_dvd = drive->rd_buf[3];
		drive->plextor.varirec_str_dvd = drive->rd_buf[5];
	}else{
		drive->plextor.varirec_state_cd = drive->rd_buf[2];
		drive->plextor.varirec_pwr_cd = drive->rd_buf[3];
		drive->plextor.varirec_str_cd = drive->rd_buf[5];
	}
//	printf("\t"); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	return 0;
}

void print_securec_state(drive_info* drive) {
	printf("SecuRec state       : ");
	printf("%s\n", drive->plextor.securec ? "ON" : "OFF" );
	printf("Disc is protected   : ");
	printf("%s\n", drive->plextor.securec ? "YES" : "NO" );
}

int plextor_get_securec_state(drive_info* drive)
{
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[2]=PLEX_MODE_SECUREC;
	drive->cmd[10]=0x08;
	if ((drive->err = drive->cmd.transport(READ, drive->rd_buf, 8)))
		{ if (!drive->silent) sperror("PLEXTOR_GET_SECUREC", drive->err); return drive->err;}
	drive->plextor.securec = drive->rd_buf[3];
	drive->plextor.securec_disc = drive->rd_buf[4];
//	drive->plextor.gigarec = (drive->rd_buf[3] & 0xFF);
//	drive->plextor.gigarec_disc = (drive->rd_buf[4] & 0xFF);

#ifdef DEBUG_SECUREC
	printf("get_securec() data: ");
	for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
#endif // DEBUG_SECUREC
	return 0;
}

int plextor_set_securec(drive_info* drive, char len, char* passwd)
{
	int i;
	drive->cmd[0]=PLEXTOR_SEND_AUTH;
	if ((passwd) && (len)) {
		printf("Turning SecuRec ON\n");
//		printf("Setting SecuRec passwd to '%s' (len = %d)\n", passwd, len);
		drive->cmd[2]=0x01;
		drive->cmd[3]=0x01;
		drive->cmd[4]=0x02;
		drive->cmd[10]=0x10;

		drive->rd_buf[0]=0;
		drive->rd_buf[1]=len;
		for(i=0;i<0x0E;i++)
			if (i<len) drive->rd_buf[i+2]=passwd[i];
			else drive->rd_buf[i+2]=0;
		if ((drive->err = drive->cmd.transport(WRITE, drive->rd_buf, 0x10)))
			{if (!drive->silent) sperror("PLEXTOR_SET_SECUREC", drive->err); return drive->err;}
	}else{
		printf("Turning SecuRec OFF\n");
		if ((drive->err = drive->cmd.transport(NONE, NULL, 0)))
			{if (!drive->silent) sperror("PLEXTOR_SET_SECUREC", drive->err); return drive->err;}
	}
//	if ((drive->dev_ID == PLEXTOR_760)) plextor_px755_do_auth(drive);
	return 0;
}

void print_speedread_state(drive_info* drive){
	printf("\tSpeedRead: %s\n",drive->plextor.spdread ? "on" : "off" );
}

int plextor_set_speedread(drive_info* drive, int state) {
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_SET_MODE;
	drive->cmd[2]=PLEX_MODE_SPDREAD;
	drive->cmd[3]=!!state;
	drive->cmd[10]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("SET_SPDREAD",drive->err); return drive->err; }
//	printf("** SPDREAD: "); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	drive->plextor.spdread=drive->rd_buf[2];
	return 0;
}

int plextor_get_speedread(drive_info* drive) {
//	return 1;
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_GET_MODE;
	drive->cmd[2]=PLEX_MODE_SPDREAD;
	drive->cmd[3]=0;
	drive->cmd[10]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("GET_SPDREAD",drive->err); return drive->err; }
	drive->plextor.spdread=drive->rd_buf[2];
	return 0;
}

void print_hcdr_state(drive_info* drive) {
	printf("\tHide CD-R: %s\n",drive->plextor.hcdr ? "on" : "off");
}

void print_sss_state(drive_info* drive) {
	printf("\tSingleSession: %s\n",drive->plextor.sss ? "on" : "off");
}

int plextor_get_hidecdr_singlesession(drive_info* drive) {
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_GET_MODE;
	drive->cmd[2]=PLEX_MODE_SS_HIDE;
	drive->cmd[9]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8)))
		{ if (!drive->silent) sperror ("GET_HCDR_SSS",drive->err); return drive->err; }
	drive->plextor.hcdr = !!(drive->rd_buf[2] & 0x02);
	drive->plextor.sss = (drive->rd_buf[2]) & 0x01;
	return 0;
}

int plextor_set_hidecdr_singlesession(drive_info* drive, int hidecdr_state, int singlesession_state) {
	if (plextor_get_hidecdr_singlesession(drive)) return 1;
//	printf("Trying to change SS/HIDE state to %d...\n",2*!!hidecdr_state + !!singlesession_state);
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_SET_MODE;
	drive->cmd[2]=PLEX_MODE_SS_HIDE;
	drive->cmd[3]=2*!!hidecdr_state + !!singlesession_state;
	drive->cmd[9]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("SET_HCDR_SSS",drive->err); return drive->err;}
//	printf("** HCDR_SSS: "); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	drive->plextor.hcdr = !!(drive->rd_buf[2] & 0x02);
	drive->plextor.sss = (drive->rd_buf[2]) & 0x01;
	return 0;
}

int plextor_set_hidecdr(drive_info* drive, int state) {
	if (plextor_get_hidecdr_singlesession(drive)) return 1;
	drive->plextor.hcdr = !!state;
	return plextor_set_hidecdr_singlesession(drive, drive->plextor.hcdr, drive->plextor.sss);
}

int plextor_set_singlesession(drive_info* drive, int state) {
	if (plextor_get_hidecdr_singlesession(drive)) return 1;	
	drive->plextor.sss = !!state;
	return plextor_set_hidecdr_singlesession(drive, drive->plextor.hcdr, drive->plextor.sss);
}

int plextor_get_bitset(drive_info* drive, int disc_type)
{
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_GET_MODE;
	drive->cmd[2]=PLEX_MODE_BITSET;
	drive->cmd[3]=disc_type;
	drive->cmd[9]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("PLEXTOR_GET_BITSET",drive->err); return drive->err; }
	switch (disc_type)
	{
		case PLEX_BITSET_R: drive->book_plus_r = ((drive->rd_buf[2] & 0x02) == 0x02); break;
		case PLEX_BITSET_RDL: drive->book_plus_rdl = ((drive->rd_buf[2] & 0x01) == 0x01); break;
	}
	return 0;
}

int plextor_set_bitset(drive_info* drive, int disc_type)
{
	char book;
	switch (disc_type)
	{
		case PLEX_BITSET_R:	book = (drive->book_plus_r); break;
		case PLEX_BITSET_RDL:	book = (drive->book_plus_rdl); break;
		default:		printf("PLEXTOR_SET_BITSET: Invalid disc_type"); return 1;
	}
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_SET_MODE;
	drive->cmd[2]=PLEX_MODE_BITSET;
	drive->cmd[3]=disc_type;
	drive->cmd[5]=book;
	drive->cmd[9]=0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("PLEXTOR_SET_BITSET",drive->err); return drive->err;}
	return 0;
}

int plextor_get_testwrite_dvdplus(drive_info* drive)
{
	drive->cmd[0]=PLEXTOR_MODE;
	drive->cmd[1]=PLEX_GET_MODE;
	drive->cmd[2]=PLEX_MODE_TESTWRITE_DVDPLUS;
	drive->cmd[10]= 0x08;
	if ((drive->err=drive->cmd.transport(READ, drive->rd_buf, 8) ))
		{ if (!drive->silent) sperror ("PLEXTOR_GET_TESTWRITE_DVDPLUS",drive->err); return drive->err; }
	drive->plextor.testwrite_dvdplus = !!drive->rd_buf[2];
	return 0;
}

int plextor_set_testwrite_dvdplus(drive_info* drive)
{
	drive->cmd[0] = PLEXTOR_MODE;
	drive->cmd[1] = PLEX_SET_MODE;
	drive->cmd[2] = PLEX_MODE_TESTWRITE_DVDPLUS;
	drive->cmd[3] = drive->plextor.testwrite_dvdplus;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0) ))
		{ if (!drive->silent) sperror ("PLEXTOR_SET_TESTWRITE_DVDPLUS",drive->err); return drive->err;}
	return 0;
}

int plextor_plexeraser(drive_info* drive)
{
	long i;
	printf("Destucting disc [mode=%02X]... \n",drive->plextor.plexeraser);
//	return 0;
	drive->cmd[0] = PLEXTOR_PLEXERASER;
	drive->cmd[1] = 0x06;
	drive->cmd[2] = drive->plextor.plexeraser;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0) ))
		{ if (!drive->silent) sperror ("PLEXTOR_DO_PLEXERASER",drive->err); return drive->err;}
	while (test_unit_ready(drive)) {
		msleep(1000);
		i++;
	}
	return 0;
}

		//-----------------//
		//  AUTOSTRATEGY   //
		//-----------------//

int plextor_print_autostrategy_state(drive_info* drive)
{
	printf("AutoStrategy mode : ");
	switch (drive->astrategy.state)
	{
		case AS_OFF: printf("OFF"); break;
		case AS_AUTO: printf("AUTO"); break;
		case AS_FORCED: printf("FORCED"); break;
		case AS_ON: printf("ON"); break;
		default: printf("???");
	}
	printf(" [%d]\n",drive->astrategy.state);
	return 0;
}

int plextor_get_autostrategy(drive_info* drive)
{
	drive->cmd[0] = PLEXTOR_AS_RD;
	drive->cmd[10]= 0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("PLEXTOR_GET_AUTOSTRATEGY",drive->err); return drive->err;}
//	printf("** GET AS: "); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	drive->astrategy.state = (drive->rd_buf[2] & 0x0F);
	return 0;
}

int plextor_set_autostrategy(drive_info* drive)
{
	drive->cmd[0] = PLEXTOR_AS_RD;
	drive->cmd[2] = PLEX_SET_MODE + (drive->astrategy.state & 0x0F);
	drive->cmd[10]= 0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8) ))
		{ if (!drive->silent) sperror ("PLEXTOR_SET_AUTOSTRATEGY",drive->err); return drive->err;}
//	printf("** SET AS: "); for (int i=0; i<8; i++) printf("0x%02X ",drive->rd_buf[i]&0xFF); printf("\n");
	drive->astrategy.state = (drive->rd_buf[2] & 0x0F);
	plextor_print_autostrategy_state(drive);
	return 0;
}

int plextor_get_autostrategy_db_entry_count(drive_info* drive)
{
	drive->cmd[0] = PLEXTOR_AS_RD;
	drive->cmd[1] = 0x02;
	drive->cmd[10]= 0x08;
	if ((drive->err=drive->cmd.transport(READ,(void*)&(drive->astrategy),8) ))
		{ if (!drive->silent) sperror ("PLEXTOR_GET_ASDB_ENTRY_COUNT",drive->err); return drive->err;}
//	drive->astrategy.dbcnt = drive->rd_buf[6];
//	printf ("\t  AS DB entries: %d\n", drive->astrategy.dbcnt);
	return 0;
}

int plextor_get_autostrategy_db(drive_info* drive)//, void* database)
{
	int size = 8 + (int)drive->astrategy.dbcnt * 32;
	int i,j;
	drive->cmd[0] = PLEXTOR_AS_RD; // 0xE4
	drive->cmd[1] = 0x02;
	drive->cmd[9]= (size >> 8) & 0xFF ;
	drive->cmd[10]= size & 0xFF ;
	if ((drive->err=drive->cmd.transport(READ,(void*)&(drive->astrategy),size) ))
		{ if (!drive->silent) sperror ("PLEXTOR_GET_ASDB",drive->err); return drive->err;}
	printf("** AS DB entries: %d\n",drive->astrategy.dbcnt);
//*
	for (j=0; j<drive->astrategy.dbcnt; j++) {
		for (i=0; i<12;i++) if (drive->astrategy.entry[j].MID[i] < 0x20) drive->astrategy.entry[j].MID[i] = 0x20;
//		for (i=0; i<12;i++) if (drive->astrategy.entry[j].MID[i] < 0x20 && drive->astrategy.entry[j].MID[i]) drive->astrategy.entry[j].MID[i] = 0x20;
		drive->astrategy.entry[j].crap2 = 0;

		printf("S#%02d |%c| DVD%cR [%02X] | %3dX | %12s | %d\n",
				drive->astrategy.entry[j].number,
				drive->astrategy.entry[j].enabled ? '*':' ',
				(drive->astrategy.entry[j].type == 0xA1)? '+':'-', drive->astrategy.entry[j].type,
				drive->astrategy.entry[j].speed,
				drive->astrategy.entry[j].MID,
				(drive->astrategy.entry[j].counter[0] <<8) | drive->astrategy.entry[j].counter[1]);
	}
//*/
/*
	char ch;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,size) ))
		{ sperror ("PLEXTOR_GET_ASDB",drive->err); return drive->err;}
	printf("ASDB dump:\n");
	for (i=0; i<8; i++) printf("%02X ", drive->rd_buf[i] & 0xFF);
	printf("\n");
	for (j=0; j<drive->astrategy.dbcnt; j++) {
		for (i=0; i<32; i++) printf("%02X ", drive->rd_buf[j*32+i+8] & 0xFF);
		printf("\n");
		for (i=0; i<32; i++) {
			if (drive->rd_buf[j*32+i+8] > 0x1F) 
				ch = drive->rd_buf[j*32+i+8];
			else
				ch = 0x20;
			printf("%c", ch);
		}
		printf("\n");
	}
*/
	return 0;
}

// EXPERIMENTAL   STRATEGY SAVE

int plextor_get_strategy(drive_info* drive){
	int cnt,acnt;
	int i,s,offs;
	unsigned char *entry;
	unsigned char *entry_data;

	printf("RETR AS cnt...\n");
	drive->cmd[0] = PLEXTOR_AS_RD; // 0xE4
	drive->cmd[1] = 0x02;
	drive->cmd[2] = 0x03;
	drive->cmd[10]= 0x08;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,0x08) ))
		{ if (!drive->silent) sperror ("PLEXTOR_GET_STRATEGY",drive->err); return drive->err;}
	acnt= drive->rd_buf[0];
	cnt=  drive->rd_buf[6];
	drive->astrategy.dbcnt = cnt;
	for (i=0; i<8; i++) printf("%02X ", drive->rd_buf[i]);
	printf("\nStrategies count: %d\n", drive->astrategy.dbcnt);

	printf("RETR AS data...\n");
	drive->cmd[0] = PLEXTOR_AS_RD; // 0xE4
	drive->cmd[1] = 0x02;
	drive->cmd[2] = 0x03;
	drive->cmd[9] = cnt;
	drive->cmd[10]= 0x10;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,cnt*0x100 + 0x10) ))
		{ if (!drive->silent) sperror ("PLEXTOR_GET_STRATEGY_DATA",drive->err); return drive->err;}


#ifdef ASDB_SAVE_DEBUG
	printf("DB HDR0:\n");
	for (i=0; i<8; i++) printf("%02X ", drive->rd_buf[i]); printf("\n");
	for (s=0; s<cnt; s++) {
		offs = 8 + s*0x20;
		for (i=0; i<0x20; i++) printf("%02X ", drive->rd_buf[offs+i]);
		printf("\n");
	}
	offs = 8 + cnt*0x20;
	printf("DB HDR1:\n");
	for (i=0; i<8; i++) printf("%02X ", drive->rd_buf[offs+i]); printf("\n");
	for (s=0; s<cnt; s++) {
		printf("Strategy #%02d\n", s+1);
		for(int s1=0; s1<7; s1++) {
			offs = 0x10 + cnt*0x20 + s*0xE0 + s1*0x20;
			for (i=0; i<0x20; i++) printf("%02X ", drive->rd_buf[offs+i]);
			printf("\n");
		}
	}
#endif

	for (s=0; s<cnt; s++) {
		entry      = (unsigned char*)&(drive->astrategy.entry[s]);
		entry_data = (unsigned char*)&(drive->astrategy.entry_data[s]);

		offs = 8 + s*0x20;
		for (i=0; i<0x20; i++) entry[i] = drive->rd_buf[offs+i];

		offs = 0x10 + cnt*0x20 + s*0xE0;
		for (i=0; i<(0x20*7); i++) entry_data[i] = drive->rd_buf[offs+i];
	}
	return 0;
}

// EXPERIMENTAL   STRATEGY LOAD

int plextor_add_strategy(drive_info* drive){
	int i, cnt = drive->astrategy.dbcnt;
	int offs;
	unsigned char* entry;

	plextor_clear_autostrategy_db(drive);
	printf("Adding strategy...\n");

// sending strategy headers
	for (i=0; i<(8+cnt*0x20); i++) drive->rd_buf[i]=0;

	drive->rd_buf[0]= (cnt >> 3) & 0xFF;
	drive->rd_buf[1]= (cnt*0x20 + 6) & 0xFF;
	drive->rd_buf[2]=0x02;
	drive->rd_buf[3]=0x80;
	drive->rd_buf[6]= cnt;
	drive->rd_buf[7]=0x20;

	entry = (unsigned char*) &drive->astrategy.entry;
	for (i=0; i<cnt*0x20; i++) drive->rd_buf[8+i] = entry[i];
	for (i=0; i<cnt; i++) drive->rd_buf[8+i*0x20] = (unsigned char)((i+1) & 0xFF);

#ifdef ASDB_LOAD_DEBUG
	printf("DB HDR0:\n");
	for (i=0; i<8; i++) printf("%02X ", drive->rd_buf[i]); printf("\n");
	for (int s=0; s<cnt; s++) {
		offs = 0x8 + s*0x20;
		for (i=0; i<0x20; i++) printf("%02X ", drive->rd_buf[offs+i]);
		printf("\n");
	}
#endif

	drive->cmd[0] = PLEXTOR_AS_WR; // 0xE5
	drive->cmd[1] = 0x02;
	drive->cmd[9] = (cnt >> 3) & 0xFF;
	drive->cmd[10]= (cnt*0x20 + 0x08) & 0xFF;
	if ((drive->err=drive->cmd.transport(WRITE,drive->rd_buf,cnt*0x20+8) ))
		{ if (!drive->silent) sperror ("PLEXTOR_ADD_STRATEGY_HDR",drive->err); return drive->err;}

// sending strategy parameters
	for (i=0; i<(8+cnt*0xE0); i++) drive->rd_buf[i]=0;

	drive->rd_buf[0]= ((cnt*0xE) >> 4) & 0xFF;
	drive->rd_buf[1]= (cnt*0xE0 + 6) & 0xFF;
	drive->rd_buf[2]=0x02;
	drive->rd_buf[3]=0x81;
	drive->rd_buf[6]= cnt * 7;
	drive->rd_buf[7]=0x20;

	entry = (unsigned char*) &drive->astrategy.entry_data;
	for (i=0; i<cnt*0xE0; i++) drive->rd_buf[8+i] = entry[i];
	for (i=0; i<cnt; i++)
		for (int s1=0; s1<7; s1++) {
			drive->rd_buf[i*0xE0+s1*0x20+8] = ((i*7+s1)>>8) & 0xFF;
			drive->rd_buf[i*0xE0+s1*0x20+9] = (i*7+s1) & 0xFF;
		}

#ifdef ASDB_LOAD_DEBUG
	printf("DB HDR1:\n");


	for (i=0; i<8; i++) printf("%02X ", drive->rd_buf[i]); printf("\n");
	for (int s=0; s<cnt; s++) {
		printf("Strategy #%02d\n", s+1);
		for(int s1=0; s1<7; s1++) {
			offs = 0x8 + s*0xE0 + s1*0x20;
			for (i=0; i<0x20; i++) printf("%02X ", drive->rd_buf[offs+i]);
			printf("\n");
		}
	}
#endif

	drive->cmd[0] = PLEXTOR_AS_WR; // 0xE5
	drive->cmd[1] = 0x02;
	drive->cmd[9] = ((cnt*0xE) >> 4) & 0xFF;
	drive->cmd[10]= (cnt*0xE0 + 0x08) & 0xFF;
	if ((drive->err=drive->cmd.transport(WRITE,drive->rd_buf,cnt*0xE0+8) ))
		{ if (!drive->silent) sperror ("PLEXTOR_ADD_STRATEGY_DATA",drive->err); return drive->err;}
	return 0;
}

int plextor_clear_autostrategy_db(drive_info* drive) {
	for (int i=0; i<8; i++) drive->rd_buf[i]=0;
	drive->rd_buf[1]=0x06;
	drive->rd_buf[2]=0x02;
	drive->rd_buf[3]=0xFF;
	drive->cmd[0] = PLEXTOR_AS_WR; // 0xE5
	drive->cmd[1] = 0x02;
	drive->cmd[10]= 0x08;
	if ((drive->err=drive->cmd.transport(WRITE,drive->rd_buf,0x08) ))
		{ if (!drive->silent) sperror ("PLEXTOR_CLEAR_ASTRATEGY_DB",drive->err); return drive->err;}
	return 0;
}

//
int plextor_modify_autostrategy_db(drive_info* drive, int index, int action)
{
	drive->rd_buf[0]=0x00;
	drive->rd_buf[1]=0x08;
	drive->rd_buf[2]=0x02;
	drive->rd_buf[3]=0x00;
	drive->rd_buf[4]=0x00;
	drive->rd_buf[5]=0x00;
	drive->rd_buf[6]=0x01;
	drive->rd_buf[7]=0x02;
	drive->rd_buf[8]=index;
	drive->rd_buf[9]=action;

	drive->cmd[0] = PLEXTOR_AS_WR; // 0xE5
	drive->cmd[1] = 0x02;
	drive->cmd[10]= 0x0A;
	if ((drive->err=drive->cmd.transport(WRITE,drive->rd_buf,0x0A) ))
		{ if (!drive->silent) sperror ("PLEXTOR_MODIFY_ASDB",drive->err); return drive->err;}
	return 0;
}

int plextor_create_strategy(drive_info* drive, int mode)
{
	int i=0;
	int p=0;
//	if (!drive->silent) printf("AS create: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",		(CMD_PLEX_AS_RD) & 0xFF, 4, mode & 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	drive->cmd[0] = PLEXTOR_AS_RD;
	drive->cmd[1] = 0x04;
	drive->cmd[2] = mode;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0) ))
		{ if (!drive->silent) sperror ("PLEXTOR_CREATE_STRATEGY_START",drive->err); return drive->err;}

	if (!drive->silent) printf("AS CRE START...\n");

	drive->cmd[0] = PLEXTOR_AS_RD;
	drive->cmd[1] = 0x01;
	drive->cmd[10]= 0x12;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,0x12) ))
		{ if (!drive->silent) sperror ("PLEXTOR_AS_GET_STATUS",drive->err); return drive->err;}

	if (!drive->silent) printf("      AS CRE: "); for (i=0; i<0x12; i++) printf("%02X ", drive->rd_buf[i] & 0x0FF); printf("\n");

// Waiting until Strategy is created
	while (test_unit_ready(drive)) {
		sleep(1);
		printf("%c\r", progress[p++]); if (p==4) p=0;
		i++;
	}
	printf("Strategy creation time: %d sec\n",i);

	drive->cmd[0] = PLEXTOR_AS_RD;
	drive->cmd[1] = 0x01;
	drive->cmd[10]= 0x12;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,0x12) ))
		{ if (!drive->silent) sperror ("PLEXTOR_AS_GET_STATUS",drive->err); return drive->err;}

//	if (!drive->silent) printf(" AS CRE DONE: "); for (i=0; i<0x12; i++) printf("%02X ", drive->rd_buf[i]) & 0x0FF; printf("\n");

	return 0;
}

int plextor_media_check(drive_info* drive, int mode)
{
	int i=0;
	int p=0;
//	if (!drive->silent) printf("MQCK CDB: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
//		(PLEXTOR_AS_RD) & 0xFF, 1, mode & 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	if ( !(drive->media.type & (DISC_DVD)) ) {
		printf("Media Quality Check supported on DVD media only!\n");
		return -1;
	}

	drive->cmd[0] = PLEXTOR_AS_RD;
	drive->cmd[1] = 0x01;
	drive->cmd[2] = mode;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0)))
		{ if (!drive->silent) sperror ("PLEXTOR_MEDIA_QUALITY_CHECK_START",drive->err); return drive->err;}

	printf("Starting MQCK...\n");

// Waiting until Media is checked
	while (test_unit_ready(drive)) {
		sleep(1);
		printf("%c\r", progress[p++]); if (p==4) p=0;
		i++;
	}
	printf("\nMedia Check time: %d sec", i);

	drive->cmd[0] = PLEXTOR_AS_RD;
	drive->cmd[1] = 0x01;
	drive->cmd[10]= 0x12;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,0x12) ))
		{ if (!drive->silent) sperror ("PLEXTOR_AS_GET_STATUS",drive->err); return drive->err; }

	if(!drive->silent)
		{ printf("Media Check result RAW: "); for (i=0; i<0x12; i++) printf("%02X ", drive->rd_buf[i]); printf("\n"); }
	return 0;
}

