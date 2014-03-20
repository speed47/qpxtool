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

#include "printpreview.h"

#include <QPrintDialog>
#include <QPrinter>
#include <QToolBar>
#include <QAction>
#include <QTextFormat>
#include <QMouseEvent>
#include <QTextFrame>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>
#include <QPageSetupDialog>
#include <QSplitter>

#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <sys/time.h>

#include <abstractpreview.h>

//#define DEFAULT_THUMBS_SCALE	0.125

#ifndef QT_NO_DEBUG
//#define PP_DEBUG2
//#define DEBUG_PAINT_TIME
#endif

class PreviewView : public AbstractPreview
{
public:
	PreviewView(QWidget *p, QPrinter *printer, QTextDocument *idoc = NULL,
			AbstractPreview::PreviewMode mode = AbstractPreview::Mode_Normal);
	virtual ~PreviewView();
	void setDocument(QTextDocument *idoc);
protected:
	virtual void paintPage(QPainter *p, int numberPage, const QRect&);
	virtual void updatePageFormat();
#ifdef PRINTER_CHANGE_DEVICE
	virtual void deviceChanged(QPaintDevice* device);
#endif
	virtual void clearEvent() {};

private:
	QTextDocument *doc;
};

PreviewView::PreviewView(QWidget *p, QPrinter *printer, QTextDocument *idoc, AbstractPreview::PreviewMode mode)
	: AbstractPreview(p, printer)
{
  	setViewMode(mode);
	setDocument(idoc);
}

PreviewView::~PreviewView() {}

void PreviewView::setDocument(QTextDocument *idoc)
{
//	flushCache();
	doc = idoc;
	updatePageFormat();
	updatePreview();
}

#ifdef PRINTER_CHANGE_DEVICE
void PreviewView::deviceChanged(QPaintDevice* device) {
	doc->documentLayout()->setPaintDevice(device);
	doc->setPageSize( getPageSize() );
};
#endif

void PreviewView::paintPage(QPainter *painter, int page, const QRect& rect)
{
#ifdef PP_DEBUG2
    qDebug() << "STA: PreviewView::paintPage" << page << " rect: " << rect;
#endif
#ifdef DEBUG_PAINT_TIME
    timeval tb,te;
    gettimeofday(&tb, NULL);
#endif
    const QSizeF pgSize = doc->pageSize();
    QColor col, bgc;

    bgc = palette().color((viewport()->backgroundRole()));

    painter->save();

    QRectF docRect(QPointF(0, page * pgSize.height()) + rect.topLeft(), rect.size());
    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.clip = docRect;

    ctx.palette.setColor(QPalette::Text, Qt::black);

    painter->translate( 0, -pgSize.height() * page);
    painter->translate( -rect.topLeft());
    painter->setClipRect(docRect);

    painter->setRenderHint(QPainter::TextAntialiasing, true);
    doc->documentLayout()->draw(painter, ctx);

    painter->restore();

#ifdef DEBUG_PAINT_TIME
    gettimeofday(&te, NULL);
    qDebug() << QString("Full page time: %1").arg(te.tv_sec - tb.tv_sec + (te.tv_usec - tb.tv_usec)/1000000.0,0,'f',4);
#endif
#ifdef PP_DEBUG2
    qDebug() << "END: PreviewView::paintPage()";
#endif
}

void PreviewView::updatePageFormat()
{
	clear();
	if (!doc) return;

	doc->setPageSize( getPageSize() );

	addPages(doc->pageCount());
#ifndef QT_NO_DEBUG
	qDebug() << "pageCount: " << doc->pageCount();
#endif
}

PrintPreview::PrintPreview(QWidget *parent, QTextDocument *document)
    : QDialog(parent), printer(QPrinter::HighResolution)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: PrintPreview()");
#endif
//    printer.setOutputFormat(QPrinter::PdfFormat);

	QList<int> ss;
	ss.append(100);
	ss.append(800);

	setWindowFlags(Qt::Window);
    	setWindowTitle(tr("Print Preview"));

	if (!document) {
		doc = NULL;
	} else {
		doc = document;
	}

	layout_main = new QVBoxLayout(this);
	layout_main->setMargin(3);
	layout_main->setSpacing(3);
	setLayout(layout_main);

	layout_butt = new QHBoxLayout;
	layout_butt->setMargin(0);
	layout_butt->setSpacing(3);
	layout_main->addLayout(layout_butt);

	split = new QSplitter(this);
	layout_main->addWidget(split);

	QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
	fmt.setLeftMargin(0);
	fmt.setRightMargin(0);
	fmt.setTopMargin(0);
	fmt.setBottomMargin(0);
	doc->rootFrame()->setFrameFormat(fmt);

	thumbs = new PreviewView(split, &printer, doc, AbstractPreview::Mode_Thumbs);

#ifdef THUMBS_MULTICOLUMN
	thumbs->setMinimumWidth( 100 );
	thumbs->setMaximumWidth( 300 );
#else
	thumbs->setMinimumWidth( 130 );
	thumbs->setMaximumWidth( 130 );
#endif

	view = new PreviewView(split, &printer, doc, AbstractPreview::Mode_Normal);
	view->setScaleRange(0.25,2.0);


	pb_print = new QPushButton(this);
	pb_print->setText(tr("Print"));
	pb_print->setIcon(QIcon(":images/printer.png"));
	layout_butt->addWidget(pb_print);

	pb_psetup = new QPushButton(this);
	pb_psetup->setText(tr("Page Setup"));
	pb_psetup->setIcon(QIcon(":images/page-setup.png"));
	layout_butt->addWidget(pb_psetup);

	layout_butt->addStretch(10);

	l_scale = new QLabel(tr("Scale"),this);
	layout_butt->addWidget(l_scale);

	box_scale = new QComboBox(this);
	box_scale->setMaximumHeight(24);
	box_scale->setMinimumHeight(24);
	box_scale->addItem(tr("25%"));
	box_scale->addItem(tr("50%"));
	box_scale->addItem(tr("100%"));
	box_scale->addItem(tr("200%"));
//	box_scale->addItem(tr("400%"));
	box_scale->setEditable(true);
	layout_butt->addWidget(box_scale);

	pb_zoomin = new QPushButton(this);
	pb_zoomin->setToolTip(tr("Zoom In"));
	pb_zoomin->setIcon(QIcon(":images/zoom-in.png"));
	layout_butt->addWidget(pb_zoomin);

	pb_zoomout = new QPushButton(this);
	pb_zoomout->setToolTip(tr("Zoom Out"));
	pb_zoomout->setIcon(QIcon(":images/zoom-out.png"));
	layout_butt->addWidget(pb_zoomout);

	pb_zoom1 = new QPushButton(this);
	pb_zoom1->setToolTip(tr("Original size"));
	pb_zoom1->setIcon(QIcon(":images/zoom-orig.png"));
	layout_butt->addWidget(pb_zoom1);

	resize(880, 650);
	split->setSizes(ss);

	scaleChanged(1.0);
	connect(thumbs, SIGNAL(pageSelected(int)), view, SLOT(gotoPage(int)));
	connect(view, SIGNAL(scaleChanged(double)), this, SLOT(scaleChanged(double)));
	connect(box_scale, SIGNAL(currentIndexChanged(QString)), this, SLOT(scaleChanged(QString)));
   	connect(pb_print, SIGNAL(clicked()), this, SLOT(print()));
	connect(pb_psetup, SIGNAL(clicked()), this, SLOT(pageSetup()));
	connect(pb_zoomin, SIGNAL(clicked()), view, SLOT(scaleIn()));
	connect(pb_zoomout, SIGNAL(clicked()), view, SLOT(scaleOut()));
   	connect(pb_zoom1, SIGNAL(clicked()), view, SLOT(scaleOrig()));

	connect(view, SIGNAL(pageFormatChanged()), thumbs, SLOT(setupPageFormat()));
	connect(view, SIGNAL(currentPage(int)), thumbs, SLOT(gotoPage(int)));

	view->scaleOrig();

#ifndef QT_NO_DEBUG
	qDebug("END: PrintPreview()");
#endif
}

PrintPreview::~PrintPreview()
{
#ifndef QT_NO_DEBUG
	qDebug("~PrintPreview()");
#endif
}

void PrintPreview::setDocument(QTextDocument *document)
{
#ifdef PP_DEBUG2
	qDebug() << "STA: PreviewView::setDocument(): document @" << document;
#endif
	view->setDocument(NULL);
	thumbs->setDocument(NULL);

	if (!document) return;
	doc = document;
	doc->documentLayout()->setPaintDevice(view->viewport());
	view->setDocument(doc);
	thumbs->setDocument(doc);
}

void PrintPreview::print()
{
	view->print();
}

void PrintPreview::printDoc(QWidget* parent, QPrinter* printer, QTextDocument* doc)
{
	PreviewView *view = new PreviewView(parent, printer, doc);
	view->print(printer);
	delete view;
}

void PrintPreview::pageSetup()
{
	view->pageSetup();
}

void PrintPreview::scaleChanged(double scale)
{
	int idx = box_scale->findText(QString("%1%").arg(qRound((scale*100))));
	if (idx<0) {
		box_scale->setEditText(QString("%1%").arg(qRound((scale*100))));
	} else {
		box_scale->setCurrentIndex(idx);
	}
}

void PrintPreview::scaleChanged(QString scale)
{
#ifndef QT_NO_DEBUG
	qDebug("scaleChanged()");
#endif
	scale.remove('%');
	view->setScale(scale.toInt()/100.0);
}

