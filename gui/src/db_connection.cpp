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

#include <QMessageBox>
#include <QFile>
#include <QDateTime>
#include <QTextStream>

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

#include <db_connection.h>

DBParams::DBParams()
{
	driver = "QPSQL";
	host = "localhost";
	port = 5432;
	conn_name = "new connection";
};

DBParams::DBParams(QString idriver,
			QString ihost, 
			QString iname,
			QString iuser, 
			QString ipass,
			int iport
		)
{
	driver = idriver;
	host = ihost;
	name = iname;
	user = iuser;
	pass = ipass;
	port = iport;
	conn_name = "";
};

bool SqlProbeConnection(const DBParams& par, const QString& CONNECTION)
{
	QSqlDatabase Sql_DB;
    QString r;
    if (SqlOpenConnection(par, CONNECTION)) {
		SqlCloseConnection(CONNECTION);
		return 1;
    } else {
		return 0;
    }
}

bool SqlOpenConnection(const DBParams& par, const QString& CONNECTION)
{
#ifndef QT_NO_DEBUG
    qDebug("Connect...");
#endif
	QSqlDatabase db;
	if (CONNECTION.isEmpty()) {
		db = QSqlDatabase::addDatabase(par.driver);
	} else {
		db = QSqlDatabase::addDatabase(par.driver, CONNECTION);
	}
    db.setHostName( par.host );
    db.setDatabaseName( par.name );
    db.setUserName( par.user );
    db.setPassword( par.pass );
    db.setPort( par.port );
	if ( !db.open() ) {
#ifndef QT_NO_DEBUG
		qDebug() << "Failed to open database: (" << par.driver << ")" << par.name << "@" << par.host << ":";
		qDebug() << db.lastError().driverText();
		qDebug() << db.lastError().databaseText();
#endif
		QMessageBox::critical(0,
		    "Error",
		    db.lastError().text());
		return 0;
    }
    return 1;
}

void SqlCloseConnection(const QString& CONNECTION)
{
#ifndef QT_NO_DEBUG
    qDebug( "Disconnecting..." );
#endif
	{
		QSqlDatabase db;
		if (CONNECTION.isEmpty()) {
			db = QSqlDatabase::database();
		} else {
			db = QSqlDatabase::database(CONNECTION);
		}
		if (db.isOpen()) db.close();
	}
//    Sql_DB.removeDatabase(CONNECTION);
	if (!CONNECTION.isEmpty()) QSqlDatabase::removeDatabase(CONNECTION);
}

