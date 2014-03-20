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

#ifndef _TAB_TRANSFER_H
#define _TAB_TRANSFER_H

class QBoxLayout;
class QLabel;
class ColorLabel;

#include <graphtab.h>
#include <device.h>

//#define __LEGEND_SHOW_SPEED

class tabTransfer : public GraphTab
{
	Q_OBJECT
public:
	tabTransfer(QPxSettings *iset, devlist *idev, QString iname, bool irw,
			QWidget *p=0, Qt::WindowFlags fl = 0);
	~tabTransfer();

public slots:
//	void clear();
	void selectDevice();
	void updateLast();
	void updateSummary(device*, float);

protected:
	virtual void updateLegend();
	virtual void updateGraph();

private:
	bool		rw;
	QBoxLayout	*layout_info;

#ifdef __LEGEND_SHOW_SPEED
	ColorLabel	*pl_spd;
#endif
	QLabel		*pl_sta;
	QLabel		*l_sta_x;
	QLabel		*l_sta_kb;
	QLabel		*pl_end;
	QLabel		*l_end_x;
	QLabel		*l_end_kb;
	QLabel		*pl_avg;
	QLabel		*l_avg_x;
	QLabel		*l_avg_kb;
};

#endif

