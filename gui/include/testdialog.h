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

#ifndef TEST_DIALOG_H
#define TEST_DIALOG_H

#include <QDialog>
#include <qpxsettings.h>
#include <device.h>

class QBoxLayout;
class QGridLayout;
class QGroupBox;

class QFrame;
class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QTextBrowser;
class QPushButton;

class TestDialog : public QDialog 
{
	Q_OBJECT

public:
	TestDialog(QPxSettings *iset, device *idev, QWidget *p, Qt::WindowFlags f=0);
	~TestDialog();
	device* getDevice();

public slots:
	void mediaChanged();

private slots:
	void start();
	void updateData(bool save=1, bool setPlugin=1);
	void WTchecked(bool);
	void pluginChanged(int);
	void checkSimul();

private:
	void winit();
	void saveData();
	bool			noSimul;

	QGridLayout		*layout;

	QLabel			*ldev,
					*devid,
					*lmedia,
					*media,
					*llabel;
	QLineEdit		*elabel;

	QGroupBox		*grp_tests;
	QGridLayout		*layout_tests;
	QLabel			*l_tests;
	QLabel			*l_speeds;

	QCheckBox		*ck_RT;
	QComboBox		*spd_RT;
	QCheckBox		*ck_WT;
	QComboBox		*spd_WT;
	QCheckBox		*ck_WT_simul;
	QFrame			*hline0;
	QCheckBox		*ck_ERRC;
	QComboBox		*spd_ERRC;
	QCheckBox		*ck_JB;
	QComboBox		*spd_JB;
	QCheckBox		*ck_FT;
	QComboBox		*spd_FT;
	QCheckBox		*ck_TA;
//	QComboBox		*spd_TA;

	QFrame			*hline1;
	QLabel			*l_plugin;
	QComboBox		*cb_plugin;
	QLabel			*l_plugin_info;
/*
	QGroupBox		*grp_media;
	QBoxLayout		*layout_media;
	QTextBrowser	*media;
*/

	QBoxLayout		*layout_butt;
	QPushButton		*butt_run;
	QPushButton		*butt_cancel;

//
	device			*dev;
	QPxSettings		*settings;
};

#endif // TEST_DIALOG_H

