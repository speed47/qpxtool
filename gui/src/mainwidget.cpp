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

#include <QStackedLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QFrame>

//#define bstyle "QPushButton::flat { border: none; }"
#define bstyle "QPushButton::flat { text-align: left; }"

#define addTabButton(pb,icon,name,tabidx,layout) \
{ \
	pb = new QPushButton(QIcon(icon),name,bframe); \
	pb->setStyleSheet(bstyle); \
	pb->setFlat(true); \
	pb->setCheckable(true); \
	pb->setIconSize(QSize(24,24)); \
	pb->setMinimumHeight(26); \
	pb->setFocusPolicy(Qt::NoFocus); \
	QFont f = pb->font(); \
	f.setPointSize(f.pointSize()+1); \
	pb->setFont(f); \
	grp->addButton(pb,tabidx++); \
	layout->addWidget(pb); \
}

#include <QIcon>

#include <device.h>

#include <tab_devinfo.h>
#include <tab_mediainfo.h>
#include <tab_transfer.h>
#include <tab_errc.h>
#include <tab_jb.h>
#include <tab_fete.h>
#include <tab_ta.h>

#include "mainwidget.h"

#include <QDebug>

QPxMainWidget::QPxMainWidget(QPxSettings *iset, devlist *idev, QWidget *p)
	: QWidget(p)
{
	int tabidx=0;
#ifndef QT_NO_DEBUG
	qDebug("STA: QPxMainWidget()");
#endif

	settings = iset;
	devices = idev;

	QPushButton *pb;
	layout = new QHBoxLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);

	bframe = new QFrame(this);
	bframe->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout->addWidget(bframe);

	layout_buttons = new QVBoxLayout(bframe);
	layout_buttons->setMargin(3);
	layout_buttons->setSpacing(0);
//	layout->addLayout(layout_buttons);

	grp = new QButtonGroup(bframe);

	stack = new QStackedLayout();
	layout->addLayout(stack);

	tab_DevInfo = new tabDevInfo(iset, idev, this);
	stack->addWidget(tab_DevInfo);
	addTabButton(pb,":images/device.png", "Device", tabidx,layout_buttons);
	pb->setChecked(true);
	connect(this, SIGNAL(deviceSelected()), tab_DevInfo, SLOT(selectDevice()));

	tab_MediaInfo = new tabMediaInfo(iset, idev, this);
	stack->addWidget(tab_MediaInfo);
	addTabButton(pb,":images/disc.png", "Media", tabidx,layout_buttons);
	connect(this, SIGNAL(deviceSelected()), tab_MediaInfo, SLOT(selectDevice()));

	tab_RT = new tabTransfer(iset, idev, "RT", 0, this);
	stack->addWidget(tab_RT);
	addTabButton(pb,":images/test_rt.png", "Read Transfer", tabidx,layout_buttons);
	connect(this, SIGNAL(configured()), tab_RT, SLOT(reconfig()));
	connect(this, SIGNAL(deviceSelected()), tab_RT, SLOT(selectDevice()));

	tab_WT = new tabTransfer(iset, idev, "WT", 1, this);
	stack->addWidget(tab_WT);
	addTabButton(pb,":images/test_wt.png", "Write Transfer", tabidx,layout_buttons);
	connect(this, SIGNAL(configured()), tab_WT, SLOT(reconfig()));
	connect(this, SIGNAL(deviceSelected()), tab_WT, SLOT(selectDevice()));

	tab_ERRC = new tabERRC(iset, idev, "ERRC", this);
	stack->addWidget(tab_ERRC);
	addTabButton(pb,":images/test_errc.png", "Error Correction", tabidx,layout_buttons);
	connect(this, SIGNAL(configured()), tab_ERRC, SLOT(reconfig()));
	connect(this, SIGNAL(deviceSelected()), tab_ERRC, SLOT(selectDevice()));

	tab_JB = new tabJB(iset, idev, "JB", this);
	stack->addWidget(tab_JB);
	addTabButton(pb,":images/test_jb.png", "Jitter/Asymmetry", tabidx,layout_buttons);
	connect(this, SIGNAL(configured()), tab_JB, SLOT(reconfig()));
	connect(this, SIGNAL(deviceSelected()), tab_JB, SLOT(selectDevice()));

	tab_FETE = new tabFETE(iset, idev, "FETE", this);
	stack->addWidget(tab_FETE);
	addTabButton(pb,":images/test_ft.png", "FE/TE", tabidx,layout_buttons);
	connect(this, SIGNAL(configured()), tab_FETE, SLOT(reconfig()));
	connect(this, SIGNAL(deviceSelected()), tab_FETE, SLOT(selectDevice()));

	tab_TA = new tabTA(iset, idev, "TA", this);
	stack->addWidget(tab_TA);
	addTabButton(pb,":images/test_ta.png", "Time Analyser", tabidx,layout_buttons);
	connect(this, SIGNAL(configured()), tab_TA, SLOT(reconfig()));
	connect(this, SIGNAL(deviceSelected()), tab_TA, SLOT(selectDevice()));

	connect(grp, SIGNAL(buttonClicked(int)), stack, SLOT(setCurrentIndex(int)));
	layout_buttons->addStretch(10);


	bframe->setVisible(settings->show_sidebar);
#ifndef QT_NO_DEBUG
	qDebug("END: QPxMainWidget()");
#endif
}

QPxMainWidget::~QPxMainWidget()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~QPxMainWidget()");
	qDebug("END: ~QPxMainWidget()");
#endif
}

void QPxMainWidget::setSidebarVisible(bool en) { settings->show_sidebar = en; bframe->setVisible(en); }
void QPxMainWidget::selectTab(int idx) { grp->button(idx)->setChecked(true); stack->setCurrentIndex(idx); }
void QPxMainWidget::reconfig() { emit configured(); }
void QPxMainWidget::clearDev() { tab_MediaInfo->clear(); tab_DevInfo->clear(); }
void QPxMainWidget::clearMedia() { tab_MediaInfo->clear(); }
void QPxMainWidget::selectDevice() { emit deviceSelected(); }

void QPxMainWidget::drawGraph(QImage& img, device *dev, int ttype, int eflags)
{
	switch(ttype) {
		case TEST_RT:
			tab_RT->drawGraph(img, dev, TEST_RT);
			break;
		case TEST_WT:
			tab_WT->drawGraph(img, dev, TEST_WT);
			break;
		case TEST_ERRC:
			tab_ERRC->drawGraph(img, dev, TEST_ERRC, eflags);
			break;
		case TEST_JB:
			tab_JB->drawGraph(img, dev, TEST_JB);
			break;
		case TEST_FT:
			tab_FETE->drawGraph(img, dev, TEST_FT);
			break;
		case TEST_TA:
			tab_TA->drawGraph(img, dev, TEST_TA, eflags);
			break;
	}
}

