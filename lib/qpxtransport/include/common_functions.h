/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef __common_functions_h
#define __common_functions_h

//#warning "COMMON_FUNCTIONS_H"

#include <inttypes.h>

#if defined(__unix) || defined(__unix__)

//#warning "UNIX"


#if defined(__APPLE__) && defined(__MACH__)
#include <machine/endian.h>
#elif defined(__linux)
#include <endian.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <sys/endian.h>
#endif


#include <arpa/inet.h>

#elif defined(_WIN32)

#include <sys/param.h>
#include <winsock2.h>

#ifndef _SOCKLEN_T
typedef int socklen_t;
#define EADDRINUSE		WSAEADDRINUSE
#define ECONNABORTED	WSAECONNABORTED

#define SHUT_RD			SD_RECEIVE
#define SHUT_WR			SD_SEND
#define SHUT_RDWR		SD_BOTH
#endif

#endif

typedef struct{
	int	m;
	int	s;
	int	f;
} msf;

typedef struct{
	int	h;
	int	m;
	int	s;
} hms;

extern int64_t  qpx_bswap64(char* c);
extern int64_t  qpx_bswap64(unsigned char* c);
extern int64_t  qpx_bswap64(int64_t c_);
extern int64_t  qpx_bswap64(uint64_t c_);
extern uint64_t qpx_bswap64u(char* c);
extern uint64_t qpx_bswap64u(unsigned char* c);
extern uint64_t qpx_bswap64u(int64_t c_);
extern uint64_t qpx_bswap64u(uint64_t c_);

extern int32_t  qpx_bswap32(char* c);
extern int32_t  qpx_bswap32(unsigned char* c);
extern int32_t  qpx_bswap32(int32_t c_);
extern int32_t  qpx_bswap32(uint32_t c_);
extern uint32_t qpx_bswap32u(char* c);
extern uint32_t qpx_bswap32u(unsigned char* c);
extern uint32_t qpx_bswap32u(int32_t c_);
extern uint32_t qpx_bswap32u(uint32_t c_);

extern int16_t  qpx_bswap16(char* c);
extern int16_t  qpx_bswap16(unsigned char* c);
extern int16_t  qpx_bswap16(int16_t c);
extern int16_t  qpx_bswap16(uint16_t c);
extern uint16_t qpx_bswap16u(char* c);
extern uint16_t qpx_bswap16u(unsigned char* c);
extern uint16_t qpx_bswap16u(int16_t c);
extern uint16_t qpx_bswap16u(uint16_t c);



extern int64_t  to64(char* c);
extern int64_t  to64(unsigned char* c);
extern int64_t  to64(int64_t c_);
extern int64_t  to64(uint64_t c_);

extern uint64_t to64u(char* c);
extern uint64_t to64u(unsigned char* c);
extern uint64_t to64u(int64_t c_);
extern uint64_t to64u(uint64_t c_);

extern int32_t  to32(char* c);
extern int32_t  to32(unsigned char* c);
extern int32_t  to32(int32_t c_);
extern int32_t  to32(uint32_t c_);

extern uint32_t to32u(char* c);
extern uint32_t to32u(unsigned char* c);
extern uint32_t to32u(int32_t c_);
extern uint32_t to32u(uint32_t c_);

extern int16_t  to16(char* c);
extern int16_t  to16(unsigned char* c);
extern int16_t  to16(int16_t c_);
extern int16_t  to16(uint16_t c_);

extern uint16_t to16u(char* c);
extern uint16_t to16u(unsigned char* c);
extern uint16_t to16u(int16_t c_);
extern uint16_t to16u(uint16_t c_);


//#if qpx_BYTE_ORDER == qpx_BIG_ENDIAN
#if BYTE_ORDER == BIG_ENDIAN

//#warning "BIG ENDIAN"
#define ntoh64(x)  to64(x)
#define ntoh64u(x) to64u(x)
#define ntoh32(x)  to32(x)
#define ntoh32u(x) to32u(x)
#define ntoh16(x)  to16(x)
#define ntoh16u(x) to16u(x)

#define hton64(x)  to64(x)
#define hton64u(x) to64u(x)
#define hton32(x)  to32(x)
#define hton32u(x) to32u(x)
#define hton16(x)  to16(x)
#define hton16u(x) to16u(x)


#define be2cpu64(x)  hton64(x)
#define be2cpu64u(x) hton64u(x)
#define be2cpu32(x)  hton32(x)
#define be2cpu32u(x) hton32u(x)
#define be2cpu16(x)  hton16(x)
#define be2cpu16u(x) hton16u(x)

#define cpu2be64(x)  hton64(x)
#define cpu2be64u(x) hton64u(x)
#define cpu2be32(x)  hton32(x)
#define cpu2be32u(x) hton32u(x)
#define cpu2be16(x)  hton16(x)
#define cpu2be16u(x) hton16u(x)


#define le2cpu64(x)  qpx_bswap64(x)
#define le2cpu64u(x) qpx_bswap64u(x)
#define le2cpu32(x)  qpx_bswap32(x)
#define le2cpu32u(x) qpx_bswap32u(x)
#define le2cpu16(x)  qpx_bswap16(x)
#define le2cpu16u(x) qpx_bswap16u(x)

#define cpu2le64(x)  qpx_bswap64(x)
#define cpu2le64u(x) qpx_bswap64u(x)
#define cpu2le32(x)  qpx_bswap32(x)
#define cpu2le32u(x) qpx_bswap32u(x)
#define cpu2le16(x)  qpx_bswap16(x)
#define cpu2le16u(x) qpx_bswap16u(x)

//#elif qpx_BYTE_ORDER == qpx_LITTLE_ENDIAN
#elif BYTE_ORDER == LITTLE_ENDIAN

//#warning "LITTLE ENDIAN"
#define ntoh64(x)  qpx_bswap64(x)
#define ntoh64u(x) qpx_bswap64u(x)
#define ntoh32(x)  qpx_bswap32(x)
#define ntoh32u(x) qpx_bswap32u(x)
#define ntoh16(x)  qpx_bswap16(x)
#define ntoh16u(x) qpx_bswap16u(x)

#define hton64(x)  qpx_bswap64(x)
#define hton64u(x) qpx_bswap64u(x)
#define hton32(x)  qpx_bswap32(x)
#define hton32u(x) qpx_bswap32u(x)
#define hton16(x)  qpx_bswap16(x)
#define hton16u(x) qpx_bswap16u(x)


#define be2cpu64(x)  qpx_bswap64(x)
#define be2cpu64u(x) qpx_bswap64u(x)
#define be2cpu32(x)  qpx_bswap32(x)
#define be2cpu32u(x) qpx_bswap32u(x)
#define be2cpu16(x)  qpx_bswap16(x)
#define be2cpu16u(x) qpx_bswap16u(x)

#define cpu2be64(x)  qpx_bswap64(x)
#define cpu2be64u(x) qpx_bswap64u(x)
#define cpu2be32(x)  qpx_bswap32(x)
#define cpu2be32u(x) qpx_bswap32u(x)
#define cpu2be16(x)  qpx_bswap16(x)
#define cpu2be16u(x) qpx_bswap16u(x)


#define le2cpu64(x)  to64(x)
#define le2cpu64u(x) to64u(x)
#define le2cpu32(x)  to32(x)
#define le2cpu32u(x) to32u(x)
#define le2cpu16(x)  to16(x)
#define le2cpu16u(x) to16u(x)

#define cpu2le64(x)  to64(x)
#define cpu2le64u(x) to64u(x)
#define cpu2le32(x)  to32(x)
#define cpu2le32u(x) to32u(x)
#define cpu2le16(x)  to16(x)
#define cpu2le16u(x) to16u(x)


//#elif qpx_BYTE_ORDER == qpx_PDP_ENDIAN
#elif BYTE_ORDER == PDP_ENDIAN

#error "PDP endian byte order not supported!"

#endif


extern void lba2msf(int lba,  msf* time);
extern int  msf2lba(msf time);
extern void int2hms(int intt, hms* time);


#if defined (_WIN32)
#include <stdlib.h>
#define msleep(t) Sleep(t)
#define sleep(t)  Sleep((t) << 10)
extern int inet_aton(const char *cp, struct in_addr *addr);
#else
#define msleep(t) usleep((t) << 10)
extern int min(int a, int b);
extern int max(int a, int b);
#endif


extern void remove_double_spaces(char* str);
extern void remove_end_spaces(char* str);
//extern void file_path_name(char* str, char* fpath, char* fname);
//extern void file_suf_rm(char* str);
extern int dispers(int m, int* arr, int l);

#endif

