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

#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QContextMenuEvent>

#include <math.h>
#include "qpxgraph.h"

#include <QDebug>

#include <sys/time.h>

#define SHOW_P95ERRC
//#define SHOW_P95JB

#define MARGIN_DEFL	40
#define MARGIN_DEFR	40
#define MARGIN_DEFB	18

#define GRID_STYLE Qt::DotLine
//#define GRID_STYLE Qt::DashLine

class IntList : public QList<int>
{
public:
	IntList() : QList<int>() {};

	float M() {
		float M = 0.0;
		int s = size();
		if (!s) return 0.0;
		for (int i=0; i<s; i++) M+=at(i);
		return (float)(M/s);
	};
	float dispers (float M) {
		double disp = 0.0;
		double q;
		int s = size();
		if (!s) return 0.0;
		for (int i=0; i<s; i++) {
			q = M - at(i);
			disp += (q*q);
		}
		return (disp/s);		
	};
	int dispers (int M) {
		int disp = 0;
		int q;
		int s = size();
		if (!s) return 0;
		for (int i=0; i<s; i++) {
			q = M - at(i);
			disp += (q*q);
		}
		return (disp/s);		
	};
};

int	errc_logh_lres[]={1,2,5,10,20,50,100,200,500,0};
int	errc_logh_hres[]={1,2,3,4,5,10,20,30,40,50,100,200,300,400,500,0};
//int	errc_loghl[]={1,10,100,1000,0};

#define GRAPH_DFL_CD   (GRAPH_BLER | GRAPH_E22 | GRAPH_E32 | GRAPH_UNCR)
#define GRAPH_DFL_DVD  (GRAPH_PI8  | GRAPH_PIF | GRAPH_POF | GRAPH_UNCR)
#define GRAPH_DFL_BD   (GRAPH_LDC  | GRAPH_BIS | GRAPH_UNCR)

QPxGraph::QPxGraph(QPxSettings *iset, devlist *idev, QString iname, int ttype, QWidget *p, Qt::WindowFlags fl)
	: QWidget(p,fl)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: QPxGraph()");
#endif
	setMinimumSize(250, 150);
	settings = iset;
	devices  = idev;
	name[0]	 = iname;
	test	 = ttype;
	showspeed= 1;
	forceAll = 1;
	lastX    = 0;
	errcList = 0;
	scale[0] = NULL;
	scale[1] = NULL;

	margin_left   = MARGIN_DEFL;
	margin_right  = MARGIN_DEFR;
	margin_bottom = MARGIN_DEFB;

	taZone   = 0;
	taLayer  = 0;
#ifdef CACHE_GRAPH
	img = NULL;
#endif
	switch (test) {
/*
		case TEST_RT:
		case TEST_WT:
		case TEST_JB:
		case TEST_FT:
		case TEST_TA:
			setContextMenuPolicy(Qt::NoContextMenu);
			break;
*/
		case TEST_ERRC:
			scale[0] = new Scale(settings->scales.get(name[0]));
		//	settings->loadScale(name);
			setScaleValue(256);
			setContextMenuPolicy(Qt::DefaultContextMenu);
			break;
		case TEST_JB:
			name[0] = "Jitter";
			name[1] = "Asymm";

			scale[0] = new Scale(settings->scales.get( name[0] ));
			scale[0]->type = Scale::Linear;
			scale[1] = new Scale(settings->scales.get( name[1] ));
			scale[1]->type = Scale::Linear;
			setScaleValue(2,0);
			setScaleValue(16,1);
			setContextMenuPolicy(Qt::NoContextMenu);
			break;
		default:
			setContextMenuPolicy(Qt::NoContextMenu);
			break;
	}
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
#ifndef QT_NO_DEBUG
	qDebug("END: QPxGraph()");
#endif
}

QPxGraph::~QPxGraph()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~QPxGraph()");
#endif
	for (int i=0; i<2; i++) if (scale[i]) delete scale[i];
#ifdef CACHE_GRAPH
	if (img) delete img;
#endif
#ifndef QT_NO_DEBUG
	qDebug("END: ~QPxGraph()");
#endif
}

/*
void QPxGraph::setDataNames(QStringList dn) { dataNames = dn; update(); }
void QPxGraph::setZeroPos(float zp) { zeropos = zp; update(); }
float QPxGraph::zeroPos() { return zeropos; }
*/

void QPxGraph::setErrcList(int el, QString glabel) { errcList = el; label=glabel; }
void QPxGraph::setErrcShow(int err, bool disp) { if (disp) errcList |= err; else errcList &= ~err; }
void QPxGraph::setShowSpeed(bool s) { showspeed = s; }
void QPxGraph::setRightMarginHidden(bool hide) { margin_right = hide ? 0 : MARGIN_DEFR; }

//void QPxGraph::forceUpdate() { forceAll = 1; }
void QPxGraph::resizeEvent(QResizeEvent*)
{
#ifndef QT_NO_DEBUG
//	qDebug("QPxGraph::resizeEvent()");
#endif
	forceAll = 1;
#ifdef CACHE_GRAPH
	delete img;
#endif
}

int  QPxGraph::getLastX()
{
	return lastX + margin_left + 1;
}

void QPxGraph::setZoneTA(int zone)   {
#ifndef QT_NO_DEBUG
	qDebug() << "Zone TA: " << zone;
#endif
	taZone = zone; update();
};

void QPxGraph::setLayerTA(int layer) {
#ifndef QT_NO_DEBUG
	qDebug() << "LayerTA: " << layer ;
#endif
	taLayer = layer; update();
};

void QPxGraph::wheelEvent(QWheelEvent* e)
{
#ifndef QT_NO_DEBUG
//	qDebug(QString("QPxGraph::wheelEvent(): %1").arg(e->delta()));
#endif
	if (e->modifiers() == Qt::NoModifier) {
		if (!scale[0] || scale[0]->type != Scale::Linear) return;
		if (e->delta() > 0)		 scaleIn(0);
		else if (e->delta() < 0) scaleOut(0);
		update();
	} else if (e->modifiers() == Qt::ControlModifier) {
		if (test != TEST_JB || !scale[1] || scale[1]->type != Scale::Linear) return;
		if (e->delta() > 0)		 scaleIn(1);
		else if (e->delta() < 0) scaleOut(1);
		update();
	}
}

void QPxGraph::paintEvent(QPaintEvent* e)
{
#ifdef CACHE_GRAPH
	if (!e->rect().x() && !img) forceAll = 1; else forceAll = 0;
#else
	if (!e->rect().x()) forceAll = 1; else forceAll = 0;
#endif
#ifdef CACHE_GRAPH
	QPainter p(this);
	if (!img) img = new QImage(size(), QImage::Format_RGB32);
	img->fill(palette().color(QPalette::Window).rgb());
	QPainter ip(img);
	if (!img)
		drawGraph(&ip, size(),devices->current(),test, e->rect());
	else
		drawGraph(&ip, size(),devices->current(),test, e->rect().adjusted(lastX,0,0,0));
	p.drawImage(0,0,*img);
#else
	QPainter p(this);
	drawGraph(&p, size(),devices->current(),test, e->rect());
#endif
}

void QPxGraph::drawGraph(QPainter *p, QSize s, device *dev, int ttype, const QRect& rect, int eflags, bool forceRepaint)
{
	int   el = errcList;
	bool  ss = showspeed;
	int   taL=0, taZ=0;
	QSize sg;
	QRect grect = rect.intersected(geometry().adjusted(margin_left, 0, -margin_right, -margin_bottom)).translated(margin_left,0);

	forceAll |= forceRepaint;

	if (forceRepaint)
		p->fillRect(rect, settings->col_bg);


	if (eflags) {
		if (ttype == TEST_TA) {
			taL = taLayer;
			taZ = taZone;
			taLayer = (eflags & (0xF << 28)) >> 28;
			taZone  = (eflags & (0x0FFFFFF0)) >> 4;
		} else {
			errcList = eflags;
			showspeed = 0;
		}
	}
#if 0
#ifndef QT_NO_DEBUG
	qDebug(QString("drawGraph: rect: %1.%2 - %3.%4")
		.arg(rect.left())
		.arg(rect.top())
		.arg(rect.right())
		.arg(rect.bottom())
	);
	qDebug(QString("drawGraph: grect: %1.%2 - %3.%4")
		.arg(grect.left())
		.arg(grect.top())
		.arg(grect.right())
		.arg(grect.bottom())
	);
#endif
#endif

	p->save();
	if (ttype == TEST_TA) {
		sg = QSize(s.width()-2, s.height()-margin_bottom-1);
		if (forceAll)
			p->fillRect(1, 1,
						width()-2, height()-margin_bottom-2,
						QBrush(settings->col_bg));
	} else {
		sg = QSize(s.width() - (margin_left + margin_right +2), s.height()-margin_bottom-1);
		if (forceAll || margin_left > rect.x())
			p->fillRect(margin_left+1, 1,
						width() - (margin_left + margin_right +2), height()-margin_bottom-1,
						QBrush(settings->col_bg));
		else
			p->fillRect(rect.x(), 1,
						width() - rect.x() - margin_left, height()-margin_bottom-1,
						QBrush(settings->col_bg));

		Vscale = (s.height() - margin_bottom) / 60.0;
		if (dev->media.type.startsWith("CD")) {
			HscaleLBA = 450000/sg.width();
			Vscale1X = Vscale;
		} else if (dev->media.type.startsWith("DVD")) {
			HscaleLBA = (1<<19) * 5 * dev->media.ilayers/sg.width();
			Vscale1X = Vscale * 3;
		} else if (dev->media.type.startsWith("BD")) {
			HscaleLBA = (1<<19) * 25 * dev->media.ilayers/sg.width();
			Vscale1X = Vscale * 4;
		}

		p->translate(margin_left+1, 1);
	}

	switch (ttype) {
		case TEST_RT:
			drawTransfer(p, sg, dev, grect, 0);
			break;
		case TEST_WT:
			drawTransfer(p, sg, dev, grect, 1);
			break;
		case TEST_ERRC:
			drawErrc(p, sg, dev, grect);
			break;
		case TEST_JB:
			drawJB(p, sg, dev, grect);
			break;
		case TEST_FT:
			drawFT(p, sg, dev, grect);
			break;
		case TEST_TA:
			drawTA(p, sg, dev, grect);
			break;
		default:
			break;
	}
	p->restore();
	if (ttype == TEST_TA) {
		drawGridTA (p,s,dev, ttype);
	} else {
		drawGrid (p,s,dev,ttype);
	}
	if (eflags) {
		if (ttype == TEST_TA) {
			taLayer = taL;
			taZone  = taZ;
		} else {
			errcList = el;
			showspeed = ss;
		}
	}
}

void QPxGraph::drawTransfer(QPainter* p, const QSize& s, device *dev, const QRect& rect, bool rw)
{
	Q_UNUSED(rect);

	IntList xerr;
	QPainterPath pp;
	int x,xo=0;
	int startidx=0;
	int i;
	uint64_t lastLBA=0;
	bool first;
#ifndef QT_NO_DEBUG
	qDebug("QPxGraph::drawTransfer()");
#endif
	if (!dev->testData.rt.size() && !dev->testData.wt.size()) return;
//	qDebug(QString("hscale: %1, capX: %2").arg(hscale).arg(hscale * dev->media.cread));
//	qDebug(QString("hscale: %1, b: %2").arg(hscale).arg(b));
//	qDebug(QString("hscale: %1").arg(hscale));
//	p->scale(1, hscale);

	p->setRenderHint(QPainter::Antialiasing, true);

	if (!rw && dev->testData.rt.size()) {
		startidx=0;
		if (!forceAll) {
			lastLBA	= (uint64_t)(lastX * HscaleLBA);
			for (i=dev->testData.rt.size()-1; i>=0 && dev->testData.rt[i].lba > lastLBA ; ) i--;
			if (i>0) startidx = i;
		}
		xo=0;
		xerr.clear();
		first = 1;
		for (i=startidx; i<dev->testData.rt.size(); i++) {
			x = (int) (dev->testData.rt[i].lba/HscaleLBA);
			xerr.append((int)(dev->testData.rt[i].spdx * 100));
			if (x!=xo || i==(dev->testData.rt.size()-1)) {
				if (first) {
					pp.moveTo(xo, s.height() - xerr.M()*Vscale1X/100.0);
					first=0;
				} else {
					pp.lineTo(xo, s.height() - xerr.M()*Vscale1X/100.0);
				}
				xerr.clear();
				xo = x;
			}
		}
		p->setPen(QPen(settings->col_rspeed, 2));
		p->drawPath(pp);

		lastX = (uint64_t)(dev->testData.rt.last().lba / HscaleLBA);
		if (lastX) lastX--;
	}

	if (rw && dev->testData.wt.size()) {
		startidx=0;
		if (!forceAll) {
			lastLBA	= (uint64_t)(lastX * HscaleLBA);
			for (i=dev->testData.wt.size()-1; i>=0 && dev->testData.wt[i].lba > lastLBA ; ) i--;
			if (i>0) startidx = i;
		}

		xo=0;
		xerr.clear();
		first = 1;
		for (i=startidx; i<dev->testData.wt.size(); i++) {
			x = (int) (dev->testData.wt[i].lba/HscaleLBA);
			xerr.append((int)(dev->testData.wt[i].spdx * 100));
			if (x!=xo || i==(dev->testData.wt.size()-1)) {
				if (first) {
					pp.moveTo(xo, s.height() - xerr.M()*Vscale1X/100.0);
					first=0;
				} else {
					pp.lineTo(xo, s.height() - xerr.M()*Vscale1X/100.0);
				}
				xerr.clear();
				xo = x;
			}
		}
		p->setPen(QPen(settings->col_wspeed, 2));
		p->drawPath(pp);

		lastX = (uint64_t)(dev->testData.wt.last().lba / HscaleLBA);
		if (lastX) lastX--;
	}
}

void QPxGraph::drawErrc(QPainter* p, const QSize& s, device *dev, const QRect&)
{
	QPainterPath pps;
	IntList xerr;
//	bool isCD;
	int x, xo;
	int errc;
	int errc_dfl = 0;
#ifdef SHOW_P95ERRC
	int   M,D;
//	float M,D;
#endif
	int min=-1,max=-1;
	int i;
	int startidx=0;
	uint64_t lastLBA=0;
	bool first;

	if (dev->media.type.startsWith("CD")) {
		errc_dfl = GRAPH_DFL_CD;
	} else if (dev->media.type.startsWith("DVD")) {
		errc_dfl = GRAPH_DFL_DVD;
	} else if (dev->media.type.startsWith("BD")) {
		errc_dfl = GRAPH_DFL_BD;
	}
	errc = errcList ? errcList : errc_dfl;

#ifndef QT_NO_DEBUG
	timeval b,e;
	double t0,t1;
//	qDebug(QString("QPxGraph::drawErrc() : %1").arg(errc,8,2));
	gettimeofday(&b,NULL);
#endif
	if (!dev->testData.errc.size()) return;

#ifndef QT_NO_DEBUG
	qDebug() << QString("lastBLA: %1, lastX: %2").arg(lastLBA).arg(lastX);
#endif
	if (forceAll) {
		startidx=0;
	} else {
		lastLBA	= (int)(lastX * HscaleLBA);
		for (i=dev->testData.errc.size()-1; i>=0 && dev->testData.errc[i].raw.lba > lastLBA ; ) i--;
		if (i>0) startidx = i;
#ifndef QT_NO_DEBUG
		qDebug() << QString("lastBLA: %1, startidx: %2, errc.size: %3")
			.arg(lastLBA)
			.arg(startidx)
			.arg(dev->testData.errc.size());
#endif
	}

	p->setRenderHint(QPainter::Antialiasing, false);
	xo = 0;
	xerr.clear();
	for (int e = 0; e<8; e++) {
		if ( (1<<e) & errc ) {
			
		//	p->setPen(QPen(*settings->col_errc.raw[e], 1));
			for (i=startidx; i<dev->testData.errc.size(); i++) {
//				if (i) i--;
	//	if ( (1<<e) & errc ) for (i=0; i<dev->testData.errc.size(); i++) {
				x = (int) (dev->testData.errc[i].raw.lba/HscaleLBA);
				if (dev->testData.errc[i].raw.err[e] >=0 ) {
#ifdef SHOW_P95ERRC
					xerr.append(dev->testData.errc[i].raw.err[e]);
#endif
// update min/max
					if (min < 0 || min > dev->testData.errc[i].raw.err[e])
						min = dev->testData.errc[i].raw.err[e];
					if (max < dev->testData.errc[i].raw.err[e])
						max = dev->testData.errc[i].raw.err[e];
				}

				if (x!=xo || i==(dev->testData.errc.size()-1)) {
// min-max
					p->setPen(QPen(*settings->col_errc.raw[e], 1));
					p->drawLine(xo, errc2h(s.height(), min),
								xo, errc2h(s.height(), max));

// P=0.95
#ifdef SHOW_P95ERRC
					M = (int)xerr.M();
					D = (int)sqrt(xerr.dispers(M));
					p->setPen(QPen(settings->col_errc.raw[e]->darker(), 1));
					p->drawLine(xo, errc2h(s.height(), M-D),
								xo, errc2h(s.height(), M+D));
					xerr.clear();
#endif
					min = -1;
					max = -1;
					xo = x;
				}
			}
		}
	}

#ifndef QT_NO_DEBUG
	gettimeofday(&e,NULL);
	t0 = (e.tv_sec - b.tv_sec) + (e.tv_usec - b.tv_usec)/1000000.0;
#endif

	if (showspeed)
	{
// draw speed
#ifndef QT_NO_DEBUG
		gettimeofday(&b,NULL);
#endif
		p->setRenderHint(QPainter::Antialiasing, true);
		xo=0;
		xerr.clear();
		first=1;
		for (i=startidx; i<dev->testData.errc.size(); i++) {
			x = (int) (dev->testData.errc[i].raw.lba/HscaleLBA);

			xerr.append( (int) (dev->testData.errc[i].raw.spdx * 100) );
			if (x!=xo || i==(dev->testData.errc.size()-1)) {
				if (first) {
					pps.moveTo(xo, s.height() - xerr.M()*Vscale1X/100.0);
					first=0;
				} else {
					pps.lineTo(xo, s.height() - xerr.M()*Vscale1X/100.0);
				}
				xerr.clear();
				xo = x;
			}
		}
		p->setPen(QPen(settings->col_bginv, 2));
		p->drawPath(pps);

#ifndef QT_NO_DEBUG
		gettimeofday(&e,NULL);
		t1 = (e.tv_sec - b.tv_sec) + (e.tv_usec - b.tv_usec)/1000000.0;
		qDebug() << QString("draw time(sec): %1  ERRC,  %2  Speed")
			.arg(t0,0,'f',6)
			.arg(t1,0,'f',6);
#endif
	}

	lastX = (uint64_t)(dev->testData.errc.last().raw.lba / HscaleLBA);
	if (lastX) lastX--;
}

void QPxGraph::drawJB(QPainter* p, const QSize& s, device *dev, const QRect&)
{
#ifndef QT_NO_DEBUG
	qDebug("QPxGraph::drawJB()");
#endif
	QPainterPath pps;
	IntList xj, xb;
	int x, xo;
#ifdef SHOW_P95JB
	int   M,D;
//	float M,D;
#endif
	float jmin=0, jmax=0;
	float bmin=0, bmax=0;
	int i;
	int startidx=0;
	uint64_t lastLBA=0;
	bool first=1;

#ifndef QT_NO_DEBUG
	timeval b,e;
	double t0,t1;
//	qDebug(QString("QPxGraph::drawErrc() : %1").arg(errc,8,2));
	gettimeofday(&b,NULL);
#endif
	if (!dev->testData.jb.size()) return;

#ifndef QT_NO_DEBUG
	qDebug() << QString("lastBLA: %1, lastX: %2").arg(lastLBA).arg(lastX);
#endif
	if (forceAll) {
		startidx=0;
	} else {
		lastLBA	= (uint64_t)(lastX * HscaleLBA);
		for (i=dev->testData.jb.size()-1; i>=0 && dev->testData.jb[i].lba > lastLBA ; ) i--;
		if (i>0) startidx = i;
#ifndef QT_NO_DEBUG
		qDebug() << QString("lastBLA: %1, startidx: %2, jb.size: %3")
			.arg(lastLBA)
			.arg(startidx)
			.arg(dev->testData.jb.size());
#endif
	}

	p->setRenderHint(QPainter::Antialiasing, false);
	xo = 0;
	xj.clear();
	xb.clear();

//	p->setPen(QPen(*settings->col_errc.raw[e], 1));
	for (i=startidx; i<dev->testData.jb.size(); i++) {
	//	if ( (1<<e) & errc ) for (i=0; i<dev->testData.errc.size(); i++) {
		x = (int) (dev->testData.jb[i].lba/HscaleLBA);
		if (dev->testData.jb[i].jitter >=-100 && dev->testData.jb[i].asymm >=-100 ) {
#ifdef SHOW_P95JB
			xj.append(dev->testData.jb[i].jitter * 100);
			xj.append(dev->testData.jb[i].jitter * 100);
			xb.append(dev->testData.jb[i].asymm * 100);
			xb.append(dev->testData.jb[i].asymm * 100);
#endif
// update min/max
			if (first) {
				jmin = dev->testData.jb[i].jitter;
				jmax = dev->testData.jb[i].jitter;
				bmin = dev->testData.jb[i].asymm;
				bmax = dev->testData.jb[i].asymm;
				first=0;
			} else {
				if (jmin > dev->testData.jb[i].jitter)
					jmin = dev->testData.jb[i].jitter;
				if (jmax < dev->testData.jb[i].jitter)
					jmax = dev->testData.jb[i].jitter;
				if (bmin > dev->testData.jb[i].asymm)
					bmin = dev->testData.jb[i].asymm;
				if (bmax < dev->testData.jb[i].asymm)
					bmax = dev->testData.jb[i].asymm;
			}
		}


		if (x!=xo || i==(dev->testData.jb.size()-1)) {
#ifndef QT_NO_DEBUG
			qDebug() << QString("i=%1   J: %2..%3   A: %4..%5")
				.arg(i)
				.arg(jmin)
				.arg(jmax)
				.arg(bmin)
				.arg(bmin);
#endif
// ******* Jitter
// min-max
			p->setPen(QPen(settings->col_jitter, 1));
			p->drawLine(xo, jitter2h(s.height(), jmin),
						xo, jitter2h(s.height(), jmax));
// P=0.95
#ifdef SHOW_P95JB
			M = xj.M();
			D = sqrt(xj.dispers(M));
			p->setPen(QPen(settings->col_jitter.darker(), 1));
			p->drawLine(xo, jitter2h(s.height(), (M-D)/100.0),
						xo, jitter2h(s.height(), (M+D)/100.0));
			xj.clear();
#endif


// ******* Asymmmethry
// min-max
			p->setPen(QPen(settings->col_asymm, 1));
			p->drawLine(xo, asymm2h(s.height(), bmin),
						xo, asymm2h(s.height(), bmax));
// P=0.95
#ifdef SHOW_P95JB
			M = xb.M();
			D = sqrt(xb.dispers(M));
			p->setPen(QPen(settings->col_asymm.darker(), 1));
			p->drawLine(xo, asymm2h(s.height(), (M-D)/100.0),
						xo, asymm2h(s.height(), (M+D)/100.0));
			xb.clear();
#endif
			xo = x;

			first = 1;
		}
	}

#ifndef QT_NO_DEBUG
	gettimeofday(&e,NULL);
	t0 = (e.tv_sec - b.tv_sec) + (e.tv_usec - b.tv_usec)/1000000.0;
	gettimeofday(&b,NULL);
#endif

// draw speed
	p->setRenderHint(QPainter::Antialiasing, true);
	xo=0;
	xj.clear();
	first=1;
	for (i=startidx; i<dev->testData.jb.size(); i++) {
		x = (int) (dev->testData.jb[i].lba/HscaleLBA);

		xj.append((int)(dev->testData.jb[i].spdx * 100));
		if (x!=xo || i==(dev->testData.jb.size()-1)) {
			if (first) {
				pps.moveTo(xo, s.height() - xj.M()*Vscale1X/100.0);
				first=0;
			} else {
				pps.lineTo(xo, s.height() - xj.M()*Vscale1X/100.0);
			}
			xj.clear();
			xo = x;
		}
	}
	p->setPen(QPen(settings->col_bginv, 2));
	p->drawPath(pps);

#ifndef QT_NO_DEBUG
	gettimeofday(&e,NULL);
	t1 = (e.tv_sec - b.tv_sec) + (e.tv_usec - b.tv_usec)/1000000.0;
	qDebug() << QString("draw time(sec): %1  ERRC,  %2  Speed")
		.arg(t0,0,'f',6)
		.arg(t1,0,'f',6);
#endif
	lastX = (uint64_t)(dev->testData.jb.last().lba / HscaleLBA);
	if (lastX) lastX--;
}

void QPxGraph::drawFT(QPainter* p, const QSize& s, device *dev, const QRect&)
{
	QPainterPath ppf,ppt, pps;
	int x;
#ifndef QT_NO_DEBUG
	qDebug("QPxGraph::drawFT()");
#endif
/*
	p->setPen(QPen(settings->col_wspeed, 2));
	p->drawLine(360000/HscaleLBA, 1, 360000/HscaleLBA, s.height());
*/
	if (!dev->testData.ft.size()) return;

	pps.moveTo(0, s.height() - dev->testData.ft[0].spdx*Vscale1X);
	ppf.moveTo(0, s.height() - dev->testData.ft[0].fe*Vscale);
	ppt.moveTo(0, s.height() - dev->testData.ft[0].te*Vscale);

	for (int i=1; i<dev->testData.ft.size(); i++) {
		x = (int)(dev->testData.ft[i].lba/HscaleLBA);
		pps.lineTo( x, s.height() - dev->testData.ft[i].spdx*Vscale1X);
		ppf.lineTo( x, s.height() - dev->testData.ft[i].fe*Vscale);
		ppt.lineTo( x, s.height() - dev->testData.ft[i].te*Vscale);
//		qDebug(QString("p: %1:%2").arg(x).arg(y));
	}

	p->setRenderHint(QPainter::Antialiasing, true);

	p->setPen(QPen(settings->col_bginv, 2));
	p->drawPath(pps);

	p->setPen(QPen(settings->col_fe, 2));
	p->drawPath(ppf);

	p->setPen(QPen(settings->col_te, 2));
	p->drawPath(ppt);
}

void QPxGraph::drawTA(QPainter* p, const QSize& s, device *dev, const QRect&)
{
	int taIdx = taLayer*3 + taZone;
	int h;
	int x, prevh=0;
	QPainterPath pp;
#ifndef QT_NO_DEBUG
	qDebug("QPxGraph::drawTA()");
#endif
	if ( taIdx < 0 || taIdx > 5 ) return;

	Hscale = s.width() / 13.0;

	p->setPen(QPen( taMode ? settings->col_taland : settings->col_tapit, 1));
	for (int i=0; i<dev->testData.ta[taIdx].size(); i++) {
		h = ta2h(s.height(), taMode ? dev->testData.ta[taIdx][i].land : dev->testData.ta[taIdx][i].pit);
#if 0
		if (h>0) {
			x = (int)(((dev->testData.ta[taIdx][i].idx - 64) / 21.5454 + 1) * Hscale);
			p->drawLine( x, s.height(), x, h );
		}
#else
		x = (int)(((dev->testData.ta[taIdx][i].idx - 64) / 21.5454 + 1) * Hscale);
		if (h<s.height() || prevh<s.height()) {
			pp.lineTo(x,h);
		} else {
			pp.moveTo(x,h);	
		}
		prevh = h;
#endif
	}
	p->drawPath(pp);
}

#define VGRIDS  10.0

void QPxGraph::drawGrid(QPainter* p, const QSize& s, device *dev, int ttype)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: QPxGraph::drawGrid()");

	timeval b,e;
	gettimeofday(&b,NULL);
	double t;
#endif
//	int idxn, idxd;
//	int scaleType;
	float HscaleX;
	bool  isCD = 0;
	int	  spdMax;
	int   GBperLayer;
//	TestData* data;
	QFont dfont = p->font();
	QFont lfont = dfont;

	lfont.setPointSizeF(lfont.pointSizeF()*1.3);
//	lfont.setBold(true);
//	lfont.setItalic(true);

	if (ttype & TEST_TA) return;

	p->setRenderHint(QPainter::Antialiasing, false);
	p->setRenderHint(QPainter::TextAntialiasing, true);

// graph border
	p->setPen(QPen(palette().color(QPalette::Dark), 1));
	//p->setPen(QPen(palette().color(QPalette::Highlight), 1));
	p->drawRect(margin_left, 0, s.width()- (margin_left + margin_right) - 1, s.height()-margin_bottom);
	if (dev->media.type == "-") return;
/*
	p->drawLine(margin_L-1,s.height()-margin_bottom,
				s.width(),s.height()-margin_bottom);
	p->drawLine(margin_L-1,s.height()-margin_bottom,
				margin_L-1,0);
*/
	if (dev->media.type.startsWith("CD")) {
		isCD = 1;
	} else if (dev->media.type.startsWith("DVD")) {
		isCD = 0;
		GBperLayer = 5;
	} else if (dev->media.type.startsWith("BD")) {
		isCD = 0;
		GBperLayer = 25;
	}

	if (isCD) {
		Hscale = (s.width()-( margin_left + margin_right )-2) / 1.0 / 100/4500;
		spdMax = 60;
	} else {
		Hscale = (s.width()-( margin_left + margin_right )-2) / 1.0 / GBperLayer / (1<<19) /  dev->media.ilayers;
		spdMax = 20;
	}


	HscaleX = (s.width() - (margin_left + margin_right )) / VGRIDS;

// vertical grid lines
	p->setPen( QPen(settings->col_grid, 1, GRID_STYLE) );
	for (int i=1; i<VGRIDS; i++)
		p->drawLine(margin_left + (int)(i*HscaleX), s.height()-margin_bottom-1,
					margin_left + (int)(i*HscaleX), 1);

	p->setPen( QPen(settings->col_bginv, 1, GRID_STYLE) );
// capacity
	if (dev->media.creads) {
		p->drawLine(margin_left + (int)(Hscale*dev->media.creads) + 2, s.height()-margin_bottom-1,
					margin_left + (int)(Hscale*dev->media.creads) + 2, 1);
	}
	p->drawLine(margin_left + (int)(Hscale*dev->media.ctots) + 2,  s.height()-margin_bottom-1,
				margin_left + (int)(Hscale*dev->media.ctots) + 2,  1);

// bottom text labels
	p->setPen(QPen(palette().color(QPalette::Text), 1));
	if (isCD) {
		for (int i=1; i<VGRIDS; i++) {
			if (!(i&1))
				p->drawText( margin_left +  (int)(HscaleX*(i-1)),
						s.height()-margin_bottom+2,
						(int)(HscaleX*2),
						margin_bottom-2,
						Qt::AlignVCenter | Qt::AlignHCenter,
						QString::number(i*(100/VGRIDS))+" min");
		}
	} else {
		for (int i=1; i<VGRIDS; i++) {
			if (!(i&1))
				p->drawText( margin_left + (int)(HscaleX*(i-1)),
						s.height()-margin_bottom+2,
						(int)(HscaleX*2),
						margin_bottom-2,
						Qt::AlignVCenter | Qt::AlignHCenter,
						QString::number(i*( dev->media.ilayers*GBperLayer/VGRIDS))+" GB");
		}
	}

	p->setPen( QPen(settings->col_grid, 1, GRID_STYLE) );
// horizontal grid lines and left labels
	int h;
	switch (ttype) {
		case TEST_RT:
		case TEST_WT:
			for (int i=(isCD ? 4:2); i<spdMax; i+=(isCD ? 4 : 2)) {
				h = (int)(s.height() - margin_bottom - i*Vscale1X);
				p->drawLine(margin_left+1, h,
							s.width()-margin_right-2, h);
				if (!(i & (isCD ? 7 : 1))) {
					p->setPen(QPen(palette().color(QPalette::Text), 1));
					p->drawText( 2, h - 10, margin_left-4, 20,
							Qt::AlignVCenter | Qt::AlignRight,
							QString::number(i));
					p->setPen(QPen(settings->col_grid, 1, GRID_STYLE));
				}
			}
			break;
		case TEST_ERRC:
			if (scale[0]->type == Scale::Log) {
		// LOG10: hlines with text labels
				int *errc_logh = errc_logh_hres;
				for (int i=0; errc_logh[i]>0; i++) {
					h = errc2h(s.height()-margin_bottom,errc_logh[i]);
					p->drawLine(margin_left+1, h,
								s.width()-margin_right-2, h);
				}

				if (s.height() < 250) errc_logh = errc_logh_lres;

				p->setPen(QPen(palette().color(QPalette::Text), 1));
				for (int i=0; errc_logh[i]>0; i++) {
					h = errc2h(s.height()-margin_bottom,errc_logh[i]);
					p->drawText( 2, h - 10, margin_left-4, 20,
							Qt::AlignVCenter | Qt::AlignRight,
							QString::number(errc_logh[i]));
				}
			} else {
				int steps = (int)((float)(s.height() - margin_bottom) / 20);
				if (!steps) steps=1;
				int step =  (scale[0]->value / steps + 9) / 10 * 10;
#ifndef QT_NO_DEBUG
//				qDebug(QString("scale step: %1").arg(step));
#endif
				h = errc2h(s.height()-margin_bottom,step);
				for (int i=step; h>10; i+=step) {
		// LINEAR: hlines with text labels
					h = errc2h(s.height()-margin_bottom,i);
					p->drawLine(margin_left+1, h,
								s.width()-margin_right-2, h);
					p->setPen(QPen(palette().color(QPalette::Text), 1));
					if (h-10 >= 0) {
						p->drawText( 2, h - 10, margin_left-4, 20,
								Qt::AlignVCenter | Qt::AlignRight,
								QString::number(i));
						p->setPen(QPen(settings->col_grid, 1, GRID_STYLE));
					}
				}
			}
			if (!label.isEmpty()) {
				p->setFont(lfont);
				p->setPen(QPen( settings->col_bginv, 1));
				p->drawText(margin_left + 5,4,60,20,
					Qt::AlignTop | Qt::AlignLeft,
					label);
				p->setFont(dfont);
			}
		// error limit hlines
			p->setPen(QPen(Qt::red, 1, GRID_STYLE));
			if (isCD) {
				h = errc2h(s.height() - margin_bottom, 220);
				p->drawLine(margin_left+1, h,
							s.width()-margin_right-2, h);
			} else {
				h = errc2h(s.height() - margin_bottom, 4);
				p->drawLine(margin_left+1, h,
							s.width()-margin_right-2, h);
				h = errc2h(s.height() - margin_bottom, 280);
				p->drawLine(margin_left+1, h,
							s.width()-margin_right-2, h);
			}
			break;
		case TEST_JB:
			{
				float dj = scale[0]->value/8.0;
				float da = scale[1]->value/8.0;
				float vj=0, va=-da*4;
		// Jitter/asymm hlines & text labels
				for ( int i=0; i<8; i++) {
					h = jitter2h(s.height() - margin_bottom, vj);
					p->drawLine(margin_left+1, h,
								s.width()-margin_right-2, h);

					p->setPen(QPen(palette().color(QPalette::Text), 1));
					p->drawText( 2, h - 10, margin_left-4, 20,
								Qt::AlignVCenter | Qt::AlignRight,
								QString("%1/%2").arg(vj).arg(va));
					p->setPen(QPen(settings->col_grid, 1, GRID_STYLE));
					vj+=dj;
					va+=da;
				}
			}
			break;
		case TEST_FT:
			for (int i=4; i<60; i+=4) {
				h = (int) (s.height() - margin_bottom - i*Vscale);
				p->drawLine(margin_left+1, h,
							s.width()-margin_right-2, h);
				if (!(i & 7)) {
					p->setPen(QPen(palette().color(QPalette::Text), 1));
					p->drawText( 2, h - 10, margin_left-4, 20,
							Qt::AlignVCenter | Qt::AlignRight,
							QString::number(i));
					p->setPen(QPen(settings->col_grid, 1, GRID_STYLE));
				}
			}
			break;
	}

// right speed labels for all graphs, except TA
//	qDebug() << "Draw speed labels... showspeed:" << showspeed << " margin_right: " << margin_right;
	if (showspeed && margin_right) {

		for (int i=(isCD ? 4:2); i<spdMax; i+=(isCD ? 4 : 2)) {
			p->setPen(QPen(palette().color(QPalette::Text), 1));
			h = (int)(s.height() - margin_bottom - i*Vscale1X);
			if (!(i & (isCD ? 7 : 1))) {
				p->drawText( s.width() - margin_right + 2, h - 10, margin_right-4, 20,
						Qt::AlignVCenter | Qt::AlignLeft,
						QString::number(i));
			}
		}
	}

#ifndef QT_NO_DEBUG
	gettimeofday(&e,NULL);
	t = (e.tv_sec - b.tv_sec) + (e.tv_usec - b.tv_usec)/1000000.0;
	qDebug() << QString("END: QPxGraph::drawGrid(): %1 sec").arg(t,0,'f',6);
#endif
}

void QPxGraph::drawGridTA(QPainter* p, const QSize& s, device *dev, int ttype)
{
//	int idxn, idxd;
//	int scaleType;
	float Hscale;
	int   TAgrids;
//	TestData* data;

#ifndef QT_NO_DEBUG
	qDebug("STA: QPxGraph::drawGridTA()");
#endif
// draw background and border
	if (ttype != TEST_TA) return;

	p->setRenderHint(QPainter::Antialiasing, false);
	p->setRenderHint(QPainter::TextAntialiasing, true);

	p->setPen(QPen(palette().color(QPalette::Dark), 1));
//	p->setPen(QPen(settings->col_grid, 1));
	p->drawRect(0,0,s.width()-1, s.height()-margin_bottom);

	if (dev->media.type == "-") return;
//	p->drawLine(0,s.height()-margin_bottom,s.width(),s.height()-margin_bottom);


	TAgrids=14;
/*
	if (dev->media.type.startsWith("CD")) {
		TAgrids=11;
	} else {
		TAgrids=14;
	}
*/

//	Hscale = width() / ((float)TAgrids-1);
	Hscale = s.width() / 13.0;

	p->setPen(QPen(palette().color(QPalette::Dark), 1, GRID_STYLE));
// vertical grid lines
	for (int i=3; i<=11; i++)
		p->drawLine((int)((i-2)*Hscale),s.height()-margin_bottom,
					(int)((i-2)*Hscale),0);
	
	if (dev->media.type.startsWith("DVD")) {
		p->drawLine((int)(12*Hscale),s.height()-margin_bottom,
					(int)(12*Hscale),0);
	}
// bottom text labels
	p->setPen(QPen(palette().color(QPalette::Text), 1));
	for (int i=3; i<=11; i++) {
		p->drawText((int)(Hscale*(i-2.5)),
					s.height()-margin_bottom+2,
					(int)(Hscale),
					margin_bottom-2,
					Qt::AlignVCenter | Qt::AlignHCenter,
					QString("T%1").arg(i));
	}
	if (dev->media.type.startsWith("DVD")) {
		p->drawText((int)(Hscale*(11.5)),
					s.height()-margin_bottom+2,
					(int)(Hscale),
					margin_bottom-2,
					Qt::AlignVCenter | Qt::AlignHCenter,
					"T14");
	}

//	p->setPen(QPen( settings->col_bginv, 1));
	if (!taMode) {
		p->setPen(QPen( settings->col_tapit, 1));
		p->drawText(10,10,200,30,
			Qt::AlignTop | Qt::AlignLeft,
			QString("Pit: Layer%1, Zone %2").arg(taLayer).arg(taZone));
	} else {
		p->setPen(QPen( settings->col_taland, 1));
		p->drawText(10,10,200,30,
			Qt::AlignTop | Qt::AlignLeft,
			QString("Land: Layer%1, Zone %2").arg(taLayer).arg(taZone));
	}
#ifndef QT_NO_DEBUG
	qDebug("END: QPxGraph::drawGridTA()");
#endif
}

void QPxGraph::contextMenuEvent(QContextMenuEvent *e)
{
	QMenu *cmenu;
	QAction *act;
	QIcon   icon_ok(":images/ok.png");
	if (!scale) return;

	cmenu = new QMenu(this);
	cmenu->addAction(
		(scale[0]->type == Scale::Log) ? icon_ok : QIcon(),
		tr("Logarithmic scale"),
		this, SLOT(setScaleTypeLog()));
	cmenu->addAction(
		(scale[0]->type == Scale::Linear) ? icon_ok : QIcon(),
		tr("Linear Scale"),
		this, SLOT(setScaleTypeLin()));
	cmenu->addSeparator();

/*
	cmenu->addAction(
		(settings->scales.get(name).policy == Scale::Auto) ? QIcon(":images/ok.png") : QIcon(),
		tr("Auto scale"),
		this, SLOT(setScalePolicyAuto()));
	cmenu->addAction(
		(settings->scales.get(name).policy == Scale::Fixed) ? QIcon(":images/ok.png") : QIcon(),
		tr("Fixed scale"),
		this, SLOT(setScalePolicyFixed()));
	cmenu->addSeparator();
*/
	act = cmenu->addAction( QIcon(":images/plus.png"),  tr("Scale in"), this, SLOT(scaleIn()));
	act->setEnabled(scale[0]->type == Scale::Linear);
	act = cmenu->addAction( QIcon(":images/minus.png"), tr("Scale out"), this, SLOT(scaleOut()));
	act->setEnabled(scale[0]->type == Scale::Linear);

#if 0
	if (test == TEST_ERRC) {
		qDebug() << "Errc List: " << errcList;
		if (devices->current()->media.type.startsWith("CD-")) {
			cmenu->addSeparator();
			cmenu->addAction( (errcList & GRAPH_BLER) ? icon_ok : QIcon(), "BLER");
			cmenu->addAction( (errcList & GRAPH_E11)  ? icon_ok : QIcon(), "E11");
			cmenu->addAction( (errcList & GRAPH_E21)  ? icon_ok : QIcon(), "E21");
			cmenu->addAction( (errcList & GRAPH_E31)  ? icon_ok : QIcon(), "E31");
			cmenu->addAction( (errcList & GRAPH_E12)  ? icon_ok : QIcon(), "E12");
			cmenu->addAction( (errcList & GRAPH_E22)  ? icon_ok : QIcon(), "E22");
			cmenu->addAction( (errcList & GRAPH_E32)  ? icon_ok : QIcon(), "E32");
			cmenu->addAction( (errcList & GRAPH_UNCR) ? icon_ok : QIcon(), "UNCR");
		} else if (devices->current()->media.type.startsWith("DVD-")) {
			cmenu->addSeparator();
			cmenu->addAction( (errcList & GRAPH_PIE)  ? icon_ok : QIcon(), "PIE");
			cmenu->addAction( (errcList & GRAPH_PI8)  ? icon_ok : QIcon(), "PI8");
			cmenu->addAction( (errcList & GRAPH_PIF)  ? icon_ok : QIcon(), "PIF");
			cmenu->addAction( (errcList & GRAPH_POE)  ? icon_ok : QIcon(), "POE");
			cmenu->addAction( (errcList & GRAPH_PO8)  ? icon_ok : QIcon(), "PO8");
			cmenu->addAction( (errcList & GRAPH_POF)  ? icon_ok : QIcon(), "POF");
			cmenu->addAction( (errcList & GRAPH_UNCR) ? icon_ok : QIcon(), "UNCR");
		}
	}
#endif
	cmenu->exec( e->globalPos());
	delete cmenu;
	update();
}

void QPxGraph::changeScale(int idx) {
	if (idx<0 || idx>1 || !scale[idx]) return;
	scale[idx]->type   = settings->scales.get(name[idx]).type;
	scale[idx]->policy = settings->scales.get(name[idx]).policy;
	scale[idx]->value  = settings->scales.get(name[idx]).value;
	update();
}

void QPxGraph::setScaleTypeLog() {
	if (!scale[0] || (scale[0]->type == Scale::Log)) return;
	scale[0]->type = Scale::Log;
	settings->scales.get(name[0]).type = scale[0]->type;
#ifdef CACHE_GRAPH
	if (img) delete img;
#endif
	emit scaleChanged();
}

void QPxGraph::setScaleTypeLin() {
	if (!scale[0] || (scale[0]->type == Scale::Linear)) return;
	scale[0]->type = Scale::Linear;
	settings->scales.get(name[0]).type = scale[0]->type;
#ifdef CACHE_GRAPH
	if (img) delete img;
#endif
	emit scaleChanged();
}

void QPxGraph::setScalePolicyAuto()	{
	for (int i=0; i<2; i++) 
		if (scale[i]) {
			scale[i]->policy = Scale::Auto;
			settings->scales.get(name[i]).policy = scale[i]->policy;
		}
	emit scaleChanged();
}

void QPxGraph::setScalePolicyFixed() {
	for (int i=0; i<2; i++) 
		if (scale[i]) {
			scale[i]->policy = Scale::Fixed;
			settings->scales.get(name[i]).policy = scale[i]->policy;
		}
	emit scaleChanged();
}

void QPxGraph::setScaleValue( int val, int idx)	{
	if (idx<0 || idx>1) {
		for (int i=0; i<2; i++) 
			if (scale[i]) {
				scale[i]->value = val;
				settings->scales.get(name[i]).value = val;
			}
	} else {
		if (scale[idx]) {
			scale[idx]->value = val;
			settings->scales.get(name[idx]).value = val;
		}
	}

#ifdef CACHE_GRAPH
	if (img) delete img;
#endif
	emit scaleChanged();
}

void QPxGraph::scaleIn(int idx)
{
#ifndef QT_NO_DEBUG
	qDebug() << "scaleIn (" << idx << ")";
#endif
	if (idx<0 || idx>1 || !scale[idx]) return;
	switch (test) {
		case TEST_ERRC:
			if (!scale[idx] || scale[idx]->value <= 64) return; 
			scale[idx]->value  /= 2;
			settings->scales.get(name[idx]).value = scale[idx]->value; 
			break;
		case TEST_JB:
			if (!idx) {
				if (!scale[idx] || scale[idx]->value <= 2) return; // jitter
			} else {
				if (!scale[idx] || scale[idx]->value <= 8) return; // asymmetry
			}
			scale[idx]->value  /= 2;
			settings->scales.get(name[idx]).value = scale[idx]->value; 
			break;
		default:
			break;
	}
#ifdef CACHE_GRAPH
	if (img) delete img;
#endif
	emit scaleChanged();
}

void QPxGraph::scaleOut(int idx)
{
#ifndef QT_NO_DEBUG
	qDebug() << "scaleOut (" << idx << ")";
#endif
	if (idx<0 || idx>1 || !scale[idx]) return;
	switch (test) {
		case TEST_ERRC:
			if (!scale[idx] || scale[idx]->value >= 2048) return; 
			scale[idx]->value  *= 2;
			settings->scales.get(name[idx]).value = scale[idx]->value; 
			break;
		case TEST_JB:
			if (!idx) {
				if (!scale[idx] || scale[idx]->value >= 8) return;  // jitter
			} else {
				if (!scale[idx] || scale[idx]->value >= 32) return;  // asymmetry
			}
			scale[idx]->value  *= 2;
			settings->scales.get(name[idx]).value = scale[idx]->value; 
			break;
		default:
			break;
	}
#ifdef CACHE_GRAPH
	if (img) delete img;
#endif
	emit scaleChanged();
}

int QPxGraph::errc2h(int h, int val)
{
	if (val<=0) return h;
	if (scale[0]->type == Scale::Log) {
		return (int) (h - 10 - h*((float)log10(val)*0.31));
	} else {
		return (int) (h - 2 - h*(float)val / scale[0]->value);
	}
}

int QPxGraph::jitter2h(int h, float val) { return (int) (h - h * val / scale[0]->value); }
int QPxGraph::asymm2h(int h, float val)  { return (int) (h/2 - h * val / scale[1]->value); }

int QPxGraph::ta2h(int h, int val)
{
	if (val<=0) return h;
	return (int) (h - h*(float)log10(val)*0.2);
}

