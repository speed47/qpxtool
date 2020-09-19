/*
 *
 * image writer class header
 * Copyright (C) 2007, 2010, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 */

#ifndef __IMGWRITER_H
#define __IMGWRITER_H

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <threads.h>

class smap;

class imgwriter {
public:
	imgwriter(char* fn, smap* map);
	~imgwriter();
	int	write(uint32_t lba, int scnt, int ssz, void* buff);
//	void	set_file(char* fn);
//	int	open();
//	int	close();
private:
	char*		fname;
	FILE*		iso;
	Mutex*		mutex;
};

#endif

