/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef __ERRC_DETAILED_H
#define __ERRC_DETAILED_H

#include <QDialog>

class QGridLayout;
class QFrame;
class QLabel;
class QPxGraph;

class QPxSettings;
class device;
class devlist;

class ErrcDetailedDialog : public QDialog
{
	Q_OBJECT

public:
	ErrcDetailedDialog(QPxSettings *iset, devlist *idev, QWidget *p=0, Qt::WindowFlags fl = 0);
	~ErrcDetailedDialog();
	void updateAll();
	void updateGraphs(device *idev = NULL);

signals:
	void closed();

protected:
	void hideEvent(QHideEvent*);

protected slots:
	void changeScale();

private:
	devlist		*devices;
	QPxSettings	*settings;
	const char** labels;

	QGridLayout *layout;
	QGridLayout *layout_top;
	QPxGraph	*graph[8];

	QGridLayout *layout_summary;
	QLabel		*pl_tot, *pl_max, *pl_avg;
	QFrame		*hline0, *hline1;
	QLabel		*pl_name[8];
	QLabel		*l_tot[8];
	QLabel		*l_max[8];
	QLabel		*l_avg[8];
};

#endif

