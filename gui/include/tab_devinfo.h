/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef _TAB_DEVINFO_H
#define _TAB_DEVINFO_H

#include <QWidget>
#include <qpxsettings.h>
#include <device.h>


class QBoxLayout;
class QGridLayout;

class QFrame;
class QLabel;
class QComboBox;

#ifdef DINFO_TREE
class QTreeWidget;
class QTreeWidgetItem;
#else

#define CAP_COLS 4

class MediaCapWidget;
#endif

class device;

class tabDevInfo : public QWidget {
	Q_OBJECT
public:
	tabDevInfo(QPxSettings *iset, devlist *idev, QWidget *p=0, Qt::WindowFlags fl = 0);
	~tabDevInfo();

public slots:
	void clear();
	void selectDevice();
	void updateData(int xcode=0);

private:
	devlist		*devices;
	QPxSettings	*settings;

	QBoxLayout  *layout;
	QGridLayout *layout_left;

// basic info
	QLabel		*label_left;

	QLabel		*pl_vendor,
				*l_vendor,
				*pl_model,
				*l_model,
				*pl_fw,
				*l_fw,
				*pl_tla,
				*l_tla,
				*pl_buf,
				*l_buf,
				*pl_sn,
				*l_sn;

	QLabel		*pl_iface,
				*l_iface,
				*pl_loader,
				*l_loader;

// DVD RPC info
	QLabel		*pl_rpc_phase,
				*l_rpc_phase,
				*pl_rpc_reg,
				*l_rpc_reg,
				*pl_rpc_ch,
				*l_rpc_ch,
				*pl_rpc_rst,
				*l_rpc_rst;
// Plextor Life
	QFrame		*hline0;
	QLabel		*pl_life_dn,
				*l_life_dn,
				*pl_life_cr,
				*l_life_cr,
				*pl_life_cw,
				*l_life_cw,
				*pl_life_dr,
				*l_life_dr,
				*pl_life_dw,
				*l_life_dw;

	QGridLayout		*cap_grid;
	QLabel			*lc_media,
					*lc_generic;
	QList<MediaCapWidget*>	cap_media,
							cap_generic;
};

#endif

