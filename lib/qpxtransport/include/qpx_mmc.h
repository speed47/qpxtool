/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef __qpxtool_mmc_h
#define __qpxtool_mmc_h

//#include <qpx_const.h>
#include "qpx_transport.h"
#include "common_functions.h"

//#ifdef HAVE_LIMITS_H
#include <limits.h>
//#endif
#include <inttypes.h>

#include "qpx_mmc_defs.h"

#define bufsz_dev  0x0000FF
#define bufsz_rd   0x010000
#define bufsz_ATIP 0x000800

typedef struct{
	int	n;
	int	session;
	int	track_mode;
	int	data_mode;
	int	start;
	msf	msf_start;
	int	next_writable;
	msf	msf_next;
	int	free;
	msf	msf_free;
	int	packet_size;
	int	size;
	msf	msf_size;
	int	last;
	msf	msf_last;
//	int	end;
//	msf	msf_end;
} trk;

typedef struct{
	int	test;
	int	idx;
	int32_t	lba;
	int	block,blocks;
	float	speed_kb;
	float	speed_x;
	int	speed_h;
	int	err_total;
	int	err_max;
	int	err_cur;
	int	err_min;
	int	err_m;
	int	err_d;
	float	err_avg;
	int	pit;
	int	land;
	float	jmax;
	float	jmin;
	float	bmax;
	float	bmin;
	long	time;
	int	nte, ote;
	int	nfe, ofe;
	int	ext;
} block_data;

#define DVD_KEY_SIZE 5

#define DVDCSS_KEY_CACHE  0

#define DVDCSS_METHOD_NONE  0
#define DVDCSS_METHOD_KEY   1
#define DVDCSS_METHOD_DISC  2
#define DVDCSS_METHOD_TITLE 3

#define DVDCSS_BLOCK_SIZE 2048

#define DVDCSS_NOFLAGS		0
#define DVDCSS_READ_DECRYPT (1 << 0)
#define DVDCSS_SEEK_MPEG	(1 << 0)
#define DVDCSS_SEEK_KEY		(1 << 1)

typedef uint8_t dvd_key_t[DVD_KEY_SIZE];

typedef struct dvd_title_s
{
    int                 i_startlb;
    dvd_key_t           p_key;
    struct dvd_title_s *p_next;
} dvd_title_t;

typedef struct {
	bool			asf;	/* Authenication Success Flag */
	uint8_t	agid;	/* Current Authenication Grant ID */
	dvd_key_t		BK;		/* Current session key (Bus Key) */
	dvd_key_t		DK;		/* This DVD disc's key */
	uint8_t	CK[2*DVD_KEY_SIZE];
	dvd_key_t		K1;
	dvd_key_t		K2;
	dvd_key_t		TK;		/* Current title key */
	dvd_title_s		*p_titles;	

	uint8_t	protection;
	uint8_t	regmask;
	int method;
	int pos;
#if (DVDCSS_KEY_CACHE > 0)
	char psz_cachefile[PATH_MAX];
	char *psz_block;
#endif
} dvdcss_t;

#define MID_RAW_MAX  1024

enum mid_type_t {
	MID_type_NONE = 0,
	MID_type_CD   = 1,
	MID_type_DVDp = 2,
	MID_type_DVDm = 4,
	MID_type_DVDRAM = 8,
	MID_type_BD = 16
};

typedef	struct {
	char		MID[48];	// MediaID for DVD, manufacturer for CD
	mid_type_t  MID_type;
	uint8_t		ATIP[bufsz_ATIP];
	int			ATIP_size;
	uint16_t	MID_size;
	unsigned char MID_raw[4+MID_RAW_MAX];
	uint64_t	type;	    // Media subtype
	uint8_t		book_type;	// Book type (DVD)
	uint8_t		max_rate;
	uint8_t		disc_size;	// indicates 120/80mm disc
	uint8_t		polarity;   // Push-Pull polarity flags per layer for BD (indicates HtL or LtH)
	uint8_t		layers;		// Layers num (!CD)	
	int			sectsize;
	int32_t	capacity;	// Recorded capacity in sectors
	msf		capacity_msf;
	int32_t	capacity_free;	// Free sectors
	msf		capacity_free_msf;
	int32_t	capacity_total;	// Total sectors
	msf		capacity_total_msf;
	int		spare_psa_total,
			spare_ssa_total,
			spare_psa_free,
			spare_ssa_free;
	int		last_lead_out;
	int		dstatus;	// Empty/Apeendable/Complete
	int		sstatus;
	int		sessions;
	int		tracks;
	int		erasable;
	char	writer[0x3F];
	trk		track[0xFF];
	dvdcss_t dvdcss;
} media_info;

#define speed_tbl_size       64
#define STATUS_OPEN			 0x0001
#define STATUS_MEDIA_PRESENT 0x0002
#define STATUS_LOCK			 0x0004

typedef struct {
	uint8_t	status;
	uint8_t	event;
	int	interval;
	int	tests;
	int8_t	spindown_idx;
	int16_t	speed_idx;
	float	speed_mult;
	int16_t	speed_tbl[speed_tbl_size];
	int32_t	speed_tbl_kb[speed_tbl_size];
	int16_t	wr_speed_tbl[speed_tbl_size];
	int32_t	wr_speed_tbl_kb[speed_tbl_size];
	float	wr_speed_tbl_media[speed_tbl_size];
	int32_t	scan_speed_cd;
	int32_t	scan_speed_dvd;
	int32_t	read_speed_kb;
	int32_t	read_speed_cd;
	int32_t	read_speed_dvd;
	int32_t	max_read_speed_kb;
	int32_t	max_read_speed_cd;
	int32_t	max_read_speed_dvd;
	int32_t	max_write_speed_kb;
	int32_t	max_write_speed_cd;
	int32_t	max_write_speed_dvd;
	int32_t	write_speed_kb;
	int32_t	write_speed_cd;
	int32_t	write_speed_dvd;
} drive_parms;

typedef struct {
	int32_t	lba_s;
	int32_t spd_s;
	int32_t	lba_e;
	int32_t spd_e;
} perf_desc;

typedef struct {
	int	max;
	int	min;
	int	m;
	int	d;
} err;

/*
typedef struct {
	err*	BLER;
	err*	E11;
	err*	E21;
	err*	E31;
	err*	E12;
	err*	E22;
	err*	E32;
	int	tot[7];
	int	max[7];
	float	avg[7];
	uint32_t	color[7];
	uint32_t	colorl[7];
} _Exx;
*/

typedef struct
{
	char	type;
	char	len;
	char	state;
	char	rd;
	char	wr;
	char	access;
	char	eject;
	char	load;

	char	psaved;
	char	pstate;
	char	prd_cd;
	char	prd_dvd;
	char	pwr_cd;
//	char	pwr_dvd;
	char	paccess;
	char	peject;
	char	pload;
} plex_silent;

typedef struct {
	uint8_t	number;
	uint8_t	type;  // 0x25 == -R, 0xA1 == +R
	char	MID[12];
	uint8_t	crap2;
	uint8_t	enabled;
	char	counter[2];
	uint8_t	speed;
	uint8_t	crap3[13];
} as_entry;

typedef struct {
	uint8_t	number[2];
	uint8_t	crap[30];
} as_data;

typedef struct {
	char		sizeb[2];
	char		crap1[4];
	char		dbcnt;
	char		entry_size;
	as_entry	entry[32];
	as_data		entry_data[32][7];
	char		state;
	int			size;
} plex_as;

typedef struct {
	bool	 ok;
	uint16_t dn;
	hms		 cr,
			 cw,
			 dr,
			 dw;
} plex_life;

typedef struct {
	char	gigarec;	// Current GigaRec value
	char	gigarec_disc;	// GigaRec value of inserted CD
	char	powerec_state;	// Current PoweRec state
	uint16_t	powerec_spd;	// Current PoweRec recomended speed
	char	varirec_state_cd;	// Current VariRec CD state
	char	varirec_pwr_cd;		// Current VariRec CD LaserPower
	char	varirec_str_cd;		// Current VariRec CD Strategy
	char	varirec_state_dvd;	// Current VariRec DVD state
	char	varirec_pwr_dvd;	// Current VariRec DVD LaserPower
	char	varirec_str_dvd;	// Current VariRec DVD Strategy
	char	hcdr;		// Hide CD-R State
	char	securec;
	char	securec_disc;
	char	sss;		// SingleSession State
	char	spdread;	// SpeedRead
	char	testwrite_dvdplus; // Simulation on DVD+R(W)
	char	plexeraser;	// PlexEraser mode
} plex_features;

typedef struct {
	char    	amqr;
	char		forcespeed;
	uint32_t	tattoo_i;
	uint32_t	tattoo_o;
	uint32_t	tattoo_rows;
} yamaha_features;

typedef struct {
	char	silent;
	char	limit;
	bool	peakpower;
	char	pureread;	
} pio_quiet;

typedef struct {
	uint8_t phase;
	uint8_t region;
	uint8_t ch_u;
	uint8_t ch_v;
} rpc_state;

class	drive_info {
public:
	drive_info(const char* _device);
	~drive_info();

//	bool	isBusy();
//	bool	lock();
//	bool	unlock();
//	void	wait_free();

	Scsi_Command	cmd;
	int			err;

	char*		device;		// device adress
	char		ven[9];		// vendor string
	uint32_t	ven_ID;		// drive vendor ID
	char		dev[17];	// model string
	uint32_t	dev_ID;		// model ID
	char		fw[5];		// FirmWare
	char		serial[17];	// drive serial#
	char		TLA[5];		// TLA# - only rof Plextor PX-712, PX-716
	uint16_t	ver_id[8];

//	int	z;

	uint32_t	buffer_size;	 // drive buffer size
	uint64_t	capabilities;	 // common capabilities
	uint64_t	rd_capabilities; // read capabilities
	uint64_t	wr_capabilities; // write capabilities
	uint32_t	wr_modes;		// write modes
	uint32_t	ven_features;	// vendor-specific features
	uint32_t	chk_features;	// media check features

	uint32_t	iface_id;
	str_if		iface;
	uint8_t		loader_id;

	short		book_plus_r;
	short		book_plus_rw;
	short		book_plus_rdl;

	yamaha_features yamaha;
	plex_features	plextor;
	plex_life   life;
	plex_as		astrategy;
	plex_silent	plextor_silent;
	pio_quiet	pioneer;
	media_info	media;
	drive_parms	parms;
	perf_desc	perf;

	uint8_t*	rd_buf;
	char		mmc;

	rpc_state	rpc;

	bool		get_performance_fail;
	char		silent;
private:
	bool		busy;
};

static const int drive_info_size=sizeof(drive_info);

extern int print_sense (int err);
extern int print_opcode (uint8_t opcode);

extern int scanbus(int vendor_mask=0);
extern int inquiry(drive_info* drive);
extern int isPlextor(drive_info* drive);
extern int isPlextorLockPresent(drive_info* drive);
extern int isYamaha(drive_info* drive);
extern int isPioneer(drive_info* drive);

extern int test_unit_ready(drive_info* drive);
extern int wait_unit_ready(drive_info* drive, int secs, bool need_media=1);
extern int check_burnfree(drive_info* drive);
extern int check_write_modes(drive_info* drive);
extern int reserve_track(drive_info* drive, uint32_t size);
extern int close_track_session(drive_info* drive, int n, int cltype);
extern int wait_fix(drive_info* drive, int secs);
extern int request_sense(drive_info* drive, char add);
extern int  get_configuration(drive_info* drive, int feature_number, uint32_t* data_length, int* current, uint8_t ReqType = 0x02);
extern void detect_iface(drive_info* drive);

extern int write_buffer(drive_info* drive, uint8_t mode, uint8_t buff_id, uint32_t offs, uint32_t len);
extern int read_buffer(drive_info* drive, uint8_t mode, uint8_t buff_id, uint32_t offs, uint32_t len);
extern int test_dma_speed(drive_info* drive, long msecs = 250);
extern int flush_cache(drive_info* drive, bool IMMED);
extern int set_cache(drive_info* drive, bool rd, bool wr);
extern int get_cache(drive_info* drive, bool *rd = NULL, bool *wr = NULL);

// some DVD related functions
extern int get_rpc_state(drive_info* drive);
extern int read_disc_regions(drive_info* drive);

extern int css_disckey( drive_info* drive);
extern int css_title ( drive_info* drive, int32_t lba);
//extern int css_titlekey( drive_info* drive, int lba, dvd_key_t p_title_key );
//extern int css_unscramble( dvd_key_t p_key, uint8_t *p_sec );
//extern void css_printkey (char *, uint8_t const * );

// device capabilities detection functions
extern int get_profiles_list(drive_info* drive);
extern int get_features_list(drive_info* drive);
extern int get_mode_pages_list(drive_info* drive);
extern void detect_capabilities(drive_info* drive);

// media information functions
extern int read_atip(drive_info* drive, int silent);
extern int read_toc(drive_info* drive, int silent);
extern int read_track_info(drive_info* drive, trk* track, uint32_t track_n);
extern int get_track_list(drive_info* drive);
extern int read_disc_information(drive_info* drive);

extern int read_capacities(drive_info* drive);
extern int read_disc_info(drive_info* drive, int len);
extern int determine_cd_type(drive_info* drive);
//extern int determine_cdrw_subtype(drive_info* drive);
extern int read_mediaid_dvd(drive_info* drive);
extern int read_mediaid_dvdram(drive_info* drive);
extern int read_mediaid_dvdminus(drive_info* drive);
extern int read_mediaid_dvdplus(drive_info* drive);
extern int read_mediaid_bd(drive_info* drive);
extern int determine_disc_type(drive_info* drive);

extern int mode_sense(drive_info* drive, int page, int page_control, int dest_len);
extern int mode_select(drive_info* drive, int dest_len);

// speed settings
extern int get_spindown(drive_info* drive);
extern int set_spindown(drive_info* drive);
extern int get_performance(drive_info* drive, bool rw, uint8_t type);
extern int get_write_speed_tbl(drive_info* drive);
extern int set_streaming(drive_info* drive);
extern int get_rw_speeds(drive_info* drive);
extern int set_rw_speeds(drive_info* drive);
extern int detect_speeds(drive_info *drive);

// media change detection, media lock, load/eject
extern int get_media_status(drive_info* drive);
extern int start_stop(drive_info* drive, bool start);
extern int load_eject(drive_info* drive, bool load, bool IMMED);
extern int load_eject(drive_info* drive, bool IMMED);
extern int get_lock(drive_info* drive);
extern int set_lock(drive_info* drive);

//extern void spinup(drive_info* drive);
extern void spinup(drive_info* drive, uint8_t secs = 2);
extern int seek(drive_info* drive, int32_t lba, uint8_t flags = 0);
extern int play_audio_msf(drive_info* drive, msf beg, msf end);
extern int play_audio(drive_info* drive, int32_t beg, short int len);
extern int read_cd(drive_info* drive, uint8_t *data, int32_t lba, int sector_count, uint8_t flags, uint8_t FUA = 0);
extern int read(drive_info* drive, uint8_t *data, int32_t lba, int sector_count, uint8_t FUA = 0);
extern int read_one_ecc_block(drive_info* drive, uint8_t *data, int32_t lba);
extern int get_drive_serial_number(drive_info* drive);
extern int get_buffer_capacity(drive_info* drive);
extern int get_wbuffer_capacity(drive_info* drive, uint32_t *btot, uint32_t *bfree);
extern int read_writer_info(drive_info* drive);

extern int read_dvd(drive_info* drive, uint8_t *data, int32_t lba, int sector_count, int flags = 0);
extern int seek_dvd( drive_info* drive, int32_t lba, int flags );

extern int detect_mm_capabilities(drive_info* drive);
//extern int detect_check_capabilities(drive_info* drive);
//extern int convert_to_ID (drive_info* drive);


extern int plextor_px755_do_auth(drive_info* dev);
extern int plextor_px755_get_auth_code(drive_info* dev, unsigned char* auth_code);
extern int plextor_px755_send_auth_code(drive_info* dev, unsigned char* auth_code);
//	extern int cmd_px755_clear_auth_status(drive_info* dev);
extern int plextor_px755_calc_auth_code(drive_info* dev, unsigned char* auth_code);

#endif

