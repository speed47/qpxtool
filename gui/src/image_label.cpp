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

#include <QLabel>
//#include <QFrame>
#include <QBoxLayout>

#include "image_label.h"
#include <QDebug>

#include <QPainter>

#define MAX_IMAGE_SIZE 64

ImageLabel::ImageLabel(int iminW, QString label, QImage image, int idx, QWidget *parent)
		: QWidget (parent)
{
#ifndef QT_NO_DEBUG
	qDebug("ImageLabel()");
#endif
	iconw = 64;
	iconh = 64;
	minW = qMax(iconw+4, iminW);
	setMinimumSize(minW, iconh+24);
	name = label;
	img  = image;
	id = idx;
	focus = 0;
	if (!id) ck=1; else ck=0;
}

	
ImageLabel::ImageLabel(int iminW, QSize is, QString label, QImage image, int idx, QWidget *parent)
		: QWidget (parent)
{
#ifndef QT_NO_DEBUG
	qDebug("ImageLabel()");
#endif
	iconw = is.width();
	iconh = is.height();
	minW = qMax(iconw+4, iminW);
	setMinimumSize(minW, iconh+24);
	name = label;
	img  = image;
	id = idx;
	focus = 0;
	if (!id) ck=1; else ck=0;
}

ImageLabel::ImageLabel(int iminW, int iw, int ih, QString label, QImage image, int idx, QWidget *parent)
		: QWidget (parent)
{
#ifndef QT_NO_DEBUG
	qDebug("ImageLabel()");
#endif
	iconw = iw;
	iconh = ih;
	minW = qMax(iconw+4, iminW);
	setMinimumSize(minW, iconh+24);
	name = label;
	img  = image;
	id = idx;
	focus = 0;
	if (!id) ck=1; else ck=0;
}

ImageLabel::~ImageLabel()
{
#ifndef QT_NO_DEBUG
	qDebug("~ImageLabel()");
#endif
}

void ImageLabel::mousePressEvent(QMouseEvent*)
{
	if (ck) return;
	ck = 1;
	update();
	emit selected(id);
}

void ImageLabel::select(int idx)
{
	if (ck == ((idx == id))) return;
#ifndef QT_NO_DEBUG
//	qDebug(QString("ImageLabel::select(%1) %2").arg(idx).arg(id));
#endif
	ck = (idx == id);
	update();
};

#ifdef HIGHLIGHT_FOCUSED
void ImageLabel::enterEvent(QEvent*)
{
	focus = 1;
	update();
}

void ImageLabel::leaveEvent(QEvent*)
{
	focus = 0;
	update();
}
#endif

void ImageLabel::paintEvent(QPaintEvent*)
{
//	qDebug("paintEvent()");
	QPainter p(this);
//	qDebug("ImageLabel::paintEvent()");

//	p.setRenderHint(QPainter::Antialiasing, true);
	if (ck)
		p.fillRect( QRect(1,1, width()-2, height()-2), QBrush( palette().color(QPalette::Highlight) ) );

#ifdef HIGHLIGHT_FOCUSED
	if (focus) {
		if (ck)
			p.setPen( QPen( palette().color(QPalette::HighlightedText), 2, Qt::SolidLine) );
		else
			p.setPen( QPen( palette().color(QPalette::Highlight), 2, Qt::SolidLine) );
		p.drawRect( QRect(1,1, width()-2, height()-2) );
	}
#endif
	p.drawImage( ((width()-img.width()) >> 1), ((iconh-img.height()) >> 1) + 2, img);

	if (ck)
		p.setPen( QPen( palette().color(QPalette::HighlightedText), 2, Qt::SolidLine) );
	else
		p.setPen( QPen( palette().color(QPalette::Text), 2, Qt::SolidLine) );
	p.drawText(2, iconh+3, width()-4, height()-iconh, Qt::AlignHCenter | Qt::AlignVCenter, name);

}

