/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef DR_THREADS_H
#define DR_THREADS_H

#if defined (__unix) || defined (__unix__)

#include <stdio.h>
#include <pthread.h>
class Mutex {
public:
	Mutex();
	~Mutex();
	void lock();
	void unlock();
private:
	pthread_mutex_t m;
};

#define thread_t pthread_t
#define thread_create(id,attr,func,arg)	pthread_create(id,attr,func,arg)
#define thread_join(id,f)				pthread_join(id, f)
#define thread_exit(r)					pthread_exit((void*)(r))

#elif defined (_WIN32)

#include <windows.h>
class Mutex {
public:
	Mutex();
	~Mutex();
	void lock();
	void unlock();
private:
	CRITICAL_SECTION m;
};

#define thread_t HANDLE
extern int WIN32_thread_create(HANDLE *tid, void *attr, void*(*func)(void*), void* arg);
extern int WIN32_thread_join(HANDLE& tid, void **ret);

#define thread_create(id,attr,func,arg)	WIN32_thread_create(id,attr,func,arg)
#define thread_join(id,f)				WIN32_thread_join(id, f)
#define thread_exit(r)					ExitThread(r)

extern int close(HANDLE h);

#endif

typedef int pipe_t[2];

extern char **add_arg(char **args, int *argc, const char *arg);
//extern int createchild(char **argv, pipe_t &rdpipe, bool r, pipe_t &wrpipe, bool w);
extern int createChildProcess(char **argv, pipe_t *rdpipe = NULL, pipe_t *wrpipe = NULL);

#endif // DR_THREADS_H

