/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009-2012 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <QGridLayout>
//#include <QStringList>
#include <QLabel>
#include <QFrame>

#include <device.h>
#include <qpxgraph.h>

#include "errc_detailed.h"
#include <QDebug>


static const char* labels_cd[8] = {
	"BLER",
	"E11",
	"E21",
	"E31",
	"E12",
	"E22",
	"E32",
	"UNCR"
};

static const char* labels_dvd[8] = {
	"",
	"PIE",
	"PI8",
	"PIF",
	"POE",
	"PO8",
	"POF",
	"UNCR"
};

static const char* labels_bd[8] = {
	"",
	"LDC",
	"",
	"",
	"BIS",
	"",
	"",
	"UNCR"
};

static const char* labels_null[8] = { "", "", "", "", "", "", "", "" };

ErrcDetailedDialog::ErrcDetailedDialog(QPxSettings *iset, devlist *idev, QWidget *p, Qt::WindowFlags fl)
	: QDialog(p,fl)
{
#ifndef QT_NO_DEBUG
	qDebug("ErrcDetailedDialog()");
#endif
	devices  = idev;
	device *dev = devices->current();
	settings = iset;

	if (dev->media.type.startsWith("CD-")) {
		labels = labels_cd;
	} else if (dev->media.type.startsWith("DVD")) {
		labels = labels_dvd;
	} else if (dev->media.type.startsWith("BD-")) {
		labels = labels_bd;
	} else {
		labels = labels_null;
	}

	setWindowTitle("QPxTool - " + tr("Detailed Error Correction"));

	layout = new QGridLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);
	layout_top = new QGridLayout();
	layout_top->setMargin(0);
	layout_top->setSpacing(3);
	layout->addLayout(layout_top, 0, 0, 1, 3);

	settings->loadScale("XERRC");

	graph[0] = new QPxGraph(iset, idev, "XERRC", TEST_ERRC, this);
	layout_top->addWidget(graph[0], 0, 1);
	graph[7] = new QPxGraph(iset, idev, "XERRC", TEST_ERRC, this);
	layout_top->addWidget(graph[7], 0, 2);

	for (int i=0; i<6; i++) {
		graph[i+1] = new QPxGraph(iset, idev, "XERRC", TEST_ERRC, this);
		layout->addWidget(graph[i+1], i/3+1, i%3);
	}

	for (int i=0; i<8; i++) {
		graph[i]->setErrcList(1<<i, labels[i]);
		graph[i]->setShowSpeed(0);
		graph[i]->setRightMarginHidden(true);
	}

	layout_summary = new QGridLayout();
	layout_summary->setMargin(0);
	layout_summary->setHorizontalSpacing(3);
	layout_summary->setVerticalSpacing(1);
	layout_top->addLayout(layout_summary, 0, 0);

	layout_top->setColumnStretch(0,1);
	layout_top->setColumnStretch(1,1);
	layout_top->setColumnStretch(2,1);

	pl_tot = new QLabel("Tot",this);
	pl_tot->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_summary->addWidget(pl_tot,0,1, 1,2);
	pl_max = new QLabel("Max",this);
	pl_max->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_summary->addWidget(pl_max,0,3);
	pl_avg = new QLabel("Avg",this);
	pl_avg->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_summary->addWidget(pl_avg,0,4);

	hline0 = new QFrame(this);
	hline0->setFrameStyle(QFrame::Sunken | QFrame::HLine);
	layout_summary->addWidget(hline0, 1,0, 1,5);

	for (int i=0; i<8; i++) {
		pl_name[i] = new QLabel(labels[i], this);
		pl_name[i]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		layout_summary->addWidget(pl_name[i], i+2, 0);
		l_tot[i] = new QLabel(this);
		l_tot[i]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		layout_summary->addWidget(l_tot[i], i+2, 1, 1,2);
		l_max[i] = new QLabel(this);
		l_max[i]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		layout_summary->addWidget(l_max[i], i+2, 3);
		l_avg[i] = new QLabel(this);
		l_avg[i]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		layout_summary->addWidget(l_avg[i], i+2, 4);
	}

	hline1 = new QFrame(this);
	hline1->setFrameStyle(QFrame::Sunken | QFrame::HLine);
	layout_summary->addWidget(hline1, 10,0, 1,5);

	layout_summary->setRowStretch(11,10);

	for (int i=0; i<8; i++)
		connect(graph[i], SIGNAL(scaleChanged()), this, SLOT(changeScale()));
}

ErrcDetailedDialog::~ErrcDetailedDialog()
{
#ifndef QT_NO_DEBUG
	qDebug("~ErrcDetailedDialog()");
#endif
}

void ErrcDetailedDialog::changeScale()
{
	QObject *sgraph = sender();
	for (int i=0; i<8; i++) {
		if (sgraph != graph[i]) graph[i]->changeScale();
	}
}

void ErrcDetailedDialog::updateAll()
{
	device *dev = devices->current();
	if (dev->media.type.startsWith("CD-")) {
		labels = labels_cd;
	} else if (dev->media.type.startsWith("DVD")) {
		labels = labels_dvd;
	} else if (dev->media.type.startsWith("BD-")) {
		labels = labels_bd;
	} else {
		labels = labels_null;
	}

	for (int i=0; i<8; i++) {
		pl_name[i]->setText(labels[i]);
		graph[i]->setErrcList(1<<i, labels[i]);
	}

	updateGraphs(dev);
}

void ErrcDetailedDialog::updateGraphs(device *idev)
{
	device *dev;
	if (!idev) {
		dev = devices->current();
	} else {
		dev = idev;
	}

	graph[0]->setVisible(labels == labels_cd);
	graph[2]->setVisible(labels != labels_bd);
	graph[3]->setVisible(labels != labels_bd);
	graph[5]->setVisible(labels != labels_bd);
	graph[6]->setVisible(labels != labels_bd);
	for (int i=0; i<8; i++) {
//		if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<i))) {
			l_tot[i]->setText( QString::number(dev->testData.errcTOT.raw.err[i]) );
			l_max[i]->setNum( dev->testData.errcMAX.raw.err[i] );
			l_avg[i]->setText( QString::number(dev->testData.errcAVG.raw.err[i], 'f', 2) );
		//	graph[i]->setErrcList(1<<i, labels[i]);
			graph[i]->update();
//		} else {
//			graph[i]->hide();
//		}
	}
}

void ErrcDetailedDialog::hideEvent(QHideEvent* e)
{
	emit closed();
	QDialog::hideEvent(e);
}

