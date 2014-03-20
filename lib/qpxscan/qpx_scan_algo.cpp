/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2007-2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <qpx_scan.h>
#include <threads.h>

#include <qpx_writer.h>

#define EMAXLINE 1024

#define USE_FFLUSH

//void qscanner::show_avg(struct timeval s, struct timeval e, long lba)
void qscanner::show_avg_speed(long lba)
{
	double btime;
	int spdKB;
	float spdX;
	btime = (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)/1000000.0;
	spdKB = (int) (((lba - lba_sta + 1) << 1) / btime);
	spdX  = spdKB/(float)spd1X;
	printf("\nTest time: %6.2fs\navg speed: %5.3f X  %5d kB/s\n", btime, spdX, spdKB);
}

void qscanner::calc_cur_speed(long sects)
{
	double btime;
//	int spdKB;
//	float spdX;
	btime = (blke.tv_sec - blks.tv_sec) + (blke.tv_usec - blks.tv_usec)/1000000.0;
	spdKB = (int) ((sects << 1) / btime);
	spdX  = spdKB/(float)spd1X;
//	printf("\nTest time: %6.2fs\navg speed: %5.3f X  %5d kB/s\n", btime, spdX, spdKB);	
}

int qscanner::readline(int fd, char *buf, int maxlen) {
	int cnt=0;
	char *cbuf=buf;
	int r;
	int sret;
	fd_set rd_set;
	timeval tv;

	FD_ZERO(&rd_set);
	if (tchar>=0) {
		cbuf[0] = tchar;
		cnt++;
		cbuf++;
		tchar = -1;
	}
	while ( !stop_req && (cnt<(maxlen-1))) {
		FD_SET(fd, &rd_set);
		tv.tv_sec  = 1;
		tv.tv_usec = 0;

#ifndef _WIN32
		sret = select(fd+1, &rd_set, NULL, NULL, &tv);
#else
//		sret = WaitForSingleObject((void*)fd, 1000);
//		errno = GetLastError();
		sret  = 1;
		errno = 0;
#endif
//		printf("readline select(): %d, errno = [%d] %s\n", sret, errno, strerror(errno));
		if (sret < 0) {
			printf("select():  %s\n", strerror(errno));
			if (errno == EINTR) continue;
			return -1;
		}

		else if (sret > 0 && FD_ISSET(fd, &rd_set)) {
			r = read(fd, cbuf, 1);
			if (r<0) {
				printf("read = %d, %d, %s\n", r, errno, strerror(errno));
				switch (errno) {
					case EAGAIN:
						printf("EAGAIN\n");
						continue;
					case EINTR: 
						printf("EINTR\n");
						continue;
					default:
						return -1;
				}
			}
			if (!r) return -1;
	// look for CR/LF/CR+LF
			if (cnt && ((buf[cnt-1] == 0x0A) || (buf[cnt-1] == 0x0D))) {
				if (buf[cnt] != 0x0A && buf[cnt] != 0x0D) {
					tchar = buf[cnt];
	//			} else {
	//				printf("cr+lf\n");
				}
				buf[cnt-1]='\n';
				buf[cnt]=0;
				return cnt;
			}
			cnt++;
			cbuf++;
		}
	}
	if (stop_req) return -1;
	buf[cnt]='\n';
	buf[cnt+1]=0;
	return cnt+1;
}

//
// CD/DVD read transfer rate
//

int qscanner::run_rd_transfer()
{
//	int     flushcnt = FLUSH_LINES;
	bool    use_readcd = 0;
	int     bsize = -1;
	int     rsize = -1;
	long	lba;
//	double	btime;
	int		err=0;
	int		br =0;
	dev->parms.read_speed_kb = (int) (speed * dev->parms.speed_mult);
	set_rw_speeds(dev);
	get_rw_speeds(dev);
	speed = (int) (dev->parms.read_speed_kb / (dev->parms.speed_mult - 0.5));
	if (dev->media.type & DISC_CD) {
		printf("Running READ transfer rate test on CD at speed %d...\n", speed);
		if (dev->capabilities & CAP_DAE) use_readcd = 1;
		rsize=15;
		//bsize=75;
		bsize = rsize*10; // 2 sec
		spd1X = 150;
	} else if (dev->media.type & DISC_DVD) {
		printf("Running READ transfer rate test on DVD at speed %d...\n", speed);
		rsize=16;
		bsize= rsize * 64; // 2M
		spd1X = 1385;
		// check if DVD is region-protected
		// dev->silent++;
		get_rpc_state(dev);
		read_disc_regions(dev);
		//dev->media.dvdcss.method = DVDCSS_METHOD_DISC;
		switch (dev->media.dvdcss.protection) {
			case 0: // unprotected DVD
				dev->media.dvdcss.method = DVDCSS_METHOD_NONE;
				break; 
			case 1: // CSS/CPPM protected DVD
			case 2: // CPRM - protected DVD
				// just to auth to be able read data
				dev->media.dvdcss.method = DVDCSS_METHOD_KEY;
				if (css_disckey(dev)) {
					printf("DVD auth failure!\n");
					return -1;
				}
				break; 
			default:
				printf("Unknown DVD protection scheme: %02X\n",dev->media.dvdcss.protection);
				dev->media.dvdcss.method = DVDCSS_METHOD_NONE;
				break;
		}
		//dev->silent--;
	} else if (dev->media.type & DISC_BD) {
		printf("Running READ transfer rate test on DVD at speed %d...\n", speed);
		rsize=16;
		bsize= rsize * 64; // 2M
		spd1X = 4500;
	} else {
		printf("Can't run read transfer rate test: unsupported media!\n");
		return -1;
	}
	if (stop_req) return 0;
	printf("Using %s command\n", use_readcd ? "READ CD" : "READ");
//	set_read_speed();
//	wait_unit_ready(dev, 6);

	spinup(dev, 4); // spin up (4 seconds)

	//	flush_cache(dev);
	seek(dev, lba_sta);
//	wait_unit_ready(dev, 6);
	msleep(300);
	gettimeofday(&s, NULL);
	printf("Reading blocks: %ld - %ld (%ld MB)\n", lba_sta, lba_end, (lba_end-lba_sta) >> 9);
	gettimeofday(&blks, NULL);
#ifdef USE_FFLUSH
	fflush(stdout);
#endif
	for (lba=lba_sta; (!stop_req) && !err && lba<lba_end; lba+=rsize)
	{
		if (lba+rsize>=lba_end) rsize = lba_end-lba;
		if (use_readcd)
			err = read_cd(dev, dev->rd_buf, lba, rsize, 0xF8);
		else
			err = read(dev, dev->rd_buf, lba, rsize);
		br += rsize;
		if (err) {
			if ((err & 0x7FF00) == 0x23A00){
				printf("Media removed! Terminating scan...\n");
			} else {
				printf("Read error! Terminating scan...\n");
			}
		}
//		printf("lba=%d +%d\n",lba,rsize);
		if (lba>lba_sta && (!(lba%bsize) || lba+rsize == lba_end || stat_req)) {
			gettimeofday(&blke, NULL);
			/*
			btime=(blke.tv_sec - blks.tv_sec) + (blke.tv_usec - blks.tv_usec)/1000000.0;
			spdKB = (bsize << 1) / btime;
			spdX  = spdKB/(float)spd1X;
			*/
			calc_cur_speed( br );
			printf("lba: %7ld    speed: %6.2f X  %6d kB/s\r", lba, spdX, spdKB);
#ifdef USE_FFLUSH
			fflush(stdout);
#endif
			gettimeofday(&blks, NULL);
			stat_req=0;
			br=0;
		}
	}
	gettimeofday(&e, NULL);
	show_avg_speed(lba);
	return 0;
}

#ifdef DISABLE_INTERNAL_WT
int qscanner::run_wr_transfer()
{
	ssize_t	n;
	pid_t	cpid;
	pipe_t	pipefd;
	char	linei[EMAXLINE+1];
	char	lines[16];
	char	linef[64];

	int  argc;
	char **argv;

	printf("Running WRITE transfer rate test for CD/DVD/BD at speed %d...\n", speed);

// creating argv...
	argc = 0;
	argv = (char**) malloc(sizeof(char*));
	argv[0] = NULL;

	//	execlp("cat", "cat", "/home/kgs/cdrecord.log.cd", NULL);
	argv = add_arg(argv, &argc, "cdrecord");

	sprintf(linei, "dev=%s", dev->device);
	argv = add_arg(argv, &argc, linei);

	argv = add_arg(argv, &argc, "-v");

	if (WT_simul)
		argv = add_arg(argv, &argc, "-dummy");

	argv = add_arg(argv, &argc, "gracetime=2");
	argv = add_arg(argv, &argc, "driveropts=burnfree");

	sprintf(lines, "speed=%d", speed);
	argv = add_arg(argv, &argc, lines);

	if (dev->media.type & DISC_DVDpRW) {
		sprintf(linef, "tsize=%dk", dev->media.capacity_total * 2);
	} else {
		sprintf(linef, "tsize=%dk", dev->media.capacity_free * 2);
	}
	argv = add_arg(argv, &argc, linef);

	argv = add_arg(argv, &argc, "-sao");
#ifndef _WIN32
	argv = add_arg(argv, &argc, "/dev/zero");
#else
	argv = add_arg(argv, &argc, "ZERO");
#endif

	printf("qscan: starting cdrecord...\n");
	printf("cdrecord args: \n");
	for(int i=0; argv[i]; i++)
		printf("%s ",argv[i]);
	printf("\n");

//	printf("DUMMY mode, not executing cdrecord\n");
//	return 0;

	if ((cpid = createChildProcess(argv, &pipefd, NULL)) == -1) {
		printf("qscan: can't create child process\n");
		return -1;
	}
	printf("qscan: child created, reading from pipe...\n");
	int wn, woffs;
	while((n = readline((int)pipefd[0], linei, EMAXLINE))>=0) {
		//	while((n = read(pipefd[0], lineo, EMAXLINE)) > 0) {
	//			sprintf(linei,"\nread #%d: %d bytes\n\0",idx, n);
	//			write(sockfd, linei, strlen(linei));
	//			idx++;
//				printf(lineo);
		woffs=0;
		while (woffs<n) {
			wn = write( fileno(stdout), linei+woffs, n-woffs);
			if (wn<0) {
				switch (errno) {
					case EAGAIN:
						break;
					default:
						woffs=n;
				}
			} else {
				woffs+=wn;
			}
#ifdef USE_FFLUSH
			fflush(stdout);
#endif
		}
	}
	printf("readline: %d\n", n);
	close(pipefd[0]);
	return 0;
}

#else // #ifdef DISABLE_INTERNAL_WT
#warning Using internal WT implementation!

int qscanner::run_wr_transfer()
{
//	int r = -1;
	int32_t bsize = 0;
	int32_t wsize = 0;
	uint32_t ubuft, ubuff, ubufp = 0;
	int32_t lba;
//	double	btime;
	int		err=0;

	if (dev->media.type & (DISC_CD)) {
		writer = new qpxwriter_cd(dev);
		wsize=25; bsize=150;
		spd1X = 176;
	} else if (dev->media.type & (DISC_DVDminus)) {
		writer = new qpxwriter_dvdminus(dev);
		wsize=32; bsize=1024;
		spd1X = 1385;
	} else if (dev->media.type & (DISC_DVDplus)) {
		writer = new qpxwriter_dvdplus(dev);
		wsize=32; bsize=1024;
		spd1X = 1385;
	} else if (dev->media.type & (DISC_DVDRAM)) {
		writer = new qpxwriter_dvdram(dev);
		wsize=32; bsize=1024;
		spd1X = 1385;
	} else if (dev->media.type & (DISC_BD) && dev->media.type != DISC_BD_ROM) {
		writer = new qpxwriter_bd(dev);
		wsize=32; bsize=1024;
		spd1X = 4500;
	} else {
		printf("Internal write transfer rate test not implemented for mounted media!");
		return -1;
	}
//	lba_end = 4096;

	get_wbuffer_capacity(dev,&ubuft,&ubuff);
	printf("Write buffer capacity: %d kB\n", ubuft >> 10);

	wait_unit_ready(dev, 6);
	printf("Writing blocks: %ld - %ld (%ld MB)\n", lba_sta, lba_end, (lba_end-lba_sta) >> 9);

	dev->parms.write_speed_kb = speed * spd1X;
	set_rw_speeds(dev);
	get_rw_speeds(dev);
	writer->setSimul(WT_simul);
	if (dev->media.type & DISC_CD) {
		printf("Running write transfer rate test on CD at speed %d...\n", dev->parms.write_speed_kb / spd1X);
		spd1X = 150;
	} else if (dev->media.type & (DISC_DVD | DISC_BD)) {
		printf("Running write transfer rate test on DVD/BD at speed %d...\n", dev->parms.write_speed_kb / spd1X);
	}

	if (writer->open_session()) {
		printf("Can't open session!\n");
		goto write_cleanup;
	}

	if (writer->send_opc()) {
		printf("OPC failed!\n");
		goto write_cleanup;
	}

	if (writer->open_track(lba_end)) {
		printf("Can't start new track!\n");
		goto write_cleanup;
	}

	printf("Starting write...\n");
	memset(dev->rd_buf,0,bufsz_rd);

	gettimeofday(&s, NULL);
	gettimeofday(&blks, NULL);
	for (lba=lba_sta; !stop_req && !err && lba<lba_end; lba+=wsize)
	{
		if (lba+wsize>=lba_end) wsize = lba_end-lba;

		get_wbuffer_capacity(dev,&ubuft,&ubuff);
	//	if (ubuff >=0 && ubuff<(wsize<<11)) {
		if ((ubuff >> 11)<((uint32_t)wsize)) {
			msleep(20);
		}
		if (wsize && writer->write_data(lba, wsize)) {
			printf("\nWrite error at sector %d (wsize=%d)\n", lba, wsize);
			stop_req = 1;
		}
		if (!(lba%bsize) || lba+wsize == lba_end || stat_req || stop_req || !wsize) {
			ubufp = (uint32_t) (ubuft ? 100*( 1.0f-ubuff/(float)ubuft) : 0);
			gettimeofday(&blke, NULL);
			/*
			btime=(blke.tv_sec - blks.tv_sec) + (blke.tv_usec - blks.tv_usec)/1000000.0;
			spdKB = (bsize << 1) / btime;
			spdX  = spdKB/(float)spd1X;
			*/
			calc_cur_speed(((lba-1)%bsize) + 1);
			printf("lba: %7d    speed: %6.2f X  %6d kB/s, written: %4ldMB/%4ldMB, Ubuf: %3d%%\r",
					lba, spdX, spdKB, (lba-lba_sta) >> 9, (lba_end-lba_sta) >> 9, ubufp);
			gettimeofday(&blks, NULL);
			stat_req=0;
#ifdef USE_FFLUSH
			fflush(stdout);
#endif
		}
	}
	printf("\n");
	gettimeofday(&e, NULL);
	show_avg_speed(lba);
	writer->close_track();

	writer->fixate();
	start_stop(dev,0);
	start_stop(dev,1);
	delete writer; writer = NULL;
	return 0;

write_cleanup:
	printf("Errors before writing! cleaning up...\n");
	flush_cache(dev,true);
	start_stop(dev,0);
	start_stop(dev,1);
	delete writer; writer = NULL;
	return 1;
}

#endif // #ifdef DISABLE_INTERNAL_WT

//
//  CD algo's
//

int qscanner::run_cd_errc()
{
    cd_errc err, err_tot, err_max;
	int  errc_data;
    long lba=lba_sta;
    long lbao;
    if (!attached) return -1;
    if (!(dev->media.type & DISC_CD)) return 1;
    lba=0;
	errc_data = plugin->errc_data();
//    seek(dev,lba);
    if (plugin->start_test(CHK_ERRC_CD,lba,speed)) {
		printf("CD ERRC test init failed!\n");
    	return 2;
    }
    printf("Running CD Error Correction test at speed %d...\n", speed);

    spd1X = 150;
    gettimeofday(&s, NULL);
    wait_unit_ready(dev, 6);
    printf("\nTesting %ld sectors: %ld - %ld\n", lba_end-lba_sta+1, lba_sta, lba_end);
    printf("         lba |        speed        |  BLER |  E11   E21   E31  |  E12   E22   E32  |  UNCR\n");
    for (; (!stop_req) && lba<lba_end; )
    {
		lbao=lba;
		gettimeofday(&blks, NULL);
		if(plugin->scan_block((void*)&err,&lba))
			{ printf("\nBlock scan error! terminating...\n"); stop_req=1; }
		gettimeofday(&blke, NULL);
		calc_cur_speed(lba-lbao);
		printf("cur : %6ld | %6.2f X %5d kB/s | %5ld | %5ld %5ld %5ld | %5ld %5ld %5ld | %5ld\r", lba, spdX, spdKB,
				err.bler,
				err.e11,err.e21,err.e31,
				err.e12,err.e22,err.e32,
				err.uncr);
		err_tot+=err;
		err_max.EMAX(err);
#ifdef USE_FFLUSH
		fflush(stdout);
#endif
    }
    plugin->end_test();
    gettimeofday(&e, NULL);
    show_avg_speed(lba);
    printf("\n%ld sectors tested: %ld - %ld\n", lba-lba_sta, lba_sta, lba-1);
    printf("Test summary:\n");
    printf("       BLER |  E11   E21   E31  |  E12   E22   E32  |  UNCR\n");
    printf("tot : %5ld | %5ld %5ld %5ld | %5ld %5ld %5ld | %5ld\n",
		err_tot.bler,
		err_tot.e11,err_tot.e21,err_tot.e31,
		err_tot.e21,err_tot.e22,err_tot.e32,
		err_tot.uncr);
    printf("max : %5ld | %5ld %5ld %5ld | %5ld %5ld %5ld | %5ld\n",
		err_max.bler,
		err_max.e11,err_max.e21,err_max.e31,
		err_max.e21,err_max.e22,err_max.e32,
		err_max.uncr);
    printf("avg : %5.2f | %5.2f %5.2f %5.2f | %5.2f %5.2f %5.2f | %5.2f\n",
		err_tot.bler/(float)lba,
		err_tot.e11/(float)lba,err_tot.e21/(float)lba,err_tot.e31/(float)lba,
		err_tot.e21/(float)lba,err_tot.e22/(float)lba,err_tot.e32/(float)lba,
		err_tot.uncr/(float)lba);
#ifdef USE_FFLUSH
	fflush(stdout);
#endif
	return 0;
}

int qscanner::run_cd_jb()
{
    cdvd_jb jb, jb_min, jb_max;
    long lba=lba_sta;
    long lbao;
    if (!attached) return -1;
    if (!(dev->media.type & DISC_CD)) return 1;
//    seek(dev,lba);
    if (plugin->start_test(CHK_JB_CD,lba,speed)) {
		printf("CD Jitter/Asymm test init failed!\n");
    	return 2;
    }
    printf("Running CD Jitter/Asymm test at speed %d...\n", speed);

    spd1X = 150;
    gettimeofday(&s, NULL);
    wait_unit_ready(dev, 6);
    printf("\nTesting %ld sectors: %ld - %ld\n", lba_end-lba_sta+1, lba_sta, lba_end);
    printf("         lba |        speed        | Jitter |  Asymm\n");
    for (; (!stop_req) && lba<lba_end; )
    {
		lbao=lba;
		gettimeofday(&blks, NULL);
		if(plugin->scan_block((void*)&jb,&lba))
			{ printf("\nBlock scan error! terminating...\n"); stop_req=1; }
		gettimeofday(&blke, NULL);
		calc_cur_speed(lba-lbao);
		printf("cur : %6ld | %6.2f X %5d kB/s | %6.2f | %6.2f\r", lba, spdX, spdKB, jb.jitter/1000.0, jb.asymm/10.0);
		jb_min.EMIN(jb);
		jb_max.EMAX(jb);
#ifdef USE_FFLUSH
		fflush(stdout);
#endif
	}
    plugin->end_test();
    gettimeofday(&e, NULL);
    show_avg_speed(lba);
    printf("\n%ld sectors tested: %ld - %ld\n", lba-lba_sta, lba_sta, lba-1);
    printf("Test summary:\n");
    printf("               Jitter |  Asymm\n");
    printf("         min : %6.2f | %6.2f\n", jb_min.jitter/100.0, jb_min.asymm/10.0);
    printf("         max : %6.2f | %6.2f\n", jb_max.jitter/100.0, jb_max.asymm/10.0);
//    printf("         avg : %6.2f | %6.2f", lba, jb_max.jitter/100.0, jb_max.asymm/10.0);
#ifdef USE_FFLUSH
	fflush(stdout);
#endif
	return 0;
}

int qscanner::run_cd_ta()
{
    if (!attached) return -1;
    if (!(dev->media.type & DISC_CD)) return 1;
	printf("Running CD Time Analyser test...\n");

	printf("Can't start test: not implemented!\n");
    return -1;
}

//
//  DVD algo's
//

int qscanner::run_dvd_errc()
{
    dvd_errc err, err_tot, err_max;
    int  errc_data;
    long lba=lba_sta;
    long lbas;
    long lbao;
    long pi8=0, pi8_max=0;
    long po8=0, po8_max=0;
    if (!attached) return -1;
    if (!(dev->media.type & DISC_DVD)) return 1;
//    lba=0; slba=0;
	errc_data = plugin->errc_data();
//    seek(dev,lba);
    if (plugin->start_test(CHK_ERRC_DVD,lba,speed)) {
		printf("DVD ERRC test init failed!\n");
    	return 2;
    }
    printf("Running DVD Error Correction test at speed %d...\n", speed);

    spd1X = 1385;
    gettimeofday(&s, NULL);
    wait_unit_ready(dev, 6);
    lbas = lba;
    printf("\nTesting %ld sectors: %ld - %ld\n", lba_end-lba_sta+1, lba_sta, lba_end);
    printf("          lba |        speed        |  PIE   PI8   PIF  |  POE   PO8   POF  |  UNCR\n");
    for (; (!stop_req) && lba<lba_end; )
    {
		lbao=lba;
		gettimeofday(&blks, NULL);
		if(plugin->scan_block((void*)&err,&lba))
			{ printf("\nBlock scan error! terminating...\n"); stop_req=1; }
		err_tot+=err;
		err_max.EMAX(err);
		pi8+=err.pie;
		po8+=err.poe;
		gettimeofday(&blke, NULL);
		calc_cur_speed(lba-lbao);
//		if (!(lba & 0x7F)) {
		if ((lba-lbas) >= 0x80) {
			if (pi8_max<pi8) pi8_max=pi8;
			if (po8_max<po8) po8_max=po8;
	    		printf("cur : %7ld | %6.2f X %5d kB/s | %5ld %5ld %5ld | %5ld %5ld %5ld | %5ld\r",
					lba, spdX, spdKB,
					err.pie,pi8,err.pif,
					err.poe,po8,err.pof,
					err.uncr);
			pi8=0; po8=0;
			lbas = lba;
		} else {
			printf("cur : %7ld | %6.2f X %5d kB/s | %5ld %5ld %5ld | %5ld %5ld %5ld | %5ld\r",
					lba, spdX, spdKB,
					err.pie,(long)-1,err.pif,
					err.poe,(long)-1,err.pof,
					err.uncr);
		}
#ifdef USE_FFLUSH
		fflush(stdout);
#endif
	}
    plugin->end_test();
    gettimeofday(&e, NULL);
    show_avg_speed(lba);
    printf("\n%ld sectors tested: %ld - %ld\n", lba-lba_sta, lba_sta, lba-1);
    printf("Test summary:\n");
    printf("       PIE   PI8   PIF  |  POE   PO8   POF  |  UNCR\n");
    printf("tot : %5ld %5ld %5ld | %5ld %5ld %5ld | %5ld\n",
		err_tot.pie,err_tot.pie,err_tot.pif,
		err_tot.poe,err_tot.poe,err_tot.pof,
		err_tot.uncr);
    printf("max : %5ld %5ld %5ld | %5ld %5ld %5ld | %5ld\n",
		err_max.pie,pi8_max,err_max.pif,
		err_max.poe,po8_max,err_max.pof,
		err_max.uncr);
    printf("avg : %5.2f %5.2f %5.2f | %5.2f %5.2f %5.2f | %5.2f\n",
		err_tot.pie/(float)(lba>>4),err_tot.pie/(float)(lba>>7),err_tot.pif/(float)(lba>>4),
		err_tot.poe/(float)(lba>>4),err_tot.poe/(float)(lba>>7),err_tot.pof/(float)(lba>>4),
		err_tot.uncr/(float)(lba>>4));
#ifdef USE_FFLUSH
	fflush(stdout);
#endif
	return 0;
}

int qscanner::run_dvd_jb()
{
    cdvd_jb jb, jb_min, jb_max;
    long lba=lba_sta;
    long lbao;
    if (!attached) return -1;
    if (!(dev->media.type & DISC_DVD)) return 1;
//    seek(dev,lba);
    if (plugin->start_test(CHK_JB_DVD,lba,speed)) {
		printf("DVD Jitter/Asymm test init failed!\n");
    	return 2;
    }
    printf("Running DVD Jitter/Asymm test at speed %d...\n", speed);

    spd1X = 1385;
    gettimeofday(&s, NULL);
    wait_unit_ready(dev, 6);
    printf("\nTesting %ld sectors: %ld - %ld\n", lba_end-lba_sta+1, lba_sta, lba_end);
    printf("         lba |        speed        | Jitter |  Asymm\n");
    for (; (!stop_req) && lba<lba_end; )
    {
		lbao=lba;
		gettimeofday(&blks, NULL);
		if(plugin->scan_block((void*)&jb,&lba))
			{ printf("\nBlock scan error! terminating...\n"); stop_req=1; }
		gettimeofday(&blke, NULL);
		calc_cur_speed(lba-lbao);
		printf("cur : %6ld | %6.2f X %5d kB/s | %6.2f | %6.2f\r", lba, spdX, spdKB, jb.jitter/1000.0, jb.asymm/10.0);
		jb_min.EMIN(jb);
		jb_max.EMAX(jb);
#ifdef USE_FFLUSH
		fflush(stdout);
#endif
	}
    plugin->end_test();
    gettimeofday(&e, NULL);
    show_avg_speed(lba);
    printf("\n%ld sectors tested: %ld - %ld\n", lba-lba_sta, lba_sta, lba-1);
    printf("Test summary:\n");
    printf("               Jitter |  Asymm\n");
    printf("         min : %6.2f | %6.2f\n", jb_min.jitter/100.0, jb_min.asymm/10.0);
    printf("         max : %6.2f | %6.2f\n", jb_max.jitter/100.0, jb_max.asymm/10.0);
//    printf("         avg : %6.2f | %6.2f", lba, jb_max.jitter/100.0, jb_max.asymm/10.0);
#ifdef USE_FFLUSH
	fflush(stdout);
#endif
	return 0;
}

#define MAX_RETRY 16
int qscanner::run_fete()
{
    struct cdvd_ft ft, ft_max, ft_min;
    long lba=lba_sta;
    long lbao;
	int retry = MAX_RETRY;
    if (!attached) return -1;
    if (dev->media.type & DISC_CD) {
		printf("Running FE/TE test for CD at speed %d...\n", speed);
//		if (dev->capabilities & CAP_DAE) use_readcd = 1;
//		rsize=15; bsize=75;
		spd1X = 150;
    } else if (dev->media.type & DISC_DVD) {
		printf("Running FE/TE test for DVD at speed %d...\n", speed);
		spd1X = 1385;
    } else if (dev->media.type & DISC_BD) {
		printf("Running FE/TE test for BD at speed %d...\n", speed);
		spd1X = 4500;
    } else {
		printf("Can't run FE/TE test: unsupported media!\n");
		return 1;
    }
    wait_unit_ready(dev, 6);
    if (plugin->start_test(CHK_FETE,lba,speed)) {
		printf("Scan init failed!\n");
    	return 2;
    }
    gettimeofday(&s, NULL);
//    wait_unit_ready(dev, 6);
    printf("\nTesting %ld sectors: %ld - %ld\n", lba_end-lba_sta+1, lba_sta, lba_end);
    printf("         lba |        speed        |  FE  |  TE\n");
	gettimeofday(&blks, NULL);
    for (; (!stop_req) && lba<lba_end; )
    {
		lbao=lba;
block_retry:
		if(plugin->scan_block((void*)&ft,&lba))
			{ printf("\nBlock scan error! terminating...\n"); stop_req=1; }
		if(lba<0) {
			if (retry--)
				goto block_retry;
			printf("\nDrive returned negative LBA %d times! terminating...\n", MAX_RETRY);
			stop_req=1;
		}
		if(lba == lbao) {
			if (retry--)
				goto block_retry;
			printf("\nDrive returned same LBA %d times! terminating...\n", MAX_RETRY);
			stop_req=1;
		}
		retry = MAX_RETRY;
		gettimeofday(&blke, NULL);
		ft_max.EMAX(ft);
		calc_cur_speed(lba-lbao);
//		 show current data
		printf("cur : %6ld | %6.2f X %5d kB/s | %4d | %4d\n", lba, spdX, spdKB, ft.fe, ft.te);
		blks.tv_sec  = blke.tv_sec;
		blks.tv_usec = blke.tv_usec;
#ifdef USE_FFLUSH
		fflush(stdout);
#endif
	}
    plugin->end_test();
    gettimeofday(&e, NULL);
    show_avg_speed(lba);
    printf("\n%ld sectors tested: %ld - %ld\n", lba-lba_sta, lba_sta, lba-1);
    printf("Test summary:\n");
    printf("       FE  |  TE\n");
	printf("max : %4d | %4d\n", ft_max.fe, ft_max.te);
#ifdef USE_FFLUSH
	fflush(stdout);
#endif
	return 0;
}

int qscanner::run_dvd_ta()
{
	struct cdvd_ta ta;
	long   lba;
    if (!attached) return -1;
    if (!(dev->media.type & DISC_DVD)) return 1;
	printf("Running DVD Time Analyser test...\n");
    if (plugin->start_test(CHK_TA,lba,speed)) {
		printf("Scan init failed!\n");
    	return 2;
    }

	for (int i=0; i<3*dev->media.layers; i++) {
		ta.pass = i;
		plugin->scan_block(&ta, &lba);
		printf("   idx    pits   lands\n");
		for (int idx=0; idx<TA_HIST_SIZE; idx++) {
			printf("TA %3d  %6d  %6d\r", idx, ta.pit[idx], ta.land[idx]);
		}
	}

	// not implemented
    return -1;
}

//
//  BD algo's
//

int qscanner::run_bd_errc()
{
    bd_errc err, err_tot, err_max;
    int  errc_data;
    long lba=lba_sta;
    long lbas;
    long lbao;
    if (!attached) return -1;
    if (!(dev->media.type & DISC_BD)) return 1;
//    lba=0; slba=0;
	errc_data = plugin->errc_data();
//    seek(dev,lba);
    if (plugin->start_test(CHK_ERRC_BD,lba,speed)) {
		printf("BD ERRC test init failed!\n");
    	return 2;
    }
    printf("Running BD Error Correction test at speed %d...\n", speed);

    spd1X = 4500;
    gettimeofday(&s, NULL);
    wait_unit_ready(dev, 6);
    lbas = lba;
    printf("\nTesting %ld sectors: %ld - %ld\n", lba_end-lba_sta+1, lba_sta, lba_end);
    printf("          lba |        speed        |  LDC   BIS  |  UNCR\n");
    for (; (!stop_req) && lba<lba_end; )
    {
		lbao=lba;
		gettimeofday(&blks, NULL);
		if(plugin->scan_block((void*)&err,&lba))
			{ printf("\nBlock scan error! terminating...\n"); stop_req=1; }
		err_tot+=err;
		err_max.EMAX(err);
		gettimeofday(&blke, NULL);
		calc_cur_speed(lba-lbao);
//		if (!(lba & 0x7F)) {
		printf("cur : %7ld | %6.2f X %5d kB/s | %5ld %5ld | %5ld\r",
				lba, spdX, spdKB,
				err.ldc,err.bis,
				err.uncr);
#ifdef USE_FFLUSH
		fflush(stdout);
#endif
	}
    plugin->end_test();
    gettimeofday(&e, NULL);
    show_avg_speed(lba);
    printf("\n%ld sectors tested: %ld - %ld\n", lba-lba_sta, lba_sta, lba-1);
    printf("Test summary:\n");
    printf("       LDC   BIS  |  UNCR\n");
    printf("tot : %5ld %5ld | %5ld\n",
		err_tot.ldc,err_tot.bis,
		err_tot.uncr);
    printf("max : %5ld %5ld | %5ld\n",
		err_max.ldc,err_max.bis,
		err_max.uncr);
    printf("avg : %5.2f %5.2f | %5.2f\n",
		err_tot.ldc/(float)(lba>>4),err_tot.bis/(float)(lba>>4),
		err_tot.uncr/(float)(lba>>4));
#ifdef USE_FFLUSH
	fflush(stdout);
#endif
	return 0;
}

