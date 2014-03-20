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

#include "qpxiodevice.h"

#include <QProcess>
#include <QTcpSocket>

#include <QDebug>

#include <typeinfo>

#ifndef QT_NO_DEBUG
//#define QPXIO_DEBUG
#endif
	
QPxIODevice::QPxIODevice(QObject* p)
	:QObject(p)
{
	buf = "";
	io = NULL;
}

QPxIODevice::~QPxIODevice() {}

void QPxIODevice::setIODevice(QIODevice* iio)
{
#ifdef QPXIO_DEBUG
	qDebug("STA: QPxIODevice::setIODevice()");
#endif
	buf = "";
	io = iio;
	if (io) {
		if ( typeid(*io) == typeid(QProcess) )
			connect(io, SIGNAL(readyReadStandardOutput()), this, SLOT(splitInput()));
		else if ( typeid(*io) == typeid(QTcpSocket) )
			connect(io, SIGNAL(readyRead()), this, SLOT(splitInput()));
	}
#ifdef QPXIO_DEBUG
	qDebug("END: QPxIODevice::setIODevice()");
#endif
}

QIODevice* QPxIODevice::IODevice() { return io; }

QString QPxIODevice::readLine()
{
	QString ts="";
	if (lines.size()) {
		ts = lines[0];
		lines.removeFirst();
	}
	return ts; 
}

int QPxIODevice::linesAvailable() { return lines.size(); }

void QPxIODevice::splitInput()
{
#ifdef QPXIO_DEBUG
	qDebug("STA: QPxIODevice::splitInput()");
#endif

	int    bi;
	qint64 ba;
	while ((ba = io->bytesAvailable())) {
	//while ((io->bytesAvailable())) {
#ifdef QPXIO_DEBUG
		qDebug() << "IODevice bytes: " << ba;
#endif

/*
#ifdef QPXIO_DEBUG
		if ( typeid(*io) == typeid(QTcpSocket) ) {
//			QByteArray tb;
			while ((io->bytesAvailable())) {
				buf+=io->read(1).replace(0,'\n');
			}
//			buf+=QString(tb);
//			qDebug() << "ba.size(): " << tb.size();
//			qDebug() << "buf: " << buf;
		} else {
			buf+=io->readAll();
		}
#else
*/
		buf+=io->readAll().replace(0,'\n');
/*
#endif
*/
		while ((bi = buf.indexOf(QRegExp("[\n\r]"),0)) >=0 ) {
			if (bi>0) {
				lines << buf.left(bi);
				buf.remove(0,bi);
			}
			buf.remove(0,1);
		}
//		if (lines.size() > 4) emit readyReadLine();
	}
	if (lines.size()) emit readyReadLine();
#ifdef QPXIO_DEBUG
	qDebug("END: QPxIODevice::splitInput()");
#endif
}

