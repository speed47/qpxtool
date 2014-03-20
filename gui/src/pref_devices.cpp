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

#include <QMenu>
#include <QEvent>
#include <QContextMenuEvent>
#include <QMessageBox>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <hostedit_dialog.h>
#include <qpxsettings.h>
#include "pref_devices.h"
#include <QDebug>

prefDevices::prefDevices(QPxSettings *iset, QWidget *p, Qt::WindowFlags f)
	: QWidget(p,f)
{
#ifndef QT_NO_DEBUG
	qDebug("STA: prefDevices()");
#endif
	set = iset;

	layout = new QVBoxLayout(this);
	layout->setMargin(0);

	ck_local = new QCheckBox(tr("Use local devices"),this);
	ck_local->setChecked(set->useLocal);
	layout->addWidget(ck_local);

	ck_remote = new QCheckBox(tr("Use network devices"),this);
	ck_remote->setChecked(set->useRemote);
	layout->addWidget(ck_remote);

//	g_hosts = new QGroupBox(tr("Hosts"), this);
	//g_hosts->setCheckable(true);
//	layout->addWidget(g_hosts);

//	l_hosts = new QVBoxLayout(g_hosts);

	//lst_hosts = new QTreeWidget(g_hosts);
	lst_hosts = new QTreeWidget(this);
	lst_hosts->setRootIsDecorated(false);
	lst_hosts->setColumnCount(2);
	lst_hosts->setColumnWidth(0, 130);
	lst_hosts->setHeaderLabels(QStringList() << tr("host") << tr("qscand port"));
	layout->addWidget(lst_hosts);

	QTreeWidgetItem *hitem;
	for (int i=0; i<set->hosts.size(); i++) {
		//hitem = new QTreeWidgetItem(lst_hosts, QStringList(set->hosts[i]) );
		hitem = new QTreeWidgetItem(lst_hosts);
		hitem->setFlags( hitem->flags() | Qt::ItemIsUserCheckable);
		hitem->setCheckState(0, (set->hosts[i][0] == '*') ? Qt::Checked : Qt::Unchecked);

		hitem->setText(0, set->hosts[i].remove(0,1) );
		if (i<set->ports.size())
			hitem->setText(1, set->ports[i] );
		else
			hitem->setText(1, "46660" );
		lst_hosts->addTopLevelItem(hitem);
	}
//	lst_hosts->sortItems(0,Qt::AscendingOrder);
	lst_hosts->installEventFilter(this);
#ifndef QT_NO_DEBUG
	qDebug("END: prefDevices()");
#endif
}

prefDevices::~prefDevices()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~prefDevices()");
#endif
	int hcnt = lst_hosts->topLevelItemCount();
	QTreeWidgetItem *hitem;

	set->useLocal  = ck_local->isChecked();
	set->useRemote = ck_remote->isChecked();
	set->hosts.clear();
	set->ports.clear();

	for (int i=0; i<hcnt; i++) {
		hitem = lst_hosts->topLevelItem(i);

		set->hosts.append( ((hitem->checkState(0) == Qt::Checked) ? "*" : "-") + hitem->text(0) );
		set->ports.append( hitem->text(1) );
	}

#ifndef QT_NO_DEBUG
	qDebug("END: ~prefDevices()");
#endif
}

bool prefDevices::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == lst_hosts) {
#ifndef QT_NO_DEBUG
//		qDebug(QString("lst_hosts::event: %1").arg(e->type()));
#endif
		if (e->type() == QEvent::ContextMenu) {
			hostsContextMenu( (QContextMenuEvent*)e );
			return true;
//		} else if (e->type() == QEvent::MouseButtonDblClick) {
//			hostEdit();
//			return true;
		} else if (e->type() == QEvent::KeyPress) {
			switch ( ((QKeyEvent*)e)->key()) {
			//	case Qt::Key_Enter:
				case Qt::Key_Return:
					hostEdit();
					return true;
				case Qt::Key_Insert:
					hostAdd();
					return true;
				case Qt::Key_Delete:
					hostRemove();
					return true;
				default:
					return false;
			}
		} else {
			return false;
		}
	} else {
		QWidget::eventFilter(obj,e);
	}
	return false;
}

void prefDevices::hostsContextMenu(QContextMenuEvent* e)
{
	QMenu *cmenu;
	QAction *act;
#ifndef QT_NO_DEBUG
	qDebug("hostsContextMenu()");
#endif
	cmenu = new QMenu(this);
	act = cmenu->addAction(QIcon(":images/edit.png"),tr("Edit host"),   this, SLOT(hostEdit()));
	if (!lst_hosts->currentItem()) act->setEnabled(false);
	act = cmenu->addAction(QIcon(":images/add.png"), tr("Add host"),    this, SLOT(hostAdd()));
	act = cmenu->addAction(QIcon(":images/x.png"),   tr("Remove host"), this, SLOT(hostRemove()));
	if (!lst_hosts->currentItem()) act->setEnabled(false);
	cmenu->exec(e->globalPos());
	delete cmenu;
}

void prefDevices::hostAdd()
{
	QTreeWidgetItem *hitem;
#ifndef QT_NO_DEBUG
	qDebug("hostAdd()");
#endif
	hostEditDialog	*hadd = new hostEditDialog("", 46660, this);
	if (hadd->exec() && !hadd->hostname().isEmpty()) {
		hitem = new QTreeWidgetItem(lst_hosts);
		hitem->setFlags(hitem->flags() | Qt::ItemIsUserCheckable);
		hitem->setCheckState(0, Qt::Checked);

		hitem->setText(0, hadd->hostname() );
		hitem->setText(1, QString::number(hadd->port()) );
		lst_hosts->addTopLevelItem(hitem);
	}
	delete hadd;
}

void prefDevices::hostEdit()
{
	QTreeWidgetItem *hitem = lst_hosts->currentItem();
#ifndef QT_NO_DEBUG
	qDebug("hostEdit()");
#endif
	if (!hitem) return;
	hostEditDialog	*hedit = new hostEditDialog(hitem->text(0), hitem->text(1).toInt(), this);
	hedit->setWindowTitle(tr("Edit host"));

	if (hedit->exec() && !hedit->hostname().isEmpty()) {
		hitem->setText(0, hedit->hostname() );
		hitem->setText(1, QString::number(hedit->port()) );
	}	
	delete hedit;
}

void prefDevices::hostRemove()
{
	QTreeWidgetItem *hitem = lst_hosts->currentItem();
#ifndef QT_NO_DEBUG
	qDebug("hostRemove()");
#endif
	if (!hitem) return;
	if (QMessageBox::warning(this,
				tr("Remove host?"),
				tr("You are about to remove host from list:\n%1:%2\nAre you sure?").arg(hitem->text(0)).arg(hitem->text(1)),
				QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) == QMessageBox::Ok)
	delete lst_hosts->takeTopLevelItem( lst_hosts->indexOfTopLevelItem(hitem));
}

