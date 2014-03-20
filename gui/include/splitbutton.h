/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef SPLITBUTTON_H
#define SPLITBUTTON_H

#include <QWidget>

class SplitButton : public QWidget
{
	Q_OBJECT

public:
	SplitButton(Qt::Orientation o, QWidget *p=0, Qt::WindowFlags f=0);
	~SplitButton();

protected:
	void paintEvent(QPaintEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void enterEvent(QEvent*);
	void leaveEvent(QEvent*);

private:
	Qt::Orientation orient;
	bool mouseFocus;

signals:
	void clicked();
};

#endif

