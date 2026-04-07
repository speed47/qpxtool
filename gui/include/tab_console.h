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

#ifndef _TAB_CONSOLE_H
#define _TAB_CONSOLE_H

class QPlainTextEdit;
class QPushButton;
class QBoxLayout;

class device;
class devlist;
class QPxSettings;

#include <QPointer>
#include <QWidget>

class tabConsole : public QWidget {
	Q_OBJECT
public:
	tabConsole(QPxSettings* iset, devlist* idev, QWidget* p = 0, Qt::WindowFlags fl = Qt::WindowFlags());
	~tabConsole();

public slots:
	void selectDevice();
	void reconfig();
	void appendLine(const QString& line);
	void appendSeparator(const QString& cmdline);
	void clear();
	void saveToFile();

private:
	devlist* devices;
	QPxSettings* settings;

	QBoxLayout* layout;
	QPlainTextEdit* textEdit;
	QPushButton* saveButton;
	QPushButton* clearButton;

	QPointer<device> connectedDev;
	void connectToDevice(device* dev);
	void disconnectFromDevice();
};

#endif
