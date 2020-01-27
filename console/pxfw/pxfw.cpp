/*
 * This file is a part of QPxTool project
 * Copyright (C) 2006-2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; see the file COPYING.  If not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

// const unsigned char op_blacklist[]={ 0xAC, 0xDE, 0xDF , 0xF8 };

const unsigned char op_blacklist[]={ 0xDE, 0xDF, 0xF8 };  // used in TEST mode
const int           op_blacklist_sz=sizeof(op_blacklist);

int op_blacklisted(unsigned char opcode){
    if (op_blacklist_sz) for (int i=0; i<op_blacklist_sz; i++) {
	if (op_blacklist[i] == opcode) return 1;	
    }
    return 0;
}

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include <qpx_mmc.h>
#include <plextor_features.h>

#include <math.h>

#include "pxfw.h"

#include "version.h"

long fsize(FILE* f){
	struct stat st;
	fstat(fileno(f),&st);
	return st.st_size;
}


int custom_command (drive_info* dev, unsigned char opcode) {
	dev->cmd[0]=opcode;
	if ((dev->err=dev->cmd.transport(NONE,NULL,0))) return dev->err;
	return 0;
}

void FW_convert_to_ID (int* dev_ID, char* buf, int* FWSZ_CRC) {
	*FWSZ_CRC=0;
	if (!strncmp(buf,"PLEXTOR CD-R   PX-W4824A",24)) {
		*dev_ID=PLEXTOR_4824;
		*FWSZ_CRC=0x79FFE;
	} else
	if (!strncmp(buf,"PLEXTOR CD-R   PX-W5224A",24)) {
		*dev_ID=PLEXTOR_5224;
		*FWSZ_CRC=0x79FFE;
	} else
	if (!strncmp(buf,"PLEXTOR CD-R   PREMIUM2",23)) {
		*dev_ID=PLEXTOR_PREMIUM2;
		*FWSZ_CRC=0xEFFFE;
	} else
	if (!strncmp(buf,"PLEXTOR CD-R   PREMIUM",22)) {
		*dev_ID=PLEXTOR_PREMIUM;
		*FWSZ_CRC=0x7C7FE;
	} else
	if (!strncmp(buf,"PLEXTOR DVDR   PX-708A",22)) {
		*dev_ID=PLEXTOR_708;
		*FWSZ_CRC=0xEFFFE;
	} else
	if (!strncmp(buf,"PLEXTOR DVDR   PX-712A",22)) {
		*dev_ID=PLEXTOR_712 | PLEXTOR_708A2;
		*FWSZ_CRC=0xEFFFE;
//		*FWSZ_CRC=0xFAD9E;
//		*FWSZ_CRC=0x0F0000;
	} else
	if (!strncmp(buf,"PLEXTOR DVDR   PX-716A ",23)) {
		*dev_ID=PLEXTOR_716 | PLEXTOR_714;
		*FWSZ_CRC=0xEFFFE;
	} else
	if (!strncmp(buf,"PLEXTOR DVDR   PX-716AL",23)) {
		*dev_ID=PLEXTOR_716AL;
		*FWSZ_CRC=0xEFFFE;
	} else
	if (!strncmp(buf,"PLEXTOR DVDR   PX-760A",22)) {
		*dev_ID=PLEXTOR_760 | PLEXTOR_755;
		*FWSZ_CRC=0x1EFFFE;
	} else
		*dev_ID = 0;
}

void PLEXTOR_convert_to_ID (drive_info* dev, int* FWSZ) {
	*FWSZ=0;
	if (!strncmp(dev->ven,"PLEXTOR ",8)) {
		dev->ven_ID=DEV_PLEXTOR;
		if(!strncmp(dev->dev,"CD-R   PX-W4824A",16)) {
			dev->dev_ID=PLEXTOR_4824;
			*FWSZ=524288;
		} else
		if(!strncmp(dev->dev,"CD-R   PX-W5224A",16)) {
			dev->dev_ID=PLEXTOR_5224;
			*FWSZ=524288;
		} else
		if(!strncmp(dev->dev,"CD-R   PREMIUM ",15)) {
			dev->dev_ID=PLEXTOR_PREMIUM;
			*FWSZ=524288;
		} else
		if(!strncmp(dev->dev,"CD-R   PREMIUM2",15)) {
			dev->dev_ID=PLEXTOR_PREMIUM2;
			*FWSZ=983040;
		} else
		if(!strncmp(dev->dev,"DVDR   PX-708A ",15)) {
			dev->dev_ID=PLEXTOR_708;
			*FWSZ=983040;
		} else
		if(!strncmp(dev->dev,"DVDR   PX-708A2",15)) {
			dev->dev_ID=PLEXTOR_708A2;
			*FWSZ=1028096;
		} else
		if(!strncmp(dev->dev,"DVDR   PX-712A",14)) {
			dev->dev_ID=PLEXTOR_712;
			*FWSZ=1028096;
		} else
		if(!strncmp(dev->dev,"DVDR   PX-714A",14)) {
			dev->dev_ID=PLEXTOR_714;
			*FWSZ=983040;
		} else
		if(!strncmp(dev->dev,"DVDR   PX-716A ",15)) {
			dev->dev_ID=PLEXTOR_716;
			*FWSZ=983040;
		} else
		if(!strncmp(dev->dev,"DVDR   PX-716AL",15)) {
			dev->dev_ID=PLEXTOR_716AL;
			*FWSZ=983040;
		} else
		if(!strncmp(dev->dev,"DVDR   PX-755A",14)) {
			dev->dev_ID=PLEXTOR_755;
			*FWSZ=2031616;
		} else
		if(!strncmp(dev->dev,"DVDR   PX-760A",14)) {
			dev->dev_ID=PLEXTOR_760;
			*FWSZ=2031616;
		} else
			dev->dev_ID=PLEXTOR_OLD;
	}
}

/*
int plextor_read_eeprom(drive_info* dev, unsigned char idx, unsigned int sz) {
//	char*		data;
	unsigned int	i,j;
	int		offs=idx*sz;
	unsigned char*  buf=dev->rd_buf+offs;

	dev->cmd[0] = 0xF1;
	dev->cmd[1] = 0x01;
	dev->cmd[7] = idx;
	dev->cmd[8] = (sz >> 8) & 0xFF;
	dev->cmd[9] = sz & 0xFF;
	if ((dev->err=dev->cmd.transport(READ,buf,sz) ))
		{ sperror ("read EEPROM",dev->err); 	return (0); }

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
	return 1;
}
*/

int fwblk_send(drive_info* dev, int offs, int blksz, bool last) {
	dev->cmd[0] = 0x3B;
	dev->cmd[1] = last ? 0x05:0x04;
	dev->cmd[2] = 0x00;
	dev->cmd[3] = (offs >> 16) & 0xFF;
	dev->cmd[4] = (offs >> 8) & 0xFF;
	dev->cmd[5] = offs & 0xFF;
	dev->cmd[6] = (blksz >> 16) & 0xFF;
	dev->cmd[7] = (blksz >> 8) & 0xFF;
	dev->cmd[8] = blksz & 0xFF;
	dev->cmd[9] = 0x00;
	dev->cmd[10]= 0x00;
	dev->cmd[11]= 0x00;

	if ((dev->err=dev->cmd.transport(WRITE,dev->rd_buf,blksz) )){
		sperror ("SEND_FWBLK",dev->err);
		if (!last) return 1;
	}
	return 0;
}


int fwblk_read(drive_info* dev, int offs, int blksz, unsigned char flag) {
        printf("data read...\n");
	return 1;
}

/*
	dev->cmd[0] = 0x3B;
//	cmd[1] = last ? 0x05:0x04;
	dev->cmd[1] = flag;

	dev->cmd[2] = 0x00;
	dev->cmd[3] = (offs >> 16) & 0xFF;
	dev->cmd[4] = (offs >> 8) & 0xFF;
	dev->cmd[5] = offs & 0xFF;
	dev->cmd[6] = (blksz >> 16) & 0xFF;
	dev->cmd[7] = (blksz >> 8) & 0xFF;
	dev->cmd[8] = blksz & 0xFF;
	dev->cmd[9] = 0x00;
	dev->cmd[10]= 0x00;
	dev->cmd[11]= 0x00;

	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,blksz) )){
		sperror ("READ_FWBLK",dev->err);
		return 1;
	}
	return 0;
}
*/

void csum_init(unsigned short int *csum, int FW_dev_ID){
    *csum = 0;
    if (FW_dev_ID <= PLEXTOR_PREMIUM) *csum = 0x8000;
}

void csum_update(unsigned short int *csum, unsigned char* buf, int len) {
	int		i;
	unsigned int	ad;
	for (i=0; i<len; i++)
		{ ad = buf[i]; *csum += ad; }
}

void usage(char* bin) {
	fprintf (stderr,"\nusage: %s [-d device] [options]\n",bin);
	printf("\t-h\t\t show help\n");
	printf("\t-v\t\t debug\n");
	printf("\t-l\t\t scan IDE/SCSI bus\n");
	printf("\t-u\t\t write FW\n");
	printf("\t-if [file]\t read FW from this file\n");
//	printf("\t-of [file]\t save FW to this file\n");
	printf("\t-f\t\t force flashing, even if device not recognized or checksum error\n");
	printf("\t-e\t\t read EEPROM\n");
//	printf("\t-ie [file]\t load EEPROM from this file\n");
	printf("\t-oe [file]\t save EEPROM to this file\n");
	printf("\t-r\t\t reset device\n");
	printf("\t-t\t\t test opcodes supported by devive (EXPERIMENTAL),\n");
	printf("\t\t\t use it only if you know what are you doing!\n");
}

int main(int argc,char *argv[]) {
	drive_info  *dev=0;
	char        *devname=0;
	char	    *fwfname=0;
	char	    *eefname=0;
	FILE	    *fwfile;
	FILE	    *eefile;

	int i,j;
//    char 	wrinfo[0x3F];

	int	FW_dev_ID=0;
//	int	cap;

//	int	blen;
	int	fwblk; // = 4096;
	int	fwblk_crc = 2048; //= fwblk/2;
	int	fwblocks;
	int	fwblocks_crc;
	int	fwsz;
	int	crc_offs;
	int	fwfsz;
	int	err;

	int 	last;
	int	flags=0;
	unsigned short int fCSUM=0;
	unsigned short int CSUM=0;
	unsigned short int CSUM_diff=0;

//	int crca=0;

	printf("**  Plextor FirmWare updater v%s (c) 2006-2009  Gennady \"ShultZ\" Kozlov **\n",VERSION);
/*
	if (argc<2) {
		usage(argv[0]);
		exit (2);
	}
*/
//	flags = FL_UPDATE;
	if (argc>1) {
//		printf("Parsing additional options...\n");
		for (i=1; i<argc; i++) {
//			printf("arg[%d]: %s\n",i,argv[i]);

			if(!strcmp(argv[i],"-d")) {
				if(argc>(i+1)) {
					i++;
					flags |= FL_DEV;
					devname = argv[i];
				} else {
					printf("Option %s needs parameter\n",argv[i]);
					exit (1);
				}
			}
			else if(!strcmp(argv[i],"-if")) {
				if(argc>(i+1)) {
					i++;
					flags |= FL_FWIF;
					fwfname = argv[i];
				} else {
					printf("Option %s needs parameter\n",argv[i]);
					exit (1);
				}
			}
			else if(!strcmp(argv[i],"-oe")) {
				if(argc>(i+1)) {
					i++;
					flags |= FL_EEOF;
					eefname = argv[i];
				} else {
					printf("Option %s needs parameter\n",argv[i]);
					exit (1);
				}
			}
			else if(!strcmp(argv[i],"-h")) flags |= FL_HELP;
			else if(!strcmp(argv[i],"-l")) flags |= FL_SCAN;
			else if(!strcmp(argv[i],"-u")) flags |= FL_UPDATE;
//			else if(!strcmp(argv[i],"-b")) flags |= FL_BACKUP;
			else if(!strcmp(argv[i],"-v")) flags |= FL_DEBUG;
			else if(!strcmp(argv[i],"-t")) flags |= FL_TEST;
			else if(!strcmp(argv[i],"-e")) flags |= FL_EEPROM;
			else if(!strcmp(argv[i],"-r")) flags |= FL_RESET;
			else if(!strcmp(argv[i],"-f")) flags |= FL_FORCE;
			else {
				printf("Illegal option: %s.\nUse -h for details\n",argv[i]);
//				usage(argv[0]);
				exit (1);
			}
		}
	}
	if (flags & FL_DEBUG) {
		printf("Flags: ");
		if (flags & FL_DEV)    printf(" DEV");
		if (flags & FL_FWIF)   printf(" FWIF");
		if (flags & FL_HELP)   printf(" HELP");
		if (flags & FL_UPDATE) printf(" UPDATE");
		if (flags & FL_BACKUP) printf(" BACKUP");
		if (flags & FL_DEBUG)  printf(" DEBUG");
		if (flags & FL_TEST)   printf(" TEST");
		if (flags & FL_EEPROM) printf(" EEPROM");
		if (flags & FL_RESET)  printf(" RESET");
	}
	if (flags & FL_HELP) {
		usage(argv[0]);
		exit (0);
	}
	if (flags & FL_SCAN) {
		scanbus();
		exit (0);
	}
	if (!(flags & FL_DEV)) {
		printf("\n*** No device selected! ***\n\n");
		usage(argv[0]);
		scanbus();
		exit (3);
	}

	dev = new drive_info(devname);
	if (dev->err) {
		printf("%s: can't open device: %s\n", argv[0], devname);
		delete dev;
		return 4;
	}


	printf("\nDevice : %s\n",devname);
	inquiry(dev);
	PLEXTOR_convert_to_ID(dev,&fwsz);
//	dev->silent++;
//	detect_mm_capabilities(dev);
//	dev->silent--;

	printf("Vendor : '%s'\n",dev->ven); 
	printf("Model  : '%s'", dev->dev);
	if ( isPlextor(dev) ) {
//	if ( dev->ven_ID == DEV_PLEXTOR ) {
		plextor_get_TLA(dev);
		printf(" (TLA#%s)",dev->TLA);
	}
	printf("\nF/W    : '%s'\n",dev->fw);
	if (get_drive_serial_number(dev))
		printf("Serial#: %s\n",dev->serial);
	if ((dev->ven_ID != DEV_PLEXTOR) && (!(flags & FL_FORCE))){
		printf("%s: Only PLEXTOR drives supported!\n",argv[0]);
		return 1;
	} else {
		if ((dev->dev_ID == PLEXTOR_OLD)){
			printf("%s: Not supported PLEXTOR drive!\n",argv[0]);
			return 1;
		}
	}
//	plextor_get_life(dev);
	if (flags & FL_RESET) {
		plextor_reboot (dev);
		exit (0);
	}

	if (flags & FL_EEPROM) {
		int eelen;
//		unsigned char buf[1024];
		plextor_read_eeprom(dev, &eelen);
		if (flags & FL_EEOF) {
		    eefile = fopen(eefname,"w");
		    if (!eefile) {
			printf("%s: Can't write file: %s\n",argv[0],eefname);
			exit (2);
		    }
		    fwrite(dev->rd_buf, eelen, 1, eefile);
		    fclose(eefile);
		}
		exit (0);
	}
	
	if (flags & FL_TEST) {
		for (unsigned int opcode=0x0000; opcode<0x0100; opcode++) {
			printf("Trying OpCode: ");
		        print_opcode((unsigned char)opcode);			
			if (!op_blacklisted((unsigned char)opcode)) {			
			        int err = custom_command(dev, (opcode & 0x00FF));
				if (!err) printf ("OK");
				else if (err==-1) printf ("command didn't passed to dev");
				else if (err==0x52000) printf ("*** command not supported ***");
				else print_sense(err);
			} else {
				printf("!!! BLACK LIST !!!");
			}
			printf("\n");
		}
		exit(0);
	}

	if (!(flags & FL_FWIF)) exit(3);

	fwfile = fopen(fwfname, "r");
	if (!fwfile) {
		printf("%s: Can't open file: %s\n",argv[0],fwfname);
		return 1;
	}

//	fwblk=4096;
	const int FB=64;
	const int HH=16;
	fwfsz=fsize(fwfile);
	if (!fread(dev->rd_buf, FB, 1, fwfile)) {
		printf("%s: error reading file: %s\n", argv[0], fwfname);
		return 1;		
	}

	printf("this dev FW size : %7d (%06X)\n",fwsz,fwsz);
	printf("FW file size       : %7d (%06X)\n",fwfsz,fwfsz);
	printf("First %d bytes of FW:\n", FB);

	for (i=0; i<(FB/HH); i++){
		printf("| ");
		for (j=0; j<HH; j++) printf("%02X ",dev->rd_buf[i*HH+j] & 0xFF);
		printf(" | ");
		for (j=0; j<HH; j++)
			if ((dev->rd_buf[i*HH+j] & 0xFF) > 0x20)
				printf("%c",dev->rd_buf[i*HH+j] & 0xFF);
			else
				printf(" ");
		printf(" |\n");
	}
	FW_convert_to_ID(&FW_dev_ID,(char*) dev->rd_buf,&crc_offs);
//	if (FW_dev_ID & (PLEXTOR_PREMIUM | PLEXTOR_PREMIUM2))
	if (FW_dev_ID & (PLEXTOR_5224 | PLEXTOR_4824))
		fwblk = 16384;
	else
		fwblk = 4096;
	if (flags & FL_BACKUP) {
		printf("BackUp feature not implemented yet...\n");
		return 1;
		
		
		for (i=0; i<fwblk; i++) dev->rd_buf[i]=0;
		i=0;
		if ((err = fwblk_read(dev, i*fwblk, fwblk, 2))) {
			printf("** error reading data\n");
		};

		printf("First %d bytes of FW:\n", FB);
		for (i=0; i<(FB/HH); i++){
			printf("| ");
			for (j=0; j<HH; j++) printf("%02X ", dev->rd_buf[i*HH+j] & 0xFF);
			printf(" | ");
			for (j=0; j<HH; j++)
				if ((dev->rd_buf[i*HH+j] & 0xFF) > 0x20)
					printf("%c", dev->rd_buf[i*HH+j] & 0xFF);
				else
					printf(" ");
			printf(" |\n");
		}
	}

	fseek(fwfile,0,SEEK_SET);
	last=0;
//	printf("fwblocks = %d\n",fwblocks);
	fwblocks=fwfsz/fwblk;
	fwblocks_crc=(crc_offs+2)/fwblk_crc;

#if 0
	csum_init(&CSUM, FW_dev_ID);
	for (i=0;i<fwblocks_crc;i++) {
//		fseek(fwfile,i*fwblk,SEEK_SET);
		fread(dev->rd_buf, fwblk_crc, 1, fwfile);
		if (i == (fwblocks_crc-1)) last=1;
		if (last) {
			blen = fwblk_crc - 2;
			crca = i*fwblk_crc+blen;
			fCSUM = (dev->rd_buf[fwblk_crc-2] << 8) | dev->rd_buf[fwblk_crc-1];
			printf("blk offs: %06X, last: %06X\n", fwblk_crc*i, fwblk_crc*i+blen);
		} else blen = fwblk_crc;
//		printf("Offset %04X, block # %X\n",i*fwblk,i);
//		printf ("%02X:  %4dB CRC=%04X\n",i,blen,CRC16blk);
		csum_update(&CSUM, dev->rd_buf, blen);
	}
#else
	unsigned char *fw = (unsigned char*) malloc(fwfsz);
	printf("FW buffer @%p\n", fw);
	if (!fread(fw, fwfsz, 1, fwfile)) {
		printf("%s: error reading file: %s\n", argv[0], fwfname);
		return 1;
	}

/*
	for (int of=2; of<fwfsz; of+=2) {
		csum_init(&CSUM, FW_dev_ID);
		csum_update(&CSUM, fw, of);
		fCSUM = (fw[of] << 8) | fw[of+1];
		if(CSUM == fCSUM)  printf("*  %06X\n", of);
		if (!(of & 0xFFFF)) printf("   %06X\n", of);
	}
*/



	csum_init(&CSUM, FW_dev_ID);
	csum_update(&CSUM, fw, crc_offs);
	fCSUM = (fw[crc_offs] << 8) | fw[crc_offs+1];
	free(fw);
#endif
//	fCRC = (FWBUF[fwblk-2] << 8) | FWBUF[fwblk-1];
	CSUM_diff = (CSUM  - fCSUM) & 0xFFFF;
	printf("CheckSum:\n");
	//printf("Stored [@%06X]: %04X\n",crca,fCSUM);
	printf("Stored [@%06X]: %04X\n",crc_offs,fCSUM);
	printf("Calculated      : %04X\n",CSUM);
	printf("diff            : %04X\n",CSUM_diff);
	if (CSUM_diff) printf("*** CheckSum incorrect! ***\n");

	if (!(dev->dev_ID & FW_dev_ID)) {
		printf("FW is not for selected dev!\n");
		fclose(fwfile);
		exit(4);
	}

	if (flags & FL_UPDATE) {
		if (fwfsz!=fwsz) {
			printf("*** File size does not match FW size! ***\n");
			delete dev;
			fclose(fwfile);
	        	exit(3);
		}
		if (CSUM_diff) {
			if (!(flags & FL_FORCE)) {
				printf("*** CheckSum incorrect, you can try -f option AT YOUR RISC to Force flashing ...\n");
				delete dev;
				fclose(fwfile);
				exit(3);
			} else {
				printf("*** CheckSum incorrect, but -f option found. Force flashing...\n");
			}
		}
		determine_disc_type(dev);
//		printf("Disc type: %02X\n",dev->media.type);
		if (dev->media.type > 1) {
			printf("Disc found, doing eject...\n");
			if (load_eject(dev, false, false)) {
				printf("Can't eject disc:( remove disc manually and try again\n");
				delete dev;
				fclose(fwfile);
				exit (1);
			}
	        }
		printf("Waiting for dev to become ready... ");
		if (!wait_unit_ready(dev,2,0)) {
		    printf(" OK!\n");
		} else {
		    printf("\nDrive not ready! Aborting...\n");
		    delete dev;
		    fclose(fwfile);
		    exit(1);
		}
		last=0;
		printf("Sending FirmWare to dev, %d bytes per block\n", fwblk);
		fseek(fwfile,0,SEEK_SET);
//		for (i=0;i<(fwblocks);i++) {
		for (i=0;!last;i++) {
			if (!fread(dev->rd_buf, fwblk, 1, fwfile)) {
				printf("%s: error reading file: %s\n", argv[0], fwfname);
				return 1;
			}
//			printf("Block #%d:\n",i);
			if (feof(fwfile) || (i == (fwblocks-1) )) {
				last=1;
				printf("\nData transfer complete: %d bytes (%X blocks). Updating...\n", (i+1)*fwblk, i+1);
			}
			err = fwblk_send(dev, i*fwblk, fwblk, last);
			if (err) {
			    printf("FW UPDATE ERROR!\n");
			    last=1;
			}
		}
		inquiry(dev);
		printf("FW update complete! New INQUIRY data:\n");
		printf("Vendor : '%s'\n",dev->ven); 
		printf("Model  : '%s'\n",dev->dev);
		printf("F/W    : '%s'\n",dev->fw);
	}
	printf("\n");
	delete dev;
	fclose(fwfile);
	return 0;
}
