/*
 *
 * reader class header
 * Copyright (C) 2007,2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 */

#ifndef __READER_H
#define __READER_H

#define _FILE_OFFSET_BITS 64

//#include <stdio.h>
//#include <stdlib.h>

//#include <qpx_mmc.h>
//#include <sectmap.h>

#include <threads.h>

const char PASS_FIRST   = 0;
const char PASS_CONT    = 1;
const char PASS_RECOVER = 2;
const char PASS_RECOVER0= PASS_RECOVER;
const char PASS_RECOVER1= 3;

class drive_info;
class map;
class imgwriter;

typedef struct {
    	int		tidx, parent;
	drive_info*	dev;
	smap*		map;
	imgwriter*      iso;
	int		running;
	int		stop;
	int		result;
	int		cnt_ok;
	int		tries;
	char		pass;
} rdparm_t;

class cdvdreader {
public:
	cdvdreader(int itidx, int iparent, drive_info* idev, smap* imap, imgwriter* iiso, int ipass=0, int itries=4);
	~cdvdreader();
	void	set_dev(drive_info* idev);
	void	set_map(smap* imap);
	void	set_iso(imgwriter* iiso);
	void	set_pass(int ipass);
	void	set_retry(int itries);
	int	start();
	void	stop();
	bool	stoped();
	void	wait();
	int	running();
	int	print_stat();

private:
	thread_t	tid;
	rdparm_t	parm;
};

#endif

