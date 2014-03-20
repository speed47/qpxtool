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
#include <QButtonGroup>
#include <QRadioButton>
#include <QPainter>

#include <qpxgraph.h>
#include <device.h>
#include "tab_ta.h"
#include <QDebug>

#include <colorlabel.h>

tabTA::tabTA(QPxSettings *iset, devlist *idev, QString iname, QWidget *p, Qt::WindowFlags fl)
	: QWidget(p, fl)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabTA()");
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

	layoutl = new QVBoxLayout();
	layoutl->setMargin(0);
	layoutl->setSpacing(3);
	layout->addLayout(layoutl);

	layoutg = new QVBoxLayout();
	layoutg->setMargin(0);
	layoutg->setSpacing(3);
	layout->addLayout(layoutg);

// Layer selection
	box_layer = new QGroupBox(tr("Layer"),this);
	layoutl->addWidget(box_layer);
	lay_layer = new QVBoxLayout(box_layer);

	grp_layer = new QButtonGroup(box_layer);
	layer0 = new QRadioButton(tr("Layer")+" 0", box_layer); layer0->setChecked(true);
	lay_layer->addWidget(layer0);
	grp_layer->addButton(layer0, 0);
	layer1 = new QRadioButton(tr("Layer")+" 1", box_layer);
	lay_layer->addWidget(layer1);
	grp_layer->addButton(layer1, 1);

// Zone Selection
	box_zone = new QGroupBox(tr("Zone"),this);
	layoutl->addWidget(box_zone);
	lay_zone = new QVBoxLayout(box_zone);

	grp_zone = new QButtonGroup(box_zone);
	zone0 = new QRadioButton(tr("Inner"), box_zone); zone0->setChecked(true);
	lay_zone->addWidget(zone0);
	grp_zone->addButton(zone0, 0);
	zone1 = new QRadioButton(tr("Middle"), box_zone);
	lay_zone->addWidget(zone1);
	grp_zone->addButton(zone1, 1);
	zone2 = new QRadioButton(tr("Outer"), box_zone);
	lay_zone->addWidget(zone2);
	grp_zone->addButton(zone2, 2);

// Legend
#ifdef __LEGEND_SHOW_TA
	cl_pit = new ColorLabel(settings->col_tapit, tr("Pit"), 0, this);
	cl_pit->setMinimumSize(100,20);
	layoutl->addWidget(cl_pit);

	cl_land = new ColorLabel(settings->col_taland, tr("Land"), 0, this);
	cl_land->setMinimumSize(100,20);
	layoutl->addWidget(cl_land);
#endif

	layoutl->addStretch(10);

// Test time
	grp_time = new QGroupBox(tr("Time"), this);
	grp_time->setMinimumWidth(100);
	layoutl->addWidget(grp_time);
	
	layoutt = new QVBoxLayout(grp_time);
	layoutt->setMargin(3);
	layoutt->setSpacing(0);

	ltime = new QLabel("0:00", grp_time);
	ltime->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ltime->setMinimumHeight(22);
	QFont tfont = ltime->font();
	tfont.setFamily("Monospace");
	ltime->setFont( tfont );

	layoutt->addWidget(ltime);

// Graphs...
	graphPit = new QPxGraph(iset, idev, name, TEST_TA, this);
	graphPit->setModeTA(0);
	layoutg->addWidget(graphPit);

	graphLand = new QPxGraph(iset, idev, name, TEST_TA, this);
	graphLand->setModeTA(1);
	layoutg->addWidget(graphLand);

	connect(grp_layer, SIGNAL(buttonClicked(int)), graphPit,  SLOT(setLayerTA(int)));
	connect(grp_layer, SIGNAL(buttonClicked(int)), graphLand, SLOT(setLayerTA(int)));
	
	connect(grp_zone , SIGNAL(buttonClicked(int)), graphPit,  SLOT(setZoneTA(int)));
	connect(grp_zone , SIGNAL(buttonClicked(int)), graphLand, SLOT(setZoneTA(int)));

	clear();
#ifndef QT_NO_DEBUG
	qDebug("END: tabTA()");
#endif
}

tabTA::~tabTA()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~tabTA()");
	qDebug("END: ~tabTA()");
#endif
}

void tabTA::clear()
{
#ifndef QT_NO_DEBUG
	qDebug("tabTA::clear()");
#endif
}


void tabTA::selectDevice()
{
#ifndef QT_NO_DEBUG
	qDebug("tabTA::selectDevice()");
#endif
	device *dev = devices->current();
	graphPit->update();
	graphLand->update();
	QObject::connect( dev, SIGNAL(doneMInfo(int)), this, SLOT(updateAll()) );
	QObject::connect( dev, SIGNAL(block_TA()), this, SLOT(updateLast()) );
}

void tabTA::updateAll()
{
	if (devices->current()->media.layers.toInt() < 2) {
		layer0->setChecked(true);
		layer1->setEnabled(false);
	} else {
		layer1->setEnabled(true);
	}
	updateLast();
}

void tabTA::updateLast()
{
	graphPit->update();
	graphLand->update();
}

void tabTA::reconfig()
{
	graphPit->update();
	graphLand->update();
}

void tabTA::drawGraph(QImage& img, device *dev, int ttype, int eflags)
{
	int w = img.width();
	int h = img.height();
	QSize s(w,h);
	QRect r(0, 0, w, h);
	QPainter p(&img);

	if (eflags & 1) {
		graphLand->drawGraph(&p, s, dev, ttype, r, eflags & (~1), FORCE_REPAINT);
	} else {
		graphPit->drawGraph(&p, s, dev, ttype, r, eflags & (~1), FORCE_REPAINT);
	}
}

