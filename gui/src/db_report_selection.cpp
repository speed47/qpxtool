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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTextBrowser>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QToolButton>
#include <QDateTime>

#include "db_report_selection.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QEvent>
#include <QDebug>

#include <qpx_mmc_defs.h>

int ListModel::rowCount(const QModelIndex& p) const
{
//	qDebug() << "ListModel::rowCount() "<< tdata.size();
	if (!p.isValid()) return tdata.size();
	return 0;
}

int ListModel::columnCount(const QModelIndex& p) const
{
//	qDebug() << "ListModel::columnCount() "<< cols;
	return 1;
}

QVariant ListModel::data(const QModelIndex& p, int role) const
{
	int r = p.row();
	if (p.column()) return QVariant();
//	qDebug() << "ListModel::data("<< r << ","<< c << ")" << role;
	if (r<0 || r >= tdata.size() ) return QVariant();
	switch (role) {
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
		case Qt::EditRole:
			return QVariant(tdata[r]);
		case Qt::DecorationRole:
			return QVariant(idata[r]);
//		case Qt::FontRole:
//		case Qt::TextAlignMentRole:
//		case Qt::CheckStateRole:
		default:
			break;
	}
	return QVariant();
}

void ListModel::clear() {
	tdata.clear();
	idata.clear();
	keys.clear();
}

void ListModel::addRow(int key, QString text, QIcon icon)
{
	tdata << text;
	idata << icon;
	keys  << key;
}

bool ListModel::setData(int row, int role, QVariant d)
{
	if (row<0 || row>=tdata.size()) return false;

	switch (role) {
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
		case Qt::EditRole:
			tdata[row] = d.toString();
			return true;
		case Qt::DecorationRole:
			idata[row] = d.value<QIcon>();
			return true;
		default:
			break;
	}
	return false;
}


void ListModel::removeRow(int row)
{
	if (row<0 || row>=tdata.size()) return;
	tdata.removeAt(row);
	idata.removeAt(row);
	keys.removeAt(row);
}

int ListModel::getKey(int row)
{
	if (keys.size() <= row) return -1;
	return keys[row];
}

//*******************
// dialog
//*******************


DbReportSelection::DbReportSelection(QString conn, QWidget *parent, Qt::WindowFlags fl)
	:QDialog(parent, fl)
{
	report_id = -1;
	model = new ListModel(this);
	db = QSqlDatabase::database(conn);
	winit();
}

DbReportSelection::~DbReportSelection()
{

}

void DbReportSelection::winit()
{
	setWindowTitle(tr("Select report..."));
	setMinimumWidth(400);

	layout = new QVBoxLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);

	layout_l = new QHBoxLayout();
	layout_l->setMargin(0);
	layout_l->setSpacing(3);
	layout->addLayout(layout_l);
// search filter
	llabel = new QLabel(tr("Label:"),this);
	layout_l->addWidget(llabel);

	elabel = new QLineEdit(this);
	elabel->setMaxLength(128);
	elabel->setMinimumHeight(22);
	elabel_clear = new QToolButton(elabel);
	elabel_clear->setCursor(Qt::ArrowCursor);
	elabel_clear->setIcon(QIcon(":images/edit-clear.png"));
	elabel_clear->setStyleSheet("QToolButton { border: none; padding: 0px; } "
                              "QToolButton{"
                              "border:none;"
                              "}"
                              "QToolButton:pressed{"
                              "background-color:none;"
							"}");

	elabel_search = new QToolButton(this);
//	elabel_search->setCursor(Qt::ArrowCursor);
	elabel_search->setIcon(QIcon(":images/search.png"));
/*
	elabel_search->setStyleSheet("QToolButton { border: none; padding: 0px; } "
                              "QToolButton{"
                              "border:none;"
                              "}"
                              "QToolButton:pressed{"
                              "background-color:none;"
							"}");
*/
	elabel->installEventFilter(this);
	layout_l->addWidget(elabel);
	layout_l->addWidget(elabel_search);

// search results
	list = new QListView(this);
	layout->addWidget(list);

	info = new QTextBrowser(this);
	info->setMaximumHeight(150);
	layout->addWidget(info);

// buttons
	layout_pb = new QHBoxLayout();
	layout_pb->setMargin(0);
	layout_pb->setSpacing(3);
	layout->addLayout(layout_pb);

	layout_pb->addStretch(10);
	pb_load = new QPushButton(QIcon(":images/ok.png"), tr("Load"), this);
	pb_load->setEnabled(false);
	layout_pb->addWidget(pb_load);
	pb_cancel = new QPushButton(QIcon(":images/x.png"), tr("Cancel"), this);
	layout_pb->addWidget(pb_cancel);


	connect(elabel_clear,  SIGNAL(clicked()), elabel, SLOT(clear()));
	connect(elabel_search, SIGNAL(clicked()), this,   SLOT(search()));

	connect(list, SIGNAL(clicked(const QModelIndex)), this, SLOT(itemActivated(const QModelIndex)));

	connect(pb_load,   SIGNAL(clicked()), this, SLOT(load()));
	connect(pb_cancel, SIGNAL(clicked()), this, SLOT(reject()));

	search();
}

void DbReportSelection::search()
{
	QSqlQuery *q;
	QIcon icon;

	pb_load->setEnabled(false);
	list->setModel(NULL);
	model->clear();

	q = new QSqlQuery(QSqlDatabase::database("reports"));
//	qDebug() << "Label pattern:" << elabel->text();
	q->prepare("SELECT \
				reports.id,id_media_type,label,copy_idx,datetime \
			FROM \
				reports \
			WHERE \
				label ILIKE '%"+elabel->text()+"%' \
			ORDER BY datetime");
	if (!q->exec()) {
		qDebug() << q->lastError().text();
		goto err;
	}
	while (q->next()) {
		icon = QPixmap::fromImage(getMediaLogo(q->value(1).toULongLong()));
		if (!q->value(3).toInt())
			model->addRow( q->value(0).toInt(), q->value(2).toString(), icon);
		else
			model->addRow( q->value(0).toInt(), q->value(2).toString() + ", "+ tr("copy") + " #" + q->value(3).toString(), icon);
	}
	list->setModel(model);

err:
	delete q;
}

void DbReportSelection::load()
{
	accept();
}

void DbReportSelection::itemActivated(const QModelIndex idx)
{
	QSqlQuery *q;
	QString html;
	QString md5;
	report_id = model->getKey(idx.row());
	pb_load->setEnabled(true);

	q = new QSqlQuery(QSqlDatabase::database("reports"));
	q->prepare("SELECT \
				label,id_media_type,media_types.name, \
				dev_vendors.name,dev_models.name,dev_fw,datetime, \
				copy_idx,md5,metadata \
			FROM \
				reports,media_types,dev_vendors,dev_models \
			WHERE \
				media_types.id=id_media_type AND \
				dev_models.id=dev_id AND \
				dev_vendors.id=id_vendor AND \
				reports.id=:id");
	q->bindValue(":id",report_id);

	if (!q->exec() || !q->next()) {
		qDebug() << q->lastError().text();
		goto err;
	}

	info->document()->addResource( QTextDocument::ImageResource,
		QUrl("images//:media_logo.png"),
		getMediaLogo(q->value(1).toULongLong()).scaled( QSize(64,64), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
	md5 = q->value(8).toString().simplified();

	html = "<HTML><BODY><table border=0 cellspacing=0 cellpadding=2>";
	html+= "<tr><td><img src=\"images//:media_logo.png\"><br></td>";
	html+= "<td><b>"+q->value(0).toString()+"</b><br>";
	html+= q->value(2).toString()+", "+tr("copy")+"# "+q->value(7).toString()+"<br>";
	if (!md5.isEmpty()) {
		html+= "MD5: " + md5;
	}
	html+= "</td></tr>";

	html+= "<tr><td bgcolor=#E0E0E0>"+tr("Device")+"</td><td>"
		+q->value(3).toString()
		+q->value(4).toString()
		+q->value(5).toString()
		+"</td></tr>";
	html+= "<tr><td bgcolor=#E0E0E0>"+tr("Date")+"</td><td>"
		+q->value(6).toDateTime().toString("yyyy.MM.dd  hh:mm")
		+"</td></tr>";
	html+=q->value(9).toString();
	html+= "</table></BODY></HTML>";

	info->setHtml(html);
err:
	delete q;
}

bool DbReportSelection::eventFilter(QObject *o, QEvent *e)
{
	if (o == elabel) {
		if (e->type() == QEvent::Resize) {
			int frameWidth = elabel->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
			elabel_clear->resize(elabel->height() - (frameWidth<<1), elabel->height() - (frameWidth<<1));
			elabel_clear->move(elabel->width() - elabel_clear->width() - frameWidth, frameWidth);
/*
			elabel_search->resize(elabel->height() - (frameWidth<<1), elabel->height() - (frameWidth<<1));
			elabel_search->move(elabel->width() - elabel_clear->width() - frameWidth, frameWidth);
			elabel_clear->resize(elabel->height() - (frameWidth<<1), elabel->height() - (frameWidth<<1));
			elabel_clear->move(elabel->width() - elabel_clear->width() - elabel_search->width() - frameWidth - 10, frameWidth);
			elabel->setStyleSheet(QString("QLineEdit { padding-right: %1px; }").arg(elabel_clear->width()+elabel_search->width()+10));
*/
			elabel->setStyleSheet(QString("QLineEdit { padding-right: %1px; }").arg(elabel_clear->width()+1));
			return true;
		} else {
			return false;
		}
	} else {
		return QDialog::eventFilter(o,e);
	}
}

QImage DbReportSelection::getMediaLogo(quint64 id)
{
	QImage logo;
	switch (id) {
// CD media
		case DISC_CDROM:
			logo = QImage(":images/media/cd.png");
			break;
		case DISC_CDR:
			logo = QImage(":images/media/cd_r.png");
			break;
		case DISC_CDRW:
			logo = QImage(":images/media/cd_rw.png");
			break;
// DVD media
		case DISC_DVDROM:
			logo = QImage(":images/media/dvd-rom.png");
			break;
		case DISC_DVDRAM:
			logo = QImage(":images/media/dvd-ram.png");
			break;
// DVD- media
		case DISC_DVDmR:
			logo = QImage(":images/media/dvd-r.png");
			break;
		case DISC_DVDmRW:
		case DISC_DVDmRWS:
		case DISC_DVDmRWR:
			logo = QImage(":images/media/dvd-rw.png");
			break;
		case DISC_DVDmRDL:
		case DISC_DVDmRDLJ:
			logo = QImage(":images/media/dvd-r_dl.png");
			break;
		case DISC_DVDmRWDL:
			logo = QImage(":images/media/dvd-rw_dl.png");
			break;

// DVD+ media
		case DISC_DVDpR:
			logo = QImage(":images/media/dvd+r.png");
			break;
		case DISC_DVDpRDL:
			logo = QImage(":images/media/dvd+r_dl.png");
			break;
		case DISC_DVDpRW:
			logo = QImage(":images/media/dvd+rw.png");
			break;
		case DISC_DVDpRWDL:
			logo = QImage(":images/media/dvd+rw_dl.png");
			break;


// DD-CD media
		case DISC_DDCD_ROM:
		case DISC_DDCD_R:
		case DISC_DDCD_RW:
			logo = QImage(":images/media/ddcd.png");
			break;

// Blu-Ray media
		case DISC_BD_ROM:
		case DISC_BD_R_SEQ:
		case DISC_BD_R_RND:
		case DISC_BD_RE:
			logo = QImage(":images/media/bd.png");
			break;

// HD DVD media
		case DISC_HDDVD_ROM:
			logo = QImage(":images/media/hddvd-rom.png");
			break;
		case DISC_HDDVD_R:
		case DISC_HDDVD_RDL:
			logo = QImage(":images/media/hddvd-r.png");
			break;
		case DISC_HDDVD_RAM:
			logo = QImage(":images/media/hddvd-ram.png");
			break;
		case DISC_HDDVD_RW:
		case DISC_HDDVD_RWDL:
			logo = QImage(":images/media/hddvd-rw.png");
			break;
		default:
			logo = QImage();
	}
	return logo;
}

