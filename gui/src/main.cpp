/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2008-2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <QApplication>
#include <QMainWindow>
#include <QSplashScreen>
#include <QLocale>
#include <QTranslator>

#include <mainwindow.h>
#include <QDebug>

#include <QIcon>

#include "../config.h"

int main(int ac, char** av)
{
    int r;
    QApplication *QPxTool;
    QPxToolMW	*mainwin;
    QTranslator *translator;
	QSplashScreen *splash;
	QString locale = QLocale::system().name();
	QPixmap *pix;
    QPxTool = new QApplication(ac,av);
#ifndef QT_NO_DEBUG
	qDebug("Creating splash screen...");
#endif
	pix = new QPixmap (":images/splash.png");
	splash = new QSplashScreen( *pix );
	splash->show();

	translator = new QTranslator;

#ifndef QT_NO_DEBUG
	qDebug("* Loading translator...");
#endif
#ifdef _WIN32
	if (!translator->load("qpxtool."+locale, "locale")) {
#else
	if (!translator->load("qpxtool."+locale, INSTALL_PREFIX"/share/qpxtool/locale")) {
#endif
		qDebug() << "** Can't load translation for current locale: " << locale;
	} else {
		QPxTool->installTranslator(translator);
	}

	QPxTool->setWindowIcon( QIcon(":images/q.png") );

    mainwin = new QPxToolMW(ac,av);

	splash->finish(mainwin);
    mainwin->show();
    r = QPxTool->exec();

    delete mainwin;
	delete translator;
	delete splash;
	delete pix;
	delete QPxTool;

    return r;
}

