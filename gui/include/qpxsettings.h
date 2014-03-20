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

#ifndef _QPXSETTINGS_H
#define _QPXSETTINGS_H

#include <QString>
#include <QStringList>
#include <QColor>
#include <QRect>

#include <inttypes.h>

#include <db_connection.h>

struct colorSet {
	QString		name;
	QColor		bg,grid,rspeed,wspeed,
				errc[8],
				jitter,asymm,
				fe,te,
				tapit,taland;
};

static const colorSet defColors = {
	"Default",
	0xFFFFFF,
	0x808080,
	0x009000,
	0xC00000,
	{	
		0x009000,
		0x00C000,
		0x00FF00,
		0xFFAA00,
		0x0055FF,
		0x550080,
		0xFF0000,
		Qt::black
	},
	0x00A000,
	0xA00000,
	0xA00000,
	0x0000A0,
	0xA00000,
	0x0000A0
};

class Scale {
public:
	Scale(QString iname);
	~Scale();

	enum ScalePolicy {
		Auto	= 1,
		Fixed	= 2
	};

	enum ScaleType {
		Linear	= 4,
		Log		= 8
	};

	QString		name;
//	ScaleType	type;
//	ScaleMode	mode;
	int			policy;
	int			type;
	int			value;

	inline bool operator== (const QString &s)  { return name == s; };
	inline bool operator== (const Scale &o)    { return name == o.name; };
};

class ScaleList : public QList<Scale> {
	public:
		inline Scale& get(const QString s) {
			int idx = indexOf(s);
			if (idx>=0)
				return (*this)[idx];
			append(Scale(s));
			return (*this)[indexOf(s)];
		};
};

struct ErrcColorCD{
	QColor *bler;
	QColor *e11;
	QColor *e21;
	QColor *e31;
	QColor *e12;
	QColor *e22;
	QColor *e32;
	QColor *uncr;
};

struct ErrcColorDVD{
	QColor *res0;
	QColor *pie;
	QColor *pi8;
	QColor *pif;
	QColor *poe;
	QColor *po8;
	QColor *pof;
	QColor *uncr;
};

struct ErrcColorBD{
	QColor *res0;
	QColor *ldc;
	QColor *res1;
	QColor *res2;
	QColor *bis;
	QColor *res3;
	QColor *res4;
	QColor *uncr;
};

union ErrcColor{
	QColor *raw[8];
	ErrcColorCD cd;
	ErrcColorDVD dvd;
	ErrcColorBD bd;
};

#define AFLAG_MWATCH			0x0001
#define AFLAG_AUTOSTART_W		0x0002
#define AFLAG_AUTOSTART_B		0x0004
#define AFLAG_EJECT_AFTER		0x0008 // +
#define AFLAG_DTEST_RT			0x0010
#define AFLAG_DTEST_WT			0x0020
#define AFLAG_DTEST_WT_SIMUL	0x0040
#define AFLAG_DTEST_ERRC		0x0080
#define AFLAG_DTEST_JB			0x0100
#define AFLAG_DTEST_FT_W		0x0200
#define AFLAG_DTEST_FT_B		0x0400
#define AFLAG_DTEST_TA			0x0800

class QPxSettings {
public:
	QPxSettings();
	QPxSettings(const QPxSettings&);
	~QPxSettings();
	void load();
	void save();
	void loadScale(QString name);
	void saveScale();

	void setDefaultColors();
	QPxSettings& operator = (const QPxSettings& o);

// geometry
	QRect		geometry_mw;
	QRect		geometry_pref;
//	QRect		geometry_testsel;
	int			tests;

// general options
	bool		show_sidebar;
	bool		show_allctl;
	bool		report_autosave;
	QString		report_path;
	uint32_t	actions_flags;

	bool		use_reports_db;
	bool		report_autosave_db;
	DBParams	db;

// device settings
	bool		useLocal;
	bool		useRemote;
	QStringList	hosts;
	QStringList ports;

// graph settings
	QColor		col_bg;
	QColor		col_bginv;
	QColor		col_grid;
	QColor		col_rspeed;
	QColor		col_wspeed;

	ErrcColor	col_errc;

	QColor		col_jitter;
	QColor		col_asymm;

	QColor		col_fe;
	QColor		col_te;

	QColor		col_tapit;
	QColor		col_taland;

	ScaleList	scales;

// paths
	QString		last_res_path_native,
				last_res_path_html,
				last_res_path_pdf;
};

#endif

