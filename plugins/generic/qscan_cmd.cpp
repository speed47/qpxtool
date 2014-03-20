/*
 * generic MMC scan commands implementation ( C2 scan only )
 *
 * This file is part of the QPxTool project.
 * Copyright (C) 2005 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include <stdio.h>
#include <stdlib.h>

#include <qpx_transport.h>
#include <qpx_mmc.h>

#include "qscan_plugin.h"

//int scan_generic::cmd_cd_errc_block(cd_errc *data)
int scan_generic::c2calc(unsigned char* buf, unsigned int lba, unsigned char sects)
{
	const unsigned int sect_data = 2352;
	const unsigned int sect_err  = 294;
	const unsigned int sect_size = sect_data + sect_err;

	unsigned int c2errs = 0;
	unsigned int s, offs, i,ii, se;


	for (s=0; s<sects; s++) {
		se = 0;
		offs = s*sect_size + sect_data;
/*
		printf("\n\n** Sector %d error data:\n", lba+s);
		for (i=0; i<sect_err; i++) {
			if (!(i%16)) printf("\n");
			printf(" %02X", buf[offs+i]);
		}
		printf("\n");
*/
		for (i=0; i<sect_err; i++) {
			for(ii=0; ii<8; ii++)
				if ((buf[offs+i] >> ii) & 0x01 ) {
//					if (!se) printf("C2 in sector %7d, first error in byte %4d ", lba+s, i*8+ii);
					se++;
				}
		}
//		if (se) printf(", %4d C2\n",se);
		c2errs += se;
	}
	return c2errs;
}

int scan_generic::cmd_cd_errc_block(cd_errc *data)
{
	int rsize=15;
	data->bler = 0;
	data->e11 = 0;
	data->e21 = 0;
	data->e31 = 0;
	data->e12 = 0;
	data->e22 = 0;
	data->e32 = 0;
	data->uncr=0;
//	if (!lba) return -1;
	for (int i=0; (i<5) && lba<dev->media.capacity; i++) {
		if (lba + rsize > dev->media.capacity)
			rsize = dev->media.capacity - lba;
//		else
//			rsize = 15;

		if (read_cd(dev, dev->rd_buf, lba, rsize, 0xFA))
			data->uncr++;
		else
			data->e22 += c2calc(dev->rd_buf, lba, rsize);
		lba+=rsize;
	}
//	*lba = 
	return 0;
}

