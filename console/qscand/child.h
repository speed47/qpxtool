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

#ifndef CHILDPROC_H
#define CHILDPROC_H

#include <qscand_defs.h>
#include <sys/types.h>
#include <sys/time.h>

typedef struct {
	bool	used;
	struct sockaddr_in cliaddr;
	int    connfd;
} child_arg_t;

typedef struct {
	thread_t	tid;
	child_arg_t	arg;
} child_t;

extern child_t children[CLIENTS_MAX+1];
extern void *child_thread(void *argp);
extern int  child_find_unused();
extern void child_list_clear();

extern int clients;
extern Mutex *cmutex;

#endif // CHILDPROC_H

