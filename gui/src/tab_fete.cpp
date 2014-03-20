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
#include "tab_fete.h"
#include <QDebug>

tabFETE::tabFETE(QPxSettings *iset, devlist *idev, QString iname, QWidget *p, Qt::WindowFlags fl)
	: GraphTab(iset, idev, iname, TEST_FT, p, fl)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabFETE()");
#endif
	layout_info = new QVBoxLayout(infow);
	layout_info->setMargin(0);
	layout_info->setSpacing(3);

#ifdef __LEGEND_SHOW_SPEED
	pl_spd = new ColorLabel(QColor(Qt::black),"Speed", 0, infow);
	pl_spd->setMinimumSize(100,20);
	layout_info->addWidget(pl_spd);
#endif
	pl_fmax = new ColorLabel(settings->col_fe, tr("FE max"), 0, infow);
//	pl_fmax->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//	pl_fmax->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	pl_fmax->setMinimumSize(80,20);
	layout_info->addWidget(pl_fmax);
	l_fmax = new QLabel(infow);
	l_fmax->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_fmax->setMinimumSize(80,22);
	layout_info->addWidget(l_fmax);


	pl_tmax = new ColorLabel(settings->col_te,tr("TE max"), 0, infow);
//	pl_tmax->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//	pl_tmax->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	pl_tmax->setMinimumSize(80,20);
	layout_info->addWidget(pl_tmax);
	l_tmax = new QLabel(infow);
	l_tmax->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_tmax->setMinimumSize(80,22);
	layout_info->addWidget(l_tmax);


	layout_info->addStretch(10);

	clear();
#ifndef QT_NO_DEBUG
	qDebug("END: tabFETE()");
#endif
}

tabFETE::~tabFETE()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~tabFETE()");
	qDebug("END: ~tabFETE()");
#endif
}
/*
void tabFETE::clear()
{
	qDebug("tabFETE::clear()");

}
*/
void tabFETE::selectDevice()
{
#ifndef QT_NO_DEBUG
	qDebug("tabFETE::selectDevice()");
#endif
	device *dev = devices->current();
	GraphTab::updateLast((int)dev->testData.ft_time, NULL, 1);
	updateSummary();

	QObject::connect( dev, SIGNAL(doneMInfo(int)), this, SLOT(updateAll()) );
	QObject::connect( dev, SIGNAL(block_FT()), this, SLOT(updateLast()) );
}

void tabFETE::updateLast()
{
	bool show;
	device *dev = devices->current();
	GraphTab::updateLast((int)dev->testData.ft_time, &show);
	if (!show) return;
	updateSummary();
}

void tabFETE::updateAll()
{
	GraphTab::updateLast((int)devices->current()->testData.ft_time, NULL, 1);
	updateSummary();
}

void tabFETE::updateLegend()
{
	pl_fmax->setColor(settings->col_fe);
	pl_tmax->setColor(settings->col_te);
}

void tabFETE::updateGraph()
{
	graph->update();
}

void tabFETE::updateSummary()
{
	device *dev = devices->current();
	if (!dev->testData.jb.size()) {
		l_fmax->clear(); l_tmax->clear();
#ifdef FT_AVG
		l_favg->clear(); l_tavg->clear();
#endif
		return;
	}
// setting values...
	l_fmax->setText(QString::number(dev->testData.ftMAX.fe,'f',2));
	l_tmax->setText(QString::number(dev->testData.ftMAX.te,'f',2));
#ifdef FT_AVG
	l_favg->setText(QString::number(dev->testData.ftMAX.favg,'f',2));
	l_tavg->setText(QString::number(dev->testData.ftMAX.tavg,'f',2));
#endif
}

