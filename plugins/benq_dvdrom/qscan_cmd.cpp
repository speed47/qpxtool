/*
 * BENQ DVDROM scan conmmands implementation
 *
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
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

// ************* Scan init commands *********
int scan_benqrom::cmd_scan_init() {
	dev->cmd[0] = 0xF1;
	dev->cmd[2] = 0xFF;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,4))){
		if(!dev->silent) sperror ("benq_rom_init_scan",dev->err);
		return dev->err;
	}
	cnt=0;
	return 0;
}

int scan_benqrom::cmd_cd_errc_block(cd_errc *data)
{
	int len = 34;
	if (!(cnt%cnt_max_cd)) {
		dev->cmd[0] = 0x78;
		if (!lba) { dev->cmd[5] = 0x01; dev->cmd[6] = 0x00; len=32; }
		else { dev->cmd[5] = 0x00; dev->cmd[6] = 0x01; }
		dev->cmd[8] = 0x20;
		dev->cmd[10]= 0x10;
		if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,len))){
			sperror ("benq_rom_cx_do_one_interval",dev->err);
			return dev->err;
		}
		for (int i=0; i<4; i++) {
			_c1[i] = ntoh16(dev->rd_buf+4+i*4);
			_c2[i] = ntoh16(dev->rd_buf+6+i*4);
//			_lba=((int)dev->rd_buf[0] * 4500 + (int)dev->rd_buf[3] * 75 + (int)dev->rd_buf[2]);
		}
		cnt = 0;
	}
	data->bler = _c1[cnt];
	data->e11 = 0;
	data->e21 = 0;
	data->e31 = 0;
	data->e12 = 0;
	data->e22 = _c2[cnt];
	data->e32 = 0; //_cu[cnt];
	data->uncr= 0;
	lba+=75;
	cnt++;
//	*lba = _lba;
	return 0;
}

int scan_benqrom::cmd_cd_jb_block(cdvd_jb* data)
{
//*
	if (!lba) {
		dev->cmd[0] = 0x78;
		dev->cmd[5] = 0x01;
		dev->cmd[6] = 0x00;
		dev->cmd[8] = 0x20;
		dev->cmd[10]= 0x10;
		if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,32))){
			sperror ("benq_rom_jitter_cd_do_one_interval",dev->err);
			return dev->err;
		}
		lba+=75;
	}
	dev->cmd[0] = 0x78;
	dev->cmd[5] = 0x00;
	dev->cmd[6] = 0x01;
	dev->cmd[8] = 0x20;
	dev->cmd[10]= 0x10;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,34))){
		sperror ("benq_rom_jitter_cd_do_one_interval",dev->err);
		return dev->err;
	}
	data->jitter = 20*(int)dev->rd_buf[0x20];
	data->asymm  = 0;
//	*lba=((int)dev->rd_buf[0] * 4500 + (int)dev->rd_buf[3] * 75 + (int)dev->rd_buf[2]);
	lba+=75;
	return 0;
}

int scan_benqrom::cmd_dvd_errc_block(dvd_errc *data)
{
	if (!(cnt%cnt_max_cd)) {
		dev->cmd[0] = 0x78;
		if (!lba) { dev->cmd[6] = 0x00; dev->cmd[7] = 0x10; }
		else { dev->cmd[6] = 0x01; dev->cmd[7] = 0x00; }
		dev->cmd[8] = 0x22;
		dev->cmd[10]= 0x10;
		if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,34))){
			sperror ("benq_rom_pie_pif_do_one_interval",dev->err);
			return dev->err;
		}
		for (int i=0; i<4; i++) {
			_pie[i] = ntoh16(dev->rd_buf+4+i*4);
			_pif[i] = ntoh16(dev->rd_buf+6+i*4);
//			_lba=((int)dev->rd_buf[0] * 4500 + (int)dev->rd_buf[3] * 75 + (int)dev->rd_buf[2]);
		}
		cnt = 0;
	}
	data->pie = _pie[cnt];
	data->pif = _pif[cnt];
	data->poe = 0;
	data->pof = 0;
	lba+=0x100;
//	printf("BenQ Pi SCAN: cnt=%d\n",cnt);
	cnt++;
//	*lba = _lba;
	return 0;
}

int scan_benqrom::cmd_dvd_jb_block(cdvd_jb* data)
{
	dev->cmd[0] = 0x78;
	if (!lba) { dev->cmd[6] = 0x00; dev->cmd[7] = 0x10; }
	else { dev->cmd[6] = 0x01; dev->cmd[7] = 0x00; }
	dev->cmd[8] = 0x22;
	dev->cmd[10]= 0x10;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,34))){
		sperror ("benq_rom_cx_do_one_interval",dev->err);
		return dev->err;
	}
	data->jitter = 20*(int)dev->rd_buf[0x20];
	data->asymm = 0;
//	*lba=((int)dev->rd_buf[0] * 4500 + (int)dev->rd_buf[3] * 75 + (int)dev->rd_buf[2]);
	lba+= 0x400;
	return 0;
}

// ************* END SCAN COMMAND *********
int scan_benqrom::cmd_cd_end()
{
	dev->cmd[0] = 0x78;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,32))){
		sperror ("benq_rom_end_scan_cd",dev->err);
		return dev->err;
	}
	printf("\n");
	return 0;
}

int scan_benqrom::cmd_dvd_end()
{
	dev->cmd[0] = 0x78;
	dev->cmd[7] = 0x40;
	dev->cmd[8] = 0x22;
	dev->cmd[10]= 0x10;
	if ((dev->err=dev->cmd.transport(READ,dev->rd_buf,34))){
		sperror ("benq_rom_end_scan_dvd",dev->err);
		return dev->err;
	}
	return 0;
}

/*
scan_commands commands_list_benq_rom = {
	NULL,	benq_rom_init_scan,	benq_rom_cx_do_one_interval,		benq_rom_cx_end_scan,
	NULL,	benq_rom_init_scan,	benq_rom_jitter_CD_do_one_interval,	benq_rom_cx_end_scan,

	NULL,	NULL,			NULL,					NULL,
	NULL,	NULL,			NULL,					NULL,
	NULL,	NULL,			NULL,					NULL,
	NULL,	benq_rom_init_scan,	benq_rom_pie_pif_do_one_interval,	benq_rom_pi_end_scan,
	NULL,	benq_rom_init_scan,	benq_rom_jitter_DVD_do_16_ecc,		benq_rom_pi_end_scan,
};

scan_commands commands_benq_rom()  { return commands_list_benq_rom; }
*/

