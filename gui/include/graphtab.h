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

#ifndef GRAPHTAB_H
#define GRAPHTAB_H

#include <QWidget>
#include <sys/time.h>

class device;
class QLabel;
class QFrame;
class QGroupBox;
class QBoxLayout;
class SplitButton;
class QPxGraph;

class devlist;
class QPxSettings;

class GraphTab : public QWidget
{
	Q_OBJECT
public:
	GraphTab(QPxSettings *iset, devlist *idev, QString iname, int test, QWidget *p=0, Qt::WindowFlags fl = 0);
	~GraphTab();
	void drawGraph(QImage& img, device *dev, int ttype, int eflags=0);

public slots:
	void clear();
	virtual void selectDevice()=0;
	void updateLast(int, bool *Tvalid=NULL, bool force=0);
//	void setDevice(device *idev);
	void reconfig() { updateLegend(); updateGraph(); };

private slots:
	void infoToggle();

protected:
	virtual void updateGraph()=0;
	virtual void updateLegend()=0;

	devlist		*devices;
	QPxSettings	*settings;
	QWidget		*infow;
	QPxGraph	*graph;
	QLabel		*ltime;
	timeval		prevT;	

private:
	QString		name;
	QBoxLayout	*layout;
	QBoxLayout	*layoutl;
	QWidget		*lw;
	QGroupBox	*grp_time;
	QBoxLayout	*layoutt;
	SplitButton	*split;
	QFrame		*vline0;
//	bool		prevTvalid;
};

#endif

