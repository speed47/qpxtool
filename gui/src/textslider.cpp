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

#include <QPainter>
#include <QStyle>

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

#include <QDebug>
#include "textslider.h"

#define VITEMSIZE 16
#define HITEMSIZE 30

TextSlider::TextSlider(QWidget *p)
	: QAbstractSlider(p)
{
	initDefaults();
}
	  
TextSlider::TextSlider(Qt::Orientation orient, QWidget *p)
	: QAbstractSlider(p)
{
	initDefaults();
	op.orientation = orient;
	if (orient == Qt::Horizontal)
	    setMinimumHeight(40);
}
	  
void TextSlider::initDefaults()
{
	en = 1;
	op.state = QStyle::State_Active | QStyle::State_Enabled;
	op.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
	op.activeSubControls = op.subControls;
	op.orientation = Qt::Vertical;
	op.pageStep = 1;
	op.singleStep = 1;
	op.tickInterval = 1;
	op.tickPosition = QSlider::TicksLeft;
	op.upsideDown = true;
	op.minimum = 0;
	op.maximum = 0;
	op.sliderPosition = 0;
	items.clear();
	setFocusPolicy(Qt::StrongFocus);
}

void TextSlider::addItem(QString item, bool en)
{
	if (!items.contains(item)) items << SliderItem(item,en);
	op.maximum = items.size() -1;
	if (items.size()) op.subControls |= QStyle::SC_SliderTickmarks;

	moveToEnabled();
	if (op.orientation == Qt::Vertical) {
		setMinimumHeight(items.size() * VITEMSIZE);
	} else {
		setMinimumWidth(items.size() * HITEMSIZE);
	}
	update();
}

void TextSlider::removeItem(int idx)
{
	if (idx < 0 || idx >=items.size()) return;
	items.removeAt(idx);
	op.maximum = items.size() -1;
	if (!items.size()) op.subControls &= ~QStyle::SC_SliderTickmarks;

	moveToEnabled();
	if (op.orientation == Qt::Vertical) {
		setMinimumHeight(items.size() * VITEMSIZE);
	} else {
		setMinimumWidth(items.size() * HITEMSIZE);
	}
	update();
}

void TextSlider::setItemEnabled(int idx, bool en)
{
	if (idx<0 || idx>=items.size()) return;
	items[idx].enabled = en;
	if(!en) moveToEnabled();
	update();
}

void TextSlider::setItemEnabled(QString s, bool en)
{
	int idx = items.indexOf(s);
	if (idx < 0) return;
	items[idx].enabled = en;
	if(!en) moveToEnabled();
	update();
}

void TextSlider::moveToEnabled()
{
	int idx = op.sliderPosition;
	if (!items.size()) return;   // list empty
	if (idx<0 || idx>=items.size()) { // index out of range
		idx = 0;
	} else {
		if (items[idx].enabled) return; // current item enabled
	}
	if (!prev()) next();
//	if (idx != op.sliderPosition)
//		emit valueChanged(op.sliderPosition);
}

void TextSlider::setTickPosition(QSlider::TickPosition position)
{
//	if (position == QSlider::TicksLeft || position == QSlider::TicksRight) op.tickPosition = position;
	if ( op.tickPosition == position ) return;
	op.tickPosition = position;
	update();
}

void TextSlider::setUpsideDown(bool v)
{
	op.upsideDown = v;
	update();
}

void TextSlider::setOrientation(Qt::Orientation orient)
{
	if (op.orientation == orient) return;
	op.orientation = orient;

	if (op.orientation == Qt::Vertical) {
		setMinimumHeight(items.size() * VITEMSIZE);
	} else {
		setMinimumWidth(items.size() * HITEMSIZE);
		setMinimumHeight(40);
	}
	update();
}

void TextSlider::first()
{
	int pos = op.sliderPosition;
	op.sliderPosition = 0;
	moveToEnabled();
	update();
	if (op.sliderPosition != pos) emit valueChanged(op.sliderPosition);
}

void TextSlider::last()
{
	int pos = op.sliderPosition;
	op.sliderPosition = items.size()-1;
	moveToEnabled();
	update();
	if (op.sliderPosition != pos) emit valueChanged(op.sliderPosition);
}

bool TextSlider::prev()
{
	int pos = op.sliderPosition;
	pos++;
	while (pos < op.maximum && !items[pos].enabled) pos++;
	if (pos > op.maximum || !items[pos].enabled) return 1;

	if (op.sliderPosition == pos) return 1;
	op.sliderPosition = pos;
	update();
	emit valueChanged(op.sliderPosition);
	return 0;
}

bool TextSlider::next()
{
	int pos = op.sliderPosition;
	pos--;
	while (pos > op.minimum && !items[pos].enabled) pos--;
	if (pos < op.minimum || !items[pos].enabled) return 1;

	if (op.sliderPosition == pos) return 1;
	op.sliderPosition = pos;
	update();
	emit valueChanged(op.sliderPosition);
	return 0;
}

int TextSlider::value()
{
	if (op.sliderPosition <0 || op.sliderPosition >= items.size()) return -1;
	return op.sliderPosition;
}

QString TextSlider::text()
{
	if (op.sliderPosition <0 || op.sliderPosition >= items.size()) return QString::null;
	return items[op.sliderPosition].text;
}

void TextSlider::setValue(int val)
{
	op.sliderPosition = val;
	moveToEnabled();
	update();
}

void TextSlider::setCurrentItem(const QString& text)
{
	int idx=-1;
	for (int i=0; i<items.size(); i++) {
		if(items[i].enabled && items[i].text == text) idx=i;
	}
	if (idx<0) return;
	setValue(idx);
}

void TextSlider::setEnabled(bool enable)
{
//	qDebug() << "TextSlider::setEnabled: " << enable;
	en = enable;
	if (en) {
		op.activeSubControls = op.subControls;
		op.state = QStyle::State_Active | QStyle::State_Enabled;
	} else {
		op.activeSubControls = 0;
		op.state = QStyle::State_None;
	}
	QAbstractSlider::setEnabled(en);
}
	
void TextSlider::setDisabled(bool disable)  { setEnabled(!disable); }

void TextSlider::mousePressEvent(QMouseEvent*) { oldpos = op.sliderPosition; }

void TextSlider::mouseReleaseEvent(QMouseEvent* e)
{
	int pos;
	if (!items.size()) return;

	if (op.orientation == Qt::Vertical) {
		pos = QStyle::sliderValueFromPosition(op.minimum, op.maximum, e->pos().y()-10, height()-21, op.upsideDown);
	} else {
		int itemw = width() / items.size();
		pos = QStyle::sliderValueFromPosition(op.minimum, op.maximum, e->pos().x()-itemw/2, width() - itemw , op.upsideDown);
	}

	if (items[pos].enabled) {
		op.sliderPosition = pos;
		update();
	}
	if (oldpos != op.sliderPosition) {
//		qDebug() << "mouseReleaseEvent()  oldpos: " << oldpos << "  pos: " << op.sliderPosition;
		emit valueChanged(op.sliderPosition);
	}
}

void TextSlider::mouseMoveEvent(QMouseEvent* e)
{
	int pos;
	if (!items.size()) return;

	if (op.orientation == Qt::Vertical) {
		pos = QStyle::sliderValueFromPosition(op.minimum, op.maximum, e->pos().y()-10, height()-21, op.upsideDown);
	} else {
		int itemw = width() / items.size();
		pos = QStyle::sliderValueFromPosition(op.minimum, op.maximum, e->pos().x()-itemw/2, width() - itemw , op.upsideDown);
	}

	if (items[pos].enabled) {
		op.sliderPosition = pos;
		update();
	}
}

void TextSlider::wheelEvent(QWheelEvent* e)
{
	int pos = op.sliderPosition;
	if (e->delta() * (op.upsideDown ? -1:1) * (op.orientation == Qt::Vertical ? 1:-1) < 0) {
		prev();
	} else if (e->delta() * (op.upsideDown ? -1:1) * (op.orientation == Qt::Vertical ? 1:-1) > 0) {
		next();
	} else {
		return;
	}
	if (op.sliderPosition != pos) emit valueChanged(op.sliderPosition);
//	qDebug() << "slider pos: " << op.sliderPosition;
}

void TextSlider::keyPressEvent(QKeyEvent* e)
{
	switch (e->key()) {
		case Qt::Key_Up:
		case Qt::Key_Right:
			if ((op.upsideDown ? -1:1) * (op.orientation == Qt::Vertical ? 1:-1) < 0) prev(); else next();
			break;
		case Qt::Key_Down:
		case Qt::Key_Left:
			if ((op.upsideDown ? -1:1) * (op.orientation == Qt::Vertical ? 1:-1) < 0) next(); else prev();
			break;
		case Qt::Key_Home:
			if ((op.upsideDown ? -1:1) * (op.orientation == Qt::Vertical ? 1:-1) < 0) first(); else last();
			break;
		case Qt::Key_End:
			if ((op.upsideDown ? -1:1) * (op.orientation == Qt::Vertical ? 1:-1) < 0) last(); else first();
			break;
		default:
			e->ignore();
			break;
	}
}

void TextSlider::focusInEvent(QFocusEvent*)
{
	op.state |= QStyle::State_HasFocus;
	update();
}

void TextSlider::focusOutEvent(QFocusEvent*)
{
	op.state &= ~QStyle::State_HasFocus;
	update();
}

void TextSlider::paintEvent(QPaintEvent*)
{
//	qWarning("TextSlider::paintEvent()");
	QPainter p(this);

//	p.fillRect(0,0,width(),height(), QBrush(Qt::red));

	int pos;
	int ctl = style()->pixelMetric( QStyle::PM_SliderControlThickness, &op, this);

	//qDebug() << "Qt::WA_ForceDisabled: " << testAttribute(Qt::WA_ForceDisabled);
	//qDebug() << "Qt::WA_Disabled: " << testAttribute(Qt::WA_Disabled);
	bool ena = en && !testAttribute(Qt::WA_Disabled);

	if (op.orientation == Qt::Vertical) {
		int itemh = height() - VITEMSIZE - ctl;
		if (items.size())
			itemh /= items.size();
		for (int i=0; i < items.size(); i++) {
			pos = QStyle::sliderPositionFromValue(op.minimum, op.maximum, i, height() - VITEMSIZE - ctl, op.upsideDown) + VITEMSIZE/2 + ctl/2;
			p.setPen( QPen( palette().color( (ena && items[i].enabled) ? QPalette::Active : QPalette::Disabled, QPalette::Text), 1) );

		//	p.drawLine(0, pos, width(), pos);
			if (op.tickPosition == QSlider::TicksLeft || op.tickPosition == QSlider::TicksBothSides )
				p.drawText(0,  pos-itemh/2, width()-30, itemh+2, Qt::AlignRight | Qt::AlignVCenter, items[i].text);
			else
				p.drawText(30, pos-itemh/2, width()-30, itemh+2, Qt::AlignLeft | Qt::AlignVCenter, items[i].text);
		}
		if (op.tickPosition == QSlider::TicksLeft || op.tickPosition == QSlider::TicksBothSides )
			p.translate(width()-26, VITEMSIZE/2);
		else
			p.translate(0, VITEMSIZE/2);
		op.rect.setWidth(26);
		op.rect.setHeight(height() - VITEMSIZE);
	} else {
		int itemw = width() - ctl;
		if (items.size())
			itemw /= items.size();
		for (int i=0; i < items.size(); i++) {
			pos = QStyle::sliderPositionFromValue(op.minimum, op.maximum, i, width()-itemw-ctl, op.upsideDown) + itemw/2 + ctl/2;
			p.setPen( QPen( palette().color( (ena && items[i].enabled) ? QPalette::Active : QPalette::Disabled, QPalette::Text), 1) );

		//	p.drawLine(pos, 0, pos, height());
			if (op.tickPosition == QSlider::TicksLeft || op.tickPosition == QSlider::TicksBothSides )
				p.drawText(pos-itemw/2, 0, itemw, height()-20, Qt::AlignHCenter | Qt::AlignBottom, items[i].text);
			else
				p.drawText(pos-itemw/2, 26, itemw, height()-20, Qt::AlignHCenter | Qt::AlignTop, items[i].text);
		}
		if (op.tickPosition == QSlider::TicksLeft || op.tickPosition == QSlider::TicksBothSides )
			p.translate(itemw/2, height()-26);
		else
			p.translate(itemw/2, 0);
		op.rect.setWidth(width() - itemw);
		op.rect.setHeight(26);
	}
	style()->drawComplexControl( QStyle::CC_Slider, &op, &p, this);
}

