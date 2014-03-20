/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef _DEVSETTINGS_H
#define _DEVSETTINGS_H

#include <QDialog>

class QBoxLayout;
class QGridLayout;

class QTreeWidget;
class QTreeWidgetItem;
class ImagesList;

class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QPushButton;

class TextSlider;
class QPxSettings;
class device;

class devSettings : public QDialog
{
	Q_OBJECT
public:
	devSettings(QPxSettings *iset, device *idev, QWidget *p=0, Qt::WindowFlags fl = 0);
	~devSettings();

public slots:

private slots:
	void setPage(int);

private:
	QBoxLayout  *layout;
	QBoxLayout  *layoutc;
	ImagesList	*ilist;
	QWidget     *cpage;

	QPxSettings	*set;
	device		*dev;
};

#endif

