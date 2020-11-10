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

#ifndef _DB_CONNECTION_H
#define _DB_CONNECTION_H

class QSqlDatabase;
class QString;

class DBParams {
public:
	DBParams();
	DBParams(QString driver, 
			QString host, 
			QString name, 
			QString user, 
			QString pass,
			int port = 5432
	);
//	DBParams(const DBParams& p);
	~DBParams() {};

    bool	params_set;
	QString	driver;
    QString	host;
    int		port;
    QString	name;
    QString	user;
    QString	pass;

	QString conn_name;
};

bool SqlProbeConnection(const DBParams& par, const QString& CONNECTION="test connection");
bool SqlOpenConnection (const DBParams& par, const QString& CONNECTION="test connection");
void SqlCloseConnection(const QString& CONNECTION="test connection");

#endif // _DB_CONNECTION_H

