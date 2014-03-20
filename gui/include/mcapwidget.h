/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <QWidget>

class MediaCapWidget : public QWidget
{
	Q_OBJECT
public:
//	MediaCapWidget(QWidget *p, Qt::WindowFlags f) : QWidget(p,f) {};
	MediaCapWidget(QString itext, bool tri, quint64 df, QWidget *p, Qt::WindowFlags f=0);
	~MediaCapWidget();
	void setText(QString itext);
	inline quint64 flag() { return cf; };
	void setR(quint64 r);
	void setW(quint64 r);
	void setCap(quint64 s);
	void clear();
//	inline void setTristate(bool s) { tristate=s; rd=0; wr=0; };

	virtual QSize sizeHint() const;
protected:
	void paintEvent(QPaintEvent*);
private:
	void setRW();
	quint64 cf;
	QImage  icon;
	QString text;
	bool rd, wr, tristate;
};

