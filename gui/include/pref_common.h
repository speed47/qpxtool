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

#ifndef PREF_COMMON_H
#define PREF_COMMON_H

#include <QWidget>
#include <qpxsettings.h>

class QVBoxLayout;
class QGridLayout;

class QGroupBox;
class QLabel;
class QCheckBox;

class prefCommon : public QWidget
{
	Q_OBJECT

public:
	prefCommon(QPxSettings *iset, QWidget *p=0, Qt::WindowFlags fl=0);
	~prefCommon();

private:
	QVBoxLayout	*layout;

	QCheckBox	*ck_autow,
				*ck_autob;

	QLabel		*pl_testsw,
				*pl_testsb;
	QGridLayout	*layout_testsb;

	QCheckBox	*ck_rt,
				*ck_wt,
				*ck_wt_simul,
				*ck_errc,
				*ck_jb,
				*ck_ftb,
				*ck_ftw,
				*ck_ta;

	QPxSettings *set;
};

#endif

