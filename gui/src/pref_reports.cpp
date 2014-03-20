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
#include "pref_reports.h"
#include <QDebug>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QLabel>
#include <QFrame>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>

#include <QFileDialog>

#include <QMessageBox>
#include <QSqlDatabase>
#include <db_connection.h>

prefReports::prefReports(QPxSettings *iset, QWidget *p, Qt::WindowFlags fl)
	: QWidget(p,fl)
{

#ifndef QT_NO_DEBUG
	qDebug("STA: prefCommon()");
#endif
	set = iset;

	layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(3);

// reports -> files
	box_rep = new QGroupBox(tr("Autosave reports"),this);
	box_rep->setCheckable(true);
	layout->addWidget(box_rep);

	layout_rep = new QVBoxLayout(box_rep);
	layout_rep->setMargin(3);
	layout_rep->setSpacing(3);

	layout_rep_path = new QHBoxLayout();
	layout_rep_path->setMargin(0);
	layout_rep_path->setSpacing(3);
	layout_rep->addLayout(layout_rep_path);

	l_rep_path  = new QLabel(tr("Path:"),box_rep);
	e_rep_path  = new QLineEdit(box_rep);
	pb_rep_path = new QPushButton(QIcon(":images/directory.png"), "", box_rep);
	pb_rep_path->setToolTip(tr("Select directory for reports saving..."));

	layout_rep_path->addWidget(l_rep_path);
	layout_rep_path->addWidget(e_rep_path);
	layout_rep_path->addWidget(pb_rep_path);

// reports -> database
	box_rep_db = new QGroupBox(tr("Use reports database"),this);
	box_rep_db->setCheckable(true);
	layout->addWidget(box_rep_db);

    layout_db = new QGridLayout(box_rep_db);
    layout_db->setMargin(3);
    layout_db->setSpacing(3);
// DB driver
    ldb_driver = new QLabel( tr("Driver:"), this);
    ldb_driver->setMinimumSize(40,22);
    ldb_driver->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout_db->addWidget(ldb_driver, 0, 0);

    db_driver = new QComboBox(this);
    db_driver->setMinimumSize(150,22);
    db_driver->addItems(QSqlDatabase::drivers());
    layout_db->addWidget(db_driver, 0, 1, 1, 2);

// DB host
    ldb_host = new QLabel( tr("Host:"), this);
    ldb_host->setMinimumSize(40,22);
    ldb_host->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout_db->addWidget(ldb_host, 1, 0);

    db_host = new QLineEdit(this);
    db_host->setMinimumSize(150,22);
    layout_db->addWidget(db_host, 1, 1, 1, 2);

// DB port
    ldb_port = new QLabel( tr("Port:"), this);
    ldb_port->setMinimumSize(40,22);
    ldb_port->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout_db->addWidget(ldb_port, 2, 0);

    db_port = new QSpinBox(this);
    db_port->setMinimumSize(40,22);
    db_port->setMinimum(1);
    db_port->setMaximum(65535);
    layout_db->addWidget(db_port, 2, 1);

// DB name
    ldb_name = new QLabel( tr("DB name:"), this);
    ldb_name->setMinimumSize(40,22);
    ldb_name->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout_db->addWidget(ldb_name, 3, 0);

    db_name= new QLineEdit(this);
    db_name->setMinimumSize(150,22);
    layout_db->addWidget(db_name, 3, 1, 1, 2);

// user
    ldb_user = new QLabel( tr("User:"), this);
    ldb_user->setMinimumSize(40,22);
    ldb_user->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout_db->addWidget(ldb_user, 4, 0);

    db_user = new QLineEdit(this);
    db_user->setMinimumSize(150,22);
    layout_db->addWidget(db_user, 4, 1, 1, 2);

// passwd
    ldb_pass = new QLabel( tr("Password:"), this);
    ldb_pass->setMinimumSize(40,22);
    ldb_pass->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout_db->addWidget(ldb_pass, 5, 0);

    db_pass = new QLineEdit(this);
    db_pass->setMinimumSize(150,22);
    db_pass->setEchoMode(QLineEdit::Password);
    layout_db->addWidget(db_pass, 5, 1, 1, 2);

// DB connection check button
    pb_db_check = new QPushButton(QIcon(":images/ok.png"), tr("Check"), this);
    pb_db_check->setMinimumSize(40,22);
    layout_db->addWidget(pb_db_check, 6, 2);

	ck_autosave_db = new QCheckBox(tr("Autosave reports into database"), box_rep);
    layout_db->addWidget(ck_autosave_db, 7, 0, 1, 3);
// common
	ck_eject = new QCheckBox(tr("Eject media after tests finished"), box_rep);
	layout->addWidget(ck_eject);

	layout->addStretch(10);

// creating objects connections...
	connect(box_rep,        SIGNAL(toggled(bool)), this, SLOT(box_rep_toggled(bool)));
	connect(box_rep_db,     SIGNAL(toggled(bool)), this, SLOT(box_rep_toggled(bool)));
	connect(ck_autosave_db, SIGNAL(toggled(bool)), this, SLOT(box_rep_toggled(bool)));

	connect(pb_rep_path, SIGNAL(clicked()), this, SLOT(select_rep_path()));
	connect(pb_db_check, SIGNAL(clicked()), this, SLOT(check_db_connection()));

// applying settings...

	box_rep->setChecked(set->report_autosave);
	e_rep_path->setText(set->report_path);
	box_rep_db->setChecked(set->use_reports_db);
	ck_autosave_db->setChecked(set->report_autosave_db);

    db_driver->setCurrentIndex( QSqlDatabase::drivers().indexOf(set->db.driver) );
	db_host->setText(set->db.host);
	db_port->setValue(set->db.port);
	db_name->setText(set->db.name);
	db_user->setText(set->db.user);
	db_pass->setText(set->db.pass);

	ck_eject->setChecked(set->actions_flags & AFLAG_EJECT_AFTER);
}

prefReports::~prefReports()
{
	if (ck_eject->isChecked()) set->actions_flags |= AFLAG_EJECT_AFTER; else set->actions_flags &= ~AFLAG_EJECT_AFTER;

	set->report_path = e_rep_path->text();
	set->report_autosave = box_rep->isChecked();
	set->use_reports_db = box_rep_db->isChecked();
	set->report_autosave_db = set->use_reports_db && ck_autosave_db->isChecked();
}

void prefReports::select_rep_path()
{
#ifndef QT_NO_DEBUG
	qDebug() << __func__;
#endif
	QString path = QFileDialog::getExistingDirectory(this, tr("Select directory..."), e_rep_path->text());
	if (path.isEmpty()) return;
	e_rep_path->setText(path);
}

void prefReports::box_rep_toggled(bool)
{
	ck_eject->setEnabled(box_rep->isChecked() || (box_rep_db->isChecked() && ck_autosave_db->isChecked()));
	if(!ck_eject->isEnabled()) ck_eject->setChecked(false);
}

void prefReports::check_db_connection()
{
#ifndef QT_NO_DEBUG
	qDebug() << __func__;
#endif
	DBParams params;

	params.driver = db_driver->currentText();
	params.host = db_host->text();
	params.port = db_port->value();
	params.name = db_name->text();
	params.user = db_user->text();
	params.pass = db_pass->text();

	if (SqlProbeConnection(params, "test connection")) {
		set->db = params;
		QMessageBox::information(this,
		    tr("Success!"),
		    tr("Database connection successfully\nestablished with given parameters"));
	}
}

