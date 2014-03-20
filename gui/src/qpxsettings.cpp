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

#include <QSettings>
#include "qpxsettings.h"

#include <QDebug>

Scale::Scale(QString iname)
{
	name=iname;
	policy = Auto;
	type = Linear;
	value = 256;
}

Scale::~Scale()
{
}

QPxSettings::QPxSettings()
{
#ifndef QT_NO_DEBUG
	qDebug() << "QPxSettings() " << this;
#endif
	geometry_mw = QRect(0,0,0,0);
	geometry_pref = QRect(0,0,0,0);
//	geometry_testsel = QRect(0,0,0,0);

	show_sidebar  = 0;
	show_allctl   = 0;
	actions_flags = 0;

	useLocal  = 1;
	useRemote = 0;
	hosts.clear();
	ports.clear();

	for (int i=0; i<8; i++)	{
		col_errc.raw[i] = new QColor();
#ifndef QT_NO_DEBUG
//		qDebug(QString("new QColor %1").arg((unsigned int)col_errc.raw[i],8,16));
#endif
	}
}

QPxSettings::QPxSettings(const QPxSettings& o)
{
#ifndef QT_NO_DEBUG
	qDebug() << "QPxSettings(const QPxSettings&) " << this;
#endif
	for (int i=0; i<8; i++)	{ 
		col_errc.raw[i] = new QColor(*(o.col_errc.raw[i]));
#ifndef QT_NO_DEBUG
//		qDebug(QString("new QColor %1").arg((unsigned int)col_errc.raw[i],8,16));
#endif
	}
	*this = o;
}

QPxSettings& QPxSettings::operator = (const QPxSettings& o)
{
#ifndef QT_NO_DEBUG
	qDebug() << "QPxSettings() " << this << " = " << &o;
#endif
	geometry_mw   = o.geometry_mw;
	geometry_pref = o.geometry_pref;

	show_sidebar    = o.show_sidebar;
	show_allctl     = o.show_allctl;
	report_autosave = o.report_autosave;
	report_path     = o.report_path;
	actions_flags   = o.actions_flags;

	use_reports_db	   = o.use_reports_db;
	report_autosave_db = o.report_autosave_db;
	db = o.db;

	useLocal  = o.useLocal;
	useRemote = o.useRemote;
	hosts = o.hosts;
	ports = o.ports;

	tests		= o.tests;

	col_bg		= o.col_bg;
	col_bginv	= o.col_bginv;
	col_grid	= o.col_grid;	
	col_rspeed	= o.col_rspeed;
	col_wspeed	= o.col_wspeed;

	for (int i=0; i<8; i++)	*col_errc.raw[i] = *o.col_errc.raw[i];

	col_jitter	= o.col_jitter;
	col_asymm	= o.col_asymm;
	col_fe		= o.col_fe;
	col_te		= o.col_te;
	col_tapit	= o.col_tapit;
	col_taland	= o.col_taland;

	scales		= o.scales;
	return *this;
}

QPxSettings::~QPxSettings()
{
#ifndef QT_NO_DEBUG
	qDebug() << "~QPxSettings() " << this;
#endif
	for (int i=0; i<8; i++) {
#ifndef QT_NO_DEBUG
//		qDebug(QString("del QColor %1").arg((unsigned int)col_errc.raw[i],8,16));
#endif
		delete col_errc.raw[i];
	}
}

void QPxSettings::load()
{
	QSettings *settings;
#ifndef QT_NO_DEBUG
	qDebug("Loading settings...");
#endif

//	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "qpxtool");
	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "QPxTool", "qpxtool");
	settings->beginGroup("/common");
		show_sidebar     = settings->value("show_sidebar", 0).toBool();
		show_allctl      = settings->value("show_allctl", 0).toBool();
		report_autosave  = settings->value("report_autosave", 0).toBool();
		report_path      = settings->value("report_path", "").toString();
		actions_flags    = settings->value("actions_flags", "0").toUInt();
	settings->endGroup();
	settings->beginGroup("/database");
		use_reports_db  = settings->value("use_reports_db", 0).toBool();
		report_autosave_db = settings->value("report_autosave_db", 0).toBool();
		db.driver		= settings->value("db_driver", "").toString();
		db.host			= settings->value("db_host", "").toString();
		db.port			= settings->value("db_port", "").toInt();
		db.name			= settings->value("db_name", "").toString();
		db.user			= settings->value("db_user", "").toString();
		db.pass			= settings->value("db_pass", "").toString();
	settings->endGroup();

	settings->beginGroup("/geometry");
		geometry_mw      = settings->value("mainwindow",  QRect(0,0,0,0) ).toRect();
		geometry_pref    = settings->value("preferences", QRect(0,0,0,0) ).toRect();
//		geometry_testsel = settings->value("testselector",  QRect(0,0,0,0) ).toRect();
	settings->endGroup();

	settings->beginGroup("/devices");
		useLocal  = settings->value("local", 1).toBool();
		useRemote = settings->value("remote", 0).toBool();
		hosts     = settings->value("hosts", QStringList()).toStringList();
		ports     = settings->value("ports", QStringList()).toStringList();
		hosts.removeAll("");
		tests	  = settings->value("tests", 0).toInt();
	settings->endGroup();

	settings->beginGroup("/colors");
		col_bg     = settings->value("graph_bg",   defColors.bg.rgb()).toInt();
		col_bginv  = QColor( (~col_bg.red()) & 0xFF, (~col_bg.green()) & 0xFF, (~col_bg.blue()) & 0xFF );
		col_grid   = settings->value("graph_grid", defColors.grid.rgb()).toInt();
	
		col_rspeed = settings->value("rspeed",   defColors.rspeed.rgb()).toInt();
		col_wspeed = settings->value("wspeed",   defColors.wspeed.rgb()).toInt();

		for(int i=0; i<8; i++)
			*col_errc.raw[i] = settings->value(QString("errc%1").arg(i), defColors.errc[i].rgb()).toInt();

		col_jitter = settings->value("jitter",	 defColors.jitter.rgb()).toInt();
		col_asymm  = settings->value("asymm",	 defColors.asymm.rgb()).toInt();

		col_fe	   = settings->value("fe",	 defColors.fe.rgb()).toInt();
		col_te	   = settings->value("te",	 defColors.te.rgb()).toInt();

		col_tapit  = settings->value("ta_pit",	 defColors.tapit.rgb()).toInt();
		col_taland = settings->value("ta_land",	 defColors.taland.rgb()).toInt();
	settings->endGroup();
	settings->beginGroup("/paths");
		last_res_path_native = settings->value("last_res_native","").toString();
		last_res_path_html   = settings->value("last_res_html","").toString();
		last_res_path_pdf    = settings->value("last_res_pdf","").toString();
	settings->endGroup();
	delete settings;
}

void QPxSettings::save()
{
	QSettings *settings;
#ifndef QT_NO_DEBUG
	qDebug("Saving settings...");
#endif
//	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "qpxtool");
	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "QPxTool", "qpxtool");
	settings->beginGroup("/common");
		settings->setValue("show_sidebar", show_sidebar);
		settings->setValue("show_allctl", show_allctl);
		settings->setValue("report_autosave", report_autosave);
		settings->setValue("report_path", report_path);
		settings->setValue("actions_flags", actions_flags);
	settings->endGroup();

	settings->beginGroup("/database");
		settings->setValue("use_reports_db", use_reports_db);
		settings->setValue("report_autosave_db", report_autosave_db);
		settings->setValue("db_driver", db.driver);
		settings->setValue("db_host", db.host);
		settings->setValue("db_port", db.port);
		settings->setValue("db_name", db.name);
		settings->setValue("db_user", db.user);
		settings->setValue("db_pass", db.pass);
	settings->endGroup();

	settings->beginGroup("/geometry");
		settings->setValue("mainwindow",  geometry_mw);
		settings->setValue("preferences", geometry_pref);
//		settings->setValue("testselector", geometry_testsel);
	settings->endGroup();

	settings->beginGroup("/devices");
		settings->setValue("local",  useLocal);
		settings->setValue("remote", useRemote);
		settings->setValue("hosts",  hosts);
		settings->setValue("ports",  ports);
		settings->setValue("tests", tests);
	settings->endGroup();

	settings->beginGroup("/colors");
		settings->setValue("graph_bg",   col_bg.rgb());
		settings->setValue("graph_grid", col_grid.rgb());

		settings->setValue("rspeed",  	col_rspeed.rgb());
		settings->setValue("wspeed",  	col_wspeed.rgb());

		for(int i=0; i<8; i++)
			settings->setValue(QString("errc%1").arg(i), col_errc.raw[i]->rgb());

		settings->setValue("jitter",	col_jitter.rgb());
		settings->setValue("asymm",		col_asymm.rgb());

		settings->setValue("fe",		col_fe.rgb());
		settings->setValue("te",		col_te.rgb());

		settings->setValue("ta_pit",	col_tapit.rgb());
		settings->setValue("ta_land",	col_taland.rgb());

	settings->endGroup();

	settings->beginGroup("/paths");
		settings->setValue("last_res_native", last_res_path_native);
		settings->setValue("last_res_html", last_res_path_html);
		settings->setValue("last_res_pdf", last_res_path_pdf);
	settings->endGroup();

	delete settings;
	saveScale();
}

void QPxSettings::loadScale(QString name)
{
	QSettings *settings;
//	Scale scale(name);

#ifndef QT_NO_DEBUG
	qDebug() << "Loading scale: " << name;
#endif
//	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "qpxtool");
	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "QPxTool", "qpxtool");

	settings->beginGroup("/scale");
		settings->beginGroup(name);
			scales.get(name).policy = settings->value("policy", Scale::Auto).toInt();
			scales.get(name).type   = settings->value("type",   Scale::Log).toInt();
			scales.get(name).value  = settings->value("value", 0).toInt();
		settings->endGroup();
	settings->endGroup();

//	scales.removeAll(name);
//	scales.append(scale);

	delete settings;
}

void QPxSettings::saveScale()
{
	QSettings *settings;
#ifndef QT_NO_DEBUG
	qDebug("Saving scale...");
#endif
	settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "QPxTool", "qpxtool");

	settings->beginGroup("/scale");
	for (int i=0; i<scales.size(); i++) {
#ifndef QT_NO_DEBUG
		qDebug() << scales[i].name;
#endif
		settings->beginGroup(scales[i].name);
			settings->setValue("policy", scales[i].policy);
			settings->setValue("type",   scales[i].type);
			settings->setValue("value",  scales[i].value);
		settings->endGroup();
	}
	settings->endGroup();

	delete settings;
}

void QPxSettings::setDefaultColors()
{
	col_bg     = defColors.bg;
	col_bginv  = QColor( (~col_bg.red()) & 0xFF, (~col_bg.green()) & 0xFF, (~col_bg.blue()) & 0xFF );
	col_grid   = defColors.grid;
	
	col_rspeed = defColors.rspeed;
	col_wspeed = defColors.wspeed;

	for(int i=0; i<8; i++) *col_errc.raw[i] = defColors.errc[i];

	col_jitter = defColors.jitter;
	col_asymm  = defColors.asymm;

	col_fe	   = defColors.fe;
	col_te	   = defColors.te;

	col_tapit  = defColors.tapit;
	col_taland = defColors.taland;
}

#if 0
QPxSettings& QPxSettings::operator = (const QPxSettings& o) {
	hosts = o.hosts;
	wgeometry = o.wgeometry;
	pgeometry = o.pgeometry;

	col_bg   = o.col_bg;
	col_grid = o.col_grid;

	scales   = o.scales;
	return (*this);
}
#endif

