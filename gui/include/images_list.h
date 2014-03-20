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

#ifndef __images_list_h
#define __images_list_h

#include <QScrollArea>
#include <QList>

#ifndef QT_NO_DEBUG
//#define __images_list_debug
#endif

class QBoxLayout;
class QWidget;
class ImageLabel;
class ImageDialog;


class ImagesList : public QScrollArea
{
	Q_OBJECT

public:
	enum ListDir {
	    Vertical   = 1,
	    Horizontal = 2
	};

	ImagesList(int minW, QWidget *parent, ListDir	orient = Vertical);
	ImagesList(int minW, QSize isize, QWidget *parent, ListDir orient = Vertical);
	ImagesList(int minW, int iw, int ih, QWidget *parent, ListDir orient = Vertical);
	~ImagesList();

	int addLabel(QString label, QImage image, int imgid=-1);

public slots:
	void clear();

private slots:
	void clicked(int);

signals:
	void selected(int);

protected:
	void keyPressEvent(QKeyEvent*);

private:
	void winit(ListDir orient);

	int	current;
//	QScrollArea *w;
	void clear_img();
	QWidget		*cwidget;
	QBoxLayout	*clayout;

	QList<ImageLabel*>	images_l;
	int iconw, iconh;
	int minW;
};

#endif

