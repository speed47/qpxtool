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

#ifndef PROGRESS_WIDGET_H
#define PROGRESS_WIDGET_H

#include <QWidget>
#include <QTimer>

class ProgressWidget: public QWidget
{
	Q_OBJECT

public:
	enum Direction {
		DirectionForward = 0,
		DirectionBackward = 1,
		DirectionBoth = 2
	};
	enum ProgressColor {
		BgColor    = 0,
		FrameColor = 1,
		TextColor  = 2,
		RectColor  = 3
	};

	ProgressWidget(int, int, QWidget *p);
	~ProgressWidget();
	void setDirection(ProgressWidget::Direction);
	void setText(QString);
	void setColor(ProgressColor, QColor);

public slots:
	virtual void setVisible(bool);

private slots:
	void step();

protected:
	void paintEvent(QPaintEvent*);

private:
	int blocks, shown;
	int idx;
	Direction dir;
	bool dir2;

	QString text;

	QTimer  timer;
	QColor  col_frame,
			col_bg,
			col_text,
			col_rect;
};

#endif
