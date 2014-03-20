/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 * */

#ifndef PREF_COLORS_H
#define PREF_COLORS_H

#include <QWidget>
#include <qpxsettings.h>

class QBoxLayout;
class QGridLayout;
class QPushButton;
class QMenu;
class QAction;
class QLineEdit;

class ColorLabel;

class prefColors : public QWidget
{
	Q_OBJECT
public:
	prefColors(QPxSettings *iset, QWidget *p=0, Qt::WindowFlags fl=0);
	~prefColors();

private slots:
	void loadPresets();
	void savePresets();
	void usePreset();
	void addPreset();
	void delPreset();
	void replacePreset();
	void presetNameValidate(QString);

private:
	QGridLayout *layout;

	ColorLabel	*lc_bg;
	ColorLabel	*lc_grid;

	ColorLabel	*lc_rspeed;
	ColorLabel	*lc_wspeed;

	ColorLabel	*lc_errc[8];

	ColorLabel	*lc_jitter;
	ColorLabel	*lc_asymm;

	ColorLabel	*lc_fe;
	ColorLabel	*lc_te;

	ColorLabel	*lc_tapit;
	ColorLabel	*lc_taland;

	QBoxLayout	*layout_presets;
	QPushButton *pb_preset,
				*pb_replace,
				*pb_add,
				*pb_del;
	QLineEdit	*e_preset;
	QMenu		*menu_preset;
	QAction		*act_default;
	QList<QAction*> act_presets;
	QList<colorSet> presets; 

	QPxSettings *set;
};

#endif

