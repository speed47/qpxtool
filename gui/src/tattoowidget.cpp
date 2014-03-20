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

#include <math.h>

#include <QPainter>
#include <QDebug>

#include "tattoowidget.h"

TattooWidget::TattooWidget( QWidget *p)
	: QWidget(p)
{
	r0mm = F1TATTOOR/3;
	r1mm = F1TATTOOR;
}

TattooWidget::~TattooWidget() {}

void TattooWidget::setRadius(int ir0, int ir1)
{
	r0mm = ir0; r1mm = ir1;
	convertImage();
	update();
}

void TattooWidget::setImage(QImage& iimg)
{
	timg = iimg;
	convertImage();
	update();
}

#define CONVERT_DIRECT

void TattooWidget::convertImage()
{
	x0 = width()/2;
	y0 = height()/2;
	if (width() < height()) {
		r1p = x0 - 4;
	} else {
		r1p = y0 - 4;
	}


	r0p = (int)((float)r1p * r0mm / r1mm);

	dimg = QImage(r1p<<1, r1p<<1, QImage::Format_ARGB32);
	dimg.fill(0xFFFFFFFF);

	float ang, rad;
	int h=timg.height();
	int w=timg.width();

#ifndef CONVERT_DIRECT
	QPainter p;
	p.begin(&dimg);
	p.setRenderHint(QPainter::Antialiasing);
#endif
	float x1,y1;
	if (w & h) {
#ifndef QT_NO_DEBUG
	qDebug() << "Converting image " << w << "x" << h << " to radial...";
#endif
		for(int y=0; y<h; y++) {
			rad = SRC_y2rad(h-y-1, h)-2;
			for(int x=0; x<w; x++) {
				ang = SRC_x2ang(x);
				x1 = r1p + rad * sin(ang);
				y1 = r1p - rad * cos(ang);
#ifdef CONVERT_DIRECT
				dimg.setPixel((int)x1,(int)y1, timg.pixel(x,y));
#else
				p.setPen(QPen(QColor(timg.pixel(x,y)), 1));
				p.drawPoint(QPointF(x1,y1));
#endif
			}
		}
#ifndef QT_NO_DEBUG
	} else {
		qDebug() << "Zero size image!";
#endif
	}
#ifndef CONVERT_DIRECT
	p.end();
#endif
}

float TattooWidget::SRC_x2ang(int x, int w) { return (float)((double)x * M_PI * 2 / w);  }
float TattooWidget::SRC_y2rad(int y, int h) { return (float)y/(float)h * (r1p-r0p+1) + r0p; }

void TattooWidget::resizeEvent(QResizeEvent*)
{
	convertImage();
}

void TattooWidget::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);
// background
	p.fillRect(0, 0, width(), height(), QBrush(Qt::white));
	p.setPen(QPen(palette().color(QPalette::Dark), 1));
	p.drawRect(0, 0, width(), height());

// image
	if (dimg.height()) p.drawImage(x0-r1p, y0-r1p, dimg);
	p.drawEllipse(x0 - r0p, y0 - r0p, r0p<<1, r0p<<1);

// disc
	p.setPen(QPen((Qt::gray), 1));
	p.drawEllipse(x0 - r1p/10, y0 - r1p/10, r1p/5, r1p/5);
	p.drawEllipse(x0 - r1p/3, y0 - r1p/3, (r1p<<1)/3, (r1p<<1)/3);

	p.drawEllipse(x0 - r1p, y0 - r1p, r1p<<1, r1p<<1);
}

