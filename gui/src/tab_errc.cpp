/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2012 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

#include <colorlabel.h>
#include <qpxgraph.h>
#include <errc_detailed.h>
#include "tab_errc.h"
#include <QDebug>

tabERRC::tabERRC(QPxSettings *iset, devlist *idev, QString iname, QWidget *p, Qt::WindowFlags fl)
	: GraphTab(iset, idev, iname, TEST_ERRC, p, fl)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabERRC()");
#endif
	xerrc = NULL;

	layout_info = new QVBoxLayout(infow);
	layout_info->setMargin(0);
	layout_info->setSpacing(3);

#ifdef __LEGEND_SHOW_SPEED
	pl_spd = new ColorLabel(QColor(Qt::black),"Speed", 0, infow);
	pl_spd->setMinimumSize(100,20);
	layout_info->addWidget(pl_spd);
#endif
	pl_e0 = new ColorLabel(QColor(Qt::black),"BLER/PI8", 0, infow);
//	pl_e0->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//	pl_e0->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout_info->addWidget(pl_e0);

	l_e0t = new QLabel(infow);
	l_e0t->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_e0t->setMinimumHeight(22);
	layout_info->addWidget(l_e0t);

	l_e0m = new QLabel(infow);
	l_e0m->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_e0m->setMinimumHeight(22);
	layout_info->addWidget(l_e0m);

	l_e0a = new QLabel(infow);
	l_e0a->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_e0a->setMinimumHeight(22);
	layout_info->addWidget(l_e0a);


	pl_e1 = new ColorLabel(QColor(Qt::black),"E22/PIF", 0, infow);
//	pl_e1->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//	pl_e1->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout_info->addWidget(pl_e1);

	l_e1t = new QLabel(infow);
	l_e1t->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_e1t->setMinimumHeight(22);
	layout_info->addWidget(l_e1t);

	l_e1m = new QLabel(infow);
	l_e1m->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_e1m->setMinimumHeight(22);
	layout_info->addWidget(l_e1m);

	l_e1a = new QLabel(infow);
	l_e1a->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_e1a->setMinimumHeight(22);
	layout_info->addWidget(l_e1a);


	pl_e2 = new ColorLabel(QColor(Qt::black),"E32/POF", 0, infow);
//	pl_e2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//	pl_e2->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout_info->addWidget(pl_e2);

	l_e2t = new QLabel(infow);
	l_e2t->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_e2t->setMinimumHeight(22);
	layout_info->addWidget(l_e2t);

	l_e2m = new QLabel(infow);
	l_e2m->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_e2m->setMinimumHeight(22);
	layout_info->addWidget(l_e2m);

	l_e2a = new QLabel(infow);
	l_e2a->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_e2a->setMinimumHeight(22);
	layout_info->addWidget(l_e2a);

	layout_info->addStretch(10);

	pb_xerrc = new QPushButton("Detailed");
	layout_info->addWidget(pb_xerrc);

	QObject::connect(pb_xerrc, SIGNAL(clicked()), this, SLOT(toggleXErrc()));

	clear();
#ifndef QT_NO_DEBUG
	qDebug("END: tabERRC()");
#endif
}

tabERRC::~tabERRC()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~tabERRC()");
	qDebug("END: ~tabERRC()");
#endif
}
/*
void tabERRC::clear()
{
	qDebug("tabERRC::clear()");
}
*/
void tabERRC::selectDevice()
{
#ifndef QT_NO_DEBUG
	qDebug("tabERRC::selectDevice()");
#endif
	device *dev = devices->current();
	updateAll();
//	GraphTab::updateLast((int)time, NULL, 1);
//	updateSummary(dev);

	//	QObject::connect( devices->current(), SIGNAL(block_ERRC()), this, SLOT(update()) );
	QObject::connect( dev, SIGNAL(doneMInfo(int)), this, SLOT(updateAll()) );
	QObject::connect( dev, SIGNAL(block_ERRC()), this, SLOT(updateLast()) );
}

void tabERRC::updateAll()
{
	bool show_e2 = true;
	device *dev = devices->current();
	GraphTab::updateLast((int)(dev->testData.errc_time), NULL, 1);
	updateSummary(dev);
	if (dev->media.type.startsWith("CD")) {
		pl_e0->setText("BLER");
		pl_e1->setText("E22");
		pl_e2->setText("E32");
	} else if (dev->media.type.startsWith("DVD")) {
		pl_e0->setText("PI8");
		pl_e1->setText("PIF");
		pl_e2->setText("POF");	
	} else if (dev->media.type.startsWith("BD")) {
		pl_e0->setText("LDC");
		pl_e1->setText("BIS");
		show_e2 = false;
	} else {
		pl_e0->setText("BLER/PI8/LDC");
		pl_e1->setText("E22/PIF/BIS");
		pl_e2->setText("E32/POF/---");
	}
	pl_e2->setVisible(show_e2);
	l_e2t->setVisible(show_e2);
	l_e2m->setVisible(show_e2);
	l_e2a->setVisible(show_e2);

	updateLegend();
	if (xerrc) xerrc->updateAll();
}

void tabERRC::updateLegend()
{
	device *dev = devices->current();
	if (dev->media.type.startsWith("CD")) {
		pl_e0->setColor( *(settings->col_errc.cd.bler));
		pl_e1->setColor( *(settings->col_errc.cd.e22));
		pl_e2->setColor( *(settings->col_errc.cd.e32));
	} else if (dev->media.type.startsWith("DVD")) {
		pl_e0->setColor( *(settings->col_errc.dvd.pi8));
		pl_e1->setColor( *(settings->col_errc.dvd.pif));
		pl_e2->setColor( *(settings->col_errc.dvd.pof));
	} else if (dev->media.type.startsWith("BD")) {
		pl_e0->setColor( *(settings->col_errc.bd.ldc));
		pl_e1->setColor( *(settings->col_errc.bd.bis));
	} else {
		pl_e0->setColor( QColor(Qt::black) ); 
		pl_e1->setColor( QColor(Qt::black) ); 
		pl_e2->setColor( QColor(Qt::black) ); 
	}
}

void tabERRC::updateGraph()
{
	graph->update();
}

void tabERRC::updateLast()
{
	bool show;
	device *dev = devices->current();
	int  time = (int) (dev->testData.errc_time);
	GraphTab::updateLast(time, &show);
	if (!show) return;
	updateSummary(dev);

	if (xerrc && xerrc->isVisible()) xerrc->updateAll();
}

void tabERRC::updateSummary(device *dev)
{
	if (!dev->testData.errc.size()) {
		l_e0t->clear(); l_e0m->clear(); l_e0a->clear(); 
		l_e1t->clear(); l_e1m->clear(); l_e1a->clear(); 
		l_e2t->clear(); l_e2m->clear(); l_e2a->clear(); 
		return;
	}
	if (dev->media.type.startsWith("CD")) {
		l_e0t->setText( "Total: " + QString::number(dev->testData.errcTOT.cd.bler) );
		l_e0m->setText( "Max: "   + QString::number(dev->testData.errcMAX.cd.bler) );
		l_e0a->setText( "Avg: "   + QString::number(dev->testData.errcAVG.cd.bler, 'f', 2) );

		l_e1t->setText( "Total: " + QString::number(dev->testData.errcTOT.cd.e22) );
		l_e1m->setText( "Max: "   + QString::number(dev->testData.errcMAX.cd.e22) );
		l_e1a->setText( "Avg: "   + QString::number(dev->testData.errcAVG.cd.e22, 'f', 2) );

		l_e2t->setText( "Total: " + QString::number(dev->testData.errcTOT.cd.e32) );
		l_e2m->setText( "Max: "   + QString::number(dev->testData.errcMAX.cd.e32) );
		l_e2a->setText( "Avg: "   + QString::number(dev->testData.errcAVG.cd.e32, 'f', 2) );
	} else if (dev->media.type.startsWith("DVD")) {
		l_e0t->setText( "Total: " + QString::number(dev->testData.errcTOT.dvd.pi8) );
		l_e0m->setText( "Max: "   + QString::number(dev->testData.errcMAX.dvd.pi8) );
		l_e0a->setText( "Avg: "   + QString::number(dev->testData.errcAVG.dvd.pi8, 'f', 2) );

		l_e1t->setText( "Total: " + QString::number(dev->testData.errcTOT.dvd.pif) );
		l_e1m->setText( "Max: "   + QString::number(dev->testData.errcMAX.dvd.pif) );
		l_e1a->setText( "Avg: "   + QString::number(dev->testData.errcAVG.dvd.pif, 'f', 2) );

		l_e2t->setText( "Total: " + QString::number(dev->testData.errcTOT.dvd.pof) );
		l_e2m->setText( "Max: "   + QString::number(dev->testData.errcMAX.dvd.pof) );
		l_e2a->setText( "Avg: "   + QString::number(dev->testData.errcAVG.dvd.pof, 'f', 2) );
	} else if (dev->media.type.startsWith("BD")) {
		l_e0t->setText( "Total: " + QString::number(dev->testData.errcTOT.bd.ldc) );
		l_e0m->setText( "Max: "   + QString::number(dev->testData.errcMAX.bd.ldc) );
		l_e0a->setText( "Avg: "   + QString::number(dev->testData.errcAVG.bd.ldc, 'f', 2) );

		l_e1t->setText( "Total: " + QString::number(dev->testData.errcTOT.bd.bis) );
		l_e1m->setText( "Max: "   + QString::number(dev->testData.errcMAX.bd.bis) );
		l_e1a->setText( "Avg: "   + QString::number(dev->testData.errcAVG.bd.bis, 'f', 2) );
	}
}

void tabERRC::toggleXErrc()
{
	if (!xerrc) {
		xerrc = new ErrcDetailedDialog(settings,devices,this);
		QObject::connect(xerrc, SIGNAL(closed()), this, SLOT(XerrcClosed()));
		xerrc->show();
		xerrc->updateAll();
	} else {
		xerrc->hide();
		delete xerrc;
		xerrc = NULL;
	}
}

void tabERRC::XerrcClosed()
{
	xerrc->disconnect();
	xerrc->deleteLater();
	xerrc = NULL;
}

