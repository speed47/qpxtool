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
#include "splitbutton.h"

SplitButton::SplitButton(Qt::Orientation o, QWidget *p, Qt::WindowFlags f)
	: QWidget(p,f)
{
	orient = o;
	mouseFocus=0;
	if (orient == Qt::Vertical) {
		setMinimumWidth(6);
		setMaximumWidth(6);
	} else {
		setMinimumHeight(6);
		setMaximumHeight(6);
	}
}

SplitButton::~SplitButton() {}

void SplitButton::paintEvent(QPaintEvent*)
{
//	wDebug("SplitButton::paintEvent()");
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);

	if (mouseFocus) {
		p.fillRect(0, 0, width(), height(), QBrush(palette().color(QPalette::Light)));
	} else {
		p.fillRect(0, 0, width(), height(), QBrush(palette().color(QPalette::Button)));
	}
	p.setPen(QPen(palette().color(QPalette::Dark),1));
	p.drawRect(0, 0, width(), height());
}

void SplitButton::mousePressEvent(QMouseEvent*)
{
//	wDebug("SplitButton::mousePressEvent()");
	emit clicked();
}

void SplitButton::mouseReleaseEvent(QMouseEvent*)
{

}

void SplitButton::enterEvent(QEvent*) {
//	wDebug("SplitButton::enterEvent()");
	mouseFocus=1; update();
}

void SplitButton::leaveEvent(QEvent*) {
//	wDebug("SplitButton::leaveEvent()");
	mouseFocus=0; update();
}

