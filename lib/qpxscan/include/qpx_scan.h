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

#ifndef __QPX_SCAN_H
#define __QPX_SCAN_H

#include <qpx_mmc.h>
#include <qpx_scan_plugin_api.h>

#include <sys/time.h>

//class drive_info;

typedef char path[128];

static const path ppaths[] = {
#if defined (__unix) || defined (__unix__) || (defined(__APPLE__) && defined(__MACH__))
	"/usr/lib/qpxtool",
	"/usr/local/lib/qpxtool",
	"/usr/lib64/qpxtool",
	"/usr/local/lib64/qpxtool",
#elif defined (_WIN32)
	"plugins",
#endif
	""
};

class qpxwriter;

//static const int ppaths_cnt = sizeof(ppaths) / sizeof(path);

class qscanner {
public:
	qscanner(drive_info* idev);
	~qscanner();
	drive_info*	device() { return dev; };
	void setTestSpeed(int);
	bool setTestWrite(bool);
	int  run(char*);
	int  check_test(unsigned int);
    int  errc_data();
	int* get_test_speeds(unsigned int);
//	int  plugins_probe();
	int  plugins_probe(bool test, bool probe_enable);
	int  plugin_attach_fallback();
	int  plugin_attach(char* name);
	int  plugin_attach(char* pname, bool probe_enable, bool no_detach, bool silent=1);
	void plugin_detach();
//	int  plugin_info();
	const char* plugin_name();
	const char* plugin_desc();
	bool is_plugin_attached() { return attached; };
//		int detect_check_capabilities();
	inline void setInterval(int sta, int end) { lba_sta = sta; lba_end = end; };
	void stop();
	void stat();

private:
	bool	stop_req, stat_req;
	struct	timeval	s,e,blks,blke;
	long	lba_sta, lba_end;
	int		spd1X;
	int		spdKB;
	float   spdX;
	bool	WT_simul;

	//void	show_avg(struct timeval s, struct timeval e, long lba);
	void	show_avg_speed(long lba);
	//void	calc_cur_speed(long sects, int* spdKB, float* spdX);
	void	calc_cur_speed(long sects);

	int	readline(int fd, char *buf, int maxlen);

	int	run_rd_transfer();
	int	run_wr_transfer();
	int	run_fete();

	int	run_cd_errc();
	int	run_cd_jb();
	int	run_cd_ta();

	int	run_dvd_errc();
	int	run_dvd_jb();
	int	run_dvd_ta();

	int	run_bd_errc();

	int				speed;
	bool			attached;
	drive_info		*dev;
	scan_plugin		*plugin;
	qpxwriter		*writer;

#if defined (_WIN32)
	HINSTANCE__     *pluginlib;
#else
	void			*pluginlib;
#endif

	scan_plugin*	(*plugin_create)  (drive_info*);
	void			(*plugin_destroy) (scan_plugin*);
	bool			listed; 
	//		unsigned int chk_features; // media check features
	char			tchar;
};

#endif // __QPX_SCAN_H

