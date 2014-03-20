/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009 Maxim Aldanov <aldmax@mail.ru>, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 * */

#ifndef QPREVIEW_H
#define QPREVIEW_H

#include <QAbstractScrollArea>
#include <QPrinter>
#include <dpi_metrics.h>
#include <QVariant>
#include <QEvent>
//#define PRINTER_CHANGE_DEVICE

class QResizeEvent;
class QPaintEvent;
class QAbstractScrollArea;
class QMouseEvent;
class QMousePressData;
class AbstractPreviewPrivate;
class QVariant;
class QWheelEvent;

class QPageInfo {
public:
    QPageInfo() {
        fpage = -1;
        frect = QRect();
    }
    QPageInfo(int page, const QRect &r) {
        fpage = page;
        frect = r;
    }
    ~QPageInfo() {}

    QPageInfo &operator=(const QPageInfo &data) {
        fpage = data.fpage;
        frect = data.frect;
        return *this;
    }

    bool isValid() { return (bool)(fpage != -1 && !frect.isEmpty()); }

    int page()const;
    QRect rect()const;
private:
    int fpage;
    QRect frect;
};

class AbstractPreview : public QAbstractScrollArea
{
	Q_OBJECT

friend class AbstractPreviewPrivate;

signals:
	void pageSelected(int);
	void scaleChanged(double);
	void pageFormatChanged();
	void currentPage(int);
public:
	enum PreviewMode {Mode_Normal, Mode_Thumbs};
	AbstractPreview(QWidget *parent, QPrinter *printer);
	AbstractPreview(QWidget *parent, QPrinter *printer, int _countPage);
	virtual ~AbstractPreview();

	QSize paperSize();

	void setInterPageSpacing(int spacing);
	int interPageSpacing();
	void setPageMargin(int margin);
	int pageMargin();

	int pageCount();

	int addPages(int count = 1);
	int insertPages(int index, int count = 1);
	int deletePages(int index, int count = 1);
	void clear();
	void setPageData(int index, QVariant data);
	QVariant pageData(int index);


	void setMargins(qreal ml, qreal mt, qreal mr, qreal mb);
	void setMarginLeft(qreal ml);
	void setMarginTop(qreal mt);
	void setMarginRight(qreal mr);
	void setMarginBottom(qreal mb);
	qreal marginLeft();
	qreal marginTop();
	qreal marginRight();
	qreal marginBottom();
        int marginLeftPx();
        int marginTopPx();
        int marginRightPx();
        int marginBottomPx();


	void setViewMode(AbstractPreview::PreviewMode mode);
	AbstractPreview::PreviewMode viewMode();

	inline void updatePreview() { viewport()->update(); };
	void repaintPreview();
        void repaintPage(int page);
        void updatePage(int page, QRect rect);

	void setScaleStep(double step);
	double scaleStep();
	void setScaleRange(double zmin, double zmax);
	void setScaleMaximum(double zmax);
	void setScaleMinimum(double zmin);
	double scaleMaximum();
	double scaleMinimum();

	int currentPage();
	qreal scale();

        int page(QPoint point);
        QPoint toPage(QPoint point);
        QPoint toCurrentPage(QPoint point);
        QRect pageRect(QPoint point);

        QPixmap *grabPage(int page, const QRectF &rect);
        QPixmap *grabPage(int page, qreal left, qreal top, qreal width, qreal height);

        //Движение мышки над страницами
        void setMouseTrackingPage(bool mtp);
        bool isMouseTrackingPage();

        //Информация о странице над которой сейчас находится мышка
        QPageInfo enterPageInfo();
        QPageInfo pageInfo(int page);
        QPageInfo pageInfo(const QPoint &point);

        QVector <QPageInfo>visiblePages();

        //Вход и выход мышки в\из области страницы
        void setMouseEnterLeavePage(bool melp);
        bool mouseEnterLeavePage();

        void setMovedContents(bool move);
        bool movedContents();

        void startMoveContext(const QPoint &point);
        void moveContext(const QPoint &point);
        void endMoveContext();
public slots:
	void setScale(double scale);
	void pageSetup();
	void setupPageFormat();
	void print();
	void print(QPrinter *printer);

	void scaleIn();
	void scaleOut();
	void scaleOrig();
	void gotoPage(int index);

private:
	AbstractPreviewPrivate *d;
	AbstractPreviewPrivate *dv, *dp;
	void paintBorder(QPainter *p);
	QPixmap* getTile(int page, int tx, int ty);
#ifdef PRINTER_CHANGE_DEVICE
	void changeDevice(QPaintDevice*);
#endif
	void autoscaleThumbs();

protected slots:

protected:
	int sizeToCountPage();

	virtual void printPages(QPrinter *printer);
	virtual void resizeEvent(QResizeEvent *event);
	virtual void paintEvent(QPaintEvent *event);
        virtual void additionalPaintEvent(QPainter *p, int page, const QRect &rect);
	virtual void scrollContentsBy(int dx, int dy);

	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void wheelEvent(QWheelEvent * event);

	virtual void scaleEvent(const double &scaleOld, const double &scaleNew);
	virtual void addedPages(int count, int begin);
	virtual void paintPage(QPainter *p, int numberPage, const QRect&) = 0;
	virtual void updatePageFormat() = 0;
#ifdef PRINTER_CHANGE_DEVICE
	virtual void deviceChanged(QPaintDevice*) = 0;
#endif
	virtual void clearEvent() = 0;

	QSize getPaperSize();
	QSize getPageSize();

        virtual void mousePressPageEvent(QMouseEvent *event, int page, QRect rect, QPoint point);
        virtual void mouseMovePageEvent(QMouseEvent *event, const QPageInfo &info);
};
#endif // QPREVIEW_H

