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

#ifndef COLOR_LABEL_H
#define COLOR_LABEL_H

#include <QColor>
#include <QWidget>

class ColorLabel : public QWidget
{
public:
	ColorLabel(QColor icol, QString itext, bool ieditable, QWidget *p=0, Qt::WindowFlags f=0);
	~ColorLabel();
	inline QColor color() { return col; };
	inline void setColor(QColor c) { col = c; update(); };
	inline void setText(QString t) { text = t; update(); };

protected:
	void mousePressEvent(QMouseEvent*);
	void paintEvent(QPaintEvent*);

private:
	QColor  col;
	QString text;
	bool	editable;
};

#endif

