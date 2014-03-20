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

#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPushButton>

#include <QKeyEvent>
#include <QFrame>

#include <images_list.h>

#include <pref_common.h>
#include <pref_devices.h>
#include <pref_colors.h>
#include <pref_reports.h>

#include "preferences.h"
#include <QDebug>

QPxPreferences::QPxPreferences(QPxSettings *iset, QWidget *p, Qt::WindowFlags f)
	: QDialog (p,f)
{
#ifndef QT_NO_DEBUG
	qDebug("QPxPreferences()");
#endif
	setWindowTitle("QPxTool - " + tr("Preferences"));
	set_old = iset;
	set = *iset;
	if (set_old->geometry_pref.width() > 0 && set_old->geometry_pref.height() > 0)
		setGeometry(set_old->geometry_pref);

	for (int i=0; i<PREF_PAGES; i++) pages[i] = NULL;
	curPage=-1;
	winit();
	setPage(0);
}

QPxPreferences::~QPxPreferences()
{
#ifndef QT_NO_DEBUG
	qDebug("~QPxPreferences()");
#endif
	set_old->geometry_pref = geometry();
}

void QPxPreferences::winit()
{
	layout = new QHBoxLayout();
	//layout = new QHBoxLayout();
	setLayout(layout);
	layout->setMargin(3);

	ilist  = new ImagesList(80,this);
	layout->addWidget(ilist);

	ilist->addLabel(tr("Common"),  QImage(":images/settings.png"));
	ilist->addLabel(tr("Devices"), QImage(":images/disc.png"));
	ilist->addLabel(tr("Colors"),  QImage(":images/colors.png"));
	ilist->addLabel(tr("Reports"), QImage(":images/document.png"));

	QObject::connect(ilist, SIGNAL(selected(int)), this, SLOT(setPage(int)));

	parea = new QVBoxLayout();
	layout->addLayout(parea);
//	parea->insertStretch(0,10);
//	parea->insertStretch(1,10);

	hline0 = new QFrame(this);
	hline0->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	parea->addWidget(hline0);

	layout_butt = new QHBoxLayout;
	layout_butt->setMargin(0);
	layout_butt->setSpacing(3);
	parea->addLayout(layout_butt);

	layout_butt->addStretch(10);

	pb_save   = new QPushButton( QIcon(":images/ok.png"), tr("Save"), this);
	pb_cancel = new QPushButton( QIcon(":images/x.png"), tr("Cancel"), this);
	pb_save->setMinimumWidth(100);
	pb_cancel->setMinimumWidth(100);

	layout_butt->addWidget(pb_save,1);
	layout_butt->addWidget(pb_cancel,1);

	connect( pb_save,   SIGNAL(clicked()), this, SLOT(save()) );
	connect( pb_cancel, SIGNAL(clicked()), this, SLOT(cancel()) );
//	playout = new QVBoxLayout;
//	playout->setMargin(0);
//	layout->addLayout(playout);
}

void QPxPreferences::setPage(int page)
{
#ifndef QT_NO_DEBUG
//	qDebug(QString("pageSelected: %1").arg(page));
#endif
	if (curPage == page) return;
	if (curPage >=0 && pages[curPage]) {

		pages[curPage]->hide();
	//	delete pages[curPage];
	//	pages[curPage] = NULL;
	}
/*
	if (pages[curPage]) {
		pages[curPage]->show();
		return;
	}
*/
	switch(page) {
		case 0:
			setWindowTitle("QPxTool - " + tr("Preferences") + ": " + tr("Common"));
			if (!pages[page]) pages[page] = new prefCommon(&set, this);
			break;
		case 1:
			setWindowTitle("QPxTool - " + tr("Preferences") + ": " + tr("Devices"));
			if (!pages[page]) pages[page] = new prefDevices(&set, this);
			break;
		case 2:
			setWindowTitle("QPxTool - " + tr("Preferences") + ": " + tr("Colors"));
			if (!pages[page]) pages[page] = new prefColors(&set, this);
			break;
		case 3:
			setWindowTitle("QPxTool - " + tr("Preferences") + ": " + tr("Reports"));
			if (!pages[page]) pages[page] = new prefReports(&set, this);
			break;
		default:
			break;
	}
	parea->insertWidget(0,pages[page]);
	pages[page]->show();
	curPage = page;
}

void QPxPreferences::save()
{
#ifndef QT_NO_DEBUG
	qDebug("QPxPreferences::save()");
#endif
	close();
	*set_old = set;
}

void QPxPreferences::cancel()
{
#ifndef QT_NO_DEBUG
	qDebug("QPxPreferences::cancel()");
#endif
	close();
}

void QPxPreferences::closeEvent(QCloseEvent* e)
{
#ifndef QT_NO_DEBUG
	qDebug("QPxPreferences::closeEvent()");
#endif
	for (int i=0; i<PREF_PAGES; i++)
		if (pages[i]) delete pages[i];
	QDialog::closeEvent(e);
}

void QPxPreferences::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_Escape) {
		close();
		e->accept();
	}
}

