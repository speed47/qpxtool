/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2007-2008, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef __QPX_SCAN_PLUGIN_API_H
#define __QPX_SCAN_PLUGIN_API_H


#include <stdio.h>
#include <qpx_mmc.h>

/*
typedef enum testID{
	none = 0,
	cd_errc,
	cd_jb,
	cd_fete,
	cd_ta,
	dvd_errc,
	dvd_jb,
	dvd_fete,
	dvd_ta
};
*/

#define CHK_RD_CD		0x00000001
#define CHK_WR_CD		0x00000002
#define CHK_RD_DVD		0x00000004
#define CHK_WR_DVD		0x00000008

#define CHK_ERRC		0x00000010 // Error correction
#define CHK_JB			0x00000020 // Jitter/Asymmetry
#define CHK_FETE		0x00000040 // Focus/Tracking
#define CHK_TA			0x00000080 // Time Analyser

#define CHK_ERRC_CD		0x00000100
#define CHK_JB_CD		0x00000200
#define CHK_FT_CD		0x00000400
#define CHK_TA_CD		0x00000800
#define CHK_TA_CDROM	0x00001800

#define CHK_ERRC_DVD	0x00002000
#define CHK_JB_DVD		0x00004000
#define CHK_FT_DVD		0x00008000
#define CHK_TA_DVD		0x00010000
#define CHK_TA_DVDROM	0x00020000

#define CHK_RD_BD		0x00040000
#define CHK_WR_BD		0x00080000
#define CHK_ERRC_BD		0x00100000

// ERRC returned data
#define ERRC_DATA_BLER	0x01
#define ERRC_DATA_E11	0x02
#define ERRC_DATA_E21	0x04
#define ERRC_DATA_E31	0x08
#define ERRC_DATA_E12	0x10
#define ERRC_DATA_E22	0x20
#define ERRC_DATA_E32	0x40

#define ERRC_DATA_PIE	0x06
#define ERRC_DATA_PIF	0x08
#define ERRC_DATA_POE	0x30
#define ERRC_DATA_POF	0x40

#define ERRC_DATA_LDC	0x02
#define ERRC_DATA_BIS	0x10

#define ERRC_DATA_UNCR	0x80

static const char errc_names_cd[][5]  = { "BLER", "E11", "E21", "E31", "E12", "E22", "E32", "UNCR" };
static const char errc_names_dvd[][5] = { "---",  "PIE", "PI8", "PIF", "POE", "PO8", "POF", "UNCR" };
static const char errc_names_bd[][5] = { "---",   "LDC", "---", "---", "BIS", "---", "---", "UNCR" };

#define DEV_OK		0
#define DEV_PROBED	1
#define DEV_FAIL	2

struct drivedesc{
	char ven[9];
	int  ven_ID;
	char dev[17];
	int  dev_ID;
	int  tests;
};

typedef struct drivedesc drivedesclist[];

class cd_errc {
public:
	cd_errc() : bler(0), e11(0), e21(0), e31(0), e12(0), e22(0), e32(0), uncr(0) {};
	~cd_errc() {};
	inline cd_errc& EMAX(cd_errc& o) {
				if (bler<o.bler) bler=o.bler;
				if (e11<o.e11) e11=o.e11;
				if (e21<o.e21) e21=o.e21;
				if (e31<o.e31) e31=o.e31;
				if (e12<o.e12) e12=o.e12;
				if (e22<o.e22) e22=o.e22;
				if (e32<o.e32) e32=o.e32;
				if (uncr<o.uncr) uncr=o.uncr;
				return *this;};
	inline cd_errc& operator= (cd_errc& o) {
				bler=o.bler;
				e11=o.e11;
				e21=o.e21;
				e31=o.e31;
				e12=o.e21;
				e22=o.e22;
				e32=o.e32;
				uncr=o.uncr;
				return *this; };
	inline cd_errc& operator+= (cd_errc& o) {
				bler+=o.bler;
				e11+=o.e11;
				e21+=o.e21;
				e31+=o.e31;
				e12+=o.e21;
				e22+=o.e22;
				e32+=o.e32;
				uncr+=o.uncr;
				return *this; };

	long bler;
	long e11;
	long e21;
	long e31;
	long e12;
	long e22;
	long e32;
	long uncr;
};

class dvd_errc {
public:
	dvd_errc() : pie(0), pif(0), poe(0), pof(0), uncr(0) {};
	~dvd_errc() {};
	inline dvd_errc& EMAX(dvd_errc& o) {
				if (pie<o.pie) pie=o.pie;
				if (pif<o.pif) pif=o.pif;
				if (poe<o.poe) poe=o.poe;
				if (pof<o.pof) pof=o.pof;
				if (uncr<o.uncr) uncr=o.uncr; return *this;};
	inline dvd_errc& operator= (dvd_errc& o) {
				pie=o.pie;
				pif=o.pif;
				poe=o.poe;
				pof=o.pof;
				uncr=o.uncr; return *this; };
	inline dvd_errc& operator+= (dvd_errc& o) {
				pie+=o.pie;
				pif+=o.pif;
				poe+=o.poe;
				pof+=o.pof;
				uncr+=o.uncr; return *this; };

	long pie;
	long pif;
	long poe;
	long pof;
	long uncr;
};

class bd_errc {
public:
	bd_errc() : ldc(0), bis(0), uncr(0) {};
	~bd_errc() {};
	inline bd_errc& EMAX(bd_errc& o) {
				if (ldc<o.ldc) ldc=o.ldc;
				if (bis<o.bis) bis=o.bis;
				if (uncr<o.uncr) uncr=o.uncr; return *this;};
	inline bd_errc& operator= (bd_errc& o) {
				ldc=o.ldc;
				bis=o.bis;
				uncr=o.uncr; return *this; };
	inline bd_errc& operator+= (bd_errc& o) {
				ldc+=o.ldc;
				bis+=o.bis;
				uncr+=o.uncr; return *this; };

	long ldc;
	long bis;
	long uncr;
};

class cdvd_jb {
public:
	cdvd_jb() {jitter=0; asymm=0; set=0; };
	~cdvd_jb() {};
	int   jitter;
	short asymm;
	inline cdvd_jb& EMAX(cdvd_jb& o) {
		if (!set) {
			jitter = o.jitter;
			asymm  = o.asymm;
			set    = 1;
		} else {
			if (o.jitter > jitter) jitter = o.jitter;
			if (o.asymm  > asymm)  asymm  = o.asymm;
		}
		return *this;
	};
	inline cdvd_jb& EMIN(cdvd_jb& o) {
		if (!set) {
			jitter = o.jitter;
			asymm  = o.asymm;
			set    = 1;
		} else {
			if (o.jitter < jitter) jitter = o.jitter;
			if (o.asymm  < asymm)  asymm  = o.asymm;
		}
		return *this;
	};
private:
	bool  set;
};

class cdvd_ft {
public:
	cdvd_ft() {fe=0; te=0;}
	~cdvd_ft() {};
	inline cdvd_ft& EMAX(cdvd_ft& o) {
		if (o.fe > fe) fe = o.fe;
		if (o.te > te) te = o.te;
		return *this;
	};
	inline cdvd_ft& operator= (cdvd_ft& o)  { fe  = o.fe; te  = o.te; return *this; };
	inline cdvd_ft& operator+= (cdvd_ft& o) { fe += o.fe; te += o.te; return *this; };
	int fe;
	int te;
};

#define TA_HIST_SIZE 512

class cdvd_ta {
public:
	cdvd_ta() { pass=-1; clear(); };
	~cdvd_ta() {};

	void clear() {
		memset(pit, 0,  sizeof(int32_t)*TA_HIST_SIZE);
		memset(land, 0, sizeof(int32_t)*TA_HIST_SIZE);
	};

	int pass;
	int32_t pit[TA_HIST_SIZE];
	int32_t land[TA_HIST_SIZE];
};
	
class drive_info;

class scan_plugin {
public:
    scan_plugin(drive_info* idev=NULL) { devlist = NULL; blklist = NULL; }
    virtual ~scan_plugin() {}
    virtual int  probe_drive() { return DEV_FAIL; }
//    virtual int  check_drive()=0;
    virtual int  check_test(unsigned int test)=0;
    virtual int  errc_data()=0;
    virtual int* get_test_speeds(unsigned int test) { return NULL; };
    virtual int  start_test(unsigned int test, long slba, int &speed)=0;
    virtual int  scan_block(void* data, long* ilba)=0;
    virtual int  end_test()=0;

    virtual const char* name()=0;
    virtual const char* desc()=0;
    drivedesc *devlist;
    drivedesc *blklist;

protected:

	void set_read_speed(int &speed) {
		dev->parms.read_speed_kb = (int) (speed * dev->parms.speed_mult);
		set_rw_speeds(dev);
		get_rw_speeds(dev);
		speed = (int) (dev->parms.read_speed_kb / dev->parms.speed_mult);
	}
	void set_write_speed(int &speed) {
		dev->parms.write_speed_kb = (int) (speed * dev->parms.speed_mult);
		set_rw_speeds(dev);
		get_rw_speeds(dev);
		speed = (int) (dev->parms.write_speed_kb / dev->parms.speed_mult);
	}
	drive_info*  dev;
    unsigned int test;
};

extern "C" {
    scan_plugin* plugin_create(drive_info* idev);
    void		 plugin_destroy(scan_plugin* iplugin);
}

//typedef char drive_supported(drive_info* drive);

#endif // __QPX_SCAN_PLUGIN_API_H

