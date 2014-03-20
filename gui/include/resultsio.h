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

#ifndef _RESULTSIO_H
#define _RESULTSIO_H

#include <QThread>

#include <device.h>

#define RECALC_ON_LOAD 1

class ResultsIO : public QThread
{
public:
	ResultsIO(device *dev) 	:QThread((QObject*)dev) { this->dev = dev; this->res = false; io = NULL; }
	~ResultsIO() {};

	inline void setIO(QIODevice *io) { this->io = io; };
	inline bool result() { return res; };

protected:
	QIODevice *io;
	device    *dev;
	bool	  res;
};


class ResultsReader : public ResultsIO
{
	Q_OBJECT
public:
	ResultsReader(device *dev) : ResultsIO(dev) {};
	~ResultsReader() {};

protected:
	virtual void run();
};

class ResultsWriter : public ResultsIO
{
	Q_OBJECT
public:
	ResultsWriter(device *dev) : ResultsIO(dev) {};
	~ResultsWriter() {};

protected:
	virtual void run();
};

#endif // _RESULTSIO_H

