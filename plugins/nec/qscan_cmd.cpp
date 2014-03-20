/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2006 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 *
 * NEC Cx scan commands got from readcd path by Alexander Noe`
 *
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
#include <common_functions.h>

#include <qscan_plugin.h>
//#include "media_check_nec.h"

// ************* Scan init commands *********
int scan_nec::cmd_cd_errc_init() {
	/* initialize scan mode */
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x01;
	if ((dev->err=dev->cmd.transport(NONE, NULL, 0))){
		if(!dev->silent) sperror ("nec_init_errc_scan",dev->err); return 1;
	}
	/* set scan interval = 75 sectors */
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x02;
	dev->cmd[8] = 75;  // interval   in sectors
	if ((dev->err=dev->cmd.transport(NONE, NULL, 0))){
		if(!dev->silent) sperror ("nec_set_scan_interval",dev->err); return 1;
	}
	return 0;
}

int scan_nec::cmd_dvd_errc_init() {
	/* initialize scan mode */
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x01;
	if ((dev->err=dev->cmd.transport(NONE, NULL, 0))){
		if(!dev->silent) sperror ("nec_init_errc_scan",dev->err); return 1;
	}
	/* set scan interval = 16 sectors = 1 ECC */
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x02;
	dev->cmd[8] = 0x01;  // interval   in ECC blocks 
	if ((dev->err=dev->cmd.transport(NONE, NULL, 0))){
		if(!dev->silent) sperror ("nec_set_scan_interval",dev->err); return 1;
	}
	return 0;
}

int scan_nec::cmd_bd_errc_init() {
	/* initialize scan mode */
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x01;
	if ((dev->err=dev->cmd.transport(NONE, NULL, 0))){
		if(!dev->silent) sperror ("nec_init_errc_scan",dev->err); return 1;
	}
	/* set scan interval = 32 sectors (equivalent to 2 ECC blocks on DVD) */
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x02;
	dev->cmd[8] = 0x02;  // interval
	if ((dev->err=dev->cmd.transport(NONE, NULL, 0))){
		if(!dev->silent) sperror ("nec_set_scan_interval",dev->err); return 1;
	}
	return 0;
}

// **********************
int scan_nec::cmd_cd_errc_block(cd_errc *data)
{
	long tlba;
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x03;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,8))){
		sperror ("nec_cd_errc_do_one_interval",dev->err);
		if (dev->err == 0x30200) return 1;
		else return -1;
	}
	data->bler = ntoh16(dev->rd_buf+4);
	data->e11 = 0;
	data->e21 = 0;
	data->e31 = 0;
	data->e12 = 0;
	data->e22 = ntoh16(dev->rd_buf+6);
	data->e32 = 0;
	data->uncr = 0;
	lba+=75;
	tlba=((int)dev->rd_buf[1] * 4500 + (int)dev->rd_buf[2] * 75 + (int)dev->rd_buf[3]);
	if (lba<tlba) lba=tlba;
	return 0;
}

int scan_nec::cmd_dvd_errc_block(dvd_errc *data)
{
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x03;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,8))){
		sperror ("nec_dvd_errc_do_one_interval",dev->err);
		if (dev->err == 0x30200) return 1;
		else return -1;
	}
	data->pie = ntoh16(dev->rd_buf+4);
//	data->pi8 = data->pie;
	data->pif = ntoh16(dev->rd_buf+6);
	data->poe = 0;
//	data->po8 = data->poe;
	data->pof = 0;
	lba=ntoh32(dev->rd_buf);
//	*lba+=0x80;
	return 0;
}

int scan_nec::cmd_bd_errc_block(bd_errc *data)
{
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x03;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,8))){
		sperror ("nec_bd_errc_do_one_interval",dev->err);
		if (dev->err == 0x30200) return 1;
		else return -1;
	}
	data->ldc = ntoh16(dev->rd_buf+4);
	data->bis = ntoh16(dev->rd_buf+6);
	lba=ntoh32(dev->rd_buf);
//	*lba+=0x80;
	return 0;
}

// ************* END SCAN COMMAND *********
int scan_nec::cmd_scan_end()
{
	dev->cmd[0] = 0xF3;
	dev->cmd[1] = 0x0F;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,8))){
		sperror ("nec_end_scan",dev->err); return 1;
	}
	return 0;
}

