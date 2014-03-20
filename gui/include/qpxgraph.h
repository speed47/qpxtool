/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef QPX_GRAPH_H
#define QPX_GRAPH_H

#include <QWidget>
#include <qpxsettings.h>
#include <device.h>

#include <inttypes.h>

//#define CACHE_GRAPH
//#define ENABLE_UNCR

#define NOFORCE_REPAINT 0
#define FORCE_REPAINT   1

class QPxGraph : public QWidget
{
	Q_OBJECT
public:
	QPxGraph(QPxSettings *iset, devlist *idev, QString iname, int ttype, QWidget *p=0, Qt::WindowFlags fl = 0);
	~QPxGraph();
//	void	setDataNames(QStringList dn);
//	void	setZeroPos(float);
//	float	zeroPos();
	void drawGraph(QPainter*, QSize, device*, int ttype, const QRect& rect, int eflags=0, bool forceRepaint=0);
	void setErrcList(int, QString glabel);
	void setErrcShow(int, bool);
	void setShowSpeed(bool);
	void setRightMarginHidden(bool);
	int  getLastX();

public slots:
	void setZoneTA(int zone);
	void setLayerTA(int layer);
	void setModeTA(bool mode) { taMode = mode; };

	void changeScale(int idx=0);

protected:
	void contextMenuEvent(QContextMenuEvent*);
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent*);
	void wheelEvent(QWheelEvent*);

	void drawTransfer(QPainter*, const QSize&, device*, const QRect& rect, bool rw);
	void drawErrc(QPainter*, const QSize&, device*, const QRect& rect);
	void drawJB(QPainter*, const QSize&, device*, const QRect& rect);
	void drawFT(QPainter*, const QSize&, device*, const QRect& rect);
	void drawTA(QPainter*, const QSize&, device*, const QRect& rect);
	void drawGrid(QPainter*, const QSize&, device*, int ttype);
	void drawGridTA(QPainter*, const QSize&, device*, int ttype);

private slots:
	void setScalePolicyAuto();
	void setScalePolicyFixed();
	void setScaleTypeLog();
	void setScaleTypeLin();
	void setScaleValue( int val, int idx=-1);
	void scaleIn(int idx=0);
	void scaleOut(int idx=0);

signals:
	void scaleChanged();

private:
	bool		taMode; // 0 - Pit, 1 - Land
	int			taZone;
	int			taLayer;
	int			errc2h(int h, int val);
	int			jitter2h(int h, float val);
	int			asymm2h(int h, float val);
	int			ta2h(int h, int val);
	double		Hscale;
	double		HscaleLBA;
	float		Vscale;
	float		Vscale1X;
	int			test;
	bool		showspeed;
	devlist		*devices;
	QPxSettings	*settings;
	QString		label;
	QString		name[2];
	QStringList dataNames;
	Scale		*scale[2];
//	float		zeropos;
	uint8_t		errcList;
	bool		forceAll;
	uint64_t	lastX;
#ifdef CACHE_GRAPH
	QImage		*img;
#endif
	int			margin_left,
				margin_right,
				margin_bottom;
};

#endif

