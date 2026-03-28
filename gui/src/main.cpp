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
#include <QDir>
#include <QPalette>
#include <QColor>
#include <QStyleFactory>

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
#if defined (_WIN32) || defined (_WIN64)
    QDir::setCurrent(QCoreApplication::applicationDirPath());
#endif
    QPxTool->setStyle("Fusion");
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, Qt::black);
    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::AlternateBase, QColor(233, 233, 233));
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
    lightPalette.setColor(QPalette::ToolTipText, Qt::black);
    lightPalette.setColor(QPalette::Text, Qt::black);
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, Qt::black);
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, QColor(0, 0, 255));
    lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);
    // Disabled state colors so disabled widgets (checkboxes, etc.) are visibly greyed out
    lightPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(160, 160, 160));
    lightPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(160, 160, 160));
    lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(160, 160, 160));
    lightPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::Disabled, QPalette::Button, QColor(225, 225, 225));
    lightPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(190, 190, 190));
    lightPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, Qt::white);
    QPxTool->setPalette(lightPalette);
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
#if defined (_WIN32) || defined (_WIN64)
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

