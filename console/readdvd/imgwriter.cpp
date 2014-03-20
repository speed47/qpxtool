/*
 *
 * image writer class for DeadDiscReader
 * Copyright (C) 2007, 2010, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 */

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>

#include <sectmap.h>
#include "imgwriter.h"

#ifndef HAVE_FOPEN64
#define fopen64 fopen
#endif

imgwriter::imgwriter(char* fn, smap* map) {
	mutex = new Mutex();
    fname=fn;

#if !( defined(HAVE_FSEEKO) && defined(OFFT_64BIT) ) && !defined(HAVE_FSEEK64)
	printf("Warning! No 64-bit file offset. Image size limits to 2GiB\n");
#endif
    if (!(iso = fopen64(fname,"r+"))){
		printf("can't open image file, creating new one!\n");
		if (!(iso = fopen64(fname,"w+"))){
		    printf("can't create image file!\n");
		}
    } else {
		printf("image opened: '%s'\n", fname);
		map->load();
    }
	if (iso) fclose(iso);
}

imgwriter::~imgwriter() {
	delete mutex;
}

int imgwriter::write(int lba, int scnt, int ssz, void* buff) {
    int res=0;
#if defined(HAVE_FSEEKO) && defined(OFFT_64BIT)
	off_t   offs = ssz*(off_t)lba;
#else
	int64_t   offs = ssz*(int64_t)lba;
#endif

	mutex->lock();
    iso = fopen64(fname, "r+");

	if (iso) {
#if defined(HAVE_FSEEKO) && defined(OFFT_64BIT)
		if (fseeko(iso, offs, SEEK_SET))
#elif defined(HAVE_FSEEK64)
		if (fseek64(iso, offs, SEEK_SET))
#else
		if (fseek(iso, offs, SEEK_SET))
#endif
		{
		    printf("\nseek() failed! Offs: %lld (%08LX)\n", offs, offs);
			mutex->unlock();
		    return 0;
		}
		res = fwrite(buff, ssz, scnt, iso);
//		printf("\nwrote: %ld of %ld\n", res, scnt);
		fclose(iso);
    }
    mutex->unlock();
    return res;
}

//void	imgwriter::set_file(char* fn) { fname=fn; }

//int	imgwriter::open(){}

//int	imgwriter::close(){}
