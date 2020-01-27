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
#include <getopt.h>

#include <common_functions.h>
#include <threads.h>
#include <child.h>
#include "qscand.h"
#ifndef _WIN32
#include <signal.h>
#endif

#include <fcntl.h>

bool term;
int childl;
pid_t pid;
// pid_t cpids[CLIENTS_MAX];
bool daemonized=0;
bool debug=0;

#if defined (__unix) || defined (__unix__)
void sigint_handler (int)
#elif defined (_WIN32)
BOOL WINAPI sigint_handler (DWORD)
#endif
{
	if (!childl) {
		if (debug && !daemonized && clients>0)
			printf("%d: SIGINT. %d client(s) left. Terminating connections...\n", pid, clients);
//		while (clients) msleep((1 << 8));
//		if (debug && !daemonized)
//			printf("%d: SIGINT. Terminating listener...\n", pid);
	} else if (childl == 1) {
		if (debug && !daemonized)
			printf("%d: SIGINT. Terminating client connection...\n", pid);
	}
	term=1;
#if defined (_WIN32)
	return 1;
#endif
};

#ifdef DAEMON_EN
int daemonize()
{
	pid_t pid;
	switch (pid = fork()) {
		case -1:
			return -1;
		case 0:
			openlog("qscand", LOG_PID, LOG_DAEMON);
			close(0);
			close(1);
			close(2);
			daemonized = 1;
			break;
		default:
			exit(0);
	}
	if (setsid() == -1)
		return -1;
	return 0;
}
#endif

static struct option long_options[] = {
	{"help",      0, NULL, 'h'},
#ifdef DAEMON_EN
	{"nodaemon",  0, NULL, 'n'},
#endif
	{"debug",     0, NULL, 'd'},
	{"iface",     1, NULL, 'i'},
	{"port",      1, NULL, 'p'},
#ifdef AUTOPORT_ENABLE
	{"portauto",  0, NULL, 'a'},
#endif
	{0,0,0,0}
};
	
int child_create(int idx)
{
	int r;
	if ((r = thread_create(&children[idx].tid, NULL, &child_thread, (void*)&children[idx].arg))) {
#ifdef DAEMON_EN
		syslog(LOG_ERR, "Error creating thread!\n");
#else
		printf("Error creating thread!\n");
#endif
		return r;
		close(children[idx].arg.connfd);
	}
	return 0;
}

int main (int argc, char **argv)
{
//	pid_t cpid;
	char *iface=NULL;
	uint32_t ip;
	uint16_t port;
	int listenfd;
	struct sockaddr_in srvaddr;
	socklen_t srvaddr_len = sizeof(srvaddr);
	int      cli_idx;
#ifdef _WIN32
	WSADATA WSAdata;
#endif

	bool portauto=0;
	bool nodaemon=0;
	daemonized = 0;

	struct timeval tv;
	fd_set rd_set;
	FD_ZERO(&rd_set);

	term=0;
	clients=0;
	cli_idx=-1;
	pid=getpid();
//	for (int i=0; i<CLIENTS_MAX; i++) cpids[i]=0;

	ip = htonl(INADDR_ANY);
	port = 0;
	while (1) {
		char c = getopt_long(argc, argv, "hndai:p:", long_options, NULL);
		if (c == -1)
			break;
		switch(c) {
			case 'h':
				printf("Usage: qscand <options>\n");
				printf("\n");
				printf("-h --help           you are reading this:)\n");
#ifdef DAEMON_EN
				printf("-n --nodaemon       don't daemonize\n");
#endif
				printf("-i --iface <ip>     listen on interface with selected address\n");
				printf("-p --port <port>    listen selected port\n");
#ifdef AUTOPORT_ENABLE
				printf("-a --portauto       autoselect port listen to\n");
#endif
				printf("-d --debug          debug\n");
				printf("\n");	
				exit(0);
			case 'n':
				nodaemon=1;
#ifndef DAEMON_EN
				printf("daemon mode not available, option '-n' ignored!\n");
#endif
				break;
			case 'd':
				debug=1;
				break;
			case 'i':
				iface = optarg; 
				break;
			case 'p':
				{
					unsigned long lp = strtol(optarg, NULL, 10);
					if (errno == ERANGE || lp > 65535) {
						printf("port number out of range!\n");
					} else {
						port = lp;
					}
				}
				break;
#ifdef AUTOPORT_ENABLE
			case 'a':
				portauto=1;
				break;
#endif
			default:
				printf("Invalid option: -%c\n", c);
				break;
		}
	}
	if (!port && !portauto)
		port = DEFPORT;
#ifdef _WIN32
	if ((errno = WSAStartup(MAKEWORD(1,1), &WSAdata))) {
		printf("Can't initialize winsock: [%d] %s\n", errno, strerror(errno));
		return -1;
	}
#endif
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		printf("%d: Can't create socket. Terminating\n", pid);
		return -1;
	}
//	fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL) | O_NONBLOCK);

	//listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	memset (&srvaddr, 0, sizeof(srvaddr));

#if defined (__unix) || defined (__unix__)
	signal(SIGINT, &sigint_handler);
//	signal(SIGCHLD, &sigchld_handler);
#elif defined (_WIN32)
	SetConsoleCtrlHandler(&sigint_handler, 1);
#endif

	srvaddr.sin_family = AF_INET;
	if (!iface || !inet_aton(iface, &srvaddr.sin_addr)) {
		srvaddr.sin_addr.s_addr = ip;
	}
	srvaddr.sin_port = htons(port);

	int on=1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));	

	if (bind( listenfd, (struct sockaddr*) &srvaddr, srvaddr_len)) {
#ifdef _WIN32
		errno = GetLastError();
#endif
//	if (errno) {
		if (errno != EADDRINUSE || !portauto) {
			printf("%d: Can'n bind() %s:%d: [%d] %s\n", pid, inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port), errno, strerror(errno));
			return -1;
		} else {
			printf("%d: Can'n bind() %s:%d, trying to autoassig port...\n", pid, inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port));
			srvaddr.sin_port = 0;
			if (bind( listenfd, (struct sockaddr*) &srvaddr, srvaddr_len)) {
				printf("%d: Can'n bind(): %s\n", pid, strerror(errno));
				return -1;
			}
		}
	}

	if (listen(listenfd, LISTENQ)) {
#ifdef _WIN32
		errno = GetLastError();
#endif
		printf("%d: Can'n listen() %s:%d: [%d] %s\n", pid, inet_ntoa(srvaddr.sin_addr), ntohs(srvaddr.sin_port), errno, strerror(errno));
		return -1;
	}
//	printf("listenfd: %d\n", listenfd);
	getsockname(listenfd, (struct sockaddr*) &srvaddr, &srvaddr_len);

#ifdef DAEMON_EN
	if (!nodaemon) daemonize();
	pid = getpid();

	if (daemonized) {
		syslog(LOG_INFO, "Listening %s:%d\n", inet_ntoa(srvaddr.sin_addr), port);
		syslog(LOG_DEBUG, "daemonized\n");
	} else
#endif
		printf("%d: Listening %s:%d\n", pid, inet_ntoa(srvaddr.sin_addr), port);

	cmutex = new Mutex();
	child_list_clear();

	for (; !term ;) {
	//	printf("listen loop...\n");
		socklen_t cliaddr_len = sizeof(children[cli_idx].arg.cliaddr);
		FD_SET(listenfd, &rd_set);
		tv.tv_sec =1;
		tv.tv_usec=0;

		cli_idx = child_find_unused();
		if (cli_idx<0) continue;

		children[cli_idx].arg.connfd=0;
	//	printf("select...\n");
		int sret = select(listenfd+1, &rd_set, NULL, NULL, &tv);
	//	printf("Sret = %d, err: %s\n", sret, strerror(errno));
		if ( sret > 0 ) {
			if ( FD_ISSET(listenfd, &rd_set) ) {
// #ifndef _WIN32
				children[cli_idx].arg.connfd = accept(listenfd,(struct sockaddr*) &children[cli_idx].arg.cliaddr, &cliaddr_len);
/*
#else
				int tconnfd = accept(listenfd,(struct sockaddr*) &children[cli_idx].arg.cliaddr, &cliaddr_len);
#endif
*/
				if (errno == ECONNABORTED) {
					children[cli_idx].arg.used=0;
					continue;
				}
#if 0
//#ifdef _WIN32
			//	children[cli_idx].arg.connfd = _get_osfhandle( tconnfd );
				children[cli_idx].arg.connfd = _open_osfhandle( tconnfd, _O_RDWR | _O_BINARY | _O_SEQUENTIAL);
				// write(children[cli_idx].arg.connfd, "012345678\n", 10);
				// write(tconnfd, "012345678\n", 10);
				printf("connfd: %d, errno: [%d] %s\n", children[cli_idx].arg.connfd, errno, strerror(errno));
#endif
			}
		}
		//if (errno == EINTR) continue;
		if (children[cli_idx].arg.connfd > 0) {
	//		write(connfd, IDENT, IDENT_LEN);
	//		fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL) | O_NONBLOCK);
			cmutex->lock();
			if (clients>=CLIENTS_MAX) {
				write(children[cli_idx].arg.connfd, "QSCAND: clients limit reached!\n", 31);
				close(children[cli_idx].arg.connfd);
				children[cli_idx].arg.used=0;
				cmutex->unlock();
				continue;
			}
			cmutex->unlock();
			if (!child_create(cli_idx)) {
				cmutex->lock();
				clients++;
				cmutex->unlock();	
				if (debug && !daemonized)
				printf("%d: Clients: %d\n", pid, clients);
			/*		for (int i=0; i<CLIENTS_MAX && cpids[i]!=cpid ; )
					{ if (!cpids[i]) cpids[i]=cpid; else i++; } */
			} else {
#ifdef DAEMON_EN
				if (daemonized)
					syslog(LOG_ERR, "Error creating child process\n");
				else
#endif
					printf("%d: Error creating child process\n", pid);
			}
		} else {
			children[cli_idx].arg.used=0;
		}
	}

	if (debug && !daemonized)
		printf("%d: Closing listen socket...\n",pid);
	close(listenfd);

	printf("%d: Waiting for children: %d remains...\n",pid,clients);
	for (int i=0; i<CLIENTS_MAX; i++ ) {
		if (!children[i].arg.used) continue;
//		printf("waiting for thread %d. used: %d\n", i, children[i].arg.used);
		if (thread_join(children[i].tid, NULL)) {
#ifdef DAEMON_EN
			if (daemonized)
				syslog(LOG_ERR, "Error waiting for child %d\n", i);
			else
#endif
				printf(	"Error waiting for child %d\n", i);
		}
	}
#ifdef _WIN32
	WSACleanup();
#endif

	delete cmutex;
#ifdef DAEMON_EN
	if (daemonized)
		syslog(LOG_INFO, "Listener exit\n");
	else
#endif
		printf("%d: Listener exit\n",pid);
#ifdef DAEMON_EN
	if (daemonized) closelog();
#endif
	return 0;
}

