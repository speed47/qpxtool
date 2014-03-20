/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <qpxsettings.h>
#include "pref_common.h"
#include <QDebug>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#include <QLabel>
#include <QCheckBox>

prefCommon::prefCommon(QPxSettings *iset, QWidget *p, Qt::WindowFlags fl)
	: QWidget(p,fl)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: prefCommon()");
#endif
	set = iset;

	layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(3);

// default actions
	ck_autow = new QCheckBox(tr("Autostart tests on written media inserted"), this);
	ck_autob = new QCheckBox(tr("Autostart tests on blank media inserted"), this);

	layout->addWidget(ck_autow);
	layout->addWidget(ck_autob);

// default tests for written media
	pl_testsw = new QLabel("<b>" + tr("Default tests for written media") + "</b>",this);
	pl_testsw->setAlignment(Qt::AlignCenter);
	pl_testsw->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout->addWidget(pl_testsw);

	ck_rt    = new QCheckBox(tr("Read Transfer Rate"),this);
	ck_errc  = new QCheckBox(tr("Error Correction"),this);
	ck_jb    = new QCheckBox(tr("Jitter/Asymmetry"),this);
	ck_ftw   = new QCheckBox(tr("Focus/Tracking"),this);
	ck_ta    = new QCheckBox(tr("Time Analyser"),this);
	layout->addWidget(ck_rt);
	layout->addWidget(ck_errc);
	layout->addWidget(ck_jb);
	layout->addWidget(ck_ftw);
	layout->addWidget(ck_ta);

// default tests for blank media
	pl_testsb = new QLabel("<b>" + tr("Default tests for blank media") + "</b>",this);
	pl_testsb->setAlignment(Qt::AlignCenter);
	pl_testsb->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout->addWidget(pl_testsb);
	layout_testsb = new QGridLayout();
	layout_testsb->setMargin(0);
	layout_testsb->setSpacing(3);
	layout->addLayout(layout_testsb);

	ck_wt    = new QCheckBox(tr("Write Transfer Rate"),this);
	ck_wt_simul = new QCheckBox(tr("Simulation"),this); ck_wt_simul->setEnabled(false);
	ck_ftb   = new QCheckBox(tr("Focus/Tracking"),this);
	layout_testsb->addWidget(ck_wt,    0,0,1,2);
	layout_testsb->addWidget(ck_wt_simul,1,1);
	layout_testsb->addWidget(ck_ftb,   2,0,1,2);

	layout_testsb->setColumnStretch(0,1);
	layout_testsb->setColumnStretch(1,10);
	layout->addStretch(10);

// creating objects connections...
	connect(ck_wt, SIGNAL(toggled(bool)), ck_wt_simul, SLOT(setEnabled(bool)));

// applying settings...
	ck_autow->setChecked(set->actions_flags & AFLAG_AUTOSTART_W);
	ck_autob->setChecked(set->actions_flags & AFLAG_AUTOSTART_B);

	ck_rt->setChecked(set->actions_flags & AFLAG_DTEST_RT);
	ck_wt->setChecked(set->actions_flags & AFLAG_DTEST_WT);
	ck_wt_simul->setChecked(set->actions_flags & AFLAG_DTEST_WT_SIMUL);
	ck_errc->setChecked(set->actions_flags & AFLAG_DTEST_ERRC);
	ck_jb->setChecked(set->actions_flags & AFLAG_DTEST_JB);
	ck_ftw->setChecked(set->actions_flags & AFLAG_DTEST_FT_W);
	ck_ftb->setChecked(set->actions_flags & AFLAG_DTEST_FT_B);
	ck_ta->setChecked(set->actions_flags & AFLAG_DTEST_TA);
#ifndef QT_NO_DEBUG
	qDebug("END: prefCommon()");
#endif
}

prefCommon::~prefCommon()
{
	if (ck_autow->isChecked()) set->actions_flags |= AFLAG_AUTOSTART_W; else set->actions_flags &= ~AFLAG_AUTOSTART_W;
	if (ck_autob->isChecked()) set->actions_flags |= AFLAG_AUTOSTART_B; else set->actions_flags &= ~AFLAG_AUTOSTART_B;

	if (ck_rt->isChecked()) set->actions_flags |= AFLAG_DTEST_RT; else set->actions_flags &= ~AFLAG_DTEST_RT;
	if (ck_wt->isChecked()) set->actions_flags |= AFLAG_DTEST_WT; else set->actions_flags &= ~AFLAG_DTEST_WT;
	if (ck_wt_simul->isChecked()) set->actions_flags |= AFLAG_DTEST_WT_SIMUL; else set->actions_flags &= ~AFLAG_DTEST_WT_SIMUL;
	if (ck_errc->isChecked()) set->actions_flags |= AFLAG_DTEST_ERRC; else set->actions_flags &= ~AFLAG_DTEST_ERRC;
	if (ck_jb->isChecked()) set->actions_flags |= AFLAG_DTEST_JB; else set->actions_flags &= ~AFLAG_DTEST_JB;
	if (ck_ftw->isChecked()) set->actions_flags |= AFLAG_DTEST_FT_W; else set->actions_flags &= ~AFLAG_DTEST_FT_W;
	if (ck_ftb->isChecked()) set->actions_flags |= AFLAG_DTEST_FT_B; else set->actions_flags &= ~AFLAG_DTEST_FT_B;
	if (ck_ta->isChecked()) set->actions_flags |= AFLAG_DTEST_TA; else set->actions_flags &= ~AFLAG_DTEST_TA;
}

