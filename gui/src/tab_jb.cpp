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

#include <QVBoxLayout>
#include <QLabel>

#include <colorlabel.h>
#include <qpxgraph.h>
#include <device.h>
#include "tab_jb.h"
#include <QDebug>

tabJB::tabJB(QPxSettings *iset, devlist *idev, QString iname, QWidget *p, Qt::WindowFlags fl)
	: GraphTab(iset, idev, iname, TEST_JB, p, fl)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabJB()");
#endif
	layout_info = new QVBoxLayout(infow);
	layout_info->setMargin(0);
	layout_info->setSpacing(3);

#ifdef __LEGEND_SHOW_SPEED
	pl_spd = new ColorLabel(QColor(Qt::black),"Speed", 0, infow);
	pl_spd->setMinimumSize(100,20);
	layout_info->addWidget(pl_spd);
#endif
	pl_jitter = new ColorLabel(settings->col_jitter, tr("Jitter"), 0, infow);
//	pl_jitter->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//	pl_jitter->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	pl_jitter->setMinimumSize(100,20);
	layout_info->addWidget(pl_jitter);

	l_jmax = new QLabel(infow);
	l_jmax->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_jmax->setMinimumSize(80,22);
	layout_info->addWidget(l_jmax);
	
	l_jmin = new QLabel(infow);
	l_jmin->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_jmin->setMinimumSize(80,22);
	layout_info->addWidget(l_jmin);

	pl_asymm = new ColorLabel(settings->col_asymm, tr("Asymmetry"), 0, infow);
//	pl_asymm->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//	pl_asymm->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	pl_asymm->setMinimumSize(100,20);
	layout_info->addWidget(pl_asymm);

	l_amax = new QLabel(infow);
	l_amax->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_amax->setMinimumSize(80,22);
	layout_info->addWidget(l_amax);
	
	l_amin = new QLabel(infow);
	l_amin->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_amin->setMinimumSize(80,22);
	layout_info->addWidget(l_amin);
	
	layout_info->addStretch(10);

	clear();
#ifndef QT_NO_DEBUG
	qDebug("END: tabJB()");
#endif
}

tabJB::~tabJB()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~tabJB()");
	qDebug("END: ~tabJB()");
#endif
}
/*
void tabJB::clear()
{
	qDebug("tabJB::clear()");
}
*/
void tabJB::selectDevice()
{
#ifndef QT_NO_DEBUG
	qDebug("tabJB::selectDevice()");
#endif
	device *dev = devices->current();
	GraphTab::updateLast((int)dev->testData.jb_time, NULL, 1);
	updateSummary();

	QObject::connect( dev, SIGNAL(doneMInfo(int)), this, SLOT(updateLast()) );
	QObject::connect( dev, SIGNAL(block_JB()), this, SLOT(updateLast()) );
}
void tabJB::updateLast()
{
	bool show;
	device *dev = devices->current();
	GraphTab::updateLast((int)dev->testData.jb_time, &show);
	if (!show) return;
	updateSummary();
}

void tabJB::updateAll()
{
	GraphTab::updateLast((int)devices->current()->testData.jb_time, NULL, 1);
	updateSummary();
}

void tabJB::updateLegend()
{
	pl_jitter->setColor(settings->col_jitter);
	pl_asymm->setColor(settings->col_asymm);
}

void tabJB::updateGraph()
{
	graph->update();
}

void tabJB::updateSummary()
{
	device *dev = devices->current();
	if (!dev->testData.jb.size()) {
		l_jmax->clear(); l_jmin->clear();
		l_amax->clear(); l_amin->clear();
		return;
	}
// setting values...
	l_jmax->setText(QString::number(dev->testData.jbMM.jmax,'f',2));
	l_jmin->setText(QString::number(dev->testData.jbMM.jmin,'f',2));

	l_amax->setText(QString::number(dev->testData.jbMM.bmax,'f',2));
	l_amin->setText(QString::number(dev->testData.jbMM.bmin,'f',2));
}

