/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2007-2012, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <getopt.h>
#include <string.h>
#include <strings.h>
#include <signal.h>

#include <qpx_mmc.h>
#include <qpx_scan.h>
#include <plextor_features.h>

#ifdef _WIN32
#include <wincon.h>
#endif

#include "version.h"

#define MSGPREF "qscan: "
#define CAPDEV   "CD: "
#define CAPVND   "CV: "
#define CAPMEDIA "CM: "
#define IDEV     "ID: "
#define IMEDIA   "IM: "
#define SMEDIA   "SM: "

//#define USE_SIGUSR2

#define FL_INFO		0x00000001
#define FL_SCANBUS	0x00000002
#define FL_DEV		0x00000004
#define FL_SPEED	0x00000008
#define FL_SPEEDS	0x00000010
#define FL_MINFO	0x00000020
#define FL_DEBUG	0x00000040
#define FL_LPLUGIN	0x00000080
#define FL_DINFO	0x00000100
#define FL_MID_RAW	0x00000200

qscanner	*scanner;

#if defined (__unix) || defined (__unix__)
void sigint_handler (int) {
	printf("\nSIGINT\n");
	if (scanner) {
		printf("Terminating scan...\n");
		scanner->stop();
	}
//	printf("\nwaiting scanner to stop\n");
}

void sigusr_handler (int signum) {
	switch (signum) {
		case SIGUSR1:
			printf("\nSIGUSR1\n");
			if (scanner)
				scanner->stat();
			break;
#ifdef USE_SIGUSR2
		case SIGUSR2:
			printf("\nSIGUSR2\n");
			if (scanner)
				scanner->stat();
			break;
#endif
		default:
			break;
	}
}

#elif defined (_WIN32)
BOOL WINAPI sigint_handler (DWORD) {
	printf("\nSIGINT\n");
	if (scanner) {
		printf("Terminating scan...\n");
		scanner->stop();
	}
	return true;
//	printf("\nwaiting scanner to stop\n");
}
#endif

void detect_vendor_features(drive_info *dev) {
	if (isPlextor(dev)) {
		plextor_get_life(dev);
//		if ((dev->dev_ID == PLEXTOR_755) || (dev->dev_ID == PLEXTOR_760) || (dev->dev_ID == PLEXTOR_PREMIUM2))
//		if (isPlextorLockPresent(dev))
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
	//if ((dev->dev_ID == PLEXTOR_755) || (dev->dev_ID == PLEXTOR_760) || (dev->dev_ID == PLEXTOR_PREMIUM2))
	if (isPlextorLockPresent(dev))
		dev->ven_features|=PX_ERASER;
#if 0
	} else if (!strncmp(dev->ven,"YAMAHA",7)) {
		if (!yamaha_check_amqr(dev)) dev->ven_features|=YMH_AMQR;
		if (!yamaha_check_forcespeed(dev)) dev->ven_features|=YMH_FORCESPEED;
#endif
	}
}

static struct option long_options[] = {
	{"help",      0, NULL, 'h'},
	{"scan",      0, NULL, 'l'},
	{"dev",       1, NULL, 'd'},
	{"plugins",   0, NULL, 'p'},
	{"force-plugin", 1, NULL, 'f'},
	{"test",      1, NULL, 't'},
	{"write",     0, NULL, 'W'},
	{"speed",     1, NULL, 's'},
	{"speeds",    0, NULL, 'S'},
	{"rspeed",    1, NULL, 'r'},
	{"wspeed",    1, NULL, 'w'},
	{"info",      0, NULL, 'i'},
	{"infoshort", 0, NULL, 'I'},
	{"media",     0, NULL, 'm'},
	{"mediashort",0, NULL, 'M'},
	{"verbose",   0, NULL, 'v'},
	{0,0,0,0}
};

void show_available_errc_data(qscanner *scanner) {
	int errc_data = 0;
	drive_info *dev;
	if (!scanner || !(dev=scanner->device())) return;
	errc_data = scanner->errc_data();
	printf(IMEDIA "ERRC data     :");
	if (dev->media.type & DISC_CD) {
		for (int i=0; i<8; i++) if (errc_data & (1<<i)) printf(" %s",errc_names_cd[i]);
	} else if (dev->media.type & DISC_DVD) {
		for (int i=0; i<8; i++) if (errc_data & (1<<i)) printf(" %s",errc_names_dvd[i]);
	} else if (dev->media.type & DISC_BD) {
		for (int i=0; i<8; i++) if (errc_data & (1<<i)) printf(" %s",errc_names_bd[i]);
	}
	printf("\n");
}

int main(int argc, char** argv) {
	int result = 0;
	
	drive_info  *dev = NULL;
/*
	struct sigaction sa_int;
	struct sigaction sa_usr1;
	struct sigaction sa_usr2;
*/
    unsigned int flags=0;
    char    *device = NULL;
	char	*test   = NULL;
	char    *pname  = NULL;
	int     c;
	int     speed=-1;
	int     rspeed=-1;
	int     wspeed=-1;
	scanner=NULL;
	bool	simul=1;
	printf( "qScan " VERSION " (C) 2007-2009  Gennady \"ShultZ\" Kozlov\n");
	while (1) {
		c = getopt_long(argc, argv, "hvliImMd:pf:t:WSs:r:w:", long_options, NULL);
		if (c == -1)
			break;
		switch(c) {
			case 'h':
				printf("Usage: qscan <options>\n");
				printf("\n");
				printf("qscan is a CD/DVD quality scanning utility\n");
				printf("\n");
				printf("-h --help           you are reading this:)\n");
				printf("-d --dev DEVICE     use this device\n");
				printf("-s --speed #        test media at given speed (default: maximum). Don't use with -r or -w\n");
				printf("-S --speeds         detect available read/write speeds\n");
				printf("-r --rspeed #       set read speed\n");
				printf("-w --wspeed #       set write speed\n");
				printf("-l --scanbus        scan IDE/SATA/SCSI buses for MMC-compliant devices\n");
				printf("-t --test TEST      run specified test:\n");
				printf("                    rt   : Read Transfer rate\n");
				printf("                    wt   : Write Transfer rate\n");
				printf("                    errc : Error Correction \n");
				printf("                    jb   : Jitter/Asymmetry\n");
				printf("                    ft   : Focus/Tracking errors\n");
				printf("                    ta   : Time Analyser\n");
				printf("-W --write          do real write instead simulation (for use with --test wt)\n");
				printf("-p --plugins        list all available plugins\n");
				printf("-f --force PLUGIN   force using specified plugin (default: autodetect)\n");
				printf("-I --shortinfo      print device info\n");
				printf("-i --info           print device info (with supported features list)\n");
				printf("-m --media          print media info\n");
				printf("-M --mediashort     print media info without raw media identification data\n");
				printf("-v --verbose        print a lot of debug info\n");
				printf("\n");
				return 0;
			case 'd':
				if (flags & FL_DEV) {
					printf("Duplicated device option ignored: %s\n", optarg);
				} else {
					device = optarg;
					flags |= FL_DEV;
				}
				break;
			case 'i':
				if (!device) {
					printf( MSGPREF "no device selected!\n");
				}
				flags |= (FL_DINFO | FL_INFO);
				break;
			case 'I':
				if (!device) {
					printf( MSGPREF "no device selected!\n");
				}
				flags |= FL_INFO;
				break;
			case 'M':
				if (!device) {
					printf( MSGPREF "no device selected!\n");
				}
				flags |= FL_MINFO;
				break;
			case 'm':
				if (!device) {
					printf( MSGPREF "no device selected!\n");
				}
				flags |= FL_MINFO;
				flags |= FL_MID_RAW;
				break;
			case 'S':
				flags |= FL_SPEEDS;
				break;
			case 's':
				flags |= FL_SPEED;
				speed = atol(optarg);
				break;
			case 'r':
//				printf(MSGPREF " -r option conflicts with -s!\n");
				flags |= FL_SPEED;
				rspeed = atol(optarg);
				break;
			case 'w':
				flags |= FL_SPEED;
				wspeed = atol(optarg);
				break;
			case 'l':
				flags |= FL_SCANBUS;
				scanbus();
				return 0;
			case 't':
				if (!(strcmp(optarg, "rt")
					&& strcmp(optarg, "wt")
					&& strcmp(optarg, "errc")
					&& strcmp(optarg, "jb")
					&& strcmp(optarg, "ft")
					&& strcmp(optarg, "ta")))
				{
					test = optarg;
				} else {
					printf( MSGPREF "invalid test name: %s\n", optarg);
					return 4;
				}
				break;
			case 'W':
				simul=0;
				break;
			case 'f':
				pname = optarg;
				break;
			case 'p':
				if (test) {
					printf( MSGPREF "option -p conflicts with -t !\n");
				} else {
					flags |= FL_LPLUGIN;
				}
				break;
			case 'v':
				flags |= FL_DEBUG;
				break;
			default:
				break;
		}
	}
	if (!device) {
		printf( MSGPREF "no device specified! Try using -l to see list\n");
		return 1;
	}

    dev = new drive_info(device);
    if (dev->mmc < 0) {
		printf( MSGPREF " can't open device %s!\n",device);
    		delete dev;
		return 2;	
    }
    switch (inquiry(dev)) {
		case 0:
		    break;
		case ERR_NO_DEV:
		    printf( MSGPREF "%s: no such device!\n",device);
		    delete dev;
		    return 3;
		case ERR_NO_SCSI:
		    printf( MSGPREF "%s: device is not SCSI-compliant!\n",device);
		    delete dev;
		    return 3;
		case ERR_NO_MMC:
			printf( MSGPREF "%s: device is not MMC-compliant!\n",device);
    	    delete dev;
			return 3;
		default:
			printf( MSGPREF "%s: unknown error\n",device);
    	    delete dev;
			return 3;
    }
	printf( MSGPREF "using device '%s': '%s' '%s' '%s'\n", device, dev->ven, dev->dev, dev->fw);
    //dev->silent = 0;
//	get_features_list(dev);
	if (!(flags & FL_DEBUG)) dev->silent++;
    detect_capabilities(dev);
	get_buffer_capacity(dev);
    determine_disc_type(dev);
    read_capacities(dev);

	detect_vendor_features(dev);
	get_rw_speeds(dev);

//	if (!(flags & FL_DEBUG)) dev->silent--;

//	if (flags & (FL_INFO | FL_MINFO | FL_SPEED | FL_SPEEDS))
	detect_speeds(dev);
//	test_dma_speed(dev);

    if (flags & FL_INFO) {
		printf(IDEV "Device: '%s'\n", device);
		printf(IDEV "Vendor: '%s'\n",dev->ven);
		printf(IDEV "Model : '%s'\n",dev->dev);
		printf(IDEV "F/W   : '%s'\n",dev->fw);
		if (isPlextor(dev)) {
			plextor_get_TLA(dev);
			printf(IDEV "TLA#  : %s\n",dev->TLA);
		} else {
			printf(IDEV "TLA#  : N/A \n");
		}
		printf(IDEV "Buffer: %d kB\n", dev->buffer_size);
		printf(IDEV "S/N   : %s\n", dev->serial);
		printf(IDEV "IFace : %s\n", dev->iface);
		printf(IDEV "Loader: %s\n", loader_list[dev->loader_id]);

		if (dev->life.ok) {
			printf(IDEV "Discs loaded: %6d\n", dev->life.dn);
			printf(IDEV "Drive operating time:\n");
			printf(IDEV "  CD Rd  : %4d:%02d:%02d\n", dev->life.cr.h, dev->life.cr.m, dev->life.cr.s);
			printf(IDEV "  CD Wr  : %4d:%02d:%02d\n", dev->life.cw.h, dev->life.cw.m, dev->life.cw.s);
			if (dev->rd_capabilities & DEVICE_DVD)
				printf(IDEV "  DVD Rd : %4d:%02d:%02d\n", dev->life.dr.h, dev->life.dr.m, dev->life.dr.s);
			if (dev->wr_capabilities & DEVICE_DVD)
				printf(IDEV "  DVD Wr : %4d:%02d:%02d\n", dev->life.dw.h, dev->life.dw.m, dev->life.dw.s);
		}
	}

	if (flags & (FL_INFO | FL_MINFO)) {
		get_rw_speeds(dev);

		printf(IDEV "Current read speed  : %4.1fX  ( %5d kB/s)\n",
			(float)dev->parms.read_speed_kb / dev->parms.speed_mult,
			dev->parms.read_speed_kb);
		if (dev->wr_capabilities) {
			printf(IDEV "Current write speed : %4.1fX  ( %5d kB/s) \n",
				(float)dev->parms.write_speed_kb / dev->parms.speed_mult,
				dev->parms.write_speed_kb);
		}
	}

	if (dev->wr_capabilities) {
		check_write_modes(dev);
	}
	if (flags & FL_INFO) {
		printf(IDEV "Device Generic capabilities : 0x%016LX\n",dev->capabilities);
		printf(IDEV "Device Read capabilities    : 0x%016LX\n",dev->rd_capabilities);
		printf(IDEV "Device Write capabilities   : 0x%016LX\n",dev->wr_capabilities);
	}

	if (dev->wr_capabilities) {
		printf(IDEV "Supported write modes:");
		for (int i=0; wr_modes[i].id; i++) {
			if ((dev->wr_modes & wr_modes[i].id) == wr_modes[i].id) printf(" %s", wr_modes[i].name);
		}
		printf("\n");
	}


    if (flags & FL_DINFO) {
		printf("\nDevice capabilities:\n");	
		for (int idx=0; capabilities[idx].id || strlen(capabilities[idx].name); idx++) {
			if (capabilities[idx].id)
				printf(CAPDEV "%18s: %s\n", capabilities[idx].name,
					(dev->capabilities & capabilities[idx].id) ? "YES" : "-");
		}

		if (dev->capabilities & CAP_DVD_CSS) {
			get_rpc_state(dev);
			printf(IDEV "  RPC phase       : %d\n", dev->rpc.phase);
			if (dev->rpc.phase == 2) {
				if (dev->rpc.region)
					printf(IDEV "  Cur Region      : %d\n", dev->rpc.region);
				else
					printf(IDEV "  Cur Region      : not set\n");
				printf(IDEV "  Changes left    : %d\n", dev->rpc.ch_u);
				printf(IDEV "  Resets left     : %d\n", dev->rpc.ch_v);
			}
		}
		printf(CAPDEV "Media is locked   : %s\n", (dev->parms.status & STATUS_LOCK) ? "LOCK": "OFF");
/*
		if (get_drive_serial_number(dev)) {
			text_serial->setText(drive->serial);
		}
*/
		printf("\nVendor-specific features:\n");
#if 0
		printf("AudioMaster Q.R.: %s\n", dev->ven_features & YMH_AMQR ? "YES" : "-");
//		if ((dev->ven_features & YMH_AMQR)       && ((flags & FL_CURRENT) || (flags & FL_YMH_AMQR)))
//			printf("AudioMaster Q.R.    : %s\n", drive->yamaha.amqr ? "ON":"OFF");
		printf("Yamaha ForceSpd : %s\n", dev->ven_features & YMH_FORCESPEED ? "YES" : "-");
//		if ((dev->ven_features & YMH_FORCESPEED)       && ((flags & FL_CURRENT) || (flags & FL_YMH_FORCESPEED)))
//			printf("Yamaha ForceSpeed   : %s\n", drive->yamaha.forcespeed ? "ON":"OFF");
#endif
		printf(CAPVND "Hide CD-R         : %s\n",
			dev->ven_features & PX_HCDRSS ? (dev->plextor.hcdr ? "ON":"OFF") : "-");
		printf(CAPVND "SingleSession     : %s\n",
			dev->ven_features & PX_HCDRSS ? (dev->plextor.sss ? "ON":"OFF") : "-");
		printf(CAPVND "SpeedRead         : %s\n",
			dev->ven_features & PX_SPDREAD ? (dev->plextor.spdread ? "ON":"OFF") : "-");
		printf(CAPVND "Silent mode       : %s\n",
			dev->ven_features & PX_SILENT ? (dev->plextor_silent.state ? "ON":"OFF") : "-");
		if ((dev->ven_features & PX_SILENT) && (dev->plextor_silent.state)) plextor_print_silentmode_state(dev);
		printf(CAPVND "SecuRec           : %s\n",
			dev->ven_features & PX_SECUREC ? (dev->plextor.securec ? "ON":"OFF") : "-");
		printf(CAPVND "PoweRec           : %s\n",
			dev->ven_features & PX_POWEREC ? (dev->plextor.powerec_state ? "ON":"OFF") : "-");
		if (dev->ven_features & PX_POWEREC) {
			if (dev->media.type & DISC_CD)
				printf(CAPVND "    PoweREC Speed : %dX (CD)\n",dev->plextor.powerec_spd / 176);
			if (dev->media.type & DISC_DVD)
				printf(CAPVND "    PoweREC Speed : %dX (DVD)\n",dev->plextor.powerec_spd / 1385);
		}
		printf(CAPVND "GigaRec           : %s\n", dev->ven_features & PX_GIGAREC ? "YES" : "-");
		if (dev->ven_features & PX_GIGAREC) print_gigarec_value(dev);
		printf(CAPVND "VariRec CD        : %s\n", dev->ven_features & PX_VARIREC_CD ? (dev->plextor.varirec_state_cd ? "ON":"OFF") : "-");
		if ((dev->ven_features & PX_VARIREC_CD) && (dev->plextor.varirec_state_cd)) print_varirec(dev, VARIREC_CD);
		if (dev->wr_capabilities & DEVICE_DVD) {
			printf(CAPVND "VariRec DVD       : %s\n", dev->ven_features & PX_VARIREC_DVD ? (dev->plextor.varirec_state_dvd ? "ON":"OFF") : "-");
			if ((dev->ven_features & PX_VARIREC_DVD) && (dev->plextor.varirec_state_dvd)) print_varirec(dev, VARIREC_DVD);
			printf(CAPVND "DVD+R bitset      : %s\n",
				dev->ven_features & PX_BITSET_R ? (dev->book_plus_r ? "DVD-ROM book" : "DVD+R book") : "-");
			printf(CAPVND "DVD+R DL bitset   : %s\n",
				dev->ven_features & PX_BITSET_RDL ? (dev->book_plus_rdl ? "DVD-ROM book" : "DVD+R DL book") : "-");
			printf(CAPVND "DVD+ testwrite    : %s\n",
				dev->ven_features & PX_SIMUL_PLUS ? (dev->plextor.testwrite_dvdplus ? "ON":"OFF") : "-");
			printf(CAPVND "AutoStrategy      : %s%s\n",
				dev->ven_features & PX_ASTRATEGY ? "YES" : "-",
				((dev->dev_ID == PLEXTOR_755) || (dev->dev_ID == PLEXTOR_760)) ? " (EXTENDED)" : "");
			if (dev->ven_features & PX_ASTRATEGY) { printf(CAPVND); plextor_print_autostrategy_state(dev); }
		}
		printf(CAPVND "PlexEraser        : %s\n", dev->ven_features & PX_ERASER ? "YES" : "-");

		printf("\nDevice Read/Write capabilities:\n");
		for (int idx=0; rw_capabilities[idx].id || strlen(rw_capabilities[idx].name); idx++) {
			if (rw_capabilities[idx].id)
				printf(CAPMEDIA "%11s: %c%c\n", rw_capabilities[idx].name,
					(dev->rd_capabilities & rw_capabilities[idx].id) ? 'R' : '-',
					(dev->wr_capabilities & rw_capabilities[idx].id) ? 'W' : '-');
		}
		if (!(flags & FL_MINFO))
			goto end;
    }

	if ((flags & (FL_SPEEDS | FL_MINFO)) && dev->media.type) {
		printf(SMEDIA "Read speeds:\n");
		printf(SMEDIA "  RD speed max: %4.1fX (%d kB/s)\n",
					((float)dev->parms.max_read_speed_kb) / dev->parms.speed_mult,
					dev->parms.max_read_speed_kb);
		for (int i=0; i<speed_tbl_size && dev->parms.speed_tbl_kb[i] > 0; i++) {
			printf(SMEDIA "  RD speed #%02d: %4.1fX (%d kB/s)\n", i,
					((float)dev->parms.speed_tbl_kb[i]) / dev->parms.speed_mult,
					dev->parms.speed_tbl_kb[i]);
		}
		if (!(dev->media.type & ( DISC_CDROM | DISC_DDCD_ROM| DISC_DVDROM | DISC_BD_ROM | DISC_HDDVD_ROM))) {
			printf(SMEDIA "Write speeds:\n");
			printf(SMEDIA"D WR speed max: %4.1fX (%d kB/s)\n",
						((float)dev->parms.max_write_speed_kb) / dev->parms.speed_mult,
						dev->parms.max_write_speed_kb);
			for (int i=0; i<speed_tbl_size && dev->parms.wr_speed_tbl_kb[i] > 0; i++) {
				printf(SMEDIA "D WR speed #%02d: %4.1fX (%d kB/s)\n", i,
							((float)dev->parms.wr_speed_tbl_kb[i]) / ((float)dev->parms.speed_mult),
							dev->parms.wr_speed_tbl_kb[i]);
			}
		}
	}

	scanner = new qscanner(dev);
	if (pname) {
	//	scanner = new qscanner(dev);
		scanner->plugin_attach(pname);
	}

	if (!pname && ((test && (strcmp(test,"rt") && strcmp(test,"wt"))) || (flags & (FL_MINFO | FL_LPLUGIN))) ) {
		if (!dev->silent) printf( MSGPREF "creating scanner...\n");
	//	scanner = new qscanner(dev);
		if (scanner->plugins_probe(!test && (flags & FL_LPLUGIN), false) && !(flags & FL_LPLUGIN) ) {
			printf( MSGPREF "Device not found in any plugin support list, trying to probe...\n");
			if (scanner->plugins_probe(!test && (flags & FL_LPLUGIN), !(flags & FL_LPLUGIN))) {
				printf( MSGPREF "Probe failed! Trying fallback plugin...\n");
				if (!(flags & FL_LPLUGIN)) {
					if (scanner->plugin_attach_fallback()) {
						printf( MSGPREF "Error loading fallback plugin!\n");
					}
				}
			}
		}
		printf( MSGPREF "using plugin: %s\n", scanner->plugin_name() ? scanner->plugin_name() : "no plugin" );
	}

    if (flags & FL_MINFO) {
		char *mt;
		int i=0;
		if (!dev->media.type) {
			printf(IMEDIA "Media type    : No media\n");
			return 0;
		}

		while ( MEDIA[i].id != 0xFFFFFFFF && (dev->media.type & (~DISC_CDRWSUBT)) != MEDIA [i].id) i++;
		mt = (char*) MEDIA[i].name ;

		if (dev->media.type & DISC_CD) {
			printf(IMEDIA "Media class   : CD\n");
			printf(IMEDIA "Media type    : %s\n", mt);
			if (dev->media.type & DISC_CDR) {
				printf(IMEDIA "Disc Category : %s\n",
				cdr_subtype_tbl[ (dev->media.type >> 3) & 0x7]);
			}
			if (dev->media.type & DISC_CDRW) {
				printf(IMEDIA "Disc Category : %s\n",
				cdrw_subtype_tbl[ (dev->media.type >> 3) & 0x7]);
			}
		} else if (dev->media.type & DISC_DVD) {
			printf(IMEDIA "Media class   : DVD\n");
			printf(IMEDIA "Media type    : %s\n", mt);
			printf(IMEDIA "Disc Category : %s [rev %d]\n",
				book_type_tbl[ (dev->media.book_type >> 4) & 0xF],
				dev->media.book_type & 0xF);
			printf(IMEDIA "Media size    : %s\n",
				dev->media.disc_size ? "80mm" : "120mm");
			printf(IMEDIA "Maximum rate  : %s\n",
				max_rate_tbl[ dev->media.max_rate ]);

			if (!(dev->media.type & DISC_DVDRAM)) {
				get_rpc_state(dev);
				read_disc_regions(dev);
			}
		} else if (dev->media.type & DISC_BD) {
			printf(IMEDIA "Media class   : BD\n");
			printf(IMEDIA "Media type    : %s\n", mt);
			printf(IMEDIA "Media size    : %s\n",
				dev->media.disc_size ? "80mm" : "120mm");
			printf(IMEDIA "Polarity flags: %02x\n", dev->media.polarity);
			printf(IMEDIA "Disc Category : %s\n", dev->media.polarity ? "LTH" : "HTL");
		} else {
			printf(IMEDIA "Media type    : unknown\n");
			goto end;
		}

		if (scanner && scanner->is_plugin_attached()) {
			printf(IMEDIA "Available quality tests:%s%s%s%s%s%s\n",
				dev->media.capacity ? " rt" : "",
				dev->media.dstatus  ? "" : " wt",
				scanner->check_test(CHK_ERRC) ? "" : " errc",
				scanner->check_test(CHK_JB) ? "" : " jb",
				scanner->check_test(CHK_FETE) ? "" : " ft",
				scanner->check_test(CHK_TA) ? "" : " ta"
			);
			if (!scanner->check_test(CHK_ERRC)) {
				show_available_errc_data(scanner);

				int* sp = scanner->get_test_speeds(CHK_ERRC);
				if (!sp) sp = dev->parms.speed_tbl_kb;				
				printf(IMEDIA "ERRC speeds   :");

				for (int i=0; i<speed_tbl_size && sp[i] > 0; i++)
					printf(" %.0fX",((float)sp[i]) / dev->parms.speed_mult);

				printf("\n");
			}
			if (!scanner->check_test(CHK_JB)) {
				int* sp = scanner->get_test_speeds(CHK_JB);
				if (!sp) sp = dev->parms.speed_tbl_kb;				
				printf(IMEDIA "JB speeds     :");

				for (int i=0; i<speed_tbl_size && sp[i] > 0; i++)
					printf(" %.0fX",((float)sp[i]) / dev->parms.speed_mult);

				printf("\n");
			}
		}
		printf(IMEDIA "Layers        : %d\n", dev->media.layers);
	
		if ( dev->media.type & (DISC_DVD) ) {
//			read_disc_regions(drive);
			if (!dev->media.dvdcss.protection) {
				printf(IMEDIA "Protection    : none\n");
			} else {
				switch (dev->media.dvdcss.protection) {
					case 0x01:
						printf(IMEDIA "Protection    : CSS/CPPM\n");
						break;
					case 0x02:
						printf(IMEDIA "Protection    : CPRM\n");
						break;
					default:
						printf(IMEDIA "Protection    : Unknown\n");
						break;
				}
				printf(IMEDIA "Disc regions  : ");
				if (dev->media.dvdcss.regmask != 0xFF) {
					for (i=0; i<8; i++)
						if (!((dev->media.dvdcss.regmask >> i) & 1))
							{
								printf("%d",i+1); 
							//dev->rpc.region = i+1;
							}
					printf("\n");
				} else {
					printf("Invalid region mask!\n");
				}
			}
/*
		} else if ( dev->media.type & (DISC_BD) ) {
			printf(IMEDIA "Protection    : ??? (BD)\n");
		} else if ( dev->media.type & (DISC_HDDVD) ) {
			printf(IMEDIA "Protection    : ??? (HDDVD)\n");
*/
		}

		printf(IMEDIA "Erasable      : %s\n",dev->media.erasable ? "yes" : "no" );
		printf(IMEDIA "Disc state    : %s\n",disc_status_list[dev->media.dstatus]);
		printf(IMEDIA "Session state : %s\n",session_status_list[dev->media.sstatus]);
		printf(IMEDIA "Read capacity : %d sectors/%dMB/%02d:%02d.%02dMSF\n",
				dev->media.capacity,
				dev->media.capacity/512,
				dev->media.capacity/4500, ((dev->media.capacity)/75)%60, dev->media.capacity%75);
		printf(IMEDIA "Free capacity : %d sectors/%dMB/%02d:%02d.%02dMSF\n",
				dev->media.capacity_free,
				dev->media.capacity_free/512,
				dev->media.capacity_free/4500, ((dev->media.capacity_free)/75)%60, dev->media.capacity_free%75);
		printf(IMEDIA "Total capacity: %d sectors/%dMB/%02d:%02d.%02dMSF\n",
				dev->media.capacity_total,
				dev->media.capacity_total/512,
				dev->media.capacity_total/4500, ((dev->media.capacity_total)/75)%60, dev->media.capacity_total%75);

		if (dev->media.type & (DISC_DVDRAM | DISC_HDDVD_RAM) ) {
			printf(IMEDIA "DVD-RAM Spare Area information (free/total):\n");
			printf(IMEDIA "Primary SA    : %d (%dMB) / %d (%dMB)\n",
				dev->media.spare_psa_free,
				dev->media.spare_psa_free/512,
				dev->media.spare_psa_total,
				dev->media.spare_psa_total/512);
			printf(IMEDIA "Supplement. SA: %d (%dMB) / %d (%dMB)\n",
				dev->media.spare_ssa_free,
				dev->media.spare_ssa_free/512,
				dev->media.spare_ssa_total,
				dev->media.spare_ssa_total/512);
		}

		if ( !(dev->media.type & (DISC_CDROM | DISC_DVDROM)) )
			printf(IMEDIA "Media ID      : %s\n", dev->media.MID);
		if ( dev->media.type & (DISC_DVDminus) )
			printf(IMEDIA "Written on    : %s\n", dev->media.writer);
		if ((flags & FL_MID_RAW) && (dev->media.type & DISC_CD) && dev->media.ATIP_size) {
			for (int i=0; i<((dev->media.ATIP_size-4) >> 2); i++) {
				printf(IMEDIA "ATIP data %X   : ", i);
					printf(" %02X %02X %02X %02X",
						dev->media.ATIP[(i<<2)+4], dev->media.ATIP[(i<<2)+5], dev->media.ATIP[(i<<2)+6], dev->media.ATIP[(i<<2)+7]);
					printf(" | %3d %3d %3d %3d\n",
						dev->media.ATIP[(i<<2)+4], dev->media.ATIP[(i<<2)+5], dev->media.ATIP[(i<<2)+6], dev->media.ATIP[(i<<2)+7]);
			}
		}
		if (dev->media.MID_size) {
			if (dev->parms.wr_speed_tbl_media[0] > 0) {
				printf(SMEDIA "Manufacturer defined write speeds:\n");
				for (int i=0; i<speed_tbl_size && dev->parms.wr_speed_tbl_media[i] > 0; i++)
				//for (int i=0; i<speed_tbl_size; i++)
					printf(SMEDIA "M WR speed #%02d: %4.1fX\n", i, dev->parms.wr_speed_tbl_media[i]);
			}
			if (flags & FL_MID_RAW) {
				for (int i=0; i<(dev->media.MID_size >> 4); i++) {
					unsigned char c;
					printf(IMEDIA "MID raw data %X0: ", i);
					for (int ii=0; ii<16 && ( (i<<4) +ii) < dev->media.MID_size ; ii++) {
						printf("%02X ", (unsigned char) dev->media.MID_raw[(i<<4) + ii + 4]);
					}
					printf(" | ");
					for (int ii=0; ii<16 && ( (i<<4) +ii) < dev->media.MID_size ; ii++) {
						c = dev->media.MID_raw[(i<<4) + ii + 4];
						printf("%c", (c > 32) ? c : '.' );
					}
					printf("\n");
				}
			}
		}
		get_track_list(dev);
		printf(IMEDIA "Total tracks  : %d\n", dev->media.tracks);
		printf(IMEDIA "Track:     Session   TMode   DMode      Start        End       Size       Free   Next Writable\n");
		printf("--------------------------------------------------------------------------------------------------\n");
		if (dev->media.tracks) for (i=0; i<dev->media.tracks; i++) {
			printf(IMEDIA "Track# %2d       %2d      %2d      %2d   %8d   %8d   %8d   %8d   %8d\n",
				dev->media.track[i].n,
				dev->media.track[i].session,
				dev->media.track[i].track_mode,
				dev->media.track[i].data_mode,
				dev->media.track[i].start,
				dev->media.track[i].last,
				dev->media.track[i].size,
				dev->media.track[i].free,
				dev->media.track[i].next_writable);
		}
		printf("--------------------------------------------------------------------------------------------------\n\n");	
		goto end;
	}
	if (test && !strcmp(test,"wt")) {
		if (!dev->media.type) {
			printf("No media!\n");
			result=2;
			goto end;
		}
		if (dev->media.dstatus & !(dev->media.type & (DISC_DVDRAM | DISC_DVDpRW | DISC_DVDpRWDL))) {
			printf("Neither blank media nor DVD-RAM/DVD+RW found!\n");
			result=3;
			goto end;
		}
	}

//	if (test && (strcmp(test,"rt") && strcmp(test,"wt")) && !scanner->is_plugin_attached()) {
//	}

	if (speed>0)
		scanner->setTestSpeed(speed);
	if (test) {
//		printf("setting signal handlers...\n");
#if defined (__unix) || defined (__unix__)
		signal(SIGINT, &sigint_handler);
		signal(SIGUSR1, &sigusr_handler);
#ifdef USE_SIGUSR2
		signal(SIGUSR2, &sigusr_handler);
#endif
#elif defined (_WIN32)
		SetConsoleCtrlHandler(&sigint_handler, 1);
#endif

// starting test...
		//fcntl(1, F_SETFL, fcntl(1, F_GETFL) | O_DIRECT);
		//fcntl(2, F_SETFL, fcntl(2, F_GETFL) | O_DIRECT);
		if (!strcmp(test, "wt")) {
			if ((dev->media.dstatus || !dev->media.capacity_free) && !(dev->media.type & (DISC_DVDRAM| DISC_DVDpRW | DISC_DVDpRWDL))) {
//	sprintf(linef, "tsize=%dk", 2295104 * 2);
				printf("Media should be blank to run write transfer rate. Aborting...\n");
				result=4;
				goto end;
			}
			if (scanner->setTestWrite( simul )) {
				printf("Can't turn TestWrite %s\n", simul ? "ON" : "OFF");
				result=5;
				goto end;
//			} else {
//				printf("TestWrite set\n");
			}
		} else if (!strcmp(test, "errc")) {
			if (!scanner->check_test(CHK_ERRC)) {
				show_available_errc_data(scanner);
			}
		}
		result = scanner->run(test);
	} else {
		if (!(flags & FL_SPEED)) {
			printf( MSGPREF "no test selected!\n");
		} else if (rspeed > 0 || wspeed > 0) {
			//printf("1X speed mult: %.1f\n", dev->parms.speed_mult);
			get_rw_speeds(dev);
			if (rspeed>0) {
				dev->parms.read_speed_kb = (int) (rspeed * dev->parms.speed_mult);
				printf(IDEV "Setting read speed  : %2d X, %5d kB/s\n",
					rspeed,
					dev->parms.read_speed_kb);
			}
			if (dev->wr_capabilities && wspeed>0) {
				dev->parms.write_speed_kb = (int) (wspeed * dev->parms.speed_mult);
				printf(IDEV "Setting write speed : %2d X, %5d kB/s\n",
					wspeed,
					dev->parms.write_speed_kb);
			}
			set_rw_speeds(dev);
			get_rw_speeds(dev);
			if (rspeed>0) {
				printf(IDEV "Current read speed  : %4.1f X, %5d kB/s\n",
					dev->parms.read_speed_kb / dev->parms.speed_mult,
					dev->parms.read_speed_kb);
			}
			if (dev->wr_capabilities && wspeed>0) {
				printf(IDEV "Current write speed : %4.1f X, %5d kB/s\n",
					dev->parms.write_speed_kb / dev->parms.speed_mult,
					dev->parms.write_speed_kb);
			}
		}
	}
end:
	if (scanner) {
		if (!dev->silent) printf( MSGPREF "destroying scanner...\n");
		delete scanner;
	}
    if (!dev->silent) printf( MSGPREF "destroying dev...\n");
    delete dev;
    return result;
}
