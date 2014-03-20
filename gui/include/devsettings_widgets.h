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

#ifndef _DEVSETTINGS_WIDGETS_H
#define _DEVSETTINGS_WIDGETS_H

#include <QWidget>
#include <QGroupBox>
#include <QDialog>

class QBoxLayout;
class QGridLayout;

class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QPushButton;
class QRadioButton;
class QTreeWidget;
class QTreeWidgetItem;
class QButtonGroup;
class QSlider;

class TextSlider;
class device;

class devctlAutostrategy;
class TattooWidget;

class devctlCommon : public QGroupBox
{
	Q_OBJECT

public:
	devctlCommon(device* idev, QWidget* p=NULL);
	~devctlCommon() {};

private slots:
	void set_prec(bool);
	void set_hcdr(bool);
	void set_ss(bool);
	void set_spdrd(bool);
	void set_bitset(bool);
	void set_bitset_dl(bool);
	void set_simulplus(bool);
private:
	device* dev;

	QGridLayout *layout;
	QCheckBox	*ck_prec;
	QLabel		*pl_prec;
	QLabel		*l_prec;

	QCheckBox	*ck_hcdr,
				*ck_ss,
				*ck_spdrd,
				*ck_bitset,
				*ck_bitset_dl,
				*ck_simulplus;

	devctlAutostrategy *as;
};

// GigaRec control

class devctlGigarec : public QGroupBox
{
	Q_OBJECT
public:
	devctlGigarec(device* idev, QWidget* p=NULL);
	~devctlGigarec() {};

private slots:
	void set(bool);
	void set();

private:
	device* dev;
	QBoxLayout  *layout;
	TextSlider	*grec_ratio;
	QLabel		*l_cap;
	QLabel		*l_ncap;
	QLabel		*l_hint;
};

// VariRec control

class devctlVarirecBase : public QGroupBox
{
	Q_OBJECT

public:
	devctlVarirecBase(device* idev, QString title, QWidget* p=NULL);
	~devctlVarirecBase() {};

protected:
	device* dev;
	QBoxLayout  *layout;
	QLabel		*pl_pwr;
	TextSlider	*pwr;
	QBoxLayout	*layouts;
	QLabel		*pl_str;
	QComboBox	*str;
	QLabel		*l_hint;
};

// VariRec control (CD)

class devctlVarirecCD : public devctlVarirecBase
{
	Q_OBJECT
public:
	devctlVarirecCD(device* idev, QWidget* p=NULL);
	~devctlVarirecCD() {};

private slots:
	void set(bool);
	void set();
};

// VariRec control (DVD)

class devctlVarirecDVD : public devctlVarirecBase
{
	Q_OBJECT
public:
	devctlVarirecDVD(device* idev, QWidget* p=NULL);
	~devctlVarirecDVD() {};

private slots:
	void set(bool);
	void set();
};

class devctlVarirec: public QWidget
{
	Q_OBJECT
public:
	devctlVarirec(device* idev, QWidget* p=NULL);
	~devctlVarirec() {};

private:
	QBoxLayout		 *layout;
	devctlVarirecCD  *vrec_cd;
	devctlVarirecDVD *vrec_dvd;
};

// SecuRec control

class devctlSecurec : public QGroupBox
{
	Q_OBJECT
public:
	devctlSecurec(device* idev, QWidget* p=NULL);
	~devctlSecurec() {};

private slots:
	void set();
	void reset();
	int  validate();

private:
	device* dev;
	QGridLayout  *layout;

	QLabel		*l_pwd[2];
	QLineEdit	*e_pwd[2];
	QFrame		*hline0;
	QPushButton *pb_set;
	QLabel		*l_active;
	QPushButton *pb_reset;
	QLabel		*l_hint;
};

// Silent Mode control

class devctlSilent : public QWidget
{
	Q_OBJECT
public:
	devctlSilent(device* idev, QWidget* p=NULL);
	~devctlSilent() {};

private slots:
	void set();

private:
	device* dev;
	QBoxLayout  *layoutm;
	QGroupBox	*cw;
	QGridLayout	*layout;

	QLabel		*h_speed,
				*h_access,
				*h_tray;
// R/W Speeds
	QLabel		*spdr, *spdw, *spdc, *spdd;
	QComboBox	*box_rcd,
				*box_wcd,
				*box_rdvd,
				*box_wdvd;
// Access time
	QButtonGroup *grp_access;
	QRadioButton *a_slow,
				 *a_fast;
// Tray speed
	QLabel		*l_load, *l_eject;
	QSlider		*s_load, *s_eject;

	QBoxLayout  *layoutb;
	QCheckBox	*ck_perm;
	QPushButton *pb_save;
};

// AutoStrategy control

class devctlAutostrategy : public QWidget
{
	Q_OBJECT
public:
	devctlAutostrategy(device* idev, QWidget* p=NULL);
	~devctlAutostrategy() {};
private slots:
	void asdb();
	void set_mode(int);
	void run_mqck();

private:
	device* dev;
	QGridLayout		*layout;

// AS mode
	QGroupBox		*box_as;
	QGridLayout		*layout_as;
	QButtonGroup	*grp_mode;
	QRadioButton	*rb_as_on,
					*rb_as_off,
					*rb_as_auto,
					*rb_as_forced;
	QPushButton		*pb_asdb;
// MQCK
	QGroupBox		*box_mqck;
	QGridLayout		*layout_mqck;
	QButtonGroup	*grp_mqck;

	QRadioButton	*rb_mqck_q,
					*rb_mqck_a;
	QLabel			*lmqck_spd;
	QComboBox		*bmqck_spd;
	QPushButton		*pb_mqck;
	QLabel			*lmqck_res;
};

// AutoStrategy DB control

class devctlAutostrategyDB : public QDialog
{
	Q_OBJECT
public:
	devctlAutostrategyDB(device* idev, QWidget* p=NULL);
	~devctlAutostrategyDB() {};

public slots:
	void setVisible(bool);

private slots:
	void db_toggle(QTreeWidgetItem*);
	void db_act();
	void db_act(int);
	void db_deact();
	void db_deact(int);
	void db_del();
	void db_clear();

	void run_ascre();
	void db_update(int idx=-1);
	void db_update_done(bool);

private:

	device* dev;

	QBoxLayout		*layout;
	QBoxLayout		*layoutb;

	QTreeWidget		*tree;
	QPushButton		*pb_act,
					*pb_deact,
					*pb_del,
					*pb_clear;

// Strategy creation
	QGroupBox		*box_ascre;
	QGridLayout		*layout_ascre;
	QGroupBox		*grp_ascre_mode,
					*grp_ascre_act;
	QBoxLayout		*layout_crem,
					*layout_crea;
	QRadioButton	*rb_ascre_quick,
					*rb_ascre_full,
					*rb_ascre_add,
					*rb_ascre_replace;
	QPushButton		*ascre_start;


};

// PlexEraser control

class devctlDestruct : public QGroupBox
{
	Q_OBJECT
public:
	devctlDestruct(device* idev, QWidget* p=NULL);
	~devctlDestruct() {};
private slots:
	void start();
private:
	device* dev;
	QGridLayout  *layout;

	QRadioButton *rb_quick;
	QLabel		 *pl_quick;
	QRadioButton *rb_full;
	QLabel		 *pl_full;

	QPushButton  *pb_start;
	QLabel		 *l_hint;
};


// Pioneer QuietMode

class devctlPioquiet : public QGroupBox
{
	Q_OBJECT
public:
	devctlPioquiet(device* idev, QWidget* p=NULL);
	~devctlPioquiet() {};

private slots:
	void set();

private:
	device* dev;
	QGridLayout  *layout;

	QGroupBox	 *box_mode;
	QBoxLayout	 *layoutm;
	QRadioButton *rb_quiet;
	QRadioButton *rb_std;
	QRadioButton *rb_perf;

	QCheckBox	 *ck_limit;

	QCheckBox	 *ck_perm;
	QPushButton	 *pb_set;
};

// Yamaha CRW-F1 Disc T@2

class devctlF1Tattoo : public QGroupBox
{
	Q_OBJECT
public:
	devctlF1Tattoo(device* idev, QWidget* p=NULL);
	~devctlF1Tattoo() {};

private slots:
	void loadImage();
	void burn();

private:
	device*	dev;
	int		r0,r1;
	QImage	srcimg, img;

	QGridLayout		*layout;
	TattooWidget	*tw;
	QBoxLayout		*layoutb;
	QLabel			*l_file;
	QPushButton		*pb_load;
	QPushButton		*pb_burn;
};

#endif

