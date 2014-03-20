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

#ifndef QSCAND_DEFS_H
#define QSCAND_DEFS_H

#if defined (__unix) || defined (__unix__)
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <syslog.h>
#include <netinet/in.h>
#define DAEMON_EN
#elif defined (_WIN32)
#include <winsock2.h>
#endif


#define CLIENTS_MAX 16

#define MAXLINE 1024
#define DEFPORT 46660
#define LISTENQ 16
//#define AUTOPORT_ENABLE
//#define ECHO_CHILD

#define IDENT "QSCAND\n"
#define IDENT_LEN sizeof(IDENT)
#define IDENTV "qScand 0.1\n"
#define IDENTV_LEN sizeof(IDENTV)
#define PROMPT "qscand $ "
#define PROMPT_LEN sizeof(PROMPT)

extern pid_t pid;
extern bool	 daemonized;
extern bool	 debug;
extern bool	 term;

#endif // QSCAND_DEFS_H

