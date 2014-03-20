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


#include <QWidget>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTreeWidget>

#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>

#include <unistd.h>
#include <textslider.h>
#include <progresswidget.h>
#include <tattoowidget.h>
#include <device.h>
#include <defs.h>
#include "devsettings_widgets.h"

#include <qpx_mmc_defs.h>

#include <QApplication>
#include <QDebug>

devctlCommon::devctlCommon(device* idev, QWidget* p)
	:QGroupBox(p)
{
	dev = idev;
	layout = new QGridLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);

	ck_prec = new QCheckBox("PoweRec",this);
	layout->addWidget(ck_prec, 0,0);
	pl_prec = new QLabel("Speed:",this);
	pl_prec->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(pl_prec,0,1);
	l_prec = new QLabel(this);
	l_prec->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
	layout->addWidget(l_prec,0,2);

	ck_hcdr  = new QCheckBox("Hide CD-R",this);
	layout->addWidget(ck_hcdr, 1,0);
	ck_ss    = new QCheckBox("SingleSession",this);
	layout->addWidget(ck_ss,2,0);
	ck_spdrd = new QCheckBox("SpeedRead",this);
	layout->addWidget(ck_spdrd,3,0);


	ck_bitset = new QCheckBox("Change BookType on DVD+R",this);
	layout->addWidget(ck_bitset,4,0,1,3);
	ck_bitset_dl = new QCheckBox("Change BookType on DVD+R DL",this);
	layout->addWidget(ck_bitset_dl,5,0,1,3);
	ck_simulplus = new QCheckBox("Testwrite on DVD+R(W)",this);
	layout->addWidget(ck_simulplus,6,0,1,3);

	as = new devctlAutostrategy(dev,this);
	layout->addWidget(as,8,0,1,3);
	layout->setRowStretch(7,10);
	layout->setRowStretch(8,6);

	ck_prec->setEnabled(dev->features.supported & FEATURE_POWEREC);
	if (dev->features.supported & FEATURE_POWEREC)
		l_prec->setText(QString::number(dev->features.prec_spd)+"X");
	ck_hcdr->setEnabled(dev->features.supported & FEATURE_HIDECDR);
	ck_ss->setEnabled(dev->features.supported & FEATURE_SINGLESESSION);
	ck_spdrd->setEnabled(dev->features.supported & FEATURE_SPEEDREAD);
	ck_bitset->setEnabled(dev->features.supported & FEATURE_BITSETR);
	ck_bitset_dl->setEnabled(dev->features.supported & FEATURE_BITSETRDL);
	ck_simulplus->setEnabled(dev->features.supported & FEATURE_SIMULPLUS);

	ck_prec->setChecked(dev->features.enabled & FEATURE_POWEREC);
	ck_hcdr->setChecked(dev->features.enabled & FEATURE_HIDECDR);
	ck_ss->setChecked(dev->features.enabled & FEATURE_SINGLESESSION);
	ck_spdrd->setChecked(dev->features.enabled & FEATURE_SPEEDREAD);
	ck_bitset->setChecked(dev->features.enabled & FEATURE_BITSETR);
	ck_bitset_dl->setChecked(dev->features.enabled & FEATURE_BITSETRDL);
	ck_simulplus->setChecked(dev->features.enabled & FEATURE_SIMULPLUS);

	connect(ck_prec,  SIGNAL(toggled(bool)), this, SLOT(set_prec(bool)));
	connect(ck_hcdr,  SIGNAL(toggled(bool)), this, SLOT(set_hcdr(bool)));
	connect(ck_ss,    SIGNAL(toggled(bool)), this, SLOT(set_ss(bool)));
	connect(ck_spdrd, SIGNAL(toggled(bool)), this, SLOT(set_spdrd(bool)));
	connect(ck_bitset,    SIGNAL(toggled(bool)), this, SLOT(set_bitset(bool)));
	connect(ck_bitset_dl, SIGNAL(toggled(bool)), this, SLOT(set_bitset_dl(bool)));
	connect(ck_simulplus, SIGNAL(toggled(bool)), this, SLOT(set_simulplus(bool)));


}

void devctlCommon::set_prec(bool en)  { dev->setFeature(FEATURE_POWEREC, en); }
void devctlCommon::set_hcdr(bool en)  { dev->setFeature(FEATURE_HIDECDR, en); }
void devctlCommon::set_ss(bool en)    { dev->setFeature(FEATURE_SINGLESESSION, en); }
void devctlCommon::set_spdrd(bool en) { dev->setFeature(FEATURE_SPEEDREAD, en); }
void devctlCommon::set_bitset(bool en){ dev->setFeature(FEATURE_BITSETR, en); }
void devctlCommon::set_bitset_dl(bool en) { dev->setFeature(FEATURE_BITSETRDL, en); }
void devctlCommon::set_simulplus(bool en) {	dev->setFeature(FEATURE_SIMULPLUS, en); }

// GigaRec control

devctlGigarec::devctlGigarec(device* idev, QWidget* p)
	:QGroupBox("GigaRec",p)
{
	QString gc;

	dev = idev;
	layout = new QVBoxLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);

	setCheckable(true);
	bool ratio_14 = dev->dev.startsWith("CD-R   PREMIUM");
	bool ratio_11 = dev->dev.startsWith("DVDR   PX-755") || dev->dev.startsWith("DVDR   PX-760") ||	dev->dev.startsWith("CD-R   PREMIUM2");

	grec_ratio = new TextSlider(Qt::Vertical,this);
	grec_ratio->setTickPosition(QSlider::TicksBelow);
	grec_ratio->setUpsideDown(1);

	gc = QString::number((int)(dev->media.ctots*0.6) >> 9) + " MB";
	grec_ratio->addItem("0.6",ratio_14);
	gc = QString::number((int)(dev->media.ctots*0.7) >> 9) + " MB";
	grec_ratio->addItem("0.7",1);
	gc = QString::number((int)(dev->media.ctots*0.8) >> 9) + " MB";
	grec_ratio->addItem("0.8",1);
	gc = QString::number((int)(dev->media.ctots*0.9) >> 9) + " MB";
	grec_ratio->addItem("0.9",ratio_11);
	//grec_ratio->addItem("OFF",1);
	grec_ratio->addItem("",0);
	gc = QString::number((int)(dev->media.ctots*1.1) >> 9) + " MB";
	grec_ratio->addItem("1.1",ratio_11);
	gc = QString::number((int)(dev->media.ctots*1.2) >> 9) + " MB";
	grec_ratio->addItem("1.2",1);
	gc = QString::number((int)(dev->media.ctots*1.3) >> 9) + " MB";
	grec_ratio->addItem("1.3",1);
	gc = QString::number((int)(dev->media.ctots*1.4) >> 9) + " MB";
	grec_ratio->addItem("1.4",ratio_14);
	grec_ratio->setValue(4);

	layout->addWidget(grec_ratio);

	l_cap = new QLabel(this);
	l_cap->setMinimumHeight(24);
//	l_cap->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout->addWidget(l_cap);
	l_ncap = new QLabel(this);
	l_ncap->setMinimumHeight(24);
//	l_ncap->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout->addWidget(l_ncap);

	l_hint = new QLabel(
			 tr("GigaRec limits maximum write speed to 8X\n"
				"only DAO write mode can be used with GigaRec"), this);
	layout->addWidget(l_hint,10);

	//setEnabled(dev->features.supported & FEATURE_GIGAREC && dev->media.dstate == "Blank");
	setEnabled(dev->features.supported & FEATURE_GIGAREC);
	setChecked(dev->features.enabled & FEATURE_GIGAREC);

	grec_ratio->setCurrentItem(QString("%1").arg(dev->features.grec,0,'f',1));
	l_cap->setText(tr("Original capacity: ") + QString::number(dev->media.ctots >> 9) + " MB");
	l_ncap->setText(tr("GigaRec capacity: ") + QString::number((int)(dev->media.ctots * dev->features.grec) >> 9) + " MB");
	//grec_ratio->setCurrentItem("1.2");
#ifndef QT_NO_DEBUG
	qDebug() << QString("GigaRec : %1").arg(dev->features.grec,0,'f',1);
#endif
	connect(this, SIGNAL(toggled(bool)), this, SLOT(set(bool)));
	connect(grec_ratio, SIGNAL(valueChanged(int)), this, SLOT(set()));
}

void devctlGigarec::set() { set(isEnabled()); }

void devctlGigarec::set(bool en)
{
	QStringList sl;
	if (en) {
		dev->features.enabled |= FEATURE_GIGAREC;
		sl = grec_ratio->text().split(' ');
		if (sl.size() > 0)
			dev->features.grec = sl[0].toFloat();
		else
			qDebug("empty GigaRec value!");
	} else {
		dev->features.enabled &= ~FEATURE_GIGAREC;
	}
	if (!dev->setComplexFeature(FEATURE_GIGAREC, &dev->features)) {
//		l_cap->setText(tr("Original capacity: ") + QString::number(dev->media.ctots >> 9) + " MB");
		l_ncap->setText(tr("GigaRec capacity: ") + QString::number((int)(dev->media.ctots * dev->features.grec) >> 9) + " MB");
	}
}

// VariRec control

devctlVarirecBase::devctlVarirecBase(device* idev, QString title, QWidget* p)
	:QGroupBox(title, p)
{
	dev = idev;
	layout = new QVBoxLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);

	setCheckable(true);

	pl_pwr = new QLabel("Power:", this);
	layout->addWidget(pl_pwr);

	pwr = new TextSlider(Qt::Horizontal,this);
	pwr->setTickPosition(QSlider::TicksBelow);
	pwr->setUpsideDown(0);
	pwr->addItem("-4",1);
	pwr->addItem("-3",1);
	pwr->addItem("-2",1);
	pwr->addItem("-1",1);
	pwr->addItem("0",1);
	pwr->addItem("+1",1);
	pwr->addItem("+2",1);
	pwr->addItem("+3",1);
	pwr->addItem("+4",1);
	pwr->setValue(4);
	layout->addWidget(pwr);

	layouts = new QHBoxLayout();
	layouts->setMargin(3);
	layouts->setSpacing(3);
	layout->addLayout(layouts);

	pl_str = new QLabel("Strategy:", this);
	layouts->addWidget(pl_str,1);
	str = new QComboBox(this);
	layouts->addWidget(str,10);

	l_hint = new QLabel(this);
	layout->addWidget(l_hint,10);
}

devctlVarirecCD::devctlVarirecCD(device* idev, QWidget* p)
	:devctlVarirecBase(idev, "Varirec CD", p)
{
	bool ext = dev->features.supported & FEATURE_VARIREC_CDEXT;
	if (ext)
		l_hint->setText(tr("VariRec limits maximum CD write speed to 8X"));
	else
		l_hint->setText(tr("VariRec limits maximum CD write speed to 4X"));

	str->addItem("default");
	str->addItem("AZO");
	str->addItem("Cyanine");
	str->addItem("Phtalocyanine A");
	str->addItem("Phtalocyanine B");
	str->addItem("Phtalocyanine C");
	str->addItem("Phtalocyanine D");

	setEnabled(dev->features.supported & FEATURE_VARIREC_CD);
	setChecked(dev->features.enabled & FEATURE_VARIREC_CD);

	pl_str->setEnabled(ext);
	str->setEnabled(ext);

	pwr->setItemEnabled(0,ext);
	pwr->setItemEnabled(1,ext);
	pwr->setItemEnabled(7,ext);
	pwr->setItemEnabled(8,ext);

	pwr->setValue(dev->features.vrec_cd_pwr+4);
	str->setCurrentIndex(dev->features.vrec_cd_str);

	connect(pwr, SIGNAL(valueChanged(int)), this, SLOT(set()));
	connect(str, SIGNAL(currentIndexChanged(int)), this, SLOT(set()));
	connect(this, SIGNAL(toggled(bool)), this, SLOT(set(bool)));
}

void devctlVarirecCD::set() { set(isEnabled()); }

void devctlVarirecCD::set(bool en)
{
	if (en) {
		dev->features.enabled |= FEATURE_VARIREC_CD;
		dev->features.vrec_cd_pwr = pwr->text().toInt();
		dev->features.vrec_cd_str = str->currentIndex();
	} else {
		dev->features.enabled &= ~FEATURE_VARIREC_CD;
	}
	dev->setComplexFeature(FEATURE_VARIREC_CD, &dev->features);
}

devctlVarirecDVD::devctlVarirecDVD(device* idev, QWidget* p)
	:devctlVarirecBase(idev, "Varirec DVD", p)
{
	l_hint->setText(tr("VariRec limits maximum DVD write speed to 4X"));
	str->addItem("default");
	for (int s=0; s<8; s++)
	str->addItem(QString("Strategy %1").arg(s));

	setEnabled(dev->features.supported & FEATURE_VARIREC_DVD);
	setChecked(dev->features.enabled & FEATURE_VARIREC_DVD);

	pwr->setValue(dev->features.vrec_dvd_pwr+4);
	str->setCurrentIndex(dev->features.vrec_dvd_str);

	connect(pwr, SIGNAL(valueChanged(int)), this, SLOT(set()));
	connect(str, SIGNAL(currentIndexChanged(int)), this, SLOT(set()));
	connect(this, SIGNAL(toggled(bool)), this, SLOT(set(bool)));
}

void devctlVarirecDVD::set() { set(isEnabled()); }

void devctlVarirecDVD::set(bool en)
{
	if (en) {
		dev->features.enabled |= FEATURE_VARIREC_DVD;
		dev->features.vrec_dvd_pwr = pwr->text().toInt();
		dev->features.vrec_dvd_str = str->currentIndex();
	} else {
		dev->features.enabled &= ~FEATURE_VARIREC_DVD;
	}
	dev->setComplexFeature(FEATURE_VARIREC_DVD, &dev->features);
}

devctlVarirec::devctlVarirec(device* idev, QWidget* p)
	:QWidget(p)
{
	layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(3);

	vrec_cd = new devctlVarirecCD(idev, this);
	layout->addWidget(vrec_cd);
	if (idev->features.supported & FEATURE_VARIREC_DVD) {
		vrec_dvd = new devctlVarirecDVD(idev, this);
		layout->addWidget(vrec_dvd);
	} else {
		vrec_dvd = NULL;
		layout->addStretch(1);
	}

//	qDebug() << "Varirec CD  : " << (int)idev->features.vrec_cd_pwr << ", " << idev->features.vrec_cd_str;
//	qDebug() << "Varirec DVD : " << (int)idev->features.vrec_dvd_pwr << ", " << idev->features.vrec_dvd_str;
}

devctlSecurec::devctlSecurec(device* idev, QWidget* p)
	:QGroupBox("SecuRec",p)
{
	dev = idev;

	layout = new QGridLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);

	l_pwd[0] = new QLabel(tr("Password"),this);
	layout->addWidget(l_pwd[0], 0,0);
	e_pwd[0] = new QLineEdit(this);
	e_pwd[0]->setEchoMode(QLineEdit::Password);
	layout->addWidget(e_pwd[0], 0,1,1,3);
	l_pwd[1] = new QLabel(tr("Confirm"),this);
	layout->addWidget(l_pwd[1], 1,0);
	e_pwd[1] = new QLineEdit(this);
	e_pwd[1]->setEchoMode(QLineEdit::Password);
	layout->addWidget(e_pwd[1], 1,1,1,3);

	hline0 = new QFrame(this);
	hline0->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(hline0, 2,0,1,4);

	l_active = new QLabel("<font color=red>SecuRec ACTIVE<font>");
	l_active->setAlignment(Qt::AlignCenter);
	layout->addWidget(l_active, 3, 0, 1, 2);
	pb_set = new QPushButton(QIcon(":images/ok.png"),tr("Set"),this);
	pb_set->setMinimumWidth(100);
	layout->addWidget(pb_set, 3, 2);
	pb_reset = new QPushButton(QIcon(":images/edit-clear.png"),tr("Reset"),this);
	pb_reset->setMinimumWidth(100);
	layout->addWidget(pb_reset, 3, 3);

	l_hint = new QLabel(
			 tr("Password length must me between 4 and 10 characters.\n\n"
				"To read SecuRec-protected CD you have to activate SecuRec\n"
				"with same password as used for writing"), this);
	layout->addWidget(l_hint,4,0,1,4);
	layout->setRowStretch(4,10);

	setEnabled(dev->features.supported & FEATURE_SECUREC);
	l_active->setVisible(dev->features.enabled & FEATURE_SECUREC);

	connect(pb_set, SIGNAL(clicked()), this, SLOT(set()));
	connect(pb_reset, SIGNAL(clicked()), this, SLOT(reset()));
}

int devctlSecurec::validate()
{
	if (e_pwd[0]->text() != e_pwd[1]->text()) return 1;
	if (e_pwd[0]->text().length() < 4 || e_pwd[0]->text().length() > 10) return 2;
	return 0;
}

void devctlSecurec::set()
{
	switch (validate()) {
		case 0:
			break;
		case 1:
			QMessageBox::warning(this,"Different passwords","Passwords are not identical");
			return;
		case 2:
			QMessageBox::warning(this,"Invalid password","Invalid password!\nLength must be between 4 and 10 chars");
			return;
	}
	dev->features.enabled |= FEATURE_SECUREC;
	dev->features.sr_pass = e_pwd[0]->text();
	dev->setComplexFeature(FEATURE_SECUREC, &dev->features);
	l_active->setVisible(dev->features.enabled & FEATURE_SECUREC);
}

void devctlSecurec::reset()
{
	dev->features.enabled &= ~FEATURE_SECUREC;
	dev->setComplexFeature(FEATURE_SECUREC, &dev->features);
	e_pwd[0]->clear();
	e_pwd[1]->clear();
	l_active->setVisible(dev->features.enabled & FEATURE_SECUREC);
}

int sm_rcd[] = {48,40,32,24,8,4,0};
int sm_wcd[] = {48,32,24,16,8,4,0};
int sm_rdvd[] = {16,12,8,5,2,0};
// int sm_wdvd[] = {16,12,8,6,4,0};

devctlSilent::devctlSilent(device* idev, QWidget* p)
	:QWidget(p)
{
	dev = idev;
	int idx;
	bool cd=0;
	bool dvd=0;

	cw = new QGroupBox("Silent Mode",this);
	cw->setCheckable(true);

// R/W Speeds
	h_speed = new QLabel(tr("Speed Limits"), cw);
	h_speed->setAlignment(Qt::AlignCenter);
	h_speed->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	spdc = new QLabel(tr("CD"), cw);
	spdd = new QLabel(tr("DVD"), cw);
	spdr = new QLabel(tr("Read"), cw);
	spdw = new QLabel(tr("Write"), cw);
	box_rcd = new QComboBox(cw);
	for (int i=0; sm_rcd[i]>0; i++) box_rcd->addItem(QString::number(sm_rcd[i]) + "X");
	box_wcd = new QComboBox(cw);
	for (int i=0; sm_wcd[i]>0; i++) box_wcd->addItem(QString::number(sm_wcd[i]) + "X");
	box_rdvd = new QComboBox(cw);
	for (int i=0; sm_rdvd[i]>0; i++) box_rdvd->addItem(QString::number(sm_rdvd[i]) + "X");
	box_wdvd = new QComboBox(cw);
//	for (int i=0; sm_wdvd[i]>0; i++) box_wdvd->addItem(QString::number(sm_wdvd[i]) + "X");

	if (!(dev->cap_rd & DEVICE_DVD)) {
		spdd->setEnabled(false);
		box_rdvd->setEnabled(false);
	}
	box_wdvd->setEnabled(false);

// Access time
	h_access = new QLabel(tr("Access Time"), cw);
	h_access->setAlignment(Qt::AlignCenter);
	h_access->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	a_slow = new QRadioButton(tr("Slow"), cw);
	a_fast = new QRadioButton(tr("Fast"), cw);
	grp_access = new QButtonGroup(cw);
	grp_access->addButton(a_slow,0);
	grp_access->addButton(a_fast,1);
	a_slow->setChecked(true);

// Tray speed
	h_tray = new QLabel(tr("Tray Speed"), cw);
	h_tray->setAlignment(Qt::AlignCenter);
	h_tray->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	l_load  = new QLabel(tr("Load"), cw);
	l_eject = new QLabel(tr("Eject"), cw);
	s_load  = new QSlider(Qt::Horizontal,cw);
	s_load->setRange(0,80);
	s_eject = new QSlider(Qt::Horizontal,cw);
	s_eject->setRange(0,80);

// Buttons
	ck_perm = new QCheckBox(tr("Permanent"), this);
	pb_save = new QPushButton(QIcon(":images/ok.png"),tr("Save"), this);
	pb_save->setMinimumWidth(100);

// Layouts
	layoutm = new QVBoxLayout(this);
	layoutm->setMargin(0);
	layoutm->setSpacing(3);
	layoutm->addWidget(cw);

	layout = new QGridLayout(cw);
	layout->setMargin(3);
	layout->setSpacing(3);

	layout->addWidget(h_speed, 0,0,1,4);

	layout->addWidget(spdc,1,1,1,2);
	layout->addWidget(spdd,1,3);
	layout->addWidget(spdr,2,0);
	layout->addWidget(box_rcd,2,1,1,2);
	layout->addWidget(box_rdvd,2,3);
	layout->addWidget(spdw,3,0);
	layout->addWidget(box_wcd,3,1,1,2);
	layout->addWidget(box_wdvd,3,3);

	layout->addWidget(h_access, 4,0,1,4);

	layout->addWidget(a_slow, 5,0, 1,2);
	layout->addWidget(a_fast, 5,2, 1,2);

	layout->addWidget(h_tray, 6,0,1,4);

	layout->addWidget(l_load, 7,0);
	layout->addWidget(s_load, 7,1,1,3);
	layout->addWidget(l_eject, 8,0);
	layout->addWidget(s_eject, 8,1,1,3);

	layout->setColumnStretch(0,2);
	layout->setColumnStretch(1,1);
	layout->setColumnStretch(2,1);
	layout->setColumnStretch(3,2);
	layout->setRowStretch(10,10);

	layoutb = new QHBoxLayout();
	layoutb->setMargin(0);
	layoutb->setSpacing(3);
	layoutm->addLayout(layoutb);

	layoutb->addStretch(10);
	layoutb->addWidget(ck_perm);
	layoutb->addWidget(pb_save);

	cd  = dev->media.type.startsWith("CD");
	dvd = dev->media.type.startsWith("DVD");
/*
	qDebug() << " SM RD CD :" << dev->features.sm_cd_rd;
	qDebug() << " SM WR CD :" << dev->features.sm_cd_wr;
	qDebug() << " SM RD DVD:" << dev->features.sm_dvd_rd;
	qDebug() << "PSM RD CD :" << dev->features.psm_cd_rd;
	qDebug() << "PSM WR CD :" << dev->features.psm_cd_wr;
	qDebug() << "PSM RD DVD:" << dev->features.psm_dvd_rd;
*/
	idx = box_rcd->findText(QString::number(dvd ? dev->features.psm_cd_rd : dev->features.sm_cd_rd)+"X");
	if (idx>=0) box_rcd->setCurrentIndex(idx);
	idx = box_wcd->findText(QString::number(dvd ? dev->features.psm_cd_wr : dev->features.sm_cd_wr)+"X");
	if (idx>=0) box_wcd->setCurrentIndex(idx);

	idx = box_rdvd->findText(QString::number(dvd ? dev->features.sm_dvd_rd : dev->features.psm_dvd_rd)+"X");
	if (idx>=0) box_rdvd->setCurrentIndex(idx);
//	idx = box_wdvd->findText(QString::number(dvd ? dev->features.sm_wr : dev->features.sm_dvd_wr)+"X");
//	if (idx>=0) box_wdvd->setCurrentIndex(idx);

	if (dev->features.sm_access)
		a_fast->setChecked(true);
	else
		a_slow->setChecked(true);
	s_load->setValue(dev->features.sm_trayl);
	s_eject->setValue(dev->features.sm_traye);
/*
	connect(box_rcd, SIGNAL(currentIndexChanged(int)), this, SLOT(set()));
	connect(box_wcd, SIGNAL(currentIndexChanged(int)), this, SLOT(set()));
	connect(box_rdvd, SIGNAL(currentIndexChanged(int)), this, SLOT(set()));
	connect(box_wdvd, SIGNAL(currentIndexChanged(int)), this, SLOT(set()));

	connect(a_fast, SIGNAL(toggled(bool)), this, SLOT(set()));

	connect(s_load, SIGNAL(valueChanged(int)), this, SLOT(set()));
	connect(s_eject, SIGNAL(valueChanged(int)), this, SLOT(set()));
*/

	setEnabled(dev->features.supported & FEATURE_SILENT);
	cw->setChecked(dev->features.enabled & FEATURE_SILENT);

	connect(pb_save, SIGNAL(clicked()), this, SLOT(set()));
//	connect(this, SIGNAL(toggled(bool)), this, SLOT(set(bool)));
}

void devctlSilent::set()
{
	bool en = cw->isChecked();
	if (en) {
		dev->features.enabled |= FEATURE_SILENT;
		dev->features.sm_cd_rd = box_rcd->currentText().remove("X").toInt();
		dev->features.sm_cd_wr = box_wcd->currentText().remove("X").toInt();
		dev->features.sm_dvd_rd = box_rdvd->currentText().remove("X").toInt();
//		dev->features.sm_dvd_wr = box_wdvd->currentText().remove("X").toInt();
		dev->features.sm_access = a_fast->isChecked();
		dev->features.sm_trayl = s_load->value();
		dev->features.sm_traye = s_eject->value();
	} else {
		dev->features.enabled &= ~FEATURE_SILENT;
	}

	dev->features.sm_nosave = !ck_perm->isChecked();
	dev->setComplexFeature(FEATURE_SILENT, &dev->features);
}

devctlAutostrategy::devctlAutostrategy(device* idev, QWidget* p)
	:QWidget(p)
{
	dev = idev;
	setEnabled(dev->features.supported & FEATURE_AS);
	layout = new QGridLayout(this);
	layout->setMargin(0);
	layout->setSpacing(3);

// AS mode selection
	box_as = new QGroupBox(tr("AutoStrategy"), this);
	layout->addWidget(box_as);

	layout_as = new QGridLayout(box_as);
	layout_as->setMargin(3);
	layout_as->setSpacing(3);

	rb_as_off = new QRadioButton("Off", box_as);
	rb_as_auto = new QRadioButton("Auto", box_as);
	rb_as_on = new QRadioButton("On", box_as);
	rb_as_forced = new QRadioButton("Forced", box_as);
	rb_as_on->setEnabled(dev->features.supported & FEATURE_AS_EXT);
	rb_as_forced->setEnabled(dev->features.supported & FEATURE_AS_EXT);

	grp_mode = new QButtonGroup(box_as);
	grp_mode->addButton(rb_as_off,    AS_MODE_OFF);
	grp_mode->addButton(rb_as_auto,   AS_MODE_AUTO);
	grp_mode->addButton(rb_as_on,     AS_MODE_ON);
	grp_mode->addButton(rb_as_forced, AS_MODE_FORCED);
	rb_as_off->setChecked( dev->features.as_mode == AS_MODE_OFF );
	rb_as_auto->setChecked( dev->features.as_mode == AS_MODE_AUTO );
	rb_as_on->setChecked( dev->features.as_mode == AS_MODE_ON );
	rb_as_forced->setChecked( dev->features.as_mode == AS_MODE_FORCED );

	layout_as->addWidget(rb_as_off,    0, 0);
	layout_as->addWidget(rb_as_auto,   0, 1);
	layout_as->addWidget(rb_as_on,     0, 2);
	layout_as->addWidget(rb_as_forced, 0, 3);

	pb_asdb = new QPushButton(tr("AutoStrategy DataBase"), box_as);
	layout_as->addWidget(pb_asdb,   1,2,1,2);

// MQCK
	box_mqck = new QGroupBox(tr("Media Quality Check"), this);
	layout->addWidget(box_mqck);

	layout_mqck = new QGridLayout(box_mqck);
	layout_mqck->setMargin(3);
	layout_mqck->setSpacing(3);

	rb_mqck_q = new QRadioButton(tr("Quick"), box_mqck);
	rb_mqck_a = new QRadioButton(tr("Advanced"), box_mqck);
	grp_mqck = new QButtonGroup(box_mqck);
	grp_mqck->addButton(rb_mqck_q);
	grp_mqck->addButton(rb_mqck_a);
	rb_mqck_q->setChecked(true);

	lmqck_spd = new QLabel(tr("Speed:"), box_mqck);
	lmqck_spd->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	bmqck_spd = new QComboBox(box_mqck);
	bmqck_spd->addItems( dev->media.wspeedsd );

	pb_mqck = new QPushButton(tr("Check Media"), box_mqck);
	lmqck_res = new QLabel(box_mqck);
	lmqck_res->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	lmqck_res->setMinimumHeight(22);

	layout_mqck->addWidget(rb_mqck_q, 0,0);
	layout_mqck->addWidget(rb_mqck_a, 0,1);
	layout_mqck->addWidget(lmqck_spd, 0,2);
	layout_mqck->addWidget(bmqck_spd, 0,3);
	layout_mqck->addWidget(pb_mqck,   1,2,1,2);
	layout_mqck->addWidget(lmqck_res, 2,0,1,4);

//	layout->setRowStretch(2,4);

	connect(grp_mode, SIGNAL(buttonClicked(int)), this, SLOT(set_mode(int)));
	connect(pb_asdb,  SIGNAL(clicked()), this, SLOT(asdb()));
	connect(pb_mqck,  SIGNAL(clicked()), this, SLOT(run_mqck()));
}

void devctlAutostrategy::asdb()
{
	qDebug("AS: DB...");
	devctlAutostrategyDB *asdb = new devctlAutostrategyDB(dev, this);
	asdb->exec();
	delete asdb;
}

void devctlAutostrategy::set_mode(int mode)
{
	qDebug("AS: set mode...");
	dev->features.as_action = AS_ACTION_MODE;
	dev->features.as_mode = mode;
	dev->setComplexFeature(FEATURE_AS, &dev->features);
}

void devctlAutostrategy::run_mqck()
{
	ProgressWidget *progress;
	qDebug("AS: MQCK...");
	lmqck_res->setText( "" );
	dev->features.as_action = AS_ACTION_MQCK;
	dev->features.as_act_mode = rb_mqck_a->isChecked() ? ASMQCK_ADV : 0;
	dev->features.as_mqckspd = bmqck_spd->currentText().remove(QRegExp(".[0-9][Xx]")).toInt();
	dev->features.as_mqckres = "";
	if (!dev->startMqck()) {
		QMessageBox::warning(this, "MQCK error", tr("Can't run cdvdcontrol!") );
		return;
	}

	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Checking media quality..."));
	progress->show();
	while(dev->isRunning()) { msleep ( 1 << 5); qApp->processEvents(); }
	delete progress;
	if (dev->features.as_mqckres.isEmpty()) {
		lmqck_res->setText( tr("Unknown result") );
	} else {
		lmqck_res->setText( dev->features.as_mqckres);
	}
}

devctlAutostrategyDB::devctlAutostrategyDB(device* idev, QWidget* p)
	:QDialog(p)
{
	dev = idev;
	setWindowTitle(tr("AutoStrategy DataBase"));

	tree = new QTreeWidget(this);
	tree->setRootIsDecorated(false);
	tree->setHeaderLabels( QStringList() << "Act" << tr("Type") << tr("Media ID") << tr("Speed") << tr("Writes"));
	tree->setColumnWidth(0,30);
	tree->setColumnWidth(1,50);
	tree->setColumnWidth(2,150);
	tree->setColumnWidth(3,50);
	tree->setColumnWidth(4,50);

	pb_act   = new QPushButton(tr("Activate"), this);
	pb_deact = new QPushButton(tr("Deactivate"), this);
	pb_del   = new QPushButton(QIcon(":images/x.png"),tr("Remove"), this);
	pb_clear = new QPushButton(QIcon(":images/edit-clear.png"),tr("Clear"), this);

	layout = new QVBoxLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);
	layout->addWidget(tree,10);

	layoutb = new QHBoxLayout();
	layout->addLayout(layoutb);
	layoutb->setMargin(0);
	layoutb->setSpacing(3);
	layoutb->addWidget(pb_act,1);
	layoutb->addWidget(pb_deact,1);
	layoutb->addWidget(pb_del,1);
	layoutb->addWidget(pb_clear,1);

// Strategy creation
	box_ascre = new QGroupBox(tr("Strategy creation"), this);
	box_ascre->setEnabled(dev->features.supported & FEATURE_AS_EXT);
//	box_ascre->setAlignment(Qt::AlignCenter);
//	box_ascre->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout->addWidget(box_ascre);

	grp_ascre_mode = new QGroupBox(tr("Mode"), box_ascre);
	grp_ascre_act  = new QGroupBox(tr("DB Action"), box_ascre);
	ascre_start = new QPushButton(tr("Create"), box_ascre);

	layout_ascre = new QGridLayout(box_ascre);
	layout_ascre->setMargin(0);
	layout_ascre->setSpacing(3);
	layout_ascre->addWidget(grp_ascre_mode, 0,0,1,2);
	layout_ascre->addWidget(grp_ascre_act,  0,2,1,2);
	layout_ascre->addWidget(ascre_start, 1,3);
	layout_ascre->setRowStretch(0,2);

	rb_ascre_quick   = new QRadioButton(tr("Quick"), grp_ascre_mode);
	rb_ascre_full    = new QRadioButton(tr("Full"), grp_ascre_mode);
	rb_ascre_quick->setChecked(true);
	layout_crem = new QVBoxLayout(grp_ascre_mode);
	layout_crem->setMargin(3);
	layout_crem->setSpacing(3);
	layout_crem->addWidget(rb_ascre_quick);
	layout_crem->addWidget(rb_ascre_full);

	rb_ascre_add     = new QRadioButton(tr("Add"), grp_ascre_act);
	rb_ascre_replace = new QRadioButton(tr("Replace"), grp_ascre_act);
	rb_ascre_add->setChecked(true);
	layout_crea = new QVBoxLayout(grp_ascre_act);
	layout_crea->setMargin(3);
	layout_crea->setSpacing(3);
	layout_crea->addWidget(rb_ascre_add);
	layout_crea->addWidget(rb_ascre_replace);

	setMinimumWidth(360);

	connect(tree,     SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(db_toggle(QTreeWidgetItem*)));

	connect(pb_act,   SIGNAL(clicked()), this, SLOT(db_act()));
	connect(pb_deact, SIGNAL(clicked()), this, SLOT(db_deact()));
	connect(pb_del,   SIGNAL(clicked()), this, SLOT(db_del()));
	connect(pb_clear, SIGNAL(clicked()), this, SLOT(db_clear()));

	connect(ascre_start, SIGNAL(clicked()), this, SLOT(run_ascre()));
}

void devctlAutostrategyDB::setVisible(bool v)
{
	QDialog::setVisible(v);
	//if (v && !dev->asdb.size()) db_update();
	if (v) db_update();
}

void devctlAutostrategyDB::db_update(int idx)
{
	QTreeWidgetItem *it;
	ProgressWidget *progress;

	connect(dev, SIGNAL(doneGetASDB(bool)), this, SLOT(db_update_done(bool)));

	dev->getASDB();

	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Retrieving ASDB..."));
	progress->show();
	while(dev->isRunning()) { msleep ( 1 << 5); qApp->processEvents(); }
#if 0
	for (int i=0; i<10; i++) {
		msleep ( 1 << 7);
		progress->step();
		qApp->processEvents();
	}
#endif
	delete progress;

// clear tree
	while (tree->topLevelItemCount()) {
		it = tree->takeTopLevelItem(0);
		if (it) delete it;
	}
// show updated db
	for (int i=0; i<dev->asdb.size(); i++) {
		if (dev->asdb[i].present) {
			it = new QTreeWidgetItem(tree,
					QStringList() << "" << dev->asdb[i].type << dev->asdb[i].mid << dev->asdb[i].speed << dev->asdb[i].writes);
			if (dev->asdb[i].active) it->setIcon(0,QIcon(":images/ok.png"));
			tree->addTopLevelItem(it);
		}
	}

	if (idx>=0) {
		if (idx >= dev->asdb.size()) idx = dev->asdb.size()-1;
		it = tree->topLevelItem(idx);
		if (it) tree->setCurrentItem(it);
	}

	for (int i=0; i<4; i++)
		tree->resizeColumnToContents(i);

	disconnect(dev, SIGNAL(doneGetADSB(bool)), this, SLOT(db_update_done(bool)));
}

void devctlAutostrategyDB::db_update_done(bool fail)
{
	if (fail) {
		QMessageBox::warning(this, tr("Error"), tr("Error requesting ASDB!")+"\n"+tr("cdvdcontrol finished with non-zero exit code"));
	}
}

void devctlAutostrategyDB::db_toggle(QTreeWidgetItem* item)
{
	int idx = tree->indexOfTopLevelItem(item);
	if (dev->asdb[idx].active) {
		db_deact(idx);
	} else {
		db_act(idx);
	}
}

void devctlAutostrategyDB::db_act()
{
	db_act(tree->indexOfTopLevelItem(tree->currentItem()));
}

void devctlAutostrategyDB::db_act(int idx)
{
	if (idx<0 || idx>31) return;
	dev->features.as_action = AS_ACTION_ACT;
	dev->features.as_idx = idx+1;
	dev->setComplexFeature(FEATURE_AS, &dev->features);
	db_update(idx);
}

void devctlAutostrategyDB::db_deact()
{
	db_deact(tree->indexOfTopLevelItem(tree->currentItem()));
}

void devctlAutostrategyDB::db_deact(int idx)
{
	if (idx<0 || idx>31) return;
	dev->features.as_action = AS_ACTION_DEACT;
	dev->features.as_idx = idx+1;
	dev->setComplexFeature(FEATURE_AS, &dev->features);
	db_update(idx);
}

void devctlAutostrategyDB::db_del()
{
	int idx = tree->indexOfTopLevelItem(tree->currentItem());
	if (idx<0) return;
	if (QMessageBox::warning(this,
			tr("Are you sure?"),
			tr("Are you sure to delete strategy #%1?").arg(idx),
			QMessageBox::Yes | QMessageBox::Cancel,
			QMessageBox::Cancel) != QMessageBox::Yes)
		return;

	dev->features.as_action = AS_ACTION_DEL;
	dev->features.as_idx = idx+1;
	dev->setComplexFeature(FEATURE_AS, &dev->features);
	db_update(idx);
}

void devctlAutostrategyDB::db_clear()
{
	if (QMessageBox::warning(this,
			tr("Are you sure?"),
			tr("Are you sure delete all strategies?"),
			QMessageBox::Yes | QMessageBox::Cancel,
			QMessageBox::Cancel) != QMessageBox::Yes)
		return;
	dev->features.as_action = AS_ACTION_CLEAR;
	dev->setComplexFeature(FEATURE_AS, &dev->features);
	db_update();
}

void devctlAutostrategyDB::run_ascre()
{
	ProgressWidget *progress;
	qDebug("AS: creating strategy...");
//	dev->
	if (dev->media.creads || !dev->media.type.startsWith("DVD") || dev->media.erasable == "yes") {
		QMessageBox::warning(this,
			tr("Can't create strategy!"),
			tr("Strategy creation does not supported on current media: ") + dev->media.type + "\n"+
			tr("supported media types: DVD+R(DL) and DVD-R(DL)")
		);
		return;
	}


	dev->features.as_action = AS_ACTION_CRE;
	dev->features.as_act_mode = (rb_ascre_full->isChecked() ? ASCRE_FULL : 0) | (rb_ascre_replace->isChecked() ? ASCRE_REPLACE : 0);
	dev->startAScre();

	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Creating strategy..."));
	progress->show();
	while(dev->isRunning()) { msleep ( 1 << 5); qApp->processEvents(); }
	delete progress;

	db_update();
}

devctlDestruct::devctlDestruct(device* idev, QWidget* p)
	:QGroupBox(tr("Data destruction"), p)
{
	dev = idev;
	setEnabled(dev->features.supported & FEATURE_DESTRUCT);
	rb_quick = new QRadioButton(tr("Quick"), this);
	pl_quick = new QLabel(tr("Destruct Lead-in & TOC"), this);
	rb_full  = new QRadioButton(tr("Full"), this);
	pl_full  = new QLabel(tr("Destruct entire disc"), this);
	pb_start = new QPushButton(QIcon(":images/eraser.png"),tr("Destruct"), this);
	pb_start->setMinimumWidth(100);
	rb_quick->setChecked(true);

	layout = new QGridLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);
	layout->addWidget(rb_quick, 0,0,1,3);
	layout->addWidget(pl_quick, 1,1,1,2);
	layout->addWidget(rb_full, 2,0,1,3);
	layout->addWidget(pl_full, 3,1,1,2);
	layout->addWidget(pb_start, 5,2);

	layout->setColumnStretch(0,1);
	layout->setColumnStretch(1,10);
	layout->setColumnStretch(2,1);
	layout->setRowStretch(4,10);

	rb_quick->setChecked(true);

	connect(pb_start, SIGNAL(clicked()), this, SLOT(start()));
}

void devctlDestruct::start()
{
	ProgressWidget *progress;
	if (QMessageBox::warning(this,
			tr("Are you sure?"),
			tr("Media will be unreadable after this operation!"),
			QMessageBox::Yes | QMessageBox::Cancel,
			QMessageBox::Cancel) != QMessageBox::Yes)
		return;

	qDebug("Destructing media...");
	dev->features.as_act_mode = !!rb_full->isChecked();
	dev->startDestruct();

	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Destructing data..."));
	progress->show();
	while(dev->isRunning()) { msleep ( 1 << 5); qApp->processEvents(); }
	delete progress;
}

// Pioneer QuietMode

devctlPioquiet::devctlPioquiet(device* idev, QWidget* p)
	:QGroupBox(tr("Pioneer Quiet Mode"),p)
{
	dev = idev;
	setEnabled( dev->features.supported & FEATURE_PIOQUIET );

	layout = new QGridLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);

	box_mode = new QGroupBox(tr("Profile"),this);
	layout->addWidget(box_mode,0,0,1,3);

	layoutm = new QVBoxLayout(box_mode);
	layoutm->setMargin(3);
	layoutm->setSpacing(3);

	rb_quiet = new QRadioButton(tr("Quiet"), box_mode);
	rb_std   = new QRadioButton(tr("Standard"), box_mode);
	rb_perf  = new QRadioButton(tr("Performance"), box_mode);
	layoutm->addWidget(rb_quiet);
	layoutm->addWidget(rb_std);
	layoutm->addWidget(rb_perf);
	rb_quiet->setChecked( dev->features.pioq_quiet == PIOQ_QUIET );
	rb_perf->setChecked( dev->features.pioq_quiet == PIOQ_PERF );
	rb_std->setChecked( dev->features.pioq_quiet == PIOQ_STD );

	ck_limit = new QCheckBox(tr("Limit read speed to 24X for CD and 8X for DVD"),this);
	layout->addWidget(ck_limit, 1,0,1,3);
	ck_limit->setChecked( dev->features.enabled & FEATURE_PIOLIMIT );

	ck_perm = new QCheckBox(tr("Permanent"),this);
	layout->addWidget(ck_perm, 3,1);
	pb_set = new QPushButton(QIcon(":images/ok.png"),tr("Set"),this);
	pb_set->setMinimumWidth(100);
	layout->addWidget(pb_set, 3,2);

	layout->setRowStretch(0,2);
	layout->setRowStretch(1,1);
	layout->setRowStretch(2,10);
	layout->setRowStretch(3,1);

	connect(pb_set, SIGNAL(clicked()), this, SLOT(set()));
}

void devctlPioquiet::set()
{
	dev->features.pioq_nosave = !ck_perm->isChecked();
	if (ck_limit->isChecked())
		dev->features.enabled |= FEATURE_PIOLIMIT;
	else
		dev->features.enabled &= ~FEATURE_PIOLIMIT;

	if (rb_quiet->isChecked()) {
		dev->features.pioq_quiet  = PIOQ_QUIET;
	} else if (rb_perf->isChecked()) {
		dev->features.pioq_quiet  = PIOQ_PERF;
	} else {
		dev->features.pioq_quiet  = PIOQ_STD;
	}
	dev->setComplexFeature(FEATURE_PIOQUIET, &dev->features);
}

// Yamaha CRW-F1 Disc T@2

devctlF1Tattoo::devctlF1Tattoo(device* idev, QWidget* p)
	:QGroupBox("Yamaha CRW-F1 Disc T@2",p)
{
	dev = idev;
	setEnabled(dev->features.supported & FEATURE_F1TATTOO);
	
	layout = new QGridLayout(this);
	layout->setMargin(3);
	layout->setSpacing(3);

	tw = new TattooWidget(this);
	layout->addWidget(tw,0,0,1,2);

	r0=dev->features.tattoo_inner;
	r1=dev->features.tattoo_outer;
	tw->setRadius(r0,r1);

	layoutb = new QHBoxLayout();
	layoutb->setMargin(0);
	layoutb->setSpacing(3);
	layout->addLayout(layoutb,1,0,1,2);

	l_file = new QLabel(this);
	l_file->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layoutb->addWidget(l_file,20);
	pb_load = new QPushButton(this);
	pb_load->setMaximumSize(24,24);
	pb_load->setIcon(QIcon(":images/fileopen.png"));
	layoutb->addWidget(pb_load,1);

	pb_burn = new QPushButton(QIcon(":images/tattoo.png"), tr("Burn T@2"), this);
	pb_burn->setMinimumWidth(100);
	pb_burn->setEnabled(false);
	layout->addWidget(pb_burn,2,1);

	layout->setRowStretch(0,20);
	layout->setColumnStretch(0,5);
	layout->setColumnStretch(1,1);

	connect(pb_load, SIGNAL(clicked()), this, SLOT(loadImage()));
	connect(pb_burn, SIGNAL(clicked()), this, SLOT(burn()));
}

void devctlF1Tattoo::loadImage()
{

	QString fn = QFileDialog::getOpenFileName(this, tr("Select T@2 image..."));
	if (fn == QString::null) return;

	if (!srcimg.load(fn)) {
		QMessageBox::warning(this, tr("Error"), tr("Can't load image: ") + fn);
		return;
	}
	qDebug() << "Loaded image: " << srcimg.width() << "x" << srcimg.height();
	l_file->setText(fn);

	img = srcimg.scaled(F1TATTOOW, (r1-r0), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	qDebug() << "Scaled image: " << img.width() << "x" << img.height();
	tw->setImage(img);

	pb_burn->setEnabled(srcimg.width() && srcimg.height());
}

void devctlF1Tattoo::burn()
{
	ProgressWidget *progress;
	QString tfn;
	QFile   tf;
	QByteArray line(F1TATTOOW, 0);

	qDebug("Generating T@2 data...");
	//QImage timg = img.scaled(F1TATTOOW, (r1-r0+1), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	qDebug() << "T@2 image size: " << img.width() << "x" << img.height();

	QStringList env = QProcess::systemEnvironment();
	int hidx = env.indexOf(QRegExp("HOME=(.*)"));
	if (hidx>=0) {
		tfn = env[hidx].remove(0,5);
	} else {
		tfn = "/tmp";
	}
	tfn += "/tmp.tattoo";
	qDebug() << "T@2 temporary file: " << tfn;
//	img.save(tfn + ".png");

	tf.setFileName(tfn);
	if (!tf.open(QIODevice::WriteOnly))
	{
		QMessageBox::critical(this, tr("Error"), tr("Can't create DiscT@2 temporary file!"));
		return;
	}

	for(int y=img.height()-1; y>=0; y--) {
		for (int x=0; x<F1TATTOOW; x++) {
			line[x] = qGray(img.pixel(x,y)) ^ 255;
		}
		tf.write(line);
	}
	tf.close();
	qDebug("Starting T@2 burn...");
	dev->features.tattoo_file = tfn;
	dev->startTattoo();


	progress = new ProgressWidget(10,3,this);
	progress->setText(tr("Burning DiscT@2 image..."));
	progress->show();
	while(dev->isRunning()) { msleep ( 1 << 5); qApp->processEvents(); }

#if 0
	for(int i=0; i<16; i++) {
		msleep ( 1 << 7);
		progress->step();
		qApp->processEvents();
	}
#endif
	delete progress;
}

