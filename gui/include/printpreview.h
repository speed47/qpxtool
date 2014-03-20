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

#ifndef PRINTPREVIEW_H
#define PRINTPREVIEW_H

#include <QDialog>
#include <QAbstractScrollArea>
#include <QPrinter>
#include <QPointF>
#include <QSizeF>

#include <math.h>

#include "dpi_metrics.h"

class PreviewView;
class QTextDocument;
class QBoxLayout;
class QPushButton;
class QLabel;
class QComboBox;
class QSplitter;

class PrintPreview : public QDialog
{
    Q_OBJECT
	friend class PreviewView;

public:
    PrintPreview(QWidget *parent, QTextDocument *document = 0);
    virtual ~PrintPreview();
	void setDocument(QTextDocument *document);

	static void printDoc(QWidget* parent, QPrinter* printer, QTextDocument* doc);

private slots:
	void print();
	void pageSetup();
	void scaleChanged(double);
	void scaleChanged(QString);

private:
	QLabel		*l_scale;
	QComboBox	*box_scale;
	QPushButton	*pb_print,
				*pb_psetup,
				*pb_zoomin,
				*pb_zoom1,
				*pb_zoomout;

	QTextDocument *doc;
	QBoxLayout	*layout_main,
				*layout_butt;
	QSplitter	*split;
	PreviewView *view,
				*thumbs;
	QPrinter    printer;
};

#endif // PRINTPREVIEW_H

