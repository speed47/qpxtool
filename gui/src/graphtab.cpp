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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPainter>

#include <splitbutton.h>
#include "graphtab.h"

#include <qpxsettings.h>
#include <device.h>
#include <qpxgraph.h>

#include <QDebug>

GraphTab::GraphTab(QPxSettings *iset, devlist *idev, QString iname, int test, QWidget *p, Qt::WindowFlags fl)
	: QWidget(p,fl)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: GraphTab()");
#endif
	settings = iset;
	devices = idev;
	name    = iname;
//	prevTvalid = 0;
	gettimeofday(&prevT, NULL);
	settings->loadScale(name);

	layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(3);

	split = new SplitButton(Qt::Vertical, this);
	layout->addWidget(split);
	connect(split, SIGNAL(clicked()), this, SLOT(infoToggle()));

	lw = new QWidget(this);
	lw->setMinimumWidth(80);
	layout->addWidget(lw);

	layoutl = new QVBoxLayout(lw);
	layoutl->setMargin(0);
	layoutl->setSpacing(3);
//	layout->addLayout(layoutl);

	infow = new QWidget(lw);
	infow->setMinimumWidth(80);
	layoutl->addWidget(infow,20);
//	layoutl->addStretch(1);

	grp_time = new QGroupBox(tr("Time"), lw);
	grp_time->setMinimumWidth(100);
	layoutl->addWidget(grp_time,1);
	
	layoutt = new QVBoxLayout(grp_time);
	layoutt->setMargin(3);
	layoutt->setSpacing(0);

	ltime = new QLabel(grp_time);
	ltime->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ltime->setMinimumHeight(22);
	QFont tfont = ltime->font();
	tfont.setFamily("Monospace");
	ltime->setFont( tfont );

	layoutt->addWidget(ltime);

	vline0 = new QFrame(this);
	vline0->setFrameStyle(QFrame::VLine | QFrame::Sunken);
	layout->addWidget(vline0);

	graph = new QPxGraph(iset, idev, name, test, this);
//	graph->setDataNames(QStringList() << "speed_rt" << "speed_wt");
	layout->addWidget(graph);

	clear();
#ifndef QT_NO_DEBUG
	qDebug("END: GraphTab()");
#endif
}

GraphTab::~GraphTab() {}

void GraphTab::clear() {}

void GraphTab::infoToggle() { lw->setVisible(!lw->isVisible()); }

void GraphTab::updateLast(int time, bool *Tvalid, bool force)
{
	timeval curT;
	float dt;
	gettimeofday(&curT, NULL);

//	if (prevTvalid)
	dt = curT.tv_sec - prevT.tv_sec + (curT.tv_usec - prevT.tv_usec) / 1000000.0;
//	if (!prevTvalid || dt>0.5) {
	if (force || dt>0.5) {
		int s = time % 60;
		int m = (time - s) / 60;
		ltime->setText(QString("%1:%2").arg(m).arg(s, 2,10, QChar('0')));	

		gettimeofday(&prevT, NULL);
	//	prevTvalid = 1;
		if (force) {
			graph->update();
		} else {
			int x = graph->getLastX();
			graph->update(x, 0, graph->width()-x, graph->height());
		}
		if (Tvalid) *Tvalid=1;
		return;
	}
	if (Tvalid) *Tvalid=0;
}

void GraphTab::drawGraph(QImage& img, device *dev, int ttype, int eflags)
{
	int w = img.width();
	int h = img.height();
	QSize s(w,h);
	QRect r(0, 0, w, h);
	QPainter p(&img);

	graph->drawGraph(&p, s, dev, ttype, r, (ttype == TEST_ERRC) ? eflags : 0, FORCE_REPAINT);
}

