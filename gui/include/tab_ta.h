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

#ifndef _TAB_TA_H
#define _TAB_TA_H

class QLabel;
class QBoxLayout;
class QGroupBox;
class QButtonGroup;
class QRadioButton;

class QPxGraph;

class device;
class devlist;
class QPxSettings;
class ColorLabel;

//#define __LEGEND_SHOW_TA


#include <QWidget>
#include <sys/time.h>

class tabTA : public QWidget
{
	Q_OBJECT
public:
	tabTA(QPxSettings *iset, devlist *idev, QString iname, QWidget *p=0, Qt::WindowFlags fl = 0);
	~tabTA();
	void drawGraph(QImage& img, device *dev, int ttype, int eflags=0);

public slots:
	void clear();
	void selectDevice();
//	void infoToggle();
	void updateAll();
	void updateLast();
	void reconfig();

private:
	devlist		*devices;
	QPxSettings	*settings;
	QString		name;
	timeval		prevT;	
	
	QBoxLayout	*layout;
// Selectors/time
	QBoxLayout	*layoutl;
#ifdef __LEGEND_SHOW_TA
	ColorLabel	*cl_pit,
				*cl_land;
#endif
	QGroupBox    *box_layer;
	QBoxLayout   *lay_layer;
	QButtonGroup *grp_layer;
	QRadioButton *layer0;
	QRadioButton *layer1;

	QGroupBox    *box_zone;
	QBoxLayout   *lay_zone;
	QButtonGroup *grp_zone;
	QRadioButton *zone0;
	QRadioButton *zone1;
	QRadioButton *zone2;

	QGroupBox	*grp_time;
	QBoxLayout	*layoutt;
	QLabel		*ltime;

// Pit / Land TA graphs
	QBoxLayout	*layoutg;
	QPxGraph	*graphPit;
	QPxGraph	*graphLand;
};

#endif

