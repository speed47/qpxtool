/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2007-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef _MAINWIDGET_H
#define _MAINWIDGET_H

#include <QWidget>
class QBoxLayout;
class QStackedLayout;
class QFrame;

class QAbstractButton;
class QPushButton;
class QButtonGroup;

#include <qpxsettings.h>
#include <device.h>

class QIODevice;

class tabDevInfo;
class tabMediaInfo;
class tabTransfer;
class tabERRC;
class tabJB;
class tabFETE;
class tabTA;
class QPxGraph;

class QPxMainWidget : public QWidget
{
	Q_OBJECT
public:
	QPxMainWidget(QPxSettings *iset, devlist *idev, QWidget *p=0);
	~QPxMainWidget();
	void drawGraph(QImage& img, device *dev, int ttype, int eflags=0);

public slots:
	void clearDev();
	void clearMedia();
	void selectDevice();
	void reconfig();
	void setSidebarVisible(bool);
	void selectTab(int);
	//inline void reconfig() { emit configured(); };

private slots:

signals:
	void deviceSelected();
	void configured();

private:
	devlist			*devices;
	QPxSettings		*settings;

	QBoxLayout		*layout;
	QFrame			*bframe;
	QBoxLayout		*layout_buttons;
	QButtonGroup	*grp;
	QStackedLayout	*stack;

	tabDevInfo		*tab_DevInfo;
	tabMediaInfo	*tab_MediaInfo;
	tabTransfer		*tab_RT;
	tabTransfer		*tab_WT;
	tabERRC			*tab_ERRC;
	tabJB			*tab_JB;
	tabFETE			*tab_FETE;
	tabTA			*tab_TA;
};

#endif

