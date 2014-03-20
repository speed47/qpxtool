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

#include "mwatcher.h"
#include <QDebug>

#include <qpx_mmc.h>
#include <device.h>


MediaWatcher::MediaWatcher(device *qdev)
{
#ifndef QT_NO_DEBUG
	qDebug("MediaWatcher()");
#endif
	if (qdev->type == device::DevtypeLocal) {
		QByteArray ba;
		ba+=qdev->path;
		dev = new drive_info(ba.data());
		if (inquiry(dev)) {
			delete dev;
			dev = NULL;
		}
	} else {
		dev = NULL;
	}
}

MediaWatcher::~MediaWatcher()
{
#ifndef QT_NO_DEBUG
	qDebug("~MediaWatcher()");
#endif
	if (dev) delete dev;
}

void MediaWatcher::stop()  { sreq=1; }
void MediaWatcher::pause() { preq=1; }
void MediaWatcher::unPause() { preq=0; }

void MediaWatcher::run()
{
	if (!dev) {
		qDebug("Can't start watcher: NULL device!");
		return;
	}

	int op,cp;
#ifndef QT_NO_DEBUG
	qDebug() << dev->device << ": watcher started";
#endif
	sreq = 0; preq = 0;
// initialising surrent status
	op = cp = test_unit_ready(dev);
	while (!sreq) {
		if (!preq) {
			cp = test_unit_ready(dev);

			if (cp!=op) {
				switch (cp) {
					case -1:
						cp = op;
						break;
					case 0x00000: // media found
						qDebug() << dev->device << ": new media found";
						emit mediaNew();
						break;
					case 0x23A01: // no media, tray closed
						qDebug() << dev->device << ": tray closed, but no media found";
						emit mediaNoMedia();
						break;
					case 0x23A02: // no media, tray open
						qDebug() << dev->device << ": media removed";
						emit mediaRemoved();
						break;
					case 0x20401: // media recognition
						emit mediaLoading();
						qDebug() << dev->device << ": loading media";
						break;
					default:
						print_sense(cp); printf("\n");
						break;
				}
				op = cp;
			}
		}
		sleep(1);
	}
#ifndef QT_NO_DEBUG
	qDebug() << dev->device << ": watcher stoped";
#endif
}

