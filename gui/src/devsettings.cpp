	/*
	 * This file is part of the QPxTool project.
	 * Copyright (C) 2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
	 *
	 * This program is free software; you can redistribute it and/or modify
	 * it under the terms of the GNU General Public License as published by
	 * the Free Software Foundation; either version 2 of the License, or
	 * (at your option) any later version.
	 * See the file "COPYING" for the exact licensing terms.
	 *
	 */

#include "devsettings.h"

#include <QBoxLayout>
#include <QGridLayout>

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>

#include <images_list.h>

#include <textslider.h>
#include <device.h>
#include <qpxsettings.h>
#include <devsettings_widgets.h>

#include <QDebug>

#define TAB_COMMON		0
#define TAB_VARIREC		1
#define TAB_GIGAREC		2
#define TAB_SECUREC		3
#define TAB_SILENT_PLEX	4
#define TAB_SILENT_PIO	5
#define TAB_DESTRUCT	6
#define TAB_TATTOO		7

devSettings::devSettings(QPxSettings *iset, device *idev, QWidget *p, Qt::WindowFlags fl)
	: QDialog(p,fl)
{
	dev = idev;
	set = iset;
	setWindowTitle("QPxTool - "+tr("Device Controls"));
	cpage = NULL;

	layout = new QHBoxLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);
	setLayout(layout);

	ilist  = new ImagesList(80,32,32,this);
//	ilist->hideText();
	layout->addWidget(ilist,1);

	ilist->addLabel(tr("Common"),  QImage(":images/settings2.png"), TAB_COMMON);
	if (set->show_allctl || dev->features.supported & FEATURE_VARIREC)
		ilist->addLabel(tr("VariRec"), QImage(":images/varirec.png"),	TAB_VARIREC);
	if (set->show_allctl || dev->features.supported & FEATURE_GIGAREC)
		ilist->addLabel(tr("GigaRec"), QImage(":images/gigarec.png"),	TAB_GIGAREC);
	if (set->show_allctl || dev->features.supported & FEATURE_SECUREC)
		ilist->addLabel(tr("SecuRec"), QImage(":images/password.png"),	TAB_SECUREC);
	if (set->show_allctl || dev->features.supported & FEATURE_SILENT)
		ilist->addLabel(tr("Silent mode"), QImage(":images/sound.png"),	TAB_SILENT_PLEX);
	if (set->show_allctl || dev->features.supported & FEATURE_PIOQUIET)
		ilist->addLabel(tr("PioQuiet"), QImage(":images/sound.png"),	TAB_SILENT_PIO);
	if (set->show_allctl || dev->features.supported & FEATURE_DESTRUCT)
		ilist->addLabel(tr("Destruction"), QImage(":images/disc-eraser.png"), TAB_DESTRUCT);
	if (set->show_allctl || dev->features.supported & FEATURE_F1TATTOO)
		ilist->addLabel(tr("Disc T@2"), QImage(":images/tattoo.png"),	TAB_TATTOO);

	layoutc = new QVBoxLayout();
	layoutc->setMargin(0);
	layoutc->setSpacing(3);
	layout->addLayout(layoutc,4);

	setPage(0);

	connect(ilist, SIGNAL(selected(int)), this, SLOT(setPage(int)));

	setMinimumSize(450,380);
	setMaximumSize(450,580);
}

devSettings::~devSettings() {}

void devSettings::setPage(int page)
{
#ifndef QT_NO_DEBUG
	qDebug() << "setPage: " << page;
#endif
	if (cpage) {
		delete cpage;
		cpage = NULL;
	}
	switch (page) {
		case TAB_COMMON:
			cpage = new devctlCommon(dev, this);
			layoutc->addWidget(cpage);
			break;
		case TAB_VARIREC:
			cpage = new devctlVarirec(dev, this);
			layoutc->addWidget(cpage);
			break;
		case TAB_GIGAREC:
			cpage = new devctlGigarec(dev, this);
			layoutc->addWidget(cpage);
			break;
		case TAB_SECUREC:
			cpage = new devctlSecurec(dev, this);
			layoutc->addWidget(cpage);
			break;
		case TAB_SILENT_PLEX:
			cpage = new devctlSilent(dev, this);
			layoutc->addWidget(cpage);
			break;
		case TAB_SILENT_PIO:
			cpage = new devctlPioquiet(dev, this);
			layoutc->addWidget(cpage);
			break;
		case TAB_DESTRUCT:
			cpage = new devctlDestruct(dev, this);
			layoutc->addWidget(cpage);
			break;
		case TAB_TATTOO:
			cpage = new devctlF1Tattoo(dev, this);
			layoutc->addWidget(cpage);
			break;
		default:
			qDebug() << "Invalid page num: " << page;
	}
}

