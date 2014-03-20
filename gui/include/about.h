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


#ifndef _ABOUT_H
#define _ABOUT_H

#include <QDialog>

class QBoxLayout;
class QTabWidget;
class QTextEdit;
class QLabel;

class AboutDialog : public QDialog
{
	Q_OBJECT
public:
	AboutDialog(QWidget *p=NULL, Qt::WindowFlags fl=0);
	~AboutDialog();

private:
	QBoxLayout	*layout;
	QLabel		*lversion;
	QTabWidget	*tw;
	QTextEdit	*te_licence;
	QTextEdit	*te_about;
};

#endif // _ABOUT_H

