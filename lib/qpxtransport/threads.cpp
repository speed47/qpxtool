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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "threads.h"

#include <fcntl.h>

/*
 * sync_pipe_add_arg & protect_arg functions from ethereal
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@ethereal.com>
 * Copyright 1998 Gerald Combs
 *
*/

/* Append an arg (realloc) to an argc/argv array */
/* (add a string pointer to a NULL-terminated array of string pointers) */
char **
add_arg(char **args, int *argc, const char *arg)
{
  /* Grow the array; "*argc" currently contains the number of string
     pointers, *not* counting the NULL pointer at the end, so we have
     to add 2 in order to get the new size of the array, including the
     new pointer and the terminating NULL pointer. */
  args = (char**) realloc( (void*) args, (*argc + 2) * sizeof (char *));

  /* Stuff the pointer into the penultimate element of the array, which
     is the one at the index specified by "*argc". */
  args[*argc] = (char*) arg;

  /* Now bump the count. */
  (*argc)++;

  /* We overwrite the NULL pointer; put it back right after the
     element we added. */
  args[*argc] = NULL;

  return args;
}

#ifdef _WIN32
/* Quote the argument element if necessary, so that it will get
 * reconstructed correctly in the C runtime startup code.  Note that
 * the unquoting algorithm in the C runtime is really weird, and
 * rather different than what Unix shells do. See stdargv.c in the C
 * runtime sources (in the Platform SDK, in src/crt).
 *
 * Stolen from GLib's protect_argv(), an internal routine that quotes
 * string in an argument list so that they arguments will be handled
 * correctly in the command-line string passed to CreateProcess()
 * if that string is constructed by gluing those strings together.
 */
char *
protect_arg (const char *argv)
{
    char  *new_arg;
    const char *p = argv;
    char  *q;
    int   len = 0;
    bool  need_dblquotes = FALSE;

    while (*p) {
        if (*p == ' ' || *p == '\t')
            need_dblquotes = TRUE;
        else if (*p == '"')
            len++;
        else if (*p == '\\') {
            const char *pp = p;

            while (*pp && *pp == '\\')
                pp++;
            if (*pp == '"')
                len++;
	}
        len++;
        p++;
    }

    q = new_arg = (char*) malloc (len + need_dblquotes*2 + 1);
    p = argv;

    if (need_dblquotes)
        *q++ = '"';

    while (*p) {
        if (*p == '"')
            *q++ = '\\';
        else if (*p == '\\') {
            const char *pp = p;

            while (*pp && *pp == '\\')
                pp++;
            if (*pp == '"')
                *q++ = '\\';
	}
	*q++ = *p;
	p++;
    }

    if (need_dblquotes)
        *q++ = '"';
    *q++ = '\0';

    return new_arg;
}

void string_append_c(char* args, char c)
{
	int slen = strlen(args);
	args[slen] = c;
	args[slen+1] = 0;
}

void string_append(char* args, char* arg)
{
	int slen = strlen(args);
	int alen = strlen(arg);
	memcpy(args+slen, arg, alen);
	args[slen+alen] = 0;
}

int close(HANDLE h) { CloseHandle(h); }
#endif


#if defined (__unix) || defined (__unix__)

Mutex::Mutex() { pthread_mutex_init(&m, NULL); }
Mutex::~Mutex() { pthread_mutex_destroy(&m); }
void Mutex::lock()   { 
//	printf("%p mutex.lock()\n", this);
	pthread_mutex_lock(&m);
}

void Mutex::unlock() {
//	printf("%p mutex.unlock()\n", this);
	pthread_mutex_unlock(&m);
}

#elif defined (_WIN32)

Mutex::Mutex() { InitializeCriticalSection(&m); }
Mutex::~Mutex() { DeleteCriticalSection(&m); }
void Mutex::lock()   { EnterCriticalSection(&m); }
void Mutex::unlock() { LeaveCriticalSection(&m); }

int WIN32_thread_create(HANDLE *tid, void *attr, void*(*func)(void*), void* arg)
{
	*tid = CreateThread(NULL,0, (DWORD WINAPI (*)(void*)) func,arg,0,NULL);
	if (!(*tid)) return 1;
	else		 return 0;
};

int WIN32_thread_join(HANDLE& tid, void **ret)
{
	DWORD	tret;
	if (WaitForSingleObject(tid, INFINITE) == WAIT_FAILED) return -1;
	if (ret) {
		if (!GetExitCodeThread(tid, &tret)) return -1;
		*ret = (void*) tret;
	}
	CloseHandle(tid);
	tid = NULL;
	return 0;
};

#endif

//int createchild(char **argv, pipe_t &rdpipe, bool r, pipe_t &wrpipe, bool w)
int createChildProcess(char **argv, pipe_t *rdpipe, pipe_t *wrpipe)
{
	printf("createchild(): pipes: %p, %p\n", rdpipe, wrpipe);

#if defined (__unix) || defined (__unix__)
	int cpid;

	if (rdpipe) {
		if (pipe(*rdpipe)) {
			printf("Can't create pipe for stdout/stderr\n");
			return -1;
		}
		printf("rdpipe = %d, %d\n", (*rdpipe)[0], (*rdpipe)[1]);
	}
	
	if (wrpipe) {
		if (pipe(*wrpipe)) {
			printf("Can't create pipe for stdout/stderr\n");
			if (rdpipe) { close(*rdpipe[0]); close(*rdpipe[1]); }
			return -1;
		}
		printf("wrpipe = %d, %d\n", (*wrpipe)[0], (*wrpipe)[1]);
	}

	if ((cpid = fork()) == -1) {
		printf("Can't fork()\n");
		if (rdpipe) { close((*rdpipe)[0]); close((*rdpipe)[1]); }
		if (wrpipe) { close((*wrpipe)[0]); close((*wrpipe)[1]); }
		return -1;
	}

	if (!cpid) {
		printf("child: fork() ok:)\n");
		if (rdpipe) {
			close((*rdpipe)[0]); // unused read end
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			dup2((*rdpipe)[1], STDOUT_FILENO);
			dup2((*rdpipe)[1], STDERR_FILENO);
		}
		if (wrpipe) {
			close((*wrpipe)[1]); // unused write end
			close(STDIN_FILENO);
			dup2((*wrpipe)[0], STDIN_FILENO);
		}
		printf("starting %s...\n", argv[0]);
		execvp(argv[0], argv);
		printf("Can't exec() %s: %s\n", argv[0], strerror(errno));
		_exit(0);
	} else {
		if (rdpipe) close((*rdpipe)[1]); // unused write end
		if (wrpipe) close((*wrpipe)[0]); // unused read end
	}
	return cpid;

#elif defined (_WIN32)
	char args[1024] = ""; 
	char *quoted_arg;
	SECURITY_ATTRIBUTES sa;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
	HANDLE  ipiper;
	HANDLE  ipipew;
	HANDLE  opiper;
	HANDLE  opipew;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	if (rdpipe) {
		if (!CreatePipe(&ipiper, &ipipew, &sa, 5120))
		{
			printf("Can't create pipe for stdout/stderr\n");
			return -1;
		}
	//	printf("rdpipeH = %d, %d\n", (int)ipiper, (int)ipipew);
		(*rdpipe)[0] = _open_osfhandle((long)ipiper, _O_BINARY);
		(*rdpipe)[1] = _open_osfhandle((long)ipipew, _O_BINARY);
		printf("rdpipeF = %d, %d\n", (*rdpipe)[0], (*rdpipe)[1]);
	}

	if (wrpipe) {
		if (!CreatePipe(&opiper, &opipew, &sa, 5120))
		{
			if (rdpipe) { close((*rdpipe)[0]); close((*rdpipe)[1]); }
			printf("Can't create pipe for stdin\n");
			return -1;
		}
	//	printf("wrpipeH = %d, %d\n", (int)opipew, (int)opipew);
		(*wrpipe)[0] = _open_osfhandle((long)opiper, _O_BINARY);
		(*wrpipe)[1] = _open_osfhandle((long)opipew, _O_BINARY);
		printf("wrpipeF = %d, %d\n", (*wrpipe)[0], (*wrpipe)[1]);
	}

    memset(&si, 0, sizeof(si));
    si.cb          = sizeof(si);
    si.dwFlags     = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  /* this hides the console window */
	if (wrpipe) si.hStdInput    = opiper;
    if (rdpipe) { si.hStdOutput = ipipew; si.hStdError = ipipew; }

    for(int i=0; argv[i] != 0; i++) {
        if(i != 0) string_append_c(args, ' ');    // don't prepend a space before the path!!!
        quoted_arg = protect_arg(argv[i]);
        string_append(args, quoted_arg);
        free(quoted_arg);
    }

    if(!CreateProcessA(NULL, args, NULL, NULL, TRUE,
                      CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		if (rdpipe) { close((*rdpipe)[0]); close((*rdpipe)[1]); }
		if (wrpipe) { close((*wrpipe)[0]); close((*wrpipe)[1]); }
		errno = GetLastError();
		printf("Can't CreateProcess():\n%s\nERR: [%d] %s\n", args, errno, strerror(errno));
		return -1;
	}

	if (rdpipe) close((*rdpipe)[1]);
	if (wrpipe) close((*wrpipe)[0]);
	return (int) pi.hProcess;
#endif
}

