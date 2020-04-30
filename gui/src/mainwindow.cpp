/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2007-2010 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <QFont>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QToolButton>
#include <QStatusBar>
#include <QMessageBox>
#include <QPushButton>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QProgressBar>

#include <QProcess>
#include <QTcpSocket>

#include <QSqlQuery>
#include <QSqlError>

#include <QBuffer>
#include <QFile>
#include <QFileDialog>
#include <QUrl>
#include <QTextDocument>
#include <QMimeData>
#include <QDateTime>

#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>

#include <QApplication>

#include <colors.h>
#include <db_report_selection.h>
#include <preferences.h>
#include <devsettings.h>
#include <device.h>
#include <qpxiodevice.h>
#include <mainwidget.h>
#include <testdialog.h>
#include <progresswidget.h>
#include <printpreview.h>
#include <defs.h>
#include <about.h>

#include "version.h"
#include "mainwindow.h"

#include <unistd.h>

#include <QDebug>

#define PRINT_ERRC_DETAILED

#define PRINT_GRAPH_SCALE  2.0

#define HTML_GRAPH_W    600
#define HTML_GRAPH_H    300
#define HTML_GRAPH_WQ  "600"
#define HTML_GRAPH_HQ  "300"
#define HTML_GRAPH_HQ2 "150"

const QString errcNameCD[8]  = { "BLER", "E11", "E21", "E31", "E12", "E22", "E32", "UNCR" };
const QString errcNameDVD[8] = { "res",  "PIE", "PI8", "PIF", "POE", "PO8", "POF", "UNCR" };
const QString errcNameBD[8]  = { "res",  "LDC", "res", "res", "BIS", "res", "res", "UNCR" };

#include <typeinfo>
#include <getopt.h>

static struct option long_options[] = {
	{"load",        1, NULL, 'l'},
	{"help",	    0, NULL, 'h'},
	{0,0,0,0}
};

QPxToolMW::QPxToolMW(int ac, char **av, QWidget *p, Qt::WindowFlags fl)
	: QMainWindow(p, fl)
{
	char c;
	QString lname;
	splash = 1;
	testDialog = NULL;
#ifndef QT_NO_DEBUG
	qDebug("STA: QPxToolMW()");
#endif
	setWindowTitle("QPxTool - " VERSION);
	settings_load();

	while (1) {
		c = getopt_long(ac, av, "hl:", long_options, NULL);
		if (c == -1)
			break;
		switch(c) {
			case 'h':
				printf("qpxtool options:\n");
				printf("\t-l FILE\tload data from file\n");
				printf("\t-h\t\tshow this help\n");
				return;
			case 'l':
				lname = QString(optarg);
				break;
			default:
				break;
		}
	}

	scanbusio = new QPxIODevice(this);
	dhost = "";

	if (set.use_reports_db) {
		qDebug() << "Connecting to database...";
		if (!SqlOpenConnection(set.db, "reports")) {
			qDebug() << "Error connecting to database!";
			set.use_reports_db = false;
		}
	}

	winit();

	if (set.geometry_mw.width() > 0 && set.geometry_mw.height() > 0)
		setGeometry(set.geometry_mw);
	scanbus();

	if (!lname.isEmpty()) {
		load_results(lname);
	}
#ifndef QT_NO_DEBUG
	qDebug("END: QPxToolMW()");
#endif
	splash = 0;

	setAcceptDrops(true);
}

QPxToolMW::~QPxToolMW ()
{
#ifndef QT_NO_DEBUG
	qDebug("STA: ~QPxToolMW()");
#endif
	if (QSqlDatabase::database("reports").isOpen()) {
		qDebug() << "Closing database connection...";
		SqlCloseConnection("reports");
	}
	settings_save();
#ifndef QT_NO_DEBUG
	qDebug("Waiting for devices...");
#endif
	mutex_dev.lock();	
	mutex_dev.unlock();
#ifndef QT_NO_DEBUG
	qDebug("END: ~QPxToolMW()");
#endif
}

void QPxToolMW::winit()
{
	create_actions();
	winit_menubar();
	winit_toolbar();
	winit_statusbar();

	cwidget = new QWidget(this);
	setCentralWidget(cwidget);
	layout = new QVBoxLayout(cwidget);
	layout->setMargin(3);
	layout->setSpacing(3);

// device
	layout_dev = new QHBoxLayout;
	layout_dev->setMargin(0);
	layout_dev->setSpacing(3);
	layout->addLayout(layout_dev);

	l_dev = new QLabel(tr("Device:"),cwidget);
	l_dev->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	l_dev->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	layout_dev->addWidget(l_dev);

	c_dev = new QComboBox(cwidget);
	c_dev->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	QFont dfont = c_dev->font();
	//dfont.setFamily("Courier");
	dfont.setFamily("Monospace");
	c_dev->setFont( dfont );
	layout_dev->addWidget(c_dev);

	pb_loej = new QPushButton(cwidget);
	pb_loej->setIcon(QIcon(":images/loej.png"));
	pb_loej->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	layout_dev->addWidget(pb_loej);

	pb_lock = new QPushButton(cwidget);
	pb_lock->setIcon(QIcon(":images/lock.png"));
	pb_lock->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	layout_dev->addWidget(pb_lock);

/*
// separator
	hline = new QFrame(cwidget);
	hline->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(hline);
*/

// main widget
	mwidget = new QPxMainWidget(&set, &devices, cwidget);
	layout->addWidget(mwidget);

	connect(act_sb, SIGNAL(toggled(bool)), mwidget, SLOT(setSidebarVisible(bool)));

	connect(pb_loej, SIGNAL(clicked()), this, SLOT(loejToggle()));
	connect(pb_lock, SIGNAL(clicked()), this, SLOT(lockToggle()));
}

void QPxToolMW::create_actions()
{
	bool db_open = QSqlDatabase::database("reports").isOpen();

	act_exit = new QAction(QIcon(":images/exit.png"), tr("Exit"), this);
	act_scanbus = new QAction(QIcon(":images/refresh.png"), tr("Rescan local bus and network"), this);
	act_minfo = new QAction(QIcon(":images/refresh-media.png"), tr("Update media info"), this);
	act_devctl = new QAction(QIcon(":images/device.png"), tr("Device controls"), this);
	act_devctl->setShortcut( QKeySequence("Ctrl+D") );
	act_test = new QAction(QIcon(":images/scan.png"), tr("Run tests..."), this);
	act_test->setShortcut( QKeySequence("Ctrl+R") );
	act_stop = new QAction(QIcon(":images/stop.png"), tr("Stop tests"), this);
	act_pref = new QAction(QIcon(":images/settings.png"), tr("Preferences"), this);
	act_pref->setShortcut( QKeySequence("Ctrl+C") );
	act_print = new QAction(QIcon(":images/printer.png"), tr("Print test results"), this);
	act_print->setShortcut( QKeySequence("Ctrl+P") );
	act_report = new QAction(QIcon(":images/pdf.png"), tr("Export results to PDF"), this);
	act_export = new QAction(QIcon(":images/html.png"), tr("Export results to HTML"), this);
	act_save   = new QAction(QIcon(":images/save.png"), tr("Save results..."), this);
	act_save->setShortcut( QKeySequence("Ctrl+S") );
	act_load   = new QAction(QIcon(":images/fileopen.png"), tr("Load results..."), this);
	act_load->setShortcut( QKeySequence("Ctrl+O") );
	act_save_db = new QAction(QIcon(":images/db_save.png"), tr("Save results (DB)..."), this);
	act_load_db = new QAction(QIcon(":images/db_load.png"), tr("Load results (DB)..."), this);

	act_save_db->setEnabled(db_open);
	act_load_db->setEnabled(db_open);

	act_sb     = new QAction(QIcon(":images/"), tr("Show sidebar"), this);
	act_sb->setShortcut( QKeySequence("Alt+B") );
	act_sb->setCheckable(true);
	act_sb->setChecked(set.show_sidebar);

	QAction *act;
	act_sbgrp = new QActionGroup(this);

	act = act_sbgrp->addAction(QIcon(":images/device.png"), "Device");
	act_sblist << act; act->setCheckable(true);
	act->setShortcut( QKeySequence("Alt+1") );
	connect(act, SIGNAL(triggered()), this, SLOT(selectTab()));
	act->setChecked(true);
	act = act_sbgrp->addAction(QIcon(":images/disc.png"), "Media");
	act_sblist << act; act->setCheckable(true);
	act->setShortcut( QKeySequence("Alt+2") );
	connect(act, SIGNAL(triggered()), this, SLOT(selectTab()));
	act = act_sbgrp->addAction(QIcon(":images/test_rt.png"), "Read Transfer");
	act_sblist << act; act->setCheckable(true);
	act->setShortcut( QKeySequence("Alt+3") );
	connect(act, SIGNAL(triggered()), this, SLOT(selectTab()));
	act = act_sbgrp->addAction(QIcon(":images/test_wt.png"), "Write Transfer");
	act_sblist << act; act->setCheckable(true);
	act->setShortcut( QKeySequence("Alt+4") );
	connect(act, SIGNAL(triggered()), this, SLOT(selectTab()));
	act = act_sbgrp->addAction(QIcon(":images/test_errc.png"), "Error Correction");
	act_sblist << act; act->setCheckable(true);
	act->setShortcut( QKeySequence("Alt+5") );
	connect(act, SIGNAL(triggered()), this, SLOT(selectTab()));
	act = act_sbgrp->addAction(QIcon(":images/test_jb.png"), "Jitter/Asymmetry");
	act_sblist << act; act->setCheckable(true);
	act->setShortcut( QKeySequence("Alt+6") );
	connect(act, SIGNAL(triggered()), this, SLOT(selectTab()));
	act = act_sbgrp->addAction(QIcon(":images/test_ft.png"), "FE/TE");
	act_sblist << act; act->setCheckable(true);
	act->setShortcut( QKeySequence("Alt+7") );
	connect(act, SIGNAL(triggered()), this, SLOT(selectTab()));
	act = act_sbgrp->addAction(QIcon(":images/test_ta.png"), "Time Analyser");
	act_sblist << act; act->setCheckable(true);
	act->setShortcut( QKeySequence("Alt+8") );
	connect(act, SIGNAL(triggered()), this, SLOT(selectTab()));

	connect(act_exit, SIGNAL(triggered()), this, SLOT(exit()));
	connect(act_scanbus, SIGNAL(triggered()), this, SLOT(scanbus()));
	connect(act_minfo, SIGNAL(triggered()), this, SLOT(update_media_info()));
	connect(act_devctl, SIGNAL(triggered()), this, SLOT(device_settings()));
	connect(act_test, SIGNAL(triggered()), this, SLOT(select_tests()));
	connect(act_stop, SIGNAL(triggered()), this, SLOT(terminate_tests()));
	connect(act_pref, SIGNAL(triggered()), this, SLOT(preferences()));
	connect(act_print, SIGNAL(triggered()), this, SLOT(print_results()));
	connect(act_export, SIGNAL(triggered()), this, SLOT(export_results()));
	connect(act_report, SIGNAL(triggered()), this, SLOT(save_report()));
	connect(act_save, SIGNAL(triggered()), this, SLOT(save_results()));
	connect(act_load, SIGNAL(triggered()), this, SLOT(load_results()));
	connect(act_save_db, SIGNAL(triggered()), this, SLOT(save_results_db()));
	connect(act_load_db, SIGNAL(triggered()), this, SLOT(load_results_db()));
}

void QPxToolMW::winit_menubar()
{
	QMenu *menu;
//	QMenu *submenu;
	menubar = menuBar();

// menu "file"
	menu = new QMenu(tr("File"),this);
	menu->addAction(act_load_db);
	menu->addAction(act_save_db);
	menu->addSeparator();
	menu->addAction(act_load);
	menu->addAction(act_save);
	menu->addAction(act_export);
	menu->addAction(act_report);
	menu->addAction(act_print);
	menu->addSeparator();
	menu->addAction(act_pref);
	menu->addSeparator();
	menu->addAction(act_exit);

	menubar->addMenu(menu);

// menu "view"
	menu = new QMenu(tr("View"),this);

	menu->addActions(act_sblist);
	menu->addSeparator();
	menu->addAction(act_sb);

	menubar->addMenu(menu);

// menu "device"
	menu = new QMenu(tr("Device"),this);
	menu->addAction(act_scanbus);
	menu->addAction(act_minfo);
	menu->addAction(act_devctl);
	menu->addSeparator();
	menu->addAction(act_test);
	menu->addAction(act_stop);

	menubar->addMenu(menu);

// menu "delp"
	menu = new QMenu(tr("Help"),this);
	menu->addAction(QIcon(":images/info.png"), tr("About"), this, SLOT(about()));

	menubar->addMenu(menu);
}

void QPxToolMW::winit_toolbar()
{
//	QToolButton *tb;
	toolbar = new QToolBar("Main toolbar",this);
	toolbar->setIconSize(QSize(24,24));
	toolbar->setContextMenuPolicy(Qt::NoContextMenu);
	toolbar->setFloatable(false);
	toolbar->setMovable(false);

//	toolbar->addAction(act_exit);	// exit button
//	toolbar->addSeparator();
	toolbar->addAction(act_scanbus); // scanbus button
	toolbar->addAction(act_minfo);	// media info update
	toolbar->addAction(act_devctl);	// device controls
	toolbar->addAction(act_test);	// run tests
	toolbar->addAction(act_stop);	// terminate tests
	toolbar->addSeparator();
	toolbar->addAction(act_pref);	// settings button

	addToolBar(toolbar);
}

void QPxToolMW::winit_statusbar()
{
	status_progress = new QProgressBar(this);
	status_progress->setMaximumHeight(20);
	status_progress->setRange(0,1000);
	status_progress->reset();

	status_process  = new QLabel(this);
	status_process->setMaximumHeight(20);

	status_media = new QLabel(this);
	status_media->setMaximumHeight(20);

	status_mid = new QLabel(this);
	status_mid->setMaximumHeight(20);

	statusBar()->addWidget(status_progress,2);
	statusBar()->addWidget(status_process,2);
	statusBar()->addWidget(status_media,2);
	statusBar()->addWidget(status_mid,2);
}

void QPxToolMW::exit()
{
/*
#ifndef QT_NO_DEBUG
	qDebug("QPxToolMW::exit()");
#endif
	if (QMessageBox::question(this,
		"Exit QPxTool",
		"Are you sure to exit?",
		QMessageBox::Yes | QMessageBox::Cancel,
		QMessageBox::Yes) == QMessageBox::Yes)
*/
	close();
}

void QPxToolMW::selectTab()
{
	int idx;
	QAction *act = (QAction*) sender();
	idx = act_sblist.indexOf(act);
	if (act<0) return;
	mwidget->selectTab(idx);
}

void QPxToolMW::settings_load()
{
	set.load();
}

void QPxToolMW::settings_save()
{
	set.geometry_mw = geometry();
	set.save();
}

void QPxToolMW::select_tests()
{
	device *dev;
	if (!devices.size()) {
		QMessageBox::warning(this, tr("No devices"), tr("No devices found!\nCan't run tests"));
		return;
	}
	dev = devices.current();
	select_tests(dev);
}

void QPxToolMW::select_tests(device *dev)
{
	if (testDialog) return;

#ifndef QT_NO_DEBUG
	qDebug("select_tests()");
#endif
	if (!mutex_dev.tryLock()) {
		QMessageBox::warning(this, tr("Device busy"), tr("Bus scan in progress"));
		return;
	}
	if (!dev->mutex->tryLock()) {
		mutex_dev.unlock();
		QMessageBox::warning(this, tr("Device busy"), tr("It seems device already performing test\nor info updating in progress"));
		return;
	}
	dev->mutex->unlock();

	testDialog = new TestDialog(&set, dev, this);
	testDialog->setModal(true);
	//testDialog->setModal(false);

	connect (testDialog, SIGNAL(accepted()), this, SLOT(run_tests()));
	connect (testDialog, SIGNAL(rejected()), this, SLOT(cancel_run_tests()));

	testDialog->show();
}

void QPxToolMW::run_tests()
//void QPxToolMW::run_tests(device *dev)
{
	if (!testDialog) return;
	device *dev = testDialog->getDevice();
	testDialog->deleteLater();
	testDialog = NULL;

	connect( dev, SIGNAL(testsDone()), this, SLOT(tests_done()) );
	connect( dev, SIGNAL(testsError()), this, SLOT(tests_error()) );

	set.tests = dev->test_req;
//	dev->mutex->unlock();
	dev->start_tests();
	mutex_dev.unlock();
}

void QPxToolMW::cancel_run_tests()
//void QPxToolMW::cancel_run_tests(device *dev)
{
	if (!testDialog) return;
//	device *dev = testDialog->getDevice();
	testDialog->deleteLater();
	testDialog = NULL;

//	dev->mutex->unlock();
	mutex_dev.unlock();
}

void QPxToolMW::terminate_tests()
{
	device *dev = devices.current();
	if (dev->type == device::DevtypeNone) return;
	dev->stop_tests();
}

void QPxToolMW::tests_done()
{
	device *dev = (device*) sender();
	disconnect( dev, SIGNAL(testsDone()), this, SLOT(tests_done()) );
	disconnect( dev, SIGNAL(testsError()), this, SLOT(tests_error()) );

	if (set.report_autosave || set.report_autosave_db) {
		dev->mutex->lock();
		if (set.report_autosave)	autosave_report(dev);
		if (set.report_autosave_db)	save_results_db(dev);
		dev->mutex->unlock();
		if (set.actions_flags & AFLAG_EJECT_AFTER) dev->setFeature(FEATURE_LOEJ, 0);
	}
	QMessageBox::information(this, tr("Done"), dev->id + ":\n"+ tr("All tests finished"));
}

void QPxToolMW::tests_error()
{
	QMessageBox::warning(this, tr("Error"), tr("Error performing test!")+"\n"+tr("qScan finished with non-zero exit code"));
}

void QPxToolMW::scanbus()
{
	int     previdx;
	QString previd;
	device  *dev;
#ifndef QT_NO_DEBUG
	qDebug("STA: QPxToolMW::scanbus()");
#endif
//	mwidget->setDevice(NULL);
	if (!mutex_dev.tryLock()) {
		QMessageBox::information(this, tr("Device busy"), tr("Can't scan bus: some device(s) busy!"));
		return;
	}
	if (scanbusio->IODevice()) { mutex_dev.unlock(); return; }

// try lock all devices to ensure they not running any tests
	for (int i=0; i<devices.size(); i++) {
		if (!devices[i]->mutex->tryLock()) {
			for (int j=0; j<i; j++)
				devices[j]->mutex->unlock();
			mutex_dev.unlock();
			QMessageBox::information(this, tr("Device busy"), tr("Can't scan bus: some device(s) busy!"));
			return;
		}
	}
	c_dev->setEnabled(false);
	pb_loej->setEnabled(false);
	pb_lock->setEnabled(false);

	act_scanbus->setEnabled(false);

	act_export->setEnabled(false);
	act_save->setEnabled(false);
	act_load->setEnabled(false);
	act_print->setEnabled(false);
	act_minfo->setEnabled(false);
	act_devctl->setEnabled(false);
	act_test->setEnabled(false);
	act_stop->setEnabled(false);

	disconnect(c_dev, SIGNAL(currentIndexChanged(int)), this, SLOT(setDevice(int)));
//	if (scanbusDis) return;
//	scanbusDis = 1;
	previd = devices.current()->id;
	mwidget->clearDev();
	devices.setIdx(-1);
	devices.clear();
	c_dev->clear();

	if (!splash) {
		progress = new ProgressWidget(10,3,this);
		progress->setText(tr("Searching devices..."));
		progress->show();
	}

	dev = new device(this);
	dev->type = device::DevtypeVirtual;
/*
	dev->host = dhost;
	dev->port = dport;

	dev->path = outsl[1];
	dev->ven  = outsl[3];
	dev->dev  = outsl[5];
	dev->fw   = outsl[7];
*/
	dev->id = "RESULTS VIEWER [Virtual Device]";
	devices.append(dev);
	c_dev->addItem(dev->id);


	if (set.useLocal)  scanbus_local();
	if (set.useRemote) scanbus_remote();
//	scanbusDis = 0;
	if (!devices.size()) {
		c_dev->addItem(tr("No devices found!"));
	}

	mutex_dev.unlock();
	previdx = c_dev->findText(previd, Qt::MatchCaseSensitive);
#ifndef QT_NO_DEBUG
	qDebug() << "Prev device [" << previdx << "] '" << previd << "'";
#endif
// unlocking all devices
//	for (int i=0; i<devices.size(); i++)
//		devices[i]->mutex->unlock();

// selecting device
	setDevice( (previdx < 0) ? 0 : previdx );
	connect(c_dev, SIGNAL(currentIndexChanged(int)), this, SLOT(setDevice(int)));
	c_dev->setCurrentIndex( (previdx < 0) ? 0 : previdx );
#ifndef QT_NO_DEBUG
	qDebug("END: QPxToolMW::scanbus()");
#endif
	if (!splash) delete progress;
	c_dev->setEnabled( devices.size() );

	if (devices.size()) {
		act_load->setEnabled(true);
//		act_save->setEnabled(true);
		act_export->setEnabled(true);
		act_print->setEnabled(true);
/*
		act_minfo->setEnabled(true);
		act_devctl->setEnabled(true);
		act_test->setEnabled(true);
		act_stop->setEnabled(true);
*/
	}

	act_scanbus->setEnabled(true);
}

void QPxToolMW::scanbus_local()
{
	QProcess   *proc;
#ifndef QT_NO_DEBUG
	qDebug("STA: QPxToolMW::scanbus_local()");
#endif
	if (scanbusio->IODevice()) goto scanbus_local_end;
	proc = new QProcess(this);
	scanbusio->setIODevice(proc);
	dt = device::DevtypeLocal;
	dhost = "";

//	connect(proc, SIGNAL(readyReadStandardOutput()),
//					this, SLOT(qscan_process_scanbus()));
	connect(scanbusio, SIGNAL(readyReadLine()),
					this, SLOT(qscan_process_scanbus()));

	proc->start("qscan", QStringList("-l"));
	if (!proc->waitForStarted(10000)) {
#ifndef QT_NO_DEBUG
		qDebug("Can't run qscan!");
#endif
		if (!splash) progress->hide();
		QMessageBox::warning(this, tr("Warning"), tr("Unable to start qscan!\nLocal devices will not work"));
		if (!splash) progress->show();
		scanbusio->setIODevice(NULL);
		delete proc;
		goto scanbus_local_end;
	}
	proc->setReadChannel(QProcess::StandardOutput);

	while(proc->state() == QProcess::Running) {
		msleep(1<<5);
		qApp->processEvents();
//		if (!splash) progress->step();
	}

	proc->waitForFinished(10000);

	disconnect(proc);
	disconnect(scanbusio, SIGNAL(readyReadLine()),
					this, SLOT(qscan_process_scanbus()));
	scanbusio->setIODevice(NULL);
	delete proc;
scanbus_local_end:
#ifndef QT_NO_DEBUG
	qDebug("END: QPxToolMW::scanbus_local()");
#endif
	return;
}

void QPxToolMW::scanbus_remote()
{
	QTcpSocket *sock;
#ifndef QT_NO_DEBUG
	qDebug("STA: QPxToolMW::scanbus_remote()");
#endif
	int hosts_cnt = set.hosts.size();
	if (scanbusio->IODevice()) goto scanbus_remote_end;
	sock = new QTcpSocket(this);
	scanbusio->setIODevice(sock);
	dt = device::DevtypeTCP;
//	connect(sock, SIGNAL(readyRead()),
//					this, SLOT(qscan_process_scanbus()));
	connect(scanbusio, SIGNAL(readyReadLine()),
					this, SLOT(qscan_process_scanbus()));

	for (int i=0; i<hosts_cnt; i++) {
		dport=0;
		if (set.hosts[i][0] != '*' ) continue;
		dhost = set.hosts[i].mid(1,-1);
		if (i<set.ports.size())
			dport = set.ports[i].toInt();
		if (!dport) dport = 46660;
#ifndef QT_NO_DEBUG
		qDebug() << "Trying host " << i+1 << "/" << hosts_cnt << " [" << dhost << ":" << dport << "]";
#endif
// scanning host...
		sock->connectToHost(dhost, dport);
		if (!sock->waitForConnected(5000)) {
#ifndef QT_NO_DEBUG
			qDebug("Unable to connect to host!");
#endif
			if (!splash) progress->hide();
			QMessageBox::warning(this, tr("Warning"), tr("Unable to connect to qscand at %1:%2").arg(dhost).arg(dport));
			if (!splash) progress->show();
			continue;
		}
		qDebug() << "connection state: " << sock->state();
		sock->write("list\n");
		sock->write("close\n");

		while(sock->state() == QAbstractSocket::ConnectedState) {
			msleep(1<<5);
			qApp->processEvents();
//			if (!splash) progress->step();
		}
		sock->disconnectFromHost();
#ifndef QT_NO_DEBUG
		qDebug("Disconnected from QSCAND");
#endif
	}
	disconnect(sock);
	disconnect(scanbusio, SIGNAL(readyReadLine()),
					this, SLOT(qscan_process_scanbus()));
	scanbusio->setIODevice(NULL);
	delete sock;
scanbus_remote_end:
#ifndef QT_NO_DEBUG
	qDebug("END: QPxToolMW::scanbus_remote()");
#endif
	return;
}

void QPxToolMW::qscan_process_scanbus()
{
	QString  qout;
	QStringList  outsl;
	device	 *dev;

#ifndef QT_NO_DEBUG
	qDebug("STA: QPxToolMW::qscan_process_scanbus()");
#endif
	if (!scanbusio->IODevice()) goto scanbus_process_qscan_end;
	while (scanbusio->linesAvailable() ) {
		qout = scanbusio->readLine();
//		qout.remove("\n");
#ifndef QT_NO_DEBUG
		qDebug() << qout;
#endif
		if (qout.startsWith("D:")) {
			dev = new device(this);
			outsl = qout.split('\'');

			dev->type = dt;
			if (dt == device::DevtypeTCP) {
				dev->host = dhost;
				dev->port = dport;
			}
			dev->path = outsl[1];
			dev->ven  = outsl[3];
			dev->dev  = outsl[5];
			dev->fw   = outsl[7];

			if (dt == device::DevtypeLocal) {
				dev->id = QString("%1 %2 %3 [ %4 ]")
					.arg(dev->ven).arg(dev->dev).arg(dev->fw)
					.arg(dev->path);
			} else if (dt == device::DevtypeTCP) {
				dev->id = QString("%1 %2 %3 [ %4@%5:%6 ]")
					.arg(dev->ven).arg(dev->dev).arg(dev->fw)
					.arg(dev->path)
					.arg(dev->host).arg(dev->port);
			}
			devices.append(dev);
			c_dev->addItem(dev->id);
			dev->startWatcher();
		}
	}
scanbus_process_qscan_end:
#ifndef QT_NO_DEBUG
	qDebug("END: QPxToolMW::qscan_process_scanbus()");
#endif
	return;
}

void QPxToolMW::setDevice(int idx)
{
	bool   devact_en;
	device *dev;
#ifndef QT_NO_DEBUG
	qDebug() << "STA: QPxToolMW::setDevice: " << idx;
#endif
	if (idx<0 || idx >= devices.size() || devices.idx() == idx) return;
//	devices.current()->disconnect();
	devices.setIdx(idx);
	mwidget->selectDevice();
	dev = devices.current();

	devact_en = (dev->type == device::DevtypeLocal) || (dev->type == device::DevtypeTCP);

	act_minfo->setEnabled(devact_en);
	act_test->setEnabled(devact_en);
	act_stop->setEnabled(devact_en);
	act_save->setEnabled(devact_en);
	act_devctl->setEnabled(dev->type == device::DevtypeLocal);
	pb_loej->setEnabled(dev->type == device::DevtypeLocal);
	pb_lock->setEnabled(dev->type == device::DevtypeLocal);

	if (devact_en) {
		if (!dev->info_set) {
			dev->update_device_info();
			while( dev->isRunning() ) {
				msleep ( 1 << 5);
				qApp->processEvents();
			}
			dev->update_media_info();
			dev->info_set = 1;
		}
		if (dev->plextor_lock) {
			QMessageBox::warning(
					this,
					tr("Warning"),
					dev->id + "\n" + tr("Found locked Plextor device!\nQuality check and some other features will not work"));
		}
	}
	if (dev->media.type == "-") {
		status_media->setText("No Media");
		status_mid->clear();
	} else {
		status_media->setText(dev->media.dstate + " "+ dev->media.type);
		status_mid->setText(dev->media.mid);
	}
	status_process->setText(dev->nprocess);
	status_progress->setValue(dev->pprocess * 10);


#ifndef QT_NO_DEBUG
	qDebug("END: QPxToolMW::setDevice()");
#endif
	return;
}

void QPxToolMW::mediaUpdated(int) {
	device *dev = (device*) sender();
#ifndef QT_NO_DEBUG
	qDebug() << "QPxTool::mediaUpdated()" << dev->autoupdate;
#endif
	if ( typeid(*dev) != typeid(device)) return;

	if (dev == devices.current()) {
//		mwidget->closeChilds();
		if (dev->media.type == "-") {
			status_media->setText("No Media");
			status_mid->clear();
			return;
		} else {
			status_media->setText(dev->media.dstate + " "+ dev->media.type);
			status_mid->setText(dev->media.mid);
		}
	}

	if (!dev->autoupdate) return;
	dev->autoupdate = 0;

	if (testDialog) testDialog->mediaChanged();

	if (!testDialog) {
		if (dev->media.type == "-") return;
		if (
			((set.actions_flags & AFLAG_AUTOSTART_W) && dev->media.creads )
			|| ((set.actions_flags & AFLAG_AUTOSTART_B) && 
				(!dev->media.creads || dev->media.type.startsWith("DVD+RW") || dev->media.type.startsWith("DVD-RAM")) )
		) {
			select_tests(dev);
		}
	}
}

void QPxToolMW::update_device_info() { devices.current()->update_device_info(); }
void QPxToolMW::update_media_info()  { update_media_info(devices.current()); }

void QPxToolMW::update_media_info(device* dev)  {
	if (!dev->update_media_info())
		QMessageBox::warning(this, tr("Error"), dev->id + "\n" + tr("Can't update media info! Device busy"));
}

void QPxToolMW::process_started()
{
	device *dev = (device*) sender();
	if ( dev != devices.current()) return;
	status_process->setText(dev->nprocess);
	status_progress->setValue(0);
}

void QPxToolMW::process_finished()
{
	device *dev = (device*) sender();
	if ( dev != devices.current()) return;
	status_process->setText(dev->nprocess);
	status_progress->reset();
}

void QPxToolMW::process_progress()
{
	device *dev = (device*) sender();
	if ( dev != devices.current()) return;
	status_progress->setValue(dev->pprocess * 10);
}

void QPxToolMW::loejToggle()
{
	device *dev = devices.current();
	qDebug("QPxToolMW::loejToggle()");
	if (dev->type != device::DevtypeLocal) return;
	dev->setFeature(FEATURE_LOEJ_TOGGLE, 0);
}

void QPxToolMW::lockToggle()
{
	device *dev = devices.current();
	qDebug("QPxToolMW::lockToggle()");
	if (dev->type != device::DevtypeLocal) return;
	dev->setFeature(FEATURE_LOCK_TOGGLE, 0);
}

void QPxToolMW::device_settings()
{
	device *dev;
	devSettings *devSet;
#ifndef QT_NO_DEBUG
	qDebug("QPxToolMW::device_settings()");
#endif
	if (!devices.size()) {
		QMessageBox::warning(this, tr("No devices"), tr("No devices found!\nNothing to configure\n"));
		return;
	}
	dev = devices.current();
	if (!mutex_dev.tryLock()) {
		QMessageBox::warning(this, tr("Device busy"), tr("Bus scan in progress"));
		return;
	}
	if (!dev->mutex->tryLock()) {
		mutex_dev.unlock();
		QMessageBox::warning(this, tr("Device busy"), tr("Selected device busy"));
		return;
	}
	dev->mutex->unlock();

	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Retrieving device parameters..."));
	progress->show();

	connect(dev, SIGNAL(doneGetFeatures(int)), this, SLOT(update_features_done(int)));

	if (!dev->getFeatures()) {
		delete progress;
		mutex_dev.unlock();
		QMessageBox::warning(this, tr("Warning"), tr("Can't get device features!"));
		return;
	}

	while(dev->isRunning()) { msleep ( 1 << 5); qApp->processEvents(); }
	delete progress;
	disconnect(dev, SIGNAL(doneGetFeatures(int)), this, SLOT(update_features_done(int)));

	devSet = new devSettings(&set, dev, this);
	devSet->exec();
	delete devSet;
	mutex_dev.unlock();
}

void QPxToolMW::update_features_done(int exitcode)
{
	if (exitcode) {
		QMessageBox::warning(this,
				tr("Error"),
				tr("Error requesting device settings!")+"\n"+tr("cdvdcontrol finished with non-zero exit code"));
	}
}

void QPxToolMW::preferences()
{
	QPxPreferences *pref;
#ifndef QT_NO_DEBUG
	qDebug("QPxToolMW::preferences()");
#endif
	pref = new QPxPreferences(&set, this);
	pref->exec();
	delete pref;

	if (QSqlDatabase::database("reports").isOpen()) {
		qDebug() << "Closing database connection...";
		SqlCloseConnection("reports");
	}
	if (set.use_reports_db) {
		qDebug() << "Connecting to database...";
		if (!SqlOpenConnection(set.db, "reports")) {
			qDebug() << "Error connecting to database!";
			return;
		}
	}
	bool dbOpen = QSqlDatabase::database("reports").isOpen();
	act_save_db->setEnabled(dbOpen);
	act_load_db->setEnabled(dbOpen);

	mwidget->reconfig();
}

void QPxToolMW::save_report()
{
	device *dev = devices.current();
	QString fname; 
	QFileInfo finfo;
	fname = QFileDialog::getSaveFileName(this, tr("Export results to PDF"), set.last_res_path_pdf, "PDF files (*pdf *.PDF)");
	if (fname.isEmpty()) return;

	finfo.setFile(fname);
	set.last_res_path_pdf = finfo.absoluteDir().canonicalPath();
	qDebug() << "Saving PDF to: " << fname;

	save_report(dev, fname);
}

void QPxToolMW::autosave_report(device *dev)
{
#ifndef QT_NO_DEBUG
	qDebug("Autosaving report...");
#endif
	QString fpath,fname;
	QDateTime dt = QDateTime::currentDateTime();
	QDir dir;
	if (set.report_path.isEmpty()) {
#ifndef QT_NO_DEBUG
		qDebug() << "Reports path not defined!";
#endif
		return;
	}
#warning !!! hardcoded report name structure
	fpath = set.report_path + "/" + dt.toString("yyyy-MM-dd");
	fname = dt.toString("/hh-mm-ss_") + dev->media.type + ".pdf";

	if (!dir.exists(fpath) && !dir.mkpath(fpath)) {
		QMessageBox::warning(this, tr("Can't save report"), tr("Direcrory not exists and I can't create it") + ":\n" + fpath);
		return;
	}
	save_report(dev, fpath + "/" + fname);
}

void QPxToolMW::save_report(device *dev, QString fname)
{
	QPrinter printer(QPrinter::HighResolution);
	QTextDocument *doc = new QTextDocument(this);
	prepare_report(dev, doc);

	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setPaperSize(QPrinter::A4);
	printer.setOutputFileName(fname);

	PrintPreview::printDoc(this, &printer, doc);

	delete doc;
}

void QPxToolMW::print_results()
{
#ifndef QT_NO_DEBUG
	qDebug("QPxToolMW::print_results()");
#endif
	device *dev = devices.current();
	QTextDocument *doc = new QTextDocument(this);
	PrintPreview *preview;

	prepare_report(dev, doc);

	preview = new PrintPreview(this, doc);
	preview->exec();
	delete preview;

	delete doc;
}

void QPxToolMW::prepare_report(device *dev, QTextDocument* doc)
{
	QImage img;
	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Preparing report..."));
	progress->show();

	doc->setHtml( generate_html(dev, "") );

//	img = QImage(":images/logo.png");
//	doc->addResource(QTextDocument::ImageResource, QUrl("images/logo.png"), img);

	qApp->processEvents();
	if (dev->testData.rt.size() > 0) {
		img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
		mwidget->drawGraph(img, dev, TEST_RT);
		doc->addResource(QTextDocument::ImageResource, QUrl("images/rt.png"), img);

		qApp->processEvents();
	}
	if (dev->testData.wt.size() > 0) {
		img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
		mwidget->drawGraph(img, dev, TEST_WT);
		doc->addResource(QTextDocument::ImageResource, QUrl("images/wt.png"), img);

		qApp->processEvents();
	}
//	if (1) {
	if (dev->testData.errc.size() > 0) {
		img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
		mwidget->drawGraph(img, dev, TEST_ERRC);
		doc->addResource(QTextDocument::ImageResource, QUrl("images/errc.png"), img);

#ifdef PRINT_ERRC_DETAILED
		if (dev->media.type.startsWith("CD")) {
			for (int i=0; i<8; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<i))) {
					img = QImage(HTML_GRAPH_W, HTML_GRAPH_H,QImage::Format_ARGB32);
					mwidget->drawGraph(img, dev, TEST_ERRC, (1 << i));
					doc->addResource(QTextDocument::ImageResource, QUrl("images/"+errcNameCD[i]+".png"), img);
				}
				qApp->processEvents();
			}	
		} else if (dev->media.type.startsWith("DVD")) {
			for (int i=1; i<8; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<i))) {
					img = QImage(HTML_GRAPH_W, HTML_GRAPH_H,QImage::Format_ARGB32);
					mwidget->drawGraph(img, dev, TEST_ERRC, (1 << i));
					doc->addResource(QTextDocument::ImageResource, QUrl("images/"+errcNameDVD[i]+".png"), img);
				}
				qApp->processEvents();
			}
		} else if (dev->media.type.startsWith("BD")) {
			int residx[] = {1,4,7};
			for (int i=0; i<3; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<residx[i]))) {
					img = QImage(HTML_GRAPH_W, HTML_GRAPH_H,QImage::Format_ARGB32);
					mwidget->drawGraph(img, dev, TEST_ERRC, (1 << residx[i]));
					doc->addResource(QTextDocument::ImageResource, QUrl("images/"+errcNameBD[residx[i]]+".png"), img);
				}
				qApp->processEvents();
			}
		}
#endif

	}
	if (dev->testData.jb.size() > 0) {
		img = QImage(HTML_GRAPH_W, HTML_GRAPH_H,QImage::Format_ARGB32);
		mwidget->drawGraph(img, dev, TEST_JB);
		doc->addResource(QTextDocument::ImageResource, QUrl("images/jb.png"), img);

		qApp->processEvents();
	}
	if (dev->testData.ft.size() > 0) {
		img = QImage(HTML_GRAPH_W, HTML_GRAPH_H,QImage::Format_ARGB32);
		mwidget->drawGraph(img, dev, TEST_FT);
		doc->addResource(QTextDocument::ImageResource, QUrl("images/ft.png"), img);

		qApp->processEvents();
	}

	if (dev->test_req & TEST_TA) {
		for (int l=0; l<dev->media.ilayers; l++) 
		{
			for (int z=0; z<3; z++) 
			{
				img = QImage(HTML_GRAPH_W,HTML_GRAPH_H/2,QImage::Format_ARGB32);
				mwidget->drawGraph(img, dev, TEST_TA, (l << 28) | (z<<4) | 0);
				doc->addResource(QTextDocument::ImageResource, QUrl(QString("images/ta%1_%2_p.png").arg(l).arg(z)), img);

				qApp->processEvents();

				img = QImage(HTML_GRAPH_W,HTML_GRAPH_H/2,QImage::Format_ARGB32);
				mwidget->drawGraph(img, dev, TEST_TA, (l << 28) | (z<<4) | 1);
				doc->addResource(QTextDocument::ImageResource, QUrl(QString("images/ta%1_%2_l.png").arg(l).arg(z)), img);

				qApp->processEvents();
			}
		}
	}

	delete progress;
}

void QPxToolMW::export_results()
{
#ifndef QT_NO_DEBUG
	qDebug("QPxToolMW::export_results()");
#endif
	QImage img;
	QDir dir;
	QString fname, idir;
	QString html;
	QFile	f;
	QFileInfo finfo;
	device *dev = devices.current();

	fname = QFileDialog::getSaveFileName(this, tr("Export results to HTML"), set.last_res_path_html, "HTML files (*.html)");
	if (fname.isEmpty()) return;

	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Preparing report..."));
	progress->show();

	f.setFileName(fname);
	finfo.setFile(f);
	set.last_res_path_html = finfo.absoluteDir().canonicalPath();
	qDebug() << "Saving HTML to: " << fname;

	int idx = fname.lastIndexOf("/");
	idir = fname.mid(idx);
	if (idx>=0) {
		fname.truncate(idx);
	}
	idir.remove("/");
	if (idir.endsWith(".html")) idir.remove(".html");
	idir+= ".images/";

	qDebug() << "path: " << fname;
	qDebug() << "idir: " << idir;

	fname += "/" + idir;
	dir.mkpath(fname);

	qDebug() << "Images path: " << fname;

	html = generate_html(dev, idir);
	if(!f.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this, tr("Error"), tr("Unable to create file:\n")+fname);
		return;
	}
	f.write(html.toLatin1());
	f.close();

	img = QImage(":images/logo.png");
	img.save(fname+"logo.png");

//	prepare_images(true);

	if (dev->testData.rt.size() > 0) {
		img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
		mwidget->drawGraph(img, dev, TEST_RT);
		img.save(fname+"rt.png");

		qApp->processEvents();
	}
	if (dev->testData.wt.size() > 0) {
		img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
		mwidget->drawGraph(img, dev, TEST_WT);
		img.save(fname+"wt.png");

		qApp->processEvents();
	}
//	if (1) {
	if (dev->testData.errc.size() > 0) {
		img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
		mwidget->drawGraph(img, dev, TEST_ERRC);
		img.save(fname+"errc.png");
		qApp->processEvents();

		qDebug() << COL_RED << "tdata_errc:" << dev->media.tdata_errc << COL_NORM;
#ifdef PRINT_ERRC_DETAILED
		if (dev->media.type.startsWith("CD")) {
			for (int i=0; i<8; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<i))) {
					img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
					mwidget->drawGraph(img, dev, TEST_ERRC, (1 << i));
					img.save(fname+errcNameCD[i]+".png");
				}
				qApp->processEvents();
			}
		} else if (dev->media.type.startsWith("DVD")) {
			for (int i=1; i<8; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<i))) {
					img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
					mwidget->drawGraph(img, dev, TEST_ERRC, (1 << i));
					img.save(fname+errcNameDVD[i]+".png");
				}
				qApp->processEvents();
			}
		} else if (dev->media.type.startsWith("BD")) {
			int residx[] = {1,4,7};
			for (int i=0; i<3; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<residx[i]))) {
					img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
					mwidget->drawGraph(img, dev, TEST_ERRC, (1 << residx[i]));
					img.save(fname+errcNameBD[residx[i]]+".png");
				}
				qApp->processEvents();
			}
		}
#endif

	}
	if (dev->testData.jb.size() > 0) {
		img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
		mwidget->drawGraph(img, dev, TEST_JB);
		img.save(fname+"jb.png");

		qApp->processEvents();
	}
	if (dev->testData.ft.size() > 0) {
		img = QImage(HTML_GRAPH_W,HTML_GRAPH_H,QImage::Format_ARGB32);
		mwidget->drawGraph(img, dev, TEST_FT);
		img.save(fname+"ft.png");

		qApp->processEvents();
	}

	if (dev->test_req & TEST_TA) {
		for (int l=0; l<dev->media.ilayers; l++) 
		{
			for (int z=0; z<3; z++) 
			{
				img = QImage(HTML_GRAPH_W,HTML_GRAPH_H/2,QImage::Format_ARGB32);
				mwidget->drawGraph(img, dev, TEST_TA, (l << 28) | (z<<4) | 0);
				img.save( QString("images/ta%1_%2_p.png").arg(l).arg(z) );

				qApp->processEvents();

				img = QImage(HTML_GRAPH_W,HTML_GRAPH_H/2,QImage::Format_ARGB32);
				mwidget->drawGraph(img, dev, TEST_TA, (l << 28) | (z<<4) | 1);
				img.save( QString("images/ta%1_%2_l.png").arg(l).arg(z) );

				qApp->processEvents();
			}
		}
	}
	delete progress;
}


/*
void QPxToolMW::prepare_images(QString path)
{
	bool save;
	QString idir;
	if (path.isEmpty()) {
		idir=":images/";
		save = 0;
	} else {
		idir=path;
		save = 1;
	}

}
*/

QString QPxToolMW::generate_html(device *dev, QString idir)
{
	QString r;
	QString bgcolor;
	bool print = idir.isEmpty();

	r=  "<html>\n";
	r+= "<head>\n";
	r+= "\t<title>QPxTool scan results</title>\n";
	r+= "</head>\n";
	r+= "<body>\n";

	r+=	"<div align=\"center\">\n";

	if (print) {
		idir = "images";
		r+= "\t<img src=\":images/logo.png\">\n";
	} else {
		r+= "\t<img src=\""+idir+"/logo.png\">\n";
	}

	r+= "\t<br>\n";

	r+= "\t<font size=+1><b>\n";
	r+= "\t<table border=0 cellspacing=0 width=500>\n";
	r+= "\t\t<tr>\n";
	r+= "\t\t\t<td width=30\%>Device Path</td>\n";
	if (dev->host.isEmpty()) {
		r+= "\t\t\t<td width=70\%>" + dev->path + "</td>\n";
	} else {
		r+= "\t\t\t<td width=70\%>" + dev->path + "@" + dev->host + ":"+ QString::number(dev->port) + "</td>\n";
	}
	r+= "\t\t</tr>\n";
	r+= "\t\t<tr>\n";
	r+= "\t\t\t<td>Device ID</td>\n";
	r+= "\t\t\t<td>" + dev->ven + dev->dev + dev->fw + "</td>\n";
	r+= "\t\t</tr>\n";
	r+= "\t</table>\n";
	r+= "\t</b></font>\n";
//	r+= "\t<br>\n";

	r+= "\t<table border=0 cellspacing=0 width=500>\n";
	r+= "\t\t<tr>\n";
	r+= "\t\t\t<td width=30\%></td>\n";
	r+= "\t\t\t<td width=70\%>TLA# " + dev->tla + "<br>Serial# " + dev->sn + "</td>\n";
	r+= "\t\t</tr>\n";
	r+= "\t\t<tr>\n";
	r+= "\t\t\t<td>Media/Category:</td>\n";
	r+= "\t\t\t<td>" + dev->media.type + " / "+ dev->media.category + "</td>\n";
	r+= "\t\t</tr>\n";
	r+= "\t\t<tr>\n";
	r+= "\t\t\t<td>Media ID:</td>\n";
	r+= "\t\t\t<td>" + dev->media.mid + "</td>\n";
	r+= "\t\t</tr>\n";
	r+= "\t\t<tr>\n";
	r+= "\t\t\t<td>Written on:</td>\n";
	r+= "\t\t\t<td>" + dev->media.writer + "</td>\n";
	r+= "\t\t</tr>\n";
	r+= "\t\t<tr>\n";
	r+= "\t\t\t<td>Capacity (readable):</td>\n";
	r+= "\t\t\t<td>" + QString::number(dev->media.creadm) + "MB (" + QString::number(dev->media.creads) + " sectors)</td>\n";
	r+= "\t\t</tr>\n";
	r+= "\t\t<tr>\n";
	r+= "\t\t\t<td>Capacity (total):</td>\n";
	r+= "\t\t\t<td>" + QString::number(dev->media.ctotm) + "MB (" + QString::number(dev->media.ctots) + " sectors)</td>\n";
	r+= "\t\t</tr>\n";
	r+= "\t</table>\n";
	r+= "\t<br>\n";
	r+= "<div>\n";

	if (dev->testData.rt.size() > 0) {
		r+=	"<p align=\"center\">\n";
		r+= "\t<b>Read Transfer @ "+QString::number(dev->tspeeds.rt)+"X</b><br>\n";
		r+= QString("\tTest time: %1:%2<br>\n")
			.arg( ((int)dev->testData.rt_time) / 60)
			.arg( ((int)dev->testData.rt_time) % 60, 2, 10, QChar('0'));
		r+= "</p>\n";
		r+= "<table align=center border=0 cellspacing=1 width=300>\n";
		r+= "\t<tr bgcolor=#c0c0c0>\n";
		r+= "\t\t<td align=right width=33\%>Start</td>\n";
		r+= "\t\t<td align=right width=33\%>End</td>\n";
		r+= "\t\t<td align=right width=34\%>Avg</td>\n";
		r+= "\t</tr>\n";

		r+= QString("\t<tr bgcolor=#e0e0e0><td align=right>%1 X</td><td align=right>%2 X</td><td align=right>%3 X</td></tr>\n")
				.arg(dev->testData.rt.first().spdx)
				.arg(dev->testData.rt.last().spdx)
				.arg(dev->testData.rt.last().lba * 2 / dev->testData.rt_time / dev->media.spd1X, 0, 'f', 2);
		r+= QString("\t<tr bgcolor=#c0c0c0><td align=right>%1 kB/s</td><td align=right>%2 kB/s</td><td align=right>%3 kB/s</td></tr>\n")
				.arg(dev->testData.rt.first().spdk)
				.arg(dev->testData.rt.last().spdk)
				.arg(dev->testData.rt.last().lba * 2 / dev->testData.rt_time, 0, 'f', 2);

		r+= "</table>\n";
		r+= "<br>\n";
		
		r+= "<table border=0 cellspacing=0 align=\"center\">\n\t<tr><td align=\"center\">";
		r+= "<img src=\""+idir+"/rt.png\" alt=\"Read Transfer\" width="HTML_GRAPH_WQ" height="HTML_GRAPH_HQ">";
//		r+= "<br>Read Transfer";
		r+= "</td></tr>\n</table><br>\n";
	}

	if (dev->testData.wt.size() > 0) {
		r+=	"<p align=\"center\">\n";
		r+= "\t<b>Write Transfer @ "+QString::number(dev->tspeeds.wt)+"X</b><br>\n";
		r+= QString("\tTest time: %1:%2<br>\n")
			.arg( ((int)dev->testData.wt_time) / 60)
			.arg( ((int)dev->testData.wt_time) % 60, 2, 10, QChar('0'));
		r+= "</p>\n";
		r+= "<table align=center border=0 cellspacing=1 width=300>\n";
		r+= "\t<tr bgcolor=#c0c0c0>\n";
		r+= "\t\t<td align=right width=33\%>Start</td>\n";
		r+= "\t\t<td align=right width=33\%>End</td>\n";
		r+= "\t\t<td align=right width=34\%>Avg</td>\n";
		r+= "\t</tr>\n";
		r+= QString("\t<tr bgcolor=#e0e0e0><td align=right>%1 X</td><td align=right>%2 X</td><td align=right>%3 X</td></tr>\n")
				.arg(dev->testData.wt.first().spdx)
				.arg(dev->testData.wt.last().spdx)
				.arg(dev->testData.wt.last().lba * 2 / dev->testData.rt_time / dev->media.spd1X, 0, 'f', 2);
		r+= QString("\t<tr bgcolor=#c0c0c0><td align=right>%1 kB/s</td><td align=right>%2 kB/s</td><td align=right>%3 kB/s</td></tr>\n")
				.arg(dev->testData.wt.first().spdk)
				.arg(dev->testData.wt.last().spdk)
				.arg(dev->testData.wt.last().lba * 2 / dev->testData.rt_time, 0, 'f', 2);

		r+= "</table>\n";
		r+= "<br>\n";

		r+= "<table border=0 cellspacing=0 align=\"center\">\n\t<tr><td align=\"center\">";
		r+= "<img src=\""+idir+"/wt.png\" alt=\"Write Transfer\" width="HTML_GRAPH_WQ" height="HTML_GRAPH_HQ">";
//		r+= "<br>Write Transfer";
		r+= "</td></tr>\n</table><br>\n";
	}

	if (dev->testData.errc.size() > 0) {
		r+=	"<p align=\"center\">\n";
		r+= "\t<b>Error Correction @ "+QString::number(dev->tspeeds.errc)+"X</b><br>\n";
		r+= QString("\tTest time: %1:%2<br>\n")
			.arg( ((int)dev->testData.errc_time) / 60)
			.arg( ((int)dev->testData.errc_time) % 60, 2, 10, QChar('0'));
		r+= "</p>\n";
		r+= "<table align=center border=0 cellspacing=1 width=500>\n";
		r+= "\t<tr bgcolor=#c0c0c0>\n";
		r+= "\t\t<td width=25\%></td>\n";
		r+= "\t\t<td align=right width=25\%>Total</td>\n";
		r+= "\t\t<td align=right width=25\%>Max</td>\n";
		r+= "\t\t<td align=right width=25\%>Avg</td>\n";
		r+= "\t</tr>\n";

		if (dev->media.type.startsWith("CD")) {
			for (int i=0; i<8; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<i))) {
					bgcolor = (i%2) ? "#d0d0d0" : "#e0e0e0";
					r+= QString("\t\t<tr bgcolor=%1><td>%2</td><td align=right>%3</td><td align=right>%4</td><td align=right>%5</td></tr>\n")
						.arg(bgcolor)
						.arg(errcNameCD[i])
						.arg(dev->testData.errcTOT.raw.err[i])
						.arg(dev->testData.errcMAX.raw.err[i])
						.arg(dev->testData.errcAVG.raw.err[i], 0, 'f', 2);
				}
			}
		} else if (dev->media.type.startsWith("DVD")) {
			for (int i=1; i<8; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<i))) {
					bgcolor = (i%2) ? "#e0e0e0" : "#d0d0d0";
					r+= QString("\t\t<tr bgcolor=%1><td>%2</td><td align=right>%3</td><td align=right>%4</td><td align=right>%5</td></tr>\n")
						.arg(bgcolor)
						.arg(errcNameDVD[i])
						.arg(dev->testData.errcTOT.raw.err[i])
						.arg(dev->testData.errcMAX.raw.err[i])
						.arg(dev->testData.errcAVG.raw.err[i], 0, 'f', 2);
				}
			}
		} else if (dev->media.type.startsWith("BD")) {
			int residx[] = {1,4,7};
			for (int i=0; i<3; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<residx[i]))) {
					bgcolor = (i%2) ? "#e0e0e0" : "#d0d0d0";
					r+= QString("\t\t<tr bgcolor=%1><td>%2</td><td align=right>%3</td><td align=right>%4</td><td align=right>%5</td></tr>\n")
						.arg(bgcolor)
						.arg(errcNameBD[residx[i]])
						.arg(dev->testData.errcTOT.raw.err[residx[i]])
						.arg(dev->testData.errcMAX.raw.err[residx[i]])
						.arg(dev->testData.errcAVG.raw.err[residx[i]], 0, 'f', 2);
				}
			}
		}

		r+= "</table>\n";
		r+= "<br>\n";

		r+= "<table border=0 cellspacing=0 align=\"center\">\n\t<tr><td align=\"center\">";
		r+= "<img src=\""+idir+"/errc.png\" alt=\"Error correction\" width="HTML_GRAPH_WQ" height="HTML_GRAPH_HQ">\n";
		r+= "<br>Overall ERRC Graph";
		r+= "</td></tr></table><br>\n";

#ifdef PRINT_ERRC_DETAILED
		if (dev->media.type.startsWith("CD")) {
			for (int i=0; i<8; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<i))) {
					r+= "<table border=0 cellspacing=0 align=\"center\">\n\t<tr><td align=\"center\">";
					r+= "\t<img src=\""+idir+"/"+errcNameCD[i]+".png\" width="HTML_GRAPH_WQ" height="HTML_GRAPH_HQ">";
					r+= "<br>"+errcNameCD[i];
					r+= "</td></tr>\n</table><br>\n";
				}
			}	
		} else if (dev->media.type.startsWith("DVD")) {
			for (int i=1; i<8; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<i))) {
					r+= "<table border=0 cellspacing=0 align=\"center\">\n\t<tr><td align=\"center\">";
					r+= "\t<img src=\""+idir+"/"+errcNameDVD[i]+".png\" width="HTML_GRAPH_WQ" height="HTML_GRAPH_HQ">";
					r+= "<br>"+errcNameDVD[i];
					r+= "</td></tr>\n</table><br>\n";
				}
			}
		} else if (dev->media.type.startsWith("BD")) {
			int residx[] = {1,4,7};
			for (int i=0; i<3; i++) {
				if (!dev->media.tdata_errc || (dev->media.tdata_errc & (1<<residx[i]))) {
					r+= "<table border=0 cellspacing=0 align=\"center\">\n\t<tr><td align=\"center\">";
					r+= "\t<img src=\""+idir+"/"+errcNameBD[residx[i]]+".png\" width="HTML_GRAPH_WQ" height="HTML_GRAPH_HQ">";
					r+= "<br>"+errcNameBD[residx[i]];
					r+= "</td></tr>\n</table><br>\n";
				}
			}
		}
#endif
	}

	if (dev->testData.jb.size() > 0) {
		r+=	"<p align=\"center\">\n";
		r+= "\t<b>Jitter/Asymmetry @ "+QString::number(dev->tspeeds.jb)+"X</b><br>\n";
		r+= QString("\tTest time: %1:%2<br>\n")
			.arg( ((int)dev->testData.jb_time) / 60)
			.arg( ((int)dev->testData.jb_time) % 60, 2, 10, QChar('0'));
		r+= "</p>\n";
		r+= "\t<table align=center border=0 cellspacing=1 width=300>\n";
		r+= "\t\t<tr bgcolor=#c0c0c0><td></td><td>Min</td><td>Max</td></tr>";
		r+= QString("\t\t<tr bgcolor=#d0d0d0><td>Jitter</td><td>%1</td><td>%2</td></tr>")
			.arg(dev->testData.jbMM.jmin)
			.arg(dev->testData.jbMM.jmax);
		r+= QString("\t\t<tr bgcolor=#c0c0c0><td>Asymmetry</td><td>%1</td><td>%2</td></tr>")
			.arg(dev->testData.jbMM.bmin)
			.arg(dev->testData.jbMM.bmax);
		r+= "\t</table>\n";
		r+= "<br>\n";
		r+= "<table border=0 cellspacing=0 align=center>\n\t<tr><td align=center>\n";
		r+= "<img src=\""+idir+"/jb.png\" width="HTML_GRAPH_WQ" height="HTML_GRAPH_HQ">";
		r+= "</td></tr>\n</table><br>\n";
	}

	if (dev->testData.ft.size() > 0) {
		r+=	"<p align=\"center\">\n";
		r+= "\t<b>Focus/Tracking Errors @ "+QString::number(dev->tspeeds.ft)+"X</b><br>\n";
		r+= QString("\tTest time: %1:%2<br>\n")
			.arg( ((int)dev->testData.ft_time) / 60)
			.arg( ((int)dev->testData.ft_time) % 60, 2, 10, QChar('0'));
		r+= "</p>\n";
		r+= "\t<table align=center border=0 cellspacing=1 width=200>\n";
		r+= "\t\t<tr bgcolor=#c0c0c0><td>FE Max</td><td>"+QString::number(dev->testData.ftMAX.fe)+"</td></tr>";
		r+= "\t\t<tr bgcolor=#d0d0d0><td>TE Max</td><td>"+QString::number(dev->testData.ftMAX.te)+"</td></tr>";
		r+= "\t</table>\n";
		r+= "<br>\n";
		r+= "<table border=0 cellspacing=0 align=center>\n\t<tr><td align=center>\n";
		r+= "<img src=\""+idir+"/ft.png\" width="HTML_GRAPH_WQ" height="HTML_GRAPH_HQ">";
		r+= "</td></tr></table><br>\n";
	}

	if (dev->test_req & TEST_TA) {
		r+=	"<p align=\"center\">\n";
		r+= "\t<b>Time Analyser</b><br>\n";
		r+= QString("\tTest time: %1:%2<br>\n")
			.arg( ((int)dev->testData.ta_time) / 60)
			.arg( ((int)dev->testData.ta_time) % 60, 2, 10, QChar('0'));
		r+= "</p>\n";
		r+= "\t<table align=center border=0 cellspacing=0 width=80\%>\n";
#warning TA analysis data
		r+= "\t</table>\n";

		for (int l=0; l<dev->media.ilayers; l++) 
		{
			for (int z=0; z<3; z++) 
			{
				r+= "<table border=0 cellspacing=0 align=center>\n\t<tr><td align=center>";
				r+= "<img src=\""+idir+"/ta"+QString::number(l)+"_"+QString::number(z)+"_p.png\" width="HTML_GRAPH_WQ" height="HTML_GRAPH_HQ2">";
				r+= "</td></tr>\n\t<tr><td align=\"center\">";
				r+= "<img src=\""+idir+"/ta"+QString::number(l)+"_"+QString::number(z)+"_l.png\" width="HTML_GRAPH_WQ" height="HTML_GRAPH_HQ2">";
				r+= "</td></tr>\n</table><br>\n";
			}
		}
	}

	r+= "\t<br><br>\n";
	r+= "\t\n";
	r+=	"<p align=\"center\">\n";
	r+= "\tGenerated by QPxTool v"VERSION" (c) Gennady \"ShultZ\" Kozlov\n";
	r+= "\t<a href=\"http://qpxtool.sourceforge.net\">http://qpxtool.sourceforge.net</a><br>\n";
	r+= "</p>\n";
	r+= "</body>\n</html>\n";
	return r;
}

void QPxToolMW::about()
{
#ifndef QT_NO_DEBUG
	qDebug("QPxToolMW::about()");
#endif
	AboutDialog *about = new AboutDialog(this);
	about->exec();
	delete about;
}

void QPxToolMW::save_results()
{
#ifndef QT_NO_DEBUG
	qDebug("QPxToolMW::save_results()");
	timeval tb,te;
#endif
	QString fname;
	QFile	f;
	QFileInfo	finfo;

	if (!devices.size()) return;
	device *dev = devices.current();

	fname = QFileDialog::getSaveFileName(this, tr("Save results to file..."), set.last_res_path_native, "QPxTool data files (*.qpx)");
	if (fname.isEmpty()) return;

	f.setFileName(fname);
	finfo.setFile(f);
	set.last_res_path_native = finfo.absoluteDir().canonicalPath();
	qDebug() << "Saving data to: " << fname;

	if (!f.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this, tr("Error"), tr("Unable to create file:\n")+fname);
		return;
	}
#ifndef QT_NO_DEBUG
	gettimeofday(&tb, NULL);
#endif

	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Saving results..."));
	progress->show();

	dev->save(&f);
	while(dev->isSaving()) { msleep ( 1 << 5); qApp->processEvents(); }

	delete progress;
	if (!dev->saveResult()) {
		QMessageBox::warning(this, tr("Warning"), tr("Error saving tests data!"));
#ifndef QT_NO_DEBUG
	} else {
		gettimeofday(&te, NULL);
		double t = (te.tv_sec - tb.tv_sec) + (te.tv_usec - tb.tv_usec)/1000000.0;
		QMessageBox::information(this, tr("Info"), tr("Tests data saved in %1 sec").arg(t,0,'f',2));
#endif
	}
	f.close();
}

void QPxToolMW::save_results_db(device *idev)
{
#ifndef QT_NO_DEBUG
	qDebug("QPxToolMW::save_results_db()");
	timeval tb,te;
#endif
	QByteArray ba;
	QSqlQuery	*q = NULL;
	QBuffer	buf(&ba, this);
	device *dev = idev;
	int id_vendor, id_model, id_media;


	if (!devices.size()) return;
	if (!dev) dev = devices.current();

#warning !!! set media label before saving to database
	if (dev->media.label.isEmpty()) {
		//
	}

	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Saving results..."));
	progress->show();

	q = new QSqlQuery(QSqlDatabase::database("reports"));
// read media id
	q->prepare("SELECT id FROM media_types WHERE name=:name");
	q->bindValue(":name", dev->media.type);
	if (!q->exec()) {
		qDebug() << q->lastError().text();
		goto close_db;
	}
	if (!q->next()) {
		qDebug() << "No ID found for current media";
		goto close_db;
	}
	id_media = q->value(0).toInt();
	qDebug() << "id_media : " << id_media;

// check&update vendor
	q->prepare("SELECT id FROM dev_vendors WHERE name=:name");
	q->bindValue(":name", dev->ven);
	if (!q->exec()) {
		qDebug() << q->lastError().text();
		goto close_db;
	}
	if (!q->next()) {
		q->prepare("INSERT INTO dev_vendors (name) VALUES (:name) RETURNING id");
		q->bindValue(":name", dev->ven);
		if (!q->exec()) {
			qDebug() << q->lastError().text();
			goto close_db;
		}
		q->next();
		qDebug() << "added vendor id " << q->value(0).toInt();
	}
	id_vendor = q->value(0).toInt();
	qDebug() << "id_vendor: " << id_vendor;

// check&update model
	q->prepare("SELECT id FROM dev_models WHERE name=:name AND id_vendor=:id_vendor");
	q->bindValue(":id_vendor", id_vendor);
	q->bindValue(":name", dev->dev);
	if (!q->exec()) {
		qDebug() << q->lastError().text();
		goto close_db;
	}
	if (!q->next()) {
//		q->prepare("INSERT INTO dev_models (id_vendor,name) VALUES ("+QString::number(id_vendor)+",'"+dev->dev+"') RETURNING id");
		q->prepare("INSERT INTO dev_models (id_vendor,name) VALUES (:id_vendor,:name) RETURNING id");
		q->bindValue(":id_vendor", id_vendor);
		q->bindValue(":name", dev->dev);
		if (!q->exec()) {
			qDebug() << q->lastError().text();
			goto close_db;
		}
		q->next();
		qDebug() << "added model id " << q->value(0).toInt();
	}
	id_model = q->value(0).toInt();
	qDebug() << "id_model : " << id_model;

// preparing report data...
	qDebug() << "Filling buffer with report data...";

	if (!buf.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this, tr("Error"), tr("Unable to open buffer!\n"));
		goto close_db;
	}
#ifndef QT_NO_DEBUG
	gettimeofday(&tb, NULL);
#endif

	dev->save(&buf);
	while(dev->isSaving()) { msleep ( 1 << 5); qApp->processEvents(); }


	if (!dev->saveResult()) {
		QMessageBox::warning(this, tr("Warning"), tr("Error saving tests data!"));
#ifndef QT_NO_DEBUG
	} else {
		gettimeofday(&te, NULL);
		double t = (te.tv_sec - tb.tv_sec) + (te.tv_usec - tb.tv_usec)/1000000.0;
		QMessageBox::information(this, tr("Info"), tr("Tests data saved in %1 sec").arg(t,0,'f',2));
#endif
	}
	buf.close();

	qDebug() << "Buffer prepared, writing to database...";

// writing results...
	q->prepare("INSERT INTO reports ( \
			dev_id, dev_fw, \
			id_media_type, label, copy_idx, \
			md5, data_xml \
		) VALUES (\
			:dev_id, :dev_fw, \
			:id_media_type, :label, :copy_idx, \
			:md5, :data_xml \
		) RETURNING id");

#warning empty fields in report saving query!!!
	q->bindValue(":dev_id", id_model);
	q->bindValue(":dev_fw", dev->fw);
	q->bindValue(":id_media_type", id_media);
	q->bindValue(":label", dev->media.label);
	q->bindValue(":copy_idx", 0);
	q->bindValue(":md5", "");
	q->bindValue(":data_xml", ba);
	if (!q->exec()) {
		qDebug() << q->lastError().text();
	} else {
		q->next();
		qDebug() << "added report id " << q->value(0).toInt();
	}

close_db:
	if (q) delete q;
	delete progress;
}

void QPxToolMW::load_results()
{
	QString fname;
	QFileInfo finfo;
	if (!devices.size()) return;
	fname = QFileDialog::getOpenFileName(this, tr("Load results from file..."), set.last_res_path_native, "QPxTool data files (*.qpx)");
	if (fname.isEmpty()) return;

	finfo.setFile(fname);
	set.last_res_path_native = finfo.absoluteDir().canonicalPath();
	qDebug() << "Loading data from: " << fname;

	load_results(fname);
}

void QPxToolMW::load_results(QString fname)
{
#ifndef QT_NO_DEBUG
	qDebug("QPxToolMW::load_results()");
	timeval tb,te;
#endif
	QFile	f;

	if (!devices.size()) return;
	device *dev = devices[0];

	if (fname.isEmpty()) return;
//	c_dev->setCurrentIndex(0);
//	setDevice(0);

	f.setFileName(fname);
	qDebug() << "Loading data from: " << fname;

	if (!f.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(this, tr("Error"), tr("Unable to open file:\n")+fname);
		return;
	}
#ifndef QT_NO_DEBUG
	gettimeofday(&tb, NULL);
#endif

	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Loading results..."));
	progress->show();

	dev->load(&f);
	while(dev->isLoading()) { msleep ( 1 << 5); qApp->processEvents(); }

	delete progress;
	if (!dev->loadResult()) {
		QMessageBox::warning(this, tr("Warning"), tr("Do tests data found in this file!"));
	} else {
		c_dev->setCurrentIndex(0);
#ifndef QT_NO_DEBUG
		gettimeofday(&te, NULL);
		double t = (te.tv_sec - tb.tv_sec) + (te.tv_usec - tb.tv_usec)/1000000.0;
		QMessageBox::information(this, tr("Info"), tr("Tests data loaded in %1 sec").arg(t,0,'f',2));
#endif
	}
	f.close();
//	setDevice(0);
}

void QPxToolMW::load_results_db()
{
#ifndef QT_NO_DEBUG
	qDebug("QPxToolMW::load_results_db()");
	timeval tb,te;
#endif
	QByteArray ba;
	QSqlQuery	*q = NULL;
	QBuffer	buf(this);
	DbReportSelection *rsel;
	int id_report;

	if (!devices.size()) return;
	device *dev = devices[0];

#ifndef QT_NO_DEBUG
	gettimeofday(&tb, NULL);
#endif

	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Loading results..."));
// report selection...
	rsel = new DbReportSelection("report", this);
	if (rsel->exec() == QDialog::Rejected) {
		delete rsel;
		return;
	}
	id_report = rsel->getReportID();
	delete rsel;

	if (id_report<0) {
		qDebug() << "Invalid report ID!";
		goto close_db;
	}

	progress->show();
	qApp->processEvents();
// reading report from database
	qDebug() << "Reading report from database...";

	q = new QSqlQuery(QSqlDatabase::database("reports"));
	qDebug() << "Report ID:" << id_report;
	q->prepare("SELECT data_xml FROM reports WHERE id=:id");
	q->bindValue(":id", id_report);
	if (!q->exec()) {
		qDebug() << q->lastError().text();
		goto close_db;
	}
	if (!q->next()) {
		qDebug() << "Invalid report ID!";
		goto close_db;
	}

	qApp->processEvents();
	ba = q->value(0).toByteArray();

// parsing data...
	buf.setBuffer(&ba);
	if (!buf.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(this, tr("Error"), tr("Unable to open buffer!\n"));
		goto close_db;
	}

	dev->load(&buf);
	while(dev->isLoading()) { msleep ( 1 << 5); qApp->processEvents(); }

	if (!dev->loadResult()) {
		QMessageBox::warning(this, tr("Warning"), tr("No tests data found in this file!"));
	} else {
		c_dev->setCurrentIndex(0);
#ifndef QT_NO_DEBUG
		gettimeofday(&te, NULL);
		double t = (te.tv_sec - tb.tv_sec) + (te.tv_usec - tb.tv_usec)/1000000.0;
		QMessageBox::information(this, tr("Info"), tr("Tests data loaded in %1 sec").arg(t,0,'f',2));
#endif
	}

close_db:
	if (q) delete q;
	delete progress;
}

void QPxToolMW::dragEnterEvent(QDragEnterEvent* e)
{
	qDebug() << "dragEnterEvent()";
	if (e->mimeData()->hasFormat("text/plain")) {
		QUrl url(e->mimeData()->text().simplified());
		if (url.path().endsWith(".qpx"))
			e->acceptProposedAction();
	}
}

void QPxToolMW::dropEvent(QDropEvent* e)
{
	qDebug() << "dropEvent()"; 
	QUrl url(e->mimeData()->text().simplified());
	load_results(url.path());
	e->acceptProposedAction();
}


