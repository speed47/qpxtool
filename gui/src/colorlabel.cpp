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

#include <QColorDialog>
#include <QPainter>
#include <QMouseEvent>

#include "colorlabel.h"
#include <QDebug>

ColorLabel::ColorLabel(QColor icol, QString itext, bool ieditable, QWidget* p, Qt::WindowFlags f) : QWidget(p, f) {
	col = icol;
	text = itext;
	editable = ieditable;
	setMinimumSize(40, 20);
}

ColorLabel::~ColorLabel() {}

void ColorLabel::mousePressEvent(QMouseEvent* e) {
	if (!editable) return;
#ifndef QT_NO_DEBUG
	qDebug() << "x: " << e->position().x() << " y: " << e->position().y();
#endif
	//	if (e->position().x() < height() && e->position().x() > 1 && e->position().y() > 1 && e->position().y() < height()) {
	if (e->button() == Qt::LeftButton && e->position().x() < (height() + 2)) {
		col = QColorDialog::getColor(col, this);
		update();
	}
}

QSize ColorLabel::sizeHint() const {
	QFontMetrics fm(font());
	int h = qMax(20, fm.height() + 4);
	int w = h + 4 + fm.horizontalAdvance(text) + 2;
	return QSize(w, h);
}


void ColorLabel::paintEvent(QPaintEvent*) {
	QRect rect(1, 1, height() - 2, height() - 2);
	QRect trect(height() + 4, 1, width() - height() - 6, height() - 2);
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setRenderHint(QPainter::TextAntialiasing, true);

	p.setPen(QPen(col, 2));
	p.fillRect(rect.adjusted(1, 1, -1, -1), QBrush(col));

	p.setPen(QPen(QColor(Qt::black), 2));
	p.drawRoundedRect(rect, 20, 20);

	p.drawText(trect, Qt::AlignLeft | Qt::AlignVCenter, text);
}
