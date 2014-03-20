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

#ifndef _TAB_MEDIAINFO_H
#define _TAB_MEDIAINFO_H

#include <QWidget>
#include <qpxsettings.h>
#include <device.h>

class QBoxLayout;
class QGridLayout;


class QLabel;
class QFrame;
class QPushButton;
class QCheckBox;

#ifdef MINFO_TREE
class QTreeWidget;
class QTreeWidgetItem;
#endif

class tabMediaInfo : public QWidget
{
	Q_OBJECT
public:
	tabMediaInfo(QPxSettings *iset, devlist *idev, QWidget *p=0, Qt::WindowFlags fl = 0);
	~tabMediaInfo();

public slots:
	void clear();
	void selectDevice();
	void updateData(int xcode=0);

private:
	devlist		*devices;
	QPxSettings	*settings;

	QBoxLayout  *layout;

	QGridLayout *layout_info;
	QLabel		*pl_type;
	QLabel		*l_type;
	QLabel		*pl_book;
	QLabel		*l_book;
	QLabel		*pl_mid;
	QLabel		*l_mid;
	QLabel		*pl_layers;
	QLabel		*l_layers;
	QLabel		*pl_erasable;
	QLabel		*l_erasable;
	QLabel		*pl_prot;
	QLabel		*l_prot;
	QLabel		*pl_regions;
	QLabel		*l_regions;
	QLabel		*pl_writer;
	QLabel		*l_writer;

	QGridLayout  *l_cap;
	QLabel		*pl_cread;
	QLabel		*l_creads;
	QLabel		*l_creadm;
	QLabel		*l_creadmsf;
	QLabel		*pl_cfree;
	QLabel		*l_cfrees;
	QLabel		*l_cfreem;
	QLabel		*l_cfreemsf;
	QLabel		*pl_ctot;
	QLabel		*l_ctots;
	QLabel		*l_ctotm;
	QLabel		*l_ctotmsf;

	QLabel		*pl_dstate;
	QLabel		*l_dstate;
	QLabel		*pl_sstate;
	QLabel		*l_sstate;
	QLabel		*pl_rspeeds;
	QLabel		*l_rspeeds;
	QLabel		*pl_wspeedsm;
	QLabel		*l_wspeedsm;
	QLabel		*pl_wspeedsd;
	QLabel		*l_wspeedsd;


	QFrame		*hline0;


// media speeds info
#ifdef SHOW_SPEEDS
	QLabel		*pl_rd_max;
	QLabel		*l_rd_max;
	QLabel		*pl_rd_lst;
	QComboBox	*c_rd_lst;
	QLabel		*pl_wr_max;
	QLabel		*l_wr_max;
	QLabel		*pl_wr_lst;
	QComboBox	*c_wr_lst;
#endif


#if 0
	QCheckBox	*ch_rt;
	QCheckBox	*ch_wt;
	QCheckBox	*ch_errc;
	QCheckBox	*ch_jb;
	QCheckBox	*ch_fete;
	QCheckBox	*ch_ta;
#endif
	//	device *dev;
#ifdef MINFO_TREE
	QTreeWidget *minfo;
	QTreeWidgetItem *minfo_detail;
#endif
};

#endif

