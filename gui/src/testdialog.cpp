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

#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTextBrowser>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "testdialog.h"

#include <QApplication>
#include <QDebug>
#include <progresswidget.h>
#include <defs.h>
#include <unistd.h>

#include <qpx_mmc_defs.h>

TestDialog::TestDialog(QPxSettings *iset, device *idev, QWidget *p, Qt::WindowFlags f)
	: QDialog(p,f)
{
	settings = iset;
	dev = idev;
	setWindowTitle(tr("Select tests..."));

	winit();
	checkSimul();
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

// setting default selection for tests autostart
	dev->test_req = 0;
	if(!dev->media.creads || dev->media.type.startsWith("DVD+RW") || dev->media.type.startsWith("DVD-RAM")) {
		dev->test_req |=  (settings->actions_flags & AFLAG_DTEST_WT) ? TEST_WT : 0;
		dev->test_req |=  (settings->actions_flags & AFLAG_DTEST_FT_W) ? TEST_FT : 0;
	}
	if (!dev->media.creads || dev->media.type.startsWith("DVD+RW")) {
		dev->WT_simul = (settings->actions_flags & AFLAG_DTEST_WT_SIMUL) ? !!noSimul : 0;
	}
	if(dev->media.creads) {
		dev->test_req |=  (settings->actions_flags & AFLAG_DTEST_RT) ? TEST_RT : 0;
		dev->test_req |=  (settings->actions_flags & AFLAG_DTEST_ERRC) ? TEST_ERRC : 0;
		dev->test_req |=  (settings->actions_flags & AFLAG_DTEST_JB) ? TEST_JB : 0;
		dev->test_req |=  (settings->actions_flags & AFLAG_DTEST_FT_B) ? TEST_FT : 0;
		dev->test_req |=  (settings->actions_flags & AFLAG_DTEST_TA) ? TEST_TA : 0;
	}
	updateData(false);
}

TestDialog::~TestDialog()
{

}

device* TestDialog::getDevice()
{
	return dev;
}

void TestDialog::winit()
{
	layout = new QGridLayout(this);
	layout->setMargin(3);
	layout->setSpacing(6);
/*
	layout_dev = new QHBoxLayout();
	layout_dev->setMargin(3);
	layout_dev->setSpacing(3);
	layout->addLayout(layout_dev, 0,0,1,4);
*/	
	ldev = new QLabel(tr("Device:"),this);
	ldev->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(ldev,0,0);

	devid = new QLabel(this);
	devid->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout->addWidget(devid,0,1);

	lmedia = new QLabel(tr("Media:"),this);
	lmedia->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(lmedia,1,0);

	media = new QLabel(this);
	media->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout->addWidget(media,1,1);

	llabel = new QLabel(tr("Label:"),this);
	llabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(llabel,2,0);

	elabel = new QLineEdit(this);
	elabel->setMaxLength(128);
	layout->addWidget(elabel,2,1);

// tests selection

//	grp_tests = new QGroupBox(tr("Tests"),this);
	grp_tests = new QGroupBox(this);
	layout->addWidget(grp_tests, 3,0, 1,2);
	layout_tests = new QGridLayout(grp_tests);
	layout_tests->setMargin(3);
	layout_tests->setSpacing(3);
//	grp_tests->setLayout(layout_tests);

	l_tests  = new QLabel(tr("Tests:"),this);
	layout_tests->addWidget(l_tests, 0,0);
	l_speeds = new QLabel(tr("Speeds:"),this);
	layout_tests->addWidget(l_speeds, 0,1);

	ck_RT = new QCheckBox(tr("Read Transfer Rate"), this);
	layout_tests->addWidget(ck_RT,  1,0);
	spd_RT = new QComboBox(this);
	layout_tests->addWidget(spd_RT, 1,1);

	ck_WT = new QCheckBox(tr("Write Transfer Rate"), this);
	layout_tests->addWidget(ck_WT,  2,0);
	spd_WT = new QComboBox(this);
	layout_tests->addWidget(spd_WT, 2,1);
	ck_WT_simul = new QCheckBox(tr("Simulation"), this);
	ck_WT_simul->setEnabled(false);
	layout_tests->addWidget(ck_WT_simul,  3,1);

	hline0 = new QFrame(this);
	hline0->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout_tests->addWidget(hline0, 4,0,1,2);

	
	ck_ERRC = new QCheckBox(tr("Error Correction"), this);
	layout_tests->addWidget(ck_ERRC,  5,0);
	spd_ERRC = new QComboBox(this);
	layout_tests->addWidget(spd_ERRC, 5,1);

	
	ck_JB = new QCheckBox(tr("Jitter/Asymmetry"), this);
	layout_tests->addWidget(ck_JB,  6,0);
	spd_JB = new QComboBox(this);
	layout_tests->addWidget(spd_JB, 6,1);
	
	ck_FT = new QCheckBox(tr("Focus/Tracking"), this);
	layout_tests->addWidget(ck_FT,  7,0);
	spd_FT = new QComboBox(this);
	layout_tests->addWidget(spd_FT, 7,1);

	
	ck_TA = new QCheckBox(tr("Time Analyser"), this);
	layout_tests->addWidget(ck_TA,  8,0);
/*
	spd_TA = new QComboBox(this);
	layout_tests->addWidget(spd_TA, 5,1);
*/

	hline1 = new QFrame(this);
	hline1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout_tests->addWidget(hline1, 9,0,1,2);

	l_plugin = new QLabel(tr("qScan plugin:"), this);
	layout_tests->addWidget(l_plugin, 10,0);

	cb_plugin = new QComboBox(this);
//	cb_plugin->setEnabled(false);
	layout_tests->addWidget(cb_plugin, 10,1);

	l_plugin_info = new QLabel(this);
	layout_tests->addWidget(l_plugin_info, 11,0,1,2);

	layout_tests->setRowStretch(12,10);

/*
// media summary
	grp_media = new QGroupBox(tr("Media summary:"),this);
	layout->addWidget(grp_media, 1,2, 1,2);
	layout_media = new QHBoxLayout(grp_media);
	layout_media->setMargin(3);
	layout_media->setSpacing(3);
//	grp_media->setLayout(layout_media);

	media = new QTextBrowser(grp_media);
	layout_media->addWidget(media);
	layout->setRowStretch(1,10);
*/
	layout_butt = new QHBoxLayout;
	layout_butt->setMargin(0);
	layout_butt->setSpacing(3);
	layout->addLayout(layout_butt, 4, 0, 1, 2);

	layout_butt->addStretch(3);
	butt_run = new QPushButton(tr("Run"),this);
	butt_run->setIcon(QIcon(":images/scan.png"));
	layout_butt->addWidget(butt_run,1);
	butt_cancel = new QPushButton(tr("Cancel"),this);
	butt_cancel->setIcon(QIcon(":images/x.png"));
	layout_butt->addWidget(butt_cancel,1);

	layout->setRowStretch(0,1);
	layout->setRowStretch(1,1);
	layout->setRowStretch(2,1);
	layout->setRowStretch(3,20);
	layout->setRowStretch(4,1);

	connect( ck_RT,   SIGNAL(clicked(bool)), spd_RT,SLOT(setEnabled(bool)));
	connect( ck_WT,   SIGNAL(clicked(bool)), spd_WT,SLOT(setEnabled(bool)));
	connect( ck_WT,   SIGNAL(clicked(bool)), this,SLOT(WTchecked(bool)));
	connect( ck_ERRC, SIGNAL(clicked(bool)), spd_ERRC,SLOT(setEnabled(bool)));
	connect( ck_JB,   SIGNAL(clicked(bool)), spd_JB,SLOT(setEnabled(bool)));
	connect( ck_FT,   SIGNAL(clicked(bool)), spd_FT,SLOT(setEnabled(bool)));
//	connect( ck_TA,   SIGNAL(clicked(bool)), spd_TA,SLOT(setEnabled(bool)));
//	connect( ck_plugin, SIGNAL(clicked(bool)), cb_plugin,SLOT(setEnabled(bool)));
	connect( cb_plugin, SIGNAL(activated(int)), this, SLOT(pluginChanged(int)));

	connect( butt_run,    SIGNAL(clicked()), this, SLOT(start()) );
	connect( butt_cancel, SIGNAL(clicked()), this, SLOT(reject()) );
}

void TestDialog::WTchecked(bool en)
{
	if (noSimul) return;
	ck_WT_simul->setEnabled(en);
}

void TestDialog::checkSimul()
{
	noSimul = 1;
	if ((dev->media.type.startsWith("CD-") && (dev->cap & CAP_TEST_WRITE_CD)) ||
		(dev->media.type.startsWith("DVD-") && !dev->media.type.startsWith("DVD-RAM") && (dev->cap & CAP_TEST_WRITE_DVD)) ||
		(dev->media.type.startsWith("DVD+") && (dev->cap & CAP_TEST_WRITE_DVD_PLUS)) )
	{
		noSimul = 0;
		ck_WT_simul->setEnabled( ck_WT->isChecked() );
	} else {
		ck_WT_simul->setEnabled(false);
	}
}

void TestDialog::start()
{
	saveData();
	if (elabel->text().isEmpty()) {
		QMessageBox::information(this, tr("Media label is empty!"), tr("You have to define a media label!"));
		return;
	}
	if (!dev->test_req) {
		QMessageBox::information(this, tr("No tests selected!"), tr("You have selected no tests!"));
		return;
	}
#ifndef QT_NO_DEBUG
	qDebug() << "Selected plugin: "<< dev->plugin;
#endif
	dev->media.label = elabel->text();
	accept();
}

void TestDialog::mediaChanged()
{
	qDebug() << "TestDialog::mediaChanged()";
	updateData(true, true);
}

void TestDialog::updateData(bool save, bool setPlugin)
{
	if (save) saveData();
	devid->setText(dev->id);
	if (dev->media.type == "-") {
		grp_tests->setEnabled(false);
		butt_run->setEnabled(false);
		media->setText("No Media");
		return;
	}
	butt_run->setEnabled(true);

	media->setText(dev->media.dstate + " "+ dev->media.type);
	grp_tests->setEnabled(true);

	ck_RT->setEnabled(dev->media.creads);
	ck_RT->setChecked(dev->test_req & TEST_RT);

	ck_WT->setEnabled(dev->media.dstate.contains("Blank", Qt::CaseInsensitive) 
			|| dev->media.type.startsWith("DVD+RW")
			|| dev->media.type.startsWith("DVD-RAM")
			);
	ck_WT->setChecked(dev->test_req & TEST_WT);
	ck_WT_simul->setChecked(dev->WT_simul);

	ck_ERRC->setEnabled(dev->test_cap & TEST_ERRC && dev->media.creads);
	ck_ERRC->setChecked(ck_ERRC->isEnabled() && (dev->test_req & TEST_ERRC));

	ck_JB->setEnabled(dev->test_cap & TEST_JB && dev->media.creads);
	ck_JB->setChecked(ck_JB->isEnabled() && (dev->test_req & TEST_JB));

	ck_FT->setEnabled(dev->test_cap & TEST_FT && dev->media.type!="-" && !dev->media.type.contains("-ROM", Qt::CaseInsensitive));
	ck_FT->setChecked(ck_FT->isEnabled() && (dev->test_req & TEST_FT));

	ck_TA->setEnabled(dev->test_cap & TEST_TA && dev->media.creads);
	ck_TA->setChecked(ck_TA->isEnabled() && (dev->test_req & TEST_TA));

	spd_RT->clear();   spd_RT->setEnabled(ck_RT->isChecked());
	spd_WT->clear();   spd_WT->setEnabled(ck_WT->isChecked()); ck_WT_simul->setEnabled(ck_WT->isChecked());
	spd_ERRC->clear(); spd_ERRC->setEnabled(ck_ERRC->isChecked());
	spd_JB->clear();   spd_JB->setEnabled(ck_JB->isChecked());
	spd_FT->clear();   spd_FT->setEnabled(ck_FT->isChecked());
//	spd_TA->clear();   spd_TA->setEnabled(ck_TA->isChecked());

	int idx;

	spd_RT->addItems(dev->media.rspeeds);    spd_RT->addItem("Maximum");
	idx = spd_RT->findText(QString::number(dev->tspeeds.rt)+".",Qt::MatchStartsWith);
#ifndef QT_NO_DEBUG
	qDebug() << "spd_RT: " << dev->tspeeds.rt <<"idx:" << idx;
#endif
	if (idx > 0) spd_RT->setCurrentIndex( idx );

	spd_WT->addItems(dev->media.wspeedsd);   spd_WT->addItem("Maximum");
	idx = spd_WT->findText(QString::number(dev->tspeeds.wt)+".",Qt::MatchStartsWith);
#ifndef QT_NO_DEBUG
	qDebug() << "spd_WT: " << dev->tspeeds.wt <<"idx:" << idx;
#endif
	if (idx > 0) spd_WT->setCurrentIndex( idx );

	spd_ERRC->addItems(dev->media.tspeeds_errc); spd_ERRC->addItem("Maximum");
	idx = spd_ERRC->findText(QString::number(dev->tspeeds.errc)+"X",Qt::MatchStartsWith);
#ifndef QT_NO_DEBUG
	qDebug() << "spd_ERRC: " << dev->tspeeds.errc <<"idx:" << idx;
#endif
	if (idx > 0) spd_ERRC->setCurrentIndex( idx );

	spd_JB->addItems(dev->media.tspeeds_jb);   spd_JB->addItem("Maximum");
	idx = spd_JB->findText(QString::number(dev->tspeeds.jb)+"X",Qt::MatchStartsWith);
#ifndef QT_NO_DEBUG
	qDebug() << "spd_JB: " << dev->tspeeds.jb <<"idx:" << idx;
#endif
	if (idx > 0) spd_JB->setCurrentIndex( idx );

	spd_FT->addItems(dev->media.wspeedsd);   spd_FT->addItem("Maximum");
	idx = spd_FT->findText(QString::number(dev->tspeeds.ft)+".",Qt::MatchStartsWith);
#ifndef QT_NO_DEBUG
	qDebug() << "spd_FT: " << dev->tspeeds.ft <<"idx:" << idx;
#endif
	if (idx > 0) spd_FT->setCurrentIndex( idx );

//	spd_TA->addItems(dev->media.rspeeds);

	if (setPlugin) {
		cb_plugin->addItem(tr("< Autodetect >")); cb_plugin->addItems(dev->plugin_names);
		idx = cb_plugin->findText(dev->plugin);
		if (idx > 0)
			cb_plugin->setCurrentIndex( idx );
		else
			pluginChanged(0);
	}
}

void TestDialog::saveData()
{
	dev->test_req =
		(ck_RT->isChecked() ?   TEST_RT : 0) |
		(ck_WT->isChecked() ?   TEST_WT : 0) |
		(ck_ERRC->isChecked() ? TEST_ERRC : 0) |
		(ck_JB->isChecked() ?   TEST_JB : 0) |
		(ck_FT->isChecked() ?   TEST_FT : 0) |
		(ck_TA->isChecked() ?   TEST_TA : 0);
	dev->WT_simul	  = noSimul ? 0 : ck_WT_simul->isChecked();

	dev->tspeeds.rt   = (int) spd_RT->currentText().remove(QRegExp("[Xx]")).toFloat();
	dev->tspeeds.wt   = (int) spd_WT->currentText().remove(QRegExp("[Xx]")).toFloat();
	dev->tspeeds.errc = (int) spd_ERRC->currentText().remove(QRegExp("[Xx]")).toFloat();
	dev->tspeeds.jb   = (int) spd_JB->currentText().remove(QRegExp("[Xx]")).toFloat();
	dev->tspeeds.ft   = (int) spd_FT->currentText().remove(QRegExp("[Xx]")).toFloat();
	dev->plugin  = (cb_plugin->currentIndex() ? cb_plugin->currentText() : "");
//	dev->tspeeds.ta   = (int) spd_TA->currentText().remove(QRegExp("[Xx]")).toFloat();
#ifndef QT_NO_DEBUG
	qDebug() << "spd RT  : " << dev->tspeeds.rt;
	qDebug() << "spd WT  : " << dev->tspeeds.wt;
	qDebug() << "spd ERRC: " << dev->tspeeds.errc;
	qDebug() << "spd JB  : " << dev->tspeeds.jb;
	qDebug() << "spd FT  : " << dev->tspeeds.ft;
#endif
}

void TestDialog::pluginChanged(int idx)
{
	ProgressWidget *progress;
//	bool relock=0;
//	bool preservePluginsList = dev->preservePluginsList;


#ifndef QT_NO_DEBUG
	qDebug() << "pluginChanged()";
#endif

	if (idx > (dev->plugin_infos.size()+2)) {
		l_plugin_info->setText(tr("plugin info error"));
		return;
	}

	if (!idx) {
		l_plugin_info->setText(tr("qScan will probe plugin for your drive"));
	} else {
		if (!dev->plugin_infos[idx-1].isEmpty()) {
			l_plugin_info->setText(dev->plugin_infos[idx-1]);
		} else {
			l_plugin_info->setText(tr("no plugin info"));
		}
	}

	saveData();
/*
	if (!dev->mutex->tryLock()) {
		relock = 1;
		dev->mutex->unlock();
	}
*/

	progress = new ProgressWidget(10,3, isVisible() ? this : (QWidget*)parent() );
	progress->setText(tr("Retrieving test capabilities..."));
	progress->show();

	dev->update_plugin_info();
	while(dev->isRunning()) { msleep ( 1 << 5); qApp->processEvents(); }
	delete progress;

//	dev->preservePluginsList = preservePluginsList;

//	if (relock) dev->mutex->lock();

	updateData(false, false);
}

