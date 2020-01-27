/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <threads.h>
#include <common_functions.h>
#include "child.h"

int    clients;
Mutex* cmutex;

child_t children[CLIENTS_MAX+1];

int child_find_unused()
{
//	printf("Child find unused...");
	int idx = -1;
	int i=0;
	cmutex->lock();
	while (idx<0 && i<(CLIENTS_MAX+1)) {
		if (!children[i].arg.used) {
			children[i].arg.used=1;
			idx=i;
		}
		i++;
	}
	cmutex->unlock();
	return idx;
};

void child_list_clear()
{
	for (int i=0; i<(CLIENTS_MAX+1); i++)
		children[i].arg.used=0;
}

const char helpstr[] = "QSCAND: valid commands:\n\
close           close connection\n\
list|scanbus    list available devices\n\
dinfo           show device info (have to set device first)\n\
minfo           show media info (have to set device first)\n\
run             run test (have to set device and test type first)\n\
\n\
get             get current parameters\n\
set <PAR>=<VAL> set parameter. PAR can be:\n\
                dev   - set device, VAL should be like '/dev/hdX' or '/dev/sdX'\n\
                test  - set test type, VAL should be one of following:\n\
                        rt   - read transfer rate\n\
                        wt   - write transfer rate\n\
                        errc - error correction test\n\
                        jb   - jitter/asymmetry test\n\
                        ft   - focus/tracking test\n\
                        ta   - time analyser\n\
                speed - set test speed, VAL should be integer\n\
                simul - set test speed, VAL should be 0 or 1 (default: 1)\n\
";

const int helpstr_sz = sizeof(helpstr);

enum qscan_mode {
	none    = 0,
	scanbus = 1,
	scan,
	dinfo,
	minfo,
	help
};

char tchar = -1;

enum fdtype_t {
	FD_PIPE   = 1,
	FD_SOCKET = 2
};

//int readline(int fd, char *buf, int maxlen, fdtype_t fdtype = FD_PIPE)
int readline(int fd, char *buf, int maxlen, fdtype_t fdtype)
{
	int cnt=0;
	char *cbuf=buf;
	int r;
	int sret;
	fd_set rd_set;
	timeval tv;

	FD_ZERO(&rd_set);

//	printf("tchar=%02X\n", tchar);
	if (tchar>=0) {
		cbuf[0] = tchar;
		cnt++;
		cbuf++;
		tchar = -1;
	}
	while ( !term && (cnt<(maxlen-1))) {
		FD_SET(fd, &rd_set);
		tv.tv_sec  = 1;
		tv.tv_usec = 0;
#ifdef _WIN32
		if (fdtype == FD_SOCKET) {
#endif
			sret = select(fd+1, &rd_set, NULL, NULL, &tv);
#ifdef _WIN32
			errno = GetLastError();
		} else {
			sret  = 1;
			errno = 0;
		}
#endif
//		printf("select(%d): %s\n", fd, strerror(errno));
		if (sret < 0) {
#ifndef _WIN32
			if (debug) {
				if (debug && !daemonized)
					printf("readline() %d select: %s\n", fd, strerror(errno));
			}
#endif
			if (errno == EINTR) continue;
			return -1;
		}
		else if (sret > 0 && FD_ISSET(fd, &rd_set)) {
#ifdef _WIN32
			if (fdtype == FD_SOCKET)
				r = recv(fd, cbuf, 1, 0);
			else
#endif
				r = read(fd, cbuf, 1);
			if (r<0) {
				if (debug && !daemonized)
					printf("read = %d, %d, %s\n", r, errno, strerror(errno));
				switch (errno) {
					case EAGAIN:
					//	printf("EAGAIN\n");
						continue;
					case EINTR: 
					//	printf("EINTR\n");
						continue;
					default:
						return -1;
				}
			}
			if (!r) return -1;
	// look for CR/LF/CR+LF
			if (fdtype == FD_SOCKET) {
				if (buf[cnt] == 0x0A || buf[cnt] == 0x0D) goto readline_end;
			} else if (cnt && (buf[cnt-1] == 0x0A || buf[cnt-1] == 0x0D)) {
				if (buf[cnt] != 0x0A && buf[cnt] != 0x0D) tchar = buf[cnt];
				buf[cnt-1]='\n';
				buf[cnt]=0;
				return cnt;
			}
			cnt++;
			cbuf++;
		}
	}
	if (term) return -1;
readline_end:
	buf[cnt]='\n';
	buf[cnt+1]=0;
	return cnt+1;
}

void child_proc(child_arg_t *arg)
{
	ssize_t	n;
	char	speeds[16];
	char	linei [MAXLINE+1];
	char	lineo [MAXLINE+IDENT_LEN+1];
	pipe_t	pipefd;
	pid_t	cpid;
	qscan_mode mode = none;
	char	device[128] = "\0";
	char	test[8]     = "\0";
	int		speed       = -1;
	bool	WT_simul	 = 1;
#ifdef _WIN32
	send(arg->connfd, IDENTV, IDENTV_LEN, 0);
#else
	write(arg->connfd, IDENTV, IDENTV_LEN);
#endif
	for (;;) {
		mode = none;
		if (term) return;
#ifdef _WIN32
		send(arg->connfd, PROMPT, PROMPT_LEN, 0);
#else
		write(arg->connfd, PROMPT, PROMPT_LEN);
#endif
		if ((n = readline(arg->connfd, linei, MAXLINE, FD_SOCKET)) < 0)
		{
		//if ((n = fgets(line, MAXLINE, arg->connfd)) == EOF) {
			//printf("Client disconnected\n");
			return;
		}
		if (n<=1) continue;
		linei[n]=0;
#ifdef _WIN32
		send(arg->connfd, "\n", 1, 0);
#else
		write(arg->connfd, "\n", 1);
#endif
		if (debug && !daemonized)
			printf("%s:%d : command: %s",
					inet_ntoa(arg->cliaddr.sin_addr),
					ntohs(arg->cliaddr.sin_port),
					linei);
		if (!strcmp(linei, "help\n")) {
#ifdef _WIN32
			send(arg->connfd, helpstr, helpstr_sz, 0);
#else
			write(arg->connfd, helpstr, helpstr_sz);
#endif
			continue;
		} else if (!strcmp(linei, "close\n")) {
//			shutdown(arg->connfd, SHUT_RDWR);
			return;
//		} else if (!strcmp(linei, "qhelp\n")) {
//			mode = help;
		} else if (!strcmp(linei, "list\n") || !strcmp(linei, "scanbus\n")) {
			mode = scanbus;
		} else if (!strcmp(linei, "dinfo\n")) {
			mode = dinfo;
		} else if (!strcmp(linei, "minfo\n")) {
			mode = minfo;
		} else if (!strcmp(linei, "run\n")) {
			mode = scan;
		} else if (!strcmp(linei, "get\n")) {
			sprintf(lineo, "current parameters:\ndevice: '%s'\ntest  : '%s'\nspeed : %d\nsimul : %s\n",
					device,
					test,
					speed,
					WT_simul ? "on" : "off");
#ifdef _WIN32
			send(arg->connfd, lineo, strlen(lineo), 0);
#else
			write(arg->connfd, lineo, strlen(lineo));
#endif
			continue;
		} else if (!strncmp(linei, "set", 3)) {
			char *linet = linei+4;
			size_t  len;
			if ((strlen(linei) <=4) || linei[3]!=' ') {
				sprintf(lineo, "QSCAND: set: needs parameter!\n");
#ifdef _WIN32
				send(arg->connfd, lineo, strlen(lineo), 0);
#else
				write(arg->connfd, lineo, strlen(lineo));
#endif
				continue;
			}
			if (!strncmp(linet, "dev=", 4)) {
				linet+=4;
				len = strlen(linet);
				if (len<sizeof(device)) {
					strncpy(device, linet, len-1);
				} else {
					sprintf(lineo, "QSCAND: too long device name!\n");
#ifdef _WIN32
					send(arg->connfd, lineo, strlen(lineo), 0);
#else
					write(arg->connfd, lineo, strlen(lineo));
#endif
				}
			} else if (!strncmp(linet, "test=", 5)) {
				linet+=5;
				len = strlen(linet);
				if (len<sizeof(test)) {
					strncpy(test, linet, len-1);
				} else {
					sprintf(lineo, "QSCAND: too long test name!\n");
#ifdef _WIN32
					send(arg->connfd, lineo, strlen(lineo), 0);
#else
					write(arg->connfd, lineo, strlen(lineo));
#endif
				}
			} else if (!strncmp(linet, "speed=", 6)) {
				linet+=6;
				speed = atol(linet);
			} else if (!strncmp(linet, "simul=", 6)) {
				linet+=6;
				WT_simul = atol(linet) ? 1 : 0;
			} else {
				sprintf(lineo, "QSCAND: set: invalid parameter!\n");
#ifdef _WIN32
				send(arg->connfd, lineo, strlen(lineo), 0);
#else
				write(arg->connfd, lineo, strlen(lineo));
#endif
			}
			continue;
		} else {
		//	sprintf(lineo, "str len: %d, cmd len: %d. '%d'\n", n, strlen(linei), linei);
			//sprintf(lineo, "str len: %d, cmd len: %d, last %02x %02x\n", n, strlen(linei), linei[n-2], linei[n-1]);
		//	write(arg->connfd, lineo, strlen(lineo));
			sprintf(lineo, "QSCAND: invalid command. try \"help\"\n");
#ifdef _WIN32
			send(arg->connfd, lineo, strlen(lineo), 0);
#else
			write(arg->connfd, lineo, strlen(lineo));
#endif
		}
		
		if (mode != none) {
			int    argc = 0;
			char **argv = (char**) malloc(sizeof(char*));
			argv[0] = NULL;
			if ( ((mode == scan) || (mode == dinfo) || (mode == minfo))  && !strlen(device)) {
#ifdef DAEMON_EN
				if (daemonized)
					syslog(LOG_WARNING, "QSCAND: No device specified!\n");
				else
#endif
					printf("QSCAND: No device specified!\n");
				_exit(0);
			}
			if ( (mode == scan) && !strlen(test)) {
#ifdef DAEMON_EN
				if (daemonized)
					syslog(LOG_WARNING, "QSCAND: No test specified!\n");
				else
#endif
					printf("QSCAND: No test specified!\n");
				_exit(0);
			}
			argv = add_arg(argv, &argc, "qscan");
			switch (mode) {
				case scanbus:
					argv = add_arg(argv, &argc, "-l");
					break;
				case scan:
					argv = add_arg(argv, &argc, "-d");
					argv = add_arg(argv, &argc, device);

					argv = add_arg(argv, &argc, "-t");
					argv = add_arg(argv, &argc, test);

					sprintf(speeds,"%d", speed);
					argv = add_arg(argv, &argc, "-s");
					argv = add_arg(argv, &argc, speeds);

					if (!strcmp(test, "wt") && !WT_simul)
						argv = add_arg(argv, &argc, "-W");
					break;
				case dinfo:
					argv = add_arg(argv, &argc, "-d");
					argv = add_arg(argv, &argc, device);
					argv = add_arg(argv, &argc, "-Ip");
					break;
				case minfo:
					argv = add_arg(argv, &argc, "-d");
					argv = add_arg(argv, &argc, device);
					argv = add_arg(argv, &argc, "-m");
					break;
				case help:
					argv = add_arg(argv, &argc, "-h");
					break;
				default:
					if (debug && !daemonized)
						printf("unknown mode: %d\n", mode);
					return;
			}

			if ((cpid = createChildProcess(argv, &pipefd, NULL)) == -1) {
#ifdef DAEMON_EN
				if (daemonized)
					syslog(LOG_ERR, "Can't start child!\n");
				else
#endif
					printf("QSCAND: Can't start child!\n");
				return;
			} else {
				// parent. copy messages from pipe to socket 
				close(pipefd[1]); // unused write end

				sprintf(lineo, "QSCAND: child created, reading from pipe...\n");
#ifdef _WIN32
				send(arg->connfd, lineo, strlen(lineo), 0);
#else
				write(arg->connfd, lineo, strlen(lineo));
#endif
				int wn, woffs;
				while((n = readline((int)pipefd[0], lineo, MAXLINE, FD_PIPE))>=0) {
			//	while((n = read(pipefd[0], lineo, MAXLINE)) > 0) {
		//			sprintf(linei,"\nread #%d: %d bytes\n\0",idx, n);
		//			write(arg->connfd, linei, strlen(linei));
		//			idx++;
//					printf(lineo);
					woffs=0;
					while (woffs<n) {
#ifdef _WIN32
						wn = send(arg->connfd, lineo+woffs, n-woffs, 0);
#else
						wn = write(arg->connfd, lineo+woffs, n-woffs);
#endif
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
					}
				}
#ifdef DAEMON_EN
				if (daemonized)
					syslog(LOG_WARNING, "Pipe end!\n");
				else
#endif
					printf("QSCAND: Pipe end!\n");
				close(pipefd[0]);
			}
		} // if (mode != none)
	}
}

void *child_thread(void *argp)
{
	child_arg_t *arg = (child_arg_t*)argp;

//	childl++;
//	pid=getpid();
//	close(listenfd);
#ifdef DAEMON_EN
	if (daemonized)
		syslog(LOG_INFO, "Client connected: %s:%d\n",
			inet_ntoa(arg->cliaddr.sin_addr),
			ntohs(arg->cliaddr.sin_port));
	else
#endif
		printf("%d: Client connected: %s:%d\n", pid,
			inet_ntoa(arg->cliaddr.sin_addr),
			ntohs(arg->cliaddr.sin_port));

	/* serve connection */
	child_proc(arg);
#ifdef DAEMON_EN
	if (daemonized)
		syslog(LOG_INFO, "Client disconnected: %s:%d\n",
			inet_ntoa(arg->cliaddr.sin_addr),
			ntohs(arg->cliaddr.sin_port));
	else
#endif
		printf("%d: Client disconnected: %s:%d\n", pid,
			inet_ntoa(arg->cliaddr.sin_addr),
			ntohs(arg->cliaddr.sin_port));
	if (debug && !daemonized)
		printf("%d: Closing client socket...\n", pid);
	shutdown(arg->connfd, SHUT_RDWR);
	close(arg->connfd);

	cmutex->lock();
	clients--;
	arg->used = 0;
	cmutex->unlock();

	return 0;
//	thread_exit(0);
}

