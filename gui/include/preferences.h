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

#ifndef _PREFERENCES_H
#define _PREFERENCES_H

#define PREF_PAGES 4

#include <QDialog>
#include <qpxsettings.h>

class QFrame;
class ImagesList;
class QHBoxLayout;
class QVBoxLayout;
class QScrollArea;
class QPushButton;

class QPxPreferences : public QDialog {
	Q_OBJECT

public:
	QPxPreferences(QPxSettings *iset, QWidget *p=0, Qt::WindowFlags f=0);
	virtual ~QPxPreferences();

public slots:
	void setPage(int);

private slots:
	void save();
	void cancel();

protected:
	void closeEvent(QCloseEvent*);
	void keyPressEvent(QKeyEvent*);

private:
	void winit();
	int curPage;

	QPxSettings set;
	QPxSettings *set_old;

	QHBoxLayout	*layout;
	ImagesList	*ilist;

	QVBoxLayout	*parea;
	QFrame		*hline0;
	QHBoxLayout	*layout_butt;
	QPushButton *pb_save,
				*pb_cancel;
//	QWidget*	pagew;
	QWidget*	pages[PREF_PAGES];
};

#endif

