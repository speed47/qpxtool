/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009 Maxim Aldanov <aldmax@mail.ru>, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 * Copyright (C) 2008 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 * */

#include <QScrollBar>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QPrinter>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QDebug>

#include <sys/time.h>

#include "abstractpreview.h"
//--------------------------------------------------------------
#define DEFAULT_MARGIN_LEFT         15
#define DEFAULT_MARGIN_RIGHT        10
#define DEFAULT_MARGIN_TOP          10
#define DEFAULT_MARGIN_BOTTOM       10
//--------------------------------------------------------------

#define DEFAULT_SCALE_STEP	1.414213562

#define TILE_MAX_WIDTH      512
#define TILE_MAX_HEIGHT     512
#define CACHE_SIZE_MAX	    (16 << 20)

#define PRINTER_USE_FULLPAGE 1

#ifndef QT_NO_DEBUG
//#define DEBUG_PAINT_TIME
#define DEBUG_PRINT_TIME
#endif
//--------------------------------------------------------------
class AbstractPreview;
class AbstractPreviewPrivate;
//--------------------------------------------------------------
int QPageInfo::page() const { return fpage; }
QRect QPageInfo::rect() const { return frect; }
//--------------------------------------------------------------
class QMousePressData
{
public:
    QMousePressData() {press = false;}
    bool press;
    QPoint pos;
};
//--------------------------------------------------------------
class QPageData
{
public:
    QPageData() {
        reDraw = true;
        index = -1;
    }
    QPageData(const QPageData &data) {
        index = data.index;
        reDraw = data.reDraw;
        this->data = data.data;
    }
    ~QPageData() {}
    QPageData &operator=(const QPageData &data) {
        if(this == &data)return *this;
        index = data.index;
        reDraw = data.reDraw;
        this->data = data.data;
        return *this;
    }

    bool isValid() {
        return (bool)(index == -1);
    }

    int index;
    QVariant data;
    bool reDraw;
protected:
private:
};
//--------------------------------------------------------------
//--------------------------------------------------------------
class TilesCache
{
private:
	class Tile
	{
	public:
	    Tile(QPixmap *pix, int p, int x, int y) : data(pix), page(p), xidx(x), yidx(y) {};
	    ~Tile() { if (data) delete data; };

	    QPixmap *data;
	    int	page,
		xidx,
		yidx;
	};

public:
	TilesCache();
	~TilesCache();
	void clear();
	void setTileSize(int maxW, int maxH);
	void insertPages(int start, int count);
	void deletePages(int start, int count);
	void addTile(QPixmap*, int page, int x, int y);
	QPixmap* getTile(int page, int x, int y);
private:
	int	maxWidth,
		maxHeight;
	int	maxTiles;
	QList<Tile*> tiles;
};
//--------------------------------------------------------------
class AbstractPreviewPrivate
{
friend class AbstractPreview;
public:
    AbstractPreviewPrivate(AbstractPreview *preview, QPrinter *printer);
    ~AbstractPreviewPrivate();

    void changeScale();
    void setPageDefaults();
    void setup();

    void calcParamDraw();
    inline int paperWidthToScale();
    inline int paperHeightToScale();
    inline int paperWidth();
    inline int paperHeight();

    int addPages(int count = 1);
    int insertPages(int &index, int count = 1);
    int deletePages(int index, int count = 1);
    QPageData *pageData(int index);
    void clear();
    void flushCache();
    void flushPage(int page);

protected:
    void inits();
    void recalcMarginsFromMM();
private:
    int tileWidth,
	tileHeight;
    int tileCols,
	tileRows;
    TilesCache cache;

    AbstractPreview *preview;
    QPrinter *printer;
    QPaintDevice *currentDevice;
    //Определение цветов для рамки страницы и тени
    QColor bgc;
    QColor cl;
    QColor sh1, sh2, sh3;
    qreal s1, s2, s3;

    //Параметры размера страницы
    QSizeF paperSizeMM;     //Оригинальный размер страницы в мм
    QSize  paperSize;       //Размер страницы с округлением и в пикселах
    QSize  paperSizeScaled; //Размер страницы с учётом округлений и масштаба

    QSizeF pageSizeMM;      //Прямоугольник рабочей области страницы в мм
    QSize  pageSize;	    //то же, но с учетом округления и в пикселах
    QSize  pageSizeScaled;  // с учетом масштаба
   
	QVector<QRect> marginRects;
    int   w, h;        //высота и ширина рабочей области с учётом округления
    //Отступы страницы и расстояние между страницами
    int   pageMargin, interPageSpacing;
	int   pageLabelHeight;
	QRect pageLabelRect;
	
    // отступы на листе в мм
    qreal mm_left,
    	  mm_right,
    	  mm_top,
    	  mm_bottom;
    // то же, но в пикселах
    int   mp_left,
		  mp_right,
		  mp_top,
		  mp_bottom;
	// с учетом масштаба
    int   ms_left,
    	  ms_top;
//  int	  ms_right,
//    	  ms_bottom;
    //Общее количество страниц, колонок и строк страниц
    int cols, rows;
    //Параметры для передвижения страниц мышкой
    QMousePressData mousePressData;
    QPoint scrollBarValueOnMousePress;
    //Мастштаб
    double scale,
    	  scaleStep,
    	  scaleMax,
    	  scaleMin;

    //Координаты для начальной страницы
    qreal xx, yy;
    int offsXpage;
    int offsYpage;

    //Необходимое количество строк страниц для отрисовки и номер начальной
    //страницы с которой начинается отрисовка документа
    int countPagesRow, beginPageNumber;

    QList <QPageData *> pagesData;

    AbstractPreview::PreviewMode viewMode;
    int selectPageIndex;
    QPoint topLeftPage;
    QVector <QPageInfo>pagesRect;
    QRect currentPageRect;
    QPageInfo enterPageInfo;

    bool mouseTrackingPage;
    bool mouseEnterLeavePage;

    bool movedContents;
};
//--------------------------------------------------------------

TilesCache::TilesCache()
{
	maxWidth  = TILE_MAX_WIDTH;
	maxHeight = TILE_MAX_HEIGHT;
	maxTiles  = CACHE_SIZE_MAX / maxWidth / maxHeight;
#ifndef QT_NO_DEBUG
	qDebug() << "maxTiles: " << maxTiles;
#endif
}

TilesCache::~TilesCache()
{
	clear();
}

void TilesCache::clear()
{
#ifndef QT_NO_DEBUG
	qDebug() << "TilesCache::clear()";
#endif
	for (int i=tiles.size(); i>0; i--)
		delete tiles.takeLast();
	tiles.clear();
}

void TilesCache::setTileSize(int maxW, int maxH)
{
#ifndef QT_NO_DEBUG
	qDebug() << "TilesCache::setTileSize() " << maxW << "x" << maxH;
#endif
	clear();
	maxWidth = maxW;
	maxHeight = maxH;
	maxTiles = CACHE_SIZE_MAX / maxWidth / maxHeight;
#ifndef QT_NO_DEBUG
	qDebug() << "maxTiles: " << maxTiles;
#endif
}

void TilesCache::insertPages(int start, int count)
{
#ifndef QT_NO_DEBUG
	qDebug() << "TilesCache::insertPages() start: " << start << " count: " << count;
#endif
	for (int i=tiles.size()-1; i>=0; i--)
		if (tiles[i]->page >= start) tiles[i]->page+=count;
}

void TilesCache::deletePages(int start, int count)
{
	int end = start+count-1;
#ifndef QT_NO_DEBUG
	qDebug() << "TilesCache::deletePages() start: " << start << " count: " << count;
#endif
	for (int i=tiles.size()-1; i>=0; i--) {
		if (tiles[i]->page >  end) tiles[i]->page-=count;
		else if (tiles[i]->page >= start) delete tiles.takeAt(i);
	}
}

QPixmap* TilesCache::getTile(int page, int xidx, int yidx)
{
	Tile *tile;
#ifndef QT_NO_DEBUG
//	qDebug() << "TilesCache::getTile(): " << page << " x:" << xidx << " y:" << yidx;
#endif
	for (int i=0; i<tiles.size(); i++) {
		tile  = tiles[i];
		if (tile->page == page && tile->xidx == xidx && tile->yidx == yidx) {
			if (i) {
				tile  = tiles.takeAt(i);
				tiles.prepend(tile);
			}
#ifndef QT_NO_DEBUG
//	qDebug() << "Tile found at index " << i;
#endif
			return tile->data;
		}
	}
	return NULL;
}

void TilesCache::addTile(QPixmap *pix, int page, int x, int y)
{
	Tile *tile = new Tile(pix,page,x,y);
#ifndef QT_NO_DEBUG
//	qDebug() << "TilesCache::addTile(): " << tile->page << " x:" << tile->xidx << " y:" << tile->yidx;
#endif
	tiles.prepend(tile);
	if (tiles.size()>maxTiles) {
#ifndef QT_NO_DEBUG
		qDebug() << "maxTiles reached. removing last";
#endif
		delete tiles.takeLast();
	}
}

//--------------------------------------------------------------
AbstractPreviewPrivate::AbstractPreviewPrivate(AbstractPreview *preview, QPrinter *printer)
{
    mouseTrackingPage = false;
    mouseEnterLeavePage = true;
    movedContents = true;

    this->preview = preview;
    this->printer = printer;

    interPageSpacing = 8;
    pageMargin = 5;
	pageLabelHeight = 0;

    offsXpage = pageMargin;
    offsYpage = pageMargin;
    scale = 1.0;
    scaleStep = DEFAULT_SCALE_STEP;
    scaleMax = 2;
    scaleMin = 0.25;
    viewMode = AbstractPreview::Mode_Normal;
    selectPageIndex = 0;

    currentDevice = preview;
    setPageDefaults();
    changeScale();
}
//--------------------------------------------------------------
AbstractPreviewPrivate::~AbstractPreviewPrivate()
{
    clear();
}
//--------------------------------------------------------------
void AbstractPreviewPrivate::inits()
{
    preview->viewport()->setBackgroundRole(QPalette::Dark);

    bgc = preview->viewport()->palette().color(QPalette::Dark);
    cl = QColor(bgc.red() * 0.9, bgc.green() * 0.9, bgc.blue() * 0.9);
    sh1 = QColor(bgc.red() * 0.4, bgc.green() * 0.4, bgc.blue() * 0.4);
    sh2 = QColor(bgc.red() * 0.6, bgc.green() * 0.6, bgc.blue() * 0.6);
    sh3 = cl;

    interPageSpacing = 8;
    pageMargin = 5;
}
//--------------------------------------------------------------
void AbstractPreviewPrivate::recalcMarginsFromMM()
{
    mp_left   = inchesToPixelsX ( mmToInches(mm_left),   currentDevice);
    mp_right  = inchesToPixelsX ( mmToInches(mm_right),  currentDevice);
    mp_top    = inchesToPixelsY ( mmToInches(mm_top),    currentDevice);
    mp_bottom = inchesToPixelsY ( mmToInches(mm_bottom), currentDevice);
}
//--------------------------------------------------------------
void AbstractPreviewPrivate::changeScale()
{
    qreal fw,fh,pw,ph;
//	QRect rect;
	flushCache();
	ms_left   = mp_left * scale;
	ms_top    = mp_top * scale;
//	ms_right  = mp_right * scale;
//	ms_bottom = mp_bottom * scale;

    s1 = 1 / scale;
    s2 = 2 / scale;
    s3 = 3 / scale;

    fw = paperSize.width() * scale;
    if(fw - ((qreal)(int)fw) > 0.0) paperSizeScaled.setWidth((int)fw + 1);
    else paperSizeScaled.setWidth((int)fw);

    fh = paperSize.height() * scale;
    if(fh - ((qreal)(int)fh) > 0.0) paperSizeScaled.setHeight((int)fh + 1);
    else paperSizeScaled.setHeight((int)fh);

    pw = pageSize.width() * scale;
    if(pw - ((qreal)(int)pw) > 0.0) pageSizeScaled.setWidth((int)pw + 1);
    else pageSizeScaled.setWidth((int)pw);

    ph = pageSize.height() * scale;
    if(ph - ((qreal)(int)ph) > 0.0) pageSizeScaled.setHeight((int)ph + 1);
    else pageSizeScaled.setHeight((int)ph);

	fh = paperSizeScaled.height();
	fw = paperSizeScaled.width();
	ph = pageSizeScaled.height();
	pw = pageSizeScaled.width();

	marginRects.clear();
	marginRects << QRect( QPoint(1,1),             QSize( ms_left-1,  fh-1));
	marginRects << QRect( QPoint(ms_left + pw,1), QSize( fw - pw - ms_left, fh-1));
	marginRects << QRect( QPoint(ms_left,1),       QSize( pw, ms_top-1));
	marginRects << QRect( QPoint(ms_left, ph + ms_top), QSize( pw, fh - ph - ms_top));


    tileWidth  = qMin(pageSizeScaled.width()+1, TILE_MAX_WIDTH);
    tileCols = pageSizeScaled.width() / tileWidth;
    if (pageSizeScaled.width() % tileWidth) tileCols++;

    tileHeight = qMin(pageSizeScaled.height()+1, TILE_MAX_HEIGHT);
    tileRows = pageSizeScaled.height() / tileHeight;
    if (pageSizeScaled.height() % tileHeight) tileRows++;

    cache.setTileSize(tileWidth, tileHeight);


#ifndef QT_NO_DEBUG
    qDebug() << "page tiles: " << tileCols << "x" << tileRows;
#endif
}

//--------------------------------------------------------------
void AbstractPreviewPrivate::setPageDefaults()
{
    mm_left   = DEFAULT_MARGIN_LEFT;
    mm_right  = DEFAULT_MARGIN_RIGHT;
    mm_top    = DEFAULT_MARGIN_TOP;
    mm_bottom = DEFAULT_MARGIN_BOTTOM;
    recalcMarginsFromMM();

    printer->setPaperSize(QPrinter::A4);
    printer->setPageMargins(mm_left, mm_top, mm_right, mm_bottom, QPrinter::Millimeter);
#ifdef PRINTER_USE_FULLPAGE
    printer->setFullPage(true);
#else
    printer->setFullPage(false);
#endif
    setup();
}
//--------------------------------------------------------------
void AbstractPreviewPrivate::setup()
{
    paperSizeMM = printer->paperRect(QPrinter::Millimeter).size();
    paperSize = QSize(
		inchesToPixelsX ( mmToInches(paperSizeMM.width()),  currentDevice) +1,
		inchesToPixelsY ( mmToInches(paperSizeMM.height()), currentDevice) +1
	);

//    if(paperSizeMM.width() -  ((qreal)(int)paperSize.width()) > 0.0)  paperSize.setWidth ((int)paperSize.width() + 1);
//    if(paperSizeMM.height() - ((qreal)(int)paperSize.height()) > 0.0) paperSize.setHeight((int)paperSize.height() + 1);

//    if(pageSizeMM.width() -  ((qreal)(int)pageSize.width()) > 0.0)  pageSize.setWidth ((int)pageSize.width() + 1);
//    if(pageSizeMM.height() - ((qreal)(int)pageSize.height()) > 0.0) pageSize.setHeight((int)pageSize.height() + 1);

    printer->getPageMargins(&mm_left, &mm_top, &mm_right, &mm_bottom, QPrinter::Millimeter);
	recalcMarginsFromMM();

#if (PRINTER_USE_FULLPAGE == 1) || (QT_VERSION < 0x040500)
	pageSizeMM = QSize(
		paperSizeMM.width() - mm_left - mm_right,
		paperSizeMM.height() - mm_top - mm_bottom
	);
#else
	pageSizeMM = printer->pageRect(QPrinter::Millimeter).size();
#endif

	pageSize = QSize(
		inchesToPixelsX ( mmToInches(pageSizeMM.width()),  currentDevice) + 1,
		inchesToPixelsY ( mmToInches(pageSizeMM.height()), currentDevice) + 1
	);

    w = paperSize.width();
    h = paperSize.height();

#ifndef QT_NO_DEBUG
    qDebug() << "device DPI (phy) X:" << currentDevice->physicalDpiX() << " Y:" << currentDevice->physicalDpiY();
    qDebug() << "device DPI (log) X:" << currentDevice->logicalDpiX() << " Y:" << currentDevice->logicalDpiY();
    qDebug() << "paperSize (mm): " << paperSizeMM.width() << "x" << paperSizeMM.height();
    qDebug() << "pageSize  (mm): " << pageSizeMM.width() << "x" << pageSizeMM.height();
    qDebug() << "pageSize  (px): " << w << "x"<< h;
#endif
}
//--------------------------------------------------------------
void AbstractPreviewPrivate::calcParamDraw()
{
#ifndef QT_NO_DEBUG
//	qDebug() << "calcParamRedraw(): preview @ "<< preview;
#endif
    const int startRow = (preview->verticalScrollBar()->value() - pageMargin) /
                         (paperHeightToScale() + interPageSpacing + pageLabelHeight);
    const double hsp = paperHeightToScale() + interPageSpacing;
    const int vph = preview->viewport()->height();

    yy = ((paperHeightToScale() + interPageSpacing + pageLabelHeight) * startRow + pageMargin);
    yy = (qreal)preview->verticalScrollBar()->value() - yy;
    xx = offsXpage;

    if(!preview->horizontalScrollBar()->value()) {
        if(preview->viewport()->width() < paperWidthToScale())
            xx = offsXpage;
        else {
            if(cols == 1)
                xx = (preview->viewport()->width() - paperWidthToScale()) / 2;
            else xx = (preview->viewport()->width() - (paperWidthToScale() * cols + interPageSpacing * (cols - 1))) / 2;
        }
    }

    countPagesRow = 1;
    int t = (qreal)(-yy);
    t += hsp;
    while(t < vph) {
        t += hsp;
        countPagesRow++;
    }

    beginPageNumber = startRow * cols;
}
//--------------------------------------------------------------

int AbstractPreviewPrivate::paperWidthToScale()  { return paperSizeScaled.width(); }
int AbstractPreviewPrivate::paperHeightToScale() { return paperSizeScaled.height(); }
int AbstractPreviewPrivate::paperWidth()  { return paperSize.width(); }
int AbstractPreviewPrivate::paperHeight() { return paperSize.height(); }

//--------------------------------------------------------------
void AbstractPreviewPrivate::flushCache()
{
    cache.clear();
}
//--------------------------------------------------------------
int AbstractPreviewPrivate::addPages(int count/* = 1*/)
{
    if(count < 1)return 0;
    int c = 0;
    for(int i = 0; i < count; i++) {
        QPageData *pd = new QPageData;
        pd->index = pagesData.count();
        pd->reDraw = true;
        pagesData.append(pd);
        c++;
    }
    return c;
}
//--------------------------------------------------------------
int AbstractPreviewPrivate::insertPages(int &index, int count/* = 1*/)
{
    if(count < 1)return 0;
    if(index >= pagesData.count()) {
        index = pagesData.count();
        return addPages(count);
    }
    index = qMax(index, 0);
    int c = 0;
    for(int i = 0; i < count; i++) {
        QPageData *pd = new QPageData;
        pd->index = pagesData.count();
        pd->reDraw = true;
        pagesData.insert(index, pd);
        c++;
    }
    cache.insertPages(index, c);
    return c;
}
//--------------------------------------------------------------
int AbstractPreviewPrivate::deletePages(int index, int count/* = 1*/)
{
    if(count < 1)return 0;
    if(index >= pagesData.count())return 0;
    int c = 0;
    while(count) {
        delete pagesData.takeAt(index);
        count--;
        c++;
    }
    cache.deletePages(index, c);
    return c;
}
//--------------------------------------------------------------
void AbstractPreviewPrivate::flushPage(int page)
{
    cache.deletePages(page, 1);
}
//--------------------------------------------------------------
QPageData *AbstractPreviewPrivate::pageData(int index)
{
    if(index < 0 || index >= pagesData.count())return NULL;
    return pagesData[index];
}
//--------------------------------------------------------------
void AbstractPreviewPrivate::clear()
{
    flushCache();
    while(pagesData.count()) {
        delete pagesData.takeLast();
    }
}
//--------------------------------------------------------------
AbstractPreview::AbstractPreview(QWidget *parent, QPrinter *printer)
        : QAbstractScrollArea(parent)
{
    setMouseTracking(true);

    dv = new AbstractPreviewPrivate(this, printer);
	dp = NULL;
	d = dv;

    verticalScrollBar()->setSingleStep(20);
    horizontalScrollBar()->setSingleStep(20);
//    d->addPage();

    d->inits();
}
//--------------------------------------------------------------
AbstractPreview::AbstractPreview(QWidget *parent, QPrinter *printer, int _countPage)
        : QAbstractScrollArea(parent)
{
    setMouseTracking(true);

    dv = new AbstractPreviewPrivate(this, printer);
	dp = NULL;
	d = dv;

    verticalScrollBar()->setSingleStep(20);
    horizontalScrollBar()->setSingleStep(20);

    if(_countPage < 0)_countPage = 1;
    d->addPages(_countPage);

    d->inits();

    sizeToCountPage();
}
//--------------------------------------------------------------
AbstractPreview::~AbstractPreview()
{
    delete d;
}
//--------------------------------------------------------------
int  AbstractPreview::sizeToCountPage()
{
    if(!d->pagesData.count()) {
        verticalScrollBar()->setRange(0, 0);
        horizontalScrollBar()->setRange(0, 0);
    	d->cols = 0;
    	d->rows = 0;
		return 0;
    }

    int cols;
    if(d->viewMode == AbstractPreview::Mode_Normal) {
        cols = (int)(viewport()->width() - 2 * d->pageMargin + d->interPageSpacing) /
               (d->paperSize.width() * d->scale + d->interPageSpacing);
	} else {
		cols = 1;
	}
    if(!cols)cols = 1;
    if(cols > d->pagesData.count())cols = d->pagesData.count();

    int rows = (d->pagesData.count() + cols - 1) / cols;
    int h = qRound(rows * (d->paperHeightToScale() + d->pageLabelHeight) +
                   (rows - 1.) * d->interPageSpacing + 
				   2. * d->pageMargin);
    int w = qRound(cols * d->paperWidthToScale() + 2. * d->pageMargin);
    horizontalScrollBar()->setRange(0, w - viewport()->width());
    horizontalScrollBar()->setPageStep(viewport()->width());
    verticalScrollBar()->setRange(0, h - viewport()->height());
    verticalScrollBar()->setPageStep(viewport()->height());

    d->cols = cols;
    d->rows = rows;
    return 0;
}
//--------------------------------------------------------------
void AbstractPreview::autoscaleThumbs()
{
//	qDebug() << "autoscaleThumbs(): w=" << d->w << " viewport()->width()=" << viewport()->width();
	int w = viewport()->width() - 2 * d->pageMargin + d->interPageSpacing - 10;
	if (w<1) w=1;
	qreal scale = w / (float) d->paperSize.width();

	if (scale != d->scale) {
		d->scale = scale;
		d->changeScale();
		d->pageLabelRect = QRect(1,d->paperSizeScaled.height() + 3, d->paperSizeScaled.width(), d->pageLabelHeight);
	}
    sizeToCountPage();
}
//--------------------------------------------------------------
void AbstractPreview::resizeEvent(QResizeEvent *)
{
    d->enterPageInfo = QPageInfo();

    if(d->viewMode == AbstractPreview::Mode_Thumbs) {
		autoscaleThumbs();
	} else {
		sizeToCountPage();
	}
    d->calcParamDraw();
}
//--------------------------------------------------------------
void AbstractPreview::scrollContentsBy(int dx, int dy)
{
    d->enterPageInfo = QPageInfo();

    d->offsXpage += dx;
    d->offsYpage += dy;
    if(dx || dy) d->calcParamDraw();
    updatePreview();
}
//--------------------------------------------------------------
void AbstractPreview::paintBorder(QPainter *p)
{
#if 0
    p->setPen(d->cl);
    p->drawRect( QRect( QPoint(0,0), d->paperSize) );

//	p->setPen(Qt::red);
//	p->drawRect( QRect( QPoint(d->mp_left, d->mp_top), d->pageSize - QSize(1,1) ) );
#endif

    p->setPen(d->sh1);
    p->drawLine(QLineF(d->w + d->s1, d->s3, d->w + d->s1, d->h + d->s1));
    p->drawLine(QLineF(d->s3, d->h + d->s1, d->w + d->s1, d->h + d->s1));

    p->setPen(d->sh2);
    p->drawLine(QLineF(d->w + d->s2, d->s3, d->w + d->s2, d->h + d->s2));
    p->drawLine(QLineF(d->s3, d->h + d->s2, d->w + d->s2, d->h + d->s2));

    p->setPen(d->sh3);
    p->drawLine(QLineF(d->w + d->s3, d->s3, d->w + d->s3, d->h + d->s3));
    p->drawLine(QLineF(d->s3, d->h + d->s3, d->w + d->s3, d->h + d->s3));
}
//--------------------------------------------------------------

QPixmap* AbstractPreview::getTile(int page, int tx, int ty)
{
	QPixmap *pix = d->cache.getTile(page,tx,ty);
	if(!pix) {
#ifndef QT_NO_DEBUG
//	qDebug() << "Tile not found in cache, rendering new one";
#endif
               // tile->data = new QPixmap(d->paperWidthToScale() + 1, d->paperHeightToScale() + 1);
		pix = new QPixmap(
			qMin(d->tileWidth,  (d->pageSizeScaled.width()  - d->tileWidth * tx)),
			qMin(d->tileHeight, (d->pageSizeScaled.height() - d->tileHeight * ty))
		);
//                qDebug() << "page " << page << "  tile: " << ty << "," << tx << " " << pix->width() << "x" << pix->height();
        //tile->data->fill(d->bkg);
        QPainter pp(pix);
        // pp.setPen(d->cl);
        pp.setPen(Qt::white);
        pp.setBrush(Qt::white);
        pp.drawRect( QRect(QPoint(0,0), pix->size()) );
        pp.scale(d->scale, d->scale);
//		pp.translate(d->mp_left, d->mp_top);

        paintPage(&pp, page, QRect( QPoint(d->tileWidth * tx, d->tileHeight * ty) / d->scale, pix->size() / d->scale ) );

		d->cache.addTile(pix, page, tx, ty);
	}
/*
#ifndef QT_NO_DEBUG
	    qDebug() << "QPainter  @" << (void*)p;
	    qDebug() << "Tile      @" << (void*)tile;
	    if (tile) {
	    	qDebug() << "Tile data @" << (void*)tile->data;
	    }
#endif
*/
	return pix;
}
void AbstractPreview::updatePage(int page, QRect rect)
{
    //функция обновления участка страницы
    QPixmap pix(rect.size());
    pix.fill(Qt::white);
    QPainter p(&pix);
    paintPage(&p, page, rect);

    QPageInfo info = pageInfo(page);
    int ty_sta, ty_end, tx_sta, tx_end;
    const int yy = info.rect().top(), xx = info.rect().left();
    const int yy2 = yy + d->pageSizeScaled.height(),
              xx2 = xx + d->pageSizeScaled.width();
    ty_sta = (yy < 0) ? (-yy / d->tileHeight) : 0;
    ty_end = (yy2 < viewport()->height()) ? d->tileRows :
             ((viewport()->height() - yy) / d->tileHeight) + 1;

    tx_sta = (xx<0) ? (-xx / d->tileWidth) : 0;
    tx_end = (xx2 < viewport()->width()) ? d->tileCols :
             ((viewport()->width() - xx) / d->tileWidth) + 1;

    for(int ty = ty_sta; ty < ty_end; ty++) {
        for(int tx = tx_sta; tx < tx_end; tx++) {
            QRect tileRect(d->tileWidth * tx, d->tileHeight * ty,
                           d->tileWidth, d->tileHeight);
            QRect sec = tileRect.intersected(rect);
            if(!sec.isValid()) continue;
            QPixmap *px = getTile(page, tx, ty);
            if(!px)continue;
            QPainter pp(px);
            pp.drawPixmap(sec.topLeft() - tileRect.topLeft(),
                          pix,
                          QRect(sec.topLeft() - rect.topLeft(),
                                sec.size()));
        }
    }

    viewport()->update();
}

void AbstractPreview::paintEvent(QPaintEvent *)
{
        d->pagesRect.clear();//Буфер координат текущих отрисованных страниц

	QPainter *p = new QPainter(viewport());
//    QPixmap *pix = NULL;
	int page = d->beginPageNumber;
	const QColor bkg = viewport()->backgroundRole();
	int xx = d->xx, yy = -d->yy;
	int xx2,yy2;
	int ty_sta,ty_end,tx_sta,tx_end;
	QRect mrect;

        p->save();

	p->translate(xx, yy);
	yy += (d->ms_top);

	for(int y = 0; y < d->countPagesRow; y++) {
		xx = d->xx + d->ms_left;
		yy2 = yy+d->pageSizeScaled.height();
		ty_sta = (yy<0) ? (-yy / d->tileHeight) : 0;
		ty_end = (yy2<viewport()->height()) ? d->tileRows : ( (viewport()->height()-yy) / d->tileHeight)+1;

		p->save();
		for(int x = 0; x < d->cols; x++) {
			xx2 = xx+d->pageSizeScaled.width();
			tx_sta = (xx<0) ? (-xx / d->tileWidth) : 0;
			tx_end = (xx2<viewport()->width()) ? d->tileCols : ( (viewport()->width()-xx) / d->tileWidth)+1;

            p->save();
            p->translate(d->ms_left, d->ms_top);
#ifdef DEBUG_PAINT_TIME
			timeval tb,te;
			gettimeofday(&tb, NULL);
#endif
			for(int ty=ty_sta; ty<ty_end; ty++) {
				for(int tx=tx_sta; tx<tx_end; tx++) {
					p->drawPixmap(d->tileWidth * tx, d->tileHeight * ty, *(getTile(page, tx, ty)));
				}
			}
#ifdef DEBUG_PAINT_TIME
			gettimeofday(&te, NULL);
			qDebug() << "Page: " << page << "  " << tx_sta << "." << ty_sta << " - " << tx_end << "." << ty_end
			<< QString("  time: %1 s").arg(te.tv_sec - tb.tv_sec + (te.tv_usec - tb.tv_usec)/1000000.0,0,'f',4);
#endif
//	    p->drawPixmap(0, 0, *(getTile(page, 0, 0)));
			p->restore();

			mrect = QRect( QPoint(-xx + d->ms_left, -yy + d->ms_top), viewport()->size() );
			for (int i=0; i<d->marginRects.size(); i++) {
				if (!i && d->viewMode == AbstractPreview::Mode_Normal) {
				//	qDebug() << "page #" << page << mrect << d->marginRects[i] << d->marginRects[i].intersected(mrect);
				}
				p->fillRect(d->marginRects[i].intersected(mrect), QBrush(Qt::white));
			}

			p->save();
			p->scale(d->scale, d->scale);

			if(d->viewMode == AbstractPreview::Mode_Thumbs && d->selectPageIndex == page) {
#ifndef QT_NO_DEBUG
				qDebug() << "Thumbs selected page: " << page;
#endif
				QRect rr = QRect( QPoint(0,0), d->paperSize);
				//p->setCompositionMode(QPainter::CompositionMode_Multiply);
				//p->setCompositionMode(QPainter::CompositionMode_Xor);
				QColor col = palette().color(QPalette::Highlight);
				QColor colb = palette().color(QPalette::Highlight);
				colb.setAlpha(0x80);
				p->setPen(col);
				p->setBrush(colb);
				p->drawRect(rr);
				//p->setCompositionMode(QPainter::CompositionMode_SourceOver);
			}
			paintBorder(p);

                        //Сохраняем параметры прорисованных страниц
                        QPageInfo pg(page,
                                     QRect(xx - d->ms_left,
                                           yy - d->ms_top,
                                           d->paperWidthToScale(),
                                           d->paperHeightToScale()));
                        d->pagesRect.append(pg);

                        //Дополнительная прорисовка страницы
                        QRect rr(0, 0, viewport()->width() - 1, viewport()->height() - 1);
                        QRect rr1 = pg.rect().intersected(rr);
                        p->save();
                        additionalPaintEvent(p, page, rr1); //Дополнительная прорисовка страниц
                        p->restore();

			p->restore();

			if (d->pageLabelHeight) {
				p->setPen(palette().color(QPalette::Text));
				p->drawText( d->pageLabelRect, Qt::AlignCenter, QString::number(page+1));
			}

			page++;
			if(page == d->pagesData.count()) break;
			xx += (d->paperWidthToScale() + d->interPageSpacing);
			p->translate((d->paperWidthToScale() + d->interPageSpacing), 0);
		} //end for(int x = 0; x < d->cols; x++)

		if(d->viewMode != Mode_Thumbs) {
			int _y = yy - d->ms_top;
			if(_y < (viewport()->height()>>1)) {
				d->selectPageIndex = page - 1;
			}
		}

		p->restore();
                if(page == d->pagesData.count())break;
		p->translate(0, d->paperHeightToScale() + d->interPageSpacing + d->pageLabelHeight);
		yy += (d->paperHeightToScale() + d->interPageSpacing + d->pageLabelHeight);
	} //end for(int y = 0; y < d->countPagesRow; y++)
        p->restore();
//        for(int i = 0; i < d->pagesRect.count(); i++) {
//            p->setPen(Qt::red);
//            p->drawRect(d->pagesRect[i].rect());
//        }
	delete p;

        if(d->viewMode != Mode_Thumbs && d->selectPageIndex >= 0) {
            emit currentPage(d->selectPageIndex);
        }
}
//--------------------------------------------------------------
void AbstractPreview::additionalPaintEvent(QPainter *p, int page, const QRect &rect)
{
    Q_UNUSED(p);
    Q_UNUSED(page);
}
//--------------------------------------------------------------
QSize AbstractPreview::getPaperSize() { return d->paperSize; }
QSize AbstractPreview::getPageSize()  { return d->pageSize; }
//--------------------------------------------------------------
int AbstractPreview::page(QPoint point)
{
    const int count = d->pagesRect.count();
    int sel = -1;
    for(int i = 0; i < count; i++) {
        if(d->pagesRect[i].rect().contains(point)) {
            sel = d->pagesRect[i].page();
            break;
        }
    }

    if(sel >= 0) return sel;
    return -1;
}
//--------------------------------------------------------------
QRect AbstractPreview::pageRect(QPoint point)
{
    const int count = d->pagesRect.count();
    int sel = -1;
    for(int i = 0; i < count; i++) {
        if(d->pagesRect[i].rect().contains(point)) {
            return d->pagesRect[i].rect();
            break;
        }
    }
    return QRect();
}
//--------------------------------------------------------------
QPoint AbstractPreview::toPage(QPoint point)
{
    const int count = d->pagesRect.count();
    for(int i = 0; i < count; i++) {
        if(d->pagesRect[i].rect().contains(point)) {
            return point - d->pagesRect[i].rect().topLeft();
        }
    }

    return QPoint();
}
//--------------------------------------------------------------
QPoint AbstractPreview::toCurrentPage(QPoint point)
{
    if(!d->currentPageRect.contains(point))return QPoint();
    return point - d->currentPageRect.topLeft();
}
//--------------------------------------------------------------
QPageInfo AbstractPreview::pageInfo(int page)
{
    if(d->enterPageInfo.isValid() && d->enterPageInfo.page() == page)
        return d->enterPageInfo;

    const int count = d->pagesRect.count();
    QPageInfo info;
    //QRect rect;
    for(int  i = 0; i < count; i++) {
        if(d->pagesRect[i].page() == page) {
            info = QPageInfo(page, d->pagesRect[i].rect());
            return info;
        }
    }
    return info;
}
//--------------------------------------------------------------
QPageInfo AbstractPreview::pageInfo(const QPoint &point)
{
    int page = -1;
    QRect rect;
    if(!d->enterPageInfo.rect().contains(point)) {
        const int count = d->pagesRect.count();
        for(int i = 0; i < count; i++) {
            rect = d->pagesRect[i].rect();
            if(rect.contains(point)) {
                page = d->pagesRect[i].page();
                break;
            }
        }
        if(page == -1) d->enterPageInfo = QPageInfo();
        else d->enterPageInfo = QPageInfo(page, rect);
    }
    return d->enterPageInfo;
}
//--------------------------------------------------------------
QVector <QPageInfo>AbstractPreview::visiblePages()
{
    return d->pagesRect;
}
//--------------------------------------------------------------
void AbstractPreview::startMoveContext(const QPoint &point)
{
    d->mousePressData.press = true;
    d->mousePressData.pos = point;
    d->scrollBarValueOnMousePress.rx() = horizontalScrollBar()->value();
    d->scrollBarValueOnMousePress.ry() = verticalScrollBar()->value();
    //d->scrollBarValueOnMousePress = point;
}
//--------------------------------------------------------------
void AbstractPreview::moveContext(const QPoint &point)
{
    if(!d->mousePressData.press && d->scrollBarValueOnMousePress.isNull())return;

    horizontalScrollBar()->setValue(d->scrollBarValueOnMousePress.x() -
                                    point.x() + d->mousePressData.pos.x());
    verticalScrollBar()->setValue(d->scrollBarValueOnMousePress.y() -
                                  point.y() + d->mousePressData.pos.y());
    d->calcParamDraw();
    horizontalScrollBar()->update();
}
//--------------------------------------------------------------
void AbstractPreview::endMoveContext()
{
    d->mousePressData.press = false;
    d->mousePressData.pos = QPoint();

    d->scrollBarValueOnMousePress = QPoint();
}
//--------------------------------------------------------------
void AbstractPreview::mousePressEvent(QMouseEvent *e)
{
    startMoveContext(e->pos());

    const int count = d->pagesRect.count();
    int sel = -1;
    for(int i = 0; i < count; i++) {
        if(d->pagesRect[i].rect().contains(e->pos())) {
            sel = d->pagesRect[i].page();
            d->currentPageRect = d->pagesRect[i].rect();
            break;
        }
    }

    if(sel >= 0) {
#warning disabled due to enable multiple selection of same page (i.e. to return to viewed page)
//      if(d->selectPageIndex == sel) return;
        d->selectPageIndex = sel;
        //Вызываем событие клика мышкой на странице
        mousePressPageEvent(e, d->selectPageIndex, d->currentPageRect, toCurrentPage(e->pos()));

        emit pageSelected(d->selectPageIndex);
        updatePreview();
    } else {
        if(d->selectPageIndex == -1) return;
        d->selectPageIndex = -1;
        updatePreview();
    }
}
//--------------------------------------------------------------
void AbstractPreview::mouseMoveEvent(QMouseEvent *e)
{
    if(!d->mousePressData.press) {
        pageInfo(e->pos());
        if(d->enterPageInfo.isValid())
            mouseMovePageEvent(e, d->enterPageInfo);

        e->ignore();
        return;
    }

    if(d->movedContents)
        moveContext(e->pos());
}
//--------------------------------------------------------------
void AbstractPreview::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    endMoveContext();
}
//--------------------------------------------------------------
void AbstractPreview::mousePressPageEvent(QMouseEvent *event, int page,
                                          QRect rect, QPoint point)
{
    Q_UNUSED(event);
    Q_UNUSED(page);
    Q_UNUSED(rect);
    Q_UNUSED(point);
}
//--------------------------------------------------------------
void AbstractPreview::mouseMovePageEvent(QMouseEvent *event, const QPageInfo &info)
{
    Q_UNUSED(event);
    Q_UNUSED(info);
}
//--------------------------------------------------------------
void AbstractPreview::wheelEvent(QWheelEvent * event)
{
    if(event->modifiers() != Qt::ControlModifier) {
        QAbstractScrollArea::wheelEvent(event);
        return;
    }

    if (event->delta() > 0) scaleIn();
    else scaleOut();
}
//--------------------------------------------------------------
void AbstractPreview::setInterPageSpacing(int spacing)
{
    if(d->interPageSpacing == spacing)return;
    d->interPageSpacing = spacing;
    sizeToCountPage();
    updatePreview();
}
//--------------------------------------------------------------
int  AbstractPreview::interPageSpacing()
{
    return d->interPageSpacing;
}
//--------------------------------------------------------------
void AbstractPreview::setPageMargin(int margin)
{
    if(d->pageMargin == margin)return;
    d->pageMargin = margin;
    sizeToCountPage();
    updatePreview();
}
//--------------------------------------------------------------
int  AbstractPreview::pageMargin()
{
    return d->pageMargin;
}
//--------------------------------------------------------------
int  AbstractPreview::pageCount()
{
    return d->pagesData.count();
}
//--------------------------------------------------------------

void AbstractPreview::setMargins(qreal ml, qreal mt, qreal mr, qreal mb)
{
    if(d->mm_left == ml && d->mm_right == mr &&
       d->mm_top == mt && d->mm_bottom == mb)return;
    d->mm_left = ml;
    d->mm_right = mr;
    d->mm_top = mt;
    d->mm_bottom = mb;

    d->mp_left = inchesToPixelsX ( mmToInches(d->mm_left), this);
    d->mp_right = inchesToPixelsX ( mmToInches(d->mm_right), this);
    d->mp_bottom = inchesToPixelsX ( mmToInches(d->mm_bottom), this);
    d->mp_top = inchesToPixelsX ( mmToInches(d->mm_top), this);
    updatePreview();
}
//--------------------------------------------------------------
void AbstractPreview::setMarginLeft(qreal ml)
{
    if(d->mm_left == ml)return;
    d->mm_left = ml;
    d->mp_left = inchesToPixelsX ( mmToInches(d->mm_left), this);
    updatePreview();
}
//--------------------------------------------------------------
void AbstractPreview::setMarginTop(qreal mt)
{
    if(d->mm_top == mt)return;
    d->mm_top = mt;
    d->mp_top = inchesToPixelsX ( mmToInches(d->mm_top), this);
    updatePreview();
}
//--------------------------------------------------------------
void AbstractPreview::setMarginRight(qreal mr)
{
    if(d->mm_right == mr)return;
    d->mm_right = mr;
    d->mp_right = inchesToPixelsX ( mmToInches(d->mm_right), this);
    updatePreview();
}
//--------------------------------------------------------------
void AbstractPreview::setMarginBottom(qreal mb)
{
    if(d->mm_bottom == mb)return;
    d->mm_bottom = mb;
    d->mp_bottom = inchesToPixelsX ( mmToInches(d->mm_bottom), this);
    updatePreview();
}
//--------------------------------------------------------------

qreal AbstractPreview::marginLeft() { return d->mm_left; }
qreal AbstractPreview::marginTop()  { return d->mm_top; }
qreal AbstractPreview::marginRight() { return d->mm_right; }
qreal AbstractPreview::marginBottom() { return d->mm_bottom; }

int AbstractPreview::marginLeftPx() { return d->mp_left; }
int AbstractPreview::marginTopPx() { return d->mp_top; }
int AbstractPreview::marginRightPx() { return d->mp_right; }
int AbstractPreview::marginBottomPx() { return d->mp_bottom; }

//--------------------------------------------------------------
void AbstractPreview::addedPages(int count, int begin)
{
}
//--------------------------------------------------------------
int AbstractPreview::addPages(int count/* = 1*/)
{
    int cc = d->pagesData.count();
    int c = d->addPages(count);
    addedPages(c, cc);
    sizeToCountPage();
    updatePreview();
    return c;
}
//--------------------------------------------------------------
int AbstractPreview::insertPages(int index, int count/* = 1*/)
{
    int c = d->insertPages(index, count);
    addedPages(c, index);
    updatePreview();
    return c;
}
//--------------------------------------------------------------
int AbstractPreview::deletePages(int index, int count/* = 1*/)
{
    int c = d->deletePages(index, count);
    updatePreview();
    return c;
}
//--------------------------------------------------------------
void AbstractPreview::pageSetup()
{
    QPageSetupDialog dlg(d->printer, this);
    if (dlg.exec() == QDialog::Accepted) {
        setupPageFormat();
        emit pageFormatChanged();
    }
}
//--------------------------------------------------------------
void AbstractPreview::setPageData(int index, QVariant data)
{
    if(index < 0 || index >= d->pagesData.count())return;
    d->pagesData[index]->data = data;
}
//--------------------------------------------------------------
QVariant AbstractPreview::pageData(int index)
{
    if(index < 0 || index >= d->pagesData.count())return QVariant();
    return d->pagesData[index]->data;
}
//--------------------------------------------------------------
void AbstractPreview::setViewMode(AbstractPreview::PreviewMode mode)
{
    if(d->viewMode == mode)return;
	if (mode == Mode_Thumbs) {
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		d->pageLabelHeight = 18;
	} else {
		setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		d->pageLabelHeight = 0;
	}
    d->viewMode = mode;
    sizeToCountPage();
    d->calcParamDraw();
    updatePreview();
}
//--------------------------------------------------------------
AbstractPreview::PreviewMode AbstractPreview::viewMode()
{
    return d->viewMode;
}
//--------------------------------------------------------------
#ifdef PRINTER_CHANGE_DEVICE
void AbstractPreview::changeDevice(QPaintDevice* device)
{
	if (device == d->printer) {
		if (!dp) dp = new AbstractPreviewPrivate(this, d->printer);
		d = dp;
		d->currentDevice = device;
		deviceChanged(d->currentDevice);
		setupPageFormat();
	} else {
		d = dv;
		if (dp) {
			delete dp;
			dp = NULL;
		}
		d->currentDevice = device;
		deviceChanged(d->currentDevice);
	}
}
#endif
//--------------------------------------------------------------
void AbstractPreview::print()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: AbstractPreview::print()");
#endif
    QPrintDialog *dlg = new QPrintDialog(d->printer, NULL);
//    d->printer->setOutputFormat(QPrinter::PdfFormat);
    if(dlg->exec() == QDialog::Accepted) {
		printPages(d->printer);
    }
    delete dlg;
#ifndef QT_NO_DEBUG
	qDebug("END: AbstractPreview::print()");
#endif
}

void AbstractPreview::print(QPrinter *printer)
{
	printPages(printer);
}

void AbstractPreview::printPages(QPrinter *printer)
{
#ifdef DEBUG_PRINT_TIME
		timeval tpb,tpe;
#ifdef PRINTER_CHANGE_DEVICE
		timeval tsb;
#endif
#endif
		int fromPage = printer->fromPage();
		int toPage = printer->toPage();
        int count = d->pagesData.count();
		if (fromPage) fromPage--;
		if (!toPage || toPage>count) toPage = count;

#ifdef PRINTER_CHANGE_DEVICE
#ifdef DEBUG_PRINT_TIME
		gettimeofday(&tsb, NULL);
#endif
		changeDevice(printer);
#endif
#ifdef DEBUG_PRINT_TIME
		gettimeofday(&tpb, NULL);
#endif
		QPainter p;
		p.begin(printer);
#ifndef PRINTER_CHANGE_DEVICE
		float scaleX = (float) d->printer->logicalDpiX() / d->preview->logicalDpiX();
		float scaleY = (float) d->printer->logicalDpiY() / d->preview->logicalDpiY();
		qDebug() << "Printer scale: "<< scaleX << "x" << scaleY;
		p.scale(scaleX, scaleY);
#endif
#if (PRINTER_USE_FULLPAGE == 1)
		p.translate(d->mp_left, d->mp_top);
#endif
		for(int i = fromPage; i < toPage; i++) {
#ifndef QT_NO_DEBUG
			qDebug() << "printing page " << i << " ...";
#endif
			if (i != fromPage) d->printer->newPage();
			paintPage(&p, i, QRect( QPoint(0,0), d->pageSize));
		}
		p.end();
#ifdef DEBUG_PRINT_TIME
		gettimeofday(&tpe, NULL);
#endif

#ifdef PRINTER_CHANGE_DEVICE
		changeDevice(d->preview);
#ifdef DEBUG_PRINT_TIME
		qDebug() << "Setup time: " << QString(" %1 s").arg(tpb.tv_sec - tsb.tv_sec + (tpb.tv_usec - tsb.tv_usec)/1000000.0,0,'f',4);
#endif
#endif

#ifdef DEBUG_PRINT_TIME
		qDebug() << "Pages time: " << QString(" %1 s").arg(tpe.tv_sec - tpb.tv_sec + (tpe.tv_usec - tpb.tv_usec)/1000000.0,0,'f',4);
#endif
}
//--------------------------------------------------------------
void AbstractPreview::repaintPreview()
{
    d->flushCache();
    updatePreview();
}
//--------------------------------------------------------------
void AbstractPreview::repaintPage(int page)
{
    d->flushPage(page);
    updatePreview();
}
//--------------------------------------------------------------
void AbstractPreview::scaleIn()
{
    if(d->scale >= d->scaleMax)return;
    setScale( d->scale * d->scaleStep );
}
//--------------------------------------------------------------
void AbstractPreview::scaleOut()
{
    if(d->scale <= d->scaleMin)return;
    setScale( d->scale / d->scaleStep );
}
//--------------------------------------------------------------
void AbstractPreview::scaleOrig() { setScale(1.0); }

void AbstractPreview::setScale(double z)
{
    if(d->scale == z) return;
    qSwap(d->scale, z);

    d->changeScale();
    sizeToCountPage();
    d->calcParamDraw();
    updatePreview();

    scaleEvent(z, d->scale);
}
//--------------------------------------------------------------
void AbstractPreview::setScaleStep(double step)
{
    d->scaleStep = step;
}
//--------------------------------------------------------------
double AbstractPreview::scaleStep()
{
    return d->scaleStep;
}
//--------------------------------------------------------------
void AbstractPreview::setScaleRange(double zmin, double zmax)
{
	if (zmin > zmax) { qSwap(zmin,zmax); }
	d->scaleMin = zmin;
	d->scaleMax = zmax;
	if(d->scale < zmin) {
		setScale(zmin);
		return;
	}
	if(d->scale > zmax) {
		setScale(zmax);
		return;
	}
}
//--------------------------------------------------------------
void AbstractPreview::setScaleMaximum(double zmax)
{
    if (zmax < d->scaleMin) zmax = d->scaleMin;
    d->scaleMax = zmax;
    if(d->scale > zmax) setScale(zmax);
}
//--------------------------------------------------------------
void AbstractPreview::setScaleMinimum(double zmin)
{
    if (zmin > d->scaleMax) zmin = d->scaleMax;
    d->scaleMin = zmin;
    if(d->scale < zmin) setScale(zmin);
}
//--------------------------------------------------------------
double AbstractPreview::scaleMaximum()
{
    return d->scaleMax;
}
//--------------------------------------------------------------
double AbstractPreview::scaleMinimum()
{
    return d->scaleMin;
}
//--------------------------------------------------------------
void AbstractPreview::scaleEvent(const double &scaleOld, const double &scaleNew)
{
    Q_UNUSED(scaleOld);
    emit scaleChanged(scaleNew);
}
//--------------------------------------------------------------
void AbstractPreview::gotoPage(int index)
{
    int row = index / d->cols;
    int y = row * (d->paperHeightToScale() + d->interPageSpacing + d->pageLabelHeight);
    int y2 = height() - d->paperHeightToScale() - 2*d->pageLabelHeight;

    if (d->viewMode == Mode_Thumbs) {
        d->selectPageIndex = index;
        updatePreview();
        if (y<=verticalScrollBar()->value()) {
            verticalScrollBar()->setValue(y);
        } else if (y-y2 > verticalScrollBar()->value()) {
            verticalScrollBar()->setValue(y-y2);
        }
    } else {
        verticalScrollBar()->setValue(y);
    }
}
//--------------------------------------------------------------
QSize AbstractPreview::paperSize()
{
    return d->paperSize;
}
//--------------------------------------------------------------
void AbstractPreview::clear()
{
    d->clear();
    clearEvent();

    d->changeScale();
    sizeToCountPage();
    d->calcParamDraw();
    updatePreview();
}
//--------------------------------------------------------------
void AbstractPreview::setupPageFormat()
{
#ifndef QT_NO_DEBUG
	qDebug("setupPageFormat()");
#endif
    d->setup();
    if (d->viewMode == Mode_Thumbs) {
            autoscaleThumbs();
    }
    updatePageFormat();
    d->changeScale();
    sizeToCountPage();
    d->calcParamDraw();
    updatePreview();
}
//--------------------------------------------------------------
int AbstractPreview::currentPage()
{
    return d->selectPageIndex;
}
//--------------------------------------------------------------
qreal AbstractPreview::scale()
{
    return d->scale;
}
//--------------------------------------------------------------
QPixmap *AbstractPreview::grabPage(int page, const QRectF &rect)
{
    if(!(page >= 0 && page < pageCount()))return NULL;
    return NULL;
}
//--------------------------------------------------------------
QPixmap *AbstractPreview::grabPage(int page, qreal left, qreal top,
                                   qreal width, qreal height)
{
    if(!(page >= 0 && page < pageCount()))return NULL;
    return NULL;
}
//--------------------------------------------------------------
void AbstractPreview::setMouseTrackingPage(bool mtp)
{
    d->mouseTrackingPage = mtp;
    setMouseTracking(d->mouseTrackingPage);
}
//--------------------------------------------------------------
bool AbstractPreview::isMouseTrackingPage() { return d->mouseTrackingPage; }
QPageInfo AbstractPreview::enterPageInfo() { return d->enterPageInfo; }
void AbstractPreview::setMouseEnterLeavePage(bool melp) { d->mouseEnterLeavePage = melp; }
bool AbstractPreview::mouseEnterLeavePage() { return d->mouseEnterLeavePage; }
//--------------------------------------------------------------
void AbstractPreview::setMovedContents(bool move) { d->movedContents = move; }
bool AbstractPreview::movedContents() { return d->movedContents; }
//--------------------------------------------------------------
