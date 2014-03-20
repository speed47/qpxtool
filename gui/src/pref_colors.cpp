/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2012 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 * */

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QLineEdit>

#include <colorlabel.h>
#include <qpxsettings.h>
#include "pref_colors.h"
#include <QDebug>

prefColors::prefColors(QPxSettings *iset, QWidget *p, Qt::WindowFlags fl)
	: QWidget(p,fl)
{
	QAction *tact;
#ifndef QT_NO_DEBUG
	qDebug("STA: prefColors()");
#endif
	set = iset;
	QFrame *f;
	int idx=0;

	layout = new QGridLayout(this);
	layout->setMargin(0);
	layout->setSpacing(3);

	lc_bg = new ColorLabel(set->col_bg, tr("Background"), 1, this);
	layout->addWidget(lc_bg,idx,0);
	lc_grid = new ColorLabel(set->col_grid, tr("Grid"), 1, this);
	layout->addWidget(lc_grid,idx,1);
	idx++;

	f = new QFrame(this);
	f->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(f,idx,0,1,2);
	idx++;

	lc_rspeed = new ColorLabel(set->col_rspeed, tr("Read Speed"), 1, this);
	layout->addWidget(lc_rspeed,idx,0);
	lc_wspeed = new ColorLabel(set->col_wspeed, tr("Write Speed"), 1, this);
	layout->addWidget(lc_wspeed,idx,1);
	idx++;

	f = new QFrame(this);
	f->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(f,idx,0,1,2);
	idx++;

	lc_jitter = new ColorLabel(set->col_jitter, tr("Jitter"), 1, this);
	layout->addWidget(lc_jitter,idx,0);
	lc_asymm = new ColorLabel(set->col_asymm, tr("Asymmetry"), 1, this);
	layout->addWidget(lc_asymm,idx,1);
	idx++;

	f = new QFrame(this);
	f->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(f,idx,0,1,2);
	idx++;

	lc_errc[0] = new ColorLabel(*set->col_errc.raw[0], "BLER", 1, this);
	layout->addWidget(lc_errc[0],idx,0);
	lc_errc[4] = new ColorLabel(*set->col_errc.raw[4], "E12/POE/BIS", 1, this);
	layout->addWidget(lc_errc[4],idx,1);
	idx++;
	lc_errc[1] = new ColorLabel(*set->col_errc.raw[1], "E11/PIE/LDC", 1, this);
	layout->addWidget(lc_errc[1],idx,0);
	lc_errc[5] = new ColorLabel(*set->col_errc.raw[5], "E22/POsum8", 1, this);
	layout->addWidget(lc_errc[5],idx,1);
	idx++;
	lc_errc[2] = new ColorLabel(*set->col_errc.raw[2], "E21/PIsum8", 1, this);
	layout->addWidget(lc_errc[2],idx,0);
	lc_errc[6] = new ColorLabel(*set->col_errc.raw[6], "E32/POF", 1, this);
	layout->addWidget(lc_errc[6],idx,1);
	idx++;
	lc_errc[3] = new ColorLabel(*set->col_errc.raw[3], "E31/PIF", 1, this);
	layout->addWidget(lc_errc[3],idx,0);
	lc_errc[7] = new ColorLabel(*set->col_errc.raw[7], "UNCR", 1, this);
	layout->addWidget(lc_errc[7],idx,1);
	idx++;

	f = new QFrame(this);
	f->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(f,idx,0,1,2);
	idx++;

	lc_fe = new ColorLabel(set->col_fe, tr("Focus Errors"), 1, this);
	layout->addWidget(lc_fe,idx,0);
	lc_te = new ColorLabel(set->col_te,tr("Tracking errors"), 1, this);
	layout->addWidget(lc_te,idx,1);
	idx++;

	f = new QFrame(this);
	f->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(f,idx,0,1,2);
	idx++;

	lc_tapit = new ColorLabel(set->col_tapit,  "TA pits", 1, this);
	layout->addWidget(lc_tapit,idx,0);
	lc_taland = new ColorLabel(set->col_taland,"TA lands", 1, this);
	layout->addWidget(lc_taland,idx,1);
	idx++;

	layout->setRowStretch(idx, 10);
	idx++;

	f = new QFrame(this);
	f->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(f,idx,0,1,2);
	idx++;

	loadPresets();

	pb_preset = new QPushButton(tr("Presets"),this);
//	pb_preset->setFlat(true);
	menu_preset = new QMenu(this);
	act_default = menu_preset->addAction(tr("Default"), this, SLOT(usePreset()));
	menu_preset->addSeparator();

	for (int i=0; i<presets.size(); i++) {
		tact = menu_preset->addAction(presets[i].name, this, SLOT(usePreset()));
		act_presets.append(tact);
	}
	pb_preset->setMenu(menu_preset);
	e_preset = new QLineEdit(this);
	e_preset->setMinimumWidth(120);
	e_preset->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	pb_add = new QPushButton(QIcon(":images/add.png"),"",this);
	pb_del = new QPushButton(QIcon(":images/x.png"),"",this);
	pb_replace = new QPushButton(QIcon(":images/save.png"),"",this);

	connect(pb_add, SIGNAL(clicked()), this, SLOT(addPreset()));
	connect(pb_del, SIGNAL(clicked()), this, SLOT(delPreset()));
	connect(pb_replace, SIGNAL(clicked()), this, SLOT(replacePreset()));
	connect(e_preset, SIGNAL(textChanged(QString)), this, SLOT(presetNameValidate(QString)));
	pb_add->setEnabled(false);
	pb_del->setEnabled(false);
	pb_replace->setEnabled(false);

	layout_presets = new QHBoxLayout();
	layout->addLayout(layout_presets, idx, 0, 1, 2);
	layout_presets->addWidget(pb_preset);
	layout_presets->addWidget(e_preset);
	layout_presets->addWidget(pb_replace);
	layout_presets->addWidget(pb_add);
	layout_presets->addWidget(pb_del);


#ifndef QT_NO_DEBUG
	qDebug("END: prefColors()");
#endif
}

prefColors::~prefColors()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~prefColors()");
#endif
	savePresets();

	set->col_bg     = lc_bg->color();
	set->col_bginv  = QColor( (~set->col_bg.red()) & 0xFF, (~set->col_bg.green()) & 0xFF, (~set->col_bg.blue()) & 0xFF );
	set->col_grid   = lc_grid->color();

	set->col_rspeed = lc_rspeed->color();
	set->col_wspeed = lc_wspeed->color();

	for (int i=0;i<8;i++)
		*set->col_errc.raw[i] = lc_errc[i]->color();

	set->col_jitter = lc_jitter->color();
	set->col_asymm  = lc_asymm->color();
	set->col_fe		= lc_fe->color();
	set->col_te		= lc_te->color();
	set->col_tapit  = lc_tapit->color();
	set->col_taland = lc_taland->color();
#ifndef QT_NO_DEBUG
	qDebug("END: ~prefColors()");
#endif
}

void prefColors::addPreset()
{
	QAction *act;
	colorSet newSet;

	newSet.name   = e_preset->text();
	newSet.bg	  = lc_bg->color();
	newSet.grid   = lc_grid->color();
	newSet.rspeed = lc_rspeed->color();
	newSet.wspeed = lc_wspeed->color();

	for (int i=0;i<8;i++)
		newSet.errc[i] = lc_errc[i]->color();

	newSet.jitter = lc_jitter->color();
	newSet.asymm  = lc_asymm->color();
	newSet.fe	  = lc_fe->color();
	newSet.te	  = lc_te->color();
	newSet.tapit  = lc_tapit->color();
	newSet.taland = lc_taland->color();

	act = menu_preset->addAction(newSet.name, this, SLOT(usePreset()));
	presets.append(newSet);
	act_presets.append(act);

	presetNameValidate(newSet.name);
}

void prefColors::delPreset()
{
	QString name = e_preset->text();
	QAction *act;
	int idx = -1;
	for (int p=0; idx<0 && p<presets.size(); p++) {
		if(presets[p].name == name) idx = p;
	}
	if (idx<0) return;
	act = act_presets.takeAt(idx);
	presets.removeAt(idx);

	menu_preset->removeAction(act);
	delete act;

	presetNameValidate(name);
}

void prefColors::replacePreset()
{
	QString name = e_preset->text();
	colorSet *newSet;
	int idx = -1;
	for (int p=0; idx<0 && p<presets.size(); p++) {
		if(presets[p].name == name) idx = p;
	}
	if (idx<0) return;
	newSet = &(presets[idx]);

	newSet->name   = e_preset->text();
	newSet->bg	  = lc_bg->color();
	newSet->grid   = lc_grid->color();
	newSet->rspeed = lc_rspeed->color();
	newSet->wspeed = lc_wspeed->color();

	for (int i=0;i<8;i++)
		newSet->errc[i] = lc_errc[i]->color();

	newSet->jitter = lc_jitter->color();
	newSet->asymm  = lc_asymm->color();
	newSet->fe	  = lc_fe->color();
	newSet->te	  = lc_te->color();
	newSet->tapit  = lc_tapit->color();
	newSet->taland = lc_taland->color();
}

void prefColors::presetNameValidate(QString name)
{
	int idx = -1;
	if (name.isEmpty() || name == "Default") {
		pb_add->setEnabled(false);
		pb_del->setEnabled(false);
		pb_replace->setEnabled(false);
		return;
	}
	for (int p=0; idx<0 && p<presets.size(); p++) {
		if(presets[p].name == name) idx = p;
	}
	pb_add->setEnabled( (idx<0) );
	pb_del->setEnabled( (idx>=0) );
	pb_replace->setEnabled( (idx>=0) );
}

void prefColors::usePreset()
{
	int idx=-1;
	QAction *act = (QAction*) sender();
	const colorSet *preset;
	colorSet newSet;

	if (act == act_default) {
		preset = &defColors;
		e_preset->setText(preset->name);
#ifndef QT_NO_DEBUG
		qDebug() << "Preset: default";
#endif
	} else {
		for (int p=0; p<act_presets.size(); p++) { if (act == act_presets[p]) idx=p; }
		if (idx<0) return;
		preset = &(presets[idx]);
		e_preset->setText(preset->name);
#ifndef QT_NO_DEBUG
		qDebug() << "Preset #" << idx << ":" << preset->name;
#endif
	}

	lc_bg->setColor(preset->bg);
	lc_grid->setColor(preset->grid);

	lc_rspeed->setColor(preset->rspeed);
	lc_wspeed->setColor(preset->wspeed);

	for (int i=0;i<8;i++)
		lc_errc[i]->setColor(preset->errc[i]);

	lc_jitter->setColor(preset->jitter);
	lc_asymm->setColor(preset->asymm);
	lc_fe->setColor(preset->fe);
	lc_te->setColor(preset->te);
	lc_tapit->setColor(preset->tapit);
	lc_taland->setColor(preset->taland);
}


void prefColors::loadPresets()
{
	QSettings *settings;
	QStringList list;
	colorSet newSet;
#ifndef QT_NO_DEBUG
	qDebug("Loading color presets...");
#endif

//	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "qpxtool");
	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "QPxTool", "qpxtool");
	settings->beginGroup("/colors");
	list = settings->value("presets_list", 0).toStringList();
	for (int p=0; p<list.size(); p++) {
		newSet.name = list[p];
		settings->beginGroup("/preset_"+newSet.name);

		newSet.bg     = settings->value("graph_bg",   defColors.bg).toInt();
		newSet.grid   = settings->value("graph_grid", defColors.grid).toInt();
		newSet.rspeed = settings->value("rspeed",   defColors.rspeed).toInt();
		newSet.wspeed = settings->value("wspeed",   defColors.wspeed).toInt();
		for(int i=0; i<8; i++)
			newSet.errc[i] = settings->value(QString("errc%1").arg(i), defColors.errc[i]).toInt();
		newSet.jitter = settings->value("jitter",	 defColors.jitter).toInt();
		newSet.asymm  = settings->value("asymm",	 defColors.asymm).toInt();
		newSet.fe	   = settings->value("fe",		 defColors.fe).toInt();
		newSet.te	   = settings->value("te",		 defColors.te).toInt();
		newSet.tapit  = settings->value("ta_pit",	 defColors.tapit).toInt();
		newSet.taland = settings->value("ta_land",	 defColors.taland).toInt();

		settings->endGroup();
		presets.append(newSet);
	}
	settings->endGroup();

	delete settings;
}

void prefColors::savePresets()
{
	QSettings *settings;
	QStringList list;
#ifndef QT_NO_DEBUG
	qDebug("Saving color presets...");
#endif

//	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "qpxtool");
	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "QPxTool", "qpxtool");
	settings->beginGroup("/colors");
	for (int p=0; p<presets.size(); p++) {
		list << presets[p].name;
		settings->beginGroup("/preset_"+presets[p].name);

		settings->setValue("graph_bg",   presets[p].bg.rgb());
		settings->setValue("graph_grid", presets[p].grid.rgb());
		settings->setValue("rspeed",  	presets[p].rspeed.rgb());
		settings->setValue("wspeed",  	presets[p].wspeed.rgb());
		for(int i=0; i<8; i++)
			settings->setValue(QString("errc%1").arg(i), presets[p].errc[i].rgb());
		settings->setValue("jitter",	presets[p].jitter.rgb());
		settings->setValue("asymm",		presets[p].asymm.rgb());
		settings->setValue("fe",		presets[p].fe.rgb());
		settings->setValue("te",		presets[p].te.rgb());
		settings->setValue("ta_pit",	presets[p].tapit.rgb());
		settings->setValue("ta_land",	presets[p].taland.rgb());

		settings->endGroup();

	}
	settings->setValue("presets_list", list);

	settings->endGroup();

	delete settings;
}

