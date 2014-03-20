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
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QImage>
#include <QPixmap>

#include <QKeyEvent>

#include <image_label.h>
#include "images_list.h"
#include <QDebug>

ImagesList::ImagesList(int iminW, QWidget *parent, ListDir orient)
	:QScrollArea(parent)
{
#ifdef __images_list_debug
	qDebug("ImagesList()");
#endif
	iconw = 64;
	iconh = 64;
	minW = qMax(iconw+4, iminW);
	winit(orient);

}

ImagesList::ImagesList(int iminW, QSize isize, QWidget *parent, ListDir orient)
	:QScrollArea(parent)
{
#ifdef __images_list_debug
	qDebug("ImagesList()");
#endif
	iconw = isize.width();
	iconh = isize.height();
	minW = qMax(iconw+4, iminW);
	winit(orient);

}

ImagesList::ImagesList(int iminW, int iw, int ih, QWidget *parent, ListDir orient)
	:QScrollArea(parent)
{
#ifdef __images_list_debug
	qDebug("ImagesList()");
#endif
	iconw = iw;
	iconh = ih;
	minW = qMax(iconw+4, iminW);
	winit(orient);

}

void ImagesList::winit(ListDir	orient)
{
	if (iconw<0) iconw=0;
	if (iconh<0) iconh=0;

	current = -1;
	cwidget = new QWidget(this);
//	cwidget->resize(80,700);
	setWidget(cwidget);
	if (orient == Vertical) {
		setMaximumWidth(minW + 30);
		setMinimumWidth(minW + 30);
		setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
		clayout = new QVBoxLayout(cwidget);
	} else {
		setMaximumHeight(iconh + 26);
		setMinimumHeight(iconh + 26);
		setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
		clayout = new QHBoxLayout(cwidget);
	}
	clayout->setMargin(1);
	clayout->setSpacing(4);
	cwidget->setLayout(clayout);
};

ImagesList::~ImagesList()
{
#ifdef __images_list_debug
	qDebug("~ImagesList()");
#endif
	clear_img();
};

int ImagesList::addLabel(QString label, QImage image, int imgid)
{
#ifndef QT_NO_DEBUG
	qDebug("Adding ImageLabel...");
#endif
	ImageLabel* l;
	int idx = images_l.size(); 
	//cwidget->resize(70, images_l.size() * 94 - 4);
	cwidget->resize(minW+8, (idx+1) * (iconh+30) - 4);
	l = new ImageLabel(
			minW,
			iconw,iconh,
			label,
			image.scaled(iconw, iconh, Qt::KeepAspectRatio, Qt::SmoothTransformation),
			(imgid < 0) ? idx : imgid,
			cwidget
	);
	images_l.append(l);
	clayout->addWidget(l);
	if (current<0) current=0;
	QObject::connect(l, SIGNAL(selected(int)), this, SLOT(clicked(int)) );
	QObject::connect(this, SIGNAL(selected(int)), l, SLOT(select(int)) );
	return idx;
};

void ImagesList::clicked(int idx) {
	current = idx;
//	for (int i=0; i<images_l.size(); i++) images_l[i]->select(idx);
	emit selected(idx);
};

void ImagesList::clear() {
	clear_img();
//	hide();
};

void ImagesList::clear_img() {
//	return;
	ImageLabel *l;
	int s = images_l.size();
#ifdef __images_list_debug
	qDebug("ImagesView::clear_img()");
#endif
	for (int i=0 ; i < s ; i++) {
#ifdef __images_list_debug
		qDebug(QString("Removing img #%1").arg(i));
#endif
		l =  images_l.takeFirst();
		l->disconnect();
		clayout->removeWidget(l);
		delete(l);
	}
	images_l.clear();
//	cwidget->adjustSize();
}

void ImagesList::keyPressEvent(QKeyEvent* e)
{
#ifndef QT_NO_DEBUG
//	qDebug("ImagesList::keyPressEvent()");
#endif

	switch (e->key()) {
		case Qt::Key_Up:
			if (current>0) {
				current--;
				ensureWidgetVisible(images_l[current],0,0);
				emit selected(current);
			}
			e->accept();
			break;
		case Qt::Key_Down:
			if (current<(images_l.size()-1)) {
				current++;
				ensureWidgetVisible(images_l[current],0,0);
				emit selected(current);
			}
			e->accept();
			break;

/*
		case Qt::Key_Home:
			e->accept();
			current= (images_l.size()) ? 0 : -1;
			emit selected(current);
			break;
		case Qt::Key_End:
			e->accept();
			current=images_l.size()-1;
			emit selected(current);
			break;
*/

		case Qt::Key_Escape:
			QScrollArea::keyPressEvent(e);
			break;
		default:
			e->ignore();
	}
}

