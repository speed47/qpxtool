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

#ifndef __image_label_h
#define __image_label_h

#include <QWidget>

#define HIGHLIGHT_FOCUSED

class QLabel;
class QLayout;
class QBoxLayout;

class ImageLabel : public QWidget
{
	Q_OBJECT

public:
	ImageLabel(int minW, QString label, QImage image, int idx, QWidget *parent);
	ImageLabel(int minW, QSize is, QString label, QImage image, int idx, QWidget *parent);
	ImageLabel(int minW, int iw, int ih, QString label, QImage image, int idx, QWidget *parent);
	~ImageLabel();
	inline void setLabel(QString label) { name = label; };
	inline void setImage(QImage image) { img = image; };

signals:
	void selected(int);

public slots:
	void select(int);

protected:
	void mousePressEvent(QMouseEvent*);

#ifdef HIGHLIGHT_FOCUSED
	void enterEvent(QEvent*);
	void leaveEvent(QEvent*);
#endif
	void paintEvent(QPaintEvent*);

private:
	bool	ck;
#ifdef HIGHLIGHT_FOCUSED
	bool	focus;
#endif
	QString name;
	QImage	img;
	int id;
	int iconw, iconh;
	int minW;
};

#endif

