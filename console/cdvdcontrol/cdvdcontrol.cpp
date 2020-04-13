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

#include <qpx_mmc.h>
#include <plextor_features.h>
#include <pioneer_spdctl.h>
#include <yamaha_features.h>

#include <version.h>


const	uint32_t	FL_HELP			= 0x00000001;
const	uint32_t	FL_SCAN			= 0x00000002;
const	uint32_t	FL_DEVICE		= 0x00000004;
const	uint32_t	FL_CURRENT		= 0x00000008;

const	uint32_t	FL_SUPPORTED	= 0x00000010;
const	uint32_t	FL_LOCK			= 0x00000020;
const	uint32_t	FL_POWEREC		= 0x00000040;
const	uint32_t	FL_GIGAREC		= 0x00000080;

const	uint32_t	FL_VARIREC_CD		= 0x00000100;
const	uint32_t	FL_VARIREC_CD_OFF	= 0x00000200;
const	uint32_t	FL_VARIREC_DVD		= 0x00000400;
const	uint32_t	FL_VARIREC_DVD_OFF	= 0x00000800;

const	uint32_t	FL_HCDR			= 0x00001000;
const	uint32_t	FL_SSS			= 0x00002000;
const	uint32_t	FL_SPDREAD		= 0x00004000;
const	uint32_t	FL_BOOK_R		= 0x00008000;

const	uint32_t	FL_BOOK_RDL		= 0x00010000;
const	uint32_t	FL_TESTWRITE	= 0x00020000;
const	uint32_t	FL_SILENT		= 0x00040000;
const	uint32_t	FL_AS			= 0x00080000;

const	uint32_t	FL_PXERASER		= 0x00100000;
const	uint32_t	FL_SECUREC		= 0x00200000;
const	uint32_t	FL_NOSECUREC	= 0x00400000;
const	uint32_t	FL_VERBOSE		= 0x00800000;

const	uint32_t	FL_YMH_AMQR		= 0x01000000;
const	uint32_t	FL_YMH_FORCESPEED	= 0x02000000;
const	uint32_t	FL_MQCK			= 0x04000000;

const	uint32_t	FL_PIOLIMIT		= 0x08000000;
const	uint32_t	FL_PIOQUIET		= 0x10000000;
const	uint32_t	FL_LOEJ			= 0x20000000;

const	uint32_t	FLAS_RETR		= 0x00000001;
const	uint32_t	FLAS_STOR		= 0x00000002;
const	uint32_t	FLAS_CREATE		= 0x00000004;
const	uint32_t	FLAS_DEL		= 0x00000008;

const	uint32_t	FLAS_VIEW		= 0x00000010;
const	uint32_t	FLAS_ACT		= 0x00000020;
const	uint32_t	FLAS_DEACT		= 0x00000040;
const	uint32_t	FLAS_CLEAR		= 0x00000080;

	uint32_t	flags = 0;
	uint32_t	flags_as = 0;

int get_device_info(drive_info* dev)
{
	dev->ven_features=0;
	dev->chk_features=0;
	detect_capabilities(dev);
//	detect_check_capabilities(dev);
	determine_disc_type(dev);
	if ( !isPlextor(dev) && !isYamaha(dev) && !isPioneer(dev)) {
		printf ("%s: drive not supported, only common controls will work!\n", dev->device);
//		return 1;
	}
	if ( isPlextor(dev) ) {
		plextor_get_life(dev);
		if (dev->life.ok) {
			printf("Discs loaded: %6d\n", dev->life.dn);
			printf("Drive operating time:\n");
			printf("  CD Rd  : %4d:%02d:%02d\n", dev->life.cr.h, dev->life.cr.m, dev->life.cr.s);
			printf("  CD Wr  : %4d:%02d:%02d\n", dev->life.cw.h, dev->life.cw.m, dev->life.cw.s);
			if (dev->rd_capabilities & DEVICE_DVD)
				printf("  DVD Rd : %4d:%02d:%02d\n", dev->life.dr.h, dev->life.dr.m, dev->life.dr.s);
			if (dev->wr_capabilities & DEVICE_DVD)
				printf("  DVD Wr : %4d:%02d:%02d\n", dev->life.dw.h, dev->life.dw.m, dev->life.dw.s);
		}

//		if ( isPlextorLockPresent(dev) )
		plextor_px755_do_auth(dev);
		if (!plextor_get_hidecdr_singlesession(dev)) dev->ven_features|=PX_HCDRSS;
		if (!plextor_get_speedread(dev)) dev->ven_features|=PX_SPDREAD;
		if (dev->wr_capabilities) {
//			if (!yamaha_check_amqr(dev)) dev->ven_features|=YMH_AMQR;
			if (!plextor_get_powerec(dev)) {
				dev->ven_features|=PX_POWEREC;
//				plextor_get_speeds(dev);
			}
			if (!plextor_get_gigarec(dev)) dev->ven_features|=PX_GIGAREC;
			if (!plextor_get_varirec(dev, VARIREC_CD)) dev->ven_features|=PX_VARIREC_CD;
//			if (!plextor_get_securec(dev)) dev->ven_features|=PX_SECUREC;
			if (!plextor_get_silentmode(dev)) dev->ven_features|=PX_SILENT;
			if (!plextor_get_securec_state(dev)) dev->ven_features|=PX_SECUREC;
		}
		if (dev->wr_capabilities & DEVICE_DVD) {
			if (!plextor_get_varirec(dev, VARIREC_DVD)) dev->ven_features|=PX_VARIREC_DVD;
			if (!plextor_get_bitset(dev, PLEX_BITSET_R)) dev->ven_features|=PX_BITSET_R;
			if (!plextor_get_bitset(dev, PLEX_BITSET_RDL)) dev->ven_features|=PX_BITSET_RDL;
			if (!plextor_get_autostrategy(dev)) dev->ven_features|=PX_ASTRATEGY;
			if (!plextor_get_testwrite_dvdplus(dev)) dev->ven_features|=PX_SIMUL_PLUS;
		}
#warning "PlexEraser DETECTION. Just assume PX755/760 and Premium-II"
		// if ((dev->dev_ID == PLEXTOR_755) || (dev->dev_ID == PLEXTOR_760) || (dev->dev_ID == PLEXTOR_PREMIUM2))
		if ( isPlextorLockPresent(dev) )
			dev->ven_features|=PX_ERASER;
		if (!yamaha_check_amqr(dev)) dev->ven_features|=YMH_AMQR;
	} else if ( isYamaha(dev) ) {
		if (!yamaha_check_amqr(dev)) dev->ven_features|=YMH_AMQR;
		if (!yamaha_check_forcespeed(dev)) dev->ven_features|=YMH_FORCESPEED;
		if (!yamaha_f1_get_tattoo(dev)) dev->ven_features|=YMH_TATTOO;
	} else if ( isPioneer(dev) ) {
		if (!pioneer_get_quiet(dev)) dev->ven_features|=PIO_QUIET;
	}

//	printf("Trying opcode E9 modes...\n");
//	for (int i=0; i<256; i++) {if (!plextor_get_mode(dev,i)) printf(" MODE 0x%02X\n",i);}


//	printf("Trying opcode ED modes...\n");
//	for (int i=0; i<256; i++) {if (!plextor_get_mode2(dev,i)) printf(" MODE 0x%02X\n",i);}


 	if (flags & FL_SUPPORTED) {
//		printf("____________________________\n");
		printf("\n** Supported features:\n");
		printf("AudioMaster Q.R.    : %s\n", dev->ven_features & YMH_AMQR ? "YES" : "-");
		printf("Yamaha ForceSpeed   : %s\n", dev->ven_features & YMH_FORCESPEED ? "YES" : "-");
		printf("Yamaha DiscT@2      : %s\n", dev->ven_features & YMH_TATTOO ? "YES" : "-");
		printf("Hide CD-R           : %s\n", dev->ven_features & PX_HCDRSS ? "YES" : "-");
		printf("SingleSession       : %s\n", dev->ven_features & PX_HCDRSS ? "YES" : "-");
		printf("SpeedRead           : %s\n", dev->ven_features & PX_SPDREAD ? "YES" : "-");
		printf("PoweRec             : %s\n", dev->ven_features & PX_POWEREC ? "YES" : "-");
		printf("GigaRec             : %s\n", dev->ven_features & PX_GIGAREC ? "YES" : "-");
		printf("VariRec CD          : %s\n", dev->ven_features & PX_VARIREC_CD ? "YES" : "-");
		printf("VariRec DVD         : %s\n", dev->ven_features & PX_VARIREC_DVD ? "YES" : "-");
		printf("SecuRec             : %s\n", dev->ven_features & PX_SECUREC ? "YES" : "-");
		printf("Silent mode         : %s\n", dev->ven_features & PX_SILENT ? "YES" : "-");
		printf("DVD+R bitsetting    : %s\n", dev->ven_features & PX_BITSET_R ? "YES" : "-");
		printf("DVD+R DL bitsetting : %s\n", dev->ven_features & PX_BITSET_RDL ? "YES" : "-");
		printf("DVD+R(W) testwrite  : %s\n", dev->ven_features & PX_SIMUL_PLUS ? "YES" : "-");
		printf("AutoStrategy        : %s%s\n",
			dev->ven_features & PX_ASTRATEGY ? "YES" : "-",
			(dev->ven_features & PX_ASTRATEGY) && (dev->dev_ID & (PLEXTOR_755 | PLEXTOR_760)) ? " (EXTENDED)" : "");
		printf("PlexEraser          : %s\n", dev->ven_features & PX_ERASER ? "YES" : "-");
		printf("Pioneer QuietMode   : %s\n", dev->ven_features & PIO_QUIET ? "YES" : "-");
	}


//	get_media_info(dev);
//	if (dev->rd_capabilities & DEVICE_DVD) {

	if (flags & FL_CURRENT) {
//		printf("____________________________\n");
		printf("\n** Current drive settings:\n");
	}
	if ((dev->capabilities & CAP_LOCK)      && (flags & FL_CURRENT))
		{ get_lock(dev);
		printf("Lock state          : %s\n", (dev->parms.status & STATUS_LOCK) ? "ON":"OFF"); }
//	if ((dev->ven_features & YMH_AMQR)       && ((flags & FL_CURRENT) || (flags & FL_YMH_AMQR)))
//		printf("AudioMaster Q.R.    : %s\n", dev->yamaha.amqr ? "ON":"OFF");
//	if ((dev->ven_features & YMH_FORCESPEED)       && ((flags & FL_CURRENT) || (flags & FL_YMH_FORCESPEED)))
//		printf("Yamaha ForceSpeed   : %s\n", dev->yamaha.forcespeed ? "ON":"OFF");
	if ((dev->ven_features & YMH_TATTOO) && (flags & FL_CURRENT)) {
		if (dev->yamaha.tattoo_rows) {
			printf("Yamaha DiscT@2      : inner %d   outer %d   image 3744 x %d\n",
				dev->yamaha.tattoo_i,
				dev->yamaha.tattoo_o,
				dev->yamaha.tattoo_rows);
		} else {
			if (dev->media.type & DISC_CD)
				printf("DiscT@2: Can't write T@2 on inserted disc! No space left?\n");
			else
				printf("DiscT@2: No disc found! Can't get T@2 info!\n");
		}
	}
	if ((dev->ven_features & PX_HCDRSS)      && ((flags & FL_CURRENT) || (flags & FL_HCDR)))
		printf("Hide-CDR state      : %s\n", dev->plextor.hcdr ? "ON":"OFF");
	if ((dev->ven_features & PX_HCDRSS)      && ((flags & FL_CURRENT) || (flags & FL_SSS)))
		printf("SingleSession state : %s\n", dev->plextor.sss ? "ON":"OFF");
	if ((dev->ven_features & PX_SPDREAD)     && ((flags & FL_CURRENT) || (flags & FL_SPDREAD)))
		printf("SpeedRead state     : %s\n", dev->plextor.spdread ? "ON":"OFF");
	if ((dev->ven_features & PX_POWEREC)     && ((flags & FL_CURRENT) || (flags & FL_POWEREC))) {
		printf("PoweRec state       : %s\n", dev->plextor.powerec_state ? "ON":"OFF");
		if (dev->media.type & DISC_CD)
			printf("      PoweRec Speed : %dX (CD)\n",dev->plextor.powerec_spd / 176);
		if (dev->media.type & DISC_DVD)
			printf("      PoweRec Speed : %dX (DVD)\n",dev->plextor.powerec_spd / 1385);
//		show_powerec_speed();
	}
	if ((dev->ven_features & PX_GIGAREC)     && ((flags & FL_CURRENT) || (flags & FL_GIGAREC)))
		print_gigarec_value(dev);
	if ((dev->ven_features & PX_VARIREC_CD)  && ((flags & FL_CURRENT) || (flags & FL_VARIREC_CD)))
		{ printf("VariRec CD state    : %s\n", dev->plextor.varirec_state_cd ? "ON":"OFF");
		if (dev->plextor.varirec_state_cd) print_varirec(dev, VARIREC_CD); }
	if ((dev->ven_features & PX_VARIREC_DVD) && ((flags & FL_CURRENT) || (flags & FL_VARIREC_DVD)))
		{ printf("VariRec DVD state   : %s\n", dev->plextor.varirec_state_dvd ? "ON":"OFF");
		if (dev->plextor.varirec_state_dvd) print_varirec(dev, VARIREC_DVD); }
	if ((dev->ven_features & PX_SECUREC)     && ((flags & FL_CURRENT) || (flags & FL_SECUREC)))
		printf("SecuRec state       : %s\n", dev->plextor.securec ? "ON":"OFF");
	if ((dev->ven_features & PX_SILENT)      && ((flags & FL_CURRENT) || (flags & FL_SILENT)))
		{ printf("Silent mode         : %s\n",dev->plextor_silent.state ? "ON":"OFF");
		if (dev->plextor_silent.state) plextor_print_silentmode_state(dev); }
	if ((dev->ven_features & PX_BITSET_R)    && ((flags & FL_CURRENT) || (flags & FL_BOOK_R)))
		printf("DVD+R bitsetting    : %s\n", dev->book_plus_r ? "ON":"OFF");
	if ((dev->ven_features & PX_BITSET_RDL)  && ((flags & FL_CURRENT) || (flags & FL_BOOK_RDL)))
		printf("DVD+R DL bitsetting : %s\n", dev->book_plus_rdl ? "ON":"OFF");
	if ((dev->ven_features & PX_SIMUL_PLUS)  && ((flags & FL_CURRENT) || (flags & FL_TESTWRITE)))
		printf("DVD+R(W) testwrite  : %s\n", dev->plextor.testwrite_dvdplus ? "ON":"OFF");
	if ((dev->ven_features & PX_ASTRATEGY)   && ((flags & FL_CURRENT) || (flags & FL_AS)))
		plextor_print_autostrategy_state(dev);
	if (dev->ven_features & PX_ERASER)
		printf("PlexEraser          : supported\n");

	if ((dev->ven_features & PIO_QUIET) && ((flags & FL_CURRENT) || (flags & FL_PIOQUIET)))
		printf("QuietMode setting   : %s\n", pioneer_silent_tbl[(int)dev->pioneer.silent]);
	if ((dev->ven_features & PIO_QUIET) && ((flags & FL_CURRENT) || (flags & FL_PIOLIMIT)))
		printf("Speed Limit         : %s\n", dev->pioneer.limit ? "ON" : "OFF" );
	return 0;
}

void usage(char* bin) {
	fprintf (stderr,"\nusage: %s [-d device] [options]\n",bin);
	printf("Common options:\n");
	printf("\t-l, --scanbus                list drives (scan IDE/SCSI bus)\n");
	printf("\t-h, --help                   show help\n");
	printf("\t-v, --verbose                be verbose\n");
	printf("\t-c, --current                show current drive settings\n");
	printf("\t-s, --supported              show supported features\n");
	printf("\t--[un]lock                   lock/unlock disc in drive\n");
	printf("\t--lockt                      toggle media lock state\n");
	printf("\t--eject                      eject media\n");
	printf("\t--load                       load media (only for tray devices)\n");
	printf("\t--loej                       load/eject media (toggle tray state)\n");
	printf("\t--loej-immed                 don't wait for media is loaded or ejected\n");

	printf("\noptions for Plextor devices:\n");
	printf("\t--spdread [on|off]           turn SpeedRead on/off (default: off)\n");
	printf("\t--sss [on|off]               turn SingleSession on/off (default: off)\n");
	printf("\t--hcdr [on|off]              turn Hide-CDR on/off (default: off)\n");
	printf("\t--powerec [on|off]           turn PoweREC on/off (default: on)\n");
//	printf("\t--amqr [on|off]              turn Yamaha AMQR on/off (default: off)\n");
//	printf("\t--forcespeed [on|off]        turn Yamaha ForceSpeed on/off (default: off)\n");
	printf("\t--gigarec <state>            set GigaREC rate or turn it off\n");
	printf("\t                             values: 0.6, 0.7, 0.8, 0.9, 1.1, 1.2, 1.3, 1.4, off\n");
	printf("\t--varirec-cd <pwr>           set VariREC power for CD or turn VariREC off\n");
	printf("\t                             values: +4, +3, +2, +1, 0, -1, -2, -3, -4, off\n");
	printf("\t--varirec-cd-strategy <str>  set VariREC strategy for CD\n");
	printf("\t                             values: default, azo, cya, pha, phb, phc, phd\n");
	printf("\t--varirec-dvd <pwr>          set VariREC power for DVD or turn VariREC off\n");
	printf("\t                             values: +4, +3, +2, +1, 0, -1, -2, -3, -4, off\n");
	printf("\t--varirec-dvd-strategy <str> set VariREC strategy for DVD\n");
	printf("\t                             values: default, 0, 1, 2, 3, 4, 5, 6, 7\n");
	printf("\t--securec <passwd>           set SecuREC password and turn it on\n");
	printf("\t                             passwd must be 4..10 characters length\n");
	printf("\t--nosecurec                  turn SecuRec off\n");
	printf("\t--bitset+r [on|off]          turn on/off setting DVD-ROM book on DVD+R media\n");
	printf("\t--bitset+rdl [on|off]        turn on/off setting DVD-ROM book on DVD+R DL media\n");
	printf("\t--mqck <quick|advanced>      run MediaQuality Check\n");
	printf("\t--mqck-speed #               set MQCK speed (default: maximum)\n");
	printf("\t--as-mode <mode>             AS: select AutoStrategy mode: forced,auto,on,off,\n");
	printf("\t--as-list                    AS: view strategies list\n");
	printf("\t--as-on #                    AS: activate strategy #\n");
	printf("\t--as-off #                   AS: deactivate strategy #\n");
	printf("\t--as-del #                   AS: delete strategy #\n");
	printf("\t--as-create <q|f> <a|r>      AS: create new strategy for inserted DVD media (PX-755/PX-760 only)\n");
	printf("\t                             mode:    q = quick, f = full\n");
	printf("\t                             action:  a = add,   r = replace\n");
	printf("\t--as-save <file>             AS: save Strategies            / EXPERIMENTAL /\n");
	printf("\t--as-load <file>             AS: load Strategy from file    / EXPERIMENTAL /\n");
	printf("\t--as-clear                   AS: remove ALL strategies from database\n");
	printf("\t--dvd+testwrite [on|off]     turn on/off testwrite on DVD+R(W) media\n");
	printf("\t--silent [on|off]            just turn Silent Mode on/off (default: on)\n");
	printf("\t--sm-nosave                  don't save Silent Mode settings\n");
	printf("\t--sm-cd-rd #                 set max CD READ speed (default: 32X)\n");
	printf("\t                             values: 4, 8, 24, 32, 40, 48\n");
	printf("\t--sm-cd-wr #                 set max CD WRITE speed (default: 32X)\n");
	printf("\t                             values: 4, 8, 16, 24, 32, 48\n");
	printf("\t--sm-dvd-rd #                set max DVD READ speed (default: 12X)\n");
	printf("\t                             values: 2, 5, 8, 12, 16\n");
//	printf("\t--sm-dvd-wr #                set max DVD WRITE speed (default: 8X)\n");
//	printf("\t                             values: 4, 6, 8, 12, 16\n");
	printf("\t--sm-load #                  set tray load speed. spd can be 0 to 80 (default: 63)\n");
	printf("\t--sm-eject #                 set tray eject speed. spd can be 0 to 80 (default: 0)\n");
	printf("\t--sm-access <slow|fast>      set access time, has effect only with CD/DVD speed setting\n");
	printf("\t--destruct <quick|full>      perform PlexEraser function\n");
	printf("\t                             WARNING! data on inserted CD-R/DVD-R will be lost!\n");
	printf("\t                             works on PX-755/PX-760/Premium2 only\n");

	printf("\noptions for Pioneer devices:\n");
	printf("\t--pio-limit [on|off]         limit (or not) read speed by 24x for CD and 8x for DVD\n");
	printf("\t--pio-quiet [quiet|perf|std] select QuietMode setting: Quiet, Performance or Standard (default: quiet)\n");
	printf("\t--pio-nosave                 don't save settings to drive (changes will be lost after reboot)\n");
}

int main (int argc, char* argv[])
{
	int	i;
	int	drvcnt=0;
	char*	device=NULL;
	drive_info* dev=NULL;
	char	aslfn[2048];
	char	assfn[2048];
//	char	asfn2[1032];
	FILE*	asf;
//	FILE*	asf2;

	int	powerec = 1;
	int	gigarec = GIGAREC_10;
	int	varirec_cd_pwr  = VARIREC_NULL;
	int	varirec_cd_str  = 0;
	int	varirec_dvd_pwr = VARIREC_NULL;
	int	varirec_dvd_str = 0;
	int	hcdr = 1;
	int	sss = 1;
	int	spdread = 1;
	int	bookr = 1;
	int	bookrdl = 1;
	int	testwrite = 1;
	int as_mqck = -1;
	int as_mqck_spd = -1;
	int	as     = AS_AUTO;
	int	ascre  = ASDB_CRE_QUICK;
	int	as_idx_act   = 0;
	int	as_idx_deact = 0;
	int	as_idx_del   = 0;
	int	silent = 1;
	int	silent_load  = 63;
	int	silent_eject = 0;
	int	silent_access = SILENT_ACCESS_FAST;
	int	silent_cd_rd = SILENT_CD_RD_32X;
	int	silent_cd_wr = SILENT_CD_WR_32X;
	int	silent_dvd_rd = SILENT_DVD_RD_12X;
//	int	silent_dvd_wr = SILENT_DVD_WR_8X;
	int	silent_cd = 0;
	int	silent_dvd = 0;
	int	silent_tray = 0;
	int	silent_save = 1;
	int	pxeraser = 0;
	int	amqr = 0;
	int	forcespeed = 0;
	char	passwd[256];

	char piosilent = PIO_SILENT_QUIET;
	bool piolimit = 1;
	int  eject    = 2;
	int	 lock     = 2;
	bool     loej_immed = 0;

	printf("**  CDVD Control v%s  (c) 2005-2009  Gennady \"ShultZ\" Kozlov\n", VERSION);

//	printf("Parsing commandline options (%d args)...\n",argc-1);
	for (i=1; i<argc; i++) {
//		printf("arg[%02d]: %s\n",i,argv[i]);
		if(!strcmp(argv[i],"-d")) {
				if(argc>(i+1)) {
					i++;
					flags |= FL_DEVICE;
					device = argv[i];
				} else {
					printf("Option %s needs parameter\n",argv[i]);
					exit (1);
				}
		}
		else if(!strcmp(argv[i],"-h")) flags |= FL_HELP;
		else if(!strcmp(argv[i],"--help")) flags |= FL_HELP;
		else if(!strcmp(argv[i],"-c")) flags |= FL_CURRENT;
		else if(!strcmp(argv[i],"--current")) flags |= FL_CURRENT;
		else if(!strcmp(argv[i],"-s")) flags |= FL_SUPPORTED;
		else if(!strcmp(argv[i],"--supported")) flags |= FL_SUPPORTED;
		else if(!strcmp(argv[i],"-l")) flags |= FL_SCAN;
		else if(!strcmp(argv[i],"--scanbus")) flags |= FL_SCAN;
//  ************   Lock
		else if(!strcmp(argv[i],"--unlock")) {
			if ( !(flags & FL_LOCK)) {
			    flags |= FL_LOCK; lock=0;
			} else {
			    printf("Conflicting/duplicated lock/unlock option!\n");
			}
		}
		else if(!strcmp(argv[i],"--lock")) {
			if ( !(flags & FL_LOCK)) {
			    flags |= FL_LOCK; lock=1;
			} else {
			    printf("Conflicting/duplicated lock/unlock option!\n");
			}
		}
		else if(!strcmp(argv[i],"--lockt")) {
			if ( !(flags & FL_LOCK)) {
			    flags |= FL_LOCK; lock=2;
			} else {
			    printf("Conflicting/duplicated lock/unlock option!\n");
			}
		}
		else if(!strcmp(argv[i],"--load")) {
			if ( !(flags & FL_LOEJ)) {
			    flags |= FL_LOEJ; eject=0;
			} else {
			    printf("Conflicting/duplicated load/eject option!\n");
			}
		}
		else if(!strcmp(argv[i],"--eject")) {
			if ( !(flags & FL_LOEJ)) {
			    flags |= FL_LOEJ; eject=1;
			} else {
			    printf("Conflicting/duplicated load/eject option!\n");
			}
		}
		else if(!strcmp(argv[i],"--loej")) {
			if ( !(flags & FL_LOEJ)) {
			    flags |= FL_LOEJ; eject=2;
			} else {
			    printf("Conflicting/duplicated load/eject option!\n");
			}
		}
		else if(!strcmp(argv[i],"--loej-immed")) {
		    loej_immed = true;
		}
		else if(!strcmp(argv[i],"--spdread")) {
			flags |= FL_SPDREAD;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) spdread = 0;
				else if (!strcmp(argv[i],"on")) spdread = 1;
				else {
					printf("Illegal argument for SpeedRead: %s\n", argv[i]);
				}
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--sss")) {
			flags |= FL_SSS;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) sss = 0;
				else if (!strcmp(argv[i],"on")) sss = 1;
				else {
					printf("Illegal argument for SingleSession: %s\n", argv[i]);
				}
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--hcdr")) {
			flags |= FL_HCDR;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) hcdr = 0;
				else if (!strcmp(argv[i],"on")) hcdr = 1;
				else {
					printf("Illegal argument for Hide-CDR: %s\n", argv[i]);
				}
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
//  ************   PoweREC
		else if(!strcmp(argv[i],"--powerec")) {
			flags |= FL_POWEREC;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) powerec = 0;
				else if (!strcmp(argv[i],"on")) powerec = 1;
				else {
					printf("Illegal argument for PoweRec: %s\n", argv[i]);
				}
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
//  ************   GigaREC
		else if(!strcmp(argv[i],"--gigarec")) {
			if (argc>(i+1)) {
				i++;
				flags |= FL_GIGAREC;
				if      (!strcmp(argv[i],"0.6")) gigarec = GIGAREC_06;
				else if (!strcmp(argv[i],"0.7")) gigarec = GIGAREC_07;
				else if (!strcmp(argv[i],"0.8")) gigarec = GIGAREC_08;
				else if (!strcmp(argv[i],"0.9")) gigarec = GIGAREC_09;
				else if (!strcmp(argv[i],"1.0")) gigarec = GIGAREC_10;
				else if (!strcmp(argv[i],"off")) gigarec = GIGAREC_10;
				else if (!strcmp(argv[i],"1.1")) gigarec = GIGAREC_11;
				else if (!strcmp(argv[i],"1.2")) gigarec = GIGAREC_12;
				else if (!strcmp(argv[i],"1.3")) gigarec = GIGAREC_13;
				else if (!strcmp(argv[i],"1.4")) gigarec = GIGAREC_14;
				else {
					printf("Illegal GigaREC value: %s\n", argv[i]);
				}
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
//  ************   VariREC  CD
		else if(!strcmp(argv[i],"--varirec-cd")) {
			if (argc>(i+1)) {
				i++;
				flags |= FL_VARIREC_CD;
				int val = 0;
				if (!strcmp(argv[i],"off")) flags |= FL_VARIREC_CD_OFF;
				else  {
					val = strtol(argv[i], NULL, 0);
					if (errno || val<-4 || val>4) {
						printf("Illegal VariREC CD power value: %s\n", argv[i]);
						return 5;
					} else switch(val) {
						case -4: varirec_cd_pwr = VARIREC_MINUS_4; break;
						case -3: varirec_cd_pwr = VARIREC_MINUS_3; break;
						case -2: varirec_cd_pwr = VARIREC_MINUS_2; break;
						case -1: varirec_cd_pwr = VARIREC_MINUS_1; break;
						case  0: varirec_cd_pwr = VARIREC_NULL; break;
						case  1: varirec_cd_pwr = VARIREC_PLUS_1; break;
						case  2: varirec_cd_pwr = VARIREC_PLUS_2; break;
						case  3: varirec_cd_pwr = VARIREC_PLUS_3; break;
						case  4: varirec_cd_pwr = VARIREC_PLUS_4; break;
					}
				}
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--varirec-cd-strategy")) {
			if (argc>(i+1)) {
				i++;
				int val = atol(argv[i]);
				if (!errno && val>=-1 && val<6) {
					varirec_cd_str = val+1;
				} else
				if (!strcmp(argv[i],"default"))  varirec_cd_str = 0;
				else if (!strcmp(argv[i],"azo")) varirec_cd_str = 1;
				else if (!strcmp(argv[i],"cya")) varirec_cd_str = 2;
				else if (!strcmp(argv[i],"pha")) varirec_cd_str = 3;
				else if (!strcmp(argv[i],"phb")) varirec_cd_str = 4;
				else if (!strcmp(argv[i],"phc")) varirec_cd_str = 5;
				else if (!strcmp(argv[i],"phd")) varirec_cd_str = 6;
				else {
					printf("Illegal VariREC CD strategy: %s\n", argv[i]);
				}
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
//  ************   VariREC  DVD
		else if(!strcmp(argv[i],"--varirec-dvd")) {
			if (argc>(i+1)) {
				i++;
				flags |= FL_VARIREC_DVD;
				int val = 0;
				if (!strcmp(argv[i],"off")) flags |= FL_VARIREC_DVD_OFF;
				else  {
					val = strtol(argv[i], NULL, 0);
					if (errno || val<-4 || val>4) {
						printf("Illegal VariREC DVD power value: %s\n", argv[i]);
						return 5;
					} else switch(val) {
						case -4: varirec_dvd_pwr = VARIREC_MINUS_4; break;
						case -3: varirec_dvd_pwr = VARIREC_MINUS_3; break;
						case -2: varirec_dvd_pwr = VARIREC_MINUS_2; break;
						case -1: varirec_dvd_pwr = VARIREC_MINUS_1; break;
						case  0: varirec_dvd_pwr = VARIREC_NULL; break;
						case  1: varirec_dvd_pwr = VARIREC_PLUS_1; break;
						case  2: varirec_dvd_pwr = VARIREC_PLUS_2; break;
						case  3: varirec_dvd_pwr = VARIREC_PLUS_3; break;
						case  4: varirec_dvd_pwr = VARIREC_PLUS_4; break;
					}
				}
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--varirec-dvd-strategy")) {
			if (argc>(i+1)) {
				i++;
				int val = atol(argv[i]);
				if (!errno && val>=-1 && val<8) {
					varirec_dvd_str = val+1;
				}
				else if (!strcmp(argv[i],"default"))  varirec_dvd_str = 0;
				else {
					printf("Illegal VariREC DVD strategy: %s\n", argv[i]);
				}
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--securec")) {
			if (argc>(i+1)) {
				i++;
				strcpy(passwd,argv[i]);
				flags |= FL_SECUREC;
			//	printf("SecuRec pass: %s\n", passwd);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--nosecurec")) flags |= FL_NOSECUREC;
		else if(!strcmp(argv[i],"--bitset+r")) {
			flags |= FL_BOOK_R;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) bookr = 0;
				else if (!strcmp(argv[i],"on")) bookr = 1;
			}
		}
		else if(!strcmp(argv[i],"--bitset+rdl")) {
			flags |= FL_BOOK_RDL;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) bookrdl = 0;
				else if (!strcmp(argv[i],"on")) bookrdl = 1;
			}
		}
		else if(!strcmp(argv[i],"--dvd+testwrite")) {
			flags |= FL_TESTWRITE;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) testwrite = 0;
				else if (!strcmp(argv[i],"on")) testwrite = 1;
			}
		}
		else if(!strcmp(argv[i],"--mqck")) {
			flags |= FL_MQCK;
			if (argc>(i+1)) {
				i++;
				as_idx_act = (int)strtol(argv[i], NULL, 0);
				if (!strcmp(argv[i],"quick"))         as_mqck = AS_MEDIACK_QUICK;
				else if (!strcmp(argv[i],"adv"))      as = AS_MEDIACK_ADV;
				else if (!strcmp(argv[i],"advanced")) as = AS_MEDIACK_ADV;
				else
					printf("invalid MQCK mode requested: %s\n", argv[i]);
			}
		}
		else if(!strcmp(argv[i],"--mqck-speed")) {
			if (argc>(i+1)) {
				i++;
				as_mqck_spd = (int)strtol(argv[i], NULL, 0);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--as-mode")) {
			flags |= FL_AS;
			if (argc>(i+1)) {
				i++;
				if (!strcmp(argv[i],"forced"))    as = AS_FORCED;
				else if (!strcmp(argv[i],"on"))   as = AS_ON;
				else if (!strcmp(argv[i],"auto")) as = AS_AUTO;
				else if (!strcmp(argv[i],"off"))  as = AS_OFF;
				else
					printf("invalid AutoStrategy mode requested: %s\n", argv[i]);
			}
		}
		else if(!strcmp(argv[i],"--as-create")) {
			flags_as |= FLAS_CREATE;
			ascre = 0;
			if (argc>(i+1)) {
				i++;
				if      (!strcmp(argv[i],"q")) ascre |= ASDB_CRE_QUICK;
				else if (!strcmp(argv[i],"f")) ascre |= ASDB_CRE_FULL;
				else
					printf("invalid --as-create parameter 1: '%s'. Only 'q|f' are valid\n", argv[i]);
			}
			if (argc>(i+1)) {
				i++;
				if      (!strcmp(argv[i],"a")) ascre |= ASDB_ADD;
				else if (!strcmp(argv[i],"r")) ascre |= ASDB_REPLACE;
				else
					printf("invalid --as-create parameter 2: '%s'. Only 'a|r' are valid\n", argv[i]);
			}
		}
		else if(!strcmp(argv[i],"--as-list")) {
			flags_as |= FLAS_VIEW;
		}
		else if(!strcmp(argv[i],"--as-clear")) {
			flags_as |= FLAS_CLEAR;
		}
		else if(!strcmp(argv[i],"--as-on")) {
			flags_as |= FLAS_ACT;
			if (argc>(i+1)) {
				i++;
				as_idx_act = (int)strtol(argv[i], NULL, 0);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--as-off")) {
			flags_as |= FLAS_DEACT;
			if (argc>(i+1)) {
				i++;
				as_idx_deact = (int)strtol(argv[i], NULL, 0);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--as-del")) {
			flags_as |= FLAS_DEL;
			if (argc>(i+1)) {
				i++;
				as_idx_del = (int)strtol(argv[i], NULL, 0);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--as-save")) {
			if (argc>(i+1)) {
				i++;
				flags_as |= FLAS_RETR;
				strcpy(assfn, argv[i]);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--as-load")) {
			if (argc>(i+1)) {
				i++;
				flags_as |= FLAS_STOR;
				strcpy(aslfn, argv[i]);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--silent")) {
			flags |= FL_SILENT;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) silent = 0;
				else if (!strcmp(argv[i],"on")) silent = 1;
			}
		}
		else if(!strcmp(argv[i],"--sm-load")) {
			flags |= FL_SILENT; silent = 1; silent_tray = 1;
			if (argc>(i+1)) {
				i++;
				silent_load = (int)strtol(argv[i], NULL, 0);
//				printf("tray load speed: %d\n", silent_load);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--sm-eject")) {
			flags |= FL_SILENT; silent = 1; silent_tray = 1;
			if (argc>(i+1)) {
				i++;
				silent_eject = (int)strtol(argv[i], NULL, 0);
//				printf("tray load speed: %d\n", silent_load);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--sm-access")) {
			flags |= FL_SILENT; silent = 1;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"fast"))  silent_access = SILENT_ACCESS_FAST;
				else if (!strcmp(argv[i],"slow")) silent_access = SILENT_ACCESS_SLOW;
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--sm-cd-rd")) {
			flags |= FL_SILENT; silent = 1; silent_cd = 1;
			if (argc>(i+1)) {
				i++;
				if      (!strcmp(argv[i],"48")) silent_cd_rd = SILENT_CD_RD_48X;
				else if (!strcmp(argv[i],"40")) silent_cd_rd = SILENT_CD_RD_40X;
				else if (!strcmp(argv[i],"32")) silent_cd_rd = SILENT_CD_RD_32X;
				else if (!strcmp(argv[i],"24")) silent_cd_rd = SILENT_CD_RD_24X;
				else if (!strcmp(argv[i], "8")) silent_cd_rd =  SILENT_CD_RD_8X;
				else if (!strcmp(argv[i], "4")) silent_cd_rd =  SILENT_CD_RD_4X;
				else
					printf("invalid --sm-cd-rd parameter: %s\n", argv[i]);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--sm-cd-wr")) {
			flags |= FL_SILENT; silent = 1; silent_cd = 1;
			if (argc>(i+1)) {
				i++;
				if      (!strcmp(argv[i],"48")) silent_cd_wr = SILENT_CD_WR_48X;
				else if (!strcmp(argv[i],"32")) silent_cd_wr = SILENT_CD_WR_32X;
				else if (!strcmp(argv[i],"24")) silent_cd_wr = SILENT_CD_WR_24X;
				else if (!strcmp(argv[i],"16")) silent_cd_wr = SILENT_CD_WR_16X;
				else if (!strcmp(argv[i], "8")) silent_cd_wr =  SILENT_CD_WR_8X;
				else if (!strcmp(argv[i], "4")) silent_cd_wr =  SILENT_CD_WR_4X;
				else
					printf("invalid --sm-cd-wr parameter: %s\n", argv[i]);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
		else if(!strcmp(argv[i],"--sm-dvd-rd")) {
			flags |= FL_SILENT; silent = 1; silent_dvd = 1;
			if (argc>(i+1)) {
				i++;
				if      (!strcmp(argv[i],"16")) silent_dvd_rd = SILENT_DVD_RD_16X;
				else if (!strcmp(argv[i],"12")) silent_dvd_rd = SILENT_DVD_RD_12X;
				else if (!strcmp(argv[i], "8")) silent_dvd_rd =  SILENT_DVD_RD_8X;
				else if (!strcmp(argv[i], "5")) silent_dvd_rd =  SILENT_DVD_RD_5X;
				else if (!strcmp(argv[i], "2")) silent_dvd_rd =  SILENT_DVD_RD_2X;
				else
					printf("invalid --sm-dvd-rd parameter: %s\n", argv[i]);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
/*
		else if(!strcmp(argv[i],"--sm-dvd-wr")) {
			flags |= FL_SILENT; silent = 1; silent_dvd = 1;
			if (argc>(i+1)) {
				i++;
				if      (!strcmp(argv[i],"18")) silent_dvd_wr = SILENT_DVD_WR_18X;
				else if (!strcmp(argv[i],"16")) silent_dvd_wr = SILENT_DVD_WR_12X;
				else if (!strcmp(argv[i],"12")) silent_dvd_wr = SILENT_DVD_WR_12X;
				else if (!strcmp(argv[i], "8")) silent_dvd_wr =  SILENT_DVD_WR_8X;
				else if (!strcmp(argv[i], "6")) silent_dvd_wr =  SILENT_DVD_WR_6X;
				else if (!strcmp(argv[i], "4")) silent_dvd_wr =  SILENT_DVD_WR_4X;
				else
					printf("invalid --sm-dvd-wr parameter: %s\n", argv[i]);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		}
*/
		else if(!strcmp(argv[i],"--sm-nosave")) silent_save = 0;
		else if(!strcmp(argv[i],"--destruct")) {
			if (argc>(i+1)) {
				i++;
				if (!strcmp(argv[i],"quick")) { flags |= FL_PXERASER; pxeraser = PLEXERASER_QUICK; }
				if (!strcmp(argv[i],"full"))  { flags |= FL_PXERASER; pxeraser = PLEXERASER_FULL;  }
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		} else if(!strcmp(argv[i],"--amqr")) {
			flags |= FL_YMH_AMQR;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) amqr = 0;
				else if (!strcmp(argv[i],"on")) amqr = 1;
			}
		} else if(!strcmp(argv[i],"--forcespeed")) {
			flags |= FL_YMH_FORCESPEED;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) forcespeed = 0;
				else if (!strcmp(argv[i],"on")) forcespeed = 1;
			}


//	printf("\t--pio-limit [on|off]         limit (or not) read speed by 24x for CD and 8x for DVD\n");	
//	printf("\t--pio-quiet [quiet|perf|std] select QuietMode setting: Quiet, Performance or Standard (default: quiet)\n");
//	printf("\t--pio-nosave				   don't save settings to drive (changes will be lost after reboot)\n");

		} else if(!strcmp(argv[i],"--pio-nosave")) {
			silent_save = 0; 
		} else if(!strcmp(argv[i],"--pio-limit")) {
			flags |= FL_PIOLIMIT;
			if (argc>(i+1)) {
				i++;
				if     (!strcmp(argv[i],"off")) piolimit = 0;
				else if (!strcmp(argv[i],"on")) piolimit = 1;
			}
		} else if(!strcmp(argv[i],"--pio-quiet")) {
			flags |= FL_PIOQUIET;
			if (argc>(i+1)) {
				i++;
				if      (!strcmp(argv[i],"quiet")) piosilent = PIO_SILENT_QUIET;
				else if (!strcmp(argv[i],"perf"))  piosilent = PIO_SILENT_PERF;
				else if (!strcmp(argv[i],"std"))   piosilent = PIO_SILENT_STD;
				else
					printf("invalid --pio-quiet parameter: %s\n", argv[i]);
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}

		} else if(!strcmp(argv[i],"-v")) flags |= FL_VERBOSE;
		else if(!strcmp(argv[i],"--verbose")) flags |= FL_VERBOSE;
		else {
			printf("unknown option: %s\n", argv[i]);
			return 6;
		}
	}
	if (flags & FL_HELP) {
		usage(argv[0]);
		return 0;
	}
	if (!flags) {
		usage(argv[0]);
		return 1;
	}

	if (flags & FL_SCAN) {
		drvcnt = scanbus( DEV_PLEXTOR | DEV_YAMAHA );
		if (!drvcnt) printf("ERR: no drives found!\n");
		return 2;
	}
	if (!(flags & FL_DEVICE)) {
		printf("** ERR: no device selected\n");
		return 3;
	}
//	printf("____________________________\n");
	printf("Device : %s\n", device);
	dev = new drive_info(device);
	if (dev->err) {
		printf("%s: can't open device: %s\n", argv[0], device);
		delete dev;
		return 4;
	}

	inquiry(dev);
//	convert_to_ID(dev);
	printf("Vendor : '%s'\n",dev->ven);
	printf("Model  : '%s'",dev->dev);
	if ( isPlextor(dev)) {
		plextor_get_TLA(dev);
		printf(" (TLA#%s)",dev->TLA);
	}
	printf("\nF/W    : '%s'\n",dev->fw);

	if (!(flags & FL_VERBOSE)) dev->silent++;
	if (get_drive_serial_number(dev)) printf("Serial#: %s\n",dev->serial);

	if (flags) {
//	if (flags & FL_VERBOSE) {
		printf("\nCDVD Control flags : ");
		if (flags & FL_VERBOSE)		printf(" VERBOSE");
		if (flags & FL_DEVICE)		printf(" DEVICE");
		if (flags & FL_HELP)		printf(" HELP");
		if (flags & FL_CURRENT)		printf(" CURRENT");
		if (flags & FL_SUPPORTED)	printf(" SUPPORTED");
		if (flags & FL_SCAN)		printf(" SCAN");
		if (flags & FL_LOCK)		printf(" LOCK");
		if (flags & FL_HCDR)		printf(" HCDR");
		if (flags & FL_SSS)			printf(" SSS");
		if (flags & FL_SPDREAD)		printf(" SPDREAD");
		if (flags & FL_POWEREC)		printf(" POWEREC");
		if (flags & FL_GIGAREC)		printf(" GIGAREC");
		if (flags & FL_VARIREC_CD)	printf(" VARIREC_CD");
		if (flags & FL_VARIREC_DVD)	printf(" VARIREC_DVD");
		if (flags & FL_SECUREC)		printf(" SECUREC");
		if (flags & FL_BOOK_R)		printf(" BITSET_R");
		if (flags & FL_BOOK_RDL)	printf(" BITSET_RDL");
		if (flags & FL_TESTWRITE)	printf(" DVD+TESTWRITE");
		if (flags & FL_AS)			printf(" AS");
		if (flags & FL_SILENT)		printf(" SILENT");
		if (flags & FL_YMH_AMQR)	printf(" AMQR");
		if (flags & FL_YMH_FORCESPEED)	printf(" FORCESPEED");
		if (flags & FL_PIOLIMIT)	printf(" PIOLIMIT");
		if (flags & FL_PIOQUIET)	printf(" PIOQUIET");
		if (flags & FL_LOEJ)		printf(" LOEJ");
		if (flags_as & FLAS_RETR)	printf(" AS_RETR");
		if (flags_as & FLAS_STOR)	printf(" AS_STOR");
		if (flags_as & FLAS_CREATE)	printf(" AS_CREATE");
		if (flags_as & FLAS_DEL)	printf(" AS_DEL");
		if (flags_as & FLAS_VIEW)	printf(" AS_VIEW");
		if (flags_as & FLAS_ACT)	printf(" AS_ACT");
		if (flags_as & FLAS_DEACT)	printf(" AS_DEACT");
		if (flags_as & FLAS_CLEAR)	printf(" AS_CLEAR");
		printf("\n\n");
	}
//	printf("____________________________\n");
	if (flags & FL_LOCK) {
//		dev->silent++;
		switch (lock) {
			case 0:
				printf("Unlocking media...\n");
				dev->parms.status &= (~STATUS_LOCK);
				break;
			case 1:
				printf("Locking media...\n");
				dev->parms.status |= STATUS_LOCK;
				break;
			case 2:
				printf("Toggle media lock state...\n");
				get_lock(dev);
				dev->parms.status ^= STATUS_LOCK;
				break;
		}
		set_lock(dev);
		printf("Media is%s locked\n", (dev->parms.status & STATUS_LOCK) ? "" : " NOT");
//		dev->silent--;
	}

	if (flags & FL_LOEJ) {
//		printf("loej_immed: %d\n",loej_immed);
		if (eject == 2) {
			load_eject(dev, loej_immed);
		} else {
			load_eject(dev,!eject, loej_immed);
		}
	}

// PLEXTOR features
	if (flags & FL_POWEREC) {
//		printf("Set PoweREC...\n");
		dev->plextor.powerec_state = powerec;
		plextor_set_powerec(dev);
	}
/*
	if (flags & FL_YMH_AMQR) {
		dev->yamaha.amqr = amqr;
		yamaha_set_amqr(dev);
	}
	if (flags & FL_YMH_FORCESPEED) {
		dev->yamaha.forcespeed = forcespeed;
		yamaha_set_forcespeed(dev);
	}
*/
	if (flags & FL_GIGAREC) {
//		printf("Set GigaREC...\n");
		dev->plextor.gigarec = gigarec;
		plextor_set_gigarec(dev);
	}
	if (flags & FL_VARIREC_CD) {
//		printf("Set VariREC CD...\n");
//		printf("PWR = %02X   STR = %02X\n",varirec_cd_pwr, varirec_cd_str);
		dev->plextor.varirec_state_cd = !(flags & FL_VARIREC_CD_OFF);
		dev->plextor.varirec_pwr_cd = varirec_cd_pwr;
		dev->plextor.varirec_str_cd = varirec_cd_str;
		plextor_set_varirec(dev, VARIREC_CD);
	}
	if (flags & FL_VARIREC_DVD) {
//		printf("Set VariREC DVD...\n");
//		printf("PWR = %02X   STR = %02X\n",varirec_dvd_pwr, varirec_dvd_str);
		dev->plextor.varirec_state_dvd = !(flags & FL_VARIREC_DVD_OFF);
		dev->plextor.varirec_pwr_dvd = varirec_dvd_pwr;
		dev->plextor.varirec_str_dvd = varirec_dvd_str;
		plextor_set_varirec(dev, VARIREC_DVD);
	}
	if (flags & FL_NOSECUREC) {
		plextor_set_securec(dev, 0, NULL);
	}
	else if (flags & FL_SECUREC) {
		int pwdlen = (char)strlen(passwd);
		if ((pwdlen>=4) && (pwdlen<=10))
			plextor_set_securec(dev, pwdlen, passwd);
		else
			printf("Invalid SecuRec password length! must be 4..10\n");
	}
	if (flags & FL_HCDR) {
		plextor_set_hidecdr(dev, hcdr);
	}
	if (flags & FL_SSS) {
		plextor_set_singlesession(dev, sss);
	}
	if (flags & FL_SPDREAD) {
		plextor_set_speedread(dev, spdread);
	}
	if (flags & FL_BOOK_R) {
		dev->book_plus_r = bookr;
		plextor_set_bitset(dev, PLEX_BITSET_R);
	}
	if (flags & FL_BOOK_RDL) {
		dev->book_plus_rdl = bookrdl;
		plextor_set_bitset(dev, PLEX_BITSET_RDL);
	}
	if (flags & FL_AS) {
		dev->astrategy.state = as;
		plextor_set_autostrategy(dev);
	}

	if (flags & FL_MQCK) {
		if (dev->media.type & ( DISC_CDROM | DISC_DDCD_ROM| DISC_DVDROM | DISC_BD_ROM | DISC_HDDVD_ROM)) {
			printf("Can't run MQCK on stamped media!\n");
			return 5;
		}
		detect_speeds(dev);

		printf("Available write speeds:\n");
		printf("WR speed max: %4.1fX (%d kB/s)\n",
					((float)dev->parms.max_write_speed_kb) / dev->parms.speed_mult,
					dev->parms.max_write_speed_kb);
		for (int i=0; i<speed_tbl_size && dev->parms.wr_speed_tbl_kb[i] > 0; i++) {
			printf("  speed #%02d: %4.1fX (%d kB/s)\n", i,
					((float)dev->parms.wr_speed_tbl_kb[i]) / ((float)dev->parms.speed_mult),
					dev->parms.wr_speed_tbl_kb[i]);
		}
		if (as_mqck >= 0) {
			dev->parms.max_write_speed_kb = (int) (as_mqck_spd * dev->parms.speed_mult),
			set_rw_speeds(dev);
			get_rw_speeds(dev);
			if ( !(dev->media.type & (DISC_DVD)) ) {
				printf("MQCK: Media Quality Check supported on DVD media only!\n");
				return 5;
			}
			printf("Starting media check at speed %.1f X...\n",
				((float)dev->parms.max_write_speed_kb) / dev->parms.speed_mult);
			if (plextor_media_check(dev, as_mqck)) {
				if ((dev->err & 0x0FFF00) == 0x023A00)
					printf("MQCK: No media found\n");
				else
					printf("MQCK: Error starting Media Check\n");
				return 5;
			}
			if (!dev->rd_buf[0x11]) {
				if ((dev->rd_buf[0x10] & 0xFF) == 0xFF)
					printf("MQCK: Cant' run Media Check: AUTOSTRATEGY is OFF\n");
				else
					printf("MQCK: Media is GOOD for writing at selected speed\n");
			} else {
				switch (dev->rd_buf[0x10]) {
					case 0x01:
						printf("MQCK: Write error may occur\n");
						break;
					case 0x03:
						printf("MQCK: Drive may not write correctly at selected speed\n");
						break;
					default:
						printf("MQCK: Unknown MQCK error: 0x%02X\n", dev->rd_buf[0x10]);
				}
			}
		}
	}

	dev->silent--;
	if (flags_as & FLAS_VIEW) {
		plextor_get_autostrategy_db_entry_count(dev);
		plextor_get_autostrategy_db(dev);
	}
	if (flags_as & FLAS_CLEAR) {
		printf("deleting all AutoStrategy entries...\n");
		plextor_clear_autostrategy_db(dev);
	}
	if (flags_as & FLAS_ACT) {
		printf("activating strategy #%d\n", as_idx_act);
		plextor_modify_autostrategy_db(dev, as_idx_act, ASDB_ENABLE);
	}
	if (flags_as & FLAS_DEACT) {
		printf("DEactivating strategy #%d\n", as_idx_deact);
		plextor_modify_autostrategy_db(dev, as_idx_deact, ASDB_DISABLE);
	}
	if (flags_as & FLAS_DEL) {
		printf("DELETING strategy #%d\n", as_idx_del);
		plextor_modify_autostrategy_db(dev, as_idx_del, ASDB_DELETE);
	}
	if (flags_as & FLAS_CREATE) {
		printf("AS: Creating new strategy, mode: %s, action: %s...\n",
			(ascre & ASDB_CRE_FULL) == ASDB_CRE_FULL? "FULL" : "QUICK",
			(ascre & ASDB_ADD) == ASDB_ADD? "ADD" : "REPLACE");
		plextor_create_strategy(dev, ascre);
	}
	if (flags_as & FLAS_RETR) {
		unsigned char hdr[9]; memset(hdr, 0, 9);
		printf("AS RETR...\n");
		plextor_get_strategy(dev);
		printf("Saving AS DB...\n");
		asf = fopen(assfn,"wb");
		if (!asf) { printf("can't create asdb file!\n"); return 6; }
		memcpy(hdr,"ASDB ",5);
		hdr[5] = dev->astrategy.dbcnt;
		fwrite((void*)hdr,1,8,asf);
		for (i=0; i<dev->astrategy.dbcnt; i++) {
			fwrite((void*)&dev->astrategy.entry[i],1,0x20,asf);
			for (int j=0; j<7; j++)
				fwrite((void*)&dev->astrategy.entry_data[i][j],1,0x20,asf);
		}
		fclose(asf);
	}
	if (flags_as & FLAS_STOR) {
		unsigned char hdr[9]; memset(hdr, 0, 9);
		printf("AS STOR...\n");

		printf("Loading AS DB...\n");
		asf = fopen(aslfn,"rb");
		if (!asf)
			{ printf("can't open asdb file!\n"); return 6; }
		if (fread((void*)hdr,1,8,asf)<8)
			{ printf("error reading asdb file!\n"); return 6; }

		if (!strncmp((char*)hdr,"ASDB ",5))
		dev->astrategy.dbcnt = hdr[5];
		i=0;
		while (!feof(asf) && i<dev->astrategy.dbcnt) {
			if (fread((void*)&dev->astrategy.entry[i],1,0x20,asf)<0x20)
				{ printf("error reading asdb file!\n"); return 6; }
			for (int j=0; j<7; j++)
				if (fread((void*)&dev->astrategy.entry_data[i][j],1,0x20,asf)<0x20)
					{ printf("error reading asdb file!\n"); return 6; }

			i++;
		}
		fclose(asf);
		if (dev->astrategy.dbcnt != i) {
			printf("Can't read all strategies! File corrupted!\n");
			return 5;
		}
		printf("%d strategies loaded, sending to drive...\n",dev->astrategy.dbcnt);
		plextor_add_strategy(dev);
	}
	if (flags_as & (FLAS_ACT | FLAS_DEACT | FLAS_DEL | FLAS_CREATE | FLAS_STOR | FLAS_CLEAR)) {
		printf("AutoStrategy DB modified...\n");
		plextor_get_autostrategy_db_entry_count(dev);
		plextor_get_autostrategy_db(dev);
	}
dev->silent++;
	if (flags & FL_TESTWRITE) {
		dev->plextor.testwrite_dvdplus = testwrite;
		plextor_set_testwrite_dvdplus(dev);
	}
	if (flags & FL_SILENT) {
		plextor_get_silentmode(dev);
		if (!silent)
			plextor_set_silentmode_disable(dev, silent_save);
		else {
			if (!dev->plextor_silent.state)
				{ silent_cd = 1; silent_dvd = 1; silent_tray = 1; }
			if (silent_cd) {
				dev->plextor_silent.access	= silent_access;
				dev->plextor_silent.rd		= silent_cd_rd;
				dev->plextor_silent.wr		= silent_cd_wr;
				plextor_set_silentmode_disc(dev, SILENT_CD, silent_save);
			}
			if (silent_dvd) {
				dev->plextor_silent.access	= silent_access;
				dev->plextor_silent.rd		= silent_dvd_rd;
//				dev->plextor_silent.wr		= silent_dvd_wr;
				plextor_set_silentmode_disc(dev, SILENT_DVD, silent_save);
			}
			if (silent_tray) {
				dev->plextor_silent.load	= silent_load;
				dev->plextor_silent.eject	= silent_eject;
				plextor_set_silentmode_tray(dev, SILENT_CD, silent_save);
			}
		}
	}
	if (flags & FL_PXERASER) {
		dev->plextor.plexeraser = pxeraser;
		plextor_plexeraser(dev);
	}

// PIONEER features
	if (flags & FL_PIOQUIET) {
		pioneer_set_silent(dev, piosilent, silent_save);
	}
	if (flags & FL_PIOLIMIT) {
		pioneer_set_spdlim(dev, piolimit, silent_save);
	}

	get_device_info(dev);
	if (!(flags & FL_VERBOSE)) dev->silent--;
	delete dev;
	return 0;
}
