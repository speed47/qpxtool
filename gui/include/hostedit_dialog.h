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

#ifndef HOSTEDIT_DIALOG_H
#define HOSTEDIT_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

class QLabel;
class QDialogButtonBox;
class QGridLayout;

class hostEditDialog : public QDialog
{
	Q_OBJECT
public:
	hostEditDialog(QString host, int port, QWidget* p=NULL, Qt::WindowFlags f=0);
	~hostEditDialog();
	inline QString hostname() { return e_host->text(); };
	inline int     port()     { return e_port->value(); };

private slots:
	void setPortDfl();
	void hostChanged(QString&);

private:
	QGridLayout			*layout;
	QLabel				*l_host;
	QLineEdit			*e_host;
	QLabel				*l_port;
	QSpinBox			*e_port;
	QPushButton			*bdef;
	QDialogButtonBox	*bbox;
};

#endif

