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

#include <QFrame>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>

#include <device.h>

#include <mcapwidget.h>

#include "tab_devinfo.h"
#include <QDebug>

#include <qpx_mmc_defs.h>

tabDevInfo::tabDevInfo(QPxSettings *iset, devlist *idev, QWidget *p, Qt::WindowFlags fl)
	: QWidget(p,fl)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabDevInfo()");
#endif
	settings = iset;
	devices = idev;

	layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(3);

	layout_left = new QGridLayout();
	layout_left->setVerticalSpacing(1);
	layout_left->setHorizontalSpacing(6);
	layout->addLayout(layout_left);

	label_left = new QLabel("<b>"+tr("Basic info")+"</b>",this);
	label_left->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
	label_left->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	label_left->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	layout_left->addWidget(label_left, 0, 0, 1, 2);

	pl_vendor = new QLabel(tr("Vendor:"), this);
	pl_vendor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_vendor, 1, 0);
	l_vendor = new QLabel(this);
	l_vendor->setMinimumWidth(120);
	layout_left->addWidget(l_vendor, 1, 1);

	pl_model = new QLabel(tr("Model:"), this);
	pl_model->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_model, 2, 0);
	l_model = new QLabel(this);
	l_model->setMinimumWidth(120);
	layout_left->addWidget(l_model, 2, 1);

	pl_fw = new QLabel(tr("F/W:"), this);
	pl_fw->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_fw, 3, 0);
	l_fw = new QLabel(this);
	l_fw->setMinimumWidth(120);
	layout_left->addWidget(l_fw, 3, 1);

	pl_tla = new QLabel(tr("TLA#"), this);
	pl_tla->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_tla, 4, 0);
	l_tla = new QLabel(this);
	l_tla->setMinimumWidth(120);
	layout_left->addWidget(l_tla, 4, 1);

	pl_sn = new QLabel(tr("S/N:"), this);
	pl_sn->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_sn, 5, 0);
	l_sn = new QLabel(this);
	l_sn->setMinimumWidth(120);
	layout_left->addWidget(l_sn, 5, 1);

	pl_buf = new QLabel(tr("Buffer:"), this);
	pl_buf->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_buf, 6, 0);
	l_buf = new QLabel(this);
	l_buf->setMinimumWidth(120);
	layout_left->addWidget(l_buf, 6, 1);

	pl_iface = new QLabel(tr("IFace:"), this);
	pl_iface->setAlignment(Qt::AlignRight | Qt::AlignVCenter);	
	layout_left->addWidget(pl_iface, 7, 0);
	l_iface = new QLabel(this);
	l_iface->setMinimumWidth(120);
	layout_left->addWidget(l_iface, 7, 1);

	pl_loader = new QLabel(tr("Loader:"), this);
	pl_loader->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_loader, 8, 0);
	l_loader = new QLabel(this);
	l_loader->setMinimumWidth(120);
	layout_left->addWidget(l_loader, 8, 1);

	pl_rpc_phase = new QLabel(tr("RPC Phase:"), this);
	pl_rpc_phase->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_rpc_phase, 9, 0);
	l_rpc_phase = new QLabel(this);
	l_rpc_phase->setMinimumWidth(120);
	layout_left->addWidget(l_rpc_phase, 9, 1);

	pl_rpc_reg = new QLabel(tr("Region:"), this);
	pl_rpc_reg->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_rpc_reg, 10, 0);
	l_rpc_reg = new QLabel(this);
	l_rpc_reg->setMinimumWidth(120);
	layout_left->addWidget(l_rpc_reg, 10, 1);

	pl_rpc_ch = new QLabel(tr("Changes:"), this);
	pl_rpc_ch->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_rpc_ch, 11, 0);
	l_rpc_ch = new QLabel(this);
	l_rpc_ch->setMinimumWidth(120);
	layout_left->addWidget(l_rpc_ch, 11, 1);

	pl_rpc_rst = new QLabel(tr("Resets:"), this);
	pl_rpc_rst->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_rpc_rst, 12, 0);
	l_rpc_rst = new QLabel(this);
	l_rpc_rst->setMinimumWidth(120);
	layout_left->addWidget(l_rpc_rst, 12, 1);

// Plextor Lifetime
	hline0 = new QFrame(this);
	hline0->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout_left->addWidget(hline0,13,0,1,2);

	pl_life_dn = new QLabel(tr("Discs loaded:"), this);
	pl_life_dn->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_life_dn, 14, 0);
	l_life_dn = new QLabel(this);
//	l_life_dn->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_life_dn->setMinimumWidth(120);
	layout_left->addWidget(l_life_dn, 14, 1);

	pl_life_cr = new QLabel(tr("CD read:"), this);
	pl_life_cr->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_life_cr, 15, 0);
	l_life_cr = new QLabel(this);
//	l_life_cr->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_life_cr->setMinimumWidth(120);
	layout_left->addWidget(l_life_cr, 15, 1);

	pl_life_cw = new QLabel(tr("CD write:"), this);
	pl_life_cw->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_life_cw, 16, 0);
	l_life_cw = new QLabel(this);
//	l_life_cw->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_life_cw->setMinimumWidth(120);
	layout_left->addWidget(l_life_cw, 16, 1);

	pl_life_dr = new QLabel(tr("DVD read:"), this);
	pl_life_dr->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_life_dr, 17, 0);
	l_life_dr = new QLabel(this);
//	l_life_dr->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_life_dr->setMinimumWidth(120);
	layout_left->addWidget(l_life_dr, 17, 1);

	pl_life_dw = new QLabel(tr("DVD write:"), this);
	pl_life_dw->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout_left->addWidget(pl_life_dw, 18, 0);
	l_life_dw = new QLabel(this);
//	l_life_dw->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_life_dw->setMinimumWidth(120);
	layout_left->addWidget(l_life_dw, 18, 1);

	layout_left->setColumnStretch(0,2);
	layout_left->setColumnStretch(1,3);
	layout_left->setRowStretch(19,20);
	
	int hidx=0, vidx=0, hidx0 = 0, rows=0;
	MediaCapWidget *cap;

	cap_grid = new QGridLayout;
	layout->addLayout(cap_grid);

	lc_media = new QLabel("<b>"+tr("Media R/W Capabilities")+"</b>",this);
	lc_media->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
	lc_media->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	lc_media->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	cap_grid->addWidget(lc_media, hidx, 0, 1, CAP_COLS);
	hidx++;
	vidx=0; hidx0=0;
	rows = (sizeof(rw_capabilities) / sizeof (desc64) - 2 + CAP_COLS) / CAP_COLS  ;
	for (int idx=0; rw_capabilities[idx].id || strlen(rw_capabilities[idx].name); idx++) {
		if (rw_capabilities[idx].id) {
			cap = new MediaCapWidget(rw_capabilities[idx].name, 1, rw_capabilities[idx].id, this);
			cap_media.append(cap);
			cap_grid->addWidget(cap, hidx+hidx0, vidx);
		}
		hidx0 = (hidx0+1) % rows;
		if (!hidx0) vidx++;
	}
	hidx+=rows;


	lc_generic = new QLabel("<b>"+tr("Generic Capabilities")+"</b>",this);
	lc_generic->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
	lc_generic->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	lc_generic->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	cap_grid->addWidget(lc_generic, hidx, 0, 1, CAP_COLS);
	hidx++;
	vidx=0; hidx0=0;
	rows = (sizeof(capabilities) / sizeof (desc64) - 2 + CAP_COLS) / CAP_COLS  ;
	for (int idx=0; capabilities[idx].id || strlen(capabilities[idx].name); idx++) {
		if (capabilities[idx].id) {
			cap = new MediaCapWidget(capabilities[idx].name, 0, capabilities[idx].id , this);
			cap_generic.append(cap);
			cap_grid->addWidget(cap, hidx+hidx0, vidx);
		}
		hidx0 = (hidx0+1) % rows;
		if (!hidx0) vidx++;
	}
	hidx+=rows;

	cap_grid->setRowStretch(hidx, 10);
	cap_grid->setColumnStretch(0, 10);
	cap_grid->setColumnStretch(1, 10);
	cap_grid->setColumnStretch(2, 10);

	layout->setStretchFactor(layout_left, 1);
	layout->setStretchFactor(cap_grid,   3);
	
	clear();
#ifndef QT_NO_DEBUG
	qDebug("END: tabDevInfo()");
#endif
}

tabDevInfo::~tabDevInfo()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~tabDevInfo()");
#endif

#ifndef QT_NO_DEBUG
	qDebug("END: ~tabDevInfo()");
#endif
}

void tabDevInfo::clear()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabDevInfo::clear()");
#endif
	l_vendor->setText("-");
	l_model->setText("-");
	l_fw->setText("-");
	l_tla->setText("-");
	l_sn->setText("-");
	l_buf->setText("-");
	l_iface->setText("-");
	l_loader->setText("-");
	l_rpc_phase->setText("-");
	l_rpc_reg->setText("-");
	l_rpc_ch->setText("-");
	l_rpc_rst->setText("-");

	for (int idx=0; idx < cap_media.size(); idx++)
		cap_media[idx]->clear();
	for (int idx=0; idx < cap_generic.size(); idx++)
		cap_generic[idx]->clear();
#ifndef QT_NO_DEBUG
	qDebug("END: tabDevInfo::clear()");
#endif
}

void tabDevInfo::selectDevice()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabDevInfo::selectDevice()");
#endif
	clear();
	updateData();
	QObject::connect( devices->current(), SIGNAL(doneDInfo(int)), this, SLOT(updateData(int)) );
#ifndef QT_NO_DEBUG
	qDebug("END: tabDevInfo::selectDevice()");
#endif
}

void tabDevInfo::updateData(int xcode)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: tabDevInfo::updateData()");
#endif
	const device *dev = devices->current();
	l_vendor->setText(dev->ven);
	l_model->setText(dev->dev);
	l_fw->setText(dev->fw);
	l_tla->setText(dev->tla);
	l_sn->setText(dev->sn);
	l_buf->setText(dev->buf);
	l_iface->setText(dev->iface);
	l_loader->setText(dev->loader);

	l_rpc_phase->setText((dev->rpc_phase < 0) ? "-" : QString::number(dev->rpc_phase) );
	l_rpc_reg->setText((dev->rpc_reg < 0) ? ((dev->rpc_reg == -2) ? "not set" : "-") : QString::number(dev->rpc_reg) );
	l_rpc_ch->setText((dev->rpc_ch < 0) ? "-" : QString::number(dev->rpc_ch) );
	l_rpc_rst->setText((dev->rpc_rst < 0) ? "-" : QString::number(dev->rpc_rst) );

	if (dev->life_dn>=0) {
		l_life_dn->setNum(dev->life_dn);
		l_life_cr->setText(dev->life_cr);
		l_life_cw->setText(dev->life_cw);
		l_life_dr->setText(dev->life_dr);
		l_life_dw->setText(dev->life_dw);
	} else {
		l_life_dn->setText("-");
		l_life_cr->setText("-");
		l_life_cw->setText("-");
		l_life_dr->setText("-");
		l_life_dw->setText("-");
	}

	for (int idx=0; idx < cap_generic.size(); idx++)
		cap_generic[idx]->setCap(dev->cap);
	for (int idx=0; idx < cap_media.size(); idx++) {
		cap_media[idx]->setR(dev->cap_rd);
		cap_media[idx]->setW(dev->cap_wr);
	}

	if (xcode) {
		QMessageBox::warning(this, tr("Error"), tr("Error updating device info!")+"\n"+tr("qScan finished with non-zero exit code"));
	}
#ifndef QT_NO_DEBUG
	qDebug("END: tabDevInfo::updateData()");
#endif
}

