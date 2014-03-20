/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <QPainter>
#include <QFontDatabase>

#include "progresswidget.h"

ProgressWidget::ProgressWidget(int iblocks, int ishown, QWidget *p)
	:QWidget(p, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
	QPoint offs = p->mapToGlobal( p->rect().topLeft() );
	blocks = iblocks;
	if (ishown>=blocks) shown = blocks-1;
	else                shown = ishown;

	idx = 0;
	dir = DirectionBoth;
	dir2 = 0;
	
	col_frame = palette().color(QPalette::Text);
	col_bg    = palette().color(QPalette::Window);
	col_text  = palette().color(QPalette::Text);
	col_rect  = palette().color(QPalette::Highlight);

	timer.setSingleShot(false);
	timer.setInterval(100);

	setGeometry(offs.x() + p->width()/2-150, offs.y() + p->height()/2-30, 300, 60);
	setMinimumSize(300,60);
	setMaximumSize(300,60);	
}

ProgressWidget::~ProgressWidget()
{
	timer.stop();
}

void ProgressWidget::setVisible(bool en)
{
	if (en) {
		timer.start();
		connect(&timer, SIGNAL(timeout()), this, SLOT(step()));
	} else {
		timer.stop();
		disconnect(&timer, SIGNAL(timeout()), this, SLOT(step()));
	}
	QWidget::setVisible(en);
}

void ProgressWidget::step()
{
	switch (dir) {
		case DirectionForward:
			idx++;
			if ((idx+shown) > blocks) idx=0;
			break;
		case DirectionBackward:
			idx--;
			if (idx < 0) idx=blocks-shown;
			break;
		case DirectionBoth:
			if (!dir2) {
				if (idx+shown >= blocks)
					dir2=1;
				else
					idx++;
			} else {
				if (idx<=0)
					dir2=0;
				else
					idx--;
			}
			break;
	}
	update();
}

void ProgressWidget::setDirection(ProgressWidget::Direction idir) { dir = idir; }
void ProgressWidget::setText(QString t) { text = t; setWindowTitle(text); }

void ProgressWidget::setColor(ProgressColor role, QColor color)
{
	switch (role) {
		case BgColor:
			col_bg = color; break;
		case FrameColor:
			col_frame = color; break;
		case TextColor:
			col_text = color; break;
		case RectColor:
			col_rect = color; break;
		default:
			return;
	}
}

void ProgressWidget::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	int blkw = (width() - 20) / blocks;
	int offs = 10 + (int)(blkw * ( idx + 0.1 ));

	p.setPen(QPen(col_frame,1));
	p.drawRect(0,0,width()-1,height()-1);

	if (text.isEmpty()) {
		for (int i=0; i<shown; i++)
			p.fillRect( offs+i*blkw, (height()-blkw) >> 1 ,
						(int)(blkw*0.8), (int)(blkw*0.8),
						QBrush(col_rect));
	} else {
		p.setPen(QPen(col_text,1));

		QFont f = p.font();
		p.setFont(f);

		p.drawText(0,0,
			width(), height()>>1,
			Qt::AlignHCenter | Qt::AlignVCenter, text);
		for (int i=0; i<shown; i++)
			p.fillRect( offs+i*blkw, height() >> 1,
						(int)(blkw*0.8), (int)(blkw*0.8),
						QBrush(col_rect));
	}
}

