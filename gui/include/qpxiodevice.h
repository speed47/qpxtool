/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef QPX_IODEVICE_H
#define QPX_IODEVICE_H

#include <QIODevice>
#include <QString>
#include <QStringList>

class QPxIODevice : public QObject
{
	Q_OBJECT
public:
	QPxIODevice(QObject*);
	~QPxIODevice();
	void setIODevice(QIODevice*);
	QIODevice* IODevice();
	QString readLine();
	int linesAvailable();

signals:
	void readyReadLine();

protected slots:
	void splitInput();

private:
	QIODevice   *io;
	QString     buf;
	QStringList lines; 
};

#endif

