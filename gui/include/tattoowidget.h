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

#ifndef TATTOOWIDGET_H
#define TATTOOWIDGET_H

#define F1TATTOOW 3744
#define F1TATTOOR 582

#include <QWidget>

class TattooWidget : public QWidget
{
	Q_OBJECT
public:
	TattooWidget(QWidget *p);
	~TattooWidget();
	void setRadius(int,int);
	void setImage(QImage&);

private:
	void convertImage();
	int		r0mm,r1mm;
	int		r0p, r1p;
	QImage	timg, dimg;

	int x0,y0;

protected:
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent*);
	float SRC_x2ang(int, int w=F1TATTOOW);
	float SRC_y2rad(int, int);
};

#endif // TATTOOWIDGET_H

