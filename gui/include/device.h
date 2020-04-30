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

#ifndef _DEVICE_H
#define _DEVICE_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QMutex>
#include <QIODevice>

#include <sys/time.h>
#include <inttypes.h>

#define MINFO_TREE
//#define FT_AVG
//#define SHOW_SPEEDS

#define TEST_MAX 6
#define TEST_RT    0x0001
#define TEST_WT    0x0002
#define TEST_ERRC  0x0004
#define TEST_JB    0x0008
#define TEST_FT    0x0010
#define TEST_TA    0x0020

#ifdef MINFO_TREE
class QTreeWidgetItem;
#endif
class QPxIODevice;
class QProcess;
class QTcpSocket;

#define GRAPH_BLER  1
#define GRAPH_E11	(1 << 1)
#define GRAPH_E21	(1 << 2)
#define GRAPH_E31	(1 << 3)
#define GRAPH_E12	(1 << 4)
#define GRAPH_E22	(1 << 5)
#define GRAPH_E32	(1 << 6)

#define GRAPH_PIE	(1 << 1)
#define GRAPH_PI8	(1 << 2)
#define GRAPH_PIF	(1 << 3)
#define GRAPH_POE	(1 << 4)
#define GRAPH_PO8	(1 << 5)
#define GRAPH_POF	(1 << 6)

#define GRAPH_LDC	(1 << 1)
#define GRAPH_BIS	(1 << 4)

#define GRAPH_UNCR	(1 << 7)


#define FEATURE_POWEREC			0x00000001
#define FEATURE_HIDECDR			0x00000002
#define FEATURE_SINGLESESSION	0x00000004
#define FEATURE_SPEEDREAD	    0x00000008
#define FEATURE_BITSETR			0x00000010
#define FEATURE_BITSETRDL		0x00000020
#define FEATURE_SIMULPLUS		0x00000040

#define FEATURE_GIGAREC			0x00000080
#define FEATURE_VARIREC_CDBASE	0x00000100
#define FEATURE_VARIREC_CDEXT	0x00000200
#define FEATURE_VARIREC_CD		( FEATURE_VARIREC_CDBASE | FEATURE_VARIREC_CDEXT)
#define FEATURE_VARIREC_DVD		0x00000400
#define FEATURE_VARIREC			( FEATURE_VARIREC_CD | FEATURE_VARIREC_DVD )
#define FEATURE_SECUREC			0x00000800
#define FEATURE_SILENT			0x00001000

#define FEATURE_AS_BASE			0x00002000
#define FEATURE_AS_EXT			0x00004000
#define FEATURE_AS				( FEATURE_AS_BASE | FEATURE_AS_EXT )
#define FEATURE_DESTRUCT		0x00008000
#define FEATURE_F1TATTOO		0x00010000
#define FEATURE_PIOLIMIT		0x00020000
#define FEATURE_PIOQUIET		0x00040000

#define FEATURE_LOEJ			0x00080000
#define FEATURE_LOEJ_TOGGLE		0x00100000
#define FEATURE_LOCK			0x00200000
#define FEATURE_LOCK_TOGGLE		0x00400000

#define AS_ACTION_MODE	1
#define AS_ACTION_ACT	2
#define AS_ACTION_DEACT	3
#define AS_ACTION_DEL	4
#define AS_ACTION_CLEAR	5
#define AS_ACTION_CRE	6
#define AS_ACTION_MQCK	7

#define AS_MODE_OFF		0
#define AS_MODE_AUTO	1
#define AS_MODE_ON		2
#define AS_MODE_FORCED	3

#define ASCRE_FULL		8
#define ASCRE_REPLACE	16

#define ASMQCK_ADV		32

#define PIOQ_QUIET		0
#define PIOQ_STD		1
#define PIOQ_PERF		2

#define QPX_FILE_SIGN   "QPXD"

#if 0
// chunk. cnout = 1
#define META_TEST_ERRC_SUMMARY	57
#define META_TEST_JB_SUMMARY	58
#define META_TEST_FT_SUMMARY	59
#endif

struct ASDB_item {
	bool present, active;
	QString type;
	QString mid,speed,writes; 
};

typedef QList<ASDB_item> ASDB;

struct DevFeatures {
	int supported;
	int enabled;

	int		prec_spd;
	float	grec;
	char	vrec_cd_pwr;
	int		vrec_cd_str;
	char	vrec_dvd_pwr;
	int		vrec_dvd_str;

	QString sr_pass;

	int		sm_cd_rd,
			sm_cd_wr,
			sm_dvd_rd;
	bool	sm_access; // 0 = slow, 1 = fast
	int		sm_trayl, sm_traye;
	bool	sm_nosave;

	int		psm_cd_rd,
			psm_cd_wr,
			psm_dvd_rd;
//			psm_dvd_wr;
	bool	psm_access; // 0 = slow, 1 = fast
	int		psm_trayl, psm_traye;

	int		tattoo_inner,
			tattoo_outer;
	QString tattoo_file;

	int		as_action;
	int		as_act_mode;
	int		as_mode;
	int		as_idx;
	int		as_mqckspd;
	QString as_mqckres;
	int		pioq_quiet;
	bool	pioq_nosave;
};

struct DI_Transfer {
	quint64  lba;
	float    spdx;
	uint32_t spdk;
};

template <typename T>
struct Errc_ARRAY {
	quint64 lba;
	float	spdx;
	T		err[8];
};

template <typename T>
struct Errc_CD {
	quint64 lba;
	float   spdx;
	T bler;
	T e11, e21, e31;
	T e12, e22, e32;
	T uncr;
};

template <typename T>
struct Errc_DVD {
	quint64 lba;
	float   spdx;
	T res;
	T pie, pi8, pif;
	T poe, po8, pof;
	T uncr;
};

template <typename T>
struct Errc_BD {
	quint64 lba;
	float   spdx;
	T res0;
	T ldc, res1, res2;
	T bis, res3, res4;
	T uncr;
};

template <typename T>
union Errc {
	Errc_ARRAY<T> raw;
	Errc_CD<T>    cd;
	Errc_DVD<T>   dvd;
	Errc_BD<T>    bd;
};

typedef Errc<int>     DI_Errc;
typedef Errc<int64_t> TOT_Errc;
typedef Errc<float>   AVG_Errc;

void ErrcADD(TOT_Errc *tot, const DI_Errc& o);
void ErrcMAX(DI_Errc *max, const DI_Errc& o);
void CDErrcAVG(AVG_Errc *avg, TOT_Errc *tot, uint64_t blocks);
void DVDErrcAVG(AVG_Errc *avg, TOT_Errc *tot, uint64_t blocks);
void BDErrcAVG(AVG_Errc *avg, TOT_Errc *tot, uint64_t blocks);

struct DI_JB {
	quint64 lba;
	float   spdx;
	float   jitter;
	float   asymm;
};

struct MM_JB {
	float jmin;
	float jmax;
	float bmin;
	float bmax;
};

struct DI_FT {
	quint64 lba;
	float   spdx;
	quint32 fe;
	quint32 te;
};

struct DI_TA {
	//float T;
	int   idx;
	int   pit;
	int   land;
};

class TestData {
public:
	TestData() {};
	~TestData() {};
	void clear() {
		clearRT();
		clearWT();
		clearErrc();
		clearJB();
		clearFT();
		clearTA();
	};

	inline void clearRT() {
		rt.clear();
		rt_time = 0;
	};
	inline void clearWT() {
		wt.clear();
		wt_time = 0;
	};
	inline void clearErrc() {
		errc.clear();
		errc_time = 0;

		errcTOT.raw.spdx=0.0;  for (int i=0; i<8; i++) errcTOT.raw.err[i]=0;
		errcMAX.raw.spdx=0.0;  for (int i=0; i<8; i++) errcMAX.raw.err[i]=0;
		errcAVG.raw.spdx=0.0;  for (int i=0; i<8; i++) errcAVG.raw.err[i]=0.0;
	};
	inline void clearJB() {
		jb.clear();
		jb_time = 0;

		jbMM.jmin=0.0;
		jbMM.jmax=0.0;
		jbMM.bmin=0.0;
		jbMM.bmax=0.0;
	};
	inline void clearFT() {
		ft.clear();
		ft_time = 0;

		ftMAX.fe = 0;
		ftMAX.te = 0;
	};
	inline void clearTA() {
		for (int i=0; i<6; i++) ta[i].clear();
		ta_time = 0;
	};

	QList<DI_Transfer> rt;
	QList<DI_Transfer> wt;
	QList<DI_Errc>     errc;
	QList<DI_JB>	   jb;
	QList<DI_FT>	   ft;
	QList<DI_TA>	   ta[6];

	TOT_Errc	errcTOT;
	DI_Errc		errcMAX;
	AVG_Errc	errcAVG;
	MM_JB		jbMM;	
	DI_FT		ftMAX;

	float		rt_time;
	float		wt_time;
	float		errc_time;
	float		jb_time;
	float		ft_time;
	float		ta_time;
};

struct MediaInfo {
	QString		type;
	QString		label;

//	bool		isCD;
//	bool		isDVD;

	int			spd1X;
//	int			itype;
	QString		category;
	QString		mid;
	QString		layers;
	int			ilayers;
	QString		prot;
	QString		regions;

	int			creads;
	int			creadm;
	QString		creadmsf;
	int			cfrees;
	int			cfreem;
	QString		cfreemsf;
	int			ctots;
	int			ctotm;
	QString		ctotmsf;

	QString		erasable;
	QString		dstate;
	QString		sstate;
	QStringList rspeeds;
	QStringList wspeedsd;
	QStringList wspeedsm;
	QStringList tspeeds_errc;
	int			tdata_errc;
	QStringList tspeeds_jb;

	float		grec;
	QString		writer;
};

struct TestSpeeds {
	int32_t rt, wt, errc, jb, ft;
};

class MediaWatcher;
class ResultsReader;
class ResultsWriter;

class device : public QObject
{
	Q_OBJECT
public:
	enum devtype {
		DevtypeNone    = 0,
		DevtypeVirtual = 1,
		DevtypeLocal   = 2,
		DevtypeTCP     = 3
	};

	enum ThreadType {
		threadNone		= 0,
		threadDevice	= 1,
		threadMedia		= 2,
		threadTest		= 3,
		threadGetFeatures = 4,
		threadGetASDB	= 5,
		threadMQCK		= 6,
		threadAScre		= 7,
		threadDestruct	= 8,
		threadTattoo	= 9
	};

	device(QObject* p);
	~device();

	bool isRunning();
	void clearMinfo();

	bool update_device_info();
	bool update_media_info();
	void clear_media_info();
	bool update_plugin_info();
	bool getFeatures();
	bool getASDB();
	bool setFeature(int, bool);
	bool setComplexFeature(int, DevFeatures*);

	bool startMqck();
	bool startAScre();
	bool startDestruct();
	bool startTattoo();

	bool start_tests();
	bool stop_tests();

	void startWatcher();
	void stopWatcher();
	void pauseWatcher();
	void unpauseWatcher();

	void save(QIODevice*);
	bool isSaving();
	bool saveResult();
	void load(QIODevice*);
	bool isLoading();
	bool loadResult();

devtype		type;
	QString		host;
	uint16_t	port;
	QString		path;
	QString		id;

	QString ven;
	QString dev;
	QString fw;
	QString tla;
	QString sn;
	QString buf;
	QString iface;
	QString loader;

	int32_t	life_dn;
	QString life_cr,
			life_cw,
			life_dr,
			life_dw;

	int8_t	rpc_phase;
	int8_t	rpc_reg;
	int8_t	rpc_ch;
	int8_t	rpc_rst;

	bool	info_set;

	uint64_t cap;
	uint64_t cap_rd;
	uint64_t cap_wr;

	bool	plextor_lock;	

	int		test_cap;
	int		test_req;
	int		tests, ctest;
	int		test_spd;
	QString		plugin;
	QStringList plugin_names;
	QStringList plugin_infos;
	QString		nprocess;
	float		pprocess;

	DevFeatures	features;
	ASDB		asdb;
	MediaInfo	media;
	TestData	testData;
	TestSpeeds	tspeeds;
	bool		WT_simul;
	timeval		timeSta;

	QMutex	   *mutex;
	QMutex	   *io_mutex;

	QProcess   *proc;
	QTcpSocket *sock;
	QPxIODevice  *io;

	ResultsReader *resReader;
	ResultsWriter *resWriter;

	bool		autoupdate;
#ifdef MINFO_TREE
	QList<QTreeWidgetItem*> info_media;
#endif
/*
	uint64_t caps;
	uint64_t caps_rd;
	uint64_t caps_wr;
*/

private slots:
	void qscan_process_info();
	void qscan_process_test();
	void qscan_callback_info();
	void qscan_callback_test();

	void watcherStarted();
	void watcherStoped();
	void watcherEventLoading();
	void watcherEventRemoved();
	void watcherEventNew();
	void watcherEventNoMedia();

	void resLoaderDone();

signals:
	void mediaRemoved();
	void mediaNew();
	void doneDInfo(int);
	void doneMInfo(int);
	void doneGetFeatures(int);
	void doneGetASDB(int);

	void process_started();
	void process_finished();
	void process_progress();

	void testsDone();
	void testsError();

	void block_RT();
	void block_WT();
	void block_ERRC();
	void block_JB();
	void block_FT();
	void block_TA();

private:
	void qscan_process_line(QString&);
	void cdvdcontrol_process_line(QString&);
	void cdvdcontrol_process_asdb(QString&);

	bool start();
	bool start_update_info();
	bool next_test();
	ThreadType threadType;
	bool stop;
	bool running;
	bool preserveMediaInfo;

	int  taIdx;
	MediaWatcher *mwatcher;
};

class devlist : public QList<device*> {
public:
	devlist();
	~devlist();
	int idx();
	void setIdx(int iidx);
	void clear();
	device* current();
	int			devidx;
};

#endif

