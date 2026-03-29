/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>

#include <device.h>
#include <qpxsettings.h>

#include "tab_console.h"

#include <QDebug>

tabConsole::tabConsole(QPxSettings* iset, devlist* idev, QWidget* p, Qt::WindowFlags fl) : QWidget(p, fl) {
#ifndef QT_NO_DEBUG
	qDebug("STA: tabConsole()");
#endif
	settings = iset;
	devices = idev;
	connectedDev = NULL;

	layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(3);

	QLabel* infoLabel = new QLabel(tr("Console options can be configured in File > Preferences > Common."), this);
	layout->addWidget(infoLabel);

	textEdit = new QPlainTextEdit(this);
	textEdit->setReadOnly(true);
	textEdit->setMaximumBlockCount(settings->console_max_lines);
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPointSize(9);
	textEdit->setFont(font);
	layout->addWidget(textEdit);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	saveButton = new QPushButton(tr("Save to file"), this);
	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveToFile()));
	buttonLayout->addWidget(saveButton);
	clearButton = new QPushButton(tr("Clear"), this);
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
	buttonLayout->addWidget(clearButton);
	layout->addLayout(buttonLayout);

#ifndef QT_NO_DEBUG
	qDebug("END: tabConsole()");
#endif
}

tabConsole::~tabConsole() {
#ifndef QT_NO_DEBUG
	qDebug("STA: ~tabConsole()");
	qDebug("END: ~tabConsole()");
#endif
}

void tabConsole::selectDevice() {
	device* dev = devices->current();
	if (dev == connectedDev) return;
	disconnectFromDevice();
	connectToDevice(dev);
}

void tabConsole::reconfig() { textEdit->setMaximumBlockCount(settings->console_max_lines); }

void tabConsole::appendLine(const QString& line) { textEdit->appendPlainText(line); }

void tabConsole::appendSeparator(const QString& cmdline) {
	textEdit->appendPlainText(QString("════════════════════════════════════════════════════════════════"));
	textEdit->appendPlainText(QString("$ %1").arg(cmdline));
	textEdit->appendPlainText(QString("════════════════════════════════════════════════════════════════"));
}

void tabConsole::clear() { textEdit->clear(); }

void tabConsole::saveToFile() {
	QString path = QFileDialog::getSaveFileName(this, tr("Save console output"), QString(),
	                                            tr("Text files (*.txt);;All files (*)"));
	if (path.isEmpty()) return;
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Error"), tr("Unable to open file for writing:\n%1").arg(path));
		return;
	}
	file.write(textEdit->toPlainText().toUtf8());
	file.close();
}

void tabConsole::connectToDevice(device* dev) {
	if (!dev || dev->type == device::DevtypeNone || dev->type == device::DevtypeVirtual) return;
	connectedDev = dev;
	connect(dev, SIGNAL(outputLine(const QString&)), this, SLOT(appendLine(const QString&)));
	connect(dev, SIGNAL(processCommand(const QString&)), this, SLOT(appendSeparator(const QString&)));
}

void tabConsole::disconnectFromDevice() {
	if (!connectedDev) return;
	disconnect(connectedDev, SIGNAL(outputLine(const QString&)), this, SLOT(appendLine(const QString&)));
	disconnect(connectedDev, SIGNAL(processCommand(const QString&)), this, SLOT(appendSeparator(const QString&)));
	connectedDev = NULL;
}
