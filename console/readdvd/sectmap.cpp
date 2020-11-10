/*
 *
 * sector map class for DeadDiscReader
 * Copyright (C) 2006-2007,2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 * it uses QPxTool SCSI transport library
 *
 */

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "sectmap.h"

long fsize(FILE* f){
    struct stat st;
    fstat(fileno(f), &st);
    return st.st_size;
}

smap::smap(char* fn, unsigned int sects) {
    sectors=sects;
    fname = fn;
    blocks=(sectors/map_block_sz) + !!(sectors%map_block_sz);
    arr=(map_block*)malloc(sizeof(map_block)*blocks);
    fill(BM_WAIT);
    mutex = new Mutex();
    printf("* map: created for %u sectors\n", sectors);
}

smap::~smap() {
	save();
	delete mutex;
    delete arr;
}

mape smap::get(unsigned int sector) {
    if (sector > sectors) return BM_INV;
    unsigned int row=sector/map_block_sz;
    unsigned int col=sector%map_block_sz;
    return arr[row][col];
}

int smap::is_done() {
    return (sectors-get_done());
}

int smap::get_wait() {
    unsigned int cnt=0;
    for (unsigned int i=0; i<sectors; i++) if (!get(i)) cnt++;
    return cnt;
}

int smap::get_read() {
    unsigned int cnt=0;
    for (unsigned int i=0; i<sectors; i++) if (get(i) == BM_READ) cnt++;
    return cnt;
}

int smap::get_done() {
    unsigned int cnt=0;
    for (unsigned int i=0; i<sectors; i++) if (get(i) == BM_DONE) cnt++;
    return cnt;
}

int smap::get_fail() {
    unsigned int cnt=0;
    for (unsigned int i=0; i<sectors; i++) if (get(i) == BM_FAIL) cnt++;
    return cnt;
}

int smap::get_tot() { return sectors; }

unsigned int smap::get_next(unsigned int *lba, mape state, int *count) {
    unsigned int offs=0;
    int cnt=0;
    int icount=1;
	if (count) icount = *count;

//	if (lba+scnt > sectors) scnt = sectors - lba;

//	printf("smap::get_next(%x,%d,%d)\n",lba ? *lba : -1,state, count ? *count : -1);

    if (lba) offs=*lba;
    if (icount<2) {
		while ((get(offs) != state) && (offs<sectors)) offs++;
    } else {
		while ((cnt<icount) && (offs<sectors)) {
			if (get(offs+cnt) == state) {
				cnt++;
			} else {
				offs+=cnt;
				offs++;
				cnt=0;
			}
	    }
    }
//	printf("capacity: %x, offs: %x\n", sectors, offs);
    if (offs >= sectors) {
		if (icount > 2) {
//			printf("Can't find %d sectors block", icount);
			(*count)--;
			return get_next(lba, state, count);
		} else {
			offs = 0xFFFFFFFF;
		}
	}
    if (lba)   *lba=offs;
	if (count) *count = icount;
    return offs;
}


void smap::set(unsigned int sector, mape state, int count) {
//	    if (sector>sectors) return 1;
//	    if (sector+count>sectors) return 2;
    if(count) {
		for (int i=0; i<count; i++) set_one(sector+i, state);
    } else {
		set_one(sector, state);
    }
//    return 0;
}


void smap::set_one(unsigned int sector, mape state) {
    unsigned int row=sector/map_block_sz;
    unsigned int col=sector%map_block_sz;
    if (sector > sectors) return;
    arr[row][col]=state;	    
}


void smap::fill(mape state){
    for (unsigned int i=0; i<blocks; i++)
	for (unsigned int j=0; j<map_block_sz; j++) arr[i][j]=state;
}

void smap::set_file(char* fn) { fname = fn; }

int smap::load() {
    FILE* f;
    int s;
    map_block	tarr;
    printf("loading map : '%s'... ", fname);
    if (!(f = fopen(fname,"r"))){
		printf("can't open map file!\n");
		return 1;
    }
    s=fsize(f);
    fseek(f, 0, SEEK_SET);
    for (unsigned i=0; (i<blocks) && (!feof(f)); i++) {
	if (fread((void*)&tarr, map_block_sz, sizeof(mape), f) < sizeof(mape)) {
		printf("error reading map file!\n");
		return 1;
	}
	for (unsigned j=0; (j<map_block_sz) && (i*map_block_sz+j)<(s/sizeof(mape)); j++)
	set(i*map_block_sz+j, tarr[j]);
    }
    fclose(f);
    printf("done\n");
    return 0;
}

int smap::save() {
    FILE* f;
    printf("\nsaving map  : '%s'...\n", fname);
    if (!(f = fopen(fname,"w"))){
		printf("can't create map file!\n");
		return 1;
    }
    fseek(f, 0, SEEK_SET);
    for (unsigned i=0; i<blocks; i++)
	fwrite((void*)arr[i], map_block_sz, sizeof(mape), f);
    fclose(f);
    printf("\nSector map saved!\n");
    return 0;
}

void smap::lock()   { mutex->lock(); }

void smap::unlock() { mutex->unlock(); }

