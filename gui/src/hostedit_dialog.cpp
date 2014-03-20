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


#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include "hostedit_dialog.h"

hostEditDialog::hostEditDialog(QString host, int port, QWidget* p, Qt::WindowFlags f)
	: QDialog(p,f)
{
	setWindowTitle(tr("Add host"));

	layout = new QGridLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);

	l_host = new QLabel(tr("Host:"), this);
	layout->addWidget(l_host, 0, 0);

	e_host = new QLineEdit(this);
	e_host->setText(host);
//	e_host->setInputMask("000.000.000.000; ");
	layout->addWidget(e_host, 0, 1, 1, 2);

	l_port = new QLabel(tr("Port:"), this);
	layout->addWidget(l_port, 1, 0);

	e_port = new QSpinBox(this);
	e_port->setMinimum(1);
	e_port->setMaximum(65535);
	e_port->setValue(port);
	layout->addWidget(e_port, 1, 1);

	bdef = new QPushButton(this);
	bdef->setMaximumSize(22,22);
	bdef->setIcon(QIcon(":images/edit-undo.png"));
	layout->addWidget(bdef, 1, 2);

	bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal , this);
	layout->addWidget(bbox, 2, 1, 1, 2);

	layout->setColumnStretch(0,2);
	layout->setColumnStretch(1,20);
	layout->setColumnStretch(2,1);

	connect(e_host, SIGNAL(textChanged(QString&)), this, SLOT(hostChanged(QString&)));
	connect(bdef, SIGNAL(clicked()), this,  SLOT(setPortDfl()));
	connect(bbox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(bbox, SIGNAL(rejected()), this, SLOT(reject()));
}

hostEditDialog::~hostEditDialog()
{

}

void hostEditDialog::setPortDfl()
{
	e_port->setValue(46660);
}

void hostEditDialog::hostChanged(QString& h)
{
	QPushButton *pb = bbox->button(QDialogButtonBox::Ok);
	if (!pb) return;
	pb->setEnabled(!h.isEmpty());
}

