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

#ifndef _TAB_FETE_H
#define _TAB_FETE_H

class ColorLabel;
class QLabel;
class QBoxLayout;

#include <graphtab.h>

class tabFETE : public GraphTab
{
	Q_OBJECT
public:
	tabFETE(QPxSettings *iset, devlist *idev, QString iname, QWidget *p=0, Qt::WindowFlags fl = 0);
	~tabFETE();

public slots:
//	void clear();
	void selectDevice();
	void updateLast();
	void updateAll();
	void updateSummary();
//	void setDevice(device *idev);
//	void update() { graph->update(); };

protected:
	virtual void updateLegend();
	virtual void updateGraph();

private:
	QBoxLayout	*layout_info;

#ifdef __LEGEND_SHOW_SPEED
	ColorLabel	*pl_spd;
#endif
	ColorLabel	*pl_fmax;
	QLabel		*l_fmax;
	ColorLabel	*pl_tmax;
	QLabel		*l_tmax;
};

#endif

