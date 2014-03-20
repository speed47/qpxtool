/*
 *
 * reader class for DeadDiscReader
 * Copyright (C) 2007,2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 */

#define _FILE_OFFSET_BITS 64


#include <stdio.h>
#include <unistd.h>
//#include <stdlib.h>

#include <qpx_mmc.h>
#include <sectmap.h>
//#include <pthread.h>

#include <reader_disc.h>
#include <imgwriter.h>

#include "reader.h"

#define sector_sz 2048

cdvdreader::cdvdreader(
    int itidx, int iparent, drive_info* idev, smap* imap, imgwriter* iiso, int ipass, int itries)
{
    parm.running=0;
    parm.stop=0;
    parm.tidx=itidx;
    parm.parent=iparent;
    parm.dev=idev;
    parm.map=imap;
    parm.iso=iiso;
    parm.pass=ipass;
    parm.tries=itries;

//    printf("%02d: device: %s\n", parm.tidx, parm.dev->device);
//    printf("%02d: %s %s %s\n", parm.tidx, parm.dev->ven, parm.dev->dev, parm.dev->fw);
}

cdvdreader::~cdvdreader() {
}

void cdvdreader::set_dev(drive_info* idev) { parm.dev=idev; }

void cdvdreader::set_map(smap* imap) { parm.map=imap; }

void cdvdreader::set_iso(imgwriter* iiso) { parm.iso=iiso; }

void cdvdreader::set_pass(int ipass) { parm.pass=ipass; }

void cdvdreader::set_retry(int itries) { parm.tries=itries; }

int  cdvdreader::start() {
//    printf("Creating thread for %s\n", parm.dev->device);
    parm.running=1;
    return thread_create(&tid, NULL,read_disc,(void*)(&parm));
//    printf("pthread for %s created successfully:)\n", parm.dev->device);
}

void cdvdreader::stop() { parm.stop=1; }

bool cdvdreader::stoped() { return parm.stop; }

void cdvdreader::wait() {
//    int x;
    while (parm.running) msleep(1);
//    x=
    thread_join(tid, NULL);
//    printf("thread %d exit state: %d\n", parm.tidx, x);
}

int  cdvdreader::running() { return parm.running; }

int  cdvdreader::print_stat() {
    printf("%s: %7d (%06x) sectors read\n", parm.dev->device, parm.cnt_ok, parm.cnt_ok);
    return parm.cnt_ok;
}

