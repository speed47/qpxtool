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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <QMutex>
#include <QMainWindow>
#include <QSqlDatabase>
#include <device.h>
#include <qpxsettings.h>

class QAction;
class QActionGroup;

class QWidget;
class QMenuBar;
class QToolBar;
class QHBoxLayout;
class QVBoxLayout;
//class QToolButton;
class QPushButton;

class QLabel;
class QProgressBar;
class QComboBox;
class QFrame;
class QProcess;
class QTcpSocket;
class QPxIODevice;

class QPxMainWidget;
class ProgressWidget;
class TestDialog;

class QTextDocument;

#ifndef QT_NO_DEBUG
class QMutex2 : public QMutex
{
public:
	inline void lock()
		{ qDebug(" * QMutex::lock()"); QMutex::lock(); } ;
	inline bool tryLock()
		{ qDebug(" * QMutex::trylock()"); return QMutex::tryLock(); } ;
	inline bool tryLock(int to)
		{ qDebug(" * QMutex::trylock(int)"); return QMutex::tryLock(to); } ;
	inline void unlock()
		{ qDebug(" * QMutex::unlock()"); QMutex::unlock(); } ;
};
#endif

class QPxToolMW : public QMainWindow
{
	Q_OBJECT

public:
	QPxToolMW(int ac, char **av, QWidget *p=0, Qt::WindowFlags fl = 0);
	~QPxToolMW();

public slots:
	void mediaUpdated(int);

public slots:
	void process_started();  // for statusBar
	void process_finished(); // updating
	void process_progress(); //

private slots:
	void setDevice(int idx);
	void loejToggle();
	void lockToggle();
	void selectTab();

	void select_tests();
	void select_tests(device *idev);
	void run_tests();
	void cancel_run_tests();
	void terminate_tests();


	void update_device_info();
	void update_media_info();
	void device_settings();
	void update_features_done(int);
	void exit();
	void scanbus();
	void qscan_process_scanbus();
	void preferences();

	void save_report();
	void print_results();
	void export_results();
	void save_results();
	void save_results_db(device *idev = NULL);
	void load_results();
	void load_results(QString);
	void load_results_db();

	void about();

	void tests_done();
	void tests_error();

protected:
	virtual void dragEnterEvent(QDragEnterEvent*);
	virtual void dropEvent(QDropEvent*);

private:
	ProgressWidget *progress;
	void winit();
	void create_actions();
	void winit_menubar();
	void winit_toolbar();
	void winit_statusbar();
	void settings_load();
	void settings_save();

	void scanbus_local();
	void scanbus_remote();
	void update_media_info(device* dev);
	void run_test(int);
//	void prepare_images(QString path=QString::null);
	void autosave_report(device* dev);
	void save_report(device* dev, QString fname);
	void prepare_report(device* dev, QTextDocument*);
	QString generate_html(device *dev, QString idir = QString());

//	infoTypeT		infoType;
	device::devtype dt;
	QString			dhost;
	int				dport;
	QPxIODevice		*scanbusio;
#ifndef QT_NO_DEBUG
	QMutex2	 mutex_dev;
#else
	QMutex	 mutex_dev;
#endif
//	bool	 scanbusDis;

	QMenuBar *menubar;

	QToolBar *toolbar;
	QAction		*act_exit,
				*act_pref,
				*act_save,
				*act_save_db,
				*act_load,
				*act_load_db,
				*act_export,
				*act_report,
				*act_print,
				*act_scanbus,
				*act_minfo,
				*act_devctl,
				*act_test,
				*act_stop,
				*act_about;

	QAction		    *act_sb;
	QList<QAction*> act_sblist;
	QActionGroup    *act_sbgrp;

	QWidget		*cwidget;

	QVBoxLayout *layout;
	QHBoxLayout *layout_dev;
	QLabel		*l_dev;
	QComboBox	*c_dev;
	QPushButton	*pb_loej;
	QPushButton	*pb_lock;
//	QFrame		*hline;
	QPxMainWidget *mwidget;

// statusbar widgets
	QProgressBar *status_progress;
	QLabel		 *status_process,
				 *status_media,
				 *status_mid;

	TestDialog	*testDialog;

	devlist			devices; // devices
//	QSqlDatabase	db;		 // reports database connection
// settings
	QPxSettings		set;
	bool			splash;
};

#endif

