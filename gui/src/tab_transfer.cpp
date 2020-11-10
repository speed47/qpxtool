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
#include "tab_transfer.h"
#include <QDebug>

tabTransfer::tabTransfer(QPxSettings *iset, devlist *idev, QString iname, bool irw,
		QWidget *p, Qt::WindowFlags fl)
	: GraphTab(iset, idev, iname, irw ? TEST_WT : TEST_RT, p, fl)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabTransfer()");
#endif
	rw = irw;
	layout_info = new QVBoxLayout(infow);
	layout_info->setMargin(0);
	layout_info->setSpacing(3);

	pl_sta = new QLabel(tr("Start"), infow);
	pl_sta->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	pl_sta->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout_info->addWidget(pl_sta);
	l_sta_x = new QLabel(infow);
	l_sta_x->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_sta_x->setMinimumHeight(22);
	layout_info->addWidget(l_sta_x);
	l_sta_kb = new QLabel(infow);
	l_sta_kb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_sta_kb->setMinimumHeight(22);
	layout_info->addWidget(l_sta_kb);

	pl_end = new QLabel(tr("End"), infow);
	pl_end->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	pl_end->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout_info->addWidget(pl_end);
	l_end_x = new QLabel(infow);
	l_end_x->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_end_x->setMinimumHeight(22);
	layout_info->addWidget(l_end_x);
	l_end_kb = new QLabel(infow);
	l_end_kb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_end_kb->setMinimumHeight(22);
	layout_info->addWidget(l_end_kb);

	pl_avg = new QLabel(tr("Average"), infow);
	pl_avg->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	pl_avg->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout_info->addWidget(pl_avg);
	l_avg_x = new QLabel(infow);
	l_avg_x->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_avg_x->setMinimumHeight(22);
	layout_info->addWidget(l_avg_x);
	l_avg_kb = new QLabel(infow);
	l_avg_kb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_avg_kb->setMinimumHeight(22);
	layout_info->addWidget(l_avg_kb);

#ifdef __LEGEND_SHOW_SPEED
	pl_spd = new ColorLabel(irw ? settings->col_wspeed : settings->col_rspeed,
			irw ? tr("Write") : tr("Read"),
			0, infow);
	pl_spd->setMinimumSize(100,20);
	layout_info->addWidget(pl_spd);
#endif

	layout_info->addStretch(10);

	clear();
#ifndef QT_NO_DEBUG
	qDebug("END: tabTransfer()");
#endif
}

tabTransfer::~tabTransfer()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~tabTransfer()");
	qDebug("END: ~tabTransfer()");
#endif
}

/*
void tabTransfer::clear()
{
	qDebug("tabTransfer::clear()");

}
*/

void tabTransfer::selectDevice()
{
#ifndef QT_NO_DEBUG
	qDebug("tabTransfer::selectDevice()");
#endif
	device *dev = devices->current();
	float  time = rw ? dev->testData.wt_time : dev->testData.rt_time;
//	graph->update();
	GraphTab::updateLast((int)time, NULL, 1);
	updateSummary(dev, time);

	QObject::connect( dev, SIGNAL(doneMInfo(int)), this, SLOT(updateLast()) );
	if (!rw) {
		QObject::connect( dev, SIGNAL(block_RT()), this, SLOT(updateLast()) );
	} else {
		QObject::connect( dev, SIGNAL(block_WT()), this, SLOT(updateLast()) );
	}
}

void tabTransfer::updateLast()
{
	bool show;
	device *dev = devices->current();
	float  time = rw ? dev->testData.wt_time : dev->testData.rt_time;
	GraphTab::updateLast((int)time, &show);
	if (!show) return;
	updateSummary(dev,time);
}

void tabTransfer::updateLegend()
{
#ifdef __LEGEND_SHOW_SPEED
	pl_spd->setColor(rw ? settings->col_wspeed : settings->col_rspeed);
#endif
}

void tabTransfer::updateGraph()
{
	graph->update();
}

void tabTransfer::updateSummary(device* dev, float time)
{
#ifndef QT_NO_DEBUG
	qDebug() << "tabTransfer::updateSummary(): " << (rw ? "WT" : "RT") << " device@" << dev;
#endif
	float    avg_x;
	uint32_t avg_kb;
	if (!(rw ? dev->testData.wt.size() : dev->testData.rt.size())) {
		l_sta_x->clear();
		l_sta_kb->clear();
		l_end_x->clear();
		l_end_kb->clear();
		l_avg_x->clear();
		l_avg_kb->clear();
		return;
	}
	if (!time) time=1;

	l_sta_x->setText( QString::number( rw ? dev->testData.wt.first().spdx : dev->testData.rt.first().spdx, 'f', 2 ) + " X");
	l_sta_kb->setText( QString::number( rw ? dev->testData.wt.first().spdk : dev->testData.rt.first().spdk) + " kB/s");

	l_end_x->setText( QString::number( rw ? dev->testData.wt.last().spdx  : dev->testData.rt.last().spdx,  'f', 2 ) + " X");
	l_end_kb->setText( QString::number( rw ? dev->testData.wt.last().spdk  : dev->testData.rt.last().spdk) + " kB/s");

	avg_kb = (int) ((rw ? dev->testData.wt.last().lba : dev->testData.rt.last().lba) * 2 / time);
	if (dev->media.spd1X)
		avg_x  = (float)avg_kb / dev->media.spd1X;
	else
		avg_x  = (float)avg_kb / 150.0;

#ifndef QT_NO_DEBUG
	printf("avg: %u KB, %.2f X, %d KB/X\n", avg_kb, avg_x, dev->media.spd1X);
#endif

	l_avg_x->setText( QString::number( avg_x, 'f', 2) + " X");
	l_avg_kb->setText( QString::number(avg_kb) + " kB/s");
}

