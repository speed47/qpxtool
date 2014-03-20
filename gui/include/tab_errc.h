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

#ifndef _TAB_ERRC_H
#define _TAB_ERRC_H

class QLabel;
class ColorLabel;
class QPushButton;
class QBoxLayout;
class ErrcDetailedDialog;

#include <graphtab.h>
#include <device.h>


class tabERRC : public GraphTab
{
	Q_OBJECT
public:
	tabERRC(QPxSettings *iset, devlist *idev, QString iname, QWidget *p=0, Qt::WindowFlags fl = 0);
	~tabERRC();

public slots:
//	void clear();
	void selectDevice();
	void updateAll();
	void updateLast();
//	void setDevice(device *idev);
//	void update() { graph->update(); };

	void XerrcClosed();

private slots:
	void updateSummary(device*);
	void toggleXErrc();

protected:
	virtual void updateLegend();
	virtual void updateGraph();

private:
	QBoxLayout	*layout_info;

#ifdef __LEGEND_SHOW_SPEED
	ColorLabel	*pl_spd;
#endif
	ColorLabel	*pl_e0;
	QLabel		*l_e0t;
	QLabel		*l_e0m;
	QLabel		*l_e0a;

	ColorLabel	*pl_e1;
	QLabel		*l_e1t;
	QLabel		*l_e1m;
	QLabel		*l_e1a;

	ColorLabel	*pl_e2;
	QLabel		*l_e2t;
	QLabel		*l_e2m;
	QLabel		*l_e2a;

	QPushButton *pb_xerrc;
	ErrcDetailedDialog *xerrc;
};

#endif

