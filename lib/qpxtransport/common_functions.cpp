/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2010, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include <stdio.h>
#include <string.h>

#include "common_functions.h"

// 64-bit wide functions

int64_t bswap64__internal(char* c) {
	int64_t *d64;
	char d[8];
	d[0]=c[7]; d[1]=c[6]; d[2]=c[5]; d[3]=c[4];
	d[4]=c[3]; d[5]=c[2]; d[6]=c[1]; d[7]=c[0];
	d64 = (int64_t*)d;
	return *d64;
}

uint64_t bswap64u__internal(char* c) {
	uint64_t *d64;
	char d[8];
	d[0]=c[7]; d[1]=c[6]; d[2]=c[5]; d[3]=c[4];
	d[4]=c[3]; d[5]=c[2]; d[6]=c[1]; d[7]=c[0];
	d64 = (uint64_t*)d;
	return *d64;
}

int64_t balign64__internal(char* c) {
	int64_t *d64;
	char d[8];
	d[0]=c[0]; d[1]=c[1]; d[2]=c[2]; d[3]=c[3];
	d[4]=c[4]; d[5]=c[5]; d[6]=c[6]; d[7]=c[7];
	d64 = (int64_t*)d;
	return *d64;
}

uint64_t balign64u__internal(char* c) {
	uint64_t *d64;
	char d[8];
	d[0]=c[0]; d[1]=c[1]; d[2]=c[2]; d[3]=c[3];
	d[4]=c[4]; d[5]=c[5]; d[6]=c[6]; d[7]=c[7];
	d64 = (uint64_t*)d;
	return *d64;
}

int64_t qpx_bswap64(char* c)			{ return bswap64__internal(c); };
int64_t qpx_bswap64(unsigned char* c) { return bswap64__internal((char*)c); };
int64_t qpx_bswap64(int64_t c)		{ return bswap64__internal((char*)&c); }
int64_t qpx_bswap64(uint64_t c)		{ return bswap64__internal((char*)&c); }

uint64_t qpx_bswap64u(char* c)		  { return bswap64u__internal(c); };
uint64_t qpx_bswap64u(unsigned char* c) { return bswap64u__internal((char*)c); };
uint64_t qpx_bswap64u(int64_t c)		  { return bswap64u__internal((char*)&c); }
uint64_t qpx_bswap64u(uint64_t c)		  { return bswap64u__internal((char*)&c); }

int64_t  to64(char* c)			 { return balign64__internal(c); };
int64_t  to64(unsigned char* c)  { return balign64__internal((char*)c); };
int64_t  to64(int64_t c)		 { return c; }
int64_t  to64(uint64_t c)		 { return (int64_t)c; }

uint64_t to64u(char* c)			 { return balign64u__internal(c); };
uint64_t to64u(unsigned char* c) { return balign64u__internal((char*)c); };
uint64_t to64u(int64_t c)		 { return (uint64_t)c; }
uint64_t to64u(uint64_t c)		 { return c; }


// 32-bit wide functions

int32_t bswap32__internal(char* c) {
	int32_t *d32;
	char d[4];
	d[0]=c[3]; d[1]=c[2]; d[2]=c[1]; d[3]=c[0];
	d32 = (int32_t*)d;
	return *d32;
}

uint32_t bswap32u__internal(char* c) {
	uint32_t *d32;
	char d[4];
	d[0]=c[3]; d[1]=c[2]; d[2]=c[1]; d[3]=c[0];
	d32 = (uint32_t*)d;
	return *d32;
}

int32_t balign32__internal(char* c) {
	int32_t *d32;
	char d[4];
	d[0]=c[0]; d[1]=c[1]; d[2]=c[2]; d[3]=c[3];
	d32 = (int32_t*)d;
	return *d32;
}

uint32_t balign32u__internal(char* c) {
	uint32_t *d32;
	char d[4];
	d[0]=c[0]; d[1]=c[1]; d[2]=c[2]; d[3]=c[3];
	d32 = (uint32_t*)d;
	return *d32;
}

int32_t qpx_bswap32(char* c)			{ return bswap32__internal(c); };
int32_t qpx_bswap32(unsigned char* c){ return bswap32__internal((char*)c); };
int32_t qpx_bswap32(int32_t c)		{ return bswap32__internal((char*)&c); }
int32_t qpx_bswap32(uint32_t c)		{ return bswap32__internal((char*)&c); }

uint32_t qpx_bswap32u(char* c)		{ return bswap32u__internal(c); };
uint32_t qpx_bswap32u(unsigned char* c) { return bswap32u__internal((char*)c); };
uint32_t qpx_bswap32u(int32_t c)		{ return bswap32u__internal((char*)&c); }
uint32_t qpx_bswap32u(uint32_t c)	{ return bswap32u__internal((char*)&c); }

int32_t  to32(char* c)			 { return balign32__internal(c); };
int32_t  to32(unsigned char* c)  { return balign32__internal((char*)c); };
int32_t  to32(int32_t c)		 { return c; }
int32_t  to32(uint32_t c)		 { return (int32_t)c; }

uint32_t to32u(char* c)			 { return balign32u__internal(c); };
uint32_t to32u(unsigned char* c) { return balign32u__internal((char*)c); };
uint32_t to32u(int32_t c)		 { return *(uint32_t*)c; }
uint32_t to32u(uint32_t c)		 { return c; }

// 16-bit wide functions

int16_t bswap16__internal(char* c) {
	int16_t *d16;
	char d[2];
	d[0]=c[1]; d[1]=c[0];
	d16 = (int16_t*)d;
	return *d16;
}

uint16_t bswap16u__internal(char* c) {
	uint16_t *d16;
	char d[2];
	d[0]=c[1]; d[1]=c[0];
	d16 = (uint16_t*)d;
	return *d16;
}

int16_t balign16__internal(char* c) {
	int16_t *d16;
	char d[2];
	d[0]=c[0]; d[1]=c[1];
	d16 = (int16_t*)d;
	return *d16;
}

uint16_t balign16u__internal(char* c) {
	uint16_t *d16;
	char d[2];
	d[0]=c[0]; d[1]=c[1];
	d16 = (uint16_t*)d;
	return *d16;
}

int16_t  qpx_bswap16(char* c)		  { return bswap16__internal(c); }
int16_t  qpx_bswap16(unsigned char* c) { return bswap16__internal((char*)c); }
int16_t  qpx_bswap16(int16_t c)		  { return bswap16__internal((char*)&c); }
int16_t  qpx_bswap16(uint16_t c)	  { return bswap16__internal((char*)&c); }
uint16_t qpx_bswap16u(char* c)		  { return bswap16u__internal(c); }
uint16_t qpx_bswap16u(unsigned char* c)  { return bswap16u__internal((char*)c); }
uint16_t qpx_bswap16u(int16_t c)	  { return bswap16__internal((char*)&c); }
uint16_t qpx_bswap16u(uint16_t c)	 { return bswap16__internal((char*)&c); }


int16_t  to16(char* c)			 { return balign16__internal(c); };
int16_t  to16(unsigned char* c)	 	 { return balign16__internal((char*)c); };
int16_t  to16(int16_t c)		 { return c; }
int16_t  to16(uint16_t c)		 { return (int16_t)c; }
uint16_t to16u(char* c)			 { return balign16u__internal(c); };
uint16_t to16u(unsigned char* c) 	 { return balign16u__internal((char*)c); };
uint16_t to16u(int16_t c)		 { return (uint16_t)c; }
uint16_t to16u(uint16_t c)		 { return c; }

void lba2msf(int lba, msf* time){
	time->m = lba/4500;
	time->s = (lba/75)%60;
	time->f = lba % 75;
}

int msf2lba(msf time){
	return time.m * 4500 + time.s * 75 + time.f;
}

void int2hms(int intt, hms* time){
	time->h = intt/3600;
	time->m = (intt/60)%60;
	time->s = intt % 60;
}

#ifndef _WIN32

int min(int a, int b)
{
	if (a<=b)
		return a;
	return b;
}

int max(int a, int b)
{
	if (a>b)
		return a;
	return b;
}

#else

int inet_aton(const char *cp, struct in_addr *addr)
{
	addr->s_addr = inet_addr(cp);
	return 1;
}

#endif

void remove_double_spaces(char* str)
{
	int len = strlen(str);
	while(len--) {
		if (*str == 0x20 && str[1] == 0x20) 
			strcpy(str, str+1);
		else
			str++;
	}
}

void remove_end_spaces(char* str)
{
	int len = strlen(str);
	while (str[--len] == 0x20)
		str[len] = 0;
}

int dispers(int m, int* arr, int l) {
	if ((!m) || (!arr) || (l<2)) return 0;
	long disp = 0;
	int i;
	int q;
	for (i=0; i<l; i++) {
	    q = (m - arr[i]);
	    disp += (q*q);
	}
	return (disp/(l-1));
}

