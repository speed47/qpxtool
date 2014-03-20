/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef DB_REPORT_SELECTION
#define DB_REPORT_SELECTION

#include <QDialog>
#include <QSqlDatabase>
#include <QAbstractListModel>
#include <QModelIndex>
#include <QIcon>

class QLabel;
class QLineEdit;
class QListView;
class QSplitter;
class QPushButton;
class QToolButton;
class QBoxLayout;
class QEvent;
class QTextBrowser;

class ListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	ListModel(QObject *p) : QAbstractListModel(p) {};
	~ListModel() {};

	void clear();
	virtual int rowCount(const QModelIndex&) const;
	virtual int columnCount(const QModelIndex&) const;
	virtual QVariant data(const QModelIndex&, int role) const;
	void addRow(int, QString, QIcon icon = QIcon());
	bool setData(int, int role, QVariant);
	void removeRow(int);
	int  getKey(int);

private:
	QList<QString>		tdata;
	QList<QIcon>		idata;
	QList<int>		keys;
};



class DbReportSelection : public QDialog
{
	Q_OBJECT

public:
	DbReportSelection(QString conn, QWidget *parent = NULL, Qt::WindowFlags fl = 0);
	~DbReportSelection();
	int getReportID() const { return report_id; };

private slots:
	void load();
	void search();
	void itemActivated(const QModelIndex);

protected:
	bool eventFilter(QObject *o, QEvent *e);
	static QImage getMediaLogo(quint64);

private:
	void winit();

	QBoxLayout	*layout,
				*layout_l,
				*layout_pb;
	QTextBrowser *info;

	QLabel		*llabel;
	QLineEdit	*elabel;
	QToolButton *elabel_clear,
				*elabel_search;
	QListView	*list;
	QPushButton	*pb_load,
				*pb_cancel;

	ListModel	*model;
	QSqlDatabase db;
	int			report_id;
};

#endif // DB_REPORT_SELECTION

