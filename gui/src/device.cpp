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

#include <QApplication>
#include <QTreeWidgetItem>
#include <QProcess>
#include <QTcpSocket>
#include <QDataStream>

#include <qpxiodevice.h>
#include <mwatcher.h>
#include <resultsio.h>
#include "device.h"

#include <sys/types.h>
#include <signal.h>
#include <qpx_mmc_defs.h>

#ifndef QT_NO_DEBUG
#include <QDebug>
static int devcnt=0;
#endif

#if defined(_WIN32)
#include <windows.h>
//#if !defined(__MINGW32__)
//#endif
#endif

void ErrcADD(Errc<int64_t> *tot, const Errc<int>& o) {
	for (int i=0; i<8; i++)
		if (o.raw.err[i] > 0) tot->raw.err[i] += o.raw.err[i];
};

void ErrcMAX(Errc<int> *max, const Errc<int>& o) {
	for (int i=0; i<8; i++)
		if (max->raw.err[i] < o.raw.err[i]) max->raw.err[i] = o.raw.err[i];
};

void CDErrcAVG(Errc<float> *avg, Errc<int64_t> *tot, uint64_t blocks) {
	if (!blocks) blocks=1;
	for (int i=0; i<8; i++)
		avg->raw.err[i] = (float) tot->raw.err[i] / blocks;
};

void DVDErrcAVG(Errc<float> *avg, Errc<int64_t> *tot, uint64_t blocks) {
	avg->dvd.pie = (float) tot->dvd.pie / blocks;
	avg->dvd.pif = (float) tot->dvd.pif / blocks;
	avg->dvd.poe = (float) tot->dvd.poe / blocks;
	avg->dvd.pof = (float) tot->dvd.pof / blocks;
	avg->dvd.uncr = (float) tot->dvd.uncr / blocks;
	if (blocks >= 8) {
		avg->dvd.pi8 = (float) tot->dvd.pi8 / ( blocks >> 3);
		avg->dvd.po8 = (float) tot->dvd.po8 / ( blocks >> 3);
	}
};

void BDErrcAVG(Errc<float> *avg, Errc<int64_t> *tot, uint64_t blocks) {
	avg->bd.ldc = (float) tot->bd.ldc / blocks;
	avg->bd.bis = (float) tot->bd.bis / blocks;
	avg->bd.uncr = (float) tot->bd.uncr / blocks;
};

static device NullDev(NULL);

/*
 *
 * Device List
 *
 */

devlist::devlist()
	: QList<device*>()
{
#ifndef QT_NO_DEBUG
	qDebug("* STA: devlist()");
#endif
	devidx=-1;
#ifndef QT_NO_DEBUG
	qDebug("* END: devlist()");
#endif
};

devlist::~devlist() {
#ifndef QT_NO_DEBUG
	qDebug("* STA: ~devlist()");
#endif
	clear();
#ifndef QT_NO_DEBUG
	qDebug("* END: ~devlist()");
#endif
};

int devlist::idx() { return devidx; };

void devlist::setIdx(int iidx) {
	if (iidx<0 || iidx>=size()) {
		devidx=-1;
		return; 
	}
	devidx=iidx;
};

void devlist::clear() {
	device *dev;
	while (size()) {
		dev = takeLast();
		dev->stopWatcher();
		delete dev;
	}
	devidx=-1; 
};

device* devlist::current() {
	if (devidx<0 || devidx>=size()) return &NullDev;
	return (*this)[devidx];
};

/*
 *
 * Device
 *
 */

device::device(QObject* p)
	: QObject(p)
{
#ifndef QT_NO_DEBUG
	qDebug() << "* STA: device(): " << this << " #"<< devcnt++ << " parent: "<< p;
#endif
	preserveMediaInfo=0;
	running=0;
	type = DevtypeNone;
	host = "";
	port = 0;
	info_set = 0;

	rpc_phase = -1;
	rpc_reg   = -1;
	rpc_ch    = -1;
	rpc_rst   = -1;

	plextor_lock = 0;

	asdb.clear();
	clearMinfo();

	cap	   = 0;
	cap_rd = 0;
	cap_wr = 0;

	life_dn = -1;
	features.supported = 0;
	features.enabled = 0;
	features.grec = 1.0;
	features.vrec_cd_pwr = 0;
	features.vrec_cd_str = 0;
	features.vrec_dvd_pwr = 0;
	features.vrec_dvd_str = 0;
	features.tattoo_inner = 22;
	features.tattoo_outer = 54;

	features.sm_cd_rd = 0;
	features.sm_cd_wr = 0;
	features.sm_dvd_rd = 0;
	features.psm_cd_rd = 0;
	features.psm_cd_wr = 0;
	features.psm_dvd_rd = 0;

	features.as_mode = AS_MODE_AUTO;
	features.pioq_quiet = PIOQ_STD;

	autoupdate = 0;

	pprocess = 0.0;
	nprocess = "";

	test_cap=0;
	test_req=0;
	tests = 0;
	ctest = 0;
	WT_simul = 1;

	tspeeds.rt = 1;
	tspeeds.wt = 1;
	tspeeds.errc = 1;
	tspeeds.jb = 1;
	tspeeds.ft = 1;

	io = new QPxIODevice(this);
	proc = NULL;
	sock = NULL;
	mwatcher = NULL;
	mutex = new QMutex();
	io_mutex = new QMutex();

	resReader = new ResultsReader(this);
	resWriter = new ResultsWriter(this);

if (!p) {
#ifndef QT_NO_DEBUG
		qDebug() << "device: NULL parent!";
#endif
	} else {
		connect(this, SIGNAL(doneMInfo(int)),     p, SLOT(mediaUpdated(int)));
		connect(this, SIGNAL(process_started()),  p, SLOT(process_started()));
		connect(this, SIGNAL(process_finished()), p, SLOT(process_finished()));
		connect(this, SIGNAL(process_progress()), p, SLOT(process_progress()));
	}
	connect(resReader, SIGNAL(finished()), this, SLOT(resLoaderDone()));

#ifndef QT_NO_DEBUG
	qDebug("* END: device()");
#endif
};

device::~device()
{
#ifndef QT_NO_DEBUG
	qDebug() << "* STA: ~device(): " << this << " #"<< --devcnt;
#endif
	stopWatcher();
	if (mwatcher) {
		delete mwatcher;
	}
	QTreeWidgetItem* item;
#ifdef MINFO_TREE
	while (info_media.size()) {
		item = info_media.takeLast();
		if (item) delete item;
	}
/*
	if (running) {
		qDebug() << "device: waiting for child process...";
		while (running) {
			msleep(100);
		}
	}
*/
#endif
//	mutex->unlock();
	delete mutex;
#ifndef QT_NO_DEBUG
	qDebug() << "* END: ~device()";
#endif
};

bool device::isRunning()
{
	return running;
};

void device::clearMinfo()
{
	QTreeWidgetItem* item;

	testData.clear();

//	media.isCD      = 0;
//	media.isDVD     = 0;
	media.label		= "";
	media.type		= "-";
	media.category	= "-";
	media.mid		= "-";
	media.erasable	= "-";
	media.layers	= "-";
	media.ilayers	= 1;
	media.prot		= "-";
	media.regions	= "-";
	media.creads	= 0;
	media.creadm	= 0;
	media.creadmsf	= "";
	media.cfrees	= 0;
	media.cfreem	= 0;
	media.cfreemsf	= "";
	media.ctots		= 0;
	media.ctotm		= 0;
	media.ctotmsf	= "";
	media.dstate	= "-";
	media.sstate	= "-";
	media.writer	= "-";
	media.grec      = 0.0;
	media.rspeeds.clear();
	media.wspeedsd.clear();
	media.wspeedsm.clear();

	media.tdata_errc = 0;
	media.tspeeds_errc.clear();
	media.tspeeds_jb.clear();
#ifdef MINFO_TREE
	while (info_media.size()) {
		item = info_media.takeLast();
		if (item) delete item;
	}
#endif
};

bool device::start()
{
#ifndef QT_NO_DEBUG
	qDebug() << "STA: device::start(" << threadType << ")";
#endif
	if (!mutex->tryLock()) {
#ifndef QT_NO_DEBUG
		qDebug() << "Device busy: " << id;
		qDebug() << "END: device::start()";
#endif
		return false;
	}
	stop=0;
	running=1;
	switch (threadType) {
		case threadDevice:
		case threadMedia:
		case threadGetFeatures:
		case threadGetASDB:
		case threadMQCK:
		case threadAScre:
		case threadDestruct:
		case threadTattoo:
			start_update_info();
			break;
		case threadTest:
			next_test();
			break;
		default:
			mutex->unlock();
#ifndef QT_NO_DEBUG
			qDebug() << "END: device::start()";
#endif
			running=0;
			return false;
	}

#ifndef QT_NO_DEBUG
	qDebug() << "END: device::start()";
#endif
	return true;
}

bool device::start_update_info()
{
#ifndef QT_NO_DEBUG
	qDebug() << "STA: device::start_update_info(" << threadType << ")";
#endif
	if (threadType==threadMedia && !preserveMediaInfo) {
		clearMinfo();
//		mwidget->update();
#warning !!! mwidget->clearMedia() call
		//mwidget->clearMedia();
	}

	//if (devices.idx()<0 || devices.idx()>= devices.size() || (threadType!=infoDevice && threadType!=infoMedia)) {
	if (type == DevtypeNone) {
		goto update_info_err;
	}
//	if (threadType == infoDevice) {
//		
//	}
	if (proc) {
		delete proc;
		proc = NULL;
	}
	if (sock) {
		delete sock;
		sock = NULL;
	}

	QObject::connect(io, SIGNAL(readyReadLine()),
		this, SLOT(qscan_process_info()));

	if (type == DevtypeLocal) {
#ifndef QT_NO_DEBUG
		qDebug() << "device: LOCAL";
#endif
		proc = new QProcess(this);
#ifndef QT_NO_DEBUG
		qDebug() << "process created";
#endif
		io->setIODevice(proc);
		proc->setReadChannel(QProcess::StandardOutput);

//		QObject::connect(proc, SIGNAL(readyReadStandardOutput()),
//				this, SLOT(qscan_process_info()));

		switch (threadType) {
			case threadDevice:
				proc->start("qscan", QStringList() << "-d" << path << "-Ip");
				break;
			case threadMedia:
				if (plugin.isEmpty()) {
					proc->start("qscan", QStringList() << "-d" << path << "-m");
				} else {
					proc->start("qscan", QStringList() << "-d" << path << "--force-plugin" << plugin << "-m");
				}
				break;
			case threadGetFeatures:
				proc->start("cdvdcontrol", QStringList() << "-d" << path << "-c");
				break;
			case threadGetASDB:
				proc->start("cdvdcontrol", QStringList() << "-d" << path << "--as-list");
				break;
			case threadMQCK:
				{
					QStringList cdvdopts;
					cdvdopts << "-d" << path;
					cdvdopts << "--mqck" << ((features.as_act_mode & ASMQCK_ADV) ? "advanced" : "quick");
					cdvdopts << "--mqck-speed" << QString::number(features.as_mqckspd);

#ifndef QT_NO_DEBUG
					qDebug() << cdvdopts;
#endif
					proc->start("cdvdcontrol", cdvdopts );
				}
				break;
			case threadAScre:
				{
					QStringList cdvdopts;
					cdvdopts << "-d" << path << "--as-create";
					cdvdopts << ((features.as_act_mode & ASCRE_FULL) ? "f" : "q");
					cdvdopts << ((features.as_act_mode & ASCRE_REPLACE) ? "r" : "a");

#ifndef QT_NO_DEBUG
					qDebug() << cdvdopts;
#endif
					proc->start("cdvdcontrol", cdvdopts );
				}
				break;
			case threadDestruct:
				proc->start("cdvdcontrol", QStringList() << "-d" << path << "--destruct" << (features.as_act_mode ? "full" : "quick"));
				break;
			case threadTattoo:
				proc->start("f1tattoo", QStringList() << "-d" << path << "--tattoo-raw" << features.tattoo_file);
				break;
			default:
				goto update_info_err;
		}
		if (!proc->waitForStarted(10000)) {
#ifndef QT_NO_DEBUG
			qDebug("Can't start qscan!");
#endif
			goto update_info_err;
		}
		QObject::connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)),
				this, SLOT(qscan_callback_info()));
#ifndef QT_NO_DEBUG
		qDebug("qscan (local) started");
#endif
		goto update_info_end;
	} else if (type == device::DevtypeTCP) {
		if ((threadType != threadDevice) && (threadType != threadMedia))
			goto update_info_err;

#ifndef QT_NO_DEBUG
		qDebug("device: TCP");
#endif
		sock = new QTcpSocket(this);
#ifndef QT_NO_DEBUG
		qDebug("socket created");
#endif
		io->setIODevice(sock);

//		QObject::connect(sock, SIGNAL(readyRead()),
//			this, SLOT(qscan_process_info()));

		sock->connectToHost(host, port);
		if (!sock->waitForConnected(5000)) {
#ifndef QT_NO_DEBUG
			qDebug("Unable to connect to host!");
#endif
			goto update_info_err;
		}

		sock->write("set dev=" + path.toLatin1() + "\n");
		switch (threadType) {
			case threadDevice:
				sock->write("dinfo\n");
				sock->write("close\n");
				break;
			case threadMedia:
				sock->write("minfo\n");
				sock->write("close\n");
				break;
			default:
				goto update_info_err;
		}
		QObject::connect(sock, SIGNAL(disconnected()),
				this, SLOT(qscan_callback_info()));
#ifndef QT_NO_DEBUG
		qDebug("qscan (TCP) started");
#endif
		goto update_info_end;
	}

update_info_err:
	qscan_callback_info();
#ifndef QT_NO_DEBUG
	qDebug() << "END: device::start_update_info(" << threadType << ")";
#endif
	return false;
update_info_end:
#ifndef QT_NO_DEBUG
	qDebug() << "END: device::start_update_info(" << threadType << ")";
#endif
	return true;
}

void device::qscan_callback_info()
{
	int xcode = 0;
	ThreadType ttype = threadType;
#ifndef QT_NO_DEBUG
	qDebug() << "STA: device::qscan_callback_info() " << this;
#endif
	io_mutex->lock();

	QObject::disconnect(io, SIGNAL(readyReadLine()),
		this, SLOT(qscan_process_info()));

	if (type == DevtypeLocal) {
//		QObject::disconnect(proc, SIGNAL(finished(int, QProcess::ExitStatus)),
//				this, SLOT(qscan_callback_info()));
		xcode = proc->exitCode();
#ifndef QT_NO_DEBUG
		qDebug() << "qscan (local) finished: " << xcode;
#endif

		disconnect(proc);
		io->setIODevice(NULL);
	} else if (type == device::DevtypeTCP) {
//		QObject::disconnect(sock, SIGNAL(disconnected()),
//				this, SLOT(qscan_callback_info()));
		sock->disconnectFromHost();
#ifndef QT_NO_DEBUG
		qDebug("qscan (TCP) finished");
#endif
		disconnect(sock);
		io->setIODevice(NULL);
	}
	io_mutex->unlock();
	threadType = threadNone;
	running=0;
	mutex->unlock();

	nprocess = "";
	emit process_finished();

	switch (ttype) {
		case threadDevice:
			emit doneDInfo(xcode);
			break;
		case threadMedia:
			emit doneMInfo(xcode);
			break;
		case threadGetFeatures:
			emit doneGetFeatures(xcode);
			break;
		case threadGetASDB:
			emit doneGetASDB(xcode);
			break;
		default:
			break;
	}

#ifndef QT_NO_DEBUG
	qDebug("END: device::qscan_callback_info()");
#endif
}

bool device::update_device_info()
{
	if (running) return false;
	plugin_names.clear();
	plugin_infos.clear();
	threadType = threadDevice;
	return start();
}

bool device::update_media_info()
{
#ifndef QT_NO_DEBUG
	qDebug("device::update_media_info()");
#endif
	if (running) return false;
	preserveMediaInfo = false;
	threadType = threadMedia;
	nprocess = tr("Updating media info...");
	emit process_started();
	return start();
}

void device::clear_media_info()
{
#ifndef QT_NO_DEBUG
	qDebug("device::clear_media_info()");
#endif
	clearMinfo();
	emit doneMInfo(0);
}

bool device::update_plugin_info()
{
	if (running) return false;
	preserveMediaInfo = true;
	threadType = threadMedia;
	return start();
}

bool device::getFeatures()
{
	if (running) return false;
	threadType = threadGetFeatures;
	return start();
}

bool device::getASDB()
{
	if (running) return false;
	threadType = threadGetASDB;
	asdb.clear();
	return start();
}

bool device::startMqck()
{
	if (running) return false;
	threadType = threadMQCK;
	return start();
}

bool device::startAScre()
{
	if (running) return false;
	threadType = threadAScre;
	return start();
}

bool device::startDestruct()
{
	if (running) return false;
	threadType = threadDestruct;
	return start();
}

bool device::startTattoo()
{
	if (running) return false;
	threadType = threadTattoo;
	return start();
}

bool device::setFeature(int f, bool en)
{
	int r=1;
#ifndef QT_NO_DEBUG
	qDebug() << "device::setFeature: " << f << en;
#endif
	if (!mutex->tryLock()) {
#ifndef QT_NO_DEBUG
		qDebug() << "Device busy: " << id;
#endif
		return false;
	}

	if (type == DevtypeLocal) {
		QStringList cdvdopts;

		pauseWatcher();

		cdvdopts << "-d" << path;
		switch(f) {
			case FEATURE_LOEJ:
				if (en) {
					cdvdopts << "--load" << "--loej-immed";
				} else {
					cdvdopts << "--eject" << "--loej-immed";
				}
				break;
			case FEATURE_LOEJ_TOGGLE:
				cdvdopts << "--loej" << "--loej-immed";
				break;
			case FEATURE_LOCK:
				if (en) {
					cdvdopts << "--lock";
				} else {
					cdvdopts << "--unlock";
				}
				break;
			case FEATURE_LOCK_TOGGLE:
				cdvdopts << "--lockt";
				break;
			case FEATURE_POWEREC:
				cdvdopts << "--powerec";
				cdvdopts << (en ? "on" : "off");
				break;
			case FEATURE_HIDECDR:
				cdvdopts << "--hcdr";
				cdvdopts << (en ? "on" : "off");
				break;
			case FEATURE_SINGLESESSION:
				cdvdopts << "--sss";
				cdvdopts << (en ? "on" : "off");
				break;
			case FEATURE_SPEEDREAD:
				cdvdopts << "--spdread";
				cdvdopts << (en ? "on" : "off");
				break;
			case FEATURE_BITSETR:
				cdvdopts << "--bitset+r";
				cdvdopts << (en ? "on" : "off");
				break;
			case FEATURE_BITSETRDL:
				cdvdopts << "--bitset+rdl";
				cdvdopts << (en ? "on" : "off");
				break;
			case FEATURE_SIMULPLUS:
				cdvdopts << "--dvd+testwrite";
				cdvdopts << (en ? "on" : "off");
				break;
			default:
				mutex->unlock();
				startWatcher();
				return false;
		}
#ifndef QT_NO_DEBUG
		qDebug() << cdvdopts.join(" ");
#endif
		r = QProcess::execute("cdvdcontrol", cdvdopts);
		unpauseWatcher();
	} else if (type == device::DevtypeTCP) {
		switch(f) {
			default:
				mutex->unlock();
				return false;
		}
	}
	mutex->unlock();
	return !!r;
}

bool device::setComplexFeature(int f, DevFeatures* data)
{
	int r=1;
#ifndef QT_NO_DEBUG
	qDebug() << "device::setComplexFeature: " << f;
#endif
	if (!data) {
#ifndef QT_NO_DEBUG
		qDebug() << "Data pointer is NULL";
#endif
		return 1;
	}
	if (!mutex->tryLock()) {
		qDebug() << "Device busy: " << id;
		return 1;
	}

	if (type == DevtypeLocal) {
		QStringList cdvdopts;

		cdvdopts << "-d" << path;
		switch(f) {
			case FEATURE_GIGAREC:
				if (data->enabled & FEATURE_GIGAREC) {
					cdvdopts << "--gigarec" << QString("%1").arg(data->grec, 3, 'f', 1);
				} else {
					cdvdopts << "--gigarec" << "off";
				}
				break;
			case FEATURE_VARIREC_CD:
				if (data->enabled & FEATURE_VARIREC_CD) {
					cdvdopts << "--varirec-cd" << QString::number(data->vrec_cd_pwr);
					if (data->supported & FEATURE_VARIREC_CDEXT) {
						cdvdopts << "--varirec-cd-strategy" << QString::number(data->vrec_cd_str-1);
					}
				} else {
					cdvdopts << "--varirec-cd" << "off";
				}
				break;
			case FEATURE_VARIREC_DVD:
				if (data->enabled & FEATURE_VARIREC_DVD) {
					cdvdopts << "--varirec-dvd" << QString::number(data->vrec_dvd_pwr);
					cdvdopts << "--varirec-dvd-strategy" << QString::number(data->vrec_dvd_str-1);
				} else {
					cdvdopts << "--varirec-dvd" << "off";
				}
				break;
			case FEATURE_SECUREC:
				if (data->enabled & FEATURE_SECUREC) {
					cdvdopts << "--securec" << data->sr_pass;
				} else {
					cdvdopts << "--nosecurec";
				}
				break;
			case FEATURE_SILENT:
				if (data->enabled & FEATURE_SILENT) {
					cdvdopts << "--silent" << "on";
					cdvdopts << "--sm-cd-rd" << QString::number(data->sm_cd_rd);
					cdvdopts << "--sm-cd-wr" << QString::number(data->sm_cd_wr);
					if (cap_rd & DEVICE_DVD) {
						cdvdopts << "--sm-dvd-rd" << QString::number(data->sm_dvd_rd);
//						cdvdopts << "--sm-dvd-wr" << QString::number(data->sm_dvd_wr);
					}
					cdvdopts << "--sm-access" << (data->sm_access ? "fast" : "slow");
					cdvdopts << "--sm-load" << QString::number(data->sm_trayl);
					cdvdopts << "--sm-eject" << QString::number(data->sm_traye);
				} else {
					cdvdopts << "--silent" << "off";
				}
				if (data->sm_nosave) cdvdopts << "--sm-nosave";
				break;
			case FEATURE_AS:
				switch (data->as_action) {
					case AS_ACTION_MODE:
						cdvdopts << "--as-mode";
						switch (data->as_mode) {
							case AS_MODE_OFF:
								cdvdopts << "off";
								break;
							case AS_MODE_AUTO:
								cdvdopts << "auto";
								break;
							case AS_MODE_ON:
								cdvdopts << "on";
								break;
							case AS_MODE_FORCED:
								cdvdopts << "forced";
								break;
							default:
								mutex->unlock();
								return 1;
						}
						break;
					case AS_ACTION_ACT:
						cdvdopts << "--as-on" << QString::number(data->as_idx);
						break;
					case AS_ACTION_DEACT:
						cdvdopts << "--as-off" << QString::number(data->as_idx);
						break;
					case AS_ACTION_DEL:
						cdvdopts << "--as-del" << QString::number(data->as_idx);
						break;
					case AS_ACTION_CLEAR:
						cdvdopts << "--as-clear";
						break;
					default:
						mutex->unlock();
						return 1;
				}
				break;
			case FEATURE_PIOQUIET:
				cdvdopts << "--pio-quiet";
				switch(data->pioq_quiet) {
					case PIOQ_QUIET:
						cdvdopts << "quiet";
						break;
					case PIOQ_STD:
						cdvdopts << "std";
						break;
					case PIOQ_PERF:
						cdvdopts << "perf";
						break;
					default:
						mutex->unlock();
						return 1;
				}
				cdvdopts << "--pio-limit";
				cdvdopts << ((features.enabled & FEATURE_PIOLIMIT) ? "on" : "off");	
				if (data->pioq_nosave)
					cdvdopts << "--pio-nosave";
				break;
			default:
				mutex->unlock();
				return 1;
		}
#ifndef QT_NO_DEBUG
		qDebug() << cdvdopts.join(" ");
#endif
		r = QProcess::execute("cdvdcontrol", cdvdopts);
	} else if (type == device::DevtypeTCP) {
		switch(f) {
			default:
				mutex->unlock();
				return 1;
		}
	}
	mutex->unlock();
	return !!r;
}

bool device::start_tests() 
{
	if (running) return false;
	threadType = threadTest;
	tests = test_req;
	return start();
}
	
bool device::stop_tests() 
{
	if (!running) return false;
	tests = 0;
	if (type == DevtypeLocal) {
		if (!proc) return false;
		Q_PID pid = proc->pid();
#if defined(__unix) || defined(__unix__)
		kill(pid, SIGINT);
#elif defined(_WIN32)
		TerminateProcess(pid, 0);
#endif
	} else if (type == device::DevtypeTCP) {
		if (!sock) return false;
#ifndef QT_NO_DEBUG
		qDebug("Scan terminate not implemented on network devices");
#endif
		sock->disconnectFromHost();
	}
	return true;
}

bool device::next_test()
{
	ctest=0;
	QString stest;
#ifndef QT_NO_DEBUG
	qDebug("STA: device::next_test()");
#endif
	pprocess = 0.0;
	if (tests & TEST_RT) {
		ctest = TEST_RT;
		stest = "rt";
		nprocess = tr("Read Transfer");
		testData.clearRT();
		test_spd = tspeeds.rt;
	} else if (tests & TEST_WT) {
		ctest = TEST_WT;
		stest = "wt";
		nprocess = tr("Write Transfer");
		testData.clearWT();
		test_spd = tspeeds.wt;
	} else if (tests & TEST_ERRC) {
		ctest = TEST_ERRC;
		stest = "errc";
		nprocess = tr("Error Correction");
		testData.clearErrc();
		test_spd = tspeeds.errc;
	} else if (tests & TEST_JB) {
		ctest = TEST_JB;
		stest = "jb";
		nprocess = tr("Jitter/Asymmetry");
		testData.clearJB();
		test_spd = tspeeds.jb;
	} else if (tests & TEST_FT) {
		ctest = TEST_FT;
		stest = "ft";
		nprocess = tr("Focus/Tracking");
		testData.clearFT();
		test_spd = tspeeds.ft;
	} else if (tests & TEST_TA) {
		ctest = TEST_TA;
		stest = "ta";
		nprocess = tr("Time Analyser");
		testData.clearTA();
//		test_spd = tspeeds.ta;
	}

	tests &= ~ctest;
	if (!ctest) {
		threadType = threadNone;
		running=0;
		mutex->unlock();
#ifndef QT_NO_DEBUG
		qDebug("END: device::next_test(): to tests remaining");
#endif
//		nprocess = "";
		emit testsDone();
		return false;
	}

#ifndef QT_NO_DEBUG
		qDebug() << "device::next_test(): starting test " << stest << " at speed " << test_spd;
#endif
/*
	run_test("rt");
	run_test("errc");
	run_test("jb");
	run_test("ft");
	run_test("ta");
*/

	if (type == DevtypeNone) {
		goto next_test_err;
	}
	if (proc) {
		delete proc;
		proc = NULL;
	}
	if (sock) {
		delete sock;
		sock = NULL;
	}

	emit process_started();

	QObject::connect(io, SIGNAL(readyReadLine()),
		this, SLOT(qscan_process_test()));

	if (type == DevtypeLocal) {
		QStringList qopts;
#ifndef QT_NO_DEBUG
		qDebug("device: LOCAL");
#endif
		proc = new QProcess(this);
#ifndef QT_NO_DEBUG
		qDebug("process created");
#endif
		io->setIODevice(proc);
		proc->setReadChannel(QProcess::StandardOutput);

//		QObject::connect(proc, SIGNAL(readyReadStandardOutput()),
//				this, SLOT(qscan_process_test()));

		qopts << "-d" << path << "-t" << stest << "-s" << QString::number(test_spd);
		if (stest == "wt" && !WT_simul)
			qopts << "-W";
		if (stest != "rt" && stest != "wt" && !plugin.isEmpty()) {
			qopts << "--force-plugin" << plugin;
		}

#if (!defined(QT_NO_DEBUG) && 0)
		for (int i=0;i<qopts.size();i++)
			qDebug("[" + QString::number(i) + "] "+ qopts[i]);
#endif
		proc->start("qscan", qopts);

		if (!proc->waitForStarted(10000)) {
#ifndef QT_NO_DEBUG
			qDebug("Can't run qscan!");
#endif
			goto next_test_err;
		}
		QObject::connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)),
				this, SLOT(qscan_callback_test()));
		gettimeofday(&timeSta, NULL);
#ifndef QT_NO_DEBUG
		qDebug("qscan (local) started");
#endif
		goto next_test_end;
	} else if (type == device::DevtypeTCP) {
#ifndef QT_NO_DEBUG
		qDebug("device: TCP");
#endif
		sock = new QTcpSocket(this);
#ifndef QT_NO_DEBUG
		qDebug("socket created");
#endif
		io->setIODevice(sock);

//		QObject::connect(sock, SIGNAL(readyRead()),
//			this, SLOT(qscan_process_test()));

		sock->connectToHost(host, port);
		if (!sock->waitForConnected(5000)) {
#ifndef QT_NO_DEBUG
			qDebug("Unable to connect to host!");
#endif
			goto next_test_err;
		}
		QObject::connect(sock, SIGNAL(disconnected()),
				this, SLOT(qscan_callback_test()));

		sock->write("set dev=" + path.toLatin1() + "\n");
		sock->write("set test=" + stest.toLatin1() + "\n");
		sock->write("set speed=" + QString::number(test_spd).toLatin1() + "\n");
		if (stest == "wt")
			sock->write("set simul=" + QString::number(WT_simul).toLatin1() + "\n");
		sock->write("run\n");
		sock->write("close\n");
		gettimeofday(&timeSta, NULL);
#ifndef QT_NO_DEBUG
		qDebug("qscan (TCP) started");
#endif
		goto next_test_end;
	}

next_test_err:
	qscan_callback_info();
#ifndef QT_NO_DEBUG
	qDebug("END: device::next_test()");
#endif
	return false;
next_test_end:
#ifndef QT_NO_DEBUG
	qDebug("END: device::next_test()");
#endif
	return true;
}

void device::qscan_process_info()
{
	QString qout;

#ifndef QT_NO_DEBUG
	qDebug("STA: qscan_process_info()");
#endif
	if (!io->IODevice()) {
#ifndef QT_NO_DEBUG
		qDebug("END: qscan_process_info(): QIODevice is NULL");
#endif
		return;
	}
	if (!io_mutex->tryLock()) {
#ifndef QT_NO_DEBUG
		qDebug("END: qscan_process_info(): Can't lock I/O Mutex");
#endif
		return;
	}
	while (io->linesAvailable() ) {
		qout = io->readLine();
//		qout.remove("\n");
		switch (threadType) {
			case threadDevice:
			case threadMedia:
				qscan_process_line(qout);
				break;
			case threadGetFeatures:
				cdvdcontrol_process_line(qout);
				break;
			case threadGetASDB:
				cdvdcontrol_process_asdb(qout);
				break;

			case threadMQCK:
			case threadAScre:
			case threadDestruct:
				cdvdcontrol_process_line(qout);
			case threadTattoo:
			default:
#ifndef QT_NO_DEBUG
				qDebug() << qout;
#endif
				break;
		}
	//	qDebug(qout.remove("\n"));
	} // while (io->bytesAvailable() )
	io_mutex->unlock();
#ifndef QT_NO_DEBUG
	qDebug("END: qscan_process_info()");
#endif
}

void device::qscan_process_line(QString& qout)
{
	QStringList sl;
#ifdef MINFO_TREE
	QTreeWidgetItem *info;
#endif
	QIcon ico_ok(":images/ok.png");
	QIcon ico_x (":images/x.png");
	QIcon ico_rd(":images/disc.png");
	QIcon ico_wr(":images/cdwriter.png");

	if (threadType==threadMedia && preserveMediaInfo) {
		if (!qout.startsWith("IM:")) return;
		// device inquiry string
		qout.remove(0,4);
#ifndef QT_NO_DEBUG
		qDebug() << qout;
#endif
		sl = qout.split(':');
		if (sl.size() <2 ) return;
		sl[1].remove('\'');
		while (!sl[1].isEmpty() && sl[1][0] == ' ') sl[1].remove(0,1);


		if (sl[0].contains("Available quality tests", Qt::CaseInsensitive)) {
			if (sl.size()>=2) {
				QStringList slt = sl[1].split(' ', QString::SkipEmptyParts);
				test_cap = 0;
				for (int ii=0; ii<slt.size(); ii++) {
					test_cap |= (slt[ii] == "errc") ? TEST_ERRC : 0;
					test_cap |= (slt[ii] == "jb")   ? TEST_JB : 0;
					test_cap |= (slt[ii] == "ft")   ? TEST_FT : 0;
					test_cap |= (slt[ii] == "ta")   ? TEST_TA : 0;
				}
			}
		} else if (sl[0].contains("ERRC speeds", Qt::CaseInsensitive)) {
			media.tspeeds_errc = sl[1].split(" ", QString::SkipEmptyParts);
		} else if (sl[0].contains("ERRC data", Qt::CaseInsensitive)) {
			QStringList td = sl[1].split(" ", QString::SkipEmptyParts);
			media.tdata_errc = 0;
			for (int ii=0; ii<td.size(); ii++) {
				if (td[ii] == "BLER")     { media.tdata_errc |= GRAPH_BLER; }
				else if (td[ii] == "E11") { media.tdata_errc |= GRAPH_E11; }
				else if (td[ii] == "E21") { media.tdata_errc |= GRAPH_E21; }
				else if (td[ii] == "E31") { media.tdata_errc |= GRAPH_E31; }
				else if (td[ii] == "E12") { media.tdata_errc |= GRAPH_E12; }
				else if (td[ii] == "E22") { media.tdata_errc |= GRAPH_E22; }
				else if (td[ii] == "E32") { media.tdata_errc |= GRAPH_E32; }
				else if (td[ii] == "PIE") { media.tdata_errc |= GRAPH_PIE; }
				else if (td[ii] == "PI8") { media.tdata_errc |= GRAPH_PI8; }
				else if (td[ii] == "PIF") { media.tdata_errc |= GRAPH_PIF; }
				else if (td[ii] == "POE") { media.tdata_errc |= GRAPH_POE; }
				else if (td[ii] == "PO8") { media.tdata_errc |= GRAPH_PO8; }
				else if (td[ii] == "POF") { media.tdata_errc |= GRAPH_POF; }
				else if (td[ii] == "LDC") { media.tdata_errc |= GRAPH_LDC; }
				else if (td[ii] == "BIS") { media.tdata_errc |= GRAPH_BIS; }
				else if (td[ii] == "UNCR"){ media.tdata_errc |= GRAPH_UNCR; }
			}
//			qDebug() << "Available ERRC data: " << media.tdata_errc;
		} else if (sl[0].contains("JB speeds", Qt::CaseInsensitive)) {
			media.tspeeds_jb = sl[1].split(" ", QString::SkipEmptyParts);
		}

		
		
		return;
	}


	if (qout.startsWith("ID:")) {
		// main device params
		qout.remove(0,4);
		sl = qout.split(':');
		if (sl.size() >=2 ) {
			sl[1].remove('\'');
			while (!sl[1].isEmpty() && sl[1][0] == ' ') sl[1].remove(0,1);

			if (sl[0].contains("Device", Qt::CaseInsensitive) && !sl[0].contains("capabilities", Qt::CaseInsensitive)) {
//				l_dev->setText(sl[1]);
#if 0
			} else if (sl[0].contains("Vendor", Qt::CaseInsensitive)) {
				l_vendor->setText(sl[1]);
			} else if (sl[0].contains("Model", Qt::CaseInsensitive)) {
				l_model->setText(sl[1]);
			} else if (sl[0].contains("F/W", Qt::CaseInsensitive)) {
				l_fw->setText(sl[1]);
#endif
			} else if (sl[0].contains("TLA", Qt::CaseInsensitive)) {
				if (sl[1].isEmpty()) {
					tla = "N/A";
				} else {
					tla = sl[1];
				}
			} else if (sl[0].contains("Discs loaded", Qt::CaseInsensitive)) {
				life_dn = sl[1].toInt();
			} else if (sl[0].contains("CD Rd", Qt::CaseInsensitive)) {
				sl.removeAt(0);
				life_cr = sl.join(":");
			} else if (sl[0].contains("CD Wr", Qt::CaseInsensitive)) {
				sl.removeAt(0);
				life_cw = sl.join(":");
			} else if (sl[0].contains("DVD Rd", Qt::CaseInsensitive)) {
				sl.removeAt(0);
				life_dr = sl.join(":");
			} else if (sl[0].contains("DVD Wr", Qt::CaseInsensitive)) {
				sl.removeAt(0);
				life_dw = sl.join(":");
			} else if (sl[0].contains("S/N", Qt::CaseInsensitive)) {
				if (sl[1].isEmpty()) {
					sn = "N/A";
				} else {
					sn = sl[1];
				}
			} else if (sl[0].contains("Buffer", Qt::CaseInsensitive)) {
				buf = sl[1];
			} else if (sl[0].contains("IFace", Qt::CaseInsensitive)) {
				iface = sl[1];
			} else if (sl[0].contains("Loader", Qt::CaseInsensitive)) {
				loader = sl[1];
			} else if (sl[0].contains("RPC Phase", Qt::CaseInsensitive)) {
				rpc_phase = sl[1].toInt();
			} else if (sl[0].contains("Region", Qt::CaseInsensitive)) {
				if (sl[1].contains("not set", Qt::CaseInsensitive))
					rpc_reg = -2;
				else
					rpc_reg = sl[1].toInt();
			} else if (sl[0].contains("Changes left", Qt::CaseInsensitive)) {
				rpc_ch = sl[1].toInt();
			} else if (sl[0].contains("Resets left", Qt::CaseInsensitive)) {
				rpc_rst = sl[1].toInt();
			} else if (sl[0].contains("Device Generic capabilities", Qt::CaseInsensitive)) {
				cap =    sl[1].toULongLong(0,16);
			} else if (sl[0].contains("Device Read capabilities", Qt::CaseInsensitive)) {
				cap_rd = sl[1].toULongLong(0,16);
			} else if (sl[0].contains("Device Write capabilities", Qt::CaseInsensitive)) {
				cap_wr = sl[1].toULongLong(0,16);
			}
		}
	} else if (qout.startsWith("CD:")) {
		// generic device capabilities
		/*
		qout.remove(0,4);
	proc->setReadChannel(QProcess::StandardOutput);
		sl = qout.split(':');
		if (sl.size() >=2 ) {

		}
		*/
	} else if (qout.startsWith("CM:")) {
		// media R/W capabilities
		/*
		bool rd,wr;
		qout.remove(0,4);
		sl = qout.split(':');
			if (sl.size() >=2 ) {
			sl[1].remove('\'');
			rd = sl[1].contains('R', Qt::CaseInsensitive);
			wr = sl[1].contains('W', Qt::CaseInsensitive);
#ifndef QT_NO_DEBUG
//				qDebug("|" + sl[0] + "|" + sl[1]);
#endif
		}
			*/
	} else if (qout.startsWith("IM:")) {
		// device inquiry string
		qout.remove(0,4);
#ifndef QT_NO_DEBUG
		qDebug() << qout;
#endif

		sl = qout.split(':');
		if (sl.size() >=2 ) {
			sl[1].remove('\'');
			while (!sl[1].isEmpty() && sl[1][0] == ' ') sl[1].remove(0,1);
#ifndef QT_NO_DEBUG
//			qDebug("|" + sl[0] + "|" + sl[1] + "|");
#endif
			if (sl[0].contains("Media type", Qt::CaseInsensitive)) {
				if (sl[1].contains("No Media", Qt::CaseInsensitive)) {
					media.type = "-";
					media.spd1X = 1;
				} else {
					media.type = sl[1];
					if (media.type.startsWith("CD")) {
						media.spd1X = 150;
					} else if (media.type.startsWith("DDCD")) {
						media.spd1X = 150;
					} else if (media.type.startsWith("DVD")) {
						media.spd1X = 1385;
					} else if (media.type.startsWith("BD")) {
						media.spd1X = 4495;
					}
				}
			} else if (sl[0].contains("Disc Category", Qt::CaseInsensitive)) {
				media.category = sl[1];
			} else if (sl[0].contains("Layers", Qt::CaseInsensitive)) {
				media.layers = sl[1];
				media.ilayers = sl[1].toInt();
				if (media.ilayers <= 0)
					media.ilayers = 1; 
			} else if (sl[0].contains("Protection", Qt::CaseInsensitive)) {
				media.prot = sl[1];
			} else if (sl[0].contains("Regions", Qt::CaseInsensitive)) {
				media.regions = sl[1];
			} else if (sl[0].contains("Erasable", Qt::CaseInsensitive)) {
				media.erasable = sl[1];
			} else if (sl[0].contains("Disc state", Qt::CaseInsensitive)) {
				media.dstate = sl[1];
			} else if (sl[0].contains("Session state", Qt::CaseInsensitive)) {
				media.sstate = sl[1];
			} else if (sl[0].contains("Read capacity", Qt::CaseInsensitive)) {
				sl.removeFirst();
				QStringList sl2 = sl.join(":").split("/");
				if (sl2.size()>=3) {
					media.creads = sl2[0].remove("sectors").toInt();
					media.creadm = sl2[1].remove("MB").toInt();
					media.creadmsf = sl2[2];
				}
			} else if (sl[0].contains("Free capacity", Qt::CaseInsensitive)) {
				sl.removeFirst();
				QStringList sl2 = sl.join(":").split("/");
				if (sl2.size()>=3) {
					media.cfrees = sl2[0].remove("sectors").toInt();
					media.cfreem = sl2[1].remove("MB").toInt();
					media.cfreemsf = sl2[2];
				}
			} else if (sl[0].contains("Total capacity", Qt::CaseInsensitive)) {
				sl.removeFirst();
				QStringList sl2 = sl.join(":").split("/");
				if (sl2.size()>=3) {
					media.ctots = sl2[0].remove("sectors").toInt();
					media.ctotm = sl2[1].remove("MB").toInt();
					media.ctotmsf = sl2[2];
				}
			} else if (sl[0].contains("Media ID", Qt::CaseInsensitive)) {
				sl.removeFirst();
				media.mid = sl.join(":");
				if (media.type.startsWith("DVD")) media.mid.remove(' ');
			} else if (sl[0].contains("Written on", Qt::CaseInsensitive)) {
				media.writer = sl[1];
#ifdef SHOW_SPEEDS
			} else if (sl[0].contains("RD speed max", Qt::CaseInsensitive)) {
				l_rd_max->setText(sl[1]);
			} else if (sl[0].contains("RD speed #", Qt::CaseInsensitive)) {
				c_rd_lst->addItem(sl[1]);		
			} else if (sl[0].contains("D WR speed max", Qt::CaseInsensitive)) {
				l_wr_max->setText(sl[1]);
			} else if (sl[0].contains("D WR speed #", Qt::CaseInsensitive)) {
				c_wr_lst->addItem(sl[1]);
#endif
/*
			} else if (sl[0].contains("Available quality tests", Qt::CaseInsensitive)) {
				if (sl.size()>=2) {
					QStringList slt = sl[1].split(' ', QString::SkipEmptyParts);
					test_cap = 0;
					for (int ii=0; ii<slt.size(); ii++) {
						test_cap |= (slt[ii] == "errc") ? TEST_ERRC : 0;
						test_cap |= (slt[ii] == "jb")   ? TEST_JB : 0;
						test_cap |= (slt[ii] == "ft")   ? TEST_FT : 0;
						test_cap |= (slt[ii] == "ta")   ? TEST_TA : 0;
					}
				}
*/
			}
		}
#ifdef MINFO_TREE
		info = new QTreeWidgetItem(QStringList(qout));
		info_media.append(info);
#endif
	} else if (qout.startsWith("SM:")) {
		qout.remove(0,4);
#ifndef QT_NO_DEBUG
		qDebug() << qout;
#endif
		sl = qout.split(':');
		if (sl.size() >=2 ) {
			sl[1].remove('\'');
			if (sl[0].contains("RD speed #", Qt::CaseInsensitive)) {
				sl[1].remove(QRegExp("\\([a-z,A-Z,0-9, /]*\\)"));
				sl[1].remove(' ');
				media.rspeeds.append(sl[1]);
			} else if (sl[0].contains("D WR speed #", Qt::CaseInsensitive)) {
				sl[1].remove(QRegExp("\\([a-z,A-Z,0-9, /]*\\)"));
				sl[1].remove(' ');
				media.wspeedsd.prepend(sl[1]);
			} else if (sl[0].contains("M WR speed #", Qt::CaseInsensitive)) {
				sl[1].remove(' ');
				media.wspeedsm.append(sl[1]);
			}

#ifdef MINFO_TREE
			info = new QTreeWidgetItem(QStringList(qout));
			info_media.append(info);
#endif
		}
	} else if (qout.startsWith("Found plugin:")) {
		int spidx;
		QString pn,pi;
		qout.remove(0,13);
		qout = qout.simplified();
		spidx = qout.indexOf(" ");
		pn = qout.mid(0,spidx).simplified();
		pi = qout.mid(spidx+1);
		pi.remove("("); pi.remove(")");

		if (!preserveMediaInfo) {
#ifndef QT_NO_DEBUG
			qDebug() << "Plugin: " << pn << pi;
#endif
			plugin_names.append(pn);
			plugin_infos.append(pi);
		}
	} else if (qout.contains("WARNING!!!  Detected locked PX-755/PX-760")) {
		plextor_lock = 1;
	}
}

void device::cdvdcontrol_process_line(QString& qout)
{
	QStringList sl;
	// vendor-specific device features
#ifndef QT_NO_DEBUG
	qDebug() << qout;
#endif
	sl = qout.split(':');
	if (sl.size() >=2 ) {
		sl[1] = sl[1].simplified();
		if (sl[0].contains("Lock state", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_LOCK;
				features.enabled   |= (sl[1].contains("ON") ? FEATURE_LOCK : 0);
			}
		} else if (sl[0].contains("Hide-CDR", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_HIDECDR;
				features.enabled   |= (sl[1].contains("ON") ? FEATURE_HIDECDR : 0);
			}
		} else if (sl[0].contains("SingleSession", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_SINGLESESSION;
				features.enabled   |= (sl[1].contains("ON") ? FEATURE_SINGLESESSION : 0);
			}
		} else if (sl[0].contains("SpeedRead", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_SPEEDREAD;
				features.enabled   |= (sl[1].contains("ON") ? FEATURE_SPEEDREAD : 0);
			}
		} else if (sl[0].contains("PSM Silent State", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_SILENT;
				features.enabled   |= (sl[1].contains("ON") ? FEATURE_SILENT : 0);
			}
		} else if (sl[0].contains("PSM CD Read speed", Qt::CaseInsensitive)) {
			features.psm_cd_rd = sl[1].remove("X").toInt();
		} else if (sl[0].contains("PSM CD Write speed", Qt::CaseInsensitive)) {
			features.psm_cd_wr = sl[1].remove("X").toInt();
		} else if (sl[0].contains("PSM DVD Read speed", Qt::CaseInsensitive)) {
			features.psm_dvd_rd = sl[1].remove("X").toInt();
//		} else if (sl[0].contains("SM DVD Write speed", Qt::CaseInsensitive)) {
//			features.sm_dvd_wr = sl[1].toInt();
		} else if (sl[0].contains("PSM Access time", Qt::CaseInsensitive)) {
			features.psm_access = !!sl[1].contains("FAST");
		} else if (sl[0].contains("PSM Load speed", Qt::CaseInsensitive)) {
			features.psm_trayl = sl[1].toInt();
		} else if (sl[0].contains("PSM Eject speed", Qt::CaseInsensitive)) {
			features.psm_traye = sl[1].toInt();
		} else if (sl[0].contains("SM Read speed", Qt::CaseInsensitive)) {
			if (media.type.startsWith("DVD")) {
				features.sm_dvd_rd = sl[1].remove("X").toInt();
			} else {
				features.sm_cd_rd = sl[1].remove("X").toInt();
			}
		} else if (sl[0].contains("SM Write speed", Qt::CaseInsensitive)) {
			if (!media.type.startsWith("DVD")) {
				features.sm_cd_wr = sl[1].remove("X").toInt();
			}
		} else if (sl[0].contains("SM Access time", Qt::CaseInsensitive)) {
			features.sm_access = !!sl[1].contains("FAST");
		} else if (sl[0].contains("SM Load speed", Qt::CaseInsensitive)) {
			features.sm_trayl = sl[1].toInt();
		} else if (sl[0].contains("SM Eject speed", Qt::CaseInsensitive)) {
			features.sm_traye = sl[1].toInt();

		} else if (sl[0].contains("PoweRec", Qt::CaseInsensitive) && !sl[0].contains("Speed", Qt::CaseInsensitive) ) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_POWEREC;
				features.enabled   |= (sl[1].contains("ON") ? FEATURE_POWEREC : 0);
			}
		} else if (sl[0].contains("PoweRec Speed", Qt::CaseInsensitive)) {
			features.prec_spd = sl[1].remove("X").remove("(CD)").remove("(DVD)").toInt();
		} else if (sl[0].contains("VariRec CD  power", Qt::CaseInsensitive)) {
			features.vrec_cd_pwr = (char) sl[1].toInt();
		} else if (sl[0].contains("VariRec CD  strategy", Qt::CaseInsensitive)) {
			int offs = sl[1].indexOf('[');
			if (offs>=0) {
				sl[1].remove(0,offs+1);
				sl[1].remove(']');
				features.vrec_cd_str = (char) sl[1].toInt();
			}
		} else if (sl[0].contains("VariRec CD", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_VARIREC_CDBASE;
				if (dev.startsWith("DVDR") || dev.startsWith("CD-R   PREMIUM"))
					features.supported |= FEATURE_VARIREC_CDEXT;
				features.enabled   |= (sl[1].contains("OFF") ? 0 : FEATURE_VARIREC_CD);
			}
		} else if (sl[0].contains("VariRec DVD power", Qt::CaseInsensitive)) {
			features.vrec_dvd_pwr = (char) sl[1].toInt();
		} else if (sl[0].contains("VariRec DVD strategy", Qt::CaseInsensitive)) {
			int offs = sl[1].indexOf('[');
			if (offs>=0) {
				sl[1].remove(0,offs+1);
				sl[1].remove(']');
				features.vrec_dvd_str = (char) sl[1].toInt();
			}
		} else if (sl[0].contains("VariRec DVD", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_VARIREC_DVD;
				features.enabled   |= (sl[1].contains("OFF") ? 0 : FEATURE_VARIREC_DVD);
			}
		} else if (sl[0].contains("GigaRec state", Qt::CaseInsensitive)) {
			features.supported |= FEATURE_GIGAREC;
			if (sl[1].contains("OFF")) {
				features.enabled &= ~FEATURE_GIGAREC;
				features.grec = 1.0;
			} else {
				features.enabled |= FEATURE_GIGAREC;
				features.grec = sl[1].toFloat();
			}
		} else if (sl[0].contains("Disc GigaRec rate", Qt::CaseInsensitive)) {
			if (sl[1].contains("OFF")) {
				media.grec = 1.0;
			} else {
				media.grec = sl[1].toFloat();
			}
		} else if (sl[0].contains("SecuRec", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_SECUREC;
				features.enabled   |= (sl[1].contains("ON") ? FEATURE_SECUREC : 0);
			}
		} else if (sl[0].contains("DVD+R bitset", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_BITSETR;
				features.enabled   |= (sl[1].contains("ON") ? FEATURE_BITSETR : 0);
			}
		} else if (sl[0].contains("DVD+R DL bitset", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_BITSETRDL;
				features.enabled   |= (sl[1].contains("ON") ? FEATURE_BITSETRDL : 0);
			}
		} else if (sl[0].contains("DVD+R(W) testwrite", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_SIMULPLUS;
				features.enabled   |= (sl[1].contains("ON") ? FEATURE_SIMULPLUS : 0);
			}
		} else if (sl[0].contains("AutoStrategy mode", Qt::CaseInsensitive)) {
			features.supported |= FEATURE_AS_BASE;
			if (dev.startsWith("DVDR   PX-755") || dev.startsWith("DVDR   PX-760"))
				features.supported |= FEATURE_AS_EXT;
			if (sl[1].contains("OFF", Qt::CaseInsensitive)) {
				features.as_mode = AS_MODE_OFF;
			} else if (sl[1].contains("AUTO", Qt::CaseInsensitive)) {
				features.as_mode = AS_MODE_AUTO;
			} else if (sl[1].contains("ON", Qt::CaseInsensitive)) {
				features.as_mode = AS_MODE_ON;
			} else if (sl[1].contains("FORCED", Qt::CaseInsensitive)) {
				features.as_mode = AS_MODE_FORCED;
			}
		} else if (sl[0].contains("Pioneer QuietMode", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_PIOQUIET;
				features.supported |= FEATURE_PIOLIMIT;
			}
		} else if (sl[0].contains("PlexEraser", Qt::CaseInsensitive)) {
			if (sl[1] != "-") {
				features.supported |= FEATURE_DESTRUCT;
			}
		} else if (sl[0].contains("Yamaha DiscT@2", Qt::CaseInsensitive)) {
			sl[1].remove("inner");
			sl[1].replace("outer", " ");
			sl[1].replace("image", " ");
			QStringList sl2 = sl[1].split(" ", QString::SkipEmptyParts);
#ifndef QT_NO_DEBUG
			qDebug () << "*** DISC T@2 ***\nsl[1]: " << sl[1] << "\nsl2 size: " << sl2.size();
#endif
			if (sl2.size() >= 3) {
				features.supported |= FEATURE_F1TATTOO;
				features.tattoo_inner = sl2[0].toInt();
				features.tattoo_outer = sl2[1].toInt();
				qDebug() << "Yamaha DiscT@2 radius:  inner " <<  features.tattoo_inner << ", outer " << features.tattoo_outer;
			}
//		} else if (sl[0].contains("MQCK", Qt::CaseInsensitive)) {
		} else if (sl[0] == "MQCK") {
			features.as_mqckres = sl[1];
		}
	}
}

void device::cdvdcontrol_process_asdb(QString& qout)
{
	ASDB_item it;
	QStringList sl;
/*
	it.present=1;
	it.active = 1;
	it.type   = "DVD+R";
	it.speed  = QString::number(8);
	it.mid    = "TYG02";
	it.writes = QString::number(25);
	asdb.append(it);
*/
#ifndef QT_NO_DEBUG
	qDebug("process_asdb");
#endif
	if (!qout.startsWith("S#")) return;
	qout.remove(0,2);
	qout.remove(" ");

	sl = qout.split('|');
	if (sl.size() <6) return;

	it.present=1;
	it.active = sl[1] == "*";
//	it.type   = sl[2].remove(QRegExp("[]"));
	it.type   = sl[2].remove("[A1]").remove("[25]");
	it.speed  = sl[3];
	it.mid    = sl[4];
	it.writes = sl[5];

	asdb.append(it);
}

void device::qscan_callback_test()
{
	int xcode = 0;
#ifndef QT_NO_DEBUG
	qDebug("STA: qscan_callback_test()");
#endif
	timeval timeEnd;
	int time;

	qscan_process_test();
	io_mutex->lock();

	QObject::disconnect(io, SIGNAL(readyReadLine()),
		this, SLOT(qscan_process_test()));

	if (type == DevtypeLocal) {
//		QObject::disconnect(proc, SIGNAL(finished(int, QProcess::ExitStatus)),
//				this, SLOT(qscan_callback_info()));
		xcode = proc->exitCode();
#ifndef QT_NO_DEBUG
		qDebug() << "qscan (local) finished" << xcode;
#endif
		disconnect(proc);
		io->setIODevice(NULL);
	} else if (type == device::DevtypeTCP) {
//		QObject::disconnect(sock, SIGNAL(disconnected()),
//				this, SLOT(qscan_callback_info()));
		sock->disconnectFromHost();
#ifndef QT_NO_DEBUG
		qDebug("qscan (TCP) finished");
#endif
		disconnect(sock);
		io->setIODevice(NULL);
	}
	io_mutex->unlock();
	nprocess = "";
	emit process_finished();

	gettimeofday(&timeEnd, NULL);
	time = (int) ((timeEnd.tv_sec - timeSta.tv_sec) + (timeEnd.tv_usec - timeSta.tv_usec)/1000000.0);

	switch(ctest) {
		case TEST_RT:
			testData.rt_time = time;
			emit block_RT();
			break;
		case TEST_WT:
			testData.wt_time = time;
			emit block_WT();
			break;
		case TEST_ERRC:
			testData.errc_time = time;
			emit block_ERRC();
			break;
		case TEST_JB:
			testData.jb_time = time;
			emit block_JB();
			break;
		case TEST_FT:
			testData.ft_time = time;
			emit block_FT();
			break;
		case TEST_TA:
			testData.ta_time = time;
			emit block_TA();
			break;
		default:
			break;
	}

	if (xcode) {
		emit testsError();
	}

	next_test();

#ifndef QT_NO_DEBUG
	qDebug("END: qscan_callback_test()");
#endif
}

void device::qscan_process_test()
{
	timeval timeEnd;
	float time;
	QString qout;
	QStringList sl;
#ifndef QT_NO_DEBUG
//	qDebug("STA: qscan_process_test()");
#endif
	if (!io->IODevice()) {
#ifndef QT_NO_DEBUG
		qDebug("END: qscan_process_test(): QIODevice is NULL");
#endif
		return;
	}
	if (!io_mutex->tryLock()) {
#ifndef QT_NO_DEBUG
		qDebug("END: qscan_process_test(): Can't lock I/O Mutex");
#endif
		return;
	}
	while (io->linesAvailable()) {
		qout = io->readLine();
		qout.remove("\n");
		qout.remove("\r");
#ifndef QT_NO_DEBUG
		qDebug() << qout;
#endif
		gettimeofday(&timeEnd, NULL);
		time = (timeEnd.tv_sec - timeSta.tv_sec) + (timeEnd.tv_usec - timeSta.tv_usec)/1000000.0;
		switch(ctest) {
			case TEST_RT:
#ifndef DISABLE_INTERNAL_WT
			case TEST_WT:
#endif
				if (qout.startsWith("lba") && qout.contains("speed")) {
//					qDebug("RT");
					DI_Transfer di;
					sl = qout.split(" ", QString::SkipEmptyParts);
			//		for (int i=0; i<sl.size(); i++) qDebug(QString::number(i) + "  '" + sl[i] + "'");
					if (sl.size()>=6) {
						di.lba = sl[1].toLongLong();
						di.spdx = sl[3].toDouble();
						di.spdk = sl[5].toInt();
#ifndef DISABLE_INTERNAL_WT
						if (ctest == TEST_WT) {
	 						testData.wt.append(di);
							testData.wt_time = time;
							emit block_WT();

							pprocess = 100.0 * (float) di.lba / media.ctots;
							emit process_progress();
						} else if (ctest == TEST_RT)
#endif
						{
							testData.rt.append(di);
							testData.rt_time = time;
							emit block_RT();

							pprocess = 100.0 * (float) di.lba / media.creads;
							emit process_progress();
						}
					}
			//		qDebug(QString("lba: %1, spdx: %2, spdk: %3").arg(di.lba).arg(di.spdx).arg(di.spdk));
				} else if (qout.startsWith("Reading blocks") || qout.startsWith("Starting write")) {
					gettimeofday(&timeSta, NULL);
				}
				break;
#ifdef DISABLE_INTERNAL_WT
			case TEST_WT:
				if (qout.startsWith("Track ") && qout.contains("MB written")) {
					DI_Transfer di;
					sl = qout.split(" ", QString::SkipEmptyParts);
			//		for (int i=0; i<sl.size(); i++) qDebug(QString::number(i) + "  '" + sl[i] + "'");
					if (sl.size()>=10) {
						di.lba = sl[2].toLongLong() << 9;
						if (sl[3] == "of") {
							if (sl.size() >=12) {
								di.spdx = sl[11].remove("x.").toDouble();
								di.spdk = (uint32_t)(di.spdx * media.spd1X);
								testData.wt.append(di);
								testData.wt_time = time;
								emit block_WT();
							}
						} else {
							if (sl.size() >=10) {
								di.spdx = sl[9].remove("x.").toDouble();
								di.spdk = (uint32_t)(di.spdx * media.spd1X);
								testData.wt.append(di);
								testData.wt_time = time;
								emit block_WT();
							}
						}
						pprocess = 100.0 * (float) di.lba / media.ctots;
						emit process_progress();
//						qDebug( sl[1] + " " + sl[3] + " " + sl[5] );
					}
			//		qDebug(QString("lba: %1, spdx: %2, spdk: %3").arg(di.lba).arg(di.spdx).arg(di.spdk));
				}
				break;
#endif
			case TEST_ERRC:
				if (qout.startsWith("cur")) {
					DI_Errc di;
					sl = qout.split(QRegExp("[:\\ |]"), QString::SkipEmptyParts);
					// for (int i=0; i<sl.size(); i++) qDebug(QString::number(i) + "  '" + sl[i] + "'");

					if (media.type.startsWith("CD") && sl.size()>=14) {
						di.cd.lba = sl[1].toInt();
						di.cd.spdx= sl[2].toFloat();

						di.cd.bler = sl[6].toInt();
						di.cd.e11  = sl[7].toInt();
						di.cd.e21  = sl[8].toInt();
						di.cd.e31  = sl[9].toInt();
						di.cd.e12  = sl[10].toInt();
						di.cd.e22  = sl[11].toInt();
						di.cd.e32  = sl[12].toInt();
						di.cd.uncr = sl[13].toInt();

						ErrcADD(&testData.errcTOT, di);
						ErrcMAX(&testData.errcMAX, di);
						CDErrcAVG(&testData.errcAVG, &testData.errcTOT, di.cd.lba/75);
						testData.errc.append(di);
						testData.errc_time = time;
						emit block_ERRC();

						pprocess = 100.0 * (float) di.cd.lba / media.creads;
						emit process_progress();
					} else if (media.type.startsWith("DVD") && sl.size()>=13) {
						di.dvd.lba  = sl[1].toInt();
						di.dvd.spdx = sl[2].toFloat();

						di.dvd.res  = 0;
						di.dvd.pie  = sl[6].toInt();
						di.dvd.pi8  = sl[7].toInt();
						di.dvd.pif  = sl[8].toInt();
						di.dvd.poe  = sl[9].toInt();
						di.dvd.po8  = sl[10].toInt();
						di.dvd.pof  = sl[11].toInt();
						di.dvd.uncr = sl[12].toInt();

						ErrcADD(&testData.errcTOT, di);
						ErrcMAX(&testData.errcMAX, di);
						DVDErrcAVG(&testData.errcAVG, &testData.errcTOT, di.dvd.lba >> 4);
						testData.errc.append(di);
						testData.errc_time = time;
						emit block_ERRC();

						pprocess = 100.0 * (float) di.dvd.lba / media.creads;
						emit process_progress();
					} else if (media.type.startsWith("BD") && sl.size()>=9) {
						di.bd.lba  = sl[1].toInt();
						di.bd.spdx = sl[2].toFloat();

						di.bd.res0 = 0;
						di.bd.ldc  = sl[6].toInt();
						di.bd.res1 = 0;
						di.bd.res2 = 0;
						di.bd.bis  = sl[7].toInt();
						di.bd.res3 = 0;
						di.bd.res4 = 0;
						di.bd.uncr = sl[8].toInt();

						ErrcADD(&testData.errcTOT, di);
						ErrcMAX(&testData.errcMAX, di);
						DVDErrcAVG(&testData.errcAVG, &testData.errcTOT, di.dvd.lba >> 4);
						testData.errc.append(di);
						testData.errc_time = time;
						emit block_ERRC();

						pprocess = 100.0 * (float) di.bd.lba / media.creads;
						emit process_progress();
					}
				}
				break;
			case TEST_JB:
				if (qout.startsWith("cur")) {
					DI_JB di;
					sl = qout.split(QRegExp("[:\\ |]"), QString::SkipEmptyParts);
//					for (int i=0; i<sl.size(); i++) qDebug(QString::number(i) + "'" + sl[i] + "'");

					if (sl.size()>=8) {
						di.lba    = sl[1].toInt();
						di.spdx   = sl[2].toFloat();

						di.jitter = sl[6].toFloat();
						di.asymm  = sl[7].toFloat();

						if (!testData.jb.size()) {
							testData.jbMM.jmin = di.jitter;
							testData.jbMM.jmax = di.jitter;
							testData.jbMM.bmin = di.asymm;
							testData.jbMM.bmax = di.asymm;
						} else {
							if (testData.jbMM.jmin > di.jitter) testData.jbMM.jmin = di.jitter;
							if (testData.jbMM.jmax < di.jitter) testData.jbMM.jmax = di.jitter;
							if (testData.jbMM.bmin > di.asymm)  testData.jbMM.bmin = di.asymm;
							if (testData.jbMM.bmax < di.asymm)  testData.jbMM.bmin = di.asymm;
						}

						testData.jb.append(di);
						testData.jb_time = time;
						emit block_JB();

						pprocess = 100.0 * (float) di.lba / media.creads;
						emit process_progress();
					}
				}
				break;
			case TEST_FT:
				if (qout.startsWith("cur")) {
					DI_FT di;
					sl = qout.split(QRegExp("[:\\ |]"), QString::SkipEmptyParts);
//					for (int i=0; i<sl.size(); i++) qDebug(QString::number(i) + "'" + sl[i] + "'");

					if (sl.size()>=8) {
						di.lba = sl[1].toInt();
						di.spdx= sl[2].toFloat();

						di.fe  = sl[6].toInt();
						di.te  = sl[7].toInt();

						if (testData.ftMAX.fe < di.fe) testData.ftMAX.fe = di.fe;
						if (testData.ftMAX.te < di.te) testData.ftMAX.te = di.te;

						testData.ft.append(di);
						testData.ft_time = time;
						emit block_FT();

						pprocess = 100.0 * (float) di.lba / media.ctots;
						emit process_progress();
					}
				}
				break;
			case TEST_TA:
				if (qout.startsWith("TA")) {
					if (taIdx <0 || taIdx>5) break;
					DI_TA di;
					sl = qout.split(" ", QString::SkipEmptyParts);
					if (sl.size() >=4) {
						di.idx  = sl[1].toInt();
						di.pit  = sl[2].toInt();
						di.land = sl[3].toInt();
					//	if (!di.idx) testData.ta[taIdx].clear();
						testData.ta[taIdx].append(di);
						testData.ta_time = time;
						emit block_TA();
					}
				} else if (qout.startsWith("Running TA on")) {
					int taLayer=0;
					int taZone=0;
					sl = qout.split(" ", QString::SkipEmptyParts);
					if (sl.size() >=6) {
						sl[3].remove("L");
#ifndef QT_NO_DEBUG
						qDebug() << "TA Layer: " << sl[3];
#endif
						taLayer = sl[3].toInt();
						if (sl[4] == "inner") {
							taZone = 0;
						} else if (sl[4] == "middle") {
							taZone = 1;
						} else if (sl[4] == "outer") {
							taZone = 2;
						}
						taIdx = taLayer*3 + taZone;

						pprocess = 100.0 * (float) taIdx / ( media.ilayers*3);
						emit process_progress();
					}
				}
				//	parsing TA test output...

				break;
			default:
				break;
		}

	}
	io_mutex->unlock();
#ifndef QT_NO_DEBUG
//	qDebug("END: qscan_process_test()");
#endif
}

void device::save(QIODevice *f)
{
#ifndef QT_NO_DEBUG
	qDebug("device::save()");
#endif
	resWriter->setIO(f);
	resWriter->start();
}

bool device::isSaving() { return resWriter->isRunning(); };
bool device::saveResult() { return resWriter->result(); };

void device::load(QIODevice *f)
{
#ifndef QT_NO_DEBUG
	qDebug("device::load()");
#endif
	resReader->setIO(f);
	resReader->start();
}

bool device::isLoading() { return resReader->isRunning(); };
bool device::loadResult() { return resReader->result(); };

void device::startWatcher()
{
	if (mwatcher) return;
	mwatcher = new MediaWatcher(this);
	connect(mwatcher, SIGNAL(started()), this, SLOT(watcherStarted()));
	connect(mwatcher, SIGNAL(finished()), this, SLOT(watcherStoped()));

	connect(mwatcher, SIGNAL(mediaLoading()), this, SLOT(watcherEventLoading()));
	connect(mwatcher, SIGNAL(mediaRemoved()), this, SLOT(watcherEventRemoved()));
	connect(mwatcher, SIGNAL(mediaNew()),     this, SLOT(watcherEventNew()));
	connect(mwatcher, SIGNAL(mediaNoMedia()), this, SLOT(watcherEventNoMedia()));

	mwatcher->start();
}

void device::stopWatcher()
{
	if (!mwatcher) return;
	mwatcher->stop();
	mwatcher->wait(3000);
}

void device::pauseWatcher()   { if (!mwatcher) return; mwatcher->pause(); }
void device::unpauseWatcher() { if (!mwatcher) return; mwatcher->unPause(); }

void device::watcherStarted() 
{
#ifndef QT_NO_DEBUG
	qDebug() << "device: " << path << ": watcher started";
#endif
}

void device::watcherStoped()
{
#ifndef QT_NO_DEBUG
	qDebug() << "device: " << path << ": watcher stoped";
#endif
	mwatcher->disconnect();
	mwatcher->deleteLater();
	mwatcher = NULL;
}

void device::watcherEventLoading()
{
	nprocess = tr("Loading media...");
	emit process_started();
}

void device::watcherEventRemoved()
{
#ifndef QT_NO_DEBUG
	qDebug("device::watcherEventRemoved()");
#endif
	autoupdate = 1;
	clear_media_info();
}

void device::watcherEventNew()
{ 
#ifndef QT_NO_DEBUG
	qDebug("device::watcherEventNew()");
#endif
	autoupdate = 1;
	update_media_info(); 
}

void device::watcherEventNoMedia()
{
	nprocess = "";
	emit process_finished();
}

void device::resLoaderDone() {
	if (!resReader->result()) return;

	emit doneDInfo(0);
	emit doneMInfo(0);
};

