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

#ifndef QPXWRITER_H
#define QPXWRITER_H

class drive_info;

class qpxwriter {
public:
	qpxwriter(drive_info *idev);
	virtual ~qpxwriter() {};

	virtual void setSimul(bool isimul);
	virtual int send_opc();
	virtual int open_session();
	virtual int open_track(uint32_t size);
	virtual int close_track();
	virtual int fixate();
	virtual int write_data(int32_t lba, int sects);

	void stop();
protected:
	virtual int mmc_write(int32_t lba, int sects);
	drive_info	*dev;
	bool		stop_req;
	bool		simul;
};

// CD-R/W
// mostly usable, SAO only, tested on CD-RW
class qpxwriter_cd : public qpxwriter {
public:
	qpxwriter_cd(drive_info *idev) : qpxwriter(idev)  {};
	virtual ~qpxwriter_cd() {};

	virtual int send_opc();
//	virtual int open_session();
	virtual int open_track(uint32_t size);
	virtual int close_track();
	virtual int fixate();
protected:
	int set_write_parameters_def(bool bfree, bool simul);
	int send_cue_sheet(uint32_t tsize);
	int write_lead_in();
};

// DVD-R(W) [/DL]
// tested on single layer media only, both -R and -RW OK
class qpxwriter_dvdminus : public qpxwriter {
public:
	qpxwriter_dvdminus(drive_info *idev) : qpxwriter(idev)  {};
	virtual ~qpxwriter_dvdminus() {};

	virtual int open_session();
	virtual int open_track(uint32_t size);
	virtual int close_track();
	virtual int fixate();
};

// DVD+R(W) [/DL]
// tested on single layer media only, both +R and +RW OK
class qpxwriter_dvdplus : public qpxwriter {
public:
	qpxwriter_dvdplus(drive_info *idev) : qpxwriter(idev)  {};
	virtual ~qpxwriter_dvdplus() {};

	virtual int open_session();
	virtual int open_track(uint32_t size);
	virtual int close_track();
	virtual int fixate();
private:
			int fixate_rw();
			int fixate_r();
};

// DVD-RAM
// OK
class qpxwriter_dvdram : public qpxwriter {
public:
	qpxwriter_dvdram(drive_info *idev) : qpxwriter(idev)  {};
	virtual ~qpxwriter_dvdram() {};
};

// BD-R/RE
// untested!!!
class qpxwriter_bd : public qpxwriter {
public:
	qpxwriter_bd(drive_info *idev) : qpxwriter(idev)  {};
	virtual ~qpxwriter_bd() {};
	virtual int open_track(uint32_t size);
	virtual int close_track();
	virtual int fixate();
private:
			int fixate_re();
			int fixate_r();
};

#endif

