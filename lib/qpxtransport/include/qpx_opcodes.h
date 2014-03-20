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

#ifndef __qpxtool_opcodes_h
#define __qpxtool_opcodes_h

/*
const char SPC_		= 0x;
const char MMC_		= 0x;
*/

/* SCSI primary commands */

const char SPC_TEST_UNIT_READY		= 0x00;
const char SPC_REQUEST_SENSE		= 0x03;
const char SPC_INQUIRY				= 0x12;
const char SPC_RESERVE_6			= 0x16;
const char SPC_RELEASE_6			= 0x17;
const char SPC_RECEIVE_DIAGNOSTIC_RESULTS	= 0x1C;
const char SPC_SEND_DIAGNOSTIC		= 0x1D;
const char SPC_PREVENT_ALLOW_MEDIUM_REMOVAL	= 0x1E;
const char SPC_WRITE_BUFFER			= 0x3B;
const char SPC_READ_BUFFER			= 0x3C;
const char SPC_LOG_SELECT			= 0x4C;
const char SPC_LOG_SENSE			= 0x4D;
const char SPC_RESERVE_10			= 0x56;
const char SPC_RELEASE_10			= 0x57;
const char SPC_PERSISTENT_RESERVE_IN	= 0x5E;
const char SPC_PERSISTENT_RESERVE_OUT	= 0x5F;

const char SPC_EXTENDED_COPY		= 0x83;
const char SPC_RECEIVE_COPY_RESULTS	= 0x84;
const char SPC_ACCESS_CONTROL_IN	= 0x86;
const char SPC_ACCESS_CONTROL_OUT	= 0x87;
const char SPC_READ_ATTRIBUTE		= 0x8C;
const char SPC_WRITE_ATTRIBUTE		= 0x8D;

const char SPC_REPORT_LUNS			= 0xA0;
const char SPC_SECURITY_PROTOCOL_IN	= 0xA2;
const char SPC_SECURITY_PROTOCOL_OUT	= 0xB5;


const char SMC_MOVE_MEDIUM_ATTACHED		= 0xA7;
const char SMC_READ_ELEMENT_STATUS_ATTACHED	= 0xB4;

/* SCSI controller commands */

const char SCC_SPARE_IN				= 0xBC;
const char SCC_SPARE_OUT			= 0xBD;
const char SCC_VOLUME_SET_IN		= 0xBE;
const char SCC_VOLUME_SET_OUT		= 0xBF;

/* SCSI multimedia commands */

const char MMC_FORMAT_UNIT			= 0x04;

const char MMC_MODE_SELECT_6		= 0x15;
const char MMC_MODE_SENSE_6			= 0x1A;
const char MMC_START_STOP_UNIT		= 0x1B;

const char MMC_READ_FORMAT_CAPACITIES	= 0x23;
const char SBC_READ_CAPACITY		= 0x25;
const char MMC_READ					= 0x28;
const char MMC_WRITE				= 0x2A;
const char MMC_SEEK					= 0x2B;
const char MMC_WRITE_AND_VERIFY		= 0x2E;
const char MMC_VERIFY				= 0x2F;

const char MMC_SYNC_CACHE			= 0x35;

const char MMC_READ_SUB_CHANNEL		= 0x42;
const char MMC_READ_TOC_PMA_ATIP	= 0x43;
const char MMC_READ_HEADER			= 0x44;
const char MMC_PLAY_AUDIO			= 0x45;
const char MMC_GET_CONFIGURATION	= 0x46;
const char MMC_PLAY_AUDIO_MSF		= 0x47;
const char MMC_GET_EVENT_STATUS_NOTIFICATION	= 0x4A;
const char MMC_PAUSE_RESUME			= 0x4B;
const char MMC_STOP_PLAY_SCAN		= 0x4E;

const char MMC_READ_DISC_INFORMATION	= 0x51;
const char MMC_READ_TRACK_INFORMATION	= 0x52;
const char MMC_RESERVE_TRACK		= 0x53;
const char MMC_SEND_OPC_INFORMATION	= 0x54;
const char MMC_MODE_SELECT_10		= 0x55;
const char MMC_REPAIR_TRACK			= 0x58;
const char MMC_READ_MASTER_CUE		= 0x59;
const char MMC_MODE_SENSE_10		= 0x5A;
const char MMC_CLOSE_TRACK_SESSION	= 0x5B;
const char MMC_READ_BUFFER_CAPACITY	= 0x5C;
const char MMC_SEND_CUE_SHEET		= 0x5D;
const char MMC_BLANK				= 0xA1;
const char MMC_SEND_EVENT			= 0xA2;
const char MMC_SEND_KEY				= 0xA3;
const char MMC_REPORT_KEY			= 0xA4;
const char MMC_PLAY_AUDIO_12		= 0xA5;
const char MMC_LOAD_UNLOAD			= 0xA6;
const char MMC_SET_READ_AHEAD		= 0xA7;
const char MMC_READ_DVD				= 0xA8;
const char MMC_GET_PERFORMANCE		= 0xAC;
const char MMC_READ_DVD_STRUCTURE	= 0xAD;

const char MMC_SET_STREAMING		= 0xB6;
const char MMC_READ_CD_MSF			= 0xB9;
const char MMC_SCAN					= 0xBA;
const char MMC_SET_SPEED			= 0xBB;
const char MMC_PLAY_CD				= 0xBC;
const char MMC_MECHANISM_STATUS		= 0xBD;
const char MMC_READ_CD				= 0xBE;

const char PLEXTOR_GET_AUTH			= 0xD4;
const char PLEXTOR_SEND_AUTH		= 0xD5;

//const char  = 0xD8;

const char PLEXTOR_PLEXERASER		= 0xE3;
const char PLEXTOR_AS_RD			= 0xE4;
const char PLEXTOR_AS_WR			= 0xE5;

const char SBC_FLUSH_CACHE			= 0xE7;

const char PLEXTOR_MODE				= 0xE9;
const char PLEXTOR_QCHECK			= 0xEA;
const char PLEXTOR_PREC_SPD			= 0xEB;
const char PLEXTOR_MODE2			= 0xED;

const char PLEXTOR_EEPROM_READ		= 0xF1;

const char PLEXTOR_SCAN_TA_FETE		= 0xF3;
const char PLEXTOR_FETE_READOUT		= 0xF5;

#endif // __qpxtool_opcodes_h

