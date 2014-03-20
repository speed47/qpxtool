/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 * */

#ifndef PREF_DEVICES_H
#define PREF_DEVICES_H

#include <QWidget>

class QLabel;
class QCheckBox;
class QGroupBox;
class QTreeWidget;

class QBoxLayout;
class QPxSettings;

class prefDevices : public QWidget
{
	Q_OBJECT
public:
	prefDevices(QPxSettings *iset, QWidget *p=0, Qt::WindowFlags f=0);
	virtual ~prefDevices();

protected:
	bool eventFilter(QObject* obj, QEvent* e);
	void hostsContextMenu(QContextMenuEvent* e);

private slots:
	void hostAdd();
	void hostEdit();
	void hostRemove();

private:
	QBoxLayout  *layout;

	QCheckBox	*ck_local;
	QCheckBox	*ck_remote;

//	QGroupBox	*g_hosts;
//	QBoxLayout	*l_hosts;
	QTreeWidget	*lst_hosts;

	QPxSettings *set;
};

#endif

