/*
 *
 * sector map class header
 * Copyright (C) 2006-2007,2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 */

#ifndef __SECTMAP_H
#define __SECTMAP_H

#define _FILE_OFFSET_BITS 64

#include <inttypes.h>
#include <threads.h>

enum mape {
    BM_WAIT = 0,
    BM_READ = 1,
    BM_DONE = 2,
    BM_FAIL = 3,
    BM_INV  = 15
};

#define map_block_sz	16384

typedef mape map_block[map_block_sz];

extern long fsize(FILE* f);

class smap {
public:
	smap(char* fn, uint32_t sects);
	~smap();
	mape	get(uint32_t sector);
	int32_t	is_done();
	int32_t	get_wait();
	int32_t	get_read();
	int32_t	get_done();
	int32_t	get_fail();
	int32_t	get_tot();
	uint32_t get_next(uint32_t *lba, mape state, int32_t *count);
	void	set(uint32_t sector, mape state, int32_t count=0);
	void	set_one(uint32_t sector, mape state);
	void	fill(mape state);
	void	set_file(char* fn);
	int32_t	load();
	int32_t	save();
	void	lock();
	void	unlock();
private:
	uint32_t	sectors;
	uint32_t	blocks;
	map_block*	arr;
	Mutex*		mutex;
	char*		fname;
};

#endif
