/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2007 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include <stdio.h>
#include <string.h>
#include "sense.h"

int sense2str(int err, char* str) {
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
	return 0;
}

