/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2009-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <QVBoxLayout>
#include <QTextEdit>
#include <QTabWidget>
#include <QFile>
#include <QLabel>

#include "about.h"
#include "version.h"

AboutDialog::AboutDialog(QWidget *p, Qt::WindowFlags fl)
	: QDialog(p,fl)
{
	QFile f;

	setWindowTitle(tr("About QPxTool"));

	layout = new QVBoxLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);

	lversion = new QLabel("QPxTool version " VERSION, this);
	lversion->setAlignment(Qt::AlignCenter);
	layout->addWidget(lversion);
	
	tw = new QTabWidget(this);
	layout->addWidget(tw);

	te_about = new QTextEdit(this);
	te_about->setReadOnly(true);
	te_licence = new QTextEdit(this);
	te_licence->setReadOnly(true);
	te_licence->setFontFamily("Monospace");

	f.setFileName(":about.html");
	f.open(QIODevice::ReadOnly);
	te_about->setHtml(f.readAll());
	f.close();

	f.setFileName(":COPYING");
	f.open(QIODevice::ReadOnly);
	te_licence->setPlainText(f.readAll());
	f.close();

	tw->addTab(te_about, tr("About QPxTool"));
	tw->addTab(te_licence, tr("Licence"));

	resize(600,400);
}

AboutDialog::~AboutDialog() {}


