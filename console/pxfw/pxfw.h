/*
 * This file is a part of QPxTool project
 * Copyright (C) 2006-2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; see the file COPYING.  If not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __PXFW_H
#define __PXFW_H

// const int WR_PLEXTOR	= 0x0001;

/*
const uint32_t PX_OLD	= 0x0001;
const uint32_t PX_PREMIUM	= 0x0002;
const uint32_t PX_708	= 0x0004;
const uint32_t PX_708A2	= 0x0008;
const uint32_t PX_712	= 0x0010;
const uint32_t PX_714	= 0x0020;
const uint32_t PX_716	= 0x0040;
const uint32_t PX_716AL	= 0x0080;
const uint32_t PX_755	= 0x0100;
const uint32_t PX_760	= 0x0200;
const uint32_t PX_PREMIUM2	= 0x0400;
*/

const uint32_t FL_HELP		= 0x00000001;
const uint32_t FL_DEV		= 0x00000002;
const uint32_t FL_FWIF		= 0x00000004;
const uint32_t FL_FWOF		= 0x00000008;
const uint32_t FL_UPDATE	= 0x00000010;
const uint32_t FL_BACKUP	= 0x00000020;
const uint32_t FL_DEBUG		= 0x00000040;
const uint32_t FL_TEST		= 0x00000080;
const uint32_t FL_RESET		= 0x00000100;
const uint32_t FL_FORCE		= 0x00000200;
const uint32_t FL_SCAN		= 0x00000400;
const uint32_t FL_EEPROM	= 0x00000800;
const uint32_t FL_EEIF		= 0x00001000;
const uint32_t FL_EEOF		= 0x00002000;

#endif // __PXFW_H
