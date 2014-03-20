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

#ifndef _TEXT_SLIDER_H
#define _TEXT_SLIDER_H

#include <QAbstractSlider>
#include <QStyleOptionSlider>

class SliderItem {
public:
	SliderItem(QString s, bool en = true) { text=s; enabled=en; }

	inline bool operator==(SliderItem o) { return text==o.text; };
	inline bool operator==(SliderItem& o) { return text==o.text; };
	inline bool operator==(QString o) { return text==o; };
	inline bool operator==(QString& o) { return text==o; };

	QString text;
	bool	enabled;
};


class TextSlider : public QAbstractSlider
{
	Q_OBJECT
public:
	TextSlider(QWidget *p = NULL);
	TextSlider(Qt::Orientation, QWidget *p = NULL);
	void addItem(QString, bool en = true);
	void setItemEnabled(int, bool);
	void setItemEnabled(QString, bool);
	inline void removeItem(QString s) { removeItem(items.indexOf(s)); };
	void removeItem(int);
	void setTickPosition(QSlider::TickPosition);
	void setUpsideDown(bool);
	void setOrientation(Qt::Orientation);
	void first();
	void last();
	bool prev();
	bool next();
	int  value();
	QString text();

public slots:
	void setValue(int);
	void setCurrentItem(const QString&);
	void setEnabled(bool);
	void setDisabled(bool);

protected:
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);
	void keyPressEvent(QKeyEvent*);
	void focusInEvent(QFocusEvent*);
	void focusOutEvent(QFocusEvent*);
	void paintEvent(QPaintEvent*);

private:
	void initDefaults();
	void moveToEnabled();
	int  oldpos;
	bool en;

	QStyleOptionSlider op;
	QList<SliderItem> items;
};

#endif

