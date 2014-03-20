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

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QFont>
#include <QLabel>
#include <QFrame>
#include <QHeaderView>
#include <QTreeWidget>
#include <QMessageBox>

#include "tab_mediainfo.h"
#include <QDebug>

tabMediaInfo::tabMediaInfo(QPxSettings *iset, devlist *idev, QWidget *p, Qt::WindowFlags fl)
	: QWidget(p,fl)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabMediaInfo()");
#endif
	settings = iset;
	devices = idev;

	layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(3);

	layout_info = new QGridLayout;
	layout_info->setVerticalSpacing(1);
	layout_info->setHorizontalSpacing(6);
	layout->addLayout(layout_info);

	pl_type = new QLabel(tr("Type:"), this);
	layout_info->addWidget(pl_type,0,0);
	l_type = new QLabel(this);
	layout_info->addWidget(l_type,0,1);
	pl_book = new QLabel(tr("Disc Category:"), this);
	layout_info->addWidget(pl_book,1,0);
	l_book = new QLabel(this);
	layout_info->addWidget(l_book,1,1);
	pl_mid = new QLabel(tr("Media ID:"), this);
	layout_info->addWidget(pl_mid,2,0);
	l_mid = new QLabel(this);
	layout_info->addWidget(l_mid,2,1);
	pl_layers = new QLabel(tr("Layers:"), this);
	layout_info->addWidget(pl_layers,3,0);
	l_layers = new QLabel(this);
	layout_info->addWidget(l_layers,3,1);
	pl_erasable = new QLabel(tr("Erasable:"), this);
	layout_info->addWidget(pl_erasable,4,0);
	l_erasable = new QLabel(this);
	layout_info->addWidget(l_erasable,4,1);
	pl_prot = new QLabel(tr("Protection:"), this);
	layout_info->addWidget(pl_prot,5,0);
	l_prot = new QLabel(this);
	layout_info->addWidget(l_prot,5,1);
	pl_regions = new QLabel(tr("Regions:"), this);
	layout_info->addWidget(pl_regions,6,0);
	l_regions = new QLabel(this);
	layout_info->addWidget(l_regions,6,1);
	pl_writer = new QLabel(tr("Written on:"), this);
	layout_info->addWidget(pl_writer, 7,0);
	l_writer = new QLabel(this);
	layout_info->addWidget(l_writer, 7,1);

	pl_cread = new QLabel(tr("Readable:"), this);
	layout_info->addWidget(pl_cread,0,2);
	pl_cfree = new QLabel(tr("Free:"), this);
	layout_info->addWidget(pl_cfree,1,2);
	pl_ctot = new QLabel(tr("Total:"), this);
	layout_info->addWidget(pl_ctot,2,2);

	l_cap = new QGridLayout();
	layout_info->addLayout(l_cap, 0,3,3,1);

	l_creads = new QLabel(this);
//	l_creads->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	l_cap->addWidget(l_creads,0,0);
	l_creadm = new QLabel(this);
//	l_creadm->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	l_cap->addWidget(l_creadm,0,1);
	l_creadmsf = new QLabel(this);
	l_creadmsf->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	l_cap->addWidget(l_creadmsf,0,2);
	l_cfrees = new QLabel(this);
//	l_cfrees->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	l_cap->addWidget(l_cfrees,1,0);
	l_cfreem = new QLabel(this);
//	l_cfreem->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	l_cap->addWidget(l_cfreem,1,1);
	l_cfreemsf = new QLabel(this);
	l_cfreemsf->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	l_cap->addWidget(l_cfreemsf,1,2);
	l_ctots = new QLabel(this);
//	l_ctots->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	l_cap->addWidget(l_ctots,2,0);
	l_ctotm = new QLabel(this);
//	l_ctotm->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	l_cap->addWidget(l_ctotm,2,1);
	l_ctotmsf = new QLabel(this);
	l_ctotmsf->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	l_cap->addWidget(l_ctotmsf,2,2);


	pl_dstate = new QLabel(tr("Disc state:"), this);
	layout_info->addWidget(pl_dstate,3,2);
	l_dstate = new QLabel(this);
	layout_info->addWidget(l_dstate,3,3);
	pl_sstate = new QLabel(tr("Session state:"), this);
	layout_info->addWidget(pl_sstate,4,2);
	l_sstate = new QLabel(this);
	layout_info->addWidget(l_sstate,4,3);
	pl_rspeeds = new QLabel(tr("Read speeds:"), this);
	layout_info->addWidget(pl_rspeeds,5,2);
	l_rspeeds = new QLabel(this);
	layout_info->addWidget(l_rspeeds,5,3);
	pl_wspeedsd = new QLabel(tr("Write speeds (Device):"), this);
	layout_info->addWidget(pl_wspeedsd,6,2);
	l_wspeedsd = new QLabel(this);
	layout_info->addWidget(l_wspeedsd,6,3);
	pl_wspeedsm = new QLabel(tr("Write speeds (Media):"), this);
	layout_info->addWidget(pl_wspeedsm,7,2);
	l_wspeedsm = new QLabel(this);
	layout_info->addWidget(l_wspeedsm,7,3);

	layout_info->setColumnStretch(0,1);
	layout_info->setColumnStretch(1,3);
	layout_info->setColumnStretch(2,1);
	layout_info->setColumnStretch(3,3);

	hline0 = new QFrame(this);
	hline0->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(hline0);

#ifdef SHOW_SPEEDS
	pl_rd_max = new QLabel(tr("RD max:"), this);
	pl_rd_max->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_rd_max, 9, 0);
	l_rd_max = new QLabel(this);
	l_rd_max->setMinimumWidth(120);
	layout_left->addWidget(l_rd_max, 9, 1, 1, 2);

	pl_rd_lst = new QLabel(tr("RD lst:"), this);
	pl_rd_lst->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_rd_lst, 10, 0);
	c_rd_lst = new QComboBox(this);
	c_rd_lst->setMinimumWidth(120);
	layout_left->addWidget(c_rd_lst, 10, 1, 1, 2);

	pl_wr_max = new QLabel(tr("WR max:"), this);
	pl_wr_max->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_wr_max, 11, 0);
	l_wr_max = new QLabel(this);
	l_wr_max->setMinimumWidth(120);
	layout_left->addWidget(l_wr_max, 11, 1, 1, 2);

	pl_wr_lst = new QLabel(tr("WR lst:"), this);
	pl_wr_lst->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_wr_lst, 12, 0);
	c_wr_lst = new QComboBox(this);
	c_wr_lst->setMinimumWidth(120);
	layout_left->addWidget(c_wr_lst, 12, 1, 1, 2);
#endif

#ifdef MINFO_TREE
	QFont mifont;

	minfo = new QTreeWidget(this);
	minfo->header()->hide();
	minfo->setRootIsDecorated(false);
	minfo->setColumnCount(1);
	mifont = minfo->font();
	mifont.setFamily("Monospace");
//	mifont.setBold(false);
//	mifont.setItalic(false);
	minfo->setFont(mifont);
	layout->addWidget(minfo);

	minfo_detail = new QTreeWidgetItem(minfo, QStringList() << tr("Detailed info") << "");
	minfo_detail->setIcon(0,QIcon(":images/info.png"));
	minfo_detail->setExpanded(true);
	minfo->addTopLevelItem(minfo_detail);
#else
	layout->addStretch(10);
#endif
	clear();
#ifndef QT_NO_DEBUG
	qDebug("END: tabMediaInfo()");
#endif
}

tabMediaInfo::~tabMediaInfo()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~tabMediaInfo()");
#endif
#ifndef QT_NO_DEBUG
	qDebug("END: ~tabMediaInfo()");
#endif
}

void tabMediaInfo::clear()
{
#ifndef QT_NO_DEBUG
	qDebug("tabMediaInfo::clear()");
#endif
	l_type->setText("-");
	l_book->setText("-");
	l_mid->setText("-");
	l_layers->setText("-");
	l_erasable->setText("-");
	l_prot->setText("-");
	l_regions->setText("-");
	l_writer->setText("-");

	l_dstate->setText("-");
	l_sstate->setText("-");
	l_creads->setText("-");
	l_creadm->clear();
	l_creadmsf->clear();
	l_cfrees->setText("-");
	l_cfreem->clear();
	l_cfreemsf->clear();
	l_ctots->setText("-");
	l_ctotm->clear();
	l_ctotmsf->clear();
	l_rspeeds->setText("-");
	l_wspeedsd->setText("-");
	l_wspeedsm->setText("-");

#ifdef SHOW_SPEEDS
	l_rd_max->setText("-");
	c_rd_lst->clear();
	l_wr_max->setText("-");
	c_wr_lst->clear();
#endif

#ifdef MINFO_TREE
	minfo_detail->takeChildren();
#endif
}

void tabMediaInfo::selectDevice()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabMediaInfo::selectDevice()");
	qDebug() << "SIGNAL sender: " << sender();
#endif
	clear();

	updateData();
	QObject::connect( devices->current(), SIGNAL(doneMInfo(int)), this, SLOT(updateData(int)) );
#ifndef QT_NO_DEBUG
	qDebug("END: tabMediaInfo::selectDevice()");
#endif
}

void tabMediaInfo::updateData(int xcode)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabMediaInfo::updateData()");
#endif
	const device *dev = devices->current();
	bool showMSF = dev->media.type.startsWith("CD");

	l_type->setText(dev->media.type);
	l_book->setText(dev->media.category);
	//l_layers->setNum(dev->media.layers);
	l_layers->setText(dev->media.layers);
	l_prot->setText(dev->media.prot);
	l_regions->setText(dev->media.regions);
	l_erasable->setText( dev->media.erasable); //? "Yes" : "No");

	l_dstate->setText(dev->media.dstate);
	l_sstate->setText(dev->media.sstate);

	if (dev->media.creads) {
		l_creads->setText(QString::number(dev->media.creads));
		l_creadm->setText(QString("%1 MB").arg(dev->media.creadm));
		l_creadmsf->setText(dev->media.creadmsf);
	} else {
		l_creads->setText("-");
		l_creadm->clear();
		l_creadmsf->clear();
	}
	if (dev->media.cfrees) {
		l_cfrees->setText(QString::number(dev->media.cfrees));
		l_cfreem->setText(QString("%1 MB").arg(dev->media.cfreem));
		l_cfreemsf->setText(dev->media.cfreemsf);
	} else {
		l_cfrees->setText("-");
		l_cfreem->clear();
		l_cfreemsf->clear();
	}
	if (dev->media.ctots)  {
		l_ctots->setText(QString::number(dev->media.ctots));
		l_ctotm->setText(QString("%1 MB").arg(dev->media.ctotm));
		l_ctotmsf->setText(dev->media.ctotmsf);
	} else {
		l_ctots->setText("-");
		l_ctotm->clear();
		l_ctotmsf->clear();
	}
	l_creadmsf->setVisible(showMSF);
	l_cfreemsf->setVisible(showMSF);
	l_ctotmsf->setVisible(showMSF);

	l_mid->setText(dev->media.mid);
	if (!dev->media.rspeeds.size())
		l_rspeeds->setText("-");
	else
		l_rspeeds->setText(dev->media.rspeeds.join(" - "));
	if (!dev->media.wspeedsd.size())
		l_wspeedsd->setText("-");
	else
		l_wspeedsd->setText(dev->media.wspeedsd.join(" - "));
	if (!dev->media.wspeedsm.size())
		l_wspeedsm->setText("-");
	else
		l_wspeedsm->setText(dev->media.wspeedsm.join(" - "));

	l_writer->setText(dev->media.writer);

#ifdef MINFO_TREE
	minfo_detail->addChildren( dev->info_media);
#endif
	if (xcode) {
		QMessageBox::warning(this, tr("Error"), tr("Error updating media info!")+"\n"+tr("qScan finished with non-zero exit code"));
	}
#ifndef QT_NO_DEBUG
	qDebug("END: tabMediaInfo::updateData()");
#endif
}

