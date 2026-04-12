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
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QFrame>

#include <QStyle>

#include <colorlabel.h>
#include <qpxgraph.h>
#include <qpxsettings.h>
#include <errc_detailed.h>
#include "tab_errc.h"
#include <QDebug>

// clang-format off
// --- ERRC label names (reused for ColorLabel init and setText) ---
static const char* const LBL_BLER = "BLER";
static const char* const LBL_E22  = "E22";
static const char* const LBL_E32  = "E32";
static const char* const LBL_UNCR = "UNCR";
static const char* const LBL_PI8  = "PI8";
static const char* const LBL_PIF  = "PIF";
static const char* const LBL_POF  = "POF";
static const char* const LBL_LDC  = "LDC";
static const char* const LBL_BIS  = "BIS";

// --- ERRC tooltip strings (reused for checkbox and info-icon tooltips) ---
static const char* const TT_BLER =
    "Block Error Rate, C1 frames per second\n"
    "with at least one erroneous byte. Sum of E11+E21+E31.";
static const char* const TT_E22 =
    "C2 correction events with 2 erroneous symbols.\n"
    "Elevated values indicate C1 is passing\n"
    "significant errors through to C2.";
static const char* const TT_E32 =
    "C2 uncorrectable, errors exceeding C2\n"
    "Reed-Solomon correction capacity. Means data loss.";
static const char* const TT_UNCR_CD =
    "Sectors where all error correction stages\n"
    "have been exhausted. Data is unrecoverable.";
static const char* const TT_PI8 =
    "Sum of Parity Inner error counts over 8 consecutive\n"
    "ECC blocks. Each PI error is a row requiring\n"
    "RS(182,172) correction.";
static const char* const TT_PIF =
    "PI Failures, PI rows with more than 5 erroneous\n"
    "bytes, exceeding RS(182,172) correction capacity.";
static const char* const TT_POF =
    "PO Failures, PO columns exceeding RS(208,192)\n"
    "correction capacity. Indicates severe physical damage.";
static const char* const TT_UNCR_DVD =
    "Sectors where both PI and PO correction stages\n"
    "have been exhausted. Data is unrecoverable.";
static const char* const TT_LDC =
    "Long Distance Code, number of corrected error symbols in the LDC ECC. Primary quality metric.\n"
    "\n"
    "Some indicative values follow, these also depend on your drive and your scan speed so YMMV:\n"
    "  Perfect:    avg < 5\n"
    "  Good:       avg < 15\n"
    "  Acceptable: avg < 30\n"
    "  Degrading:  avg >= 30";
static const char* const TT_BIS =
    "Burst Indication Subcode, uncorrectable errors in the BIS cluster.\n"
    "More severe than LDC; any significant BIS count indicates data at risk.\n"
    "\n"
    "Some indicative values follow, these also depend on your drive and your scan speed so YMMV:\n"
    "  Perfect:    avg < 0.1\n"
    "  Good:       avg < 0.5\n"
    "  Acceptable: avg < 1.0\n"
    "  Degrading:  avg >= 1.0";
// clang-format on


tabERRC::tabERRC(QPxSettings* iset, devlist* idev, QString iname, QWidget* p, Qt::WindowFlags fl)
    : GraphTab(iset, idev, iname, TEST_ERRC, p, fl) {
#ifndef QT_NO_DEBUG
	qDebug("STA: tabERRC()");
#endif
	xerrc = NULL;
	isCD = isDVD = isBD = false;
	currentErrcMask = 0;

	layout_info = new QVBoxLayout(infow);
	layout_info->setContentsMargins(0, 0, 0, 0);
	layout_info->setSpacing(3);
	infow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

#ifdef __LEGEND_SHOW_SPEED
	pl_spd = new ColorLabel(QColor(Qt::black), "Speed", 0, infow);
	pl_spd->setMinimumSize(100, 20);
	layout_info->addWidget(pl_spd);
#endif
	pl_e0 = new ColorLabel(QColor(Qt::black), "BLER/PI8", 0, infow);
	l_e0_info = new QLabel(infow);
	l_e0_info->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(16, 16));
	l_e0_info->setCursor(Qt::WhatsThisCursor);
	l_e0_info->setVisible(false);
	{
		QHBoxLayout* hb = new QHBoxLayout();
		hb->setContentsMargins(0, 0, 0, 0);
		hb->setSpacing(2);
		hb->addWidget(pl_e0);
		hb->addWidget(l_e0_info);
		layout_info->addLayout(hb);
	}

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


	pl_e1 = new ColorLabel(QColor(Qt::black), "E22/PIF", 0, infow);
	l_e1_info = new QLabel(infow);
	l_e1_info->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(16, 16));
	l_e1_info->setCursor(Qt::WhatsThisCursor);
	l_e1_info->setVisible(false);
	{
		QHBoxLayout* hb = new QHBoxLayout();
		hb->setContentsMargins(0, 0, 0, 0);
		hb->setSpacing(2);
		hb->addWidget(pl_e1);
		hb->addWidget(l_e1_info);
		layout_info->addLayout(hb);
	}

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


	pl_e2 = new ColorLabel(QColor(Qt::black), "E32/POF", 0, infow);
	l_e2_info = new QLabel(infow);
	l_e2_info->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(16, 16));
	l_e2_info->setCursor(Qt::WhatsThisCursor);
	l_e2_info->setVisible(false);
	{
		QHBoxLayout* hb = new QHBoxLayout();
		hb->setContentsMargins(0, 0, 0, 0);
		hb->setSpacing(2);
		hb->addWidget(pl_e2);
		hb->addWidget(l_e2_info);
		layout_info->addLayout(hb);
	}

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

	// --- CD checkboxes ---
	w_cb_cd = new QWidget(infow);
	{
		QVBoxLayout* vb = new QVBoxLayout(w_cb_cd);
		vb->setContentsMargins(0, 0, 0, 0);
		vb->setSpacing(2);
		cb_cd_bler = new QCheckBox(LBL_BLER, w_cb_cd);
		cb_cd_bler->setChecked(true);
		cb_cd_bler->setToolTip(TT_BLER);
		cb_cd_e22 = new QCheckBox(LBL_E22, w_cb_cd);
		cb_cd_e22->setChecked(true);
		cb_cd_e22->setToolTip(TT_E22);
		cb_cd_e32 = new QCheckBox(LBL_E32, w_cb_cd);
		cb_cd_e32->setChecked(true);
		cb_cd_e32->setToolTip(TT_E32);
		cb_cd_uncr = new QCheckBox(LBL_UNCR, w_cb_cd);
		cb_cd_uncr->setChecked(true);
		cb_cd_uncr->setToolTip(TT_UNCR_CD);
		vb->addWidget(cb_cd_bler);
		vb->addWidget(cb_cd_e22);
		vb->addWidget(cb_cd_e32);
		vb->addWidget(cb_cd_uncr);
	}
	w_cb_cd->setVisible(false);
	layout_info->addWidget(w_cb_cd);

	// --- DVD checkboxes ---
	w_cb_dvd = new QWidget(infow);
	{
		QVBoxLayout* vb = new QVBoxLayout(w_cb_dvd);
		vb->setContentsMargins(0, 0, 0, 0);
		vb->setSpacing(2);
		cb_dvd_pi8 = new QCheckBox(LBL_PI8, w_cb_dvd);
		cb_dvd_pi8->setChecked(true);
		cb_dvd_pi8->setToolTip(TT_PI8);
		cb_dvd_pif = new QCheckBox(LBL_PIF, w_cb_dvd);
		cb_dvd_pif->setChecked(true);
		cb_dvd_pif->setToolTip(TT_PIF);
		cb_dvd_pof = new QCheckBox(LBL_POF, w_cb_dvd);
		cb_dvd_pof->setChecked(true);
		cb_dvd_pof->setToolTip(TT_POF);
		cb_dvd_uncr = new QCheckBox(LBL_UNCR, w_cb_dvd);
		cb_dvd_uncr->setChecked(true);
		cb_dvd_uncr->setToolTip(TT_UNCR_DVD);
		vb->addWidget(cb_dvd_pi8);
		vb->addWidget(cb_dvd_pif);
		vb->addWidget(cb_dvd_pof);
		vb->addWidget(cb_dvd_uncr);
	}
	w_cb_dvd->setVisible(false);
	layout_info->addWidget(w_cb_dvd);

	// --- BD checkboxes ---
	w_cb_bd = new QWidget(infow);
	{
		QVBoxLayout* vb = new QVBoxLayout(w_cb_bd);
		vb->setContentsMargins(0, 0, 0, 0);
		vb->setSpacing(2);
		cb_bd_ldc = new QCheckBox(LBL_LDC, w_cb_bd);
		cb_bd_ldc->setChecked(true);
		cb_bd_ldc->setToolTip(TT_LDC);
		cb_bd_bis = new QCheckBox(LBL_BIS, w_cb_bd);
		cb_bd_bis->setChecked(true);
		cb_bd_bis->setToolTip(TT_BIS);
		vb->addWidget(cb_bd_ldc);
		vb->addWidget(cb_bd_bis);
	}
	w_cb_bd->setVisible(false);
	layout_info->addWidget(w_cb_bd);

	// Connect all checkboxes to the same slot
	QObject::connect(cb_cd_bler, SIGNAL(toggled(bool)), this, SLOT(onErrcToggled()));
	QObject::connect(cb_cd_e22, SIGNAL(toggled(bool)), this, SLOT(onErrcToggled()));
	QObject::connect(cb_cd_e32, SIGNAL(toggled(bool)), this, SLOT(onErrcToggled()));
	QObject::connect(cb_cd_uncr, SIGNAL(toggled(bool)), this, SLOT(onErrcToggled()));
	QObject::connect(cb_dvd_pi8, SIGNAL(toggled(bool)), this, SLOT(onErrcToggled()));
	QObject::connect(cb_dvd_pif, SIGNAL(toggled(bool)), this, SLOT(onErrcToggled()));
	QObject::connect(cb_dvd_pof, SIGNAL(toggled(bool)), this, SLOT(onErrcToggled()));
	QObject::connect(cb_dvd_uncr, SIGNAL(toggled(bool)), this, SLOT(onErrcToggled()));
	QObject::connect(cb_bd_ldc, SIGNAL(toggled(bool)), this, SLOT(onErrcToggled()));
	QObject::connect(cb_bd_bis, SIGNAL(toggled(bool)), this, SLOT(onErrcToggled()));

	// --- Scale type ---
	QFrame* sep_scale = new QFrame(infow);
	sep_scale->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout_info->addWidget(sep_scale);

	{
		QWidget* w_scaletype = new QWidget(infow);
		QHBoxLayout* hb = new QHBoxLayout(w_scaletype);
		hb->setContentsMargins(0, 0, 0, 0);
		hb->setSpacing(4);
		rb_log = new QRadioButton("Log", w_scaletype);
		rb_lin = new QRadioButton("Lin", w_scaletype);
		hb->addWidget(rb_log);
		hb->addWidget(rb_lin);
		layout_info->addWidget(w_scaletype);
	}

	// --- Zoom buttons ---
	{
		QWidget* w_zoom = new QWidget(infow);
		QHBoxLayout* hb = new QHBoxLayout(w_zoom);
		hb->setContentsMargins(0, 0, 0, 0);
		hb->setSpacing(4);
		pb_scaleIn = new QPushButton("+", w_zoom);
		pb_scaleOut = new QPushButton("\xe2\x88\x92", w_zoom); // minus sign
		pb_scaleIn->setMaximumWidth(40);
		pb_scaleOut->setMaximumWidth(40);
		hb->addWidget(pb_scaleOut);
		hb->addWidget(pb_scaleIn);
		layout_info->addWidget(w_zoom);
	}

	// Connect scale controls to graph slots
	QObject::connect(rb_log, SIGNAL(clicked()), graph, SLOT(setScaleTypeLog()));
	QObject::connect(rb_lin, SIGNAL(clicked()), graph, SLOT(setScaleTypeLin()));
	QObject::connect(pb_scaleIn, SIGNAL(clicked()), graph, SLOT(scaleIn()));
	QObject::connect(pb_scaleOut, SIGNAL(clicked()), graph, SLOT(scaleOut()));
	QObject::connect(graph, SIGNAL(scaleChanged()), this, SLOT(onScaleChanged()));

	// Initialise radio buttons to reflect saved scale type
	onScaleChanged();

	l_speed = new QLabel();
	layout_info->addWidget(l_speed);

	pb_xerrc = new QPushButton("Detailed");
	layout_info->addWidget(pb_xerrc);

	QObject::connect(pb_xerrc, SIGNAL(clicked()), this, SLOT(toggleXErrc()));

	clear();
#ifndef QT_NO_DEBUG
	qDebug("END: tabERRC()");
#endif
}

tabERRC::~tabERRC() {
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
void tabERRC::selectDevice() {
#ifndef QT_NO_DEBUG
	qDebug("tabERRC::selectDevice()");
#endif
	device* dev = devices->current();
	updateAll();
	//	GraphTab::updateLast((int)time, NULL, 1);
	//	updateSummary(dev);

	//	QObject::connect( devices->current(), SIGNAL(block_ERRC()), this, SLOT(update()) );
	QObject::connect(dev, SIGNAL(doneMInfo(int)), this, SLOT(updateAll()));
	QObject::connect(dev, SIGNAL(block_ERRC()), this, SLOT(updateLast()));
}

void tabERRC::updateAll() {
	bool show_e2 = true;
	device* dev = devices->current();
	GraphTab::updateLast((int)(dev->testData.errc_time), NULL, 1);
	updateSummary(dev);
	if (dev->media.type.startsWith("CD")) {
		pl_e0->setText(LBL_BLER);
		l_e0_info->setToolTip(TT_BLER);
		l_e0_info->setVisible(true);
		pl_e1->setText(LBL_E22);
		l_e1_info->setToolTip(TT_E22);
		l_e1_info->setVisible(true);
		pl_e2->setText(LBL_E32);
		l_e2_info->setToolTip(TT_E32);
		l_e2_info->setVisible(true);
	} else if (dev->media.type.startsWith("DVD")) {
		pl_e0->setText(LBL_PI8);
		l_e0_info->setToolTip(TT_PI8);
		l_e0_info->setVisible(true);
		pl_e1->setText(LBL_PIF);
		l_e1_info->setToolTip(TT_PIF);
		l_e1_info->setVisible(true);
		pl_e2->setText(LBL_POF);
		l_e2_info->setToolTip(TT_POF);
		l_e2_info->setVisible(true);
	} else if (dev->media.type.startsWith("BD")) {
		pl_e0->setText(LBL_LDC);
		l_e0_info->setToolTip(TT_LDC);
		l_e0_info->setVisible(true);
		pl_e1->setText(LBL_BIS);
		l_e1_info->setToolTip(TT_BIS);
		l_e1_info->setVisible(true);
		show_e2 = false;
	} else {
		pl_e0->setText("BLER/PI8/LDC");
		l_e0_info->setVisible(false);
		pl_e1->setText("E22/PIF/BIS");
		l_e1_info->setVisible(false);
		pl_e2->setText("E32/POF/---");
		l_e2_info->setVisible(false);
	}
	pl_e2->setVisible(show_e2);
	l_e2_info->setVisible(show_e2 && l_e2_info->isVisible());
	l_e2t->setVisible(show_e2);
	l_e2m->setVisible(show_e2);
	l_e2a->setVisible(show_e2);

	// Update graph-type checkboxes when media type changes
	bool newCD = dev->media.type.startsWith("CD");
	bool newDVD = dev->media.type.startsWith("DVD");
	bool newBD = dev->media.type.startsWith("BD");
	if (newCD != isCD || newDVD != isDVD || newBD != isBD) {
		isCD = newCD;
		isDVD = newDVD;
		isBD = newBD;
		w_cb_cd->setVisible(false);
		w_cb_dvd->setVisible(false);
		w_cb_bd->setVisible(false);

		auto setAllChecked = [](QList<QCheckBox*> cbs) {
			for (QCheckBox* cb : cbs) {
				cb->blockSignals(true);
				cb->setChecked(true);
				cb->blockSignals(false);
			}
		};

		if (isCD) {
			setAllChecked({cb_cd_bler, cb_cd_e22, cb_cd_e32, cb_cd_uncr});
			currentErrcMask = GRAPH_DFL_CD;
			w_cb_cd->setVisible(true);
		} else if (isDVD) {
			setAllChecked({cb_dvd_pi8, cb_dvd_pif, cb_dvd_pof, cb_dvd_uncr});
			currentErrcMask = GRAPH_DFL_DVD;
			w_cb_dvd->setVisible(true);
		} else if (isBD) {
			setAllChecked({cb_bd_ldc, cb_bd_bis});
			currentErrcMask = GRAPH_DFL_BD;
			w_cb_bd->setVisible(true);
		} else {
			currentErrcMask = 0;
		}
		graph->setErrcMask(currentErrcMask);
	}

	updateLegend();
	if (xerrc) xerrc->updateAll();
}

void tabERRC::updateLegend() {
	device* dev = devices->current();
	if (dev->media.type.startsWith("CD")) {
		pl_e0->setColor(*(settings->col_errc.cd.bler));
		pl_e1->setColor(*(settings->col_errc.cd.e22));
		pl_e2->setColor(*(settings->col_errc.cd.e32));
	} else if (dev->media.type.startsWith("DVD")) {
		pl_e0->setColor(*(settings->col_errc.dvd.pi8));
		pl_e1->setColor(*(settings->col_errc.dvd.pif));
		pl_e2->setColor(*(settings->col_errc.dvd.pof));
	} else if (dev->media.type.startsWith("BD")) {
		pl_e0->setColor(*(settings->col_errc.bd.ldc));
		pl_e1->setColor(*(settings->col_errc.bd.bis));
	} else {
		pl_e0->setColor(QColor(Qt::black));
		pl_e1->setColor(QColor(Qt::black));
		pl_e2->setColor(QColor(Qt::black));
	}
}

void tabERRC::updateGraph() { graph->update(); }

void tabERRC::updateLast() {
	bool show;
	device* dev = devices->current();
	int time = (int)(dev->testData.errc_time);
	GraphTab::updateLast(time, &show);
	if (!show) return;
	updateSummary(dev);

	if (!dev->testData.errc.isEmpty()) {
		float spdx = dev->testData.errc.last().raw.spdx;
		l_speed->setText(QString("Speed: %1x").arg(spdx, 0, 'f', 2));
	}

	if (xerrc && xerrc->isVisible()) xerrc->updateAll();
}

void tabERRC::updateSummary(device* dev) {
	if (!dev->testData.errc.size()) {
		l_speed->clear();
		l_e0t->clear();
		l_e0m->clear();
		l_e0a->clear();
		l_e1t->clear();
		l_e1m->clear();
		l_e1a->clear();
		l_e2t->clear();
		l_e2m->clear();
		l_e2a->clear();
		return;
	}
	if (dev->media.type.startsWith("CD")) {
		l_e0t->setText("Total: " + QString::number(dev->testData.errcTOT.cd.bler));
		l_e0m->setText("Max: " + QString::number(dev->testData.errcMAX.cd.bler));
		l_e0a->setText("Avg: " + QString::number(dev->testData.errcAVG.cd.bler, 'f', 2));

		l_e1t->setText("Total: " + QString::number(dev->testData.errcTOT.cd.e22));
		l_e1m->setText("Max: " + QString::number(dev->testData.errcMAX.cd.e22));
		l_e1a->setText("Avg: " + QString::number(dev->testData.errcAVG.cd.e22, 'f', 2));

		l_e2t->setText("Total: " + QString::number(dev->testData.errcTOT.cd.e32));
		l_e2m->setText("Max: " + QString::number(dev->testData.errcMAX.cd.e32));
		l_e2a->setText("Avg: " + QString::number(dev->testData.errcAVG.cd.e32, 'f', 2));
	} else if (dev->media.type.startsWith("DVD")) {
		l_e0t->setText("Total: " + QString::number(dev->testData.errcTOT.dvd.pi8));
		l_e0m->setText("Max: " + QString::number(dev->testData.errcMAX.dvd.pi8));
		l_e0a->setText("Avg: " + QString::number(dev->testData.errcAVG.dvd.pi8, 'f', 2));

		l_e1t->setText("Total: " + QString::number(dev->testData.errcTOT.dvd.pif));
		l_e1m->setText("Max: " + QString::number(dev->testData.errcMAX.dvd.pif));
		l_e1a->setText("Avg: " + QString::number(dev->testData.errcAVG.dvd.pif, 'f', 2));

		l_e2t->setText("Total: " + QString::number(dev->testData.errcTOT.dvd.pof));
		l_e2m->setText("Max: " + QString::number(dev->testData.errcMAX.dvd.pof));
		l_e2a->setText("Avg: " + QString::number(dev->testData.errcAVG.dvd.pof, 'f', 2));
	} else if (dev->media.type.startsWith("BD")) {
		l_e0t->setText("Total: " + QString::number(dev->testData.errcTOT.bd.ldc));
		l_e0m->setText("Max: " + QString::number(dev->testData.errcMAX.bd.ldc));
		l_e0a->setText("Avg: " + QString::number(dev->testData.errcAVG.bd.ldc, 'f', 2));

		l_e1t->setText("Total: " + QString::number(dev->testData.errcTOT.bd.bis));
		l_e1m->setText("Max: " + QString::number(dev->testData.errcMAX.bd.bis));
		l_e1a->setText("Avg: " + QString::number(dev->testData.errcAVG.bd.bis, 'f', 2));
	}
}

void tabERRC::toggleXErrc() {
	if (!xerrc) {
		xerrc = new ErrcDetailedDialog(settings, devices, this);
		QObject::connect(xerrc, SIGNAL(closed()), this, SLOT(XerrcClosed()));
		xerrc->show();
		xerrc->updateAll();
	} else {
		xerrc->hide();
		delete xerrc;
		xerrc = NULL;
	}
}

void tabERRC::XerrcClosed() {
	xerrc->disconnect();
	xerrc->deleteLater();
	xerrc = NULL;
}

void tabERRC::onErrcToggled() {
	int mask = 0;
	if (isCD) {
		if (cb_cd_bler->isChecked()) mask |= GRAPH_BLER;
		if (cb_cd_e22->isChecked()) mask |= GRAPH_E22;
		if (cb_cd_e32->isChecked()) mask |= GRAPH_E32;
		if (cb_cd_uncr->isChecked()) mask |= GRAPH_UNCR;
		if (!mask) mask = GRAPH_DFL_CD; // never go fully blank
	} else if (isDVD) {
		if (cb_dvd_pi8->isChecked()) mask |= GRAPH_PI8;
		if (cb_dvd_pif->isChecked()) mask |= GRAPH_PIF;
		if (cb_dvd_pof->isChecked()) mask |= GRAPH_POF;
		if (cb_dvd_uncr->isChecked()) mask |= GRAPH_UNCR;
		if (!mask) mask = GRAPH_DFL_DVD;
	} else if (isBD) {
		if (cb_bd_ldc->isChecked()) mask |= GRAPH_LDC;
		if (cb_bd_bis->isChecked()) mask |= GRAPH_BIS;
		if (!mask) mask = GRAPH_DFL_BD;
	}
	if (mask == currentErrcMask) return;
	currentErrcMask = mask;
	graph->setErrcMask(currentErrcMask);
}

void tabERRC::onScaleChanged() {
	bool isLog = (graph->getScaleType() == Scale::Log);
	rb_log->blockSignals(true);
	rb_lin->blockSignals(true);
	rb_log->setChecked(isLog);
	rb_lin->setChecked(!isLog);
	rb_log->blockSignals(false);
	rb_lin->blockSignals(false);
	pb_scaleIn->setEnabled(!isLog);
	pb_scaleOut->setEnabled(!isLog);
}
