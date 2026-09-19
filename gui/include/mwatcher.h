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

#ifndef MWATCHER_H
#define MWATCHER_H

class device;
class drive_info;

#include <QThread>
#include <atomic>

class MediaWatcher : public QThread {
	Q_OBJECT

public:
	MediaWatcher(device* qdev);
	~MediaWatcher();

	void stop();
	void pause();
	void unPause();

signals:
	void mediaLoading();
	void mediaRemoved();
	void mediaNew();
	void mediaNoMedia();

protected:
	virtual void run();

private:
	std::atomic<bool> preq{false}, sreq{false};
	drive_info* dev;
};

#endif
