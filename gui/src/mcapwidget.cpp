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

#include <QPainter>

#include "mcapwidget.h"

#define SHOW_X
#define MAX_ICON_SIZE 16

MediaCapWidget::MediaCapWidget(QString itext, bool tri, quint64 df, QWidget *p, Qt::WindowFlags f)
	: QWidget(p,f)
{
	tristate = tri;
	text = itext;
	cf = df;
	rd=0, wr=0;
	setMinimumSize(128, MAX_ICON_SIZE+4);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
};

MediaCapWidget::~MediaCapWidget() {};

void MediaCapWidget::setText(QString itext)
{
	text=itext;
	update();
};

QSize MediaCapWidget::sizeHint() const
{
	return QSize( MAX_ICON_SIZE + 20 + fontMetrics().width(text), MAX_ICON_SIZE+2);
}

void MediaCapWidget::setR(quint64 r) {
	if (!tristate) return;
	rd= ((cf & r));
	setRW();
};

void MediaCapWidget::setW(quint64 w) {
	if (!tristate) return;
	wr= ((cf & w));
	setRW();
};

void MediaCapWidget::setRW() {
	if (wr) {
		icon.load(":images/cdwriter.png");
	} else if (rd) {
		icon.load(":images/disc.png");
	} else {
#ifdef SHOW_X
		icon.load(":images/x.png");
#else
		icon = QImage();
#endif
	}

	icon = icon.scaled( QSize(MAX_ICON_SIZE, MAX_ICON_SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation );
	update();
};

void MediaCapWidget::setCap(quint64 s) {
	if (tristate) return;
	rd= (( cf & s));

	if (rd) {
		icon.load(":images/ok.png");
		icon = QImage(":images/ok.png");
	} else {
#ifdef SHOW_X
		icon.load(":images/x.png");
		//icon = QImage(":images/x.png");
#else
		icon = QImage();
		//icon.clear();
#endif
	}

	icon = icon.scaled( QSize(MAX_ICON_SIZE, MAX_ICON_SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation );
	update();
};

void MediaCapWidget::clear() {
	icon = QImage();
	//icon.clear();
	update();
};

void MediaCapWidget::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	if (cf) p.drawImage( 4, ((height()-icon.height()) >> 1) + 2, icon);
	p.drawText(MAX_ICON_SIZE + 9, 4, width()-MAX_ICON_SIZE-13, height()-4, Qt::AlignLeft | Qt::AlignVCenter, text);
};

