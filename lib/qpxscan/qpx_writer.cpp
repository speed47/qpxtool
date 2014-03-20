/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <stdio.h>

#include <qpx_mmc.h>
#include "qpx_writer.h"

#define WTYPE_CD_DEFAULT	WTYPE_TAO

qpxwriter::qpxwriter(drive_info *idev)
{
	dev = idev;
	stop_req = 0;
}

void qpxwriter::stop() {
	stop_req=1;
	printf("\nqpxwriter: terminating...\n");
}

void qpxwriter::setSimul(bool isimul) { simul = isimul; };
int qpxwriter::send_opc() { return 0; }
int qpxwriter::open_session() { return 0; }
int qpxwriter::open_track(uint32_t size) { return 0; }
int qpxwriter::close_track() { return 0; }
int qpxwriter::fixate() { return 0; }

int qpxwriter::mmc_write(int32_t lba, int sects)
{
//	printf("qpxwriter: sector %ld (wsize=%d)\n", lba, sects);
	dev->cmd[0] = MMC_WRITE;

	dev->cmd[2] = (lba >> 24) & 0xFF;
	dev->cmd[3] = (lba >> 16) & 0xFF;
	dev->cmd[4] = (lba >>  8) & 0xFF;
	dev->cmd[5] = lba & 0xFF;

	dev->cmd[7] = (sects >> 8);
	dev->cmd[8] = sects & 0xFF;
	if ((dev->err=dev->cmd.transport(WRITE, dev->rd_buf, sects * 2048))) {
//		printf("\nqpxwriter: Write error at sector %ld (wsize=%d)\n", lba, sects);
		if (dev->err != 0x20408) sperror ("MMC_WRITE",dev->err);
		return (dev->err);
	}
	return 0;
}

int qpxwriter::write_data(int32_t lba, int sects)
{
	int r=0;
/*
	if (lba>=0) {
		bool f=0;
		do {
			if (f) usleep(20000); else f=1;
			get_wbuffer_capacity(dev,&ubuft,&ubuff);
		//	if (ubuft) printf("%8d buf: %d %%\n", lba, 100*(ubuft-ubuff) / ubuft);
		//} while ( dev->err == 0x20408 || (ubuff < sects*2048));
		} while ( !stop_req && !dev->err && (ubuff < sects*2048));
	}
	if (stop_req) return -1;
*/
//	printf("test_unit_ready: %d\n",test_unit_ready(dev));
	if (lba>=0) for (int i=0; i<sects; i++) {
		dev->rd_buf[i*2048+0] = ((lba+i) >> 24) & 0xFF;
		dev->rd_buf[i*2048+1] = ((lba+i) >> 16) & 0xFF;
		dev->rd_buf[i*2048+2] = ((lba+i) >>  8) & 0xFF;
		dev->rd_buf[i*2048+3] =  (lba+i) & 0xFF;
	}
	do {
		if (r) {
		//	printf("waiting...\n");
			msleep(500);
		}
		r = mmc_write(lba,sects);
	} while (r == 0x20408); // for some drives like Plextor & LiteOn
	return r;
}

int qpxwriter_cd::send_opc()
{
	printf("Performing OPC...\n");

	dev->cmd[0] = MMC_SEND_OPC_INFORMATION;
	dev->cmd[1] = 0x01;
	if ((dev->err=dev->cmd.transport(NONE, NULL, 0)))
		{sperror ("SEND_OPC",dev->err); return (dev->err);}

	return (wait_unit_ready(dev, 60));
}

int qpxwriter_cd::set_write_parameters_def(bool bfree, bool simul) {
	printf("Setting write parameters...\n");
//	printf("simul: %s\n", simul ? "ON":"OFF");

	if (mode_sense(dev, MODE_PAGE_WRITE_PARAMETERS, 0, 60)) return -1;

	if (!!(dev->rd_buf[8+2] & 0x40) == bfree) {
		printf("BURN-Free is %s\n", (dev->rd_buf[8+2] & 0x40) ? "ON" : "OFF" );
	} else {
		printf("Turning BURN-Free %s\n", bfree ? "ON" : "OFF" );
	}
/*
	printf("TestWrite is %s\n", (dev->rd_buf[8+2] & 0x10) ? "ON" : "OFF" );
	if (!!(dev->rd_buf[8+2] & 0x10) != simul) {

	}
*/
/*
	for (int i=0; i<52; i++) {
		if (!(i%8)) printf("\n");
		printf(" %02X", dev->rd_buf[i+8]);
	}
	printf("\n");
*/

	memset(dev->rd_buf, 0, 60);
	dev->rd_buf[8+0] = 0x05;
	dev->rd_buf[8+1] = 0x32;

	dev->rd_buf[8+2] = ( (bfree ? 0x40 : 0) | (simul ? 0x10 : 0) ); // setting BURN-Free and TestWrite
	dev->rd_buf[8+2] |= WTYPE_CD_DEFAULT; // write type = SAO

	dev->rd_buf[8+3] = 0x04; // track mode 
	dev->rd_buf[8+4] = 0x08; // data block type:  Mode1

	dev->rd_buf[8+8]  = 0; // session format
	dev->rd_buf[8+15] = 0x96; // audio pause length = 150 frames (2 sec)
/*
	for (int i=0; i<52; i++) {
		if (!(i%8)) printf("\n");
		printf(" %02X", dev->rd_buf[i+8]);
	}
	printf("\n");
*/
	return (mode_select(dev,60));
}

int qpxwriter_cd::send_cue_sheet(uint32_t tsize)
{
	int foffs=0;
	int lout = tsize+150;
	printf("Sending CUE sheet...\n");

	memset(dev->rd_buf,0,bufsz_rd);
// lead-in
	dev->rd_buf[foffs+0] = 0x41;
	dev->rd_buf[foffs+3] = 0x14; // for lead-in data form = 0x14
	foffs+=8;

// track 0 pregap
	dev->rd_buf[foffs+0] = 0x41;
	dev->rd_buf[foffs+1] = 0x01; // track #1
	dev->rd_buf[foffs+2] = 0x00; // index = 0 - pregap
	dev->rd_buf[foffs+3] = 0x10; // data form = 0x10
	// addr 00:00.00 msf
	dev->rd_buf[foffs+5] = 0x00;
	dev->rd_buf[foffs+6] = 0x00;
	dev->rd_buf[foffs+7] = 0x00;
	foffs+=8;

// track 0 start
	dev->rd_buf[foffs+0] = 0x41;
	dev->rd_buf[foffs+1] = 0x01; // track #1
	dev->rd_buf[foffs+2] = 0x01; // index = 1 - track start
	dev->rd_buf[foffs+3] = 0x10; // data form = 0x10
	// addr 00:02.00 msf
	dev->rd_buf[foffs+5] = 0x00;
	dev->rd_buf[foffs+6] = 0x02;
	dev->rd_buf[foffs+7] = 0x00;
	foffs+=8;

// lead-out
	dev->rd_buf[foffs+0] = 0x41;
	dev->rd_buf[foffs+1] = 0xAA; // AA - lead-out
	dev->rd_buf[foffs+2] = 0x01; // for lead-out index = 1
	dev->rd_buf[foffs+3] = 0x14; // for lead-in data form = 0x14
	// lead-out end addr
	dev->rd_buf[foffs+5] = lout/4500;
	dev->rd_buf[foffs+6] = (lout/75)%60;
	dev->rd_buf[foffs+7] = lout%75;
/*
	printf("Lead-Out pos: %02d:%02d.%02d\n",
			dev->rd_buf[foffs+5],
			dev->rd_buf[foffs+6],
			dev->rd_buf[foffs+7]
			);
*/
	foffs+=8;

	dev->cmd[0] = MMC_SEND_CUE_SHEET;
	dev->cmd[6] = (foffs >> 16) & 0xFF;
	dev->cmd[7] = (foffs >>  8) & 0xFF;
	dev->cmd[8] = foffs & 0xFF;

	if ((dev->err=dev->cmd.transport(WRITE, dev->rd_buf, foffs)))
		{sperror ("SEND_CUE_SHEET",dev->err); return (dev->err);}
	return 0;
}

int qpxwriter_cd::write_lead_in()
{
	memset(dev->rd_buf,0,bufsz_rd);
	int32_t lba = -150;
	printf("Writing Lead-In...\n");
	for(int i=0; i<6; i++) {
		if (write_data(lba,25)) return dev->err;
		lba+=25;
	}
	return 0;
}

/*
int qpxwriter_cd::open_session()
{
	return (set_write_parameters_def(0,0));
}
*/

int qpxwriter_cd::open_track(uint32_t size)
{
#if (WTYPE_CD_DEFAULT == WTYPE_SAO)
	printf("Writing in SAO mode\n");
#elif (WTYPE_CD_DEFAULT == WTYPE_TAO)
	printf("Writing in TAO mode\n");
#endif
	if (set_write_parameters_def( ((dev->capabilities & CAP_BURN_FREE)) ,simul)) {
		return 1;
	}
/*
	mode_sense(dev, 0x05, 0, 60);
	for (int i=0; i<52; i++) {
		if (!(i%8)) printf("\n");
		printf(" %02X", dev->rd_buf[i+8]);
	}
*/
#if (WTYPE_CD_DEFAULT == WTYPE_SAO)
	if (send_cue_sheet(size)) return 1;
	if (write_lead_in()) return 1;
#endif
	return 0;
}

int qpxwriter_cd::close_track()
{
	printf("Closing track...\n");
	if (flush_cache(dev, true)) return 1;
	wait_unit_ready(dev, 300, 1);

#if (WTYPE_CD_DEFAULT == WTYPE_TAO)
	close_track_session(dev,1, CLOSE_TRACK); // close track
#endif
/*
	do {
		usleep(20000);
		get_buffer_capacity(dev,&ubuft,&ubuff);
		printf("buf: %d %%\r", 100*(ubuft-ubuff) / ubuft);
	} while (ubuff<ubuft);
	printf("\n");
*/
	wait_unit_ready(dev, 120);
	return 0;
}

int qpxwriter_cd::fixate()
{
	printf("Fixating...\n");
#if (WTYPE_CD_DEFAULT == WTYPE_TAO)
	close_track_session(dev,1, CLOSE_SESSION); // close session
#endif
	if (flush_cache(dev, false)) return 1;
	return (wait_fix(dev, 120));
}

//
// DVD- functions...
//
int qpxwriter_dvdminus::open_session()
{
	bool bfree = ((dev->capabilities & CAP_BURN_FREE));
	bfree = 1;
	if (mode_sense(dev, MODE_PAGE_WRITE_PARAMETERS, 0, 60)) return -1;
	if (!!(dev->rd_buf[8+2] & 0x40) == bfree) {
		printf("BURN-Free is %s\n", (dev->rd_buf[8+2] & 0x40) ? "ON" : "OFF" );
	} else {
		printf("Turning BURN-Free %s\n", bfree ? "ON" : "OFF" );
	}
	dev->rd_buf[8+2] = ( (bfree ? 0x40 : 0) | (simul ? 0x10 : 0) ); // setting BURN-Free and TestWrite
	dev->rd_buf[8+2] |= WTYPE_SAO; // write type = SAO
	return (mode_select(dev,60));
}

int qpxwriter_dvdminus::open_track(uint32_t size) {
	return (reserve_track(dev,size));
}

int qpxwriter_dvdminus::close_track() {
/*
	if (dev->media.type & (DISC_DVDmR | DISC_DVDmRDL)) {
		printf("\nClosing track...\n");
		close_track_session(dev,1, CLOSE_TRACK); // close track
		wait_unit_ready(dev, 1200, 1);
	}
*/
	return 0; 
}

int qpxwriter_dvdminus::fixate() {
	flush_cache(dev, 1);
	printf("\nWaiting for drive to become ready...\n");
	wait_unit_ready(dev, 300, 1);
	printf("\nClosing session...\n");
	close_track_session(dev,1, CLOSE_SESSION); // close session
	wait_unit_ready(dev, 300, 1);
	return 0;
}

//
// DVD+ functions...
//
int qpxwriter_dvdplus::open_session() {
	printf("Setting write parameters...\n");
	if (mode_sense(dev, MODE_PAGE_WRITE_PARAMETERS, 0, 60)) return -1;
	dev->rd_buf[8+2] &= 0xF0;
	dev->rd_buf[8+2] |= WTYPE_SAO; // write type = SAO
	return (mode_select(dev,60));
}

int qpxwriter_dvdplus::open_track(uint32_t size) {
	return 0;
}

int qpxwriter_dvdplus::close_track() {
	flush_cache(dev, 1);
	wait_unit_ready(dev, 300, 1);
	return 0;
}

int qpxwriter_dvdplus::fixate() {
	if (dev->media.type & (DISC_DVDpRW | DISC_DVDpRWDL)) {
		return fixate_rw();
	} else if (dev->media.type & (DISC_DVDpR | DISC_DVDpRDL)) {
		return fixate_r();
	}
	return 0;
}

int qpxwriter_dvdplus::fixate_rw() {
	close_track_session(dev,1, CLOSE_SESSION); // close session
	wait_unit_ready(dev, 300, 1);
	return 0; 
}

int qpxwriter_dvdplus::fixate_r() {
	printf("Closing track...\n");
	close_track_session(dev,1, CLOSE_TRACK); // close track
	wait_unit_ready(dev, 300, 1);
	printf("Closing session...\n");
	close_track_session(dev,1, CLOSE_SESSION); // close session
	wait_unit_ready(dev, 300, 1);
	return 0; 
}

//
//  BD-R/RE fonctions
//
int qpxwriter_bd::open_track(uint32_t size)
{
	return (reserve_track(dev,size));
}

int qpxwriter_bd::close_track()
{
	flush_cache(dev, 1);
	printf("\nWaiting for drive to become ready...\n");
	wait_unit_ready(dev, 300, 1);
	return 0;
}

int qpxwriter_bd::fixate()
{
	if (dev->media.type & (DISC_BD_RE)) {
		return fixate_re();
	} else if (dev->media.type & (DISC_BD_R_SEQ | DISC_BD_R_RND)) {
		return fixate_r();
	}
	return 0;
}

int qpxwriter_bd::fixate_re()
{
	return 0;
}

int qpxwriter_bd::fixate_r()
{
	wait_unit_ready(dev, 300, 1);
	printf("\nClosing track...\n");
	close_track_session(dev,1, CLOSE_TRACK); // close track
	wait_unit_ready(dev, 300, 1);

	printf("\nClosing session...\n");
	close_track_session(dev,1, CLOSE_FINALIZE); // finalize disc
	wait_unit_ready(dev, 300, 1);
	return 0;
}

