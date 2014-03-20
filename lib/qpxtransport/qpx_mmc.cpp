/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2012 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * original version of CD-R(W) manufacturer identification code got from cdrecord, (C) Joerg Schilling
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
//#include <sys/time.h>

#include "qpx_mmc.h"
#include "colors.h"

#define SPINUP_REVERSE
//#define MODE_PAGES_DEBUG
//#define FEATURE_DEBUG

#ifndef DEVTBL_DIRECT
#include <dirent.h>
#endif

int convert_to_ID (drive_info* drive);

drive_info::drive_info(const char* _device){
	device=(char*)malloc(bufsz_dev);
	strcpy(device,_device);
	rd_buf=(unsigned char*)malloc(bufsz_rd);
	if (!cmd.associate(device, NULL)) {
//		printf("** Can't open device: %16s\n",_device);
		err=1;
		mmc=-1;
		return;
	}
	mmc=0;
	life.ok=0;
	parms.interval=1;
	parms.tests=0;
	capabilities=0;
	rd_capabilities=0;
	wr_capabilities=0;
	wr_modes=0;
	get_performance_fail=0;
	ven_features=0;
	chk_features=0;
	plextor.gigarec=0;
	plextor.varirec_state_cd=0;
	plextor.varirec_pwr_cd=0;
	plextor.varirec_str_cd=0;
	plextor.varirec_state_dvd=0;
	plextor.varirec_pwr_dvd=0;
	plextor.varirec_str_dvd=0;
	plextor.powerec_state=0;
	plextor.plexeraser=0;
	ven_ID=0;
	dev_ID=0;
	iface_id=0;
	iface[0]=0;
	loader_id=0;
	parms.scan_speed_cd=8;
	parms.scan_speed_dvd=5;
	parms.spindown_idx=0;
	parms.speed_mult=176;
	silent=0;
	rpc.phase=0;
	rpc.region=0;

	plextor_silent.pstate = 0;
	plextor_silent.prd_cd = 0;
	plextor_silent.pwr_cd = 0;
	plextor_silent.prd_dvd = 0;
//	plextor_silent.pwr_dvd = 0;
	plextor_silent.paccess = 0;
	plextor_silent.peject = 0;
	plextor_silent.pload = 0;

	media.MID_type = MID_type_NONE;
	media.MID_size = 0;

	media.dvdcss.protection=0;
	media.dvdcss.p_titles=NULL;
#if (DVDCSS_KEY_CACHE > 0)
	media.dvdcss.psz_cachefile[0]=0;
	media.dvdcss.psz_block=0;
#endif
}


drive_info::~drive_info(){
//	delete urd_buf;
	busy=1;
//	delete pthread_t;
	free(rd_buf);
	free(device);
}


/*
bool drive_info::lock(){
	if (!busy) {busy=true; return true; }
	else { return false; }
}

bool drive_info::unlock(){
	busy=false;
	return true;
}

bool drive_info::isBusy(){
	return busy;
}

void drive_info::wait_free(){
	while (busy);
}
*/

int print_sense (int err) {
	char str[128];
	strcpy(str,"[unknown error]");
	switch (SK(err)) {
	case 0x1:
		switch (ASC(err)) {
		case 0x0B:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"WARNING"); break;
        		case 0x01: strcpy(str,"WARNING - SPECIFIED TEMPERATURE EXCEEDED"); break;
        		case 0x02: strcpy(str,"WARNING - ENCLOSURE DEGRADED"); break;

			default:  sprintf(str,"WARNING, ASCQ=%02X",ASCQ(err)); break;
			}
			break;
		case 0x17:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"RECOVERED DATA WITH NO ERROR CORRECTION APPLIED"); break;
			case 0x01: strcpy(str,"RECOVERED DATA WITH RETRIES"); break;
			case 0x02: strcpy(str,"RECOVERED DATA WITH POSITIVE HEAD OFFSET"); break;
			case 0x03: strcpy(str,"RECOVERED DATA WITH NEGATIVE HEAD OFFSET"); break;
			case 0x04: strcpy(str,"RECOVERED DATA WITH RETRIES AND/OR CIRC APPLIED"); break;
			case 0x05: strcpy(str,"RECOVERED DATA USING PREVIOUS SECTOR ID"); break;
			case 0x07: strcpy(str,"RECOVERED DATA WITHOUT ECC - RECOMMEND REASSIGNMENT"); break;
			case 0x08: strcpy(str,"RECOVERED DATA WITHOUT ECC - RECOMMEND REWRITE"); break;
			case 0x09: strcpy(str,"RECOVERED DATA WITHOUT ECC - DATA REWRITTEN"); break;

			default:   strcpy(str,"RECOVERED DATA WITH NO ERROR CORRECTION APPLIED"); break;
			}
			break;
		case 0x18:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"RECOVERED DATA WITH ERROR CORRECTION APPLIED"); break;
			case 0x01: strcpy(str,"RECOVERED DATA WITH ERROR CORR. & RETRIES APPLIED"); break;
			case 0x02: strcpy(str,"RECOVERED DATA - DATA AUTO-REALLOCATED"); break;
			case 0x03: strcpy(str,"RECOVERED DATA WITH CIRC"); break;
			case 0x04: strcpy(str,"RECOVERED DATA WITH L-EC"); break;
			case 0x05: strcpy(str,"RECOVERED DATA - RECOMMEND REASSIGNMENT"); break;
			case 0x06: strcpy(str,"RECOVERED DATA - RECOMMEND REWRITE"); break;
			case 0x08: strcpy(str,"RECOVERED DATA WITH LINKING"); break;

			default:   strcpy(str,"RECOVERED DATA WITH ERROR CORRECTION APPLIED"); break;
			}
			break;
		case 0x5D:
			switch (ASCQ(err)) {
			case 0x01: strcpy(str,"FAILURE PREDICTION THRESHOLD EXCEEDED - Predicted Media failure"); break;
			case 0x02: strcpy(str,"LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED"); break;
			case 0x03: strcpy(str,"FAILURE PREDICTION THRESHOLD EXCEEDED - Predicted Spare Area Exhaustion"); break;
			case 0xFF: strcpy(str,"FAILURE PREDICTION THRESHOLD EXCEEDED (FALSE)"); break;

			default:   strcpy(str,"LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED"); break;
			}
			break;
		case 0x73:
			switch (ASCQ(err)) {
			case 0x01: strcpy(str,"POWER CALIBRATION AREA ALMOST FULL"); break;
			case 0x06: strcpy(str,"RMA/PMA IS ALMOST FULL"); break;
			}
			break;
		}
	case 0x2:
		switch (ASC(err)) {
		case 0x04:
			switch (ASCQ(err)) {
        		case 0x00: strcpy(str,"LOGICAL UNIT NOT READY, CAUSE NOT REPORTABLE"); break;
        		case 0x01: strcpy(str,"LOGICAL UNIT IS IN PROCESS OF BECOMING READY"); break;
        		case 0x02: strcpy(str,"LOGICAL UNIT NOT READY, INITIALIZING CMD. REQUIRED"); break;
        		case 0x03: strcpy(str,"LOGICAL UNIT NOT READY, MANUAL INTERVENTION REQUIRED"); break;
        		case 0x04: strcpy(str,"LOGICAL UNIT NOT READY, FORMAT IN PROGRESS"); break;
        		case 0x07: strcpy(str,"LOGICAL UNIT NOT READY, OPERATION IN PROGRESS"); break;
        		case 0x08: strcpy(str,"LOGICAL UNIT NOT READY, LONG WRITE IN PROGRESS"); break;

        		default:   strcpy(str,"LOGICAL UNIT NOT READY, CAUSE NOT REPORTABLE"); break;
			}
			break;
		case 0x30:
			switch (ASCQ(err)) {
       			case 0x00: strcpy(str,"INCOMPATIBLE MEDIUM INSTALLED"); break;
       			case 0x01: strcpy(str,"CANNOT READ MEDIUM - UNKNOWN FORMAT"); break;
       			case 0x02: strcpy(str,"CANNOT READ MEDIUM - INCOMPATIBLE FORMAT"); break;
       			case 0x03: strcpy(str,"CLEANING CARTRIDGE INSTALLED"); break;
       			case 0x04: strcpy(str,"CANNOT WRITE MEDIUM - UNKNOWN FORMAT"); break;
       			case 0x05: strcpy(str,"CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT"); break;
       			case 0x06: strcpy(str,"CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM"); break;
       			case 0x07: strcpy(str,"CLEANING FAILURE"); break;
       			case 0x11: strcpy(str,"CANNOT WRITE MEDIUM - UNSUPPORTED MEDIUM VERSION"); break;

       			default:   strcpy(str,"INCOMPATIBLE MEDIUM INSTALLED"); break;
			}
			break;
		case 0x3A:
			switch (ASCQ(err)) {
       			case 0x00: strcpy(str,"MEDIUM NOT PRESENT"); break;
       			case 0x01: strcpy(str,"MEDIUM NOT PRESENT - TRAY CLOSED"); break;
       			case 0x02: strcpy(str,"MEDIUM NOT PRESENT - TRAY OPEN"); break;

       			default:   strcpy(str,"MEDIUM NOT PRESENT"); break;
			}
			break;
		case 0x3E: strcpy(str,"LOGICAL UNIT HAS NOT SELF-CONFIGURED YET"); break; /* ASCQ=00: */
		}
		break;
	case 0x3:
		switch (ASC(err)) {
		case 0x02: strcpy(str,"NO SEEK COMPLETE"); break; /* ASCQ = 0x00 */
		case 0x06: strcpy(str,"NO REFERENCE POSITION FOUND"); break;
		case 0x0C:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"WRITE ERROR"); break;
			case 0x07: strcpy(str,"WRITE ERROR - RECOVERY NEEDED"); break;
			case 0x08: strcpy(str,"WRITE ERROR - RECOVERY FAILED"); break;
			case 0x09: strcpy(str,"WRITE ERROR - LOSS OF STREAMING"); break;
			case 0x0A: strcpy(str,"WRITE ERROR - PADDING BLOCKS ADDED"); break;

			default:   strcpy(str,"WRITE ERROR"); break;
			}
			break;
		case 0x11:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"UNRECOVERED READ ERROR"); break;
			case 0x01: strcpy(str,"READ RETRIES EXHAUSTED"); break;
			case 0x02: strcpy(str,"ERROR TOO LONG TO CORRECT"); break;
			case 0x05: strcpy(str,"L-EC UNCORRECTABLE ERROR"); break;
			case 0x06: strcpy(str,"CIRC UNRECOVERED ERROR"); break;
			case 0x0F: strcpy(str,"ERROR READING UPC/EAN NUMBER"); break;
			case 0x10: strcpy(str,"ERROR READING ISRC NUMBER"); break;

			default:   strcpy(str,"UNRECOVERED READ ERROR"); break;
			}
			break;
		case 0x15:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"RANDOM POSITIONING ERROR"); break;
			case 0x01: strcpy(str,"MECHANICAL POSITIONING ERROR"); break;
			case 0x02: strcpy(str,"POSITIONING ERROR DETECTED BY READ OF MEDIUM"); break;

			default: strcpy(str,"RANDOM POSITIONING ERROR"); break;
			}
			break;
		case 0x31:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"MEDIUM FORMAT CORRUPTED"); break;
			case 0x01: strcpy(str,"FORMAT COMMAND FAILED"); break;
			case 0x02: strcpy(str,"ZONED FORMATTING FAILED DUE TO SPARE LINKING"); break;

			default:   strcpy(str,"MEDIUM FORMAT CORRUPTED"); break;
			}
			break;
		case 0x51:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"ERASE FAILURE"); break;
			case 0x01: strcpy(str,"ERASE FAILURE - INCOMPLETE ERASE OPERATION DETECTED"); break;

			default:   strcpy(str,"ERASE FAILURE"); break;
			}
			break;
		case 0x57: strcpy(str,"UNABLE TO RECOVER TABLE-OF-CONTENTS"); break; /* ASCQ = 00 */
		case 0x72:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"SESSION FIXATION ERROR"); break;
			case 0x01: strcpy(str,"SESSION FIXATION ERROR WRITING LEAD-IN"); break;
			case 0x02: strcpy(str,"SESSION FIXATION ERROR WRITING LEAD-OUT"); break;

			default:   strcpy(str,"SESSION FIXATION ERROR"); break;
			}
			break;
		case 0x73:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"CD CONTROL ERROR"); break;
			case 0x02: strcpy(str,"POWER CALIBRATION AREA IS FULL"); break;
			case 0x03: strcpy(str,"POWER CALIBRATION AREA ERROR"); break;
			case 0x04: strcpy(str,"PROGRAM MEMORY AREA UPDATE FAILURE"); break;
			case 0x05: strcpy(str,"PROGRAM MEMORY AREA IS FULL"); break;

			default:   strcpy(str,"CD CONTROL ERROR"); break;
			}
			break;
		}
		break;
	case 0x4:
		switch (ASC(err)) {
		case 0x00: strcpy(str,"CLEANING REQUESTED"); break;  /* ASCQ = 0x17 */
		case 0x05: strcpy(str,"LOGICAL UNIT DOES NOT RESPOND TO SELECTION"); break; /* ASCQ = 0x00 */
		case 0x08:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"LOGICAL UNIT COMMUNICATION FAILURE"); break;
			case 0x01: strcpy(str,"LOGICAL UNIT COMMUNICATION TIMEOUT"); break;
			case 0x02: strcpy(str,"LOGICAL UNIT COMMUNICATION PARITY ERROR"); break;
			case 0x03: strcpy(str,"LOGICAL UNIT COMMUNICATION CRC ERROR (ULTRA-DMA/32)"); break;
			}
			break;
		case 0x09:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"TRACK FOLLOWING ERROR"); break;
			case 0x01: strcpy(str,"TRACKING SERVO FAILURE"); break;
			case 0x02: strcpy(str,"FOCUS SERVO FAILURE"); break;
			case 0x03: strcpy(str,"SPINDLE SERVO FAILURE"); break;
			case 0x04: strcpy(str,"HEAD SELECT FAULT"); break;

			default:   strcpy(str,"TRACKING ERROR"); break;
			}
			break;
		case 0x15:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"RANDOM POSITIONING ERROR"); break;
			case 0x01: strcpy(str,"MECHANICAL POSITIONING ERROR"); break;

			default:   strcpy(str,"RANDOM POSITIONING ERROR"); break;
			}
			break;
		case 0x1B: strcpy(str,"SYNCHRONOUS DATA TRANSFER ERROR"); break; /* ASCQ = 0x00 */
		case 0x3B: strcpy(str,"MECHANICAL POSITIONING OR CHANGER ERROR"); break; /* ASCQ = 0x16 */
		case 0x3E:
			switch (ASCQ(err)) {
			case 0x01: strcpy(str,"LOGICAL UNIT FAILURE"); break;
			case 0x02: strcpy(str,"TIMEOUT ON LOGICAL UNIT"); break;

			default:   strcpy(str,"LOGICAL UNIT FAILURE"); break;
			}
			break;
		case 0x40: strcpy(str,"DIAGNOSTIC FAILURE ON COMPONENT NN (80H-FFH)"); break;
		case 0x44: strcpy(str,"INTERNAL TARGET FAILURE"); break;
		case 0x46: strcpy(str,"UNSUCCESSFUL SOFT RESET"); break;
		case 0x47: strcpy(str,"SCSI PARITY ERROR"); break;
		case 0x4A: strcpy(str,"COMMAND PHASE ERROR"); break;
		case 0x4B: strcpy(str,"DATA PHASE ERROR"); break;
		case 0x4C: strcpy(str,"LOGICAL UNIT FAILED SELF-CONFIGURATION"); break;
		case 0x53: strcpy(str,"MEDIA LOAD OR EJECT FAILED"); break;
		case 0x65: strcpy(str,"VOLTAGE FAULT"); break;
		}
		break;
	case 0x5:
		switch (ASC(err)) {
		case 0x07: strcpy(str,"MULTIPLE PERIPHERAL DEVICES SELECTED"); break; /* ASCQ = 0x00 */
		case 0x1A: strcpy(str,"PARAMETER LIST LENGTH ERROR"); break; /* ASCQ = 0x00 */
		case 0x20: strcpy(str,"INVALID COMMAND OPERATION CODE"); break; /* ASCQ = 0x00 */
		case 0x21:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"LOGICAL BLOCK ADDRESS OUT OF RANGE"); break;
			case 0x01: strcpy(str,"INVALID ELEMENT ADDRESS"); break;
			case 0x02: strcpy(str,"INVALID ADDRESS FOR WRITE"); break;

			default:   strcpy(str,"LOGICAL BLOCK ADDRESS OUT OF RANGE"); break;
			}
			break;
		case 0x24: strcpy(str,"INVALID FIELD IN CDB"); break;
		case 0x25: strcpy(str,"LOGICAL UNIT NOT SUPPORTED"); break;
		case 0x26:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"INVALID FIELD IN PARAMETER LIST"); break;
			case 0x01: strcpy(str,"PARAMETER NOT SUPPORTED"); break;
			case 0x02: strcpy(str,"PARAMETER VALUE INVALID"); break;
			case 0x03: strcpy(str,"THRESHOLD PARAMETERS NOT SUPPORTED"); break;
			}
			break;
		case 0x2B: strcpy(str,"COPY CANNOT EXECUTE SINCE INITIATOR CANNOT DISCONNECT"); break; /* ASCQ = 0x00 */
		case 0x2C:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"COMMAND SEQUENCE ERROR"); break;
			case 0x03: strcpy(str,"CURRENT PROGRAM AREA IS NOT EMPTY"); break;
			case 0x04: strcpy(str,"CURRENT PROGRAM AREA IS EMPTY"); break;
			}
			break;
		case 0x30:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"INCOMPATIBLE MEDIUM INSTALLED"); break;
			case 0x01: strcpy(str,"CANNOT READ MEDIUM - UNKNOWN FORMAT"); break;
			case 0x02: strcpy(str,"CANNOT READ MEDIUM - INCOMPATIBLE FORMAT"); break;
			case 0x03: strcpy(str,"CLEANING CARTRIDGE INSTALLED"); break;
			case 0x04: strcpy(str,"CANNOT WRITE MEDIUM - UNKNOWN FORMAT"); break;
			case 0x05: strcpy(str,"CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT"); break;
			case 0x06: strcpy(str,"CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM"); break;
			case 0x07: strcpy(str,"CLEANING FAILURE"); break;
			case 0x08: strcpy(str,"CANNOT WRITE - APPLICATION CODE MISMATCH"); break;
			case 0x09: strcpy(str,"CURRENT SESSION NOT FIXATED FOR APPEND"); break;
			case 0x10: strcpy(str,"MEDIUM NOT FORMATTED"); break;
			}
			break;
		case 0x39: strcpy(str,"SAVING PARAMETERS NOT SUPPORTED"); break;  /* ASCQ = 0x00 */
		case 0x3D: strcpy(str,"INVALID BITS IN IDENTIFY MESSAGE"); break; /* ASCQ = 0x00 */
		case 0x43: strcpy(str,"MESSAGE ERROR"); break; /* ASCQ = 0x00 */
		case 0x53: strcpy(str,"MEDIUM REMOVAL PREVENTED"); break; /* ASCQ = 0x02 */
		case 0x64:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"ILLEGAL MODE FOR THIS TRACK"); break;
			case 0x01: strcpy(str,"INVALID PACKET SIZE"); break;
			}
			break;
		case 0x6F:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"COPY PROTECTION KEY EXCHANGE FAILURE - AUTHENTICATION FAILURE"); break;
			case 0x01: strcpy(str,"COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT PRESENT"); break;
			case 0x02: strcpy(str,"COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT ESTABLISHED"); break;
			case 0x03: strcpy(str,"READ OF SCRAMBLED SECTOR WITHOUT AUTHENTICATION"); break;
			case 0x04: strcpy(str,"MEDIA REGION CODE IS MISMATCHED TO LOGICAL UNIT REGION"); break;
			case 0x05: strcpy(str,"LOGICAL UNIT REGION MUST BE PERMANENT/REGION RESET COUNT ERROR"); break;
			}
			break;
		case 0x72:
			switch (ASCQ(err)) {
			case 0x03: strcpy(str,"SESSION FIXATION ERROR . INCOMPLETE TRACK IN SESSION"); break;
			case 0x04: strcpy(str,"EMPTY OR PARTIALLY WRITTEN RESERVED TRACK"); break;
			case 0x05: strcpy(str,"NO MORE TRACK RESERVATIONS ALLOWED"); break;
			}
			break;
		}
		break;
	case 0x6:
		switch (ASC(err)) {
		case 0x28:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"NOT READY TO READY CHANGE, MEDIUM MAY HAVE CHANGED"); break;
			case 0x01: strcpy(str,"IMPORT OR EXPORT ELEMENT ACCESSED"); break;
			}
			break;
		case 0x29:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"POWER ON, RESET, OR BUS DEVICE RESET OCCURRED"); break;
			case 0x01: strcpy(str,"POWER ON OCCURRED"); break;
			case 0x02: strcpy(str,"BUS RESET OCCURRED"); break;
			case 0x03: strcpy(str,"BUS DEVICE RESET FUNCTION OCCURRED"); break;
			case 0x04: strcpy(str,"DEVICE INTERNAL RESET"); break;
			}
			break;
		case 0x2A:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"PARAMETERS CHANGED"); break;
			case 0x01: strcpy(str,"MODE PARAMETERS CHANGED"); break;
			case 0x02: strcpy(str,"LOG PARAMETERS CHANGED"); break;
			case 0x03: strcpy(str,"RESERVATIONS PREEMPTED"); break;
			}
			break;
		case 0x2E: strcpy(str,"INSUFFICIENT TIME FOR OPERATION"); break;
		case 0x2F: strcpy(str,"COMMANDS CLEARED BY ANOTHER INITIATOR"); break;
		case 0x3B:
			switch (ASCQ(err)) {
			case 0x0D: strcpy(str,"MEDIUM DESTINATION ELEMENT FULL"); break;
			case 0x0E: strcpy(str,"MEDIUM SOURCE ELEMENT EMPTY"); break;
			case 0x0F: strcpy(str,"END OF MEDIUM REACHED"); break;
			case 0x11: strcpy(str,"MEDIUM MAGAZINE NOT ACCESSIBLE"); break;
			case 0x12: strcpy(str,"MEDIUM MAGAZINE REMOVED"); break;
			case 0x13: strcpy(str,"MEDIUM MAGAZINE INSERTED"); break;
			case 0x14: strcpy(str,"MEDIUM MAGAZINE LOCKED"); break;
			case 0x15: strcpy(str,"MEDIUM MAGAZINE UNLOCKED"); break;
			}
			break;
		case 0x3F:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"TARGET OPERATING CONDITIONS HAVE CHANGED"); break;
			case 0x01: strcpy(str,"MICROCODE HAS BEEN CHANGED"); break;
			case 0x02: strcpy(str,"CHANGED OPERATING DEFINITION"); break;
			case 0x03: strcpy(str,"INQUIRY DATA HAS CHANGED"); break;
			}
			break;
		case 0x5A:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"OPERATOR REQUEST OR STATE CHANGE INPUT"); break;
			case 0x01: strcpy(str,"OPERATOR MEDIUM REMOVAL REQUEST"); break;
			case 0x02: strcpy(str,"OPERATOR SELECTED WRITE PROTECT"); break;
			case 0x03: strcpy(str,"OPERATOR SELECTED WRITE PERMIT"); break;
			}
			break;
		case 0x5B:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"LOG EXCEPTION"); break;
			case 0x01: strcpy(str,"THRESHOLD CONDITION MET"); break;
			case 0x02: strcpy(str,"LOG COUNTER AT MAXIMUM"); break;
			case 0x03: strcpy(str,"LOG LIST CODES EXHAUSTED"); break;
			}
			break;
		case 0x5E:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"LOW POWER CONDITION ON"); break;
			case 0x01: strcpy(str,"IDLE CONDITION ACTIVATED BY TIMER"); break;
			case 0x02: strcpy(str,"STANDBY CONDITION ACTIVATED BY TIMER"); break;
			case 0x03: strcpy(str,"IDLE CONDITION ACTIVATED BY COMMAND"); break;
			case 0x04: strcpy(str,"STANDBY CONDITION ACTIVATED BY COMMAND"); break;
			}
			break;
		}
		break;
	case 0x7:
		switch (ASC(err)) {
		case 0x27:
			switch (ASCQ(err)) {
			case 0x00: strcpy(str,"WRITE PROTECTED"); break;
			case 0x01: strcpy(str,"HARDWARE WRITE PROTECTED"); break;
			case 0x02: strcpy(str,"LOGICAL UNIT SOFTWARE WRITE PROTECTED"); break;
			case 0x03: strcpy(str,"ASSOCIATED WRITE PROTECT"); break;
			case 0x04: strcpy(str,"PERSISTENT WRITE PROTECT"); break;
			case 0x05: strcpy(str,"PERMANENT WRITE PROTECT"); break;
			case 0x06: strcpy(str,"CONDITIONAL WRITE PROTECT"); break;

			default:   strcpy(str,"WRITE PROTECTED"); break;
			}
			break;
		}
		break;
	case 0x8: strcpy(str,"BLANK CHECK"); break;
	case 0xB:
		switch (ASC(err)) {
			case 0x00: strcpy(str,"I/O PROCESS TERMINATED"); break; /* ASCQ = 06 */
        	case 0x11: strcpy(str,"READ ERROR - LOSS OF STREAMING"); break; /* ASCQ = 11 */
        	case 0x45: strcpy(str,"SELECT OR RESELECT FAILURE"); break; /* ASCQ = 00 */
        	case 0x48: strcpy(str,"INITIATOR DETECTED ERROR MESSAGE RECEIVED"); break; /* ASCQ = 00 */
        	case 0x49: strcpy(str,"INVALID MESSAGE ERROR"); break; /* ASCQ = 00 */
        	case 0x4D: strcpy(str,"TAGGED OVERLAPPED COMMANDS (NN = QUEUE TAG)"); break; /* ASCQ = xx */
		}
		break;
	}
	printf("[%05X]  %s", err, str);
	return 0;
}

int print_opcode (unsigned char opcode) {
	char str[128];
	switch (opcode) {

//	case 0x00: strcpy(str,"      "); break;


		case 0x00: strcpy(str,"SPC_TEST_UNIT_READY                           "); break;
		case 0x03: strcpy(str,"SPC_REQUEST_SENSE                             "); break;
		case 0x04: strcpy(str,"MMC_FORMAT_UNIT                               "); break;

		case 0x12: strcpy(str,"SPC_INQUIRY                                   "); break;
		case 0x15: strcpy(str,"MMC_MODE_SELECT6                              "); break;
		case 0x16: strcpy(str,"SPC_RESERVE_6                                 "); break;
		case 0x17: strcpy(str,"SPC_RELEASE_6                                 "); break;
		case 0x1A: strcpy(str,"MMC_MODE_SENSE6                               "); break;
		case 0x1B: strcpy(str,"MMC_START_STOP_UNIT                           "); break;
		case 0x1C: strcpy(str,"SPC_RECEIVE_DIAGNOSTIC_RESULTS                "); break;
		case 0x1D: strcpy(str,"SPC_SEND_DIAGNOSTIC                           "); break;
		case 0x1E: strcpy(str,"SPC_PREVENT_ALLOW_MEDIUM_REMIVAL              "); break;


		case 0x23: strcpy(str,"MMC_READ_FORMAT_CAPACITIES                    "); break;
		case 0x25: strcpy(str,"MMC_READ_RECORDED_CAPACITY                    "); break;
		case 0x28: strcpy(str,"MMC_READ                                      "); break;
		case 0x2A: strcpy(str,"MMC_WRITE                                     "); break;
		case 0x2B: strcpy(str,"MMC_SEEK                                      "); break;
		case 0x2E: strcpy(str,"MMC_WRITE_AND_VERIFY                          "); break;
		case 0x2F: strcpy(str,"MMC_VERIFY                                    "); break;

		case 0x35: strcpy(str,"MMC_SYNC_CACHE                                "); break;
		case 0x3B: strcpy(str,"SPC_WRITE_BUFFER                              "); break;
		case 0x3C: strcpy(str,"SPC_READ_BUFFER                               "); break;

		case 0x42: strcpy(str,"MMC_READ_SUB_CHANNEL                          "); break;
		case 0x43: strcpy(str,"MMC_READ_TOC_PMA_ATIP                         "); break;
		case 0x44: strcpy(str,"MMC_READ_HEADER                               "); break;
		case 0x45: strcpy(str,"MMC_PLAY_AUDIO                                "); break;
		case 0x46: strcpy(str,"MMC_GET_CONFIGURATION                         "); break;
		case 0x47: strcpy(str,"MMC_PLAY_AUDIO_MSF                            "); break;
		case 0x4A: strcpy(str,"MMC_GET_EVENT_STATUS_NOTIFICATION             "); break;
		case 0x4B: strcpy(str,"MMC_PAUSE_RESUME                              "); break;
		case 0x4C: strcpy(str,"SPC_LOG_SELECT                                "); break;
		case 0x4D: strcpy(str,"SPC_LOG_SENSE                                 "); break;
		case 0x4E: strcpy(str,"MMC_STOP_PLAY_SCAN                            "); break;

		case 0x51: strcpy(str,"MMC_READ_DISC_INFORMATION                     "); break;
		case 0x52: strcpy(str,"MMC_READ_TRACK_INFORMATION                    "); break;
		case 0x53: strcpy(str,"MMC_RESERVE_TRACK                             "); break;
		case 0x54: strcpy(str,"MMC_SEND_OPC_INFORMATION                      "); break;
		case 0x55: strcpy(str,"MMC_MODE_SELECT10                             "); break;
		case 0x56: strcpy(str,"SPC_RESERVE_10                                "); break;
		case 0x57: strcpy(str,"SPC_RELEASE_10                                "); break;
		case 0x58: strcpy(str,"MMC_REPAIR_TRACK                              "); break;
		case 0x59: strcpy(str,"MMC_READ_MASTER_CUE                           "); break;
		case 0x5A: strcpy(str,"MMC_MODE_SENSE10                              "); break;
		case 0x5B: strcpy(str,"MMC_CLOSE_TRACK_SESSION                       "); break;
		case 0x5C: strcpy(str,"MMC_READ_BUFFER_CAPACITY                      "); break;
		case 0x5D: strcpy(str,"MMC_SEND_CUE_SHEET                            "); break;
		case 0x5E: strcpy(str,"SPC_PERSISTENT_RESERVE_IN                     "); break;
		case 0x5F: strcpy(str,"SPC_PERSISTENT_RESERVE_OUT                    "); break;

		case 0x83: strcpy(str,"SPC_EXTENDED_COPY                             "); break;
		case 0x84: strcpy(str,"SPC_RECEIVE_COPY_RESULTS                      "); break;
		case 0x86: strcpy(str,"SPC_ACCESS_CONTROL_IN                         "); break;
		case 0x87: strcpy(str,"SPC_ACCESS_CONTROL_OUT                        "); break;
		case 0x8C: strcpy(str,"SPC_READ_ATTRIBUTE                            "); break;
		case 0x8D: strcpy(str,"SPC_WRITE_ATTRIBUTE                           "); break;

		case 0xA0: strcpy(str,"SPC_REPORT_LUNS                               "); break;
		case 0xA1: strcpy(str,"MMC_BLANK                                     "); break;
		case 0xA2: strcpy(str,"MMC_SEND_EVENT     / SPC_SECURITY_PROTOCOL_IN "); break;
		case 0xA3: strcpy(str,"MMC_SEND_KEY                                  "); break;
		case 0xA4: strcpy(str,"MMC_REPORT_KEY                                "); break;
		case 0xA5: strcpy(str,"MMC_PLAY_AUDIO_12                             "); break;
		case 0xA6: strcpy(str,"MMC_LOAD_UNLOAD                               "); break;
		case 0xA7: strcpy(str,"MMC_SET_READ_AHEAD / SMC_MOVE_MEDIUM_ATTACHED "); break;
		case 0xA8: strcpy(str,"MMC_READ_DVD                                  "); break;
		case 0xAC: strcpy(str,"MMC_GET_PERFORMANCE                           "); break;
		case 0xAD: strcpy(str,"MMC_READ_DVD_STRUCTURE                        "); break;

		case 0xB4: strcpy(str,"SMC_READ_ELEMENT_STATUS_ATTACHED              "); break;
		case 0xB5: strcpy(str,"SPC_SECURITY_PROTOCOL_OUT                     "); break;
		case 0xB6: strcpy(str,"MMC_SET_STREAMING                             "); break;
		case 0xB9: strcpy(str,"MMC_READ_CD_MSF                               "); break;
		case 0xBA: strcpy(str,"MMC_SCAN                                      "); break;
		case 0xBB: strcpy(str,"MMC_SET_SPEED                                 "); break;
		case 0xBC: strcpy(str,"MMC_PLAY_CD          / SCC_SPARE_IN           "); break;
		case 0xBD: strcpy(str,"MMC_MECHANISM_STATUS / SCC_SPARE_OUT          "); break;
		case 0xBE: strcpy(str,"MMC_READ_CD          / SCC_VOLUME_SET_IN      "); break;
		case 0xBF: strcpy(str,"SCC_VOLUME_SET_OUT                            "); break;

		case 0xD4: strcpy(str,"PLEXTOR_GET_AUTH                              "); break;
		case 0xD5: strcpy(str,"PLEXTOR_SEND_AUTH                             "); break;

//const char  = 0xD8;

		case 0xE3: strcpy(str,"PLEXTOR_ERASER                                "); break;
		case 0xE4: strcpy(str,"PLEXTOR_AS_RD                                 "); break;
		case 0xE5: strcpy(str,"PLEXTOR_AS_WR                                 "); break;

		case 0xE7: strcpy(str,"SBC_FLUSH_CACHE                               "); break;

		case 0xE9: strcpy(str,"PLEXTOR_MODE                                  "); break;
		case 0xEA: strcpy(str,"PLEXTOR_QCHECK                                "); break;
		case 0xEB: strcpy(str,"PLEXTOR_PREC_SPD                              "); break;
		case 0xED: strcpy(str,"PLEXTOR_MODE2                                 "); break;

		case 0xEE: strcpy(str,"PLEXTOR_RESTART                               "); break;
//		case 0xEF: strcpy(str,"PLEXTOR     REBOOT ???                        "); break;

		case 0xF1: strcpy(str,"PLEXTOR_EEPROM_READ                           "); break;

		case 0xF3: strcpy(str,"PLEXTOR_SCAN_TA_FETE                          "); break;
		case 0xF5: strcpy(str,"PLEXTOR_FETE_READOUT                          "); break;
		default:   strcpy(str,"*unknown*                                     ");
	}
	printf("[%02X]  %s", opcode, str);
	return 0;
}

int probe_drive(const char* path, int idx)
{
	drive_info*	drive;
	int inq;
//	printf("Trying device: %s\n", path);
	drive = new drive_info(path);
	drive->silent++;
	inq = inquiry(drive);
	drive->silent--;
	switch (inq) {
		case 0x00:
#if 0
//				strcpy(drvtbl[drvcnt],_devtbl[i]);
				printf("[%02d] %s %s: %s %s %s\n",(drive->ven_ID & vendor_mask) ? "*" : " ",
					drvcnt, drive->device, drive->ven, drive->dev, drive->fw);
#else
				printf("D: [%02d] '%s': '%s' '%s' '%s'\n",
					idx, drive->device, drive->ven, drive->dev, drive->fw);
#endif
				break;
			case ERR_NO_DEV:
			//	if (!drive->silent) printf("%s: no device found\n",drive->device);
				break;
			case ERR_NO_SCSI:
			//	if (!drive->silent) printf("%s: not a valid SCSI device\n",drive->device);
				break;
			case ERR_NO_MMC:
			//	printf("%s: %s %s %s",drive->device,drive->ven,drive->dev,drive->fw);
			//	printf(": device is not MMC compliant\n");
			//	for (j=0; j<8; j++)
			//	    printf("Ver ID [%d] = 0x%04X\n",j,drive->ver_id[j]);
				break;
			default:
			//	printf("%s: ???\n",drive->device);
				break;
	}
	delete drive;
	return inq;
}

int scanbus(int vendor_mask)
{
	int	i;
//	int inq;
	int	drvcnt=0;

#ifndef DEVTBL_DIRECT
	DIR  *dir;
	struct dirent *dentry;
	struct stat st;
	str_dev devstr;
	int	  dlen;
#endif

	printf("** scanning IDE/SATA/SCSI buses...\n");

#ifdef DEVTBL_DIRECT // used in win32 only
	for (i=0; strlen(_devtbl[i].name)>0 ; i++) {
		if (!probe_drive(_devtbl[i].name, drvcnt)) drvcnt++;
	}
#else
	for (i=0; strlen(_devtbl[i].name)>0 ; i++) {
		dlen = strlen(_devtbl[i].name);
		dir = opendir("/dev");
		if (dir)
		{
			dentry = readdir(dir);
			while (dentry) {
				if (!strncmp(dentry->d_name, _devtbl[i].name, dlen) &&
					(!_devtbl[i].len || (strlen(dentry->d_name) == (size_t) _devtbl[i].len)) ) {
					sprintf(devstr, "/dev/%s", dentry->d_name);
					if (!lstat(devstr, &st) && S_ISBLK(st.st_mode) && !probe_drive(devstr, drvcnt)) drvcnt++;
				}
				dentry = readdir(dir);
			}
			closedir(dir);
		}
	}
#endif
	printf("** Scan complete: %d device(s) found\n", drvcnt);
	return (drvcnt);
}

void spinup(drive_info* drive, unsigned char secs) {
	long st, et;
	int err=0;
	char use_readcd = 0;
	const int  addt = 25;
#ifdef SPINUP_REVERSE
	int32_t lba = drive->media.capacity-1;
#else
	int32_t lba = 0;
#endif
	int blk = 16;
	st = getmsecs() + addt;
	if (drive->media.type & DISC_CD) {
		blk = 15;
		if (drive->capabilities & CAP_DAE) use_readcd = 1;
	}
	printf("SpinUp using READ%s command...\n", use_readcd ? " CD" : "" );
//	if (use_readcd) read_cd(drive, 0, 1, 0xF8, 1);
	for ( et = getmsecs(); !err && (et-st) < (secs*1000); et = getmsecs() ) {
#ifdef SPINUP_REVERSE
		lba-=blk;
#else
		lba+=blk;
#endif
//		err = seek(drive, lba);
		if (!drive->silent) printf("Remaining: %.3f sec...\r", secs - ((et-st) / 1000.0));
		if (use_readcd) err = read_cd(drive, drive->rd_buf, lba, blk, 0xF8);
		else		err = read(drive, drive->rd_buf, lba, blk);
	}

	if (use_readcd)	read_cd(drive, drive->rd_buf, 0, 1, 0xF8);
	else		read(drive, drive->rd_buf, 0, 1);
//	seek(drive, 0);

	msleep( addt );
}

int inquiry(drive_info* drive) {
	unsigned char add_len;
	int i;
	if (drive->mmc == -1) return ERR_NO_DEV;
	drive->cmd[0] = SPC_INQUIRY;
	drive->cmd[4] = 36;
	drive->cmd[5] = 0;
	drive->err=drive->cmd.transport(READ,drive->rd_buf,36);
	if (drive->err) {
	    if (!drive->silent) sperror("INQUIRY", drive->err);
	    return ERR_NO_SCSI;
	}
	
	memcpy(drive->ven,drive->rd_buf+8,8);
	drive->ven[8] = 0;
	memcpy(drive->dev,drive->rd_buf+16,16);
	drive->dev[16] = 0;
	memcpy(drive->fw,drive->rd_buf+32,4);
	drive->fw[4] = 0;
	add_len = drive->rd_buf[4];

	for (i=0; i<8; i++) drive->ver_id[i]=0;
	if (add_len >= 91) {
		drive->cmd[0] = SPC_INQUIRY;
		drive->cmd[4] = 5+add_len;
		drive->cmd[5] = 0;
		drive->err=drive->cmd.transport(READ,drive->rd_buf,5+add_len);
		if (drive->err) {
			if (!drive->silent) sperror("INQUIRY ADD", drive->err);
		} else {
			for (i=0; i<8; i++) drive->ver_id[i]= ntoh16(drive->rd_buf+58+i*2);
		}
	}
	if ((drive->rd_buf[0]&0x1F) != 5) return ERR_NO_MMC;
	drive->mmc=1;
	convert_to_ID(drive);
	return 0;
}

int isPlextor(drive_info* drive)
{
	if (!strncmp(drive->ven,"PLEXTOR ",8)) {
		if( !strncmp(drive->dev,"CD-R   ", 7)) return 1;
		if( !strncmp(drive->dev,"DVDR   PX-708A",14) ||
		    !strncmp(drive->dev,"DVDR   PX-708A2",15)  ||
		    !strncmp(drive->dev,"DVDR   PX-712A",14)  ||
		    !strncmp(drive->dev,"DVDR   PX-714A",14)  ||
		    !strncmp(drive->dev,"DVDR   PX-716A ",15) ||
		    !strncmp(drive->dev,"DVDR   PX-716AL",15) ||
		    !strncmp(drive->dev,"DVDR   PX-755A",14)  ||
		    !strncmp(drive->dev,"DVDR   PX-760A",14)
		)
			return 1;
	}
	return 0;
}

int isPlextorLockPresent(drive_info* drive)
{
	if( !strncmp(drive->dev,"CD-R   PREMIUM2",15) ||
	    !strncmp(drive->dev,"DVDR   PX-755A",14) ||
	    !strncmp(drive->dev,"DVDR   PX-760A",14)
	)
	    return 1;
	else
	    return 0;
}

int isYamaha(drive_info* drive) { return !strncmp(drive->ven,"YAMAHA  ", 8); }

int isPioneer(drive_info* drive) { return !strncmp(drive->ven,"PIONEER ", 8); }

int test_unit_ready(drive_info* drive) {
	drive->cmd[0] = SPC_TEST_UNIT_READY;
	drive->cmd[3] = 0;
	return drive->err=drive->cmd.transport(NONE, NULL, 0);
}

int wait_unit_ready(drive_info* drive, int secs, bool need_media) {
	long st, et;
	st = getmsecs();
	for ( et = getmsecs(); (et-st) < ((long)secs*1000); et = getmsecs() ) {
		if (!drive->silent) printf("Remaining: %.3f sec...\r", secs - ((et-st) / 1000.0));
		if (!test_unit_ready(drive)) return 0;
		if (!need_media) {
			if (drive->err == 0x23A01) return 0;
			if (drive->err == 0x23A02) return 0;
		}
		msleep(100);
//		i++;
	}
	printf("wait_unit_ready(): Time Out (%ds)\n", secs);
	return 1;
}

int wait_fix(drive_info* drive, int secs){
	long st, et;
	st = getmsecs();
	for ( et = getmsecs(); (et-st) < ((long)secs*1000); et = getmsecs() ) {
		if (!drive->silent) printf("Remaining: %.3f sec...\r", secs - ((et-st) / 1000.0));
		if (!read_disc_info(drive, 16)) return 0;
		 //  if not SC_NOT_READY or SC_UNIT_ATTENTION return
		if ((drive->err & 0xF0000) != 0x20000 && (drive->err & 0xF0000) != 0x60000 ) return 1;
		msleep(100);
//		i++;
	}
	printf("wait_fix(): Time Out (%ds)\n", secs);
	return 1;
}


int check_burnfree(drive_info* drive)
{
	if (mode_sense(drive, MODE_PAGE_WRITE_PARAMETERS, 0, 60)) return -1;
	drive->rd_buf[8+2] |= 0x40;
	if (mode_select(drive, 60) || !(drive->rd_buf[8+2] & 0x40)) {
		printf("BURN-free seems to not supported for this media!\n");
	} else {
//		printf("BURN-free supported:)\n");
		drive->capabilities|=CAP_BURN_FREE;
	}
	return 0;
}

int check_write_modes(drive_info* drive)
{
	drive->wr_modes = 0;
	if (mode_sense(drive, MODE_PAGE_WRITE_PARAMETERS, 0, 60)) return -1;

	for (int i=0; wr_modes[i].id; i++) {
		drive->rd_buf[8+2] &= 0xF0;
		drive->rd_buf[8+2] |= wr_modes[i].wtype;
		drive->rd_buf[8+4] &= 0xF0;
		drive->rd_buf[8+4] |= wr_modes[i].dtype;
		if (!mode_select(drive, 60)) {
			drive->wr_modes |= wr_modes[i].id;
		}
	}
	check_burnfree(drive);
	return 0;
}

int reserve_track(drive_info* drive, uint32_t size)
{
	drive->cmd[0] = MMC_RESERVE_TRACK;
 	drive->cmd[5] = (size >> 24) & 0xFF;
 	drive->cmd[6] = (size >> 16) & 0xFF;
 	drive->cmd[7] = (size >>  8) & 0xFF;
 	drive->cmd[8] = size & 0xFF;

	if ((drive->err=drive->cmd.transport(NONE, NULL, 0)))
		{sperror ("RESERVE_TRACK",drive->err); return (drive->err);}
	return 0;
}

int close_track_session(drive_info* drive, int n, int cltype)
{
	drive->cmd[0] = MMC_CLOSE_TRACK_SESSION;
//	drive->cmd[1] = immed ? 0x01 : 0x00;
	drive->cmd[1] = 0x01;
	drive->cmd[2] = cltype;
	drive->cmd[4] = (n >> 8) & 0xFF;
	drive->cmd[5] = n & 0xFF;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0)))
		{sperror ("MMC_CLOSE_TRACK_SESSION",drive->err); return (drive->err);}
	return 0;
}

int request_sense(drive_info* drive, char add){
	drive->cmd[0]= SPC_REQUEST_SENSE;
	drive->cmd[4]= 0x12+add;
	drive->cmd[5]=0;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,0x12) ))
		{ if (!drive->silent) sperror ("REQUEST_SENSE",drive->err); return (drive->err); }
	return 0;
}

/*
int mode_sense6(drive_info* drive, int page, int page_control, short dest_len) {
	drive->cmd[0]=MMC_MODE_SENSE6;
	drive->cmd[1]= 0x10;
	drive->cmd[2]=page_control << 6 | page;
	drive->cmd[4]=dest_len & 0xFF;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,dest_len) ))
		{sperror ("MODE_SENSE(6)",drive->err); return (drive->err); }
	return 0;
}
*/

int mode_sense(drive_info* drive, int page, int page_control, int dest_len) {
	drive->cmd[0]=MMC_MODE_SENSE_10;
	drive->cmd[2]=page_control << 6 | page;
	drive->cmd[7]=(dest_len >> 8) & 0xFF;
	drive->cmd[8]=dest_len & 0xFF;
	drive->cmd[9]=0;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,dest_len) ))
		{ if (!drive->silent) sperror ("MODE_SENSE(10)",drive->err); return (drive->err); }
	return 0;
}

/*
int mode_select6(drive_info* drive, short dest_len) {
	drive->cmd[0]=MMC_MODE_SELECT6;
//	drive->cmd[1]= 0x10;
	drive->cmd[4]=dest_len & 0xFF;
	if ((drive->err=drive->cmd.transport(WRITE,drive->rd_buf,dest_len) ))
		{sperror ("MODE_SELECT(6)",drive->err); return (drive->err); }
	return 0;
}
*/

int mode_select(drive_info* drive, int dest_len) {
	drive->cmd[0]=MMC_MODE_SELECT_10;
	drive->cmd[1]= 0x10;
/*
	drive->cmd[5]= drive->rd_buf[0];
	drive->cmd[6]= drive->rd_buf[1];
*/
	drive->cmd[7]=(dest_len >> 8) & 0xFF;
	drive->cmd[8]=dest_len & 0xFF;
	drive->cmd[9]=0;
	if ((drive->err=drive->cmd.transport(WRITE,drive->rd_buf,dest_len) ))
		{ if (!drive->silent) sperror ("MODE_SELECT(10)",drive->err); return (drive->err); }
//		{ sperror ("MODE_SELECT(10)",drive->err); return (drive->err); }
	return 0;
}

int get_configuration(drive_info* drive, int feature_number, unsigned int* data_length, int* current, unsigned char ReqType) {
	if (data_length) *data_length = 0;
	if (current) *current = 0;
	drive->cmd[0] = MMC_GET_CONFIGURATION;
	drive->cmd[1] = ReqType;
	drive->cmd[2] = (feature_number >> 8) & 0xFF;
	drive->cmd[3] = feature_number & 0xFF;
	drive->cmd[7] = 0;
	drive->cmd[8] = 8;
	drive->cmd[9] = 0;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8)))
		{ if (!drive->silent) sperror ("GET_CONFIGURATION (LENGTH)",drive->err); return (drive->err);}
	if (data_length)
	{
		*data_length = ntoh32u(drive->rd_buf);
		drive->cmd[7] = ((*data_length+4) >> 8) & 0xFF;
		drive->cmd[8] = (*data_length+4) & 0xFF;
		drive->cmd[9] = 0;
		if (*data_length > 4)
			if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,*data_length+4)))
				{ if (!drive->silent) sperror ("GET_CONFIGURATION",drive->err); return (drive->err);}
	}
	if (current) *current = drive->rd_buf[10] & 0x01;
	return 0;
}

void detect_iface(drive_info* drive)
{
	unsigned int len=0;
	get_configuration(drive, FEATURE_CORE, &len, NULL);
	drive->iface_id= (drive->rd_buf[12] << 12) | (drive->rd_buf[13] << 8) | (drive->rd_buf[14] << 4) | drive->rd_buf[15];
	if (drive->iface_id<iface_id_max)
		strcpy(drive->iface,iface_list[drive->iface_id]);
	else
		strcpy(drive->iface,iface_list[iface_id_max+1]);
}


int write_buffer(drive_info* drive, uint8_t mode, uint8_t buff_id, uint32_t offs, uint32_t len)
{
	drive->cmd[0] = SPC_WRITE_BUFFER;
	drive->cmd[1] = (mode & 0x0F);
	drive->cmd[2] = buff_id;

	drive->cmd[3] = (offs >> 16) & 0xFF;
	drive->cmd[4] = (offs >>  8) & 0xFF;
	drive->cmd[5] = (offs) & 0xFF;

	drive->cmd[6] = (len >> 16) & 0xFF;
	drive->cmd[7] = (len >>  8) & 0xFF;
	drive->cmd[8] = (len) & 0xFF;
	drive->cmd[9] = 0;

	if ((drive->err=drive->cmd.transport(WRITE,drive->rd_buf, len) ))
		{ if (!drive->silent) sperror ("WRITE_BUFFER",drive->err); return (drive->err); }

	return 0;
}

int read_buffer(drive_info* drive, uint8_t mode, uint8_t buff_id, uint32_t offs, uint32_t len)
{
	printf("read buffer: mode %x, id %x, offs %x, len %x\n", mode, buff_id,offs,len);
	drive->cmd[0] = SPC_READ_BUFFER;
	drive->cmd[1] = (mode & 0x0F);
	drive->cmd[2] = buff_id;

	drive->cmd[3] = (offs >> 16) & 0xFF;
	drive->cmd[4] = (offs >>  8) & 0xFF;
	drive->cmd[5] = (offs) & 0xFF;

	drive->cmd[6] = (len >> 16) & 0xFF;
	drive->cmd[7] = (len >>  8) & 0xFF;
	drive->cmd[8] = (len) & 0xFF;
	drive->cmd[9] = 0;

	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf, len) ))
		{ if (!drive->silent) sperror ("READ_BUFFER",drive->err); return (drive->err); }

	return 0;
}

int read_echo_buffer_size(drive_info* drive)
{
	drive->cmd[0] = SPC_READ_BUFFER;
//	drive->cmd[1] = 0x00;
	drive->cmd[1] = 0x0B; // echo buffer descriptor
	drive->cmd[8] = 4;
	drive->cmd[9] = 0;

	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf, 4) ))
		{ if (!drive->silent) sperror ("READ_ECHO_BUFFER_SIZE",drive->err); return (0); }
	
	return ntoh16(drive->rd_buf+2) & 0x1FFF;
}

int test_dma_speed(drive_info* drive, long msecs)
{
	long st, et;
	long speed=0;

	int cnt =0;
	drive->silent--;

	uint32_t buf_size  = read_echo_buffer_size(drive);
	printf("Echo buffer size: %d\n", buf_size);

	buf_size = 4096;

	if (!buf_size) {
		drive->silent++;
		return 0;
	}

	if (!drive->silent)
		printf("** Testing DMA speed...\n");
	st = getmsecs();
	for ( et = getmsecs(); (et-st) < (msecs); et = getmsecs() ) {
		if (write_buffer(drive, 0x0A, 0, 0, buf_size)) 
		{
			if (!drive->silent) printf("WRITE BUFFER error! DMA speed test aborted!\n");
			drive->silent++;
			return 1;
		}
		if (read_buffer(drive, 0x0A, 0, 0, buf_size)) 
		{
			if (!drive->silent) printf("READ BUFFER error! DMA speed test aborted!\n");
			drive->silent++;
			return 1;
		}
		cnt++;
	}
	speed = (buf_size * cnt / msecs);
	printf("DMA speed: %6ld kB/s (%d buffers of %d bytes in %ld msecs)\n", speed, cnt, buf_size, msecs);
	drive->silent++;
	return 0;
}

int flush_cache(drive_info* drive, bool IMMED) {
//	drive->cmd[0] = SBC_FLUSH_CACHE;
	drive->cmd[0] = MMC_SYNC_CACHE;
	drive->cmd[1] = IMMED ? 0x02 : 0;

	if ((drive->err=drive->cmd.transport(NONE, NULL, 0) ))
		{ if (!drive->silent) sperror ("SBC_FLUSH_CACHE",drive->err); return (drive->err); }
//		{ sperror ("SBC_FLUSH_CACHE",drive->err); return (drive->err); }

	return 0;
}

int set_cache(drive_info* drive, bool rd, bool wr)
{
	memset(drive->rd_buf, 0, 20);
	drive->rd_buf[8] = MODE_PAGE_CACHING;
	drive->rd_buf[9] = 0x0A;
	drive->rd_buf[10] = (wr ? 0x04 : 0) | (rd ? 0 : 0x01);
	
	return (mode_select(drive, 0x20));
}

int get_cache(drive_info* drive, bool *rd, bool *wr)
{
	bool re,we;
	if (mode_sense(drive, MODE_PAGE_CACHING, 0, 20) || drive->rd_buf[8] != MODE_PAGE_CACHING) return 1;
	re = !(drive->rd_buf[10] & 0x01);
	we = !!(drive->rd_buf[10] & 0x04);
	printf("Cache:  RD %s, WR %s\n", re ? "EN":"DIS", we ? "EN":"DIS");
	if (rd) (*rd) = re;
	if (wr) (*wr) = we;
	return 0;
}

int get_mode_pages_list(drive_info* drive) {
	unsigned int len, i, ii;
	unsigned char ml=0, mn=0;
	if (!drive->silent) printf("\n** Reading supported mode pages...\n");
	if (mode_sense(drive, 0x3F, 2, 0x4000)) return 1;
	len = ntoh16u (drive->rd_buf);
//	printf("data len: %4X (%4d), Header:\n", len, len);
//	for (i=0; i<8; i++) printf(" 0x%02X",drive->rd_buf[i] & 0xFF); printf("\n");
	for (i=8; (i<len) && (i<0x4000) ; i+=ml) {
		mn = drive->rd_buf[i] & 0x3F;
		ml = drive->rd_buf[i+1] & 0xFF;
		ii = 0;
		while ((MODE_PAGES[ii].id != mn) && (MODE_PAGES[ii].id < 0x3F)) ii++;
		if (!drive->silent) {
			printf("Mode Page: 0x%02X [%s]", mn, MODE_PAGES[ii].name);
#ifdef MODE_PAGES_DEBUG
			for (ii=0; ii<(ml+2); ii++) { if (!(ii%32)) printf("\n"); printf(" %02X",drive->rd_buf[i+ii] & 0xFF); }
#endif
			printf("\n");
		}
		ml += 2;
	}
	return 0;
}

int get_features_list(drive_info* drive) {
	unsigned int len, i, ii;
	unsigned short fn;
	unsigned char  fv;
	unsigned int  fl;
	if (!drive->silent) printf("\n** Reading supported features...\n");
	if (get_configuration(drive, 0 , &len, NULL, 0)) return 1;
#if 0	
//#ifdef FEATURE_DEBUG
	printf("data len: %4X (%4d), Header:\n", len, len);
	for (i=0; i<8; i++) printf(" 0x%02X",drive->rd_buf[i] & 0xFF); printf("\n");
#endif
	for (i=8; (i<len) && (i<0x8000); i+=fl) {
		fn = ntoh16u (drive->rd_buf+i);
		fv = drive->rd_buf[i+2];
		fl = (unsigned int) drive->rd_buf[i+3];
		ii = 0;
		while ((FEATURES[ii].id != fn) && (FEATURES[ii].id < 0xFFFF)) ii++;
		if (!drive->silent) {
			printf("Feature: 0x%04X, ver %2X [%s]", fn, fv, FEATURES[ii].name);
#ifdef FEATURE_DEBUG
			for (ii=0; ii<(fl+4); ii++) {
				if (!(ii%32)) printf("\n");
				printf(" %02X",drive->rd_buf[i+ii] & 0xFF);
			}
#endif
			printf("\n");
		}
		fl += 4;
	}
	return 0;
}

int get_profiles_list(drive_info* drive) {
	unsigned int len, i, ii;
	unsigned short profile;
	if (!drive->silent) printf("\n** Reading supported profiles...\n");
	if (get_configuration(drive, FEATURE_PROFILE_LIST , &len, NULL, 0x02)) return 1;

	for( i = 0; (i < len-8) && (i<0x8000); i+=4 ) {
		profile = ntoh16u (drive->rd_buf+i+12);
		ii=0;
		while ((PROFILES[ii].id != profile) && (PROFILES[ii].id < 0xFFFF)) ii++;
		if (!drive->silent) printf("Profile: 0x%04X [%s]\n", profile, PROFILES[ii].name);

#if 0
		switch (profile) {
			case 0x08:						// CD-ROM
				drive->rd_capabilities |= DEVICE_CD_ROM;
				drive->wr_capabilities |= DEVICE_CD_ROM;
				break;
			case 0x09:						// CD-R
				drive->rd_capabilities |= DEVICE_CD_R;
				drive->wr_capabilities |= DEVICE_CD_R;
				break;
			case 0x0A:						// CD-RW
				drive->rd_capabilities |= DEVICE_CD_RW;
				drive->wr_capabilities |= DEVICE_CD_RW;
				break;
			case 0x10:						// DVD-ROM
				drive->rd_capabilities |= DEVICE_DVD_ROM;
				break;
			case 0x11:						// DVD-R Sequential
				drive->rd_capabilities |= DEVICE_DVD_R;
				drive->wr_capabilities |= DEVICE_DVD_R;
				break;
			case 0x12:						// DVD-RAM
				drive->rd_capabilities |= DEVICE_DVD_RAM;
//				drive->wr_capabilities |= DEVICE_DVD_RAM;
				break;
			case 0x13:						// DVD-RW Restricted Overwrite
				drive->rd_capabilities |= DEVICE_DVD_RW;
				drive->wr_capabilities |= DEVICE_DVD_RW;
				break;
			case 0x14:						// DVD-RW Sequential
				drive->rd_capabilities |= DEVICE_DVD_RW;
				drive->wr_capabilities |= DEVICE_DVD_RW;
				break;
			case 0x15:						// DVD-R DL Sequential
				drive->rd_capabilities |= DEVICE_DVD_R_DL;
				drive->wr_capabilities |= DEVICE_DVD_R_DL;
				break;
			case 0x16:						// DVD-R DL Layer Jump
				drive->rd_capabilities |= DEVICE_DVD_R_DL;
				drive->wr_capabilities |= DEVICE_DVD_R_DL;
				break;
			case 0x17:						// DVD-RW DL
				drive->rd_capabilities |= DEVICE_DVD_RW_DL;
				drive->wr_capabilities |= DEVICE_DVD_RW_DL;
				break;
			case 0x1A:						// DVD+RW
				drive->rd_capabilities |= DEVICE_DVD_PLUS_RW;
				drive->wr_capabilities |= DEVICE_DVD_PLUS_RW;
				break;
			case 0x1B:						// DVD+R
				drive->rd_capabilities |= DEVICE_DVD_PLUS_R;
				drive->wr_capabilities |= DEVICE_DVD_PLUS_R;
				break;
			case 0x2A:						// DVD+RW DL
				drive->rd_capabilities |= (DEVICE_DVD_PLUS_RW_DL);
				drive->wr_capabilities |= (DEVICE_DVD_PLUS_RW_DL);
				break;
			case 0x2B:						// DVD+R DL
				drive->rd_capabilities |= (DEVICE_DVD_PLUS_R_DL);
				drive->wr_capabilities |= (DEVICE_DVD_PLUS_R_DL);
				break;
			case 0x40:						// BD-ROM
				drive->rd_capabilities |= (DEVICE_BD_ROM);
				break;
			case 0x41:						// BD-R Sequential
				drive->rd_capabilities |= (DEVICE_BD_R);
				drive->wr_capabilities |= (DEVICE_BD_R);
				break;
			case 0x42:						// BD-R Random
				drive->rd_capabilities |= (DEVICE_BD_R);
				drive->wr_capabilities |= (DEVICE_BD_R);
				break;
			case 0x43:						// BD-RE
				drive->rd_capabilities |= (DEVICE_BD_RE);
				drive->wr_capabilities |= (DEVICE_BD_RE);
				break;
		}
#endif
	}
	profile = ntoh16(drive->rd_buf+6);
	ii=0;
	while ((PROFILES[ii].id != profile) && (PROFILES[ii].id < 0xFFFF)) ii++;
	if (!drive->silent) printf("Current: 0x%04X [%s]\n", profile, PROFILES[ii].name);

	return 0;
}

void detect_capabilities(drive_info* drive){
	unsigned int	len=4;
	drive->capabilities=CAP_SET_CD_SPEED;
	drive->rd_capabilities=0;
	drive->wr_capabilities=0;
	detect_mm_capabilities(drive);
	detect_iface(drive);
	get_drive_serial_number(drive);
	get_mode_pages_list(drive);
	if (drive->mmc>1) {
		// LU is MMC2 or later. Detecting capabilities by GET_CONFIGURATION
		get_profiles_list(drive);
		get_features_list(drive);
		get_configuration(drive, FEATURE_REMOVABLE_MEDIA , &len, NULL);
		if (len >= 0x0C) drive->capabilities|=CAP_REMOVABLE_MEDIA;
		get_configuration(drive, FEATURE_SMART , &len, NULL);
		if (len >= 0x08) drive->capabilities|=CAP_SMART;
		get_configuration(drive, FEATURE_MICROCODE_UPGRADE , &len, NULL);
		if (len >= 0x08) drive->capabilities|=CAP_MICROCODE_UPGRADE;
		get_configuration(drive, FEATURE_MORPHING , &len, NULL);
		if (len >= 0x08) drive->capabilities|=CAP_MORPHING;
		get_configuration(drive, FEATURE_POWER_MANAGEMENT , &len, NULL);
		if (len >= 0x08) drive->capabilities|=CAP_POWER_MANAGEMENT;
		get_configuration(drive, FEATURE_EMBEDDED_CHANGER , &len, NULL);
		if (len >= 0x08) drive->capabilities|=CAP_EMBEDDED_CHANGER;
		get_configuration(drive, FEATURE_DEFECT_MANAGEMENT , &len, NULL);
		if (len >= 0x08) {
			drive->capabilities|=CAP_DEFECT_MANAGEMENT;
			if (len >= 0x0C) {
				if (drive->rd_buf[12]&0x80) drive->capabilities|=CAP_SSA;
			}
		}
		get_configuration(drive, FEATURE_REAL_TIME_STREAMING , &len, NULL);
		if (len >= 0x08) drive->capabilities|=CAP_REAL_TIME_STREAMING;
		get_configuration(drive, FEATURE_MRW , &len, NULL);
		if (len >= 0x0C) {
			drive->rd_capabilities|=DEVICE_MRW;
			if (drive->rd_buf[12]&0x01) drive->wr_capabilities|=DEVICE_MRW;
			if (drive->rd_buf[12]&0x02) drive->rd_capabilities|=DEVICE_MRW_DVD;
			if (drive->rd_buf[12]&0x04) drive->wr_capabilities|=DEVICE_MRW_DVD;
		}
		get_configuration(drive, FEATURE_CD_READ , &len, NULL); // LU can operate with CD's
		if (len >= 0x0C) {
//			drive->rd_capabilities|=DEVICE_CD_ROM;
			if (drive->rd_buf[12]&0x1) drive->capabilities|=CAP_CD_TEXT;
			if (drive->rd_buf[12]&0x2) drive->capabilities|=CAP_C2;
		}
		get_configuration(drive, FEATURE_DDCD_READ , &len, NULL);  // LU can Reed/Write DDCD's
		if (len >= 0x0C) {
			drive->rd_capabilities|=DEVICE_DDCD_R;
			drive->rd_capabilities|=DEVICE_DDCD_RW;
			get_configuration(drive, FEATURE_DDCD_R_WRITE , &len, NULL);
			if (len >= 0x0C) drive->wr_capabilities|=DEVICE_DDCD_R;
			get_configuration(drive, FEATURE_DDCD_RW_WRITE , &len, NULL);
			if (len >= 0x0C) drive->wr_capabilities|=DEVICE_DDCD_RW;
		}
		get_configuration(drive, FEATURE_DVD_READ , &len, NULL); // LU can operate with DVD's
		if (len >= 0x08) {
			if ((drive->rd_buf[10] >> 2) > 0) drive->mmc=5;
			if (len >= 0x0C) {
			    if (drive->rd_buf[14]&0x01) drive->rd_capabilities|=DEVICE_DVD_R_DL;
			    if (drive->rd_buf[14]&0x02) drive->rd_capabilities|=DEVICE_DVD_RW_DL;
			}
			get_configuration(drive, FEATURE_DVD_CPRM , &len, NULL);
			if (len >= 0x08) drive->capabilities|=CAP_DVD_CPRM;
			get_configuration(drive, FEATURE_DVD_CSS , &len, NULL);
			if (len >= 0x08) drive->capabilities|=CAP_DVD_CSS;
			get_configuration(drive, FEATURE_DVD_R_RW_WRITE , &len, NULL);
			if (len >= 0x08) {
				drive->wr_capabilities|=DEVICE_DVD_R;
				if (len >= 0x0C) {
					if (drive->rd_buf[12]&0x02) drive->wr_capabilities|=DEVICE_DVD_RW;
					if (drive->rd_buf[12]&0x04) drive->capabilities|= CAP_TEST_WRITE_DVD;
					//if (drive->rd_buf[12]&0x08) drive->wr_capabilities|=DEVICE_DVD_R_DL_SEQ;
					if (drive->rd_buf[12]&0x08) drive->wr_capabilities|=DEVICE_DVD_R_DL;
				}
			}
			get_configuration(drive, FEATURE_LAYER_JUMP_RECORDING , &len, NULL);
			//if (len >= 0x08) drive->wr_capabilities|=DEVICE_DVD_R_DL_LJ;
			if (len >= 0x08) drive->wr_capabilities|=DEVICE_DVD_R_DL;

			get_configuration(drive, FEATURE_DVD_PLUS_R , &len, NULL);
			if (len >= 0x08) {
// 				drive->mmc=4;
				drive->rd_capabilities|=DEVICE_DVD_PLUS_R;
				if (drive->rd_buf[12]&0x01) drive->wr_capabilities|=DEVICE_DVD_PLUS_R;
			}
			get_configuration(drive, FEATURE_DVD_PLUS_RW , &len, NULL);
			if (len >= 0x0C) {
				drive->rd_capabilities|=DEVICE_DVD_PLUS_RW;
				if (drive->rd_buf[12]&0x01) drive->wr_capabilities|=DEVICE_DVD_PLUS_RW;
			}
			get_configuration(drive, FEATURE_DVD_PLUS_R_DOUBLE_LAYER , &len, NULL);
			if (len >= 0x0C) {
// 				drive->mmc=5;
				drive->rd_capabilities|=DEVICE_DVD_PLUS_R_DL;
				if (drive->rd_buf[12]&0x01) drive->wr_capabilities|=DEVICE_DVD_PLUS_R_DL;
			}
			get_configuration(drive, FEATURE_DVD_PLUS_RW_DOUBLE_LAYER , &len, NULL);
			if (len >= 0x0C) {
// 				drive->mmc=5;
				drive->rd_capabilities|=DEVICE_DVD_PLUS_RW_DL;
				if (drive->rd_buf[12]&0x01) drive->wr_capabilities|=DEVICE_DVD_PLUS_RW_DL;
			}
		}
		if (drive->wr_capabilities & DEVICE_DVD_RW)   drive->rd_capabilities|=DEVICE_DVD_RW;
		if (drive->wr_capabilities & DEVICE_DVD_R_DL) drive->rd_capabilities|=DEVICE_DVD_R_DL;
		get_configuration(drive, FEATURE_BD_READ , &len, NULL);
		if ((len >=0x08) && (drive->rd_buf[11] >=28) ) {
			if (drive->rd_buf[16] | drive->rd_buf[17] | drive->rd_buf[18] |	drive->rd_buf[19] |
				drive->rd_buf[20] |	drive->rd_buf[21] |	drive->rd_buf[22] |	drive->rd_buf[23])
				drive->rd_capabilities |= (DEVICE_BD_RE);
			if (drive->rd_buf[24] |	drive->rd_buf[25] |	drive->rd_buf[26] |	drive->rd_buf[27] |
				drive->rd_buf[28] |	drive->rd_buf[29] |	drive->rd_buf[30] |	drive->rd_buf[31])
				drive->rd_capabilities |= (DEVICE_BD_R);
			if (drive->rd_buf[32] |	drive->rd_buf[33] |	drive->rd_buf[34] |	drive->rd_buf[35] |
				drive->rd_buf[36] |	drive->rd_buf[37] |	drive->rd_buf[38] |	drive->rd_buf[39])
				drive->rd_capabilities |= (DEVICE_BD_ROM);
		}
		get_configuration(drive, FEATURE_BD_WRITE , &len, NULL);
		if ((len >=0x08) && (drive->rd_buf[11] >=20) ) {
			if (drive->rd_buf[16] | drive->rd_buf[17] | drive->rd_buf[18] |	drive->rd_buf[19] |
				drive->rd_buf[20] |	drive->rd_buf[21] |	drive->rd_buf[22] |	drive->rd_buf[23])
				drive->wr_capabilities |= (DEVICE_BD_RE);
			if (drive->rd_buf[24] |	drive->rd_buf[25] |	drive->rd_buf[26] |	drive->rd_buf[27] |
				drive->rd_buf[28] |	drive->rd_buf[29] |	drive->rd_buf[30] |	drive->rd_buf[31])
				drive->wr_capabilities |= (DEVICE_BD_R);
		}
		get_configuration(drive, FEATURE_HDDVD_READ , &len, NULL);
		if ((len >=0x08) ) {
			drive->rd_capabilities|=DEVICE_HDDVD_ROM;
			if ((len >=0x0C) ) {
				if (drive->rd_buf[12]&0x01) drive->rd_capabilities|=DEVICE_HDDVD_R;
				if (drive->rd_buf[14]&0x01) drive->rd_capabilities|=DEVICE_HDDVD_RAM;
			}
		}
		get_configuration(drive, FEATURE_HDDVD_WRITE , &len, NULL);
		if ((len >=0x0C) ) {
			if (drive->rd_buf[12]&0x01) drive->wr_capabilities|=DEVICE_HDDVD_R;
			if (drive->rd_buf[14]&0x01) drive->wr_capabilities|=DEVICE_HDDVD_RAM;
		}
	if (!drive->silent) printf("** Device is MMC-%d\n",drive->mmc);
	}
}

int read_atip(drive_info* drive) {
	unsigned char	data[4];
	int 	size = 0;
	int	i;
	drive->media.ATIP_size = 0;
	drive->cmd[0]=MMC_READ_TOC_PMA_ATIP;
	drive->cmd[1]=0;
	drive->cmd[2]=4; // ATIP
	drive->cmd[3]=0;
	drive->cmd[8]=4;
	drive->cmd[9]=0;
	if ((drive->err=drive->cmd.transport(READ,data,4) ))
		{if (!drive->silent) sperror ("READ_ATIP",drive->err); drive->media.ATIP_size = 0; return 1;}
	size = ntoh16u((char*)data);
	size += 2;
	drive->cmd[0]=MMC_READ_TOC_PMA_ATIP;
	drive->cmd[1]=0;
	drive->cmd[2]=4; // ATIP
	drive->cmd[3]=0;
	drive->cmd[7]=(size >> 8) & 0xFF;
	drive->cmd[8]=size & 0xFF;
	drive->cmd[9]=0;
	if ((drive->err=drive->cmd.transport(READ,drive->media.ATIP,size) ))
		{sperror ("READ_ATIP",drive->err); drive->media.ATIP_size = 0; return 1;}
	drive->media.ATIP_size = size;
	if (!drive->silent) {
		printf("ATIP (%d bytes):\n",size);
		for (i=0; i<(min(size, 4)); i++) printf(" %3d (%02X)",drive->media.ATIP[i],drive->media.ATIP[i]);
		if (size > 4) for (i=0; i<(size-4); i++) {
			if (!(i % 8)) printf("\n");
			else if (!(i % 4)) printf("      ");
			printf(" %3d (%02X)",drive->media.ATIP[i+4] & 0xFF,drive->media.ATIP[i+4] & 0xFF);
		}
		printf("\n");
	}
	return 0;
}

int read_toc(drive_info* drive) {
	unsigned char	data[4];
	int 	size = 0;
	int	i;
	drive->cmd[0]=MMC_READ_TOC_PMA_ATIP;
	drive->cmd[1]=0;
	drive->cmd[2]=0; // TOC
	drive->cmd[3]=0;
	drive->cmd[8]=4;
	drive->cmd[9]=0;
	if ((drive->err=drive->cmd.transport(READ,data,4) ))
		{sperror ("READ_TOC",drive->err); return 1;}
//	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,2048) ))
//		{sperror ("READ_TOC",drive->err); return 1;}
	size = ntoh16u((char*)data);
	size += 2;

	drive->cmd[0]=MMC_READ_TOC_PMA_ATIP;
	drive->cmd[1]=0;
	drive->cmd[2]=  0; // TOC
	drive->cmd[3]=0;
	drive->cmd[7]=(size >> 8) & 0xFF;
	drive->cmd[8]=size & 0xFF;
	drive->cmd[9]=0;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,size) ))
		{if (!drive->silent) sperror ("READ_TOC",drive->err); return 1;}

	if (!drive->silent) {
		printf("TOC (%d bytes):\n",size);
		for (i=0; i<(min(size, 4)); i++) printf(" %3d (%02X)",drive->rd_buf[i] & 0xFF,drive->rd_buf[i] & 0xFF);
		if (size > 4) for (i=0; i<(size-4); i++) {
			if (!(i % 8)) printf("\n");
			else if (!(i % 4)) printf("      ");
			printf(" %3d (%02X)",drive->rd_buf[i+4] & 0xFF,drive->rd_buf[i+4] & 0xFF);
		}
		printf("\n");
	}
	return 0;
}

int read_disc_info(drive_info* drive, int len) {
	drive->cmd[0] = MMC_READ_DISC_INFORMATION;
	drive->cmd[7] = (len >> 8) & 0xFF;
	drive->cmd[8] = len & 0xFF;
	drive->cmd[9]=0;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,len) ))
		{if (!drive->silent) sperror ("READ_DISC_INFO",drive->err); return 1;}
	return 0;
}

int read_track_info(drive_info* drive, trk* track, unsigned int track_n){
	int size = 2048;
	drive->cmd[0] = MMC_READ_TRACK_INFORMATION;
	drive->cmd[1] = 0x01;
	drive->cmd[2] = (track_n >> 24) & 0xFF;
	drive->cmd[3] = (track_n >> 16) & 0xFF;
	drive->cmd[4] = (track_n >> 8) & 0xFF;
	drive->cmd[5] = track_n & 0xFF ;
	drive->cmd[7] = (size >> 8) & 0xFF ;
	drive->cmd[8] = size & 0xFF ;
	drive->cmd[9] = 0;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,size) ))
		{if (!drive->silent) sperror ("READ_TRACK_INFO",drive->err); return 1;}

#if 0
	int i, len;
	len = ( drive->rd_buf[0] << 8 ) | drive->rd_buf[1];
	printf("\nTrack #%d info:\n  ",track->n);
	for (i=0; i<len+2; i++) { printf(" 0x%02X",drive->rd_buf[i] & 0xFF); if (!((i+1)%8)) printf("\n  ");}
	if (((i)%8)) printf("\n");
#endif

	track->n = ((drive->rd_buf[32]&0xFF) << 8) | (drive->rd_buf[2]&0xFF);
	track->session = ((drive->rd_buf[33]&0xFF) << 8) | (drive->rd_buf[3]&0xFF);
	track->track_mode = drive->rd_buf[5] & 0x0F;
	track->data_mode = drive->rd_buf[6] & 0x0F;
	track->start = ntoh32(drive->rd_buf+8);
	track->next_writable = ntoh32(drive->rd_buf+12);
	track->free = ntoh32(drive->rd_buf+16);
	track->size = ntoh32(drive->rd_buf+24);
	track->last = ntoh32(drive->rd_buf+28);
//	track->end = track->start+track->size-1;

	lba2msf(track->start,&track->msf_start);
	lba2msf(track->next_writable,&track->msf_next);
	lba2msf(track->free,&track->msf_free);
	lba2msf(track->size,&track->msf_size);
	lba2msf(track->last,&track->msf_last);
//	lba2msf(track->end,&track->msf_end);

	return 0;
}

int get_track_list(drive_info* drive){
	int i;
	int tf,tl;

	if (read_track_info(drive, &drive->media.track[0], 1)) {
//		if (!drive->silent)
			printf("READ TRACK INFO failed! Trying to read TOC...\n");
		read_toc(drive);
		tf = drive->rd_buf[2];
		tl = drive->rd_buf[3];
		drive->media.tracks = tl - tf +1;
		for (i=0; i<drive->media.tracks; i++) {
			drive->media.track[i].n = i+1;
			drive->media.track[i].session = 1;
			drive->media.track[i].start = ntoh32(drive->rd_buf+i*8+8);
			drive->media.track[i].last  = ntoh32(drive->rd_buf+i*8+16);
			drive->media.track[i].size = drive->media.track[i].last - drive->media.track[i].start + 1;
			drive->media.track[i].free = 0;
			drive->media.track[i].track_mode = drive->rd_buf[i*8+5] & 0x0F;
//			drive->media.track[i].data_mode = 0;
			drive->media.track[i].data_mode = (drive->rd_buf[i*8+5] >> 4) & 0x0F;

			lba2msf(drive->media.track[i].start, &drive->media.track[i].msf_start);
			lba2msf(drive->media.track[i].next_writable, &drive->media.track[i].msf_next);
			lba2msf(drive->media.track[i].last, &drive->media.track[i].msf_last);
			lba2msf(drive->media.track[i].free,  &drive->media.track[i].msf_free);
//			lba2msf(drive->media.track[i].end,  &drive->media.track[i].msf_end);
			lba2msf(drive->media.track[i].size, &drive->media.track[i].msf_size);
		}
	} else {
		read_disc_information(drive);
		for (i=0; i<drive->media.tracks; i++)
			read_track_info(drive, &drive->media.track[i], i+1);
	}
	if (!drive->silent) printf("tracks: %d\n",drive->media.tracks);
	if ((drive->media.tracks) && (!drive->silent)) for (i=0; i<drive->media.tracks; i++) {
			printf("\nTrack #  : %d\n",   drive->media.track[i].n);
			printf("Session #: %d\n",     drive->media.track[i].session);
			printf("Track mode    : %d\n",drive->media.track[i].track_mode);
			printf("Data mode     : %d\n",drive->media.track[i].data_mode);
			printf("Track start   : %d\n",drive->media.track[i].start);
			printf("Next writable : %d\n",drive->media.track[i].next_writable);
			printf("Free          : %d\n",drive->media.track[i].free);
			printf("Size          : %d\n",drive->media.track[i].size);
			printf("Last recorded : %d\n",drive->media.track[i].last);
	}
	return 0;
}

int read_capacity(drive_info* drive) {
	unsigned char data[8]; memset(data, 0, 8);
	drive->cmd[0] = SBC_READ_CAPACITY;
	drive->cmd[9] = 0;
	if ((drive->cmd.transport (READ,data,8))) {
		if (!drive->silent) sperror ("READ_CAPACITY",drive->err);
		drive->media.capacity = 0;
		drive->media.sectsize = 2048;
	} else {
		drive->media.capacity = ntoh32(data);
		if(drive->media.capacity) drive->media.capacity++;
		drive->media.sectsize = ntoh32(data+4);
	}
	lba2msf(drive->media.capacity, &drive->media.capacity_msf);
	return 0;
}

int read_capacity_free(drive_info* drive) {
	trk track;
	if (drive->media.dstatus == 2) {
		drive->media.capacity_free = 0;
		return 0;
	}

	read_disc_information(drive);
	if  (!read_track_info(drive, &track, drive->media.tracks)) {
		//printf("track %d free: %d\n",drive->media.tracks,track.free);

		drive->media.capacity_free = track.free;
		lba2msf(drive->media.capacity_free,&drive->media.capacity_free_msf);
		return 0;
	}
/*
	if (drive->media.type & DISC_CD) {
	//if (0) {
		if (((drive->media.last_lead_out >> 24) & 0xFF ) == 0xFF) {
			drive->media.capacity_free = 0;
			lba2msf(drive->media.capacity_free,&drive->media.capacity_free_msf);
			return 0;
		} else {
			drive->media.capacity_free = drive->media.last_lead_out - drive->media.capacity - 150 - 2;
			lba2msf(drive->media.capacity_free,&drive->media.capacity_free_msf);
			return 0;
		}
	} else if (drive->media.type & DISC_DVD) {
//		track.n = drive->media.sessions+1;
		read_track_info(drive, &track, drive->media.sessions+1);
		drive->media.capacity_free = track.free;
		lba2msf(drive->media.capacity_free,&drive->media.capacity_free_msf);
		return 0;
	}
*/
	drive->media.capacity_free = 0;
	return 1;
}

int read_capacity_total(drive_info* drive) {
	unsigned int	len;
//	char		header[40];
	unsigned int  phsta=0, phend=0;
//	union { unsigned char _e[4+40],_11[4+256]; } dvd;
	drive->media.capacity_total = 0;
	int doffs;

	if (drive->media.type & DISC_CD) {
		msf	lout;
		int	ilout;
//		int atippr = read_atip(drive);
//		if (!atippr) drive->ATIP_len+=4; else return 1;
		if (!drive->media.ATIP_size) {
			drive->media.capacity_total = drive->media.capacity + drive->media.capacity_free;
			return 0;
		} else {
			lout.m=drive->media.ATIP[12];
			lout.s=drive->media.ATIP[13];
			lout.f=drive->media.ATIP[14];
			ilout = msf2lba(lout);
			if (!drive->silent) printf("CD-R(W) Lead-Out: %02d:%02d.%02d\n", lout.m, lout.s, lout.f);
			drive->media.capacity_total = ilout-150;

//			lba2msf(drive->media.capacity_total, &lout);
//			ilout = msf2lba(lout);
//			if (!drive->silent) printf("CD-R(W) Capacity: %02d:%02d.%02d (sector %d)\n", lout.m, lout.s, lout.f, ilout);

			return 0;
		}
	} else if (drive->media.type & (DISC_DVDminus)) {
		if (drive->media.type & (DISC_DVDmRWR | DISC_DVDmRWS | DISC_DVDmRWDL)) {
	// DVD-RW
			len= 12+256;
			drive->cmd[0] = MMC_READ_FORMAT_CAPACITIES;
			drive->cmd[7] = len>>8;
			drive->cmd[8] = len & 0xFF;
			if ((drive->err = drive->cmd.transport(READ,drive->rd_buf,len)))
				{if (!drive->silent) sperror ("READ_FORMAT_CAPACITIES",drive->err); goto read_total_dvdmr; }

			printf ( "Capacity descriptors: %02x\n",drive->rd_buf[3] >> 3);
			doffs = 12;
			while (!drive->media.capacity_total) {
				if (!(drive->rd_buf[doffs+4] >> 2))
				drive->media.capacity_total = ntoh32(drive->rd_buf+doffs);
			}
			if (!drive->media.capacity_total) {
				switch(drive->rd_buf[8] & 0x03) {
					case 0:
						printf("* Reserved\n");
						goto read_total_dvdmr;
					case 1:
						printf("* Unformatted or blank media\n");
						break;
					case 2:
						printf("* Formatted media\n");
						break;
					case 3:
						printf("* No media or unknown capacity\n");
						goto read_total_dvdmr;
				}
				drive->media.capacity_total = ntoh32(drive->rd_buf+4);
			}
			return 0;
		} else {
	// DVD-R
read_total_dvdmr:
			len= 44;
			drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
			drive->cmd[7] = 0x10;
			drive->cmd[8] = len>>8;
			drive->cmd[9] = len & 0xFF;
			drive->cmd[11] = 0;
			if ((drive->err = drive->cmd.transport(READ,drive->rd_buf,len)))
				{if (!drive->silent) sperror ("READ_DVD_STRUCTURE 10",drive->err); return 1;}

			phsta = ntoh32(drive->rd_buf+8);
			if ((drive->rd_buf[6] & 0x60) == 0)
				phend = ntoh32(drive->rd_buf+12);
			else
				phend = ntoh32(drive->rd_buf+16);

			drive->media.capacity_total = phend - phsta + 1;
			return 0;
		}
	} else if (drive->media.type & (DISC_DVDplus | DISC_BD)) {
		if (drive->media.type & (DISC_DVDpRW | DISC_DVDpRWDL)) {
#warning DVD+RW total sectors reading
		} else {
			len= 44;
			drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
			drive->cmd[7] = 0x00;
			drive->cmd[8] = len>>8;
			drive->cmd[9] = len & 0xFF;
			drive->cmd[11] = 0;
			if ((drive->err = drive->cmd.transport(READ,drive->rd_buf,len)))
				{if (!drive->silent) sperror ("READ_DVD_STRUCTURE 00",drive->err); return 1;}

			phsta = ntoh32(drive->rd_buf+8);
			if ((drive->rd_buf[6] & 0x60) == 0)
				phend = ntoh32(drive->rd_buf+12);
			else
				phend = ntoh32(drive->rd_buf+16);
	
			drive->media.capacity_total = phend - phsta + 1;
//			printf("Phy start: %6X %d\n",phsta,phsta);
//			printf("Phy end  : %6X %d\n",phend,phend);
			return 0;

		}
	} else if (drive->media.type & DISC_DVDRAM) {
		drive->media.capacity_total = drive->media.capacity + drive->media.capacity_free;
		return 0;
	}
	printf("Unknown media type, can't get available total capacity! Assuming value from READ_CAPACITY.\n");
	drive->media.capacity_total = drive->media.capacity + drive->media.capacity_free;
	return 0;
}

int read_spare_capacities(drive_info* drive) {
	if (!(drive->media.type & (DISC_DVDRAM | DISC_HDDVD_RAM) )) {
		drive->media.spare_psa_total = 0;
		drive->media.spare_psa_free = 0;
		drive->media.spare_ssa_total = 0;
		drive->media.spare_ssa_free = 0;
		return 0;
	}

	drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
	drive->cmd[7] = 0x00;
	drive->cmd[8] = 0;
	drive->cmd[9] = 16;
	drive->cmd[11] = 0;
	if ((drive->err = drive->cmd.transport(READ,drive->rd_buf,20)))
		if (!drive->silent) { sperror ("READ_DVD_STRUCTURE 00",drive->err); return 1;}
	drive->media.spare_psa_total = (drive->rd_buf[5]&0xF0) ? 5120 : 12800;

	drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
	drive->cmd[7] = 0x0A;
	drive->cmd[8] = 0;
	drive->cmd[9] = 16;
	drive->cmd[11] = 0;
	if ((drive->err = drive->cmd.transport(READ,drive->rd_buf,16)))
		if (!drive->silent) { sperror ("READ_DVD_STRUCTURE 0A",drive->err); return 1;}

	drive->media.spare_psa_free  = ntoh32(drive->rd_buf+4);
	drive->media.spare_ssa_free  = ntoh32(drive->rd_buf+8);
	drive->media.spare_ssa_total = ntoh32(drive->rd_buf+12);
	return 0;
}
	
int read_capacities(drive_info* drive) {
	if (!drive->media.type) return 0;
//	read_disc_info(drive, 24);
//	drive->media.last_lead_out = (drive->rd_buf[20]&0xFF) << 24 | drive->rd_buf[21]*75*60 + drive->rd_buf[22]*75 + drive->rd_buf[23];
	if (read_capacity(drive))       printf("Error reading used capacity\n");
	if (read_capacity_free(drive))  printf("Error reading free capacity\n");
	if (read_capacity_total(drive)) {
//		printf("Error reading total capacity\n");
		drive->media.capacity_total = drive->media.capacity + drive->media.capacity_free;
	}
	read_spare_capacities(drive);
	return 0;
}

int read_disc_information(drive_info* drive) {
	int i=0,len=0;
	drive->cmd[0] = MMC_READ_DISC_INFORMATION;
	drive->cmd[7] = 0x08;
	drive->cmd[8] = 0x00;
	drive->cmd[9] = 0x00;
	drive->cmd.transport (READ,drive->rd_buf,2048);
	len= (drive->rd_buf[0]<<8)|drive->rd_buf[1];
	if (!drive->silent) printf("Disc info length: 0x%04X\n  ",len);
	if (!drive->silent) for (i=0; i<len+2; i++) {
		printf(" 0x%02X",drive->rd_buf[i] & 0xFF);
		if (!((i+1)%8)) printf("\n  ");
	}
	if (((i)%8)) printf("\n");
//	if (len < 0x20) {
	if (len < 0x16) {
		drive->media.erasable = 0;
		drive->media.dstatus = 0;
		drive->media.sstatus = 0;
		drive->media.sessions = 0;
		drive->media.tracks = 0;
		return 1;
	}
	drive->media.erasable = (drive->rd_buf[2]&0x10);
	drive->media.dstatus = drive->rd_buf[2]&0x03;
	drive->media.sstatus = (drive->rd_buf[2]>>2)&0x03;
	drive->media.sessions = (drive->rd_buf[4]|(drive->rd_buf[9]<<8));
// 	if (!drive->media.sstatus) drive->media.sessions--;
	drive->media.tracks = drive->rd_buf[6]|(drive->rd_buf[11]<<8);
	if (!drive->silent) {
		printf("   first track# on disc: %d\n", drive->rd_buf[3]);
		printf("   first track# in last session: %d\n", drive->rd_buf[5]|(drive->rd_buf[10]<<8));
		printf("   last  track# in last session: %d\n", drive->media.tracks);
		printf("   disc type: %02X\n", drive->rd_buf[8]&0xFF);
		printf("   disc ID: %08X\n", ntoh32(drive->rd_buf[12]));
		printf("   Last session  lead-in  start: %d:%02d.%02d\n",
			(drive->rd_buf[16]<<8)|drive->rd_buf[17],drive->rd_buf[18],drive->rd_buf[19]);
		drive->media.last_lead_out = ((drive->rd_buf[20]&0xFF) << 24) | (drive->rd_buf[21]*75*60 + drive->rd_buf[22]*75 + drive->rd_buf[23]);
		printf("   Last possible lead-out start: %d:%02d.%02d (sector 0x%08X)\n",
			(drive->rd_buf[20]<<8)|drive->rd_buf[21],drive->rd_buf[22],drive->rd_buf[23],drive->media.last_lead_out);
	}
/*
	if (!drive->media.sstatus) {
		drive->media.sessions--;
		drive->media.tracks--;
	}
*/
	return 0;
}

int determine_cd_type(drive_info* drive) {
//	unsigned char*	ATIP;
//	int		ATIP_len;
//	int		i;
	int ratip = read_atip(drive);
	if (ratip) {
		if (!drive->silent) printf("no ATIP found, assuming disc type: CD-ROM\n");
		return DISC_CDROM; // CD-ROM
//	} else {
//		drive->ATIP_len += 4;
	}
	if (drive->media.ATIP_size < 8) {
		if (!drive->silent) printf("ATIP too small, assuming disc type: CD-ROM\n");
		return DISC_CDROM; // CD-ROM
	}
/*
	printf("ATIP_len=%d\nATIP data:",drive->ATIP_len);
	for (i=0; i< drive->ATIP_len; i++) printf("%4d",(drive->ATIP[i])&0xFF);
	printf("\n");
*/
	// CD-RW?
	int cdrw = !!(drive->media.ATIP[6] & 0x40);
	int cd_subtype = (drive->media.ATIP[6] & 0x38);
	if (cdrw) {
		if (!drive->silent) printf("disc type: CD-RW\n");
		return (DISC_CDRW | cd_subtype);
	}
	if (!drive->silent) printf("disc type: CD-R\n");
	return (DISC_CDR | cd_subtype);
}

int read_mediaid_bd(drive_info* drive) {
	int		i;
	unsigned int	len;
	memset(drive->media.MID, 0, 20);

	for (int i=0; i<speed_tbl_size;i++) drive->parms.wr_speed_tbl_media[i] = -1;

	drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
	drive->cmd[1] = 1; // media type = BD
	drive->cmd[7] = 0x00;
	drive->cmd[8] = 0;
	drive->cmd[9] = 4;
	drive->cmd[11] = 0;
	if ((drive->err = drive->cmd.transport(READ,drive->rd_buf,4)))
		{if (!drive->silent) sperror ("READ_BD_STRUCTURE 00",drive->err); return 1;}
	len = (drive->rd_buf[0]<<8|drive->rd_buf[1]) + 2;
	if (len>128) len= 128;
	drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
	drive->cmd[1] = 1; // media type = BD
	drive->cmd[7] = 0x00;
	drive->cmd[8] = len>>8;
	drive->cmd[9] = len & 0xFF;
	drive->cmd[11] = 0;
	if ((drive->err = drive->cmd.transport(READ,drive->media.MID_raw,len)))
		{if (!drive->silent) sperror ("READ_BD_STRUCTURE 00",drive->err); return 1;}
	drive->media.MID_size = len-4;

	if(drive->media.MID_raw[4] != 'D' || drive->media.MID_raw[4+1] != 'I') {
		printf("READ_BD_STRUCTURE: got some data, but not disc info\n");
		return 2;
	}
	drive->media.disc_size = drive->media.MID_raw[4+11] >> 6;
	drive->media.polarity = drive->media.MID_raw[4+14];

	if (drive->media.type & DISC_BD_ROM) {
		if (!drive->silent)
			printf(COL_YEL "BD-ROM does not contain media ID" COL_NORM "\n");
		return 0;
	}
	memcpy(drive->media.MID, drive->media.MID_raw + 4+100,6);
	i = strlen(drive->media.MID);
	drive->media.MID[i++]='-';
	memcpy(drive->media.MID+i, drive->media.MID_raw + 4+100+6,3);
	i = strlen(drive->media.MID);
	drive->media.MID[i++]='/';
	sprintf(drive->media.MID+i, "%03d", drive->media.MID_raw[4+111]);

	drive->media.MID_type = MID_type_BD;
	return 0;
}

int read_mediaid_dvd(drive_info* drive){
	memset(drive->media.MID, 0, 20);
	for (int i=0; i<speed_tbl_size;i++) drive->parms.wr_speed_tbl_media[i] = -1;
	//
	if (drive->media.type & DISC_DVDRAM) {
		read_mediaid_dvdram(drive);
		if (!drive->err) return 0;
	} if (drive->media.type & DISC_DVDminus) {
		read_mediaid_dvdminus(drive);
		if (!drive->err) return 0;
	} else if (drive->media.type & DISC_DVDplus) {
		read_mediaid_dvdplus(drive);
		if (!drive->err) return 0;
#if 1
	} else {
		// try to read Media ID even media is reported as DVD-ROM
		if (read_mediaid_dvdplus(drive))
			return read_mediaid_dvdminus(drive);
#endif
	}
	return 0;
}

int read_mediaid_dvdram(drive_info* drive) {
	int		i;
	unsigned int	len;
	char		header[40];
//	union { unsigned char _e[4+40],_11[4+256]; } dvd;

	drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
	drive->cmd[7] = 0x00;
	drive->cmd[8] = 0;
	drive->cmd[9] = 4;
	drive->cmd[11] = 0;
	if ((drive->err = drive->cmd.transport(READ,header,4)))
		{if (!drive->silent) sperror ("READ_DVD_STRUCTURE 00",drive->err); return 1;}
	len = (header[0]<<8|header[1]) + 2;
	if (len>762) len= 762;
	drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
	drive->cmd[7] = 0x00;
	drive->cmd[8] = len>>8;
	drive->cmd[9] = len & 0xFF;
	drive->cmd[11] = 0;
	if ((drive->err = drive->cmd.transport(READ,drive->media.MID_raw,len)))
		{if (!drive->silent) sperror ("READ_DVD_STRUCTURE 00",drive->err); return 1;}

	drive->media.MID_size = len-4;

	memcpy(drive->media.MID, drive->media.MID_raw + 4+597,16);

	for (i=0; i<12; i++) if(drive->media.MID[i] == 0) drive->media.MID[i]=0x20;

	drive->media.MID_type = MID_type_DVDRAM;
	return 0;
}

int read_mediaid_dvdminus(drive_info* drive){
	int		i;
	unsigned int	len;
	char		header[40];
//	union { unsigned char _e[4+40],_11[4+256]; } dvd;

	drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
	drive->cmd[7] = 0x0e;
	drive->cmd[8] = 0;
	drive->cmd[9] = 4;
	drive->cmd[11] = 0;
	if ((drive->err = drive->cmd.transport(READ,header,4)))
		{if (!drive->silent) sperror ("READ_DVD_STRUCTURE 0E",drive->err); return 1;}
	len = (header[0]<<8|header[1]) + 2;
	if (len>68) len= 68;
	drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
	drive->cmd[7] = 0x0e;
	drive->cmd[8] = len>>8;
	drive->cmd[9] = len & 0xFF;
	drive->cmd[11] = 0;
	if ((drive->err = drive->cmd.transport(READ,drive->media.MID_raw,len)))
		{if (!drive->silent) sperror ("READ_DVD_STRUCTURE 0E",drive->err); return 1;}

	drive->media.MID_size = len-4;

	memcpy(drive->media.MID, drive->media.MID_raw + 4+17,6);
	i = strlen(drive->media.MID);
	drive->media.MID[i++]='-';
	memcpy(drive->media.MID+i,  drive->media.MID_raw + 4+25,6);
/*
	i = strlen(drive->media.MID);
	drive->media.MID[i++]='-';
	memcpy(drive->media.MID+i,  drive->media.MID_raw + 4+33,6);
*/
	drive->media.MID_type = MID_type_DVDm;
	return 0;
}

int read_mediaid_dvdplus(drive_info* drive){
	int		i;
	unsigned int	len;
	char		header[40];

	drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
	drive->cmd[7] = 0x11;
	drive->cmd[8] = 0;
	drive->cmd[9] = 4;
	drive->cmd[11] = 0;
	if ((drive->err = drive->cmd.transport(READ,header,4)))
		{if (!drive->silent) sperror ("READ_DVD_STRUCTURE 11",drive->err); return 1;}
	len = (header[0]<<8|header[1]) + 2;
	if (len>260) len= 260;
	drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
	drive->cmd[7] = 0x11;
	drive->cmd[8] = len>>8;
	drive->cmd[9] = len & 0xFF;
	drive->cmd[11] = 0;
	if ((drive->err = drive->cmd.transport(READ,drive->media.MID_raw,len)))
		{if (!drive->silent) sperror ("READ_DVD_STRUCTURE 11",drive->err); return 1;}

	drive->media.MID_size = len-4;

	memcpy(drive->media.MID, drive->media.MID_raw + 4+19,8);
	i = strlen(drive->media.MID);
	drive->media.MID[i++]='-';
	memcpy(drive->media.MID+i, drive->media.MID_raw + 4+27,3);
	i = strlen(drive->media.MID);
	drive->media.MID[i++]='/';
	sprintf(drive->media.MID+i, "%03d", drive->media.MID_raw[4+30]);

	drive->media.MID_type = MID_type_DVDp;

//  process manufacturer defined write speeds...
	int wr_idx = 0;
	if (drive->media.MID_raw[4+31] > 32) {
		drive->parms.wr_speed_tbl_media[wr_idx] =  drive->media.MID_raw[4+32] / 14.58;
		wr_idx++;
	}
	for (int i=0; i<6;i++) {
		//if ((drive->media.MID_raw[4+18] & (1 << i)) && (drive->media.MID_raw[4+64+i*32] == (i+1) )) {
		if ((drive->media.MID_raw[4+18] & (1 << i)) && (drive->media.MID_raw[4+64+i*32] >0)) {
		//	printf("EI #%d\n",i);
			drive->parms.wr_speed_tbl_media[wr_idx] =  drive->media.MID_raw[4+67+i*32] / 13.95;
			wr_idx++;
		}
	}
	return 0;
}

int read_mediaid_cd(drive_info* drive)
{
//	printf("read_mediaid_cd()\n");
	msf	lin;
	int type;
//	int atippr = read_atip(drive);
//	if (!atippr) drive->ATIP_len+=4; else return 1;
	if (!drive->media.ATIP_size) return 1;
	lin.m=drive->media.ATIP[8];
	lin.s=drive->media.ATIP[9];
	lin.f=drive->media.ATIP[10];
	type = lin.f % 10;
	lin.f -= type;
	int idx=0;
	int nf=1;
	while (mi[idx].lin.m && nf)
		if (lin.m == mi[idx].lin.m && lin.s == mi[idx].lin.s && lin.f == mi[idx].lin.f) nf=0;
		else idx++;
	
//	strncpy(drive->media.MID,mi[idx].name,47));
	sprintf(drive->media.MID,"[%02d:%02d.%02d] %s",lin.m,lin.s,lin.f,mi[idx].name);
	drive->media.MID_type = MID_type_CD;
	return 0;
}

int determine_disc_type(drive_info* drive) {
//	int current = 0;
//	int i=0;
	drive->media.type = DISC_NODISC;
	drive->media.book_type = 0;
	drive->media.layers = 1;
	drive->media.max_rate  = 0x0F;
	drive->media.disc_size = 0;
	drive->media.MID_type = MID_type_NONE;
	drive->media.MID_size = 0;
// 	drive->media.type = Media_NoMedia;
	if (drive->mmc>1) {
		get_configuration(drive, FEATURE_PROFILE_LIST, NULL, 0);
		switch (drive->rd_buf[7]) {
			case 0: drive->media.type = DISC_NODISC; break;
			case PROFILE_CD_ROM:		 drive->media.type = DISC_CDROM; break;
			case PROFILE_CD_R:			 drive->media.type = DISC_CDR; break;
			case PROFILE_CD_RW:			 drive->media.type = DISC_CDRW; break;
			case PROFILE_DVD_ROM:		 drive->media.type = DISC_DVDROM; break;
			case PROFILE_DVD_R_SEQ:		 drive->media.type = DISC_DVDmR; break;
			case PROFILE_DVD_RAM:		 drive->media.type = DISC_DVDRAM; break;
			case PROFILE_DVD_RW_RESTOV:	 drive->media.type = DISC_DVDmRWR; break;
			case PROFILE_DVD_RW_SEQ:	 drive->media.type = DISC_DVDmRWS; break;
			case PROFILE_DVD_RW_DL:		 drive->media.type = DISC_DVDmRWDL; break;
			case PROFILE_DVD_R_DL_SEQ:	 drive->media.type = DISC_DVDmRDL; break;
			case PROFILE_DVD_R_DL_JUMP:	 drive->media.type = DISC_DVDmRDLJ; break;
			case PROFILE_DVD_PLUS_RW:	 drive->media.type = DISC_DVDpRW; break;
			case PROFILE_DVD_PLUS_R:	 drive->media.type = DISC_DVDpR; break;
			case PROFILE_DVD_PLUS_R_DL:	 drive->media.type = DISC_DVDpRDL; break;
			case PROFILE_DVD_PLUS_RW_DL: drive->media.type = DISC_DVDpRWDL; break;

			case PROFILE_BD_ROM:		 drive->media.type = DISC_BD_ROM; break;
			case PROFILE_BD_R_SEQ:		 drive->media.type = DISC_BD_R_SEQ; break;
			case PROFILE_BD_R_RND:		 drive->media.type = DISC_BD_R_RND; break;
			case PROFILE_BD_RE:			 drive->media.type = DISC_BD_RE; break;

			case PROFILE_HDDVD_ROM:		 drive->media.type = DISC_HDDVD_ROM; break;
			case PROFILE_HDDVD_R:		 drive->media.type = DISC_HDDVD_R; break;
			case PROFILE_HDDVD_RAM:		 drive->media.type = DISC_HDDVD_RAM; break;
			case PROFILE_HDDVD_RW:		 drive->media.type = DISC_HDDVD_RW; break;
			case PROFILE_HDDVD_R_DL:	 drive->media.type = DISC_HDDVD_RDL; break;
			case PROFILE_HDDVD_RW_DL:	 drive->media.type = DISC_HDDVD_RWDL; break;

			default: drive->media.type = DISC_UN; break;
		}
		if (!drive->media.type) return 0;
		read_disc_information(drive);
		if (drive->media.type & DISC_CD) {
			drive->media.type = determine_cd_type(drive);
			read_mediaid_cd(drive);
			if (!drive->silent) printf("** MID: '%s'\n",drive->media.MID);
			return 0;
		} else if (drive->media.type & DISC_DVD) {
 			drive->rd_buf[4]=0;
			drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
			drive->cmd[7] = 0;//0x11; //dvd_dash;
			drive->cmd[9] = 36;
			drive->cmd[11] = 0;
			if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,36))) 
				if (!drive->silent) sperror ("READ_DVD_STRUCTURE",drive->err);
			drive->media.book_type = (drive->rd_buf[4] & 0xFF);
			drive->media.max_rate  = (drive->rd_buf[5] & 0x0F);
			drive->media.disc_size = ((drive->rd_buf[5] & 0xF0) >> 4);
			drive->media.layers = 1 + ((drive->rd_buf[6] & 0x60) >> 5);
			read_mediaid_dvd(drive);
			if (!drive->silent) printf("** MID: '%s'\n",drive->media.MID);
			if ((!(drive->wr_capabilities & DEVICE_DVD)) && (drive->media.type & DISC_DVDROM)) {
				if (!drive->silent) {
					printf("Device can't write DVD's or media detected as DVD-ROM,\n");
					printf("trying to corectly detect DVD type...\n");
				}
				switch ((drive->media.book_type>>4)&0x0F){
					case BOOK_DVD_R:
						if (drive->media.layers == 1) 
							drive->media.type = DISC_DVDmR;
						else
							drive->media.type = DISC_DVDmRDL;
						break;
					case BOOK_DVD_RW:
						drive->media.type = DISC_DVDmRW; break;
					case BOOK_DVD_PR:
						drive->media.type = DISC_DVDpR; break;
					case BOOK_DVD_PRW:
						drive->media.type = DISC_DVDpRW; break;
					case BOOK_DVD_PR_DL:
						drive->media.type = DISC_DVDpRDL; break;
					case BOOK_DVD_ROM:
						switch (drive->media.MID_type) {
							case MID_type_DVDRAM:
								drive->media.type = DISC_DVDRAM;
								break;
							case MID_type_DVDm:
								if (drive->media.erasable) {
									if (drive->media.layers == 1)	drive->media.type = DISC_DVDmRW;
									else							drive->media.type = DISC_DVDmRWDL;
								} else {
									if (drive->media.layers == 1)	drive->media.type = DISC_DVDmR;
									else							drive->media.type = DISC_DVDmRDL;
								}
								break;

							case MID_type_DVDp:
								if (drive->media.erasable) {
									if (drive->media.layers == 1)	drive->media.type = DISC_DVDpRW;
									else							drive->media.type = DISC_DVDpRWDL;
								} else {
									if (drive->media.layers == 1)	drive->media.type = DISC_DVDpR;
									else							drive->media.type = DISC_DVDpRDL;
								}
								break;
							default:
								break;
						}
						break;
					default:
						break;
				}
			}
			if (drive->media.type & DISC_DVDminus)
				read_writer_info(drive);
//			if (!drive->silent) printf("** Writer used: '%s'\n",drive->media.writer);

			read_disc_regions(drive);
/*
			printf("DVD Copyright info: ");
			for (i=0;i<4;i++) printf("0x%02X ",drive->rd_buf[i]);
			printf("\n");
*/
			return 0;
		} else if (drive->media.type & DISC_BD) {
 			drive->rd_buf[4]=0;
			drive->cmd[0] = MMC_READ_DVD_STRUCTURE;
			drive->cmd[7] = 0;//0x11; //dvd_dash;
			drive->cmd[9] = 36;
			drive->cmd[11] = 0;
			if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,36))) 
				if (!drive->silent) sperror ("READ_DVD_STRUCTURE",drive->err);
			drive->media.book_type = 0;
			drive->media.layers = 1 + ((drive->rd_buf[6] & 0x60) >> 5);
			read_mediaid_bd(drive);
			if (!drive->silent) printf("** MID: '%s'\n",drive->media.MID);
		}
	} else {
		read_capacity(drive);
		if (drive->media.capacity) {
			drive->media.type = DISC_CDROM;
			read_disc_information(drive);
		}
		return 0;
	}
	return 1;
}

int get_spindown(drive_info* drive) {
	mode_sense(drive, 0x0D, 00, 192);
	if (drive->err)
		{drive->parms.spindown_idx=spindowns; return (drive->err);}
	drive->parms.spindown_idx = drive->rd_buf[11] & 0x0F;
	return 0;
}

int set_spindown(drive_info* drive) {
	memset(drive->rd_buf,0,16);
	drive->rd_buf[8] = 0x0D;
	drive->rd_buf[9] = 0x06;
	drive->rd_buf[11] = drive->parms.spindown_idx & 0x0F;
	drive->rd_buf[13] = 0x3C;
	drive->rd_buf[15] = 0x4B;
	mode_select(drive, 16);
	return 0;
}

int get_performance(drive_info* drive, bool rw, uint8_t type) {
	const int max_descs=52;
	const int desc_len=16;
	uint32_t  len, descn;
	uint32_t r,w,lba;
//	int	i;
	int	j,offs;
	drive->cmd[0] = MMC_GET_PERFORMANCE;
	drive->cmd[1] = (!type) ? (0x04 * rw) : 0x00;
	drive->cmd[8] = (max_descs >> 8) & 0xFF;
	drive->cmd[9] = max_descs & 0xFF;
	drive->cmd[10] = type;
	drive->cmd[11] = 0x00;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf, 1024)))
		{ if (!drive->silent) sperror ("GET_PERFORMANCE",drive->err); return (drive->err); }
//	return 1;
	len = ntoh32(drive->rd_buf);
	descn = len/desc_len;
//	printf("Performance data length: %d; decriptors: %d\n",len, descn);

//	printf("GET_PERFORMANCE response dump:\n");
//	for (j=0; j*8<len; j++) {
//		for (i=0; i<8; i++) printf(" 0x%02X",drive->rd_buf[i]);
//		printf("\n");
//	}

	if (type == 0x03) for (j=0; j<speed_tbl_size;j++) drive->parms.wr_speed_tbl_kb[j] = -1;

	for (j=0; j<min(descn, (type==0x03) ? speed_tbl_size : descn); j++) {
		offs = 8+j*desc_len;
//		printf("\t%s descriptor #%02d:", rw ? "Write" : "Read",j);

		if (!type) {
			drive->perf.lba_s = ntoh32(drive->rd_buf+offs);
			offs = 8+j*desc_len+4;
			drive->perf.spd_s = ntoh32(drive->rd_buf+offs);
			offs = 8+j*desc_len+8;
			drive->perf.lba_e = ntoh32(drive->rd_buf+offs);
			offs = 8+j*desc_len+12;
			drive->perf.spd_e = ntoh32(drive->rd_buf+offs);

			offs = 8+j*desc_len+12;
			if (!rw)
				drive->parms.read_speed_kb = ntoh32(drive->rd_buf+offs);
			else
				drive->parms.write_speed_kb = ntoh32(drive->rd_buf+offs);

//			printf("\t%dkB/s@%d -> %dkB/s@%d\n",
//				drive->perf.spd_s,drive->perf.lba_s,drive->perf.spd_e,drive->perf.lba_e);
		} else if (type == 0x03) {
			offs = 8+j*desc_len+4;
			lba = ntoh32(drive->rd_buf+offs);
			offs = 8+j*desc_len+8;
			r = ntoh32(drive->rd_buf+offs);
			offs = 8+j*desc_len+12;
			w = ntoh32(drive->rd_buf+offs);

//			printf("LBA %d: \tW %dkB/s   R %dkB/s\n", lba, w, r);

			drive->parms.wr_speed_tbl_kb[j] = ntoh32(drive->rd_buf+offs);
			drive->parms.max_write_speed_kb = max(drive->parms.max_write_speed_kb, drive->parms.wr_speed_tbl_kb[j]);
		}
	}
	return 0;
}
 
int detect_speeds(drive_info *drive)
{
	int	idx, spd, prev_spd, rspd_kb, wspd_kb;
//	bool gp=1;
	if (!drive->silent) printf("== Detecting supported read speeds...\n");
	for (idx=1; idx<speed_tbl_size;idx++) {
		drive->parms.speed_tbl[idx]=-1;
		drive->parms.speed_tbl_kb[idx]=-1;
	}
	if (get_rw_speeds(drive)) {
		drive->parms.read_speed_kb = 1;
		drive->parms.write_speed_kb = 1;
		drive->parms.speed_mult = 1;
		return 1;
	}
	rspd_kb = drive->parms.read_speed_kb;
	wspd_kb = drive->parms.write_speed_kb;

//	speedidx = 0;
//	printf("media.type & DISC_CD  = %LX\n", drive->media.type & DISC_CD  );
//	printf("media.type & DISC_DVD = %LX\n", drive->media.type & DISC_DVD );
	if ( drive->media.type & (DISC_DVD | DISC_BD)) {
		bool BD = !!(drive->media.type & DISC_BD);
		drive->parms.read_speed_kb= BD ? 71920 : 22162;
		if (!set_rw_speeds(drive)) {
			get_rw_speeds(drive);
			if (!drive->silent) printf("Max DVD speed via GET_CD_SPEED: %dkB/s\n",
					drive->parms.read_speed_kb);
			//if ( (!drive->wr_capabilities) && (drive->capabilities & CAP_REAL_TIME_STREAMING) && (!get_performance(drive))) {
			if ( (drive->capabilities & CAP_REAL_TIME_STREAMING)
				//	&& !(drive->media.type & DISC_DVDRAM)
					&& !(get_performance(drive, 0, 0))) {
				if (drive->perf.spd_e > 1)
					drive->parms.max_read_speed_kb =
						drive->perf.spd_e;
//						(int)(drive->perf.spd_e*((float)2294912/(float)drive->perf.lba_e));
//				drive->parms.max_read_speed_kb = drive->perf.spd_e;
				drive->parms.max_read_speed_dvd = (drive->parms.max_read_speed_kb/1350);
				if (!drive->silent) printf("Max DVD speed via GET_PERFORMANCE: %d X, %dkB/s\n",
						drive->parms.max_read_speed_dvd,
						drive->parms.max_read_speed_kb);
				drive->parms.speed_mult = drive->parms.read_speed_kb/drive->parms.max_read_speed_dvd;
				if (drive->parms.speed_mult < 600) {
					drive->parms.speed_mult  = 176.4;
				} else {
					drive->parms.max_read_speed_dvd = (drive->parms.max_read_speed_kb/ (BD ? 4494 : 1384) );
					drive->parms.speed_mult = BD ? 4495 : 1385;
				}
			} else {
				drive->parms.max_read_speed_kb = drive->parms.read_speed_kb;
				if (!drive->silent) printf("GET_PERFORMANCE error: using default multiplier\n");
				drive->parms.speed_mult = BD ? 4495 : 1385;
				drive->parms.max_read_speed_dvd = (drive->parms.max_read_speed_kb/drive->parms.speed_mult);
			}
//			drive->parms.read_speed_kb = spd_kb;
//			set_rw_speeds(drive);
//			drive->parms.speed_mult= drive->parms.max_read_speed_kb/drive->parms.max_read_speed_dvd;
			if (!drive->silent) {
				printf("1X multiplier: %.1f kB/s\n", drive->parms.speed_mult);
				printf("Max spd: %d X, %d kB/s\n",
						drive->parms.max_read_speed_dvd,
						drive->parms.max_read_speed_kb);
			}

			idx=0; prev_spd=0;
			for (spd=1; ((idx<speed_tbl_size) && (spd < (drive->parms.max_read_speed_dvd+2))); spd++) {
//			for (spd=1; ((idx<speed_tbl_size) && (spd < 17)); spd++) {
				drive->parms.read_speed_kb = (int)(spd * (drive->parms.speed_mult+1));
				if (!drive->silent) printf("Trying:  %dX (%d kB/s)\n", spd, drive->parms.read_speed_kb);
				set_rw_speeds(drive);
				get_rw_speeds(drive);
				drive->parms.read_speed_dvd = (int) (drive->parms.read_speed_kb / drive->parms.speed_mult);
				if (prev_spd != drive->parms.read_speed_dvd) {
//					spd = drive->parms.read_speed_dvd;
					spd = max(spd, drive->parms.read_speed_dvd);
					drive->parms.speed_tbl[idx] = drive->parms.read_speed_dvd;
					drive->parms.speed_tbl_kb[idx] = drive->parms.read_speed_kb;
					if (!drive->silent) printf(" RD speed:  %dX (%d kB/s)\n",
									drive->parms.speed_tbl[idx],
									drive->parms.speed_tbl_kb[idx]);
					prev_spd = drive->parms.read_speed_dvd;
					idx++;
//					if (drive->capabilities & CAP_REAL_TIME_STREAMING) get_performance(drive);
				}
			}
#ifdef speedidx
		} else {
			speedidx=-1;
#endif
		}
	} else {
		drive->parms.read_speed_kb=-1;
		if (!set_rw_speeds(drive)) {
			get_rw_speeds(drive);
			drive->parms.max_read_speed_kb = drive->parms.read_speed_kb;
			drive->parms.max_read_speed_cd = (drive->parms.max_read_speed_kb/176);
//			drive->parms.speed_mult=drive->parms.max_read_speed_kb/drive->parms.max_read_speed_cd;
			drive->parms.speed_mult=176.4;
			if (!drive->silent)
				printf("Maximum CD speed: %dX, %5d kB/s; 1X = %.1f kB/s\nSpeeds:\n",
					drive->parms.max_read_speed_cd,
					drive->parms.max_read_speed_kb,
					drive->parms.speed_mult);
			idx=0; prev_spd=0;
			for (spd=1; ((idx<speed_tbl_size) && (spd < (drive->parms.max_read_speed_cd+2))); spd++) {
//			for (spd=1; ((idx<speed_tbl_size) && (spd < 72)); spd++) {
				drive->parms.read_speed_kb = (int) (spd * (drive->parms.speed_mult+1));
				if (!drive->silent) printf("Trying:  %dX (%5d kB/s)\n", spd, drive->parms.read_speed_kb);
				set_rw_speeds(drive);
				get_rw_speeds(drive);
				drive->parms.read_speed_cd = (int) (drive->parms.read_speed_kb / drive->parms.speed_mult);
				if (prev_spd != drive->parms.read_speed_cd) {
//					spd = drive->parms.read_speed_cd;
					spd = max(spd, drive->parms.read_speed_cd);
					drive->parms.speed_tbl[idx] = drive->parms.read_speed_cd;
					drive->parms.speed_tbl_kb[idx] = drive->parms.read_speed_kb;
					if (!drive->silent) printf(" RD speed:  %dX (%5d kB/s)\n",
							drive->parms.speed_tbl[idx],
							drive->parms.speed_tbl_kb[idx]);
					prev_spd = drive->parms.read_speed_cd;
					idx++;
//					if (drive->capabilities & CAP_REAL_TIME_STREAMING) get_performance(drive);
				}
			}
#ifdef speedidx
		} else {
			speedidx=-1;
#endif
		}
	}
//	combo_Speed->clear();
	idx=0;
#ifdef speedidx
	if (!speedidx) {
		while ((idx<speed_tbl_size) && (drive->parms.speed_tbl[idx]>0)){
			int mlt = 176;
			if ( drive->media.type & DISC_DVD ) mlt=1385;

//			combo_Speed->insertItem(QString().sprintf("%2dX (%dkB/s)",
//				drive->parms.speed_tbl[idx],mlt*drive->parms.speed_tbl[idx]));
			if ((spd_kb/drive->parms.speed_mult)==drive->parms.speed_tbl[idx]) speedidx = idx;
			idx++;
		}
	} else {
		speedidx = 0;
	}
#endif
// 	combo_Speed->insertItem(tr("max"));

	get_write_speed_tbl(drive);
	for (int i=0; i<speed_tbl_size && drive->parms.wr_speed_tbl_kb[i] > 0; i++) {
		if (!drive->silent)
			printf(" WR speed:  %.1fX (%d kB/s)\n",
					drive->parms.wr_speed_tbl_kb[i] / drive->parms.speed_mult,
					drive->parms.wr_speed_tbl_kb[i]);
	}

	drive->parms.read_speed_kb  = rspd_kb;
	drive->parms.write_speed_kb = wspd_kb;
	set_rw_speeds(drive);
//	combo_Speed->setCurrentItem(speedidx);
//	show_read_speed(drive);
	return 0;
}

int get_write_speed_tbl(drive_info* drive) {
	int	offs;
	int	i, spdcnt;
	drive->parms.max_write_speed_kb = 0;
	if (drive->capabilities & CAP_REAL_TIME_STREAMING) {
		get_performance(drive, 0, 0x03);
	} else {
		mode_sense(drive, MODE_PAGE_MM_CAP_STATUS, 00, 256);
		offs=0; while (((drive->rd_buf[offs]) & 0x3F) != 0x2A) offs++;
//		drive->parms.write_speed_kb = ntoh16(drive->rd_buf+offs+28);
		spdcnt = ntoh16(drive->rd_buf+offs+30);
		for (i=0; i<speed_tbl_size;i++)
			drive->parms.wr_speed_tbl_kb[i] = -1;
//		printf("== Write speeds: %d\n",spdcnt);
		for (i=0; (i<spdcnt) && (i<speed_tbl_size); i++) {
			drive->parms.wr_speed_tbl_kb[i] = ntoh16(drive->rd_buf+offs+32+i*4+2);
			drive->parms.max_write_speed_kb = max(drive->parms.max_write_speed_kb, drive->parms.wr_speed_tbl_kb[i]);
//			printf("  Speed #%02d: %d kB/s\n",i,drive->parms.wr_speed_tbl_kb[i]);
		}
	}
	return 0;
}

int get_rw_speeds(drive_info* drive) {
	int	offs;
/*

   PLEXTOR drives always returns maximum read speed via "get performance", not current!

*/
	if (!drive->get_performance_fail && isPlextor(drive)) {
		drive->get_performance_fail=1;
	}

	//if ((drive->capabilities & CAP_REAL_TIME_STREAMING) && !(drive->media.type & DISC_DVDRAM)) {
	//if (!drive->get_performance_fail && (drive->capabilities & CAP_REAL_TIME_STREAMING)) {
	if (!drive->get_performance_fail
//			&& !(drive->media.type & DISC_CD)
			&& (drive->capabilities & CAP_REAL_TIME_STREAMING)) {
		if (!drive->silent) printf("Requesting curerent speeds via GET_PERFORMANCE command...\n");
		if (!get_performance(drive, 0, 0) && !get_performance(drive, 1, 0))
			return 0;
		drive->get_performance_fail=1;
	}

	if (!drive->silent) printf("Requesting curerent speeds via page 2A...\n");
	if (!mode_sense(drive, MODE_PAGE_MM_CAP_STATUS, 00, 256)) {
		offs=0; while (((drive->rd_buf[offs]) & 0x3F) != 0x2A) offs++;
		drive->parms.read_speed_kb = ntoh16u(drive->rd_buf+offs+14);
		drive->parms.write_speed_kb = ntoh16u(drive->rd_buf+offs+28);
		return 0;
	}
	drive->parms.read_speed_kb  = 0;
	drive->parms.write_speed_kb = 0;
	return 1;
}

int set_streaming(drive_info* drive) {
	char data[28]; memset(data, 0, 28);

	uint32_t* start_lba  = (uint32_t*)&data[4];
	uint32_t* end_lba    = (uint32_t*)&data[8];
	uint32_t* read_size  = (uint32_t*)&data[12];
	uint32_t* read_time  = (uint32_t*)&data[16];
	uint32_t* write_size = (uint32_t*)&data[20];
	uint32_t* write_time = (uint32_t*)&data[24];
	uint32_t	speed_rd = 0xFFFFFFFF;
	uint32_t	speed_wr = 0xFFFFFFFF;
	if (drive->parms.read_speed_kb)  speed_rd = drive->parms.read_speed_kb;
	if (drive->parms.write_speed_kb) speed_wr = drive->parms.write_speed_kb;

//	if (read_capacity(drive)) return drive->err;
	*start_lba = ntoh32(0);
	*end_lba = ntoh32(drive->media.capacity);
//	*end_lba = ntoh32(0);
	*read_time = ntoh32(1000);
	*read_size = ntoh32(speed_rd);
	*write_time = ntoh32(1000);
	*write_size = ntoh32(speed_wr);
	drive->cmd[0] = MMC_SET_STREAMING;
	drive->cmd[10] = 28;
	drive->cmd[11] = 0;
	if ((drive->err=drive->cmd.transport(WRITE,data,28)))
		{ if(!drive->silent) sperror ("SET_STREAMING",drive->err); return (drive->err); }
	return 0;
}

int set_cd_speed(drive_info* drive) {
	uint16_t	speed_rd = 0xFFFF;
	uint16_t	speed_wr = 0xFFFF;
	if (drive->parms.read_speed_kb)  speed_rd = drive->parms.read_speed_kb;
	if (drive->parms.write_speed_kb) speed_wr = drive->parms.write_speed_kb;

	drive->cmd[0] = MMC_SET_SPEED;
	drive->cmd[1] = 0x01;
	drive->cmd[2] = (speed_rd >> 8) & 0xFF;
	drive->cmd[3] = speed_rd & 0xFF;
//	drive->cmd[4] = 0xFF;
//	drive->cmd[5] = 0xFF;
	drive->cmd[4] = (speed_wr >> 8) & 0xFF;
	drive->cmd[5] = speed_wr & 0xFF;
	drive->cmd[11] = 0;
	if ((drive->err=drive->cmd.transport(NONE,NULL,0) )) {
//		if (drive->err != 0x23A02) drive->capabilities&=(NCAP_SET_CD_SPEED);
		if (!drive->silent) sperror ("SET_CD_SPEED",drive->err); return (drive->err);
	}
	return 0;
}

//#define __STREAMING_PRIOR
int set_rw_speeds(drive_info* drive) {
	int	rez=1;

	if ((drive->capabilities & CAP_REAL_TIME_STREAMING)) {
	//if ((drive->capabilities & CAP_REAL_TIME_STREAMING) && !(drive->media.type & DISC_CD)) {
	//if ((drive->capabilities & CAP_REAL_TIME_STREAMING) && !(drive->media.type & DISC_DVDRAM)) {
		if (!drive->silent) printf("Setting speeds via SET_STREAMING command...\n");
		rez = set_streaming(drive);
	}
	if (rez) {
		if (!drive->silent) printf("Setting speeds via SET_CD_SPEED command...\n");
		rez = set_cd_speed(drive);
	}
/*
#ifdef __STREAMING_PRIOR
	if (drive->media.type & DISC_CD) {
		rez = set_cd_speed(drive);
	} else if (drive->media.type & DISC_DVD) {
		{ if ((rez = set_cd_speed(drive))) rez = set_streaming(drive); }
	}
#else
	if ((drive->capabilities & CAP_SET_CD_SPEED) )//&& (drive->media.type & DISC_CD))
		{ if ((rez = set_cd_speed(drive))) rez = set_streaming(drive); }
	else if (drive->capabilities & CAP_REAL_TIME_STREAMING)
		{ rez = set_streaming(drive); }
#endif*/
	return rez;
}

int get_media_status(drive_info* drive){
	drive->cmd[0]=MMC_GET_EVENT_STATUS_NOTIFICATION;
	drive->cmd[1]=0x01;
	drive->cmd[4]=0x10;
	drive->cmd[7]=0;
	drive->cmd[8]=8;
	if ((drive->err=drive->cmd.transport(READ,drive->rd_buf,8)))
		{ sperror ("GET_EVENT_STATUS",drive->err); return (drive->err); }
	if (drive->rd_buf[5] & 0x01) drive->parms.status |= STATUS_OPEN;
	else drive->parms.status &= (~STATUS_OPEN);
	if (drive->rd_buf[5] & 0x02) drive->parms.status |= STATUS_MEDIA_PRESENT;
	else drive->parms.status &= (~STATUS_MEDIA_PRESENT);
	drive->parms.event = drive->rd_buf[4] & 0x0F;
	return 0;
}

int start_stop(drive_info* drive, bool start) {
	drive->cmd[0]=MMC_START_STOP_UNIT;
	drive->cmd[4]= start ? 0x01 : 0;
	if ((drive->err=drive->cmd.transport(NONE,NULL,0)))
		{ sperror ("START_STOP_UNIT",drive->err); return (drive->err); }
	return 0;
}

int load_eject(drive_info* drive, bool load, bool IMMED) {
	drive->cmd[0]=MMC_START_STOP_UNIT;
	drive->cmd[1]= IMMED ? 0x01 : 0;
	drive->cmd[4]= load ? 0x02 : 0;
	if ((drive->err=drive->cmd.transport(NONE,NULL,0)))
	{
		if (drive->err != 0x55302)
			{ sperror ("LOAD_EJECT",drive->err); return (drive->err); }

		printf("Trying to unlock media...\n");
		drive->parms.status &= (~STATUS_LOCK);
		set_lock(drive);
		
		drive->cmd[0]=MMC_START_STOP_UNIT;
		drive->cmd[1]= IMMED ? 0x01 : 0;
		drive->cmd[4]= load ? 0x02 : 0;
		if ((drive->err=drive->cmd.transport(NONE,NULL,0)))
			{ sperror ("LOAD_EJECT",drive->err); return (drive->err); }
	}
	return 0;
}

int load_eject(drive_info* drive, bool IMMED) {
	get_media_status(drive);
	printf("Tray state: %s\n" ,(drive->parms.status & STATUS_OPEN) ? "open" : "close");
	load_eject(drive, drive->parms.status & STATUS_OPEN, IMMED);
	return 0;
}

int get_lock(drive_info* drive){
//	printf("get_lock()\n");
	int offs;
	if (mode_sense(drive, 0x2A, 0, 256)) 
		{ sperror ("GET_LOCK",drive->err); return (drive->err); }
	offs=0; while (((drive->rd_buf[offs]) & 0x3F) != 0x2A) offs++;
	if (drive->rd_buf[offs+6] & 0x02) drive->parms.status |= STATUS_LOCK;
	else drive->parms.status &= (~STATUS_LOCK);
//#ifndef __PXCONTROL
//	if (!drive->silent) printf("--- Disc %slocked\n",(drive->parms.status & STATUS_LOCK) ? "" : "UN")
//#endif
	return 0;
}

int set_lock(drive_info* drive){
//	printf("--- %slocking disc...\n",(drive->parms.status & STATUS_LOCK) ? "" : "UN");
	drive->cmd[0]=SPC_PREVENT_ALLOW_MEDIUM_REMOVAL;
	drive->cmd[4]= (drive->parms.status & STATUS_LOCK) ? 1 : 0;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0)))
		{ sperror ("SET_LOCK",drive->err); get_lock(drive); return (drive->err); }
	get_lock(drive);
	return 0;
}

int play_audio_msf(drive_info* drive, msf beg, msf end){
	drive->cmd[0]=MMC_PLAY_AUDIO_MSF;
	drive->cmd[3]=beg.m;
	drive->cmd[4]=beg.s;
	drive->cmd[5]=beg.f;
	drive->cmd[6]=end.m;
	drive->cmd[7]=end.s;
	drive->cmd[8]=end.f;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0)))
		{ sperror ("PLAY_AUDIO_MSF",drive->err); return (drive->err); }
	return 0;
}

int play_audio(drive_info* drive, int32_t beg, short int len){
	drive->cmd[0]=MMC_PLAY_AUDIO;
	drive->cmd[2]=(beg>>24) & 0xFF;
	drive->cmd[3]=(beg>>16) & 0xFF;
	drive->cmd[4]=(beg>>8) & 0xFF;
	drive->cmd[5]=beg & 0xFF;
	drive->cmd[7]=(len>>8) & 0xFF;
	drive->cmd[8]=len & 0xFF;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0)))
		{ sperror ("PLAY_AUDIO",drive->err); return (drive->err); }
	return 0;
}

int seek(drive_info* drive, int32_t lba, unsigned char flags){
	drive->cmd[0]=MMC_SEEK;
	drive->cmd[2]=(lba>>24) & 0xFF;
	drive->cmd[3]=(lba>>16) & 0xFF;
	drive->cmd[4]=(lba>>8) & 0xFF;
	drive->cmd[5]=lba & 0xFF;

	drive->cmd[9]=flags;
	if ((drive->err=drive->cmd.transport(NONE, NULL, 0)))
		{ sperror ("SEEK",drive->err); return (drive->err); }
	return 0;
}


int read_cd(drive_info* drive, unsigned char *data, int32_t lba, int sector_count, unsigned char flags, unsigned char FUA) {
//	int transfer_length = sector_count * 3072;

//	int sect_data = 2352;
	int sect_data = 3072;

	int transfer_length = sector_count * sect_data;
	if (sector_count<0) return -1;

//	printf("lba: %d, cnt: %d\n", lba,sector_count);
	drive->cmd[0]=MMC_READ_CD;
	drive->cmd[1]= FUA ? 0x08 : 0x00;
	drive->cmd[2]=(lba>>24) & 0xFF;
	drive->cmd[3]=(lba>>16) & 0xFF;
	drive->cmd[4]=(lba>>8) & 0xFF;
	drive->cmd[5]=lba & 0xFF;
	drive->cmd[8]=sector_count;
	drive->cmd[9]=flags;
	if ((drive->err=drive->cmd.transport(READ, data, transfer_length)))
		{ sperror ("READ_CD",drive->err); 
		  return (drive->err); }
	return 0;
}

int read(drive_info* drive, unsigned char *data, int32_t lba, int sector_count, unsigned char FUA) {
//	int transfer_length = sector_count * 3072;
	int transfer_length = sector_count * 2048;
	if (sector_count<0) return -1;
	drive->cmd[0]=MMC_READ;
	drive->cmd[1]= FUA ? 0x08 : 0x00;
	drive->cmd[2]=(lba>>24) & 0xFF;
	drive->cmd[3]=(lba>>16) & 0xFF;
	drive->cmd[4]=(lba>>8) & 0xFF;
	drive->cmd[5]=lba & 0xFF;
	drive->cmd[8]=sector_count;
	if ((drive->err=drive->cmd.transport(READ, data, transfer_length)))
		{sperror ("READ",drive->err); return (drive->err);}
	return 0;
}

int read_one_ecc_block(drive_info* drive, unsigned char *data, int32_t lba) {
	drive->cmd[0] = MMC_READ;
	drive->cmd[2] = (lba>>24) & 0xFF;
	drive->cmd[3] = (lba>>16) & 0xFF;
	drive->cmd[4] = (lba>>8) & 0xFF;
	drive->cmd[5] = lba & 0xFF;
	drive->cmd[8] = 0x10;
	if ((drive->err=drive->cmd.transport(READ, data, 0x8000)))
		{sperror ("READ_ONE_ECC_BLOCK",drive->err); return (drive->err);}
//	if ((drive->err=drive->cmd.transport(READ,(void*)((unsigned char*)drive->rd_buf+(0x0001<<14)),0x34)))
//		{sperror ("READ_ONE_ECC_BLOCK",drive->err); return (drive->err);}
	return 0;
}

int get_drive_serial_number(drive_info* drive) {
//	char data[2048]; memset(data, 0, sizeof(data));
	unsigned int data_length;
	unsigned int length;
	get_configuration(drive, FEATURE_LOGICAL_UNIT_SERIAL_NUMBER, &data_length, NULL);
	if (drive->err) return -1;
	length = drive->rd_buf[11]; drive->rd_buf[12+length]=0;
	if (data_length>8) strncpy(drive->serial, (char*)drive->rd_buf+12, 16);
	else drive->serial[0]=0;
	return 0;
}

int get_buffer_capacity(drive_info* drive){
	int offs;
	if (mode_sense(drive, MODE_PAGE_MM_CAP_STATUS, 0, 192)) return 1;
	offs=0; while (((drive->rd_buf[offs]) & 0x3F) != 0x2A) offs++;
	drive->buffer_size= ntoh16u(drive->rd_buf+offs+0x0C);
//	printf("Buffer capacity: 0x%04X (%d)KB\n", drive->buffer_size, drive->buffer_size);
	return 0;
}

int get_wbuffer_capacity(drive_info* drive, uint32_t *btot, uint32_t *bfree){
	uint8_t td[0x0C]; memset(td,0,0x0C);
	drive->cmd[0]=MMC_READ_BUFFER_CAPACITY;
	drive->cmd[8]=0x0C;
	if ((drive->err = drive->cmd.transport(READ,td,0x0C)))
		{sperror ("READ_BUFFER_CAPACITY",drive->err); return (drive->err);}

	(*btot)  = ntoh32u(td+4);
	(*bfree) = ntoh32u(td+8);
	return 0;
}

int read_writer_info(drive_info* drive)
{
	if (!(drive->media.type & DISC_DVDminus)) {
		strcpy(drive->media.writer, "n/a (only for DVD-R(W))");
		return 1;
	}
	char format=0x0D;
	drive->media.writer[0]=0;
	drive->rd_buf[8]=0;
	drive->cmd[0]=MMC_READ_DVD_STRUCTURE;
	drive->cmd[5]=0x02;
	drive->cmd[7]=format;
	drive->cmd[8]=8;
	drive->cmd[9]=8;
	if ((drive->err = drive->cmd.transport(READ,drive->rd_buf,2056)) || (!drive->rd_buf[8])) {
		printf("Read Writer Info failed\n");
		return 1;
	}
	for (int i=0; i<0x3F; i++) {
		if (!drive->rd_buf[8+i]) drive->rd_buf[8+i]=0x20;
	}
	strncpy(drive->media.writer, (char*)drive->rd_buf+8, 0x3F);
	remove_double_spaces(drive->media.writer);
//	remove_end_spaces(drive->media.writer);
	return 0;
}

int detect_mm_capabilities(drive_info* drive){
	char len;
	int offs;
	int i,j;
	if (mode_sense(drive, MODE_PAGE_MM_CAP_STATUS, 0, 256)) return 1;
	offs=0; while (((drive->rd_buf[offs]) & 0x3F) != 0x2A) offs++;
	len=drive->rd_buf[offs+1];
	if (!drive->silent) printf("CD parameters page length: 0x%02X\n",len);
	if (len >= 28) {drive->mmc=3;}
	else if (len >= 24) {drive->mmc=2;}
	else {drive->mmc=1;}
	if (!drive->silent) for (i=offs; i<(offs+len+2);i+=8){
		for (j=0;j<8;j++) printf(" %02X",drive->rd_buf[i+j] & 0xFF);
		printf("\n");
	}
//	if (drive->mmc) drive->rd_capabilities|=DEVICE_CD_ROM;
	if (drive->rd_buf[offs+2] & 0x01)drive->rd_capabilities|=DEVICE_CD_R;
	if (drive->rd_buf[offs+2] & 0x02)drive->rd_capabilities|=DEVICE_CD_RW;

	if (drive->rd_buf[offs+3] & 0x01)drive->wr_capabilities|=DEVICE_CD_R;
	if (drive->rd_buf[offs+3] & 0x02)drive->wr_capabilities|=DEVICE_CD_RW;
	if (drive->rd_buf[offs+3] & 0x04)drive->capabilities|=CAP_TEST_WRITE_CD;

	if (drive->rd_buf[offs+4] & 0x01)drive->capabilities|=CAP_CD_AUDIO;
	if (drive->rd_buf[offs+4] & 0x02)drive->capabilities|=CAP_COMPOSITE;
	if (drive->rd_buf[offs+4] & 0x04)drive->capabilities|=CAP_DIGITAL_PORT_1;
	if (drive->rd_buf[offs+4] & 0x08)drive->capabilities|=CAP_DIGITAL_PORT_2;
	if (drive->rd_buf[offs+4] & 0x10)drive->capabilities|=CAP_MODE2_FORM1;
	if (drive->rd_buf[offs+4] & 0x20)drive->capabilities|=CAP_MODE2_FORM2;
	if (drive->rd_buf[offs+4] & 0x40)drive->capabilities|=CAP_MULTISESSION;
	if (drive->rd_buf[offs+4] & 0x80)drive->capabilities|=CAP_BURN_FREE;

	if (drive->rd_buf[offs+5] & 0x01)drive->capabilities|=CAP_DAE;
	if (drive->rd_buf[offs+5] & 0x02)drive->capabilities|=CAP_ACCURATE_STREAM;
	if (drive->rd_buf[offs+5] & 0x10)drive->capabilities|=CAP_C2;
	if (drive->rd_buf[offs+5] & 0x20)drive->capabilities|=CAP_ISRC;
	if (drive->rd_buf[offs+5] & 0x40)drive->capabilities|=CAP_UPC;
	if (drive->rd_buf[offs+5] & 0x80)drive->capabilities|=CAP_READ_BAR_CODE;	

	if (drive->rd_buf[offs+6] & 0x01)drive->capabilities|=CAP_LOCK;
	if (drive->rd_buf[offs+6] & 0x08)drive->capabilities|=CAP_EJECT;

	drive->loader_id = (drive->rd_buf[offs+6] >> 5) & 0x07;

	if (drive->rd_buf[offs+7] & 0x10)drive->capabilities|=CAP_SIDE_CHANGE;

	switch (drive->mmc) {
		case 3:
			drive->parms.write_speed_kb = ntoh16u(drive->rd_buf+36);
		case 2:
			if (drive->rd_buf[offs+2] & 0x08)drive->rd_capabilities|=DEVICE_DVD_ROM;
			if (drive->rd_buf[offs+2] & 0x10)drive->rd_capabilities|=DEVICE_DVD_R;
			if (drive->rd_buf[offs+2] & 0x20)drive->rd_capabilities|=DEVICE_DVD_RAM;
			if (drive->rd_buf[offs+3] & 0x10)drive->wr_capabilities|=DEVICE_DVD_R;
			if (drive->rd_buf[offs+3] & 0x20)drive->wr_capabilities|=DEVICE_DVD_RAM;
		case 1:
			drive->parms.max_read_speed_kb  = ntoh16u(drive->rd_buf+offs+8);
			drive->parms.read_speed_kb      = ntoh16u(drive->rd_buf+offs+14);
			drive->parms.max_write_speed_kb = ntoh16u(drive->rd_buf+offs+18);
			if (drive->mmc < 3)
				drive->parms.write_speed_kb = ntoh16u(drive->rd_buf+offs+20);
			break;
	}
	if (!drive->silent) printf("Max speeds:\tR@%dKBps / W@%dKBps\nCurrent speeds:\tR@%dKBps / W@%dKBps\n",
		drive->parms.max_read_speed_kb, drive->parms.max_write_speed_kb,
		drive->parms.read_speed_kb, drive->parms.write_speed_kb);

	if (isPlextor(drive) && strncmp(drive->dev,"CD-R", 4))
		drive->capabilities|=CAP_TEST_WRITE_DVD_PLUS;
	return 0;
}

int convert_to_ID (drive_info* drive) {
//	printf("convert_to_ID() : %s %s\n", drive->ven, drive->dev);
	if (!strncmp(drive->ven,"PLEXTOR ",8)) {
		drive->ven_ID=DEV_PLEXTOR;
		if(!strncmp(drive->dev,"CD-R   PX-W4824A",16))
			drive->dev_ID=PLEXTOR_4824;
		else
		if(!strncmp(drive->dev,"CD-R   PX-W5224A",16))
			drive->dev_ID=PLEXTOR_5224;
		else
		if(!strncmp(drive->dev,"CD-R   PREMIUM2",15))
			drive->dev_ID=PLEXTOR_PREMIUM2;
		else
		if(!strncmp(drive->dev,"CD-R   PREMIUM",14))
			drive->dev_ID=PLEXTOR_PREMIUM;
		else
		if(!strncmp(drive->dev,"DVDR   PX-708A2",15))
			drive->dev_ID=PLEXTOR_708A2;
		else
		if(!strncmp(drive->dev,"DVDR   PX-712A",14))
			drive->dev_ID=PLEXTOR_712;
		else
		if(!strncmp(drive->dev,"DVDR   PX-714A",14))
			drive->dev_ID=PLEXTOR_716;
		else
		if(!strncmp(drive->dev,"DVDR   PX-716A ",15))
			drive->dev_ID=PLEXTOR_716;
		else
		if(!strncmp(drive->dev,"DVDR   PX-716AL",15))
			drive->dev_ID=PLEXTOR_716AL;
		else
		if(!strncmp(drive->dev,"DVDR   PX-755A",14))
			drive->dev_ID=PLEXTOR_760;
		else
		if(!strncmp(drive->dev,"DVDR   PX-760A",14))
			drive->dev_ID=PLEXTOR_760;
		else
		if(!strncmp(drive->dev,"CD-R   ",8))
			drive->dev_ID=PLEXTOR_OLD;
		else
			{ drive->ven_ID = DEV_GENERIC; drive->dev_ID = 0; }
	}
	else if (!strncmp(drive->ven,"YAMAHA  ",8)) {
		drive->ven_ID=DEV_YAMAHA;
		if(!strncmp(drive->dev,"CRW-F1",6))
			drive->dev_ID=YAMAHA_F1;
		else
			drive->dev_ID=YAMAHA_OLD;
	}

	else { drive->ven_ID = DEV_GENERIC; drive->dev_ID = 0; }

//	printf("convert_to_ID() : %04X:%04X\n", drive->ven_ID, drive->dev_ID);
	return 0;
}

		//----------------//
		//  PX-755 AUTH   //
		//----------------//

int plextor_px755_do_auth(drive_info* dev)
{
	if (!isPlextorLockPresent(dev))
		{ if (!dev->silent) printf("Plextor dev is older than PX-755, auth not needed\n"); return 0; }
//	cmd_px755_clear_auth_status();
	plextor_px755_get_auth_code(dev, dev->rd_buf);
	plextor_px755_calc_auth_code(dev, dev->rd_buf);
	if (plextor_px755_send_auth_code(dev, dev->rd_buf)) {
		printf(" _______________________________________________________ \n");
		printf("|                                                       |\n");
		printf("|       WARNING!!!  Detected locked PX-755/PX-760       |\n");
		printf("|                     or Premium-II                     |\n");
		printf("|           Device has 'protected' commands             |\n");
		printf("|    you'll not get full fucntionality of this drive    |\n");
		printf("|_______________________________________________________|\n");
		return 1;
	} else {
		if (!dev->silent) printf("PX-755/PX-760/Premium-II auth successfull:)\n");
		return 0;
	}
}

int plextor_px755_get_auth_code(drive_info* dev, unsigned char* auth_code)
{
	dev->cmd[0] = PLEXTOR_GET_AUTH;
	dev->cmd[10]= 0x10;
	if ((dev->err=dev->cmd.transport(READ,auth_code,16) ))
		{ if (!dev->silent) sperror ("PLEXTOR_PX755_GET_AUTH_CODE",dev->err); return dev->err;}
	if (!dev->silent) {
		printf("** Get PX755 auth: ");
		for (int i=0; i<16; i++) printf("0x%02X ",dev->rd_buf[i]&0xFF); printf("\n");
	}
	return 0;
}

int plextor_px755_send_auth_code(drive_info* dev, unsigned char* auth_code)
{
	dev->cmd[0] = PLEXTOR_SEND_AUTH;
	dev->cmd[1] = 0x01;
	dev->cmd[2] = 0x01;
	dev->cmd[10]= 0x10;
	if ((dev->err=dev->cmd.transport(WRITE,auth_code,16) ))
		{ if (!dev->silent) sperror ("PLEXTOR_PX755_SEND_AUTH_CODE",dev->err); return dev->err;}
	return 0;
}

/*
int scan_plextor::cmd_px755_clear_auth_status(drive_info* dev)
{
	dev->cmd[0] = PLEXTOR_SEND_AUTH;
	dev->cmd[1] = 0x01;
	dev->cmd[2] = 0x00;
	dev->cmd[10]= 0x10;
	if ((dev->err=dev->cmd.transport(NONE,NULL,0) ))
		{ if (!dev->silent) sperror ("PLEXTOR_PX755_CLEAR_AUTH_STATUS",dev->err); return dev->err;}
	return 0;
}
*/

int plextor_px755_calc_auth_code(drive_info* dev, unsigned char* auth_code)
{
	return 0;
}

