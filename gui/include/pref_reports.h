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

#ifndef PREF_REPORTS_H
#define PREF_REPORTS_H

#include <QWidget>
#include <qpxsettings.h>

class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;

class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QComboBox;
class QSpinBox;

class prefReports : public QWidget
{
	Q_OBJECT

public:
	prefReports(QPxSettings *iset, QWidget *p=0, Qt::WindowFlags fl=0);
	~prefReports();

private slots:
	void select_rep_path();
	void box_rep_toggled(bool);
	void check_db_connection();
private:
	QVBoxLayout	*layout;

	QGroupBox	*box_rep;
	QVBoxLayout	*layout_rep;
	QHBoxLayout	*layout_rep_path;

	QLabel		*l_rep_path;
	QLineEdit	*e_rep_path;
	QPushButton	*pb_rep_path;

// database settings
	QGroupBox	*box_rep_db;
	QGridLayout *layout_db;
	QLabel		*ldb_driver,
				*ldb_host,
				*ldb_port,
				*ldb_name,
				*ldb_user,
				*ldb_pass;
	QComboBox	*db_driver;
	QLineEdit	*db_host,
				*db_name,
				*db_user,
				*db_pass;
	QSpinBox	*db_port;
	QPushButton *pb_db_check;
	QCheckBox	*ck_autosave_db;

	QCheckBox	*ck_eject;

	QPxSettings *set;
};

#endif // PREF_REPORTS_H

