/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2010-2012 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include "resultsio.h"

#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <device.h>
#include <qpx_mmc_defs.h>
#include <colors.h>

void ResultsReader::run() {
	QXmlStreamReader xml;
	QXmlStreamAttributes attr;
	QString name;
	QStringList data;
	bool dataFound = 0,
		 isQpxData = 0,
		 isDevice = 0,
		 isMedia = 0,
		 isTests = 0;
	int  test = 0;
	int	 taZone = -1;

	DI_Transfer d_tr;
	DI_Errc		d_errc;
	DI_JB		d_jb;
	DI_FT		d_ft;
	DI_TA		d_ta;

	res = false;

	if (!io || !io->isOpen() || !io->isReadable()) { return; }
	xml.setDevice(io);
//	xml.version();

	dev->clearMinfo();
	while(!xml.atEnd()) {
		switch (xml.readNext()) {
			case QXmlStreamReader::StartElement:
				name = xml.name().toString();
				if (xml.name() == "qpxdata") {
					dataFound = 1;
					isQpxData = 1;
				}
				if (isQpxData) {
					if (test && (xml.name() == "d")) {
						data = xml.readElementText().split(",");
						switch (test) {
							case TEST_RT:
								if (data.size() == 3) {
									d_tr.lba  = data[0].toLongLong();
									d_tr.spdx = data[1].toFloat();
									d_tr.spdk = data[2].toInt();
									dev->testData.rt << d_tr;
								}
								break;
							case TEST_WT:
								if (data.size() == 3) {
									d_tr.lba  = data[0].toLongLong();
									d_tr.spdx = data[1].toFloat();
									d_tr.spdk = data[2].toInt();
									dev->testData.wt << d_tr;
								}
								break;
							case TEST_ERRC:
								if (data.size() == 10) {
									d_errc.raw.lba    = data[0].toLongLong();
									d_errc.raw.spdx   = data[1].toFloat();
									for (int i=0; i<8; i++)
										d_errc.raw.err[i] = data[2+i].toInt();
#if RECALC_ON_LOAD
									ErrcADD(&dev->testData.errcTOT, d_errc);
									ErrcMAX(&dev->testData.errcMAX, d_errc);
									if (dev->media.type.startsWith("CD")) {
										CDErrcAVG(&dev->testData.errcAVG,  &dev->testData.errcTOT, d_errc.cd.lba/75);
									} else if (dev->media.type.startsWith("DVD")) {
										DVDErrcAVG(&dev->testData.errcAVG, &dev->testData.errcTOT, d_errc.dvd.lba >> 4);
									} else if (dev->media.type.startsWith("BD")) {
										BDErrcAVG(&dev->testData.errcAVG, &dev->testData.errcTOT, d_errc.bd.lba >> 5);
									}
#endif
									dev->testData.errc << d_errc;
								}
								break;
							case TEST_JB:
								if (data.size() == 4) {
									d_jb.lba    = data[0].toLongLong();
									d_jb.spdx   = data[1].toFloat();
									d_jb.jitter = data[2].toFloat();
									d_jb.asymm  = data[3].toFloat();
#if RECALC_ON_LOAD
									if (!dev->testData.jb.size()) {
										dev->testData.jbMM.jmin = d_jb.jitter;
										dev->testData.jbMM.jmax = d_jb.jitter;
										dev->testData.jbMM.bmin = d_jb.asymm;
										dev->testData.jbMM.bmax = d_jb.asymm;
									} else {
										if (dev->testData.jbMM.jmin > d_jb.jitter) dev->testData.jbMM.jmin = d_jb.jitter;
										if (dev->testData.jbMM.jmax < d_jb.jitter) dev->testData.jbMM.jmax = d_jb.jitter;
										if (dev->testData.jbMM.bmin > d_jb.asymm)  dev->testData.jbMM.bmin = d_jb.asymm;
										if (dev->testData.jbMM.bmax < d_jb.asymm)  dev->testData.jbMM.bmin = d_jb.asymm;
									}
#endif
									dev->testData.jb << d_jb;
								}
								break;
							case TEST_FT:
								if (data.size() == 4) {
									d_ft.lba = data[0].toLongLong();
									d_ft.spdx= data[1].toFloat();
									d_ft.fe  = data[2].toInt();
									d_ft.te  = data[3].toInt();
#if RECALC_ON_LOAD
									if (dev->testData.ftMAX.fe < d_ft.fe) dev->testData.ftMAX.fe = d_ft.fe;
									if (dev->testData.ftMAX.te < d_ft.te) dev->testData.ftMAX.te = d_ft.te;
#endif
									dev->testData.ft << d_ft;
								}
								break;
							case TEST_TA:
								if (taZone>=0 && taZone<6 && data.size() == 3) {
									d_ta.idx  = data[0].toInt();
									d_ta.pit  = data[1].toInt();
									d_ta.land = data[2].toInt();

									dev->testData.ta[taZone] << d_ta;
								}
								break;
						}
					} else if (isDevice) {
					// device information
						attr = xml.attributes();
						if (xml.name() == "identify") {
//							dev->type = attr.value("type").toInt();
							dev->path = attr.value("path").toString();
							dev->host = attr.value("host").toString();
							dev->port = attr.value("port").toString().toInt();
							dev->path = attr.value("path").toString();
//							dev->id   = attr.value("id").toString();
							dev->ven  = attr.value("ven").toString();
							dev->dev  = attr.value("dev").toString();
							dev->sn   = attr.value("sn").toString();
							dev->fw   = attr.value("fw").toString();
							dev->tla  = attr.value("tla").toString();
						} else if (xml.name() == "info") {
							dev->buf    = attr.value("buf").toString();
							dev->iface  = attr.value("iface").toString();
							dev->loader = attr.value("loader").toString();
							dev->cap    = attr.value("cap").toString().toULongLong(NULL, 16);
							dev->cap_rd = attr.value("cap_rd").toString().toULongLong(NULL, 16);
							dev->cap_wr = attr.value("cap_wr").toString().toULongLong(NULL, 16);
						} else if (xml.name() == "rpc") {
							dev->rpc_phase = attr.value("phase").toString().toInt();
							dev->rpc_reg   = attr.value("region").toString().toInt();
							dev->rpc_ch    = attr.value("changes").toString().toInt();
							dev->rpc_rst   = attr.value("resets").toString().toInt();
						} else if (xml.name() == "lifetime") {
							dev->life_dn = attr.value("count").toString().toInt(); 
							dev->life_cr = attr.value("cd_rd").toString();
							dev->life_cw = attr.value("cd_wr").toString();
							dev->life_dr = attr.value("dvd_rd").toString();
							dev->life_dw = attr.value("dvd_wr").toString();
						}
					} else if (isMedia) {
					// media information
						attr = xml.attributes();
						if (xml.name() == "identify") {
							dev->media.type  = attr.value("type").toString();
							dev->media.category = attr.value("category").toString();
							dev->media.mid   = attr.value("mid").toString();
							dev->media.label = attr.value("label").toString();
						} else if (xml.name() == "capacity") {
							dev->media.creads = attr.value("read").toString().toInt();
							dev->media.cfrees = attr.value("free").toString().toInt();
							dev->media.ctots  = attr.value("tot").toString().toInt();
							dev->media.creadm = attr.value("read_m").toString().toInt();
							dev->media.cfreem = attr.value("free_m").toString().toInt();
							dev->media.ctotm  = attr.value("tot_m").toString().toInt();
							dev->media.creadmsf = attr.value("read_msf").toString();
							dev->media.cfreemsf = attr.value("free_msf").toString();
							dev->media.ctotmsf  = attr.value("tot_msf").toString();
						} else if (xml.name() == "speeds") {
							dev->media.rspeeds  = attr.value("rspeeds" ).toString().split(" ", QString::SkipEmptyParts);
							dev->media.wspeedsd = attr.value("wspeedsd").toString().split(" ", QString::SkipEmptyParts);
							dev->media.wspeedsm = attr.value("wspeedsm").toString().split(" ", QString::SkipEmptyParts);
						} else if (xml.name() == "misc") {
							dev->media.writer  = attr.value("writer").toString();
							dev->media.prot    = attr.value("protection").toString();
							dev->media.regions = attr.value("regions").toString();
							dev->media.grec    = attr.value("gigarec").toString().toDouble();
							dev->media.spd1X   = attr.value("spd1X").toString().toInt();
							dev->media.layers  = attr.value("layers").toString();
							dev->media.erasable= attr.value("erasable").toString();
							dev->media.ilayers = dev->media.layers.toInt();
							dev->media.dstate  = attr.value("dstate").toString();
							dev->media.sstate  = attr.value("sstate").toString();
						}
					} else if (isTests) {
						attr = xml.attributes();
						if (xml.name() == "rt") {
							test = TEST_RT;
							dev->tspeeds.rt = attr.value("speed").toString().toInt();
							dev->testData.rt_time = attr.value("time").toString().toDouble();
						} else if (xml.name() == "wt") {
							test = TEST_WT;
							dev->tspeeds.wt = attr.value("speed").toString().toInt();
							dev->testData.wt_time = attr.value("time").toString().toDouble();
						} else if (xml.name() == "errc") {
							test = TEST_ERRC;
							dev->tspeeds.errc = attr.value("speed").toString().toInt();
							dev->testData.errc_time = attr.value("time").toString().toDouble();
							dev->media.tdata_errc = attr.value("data").toString().toInt();
						} else if (xml.name() == "jb") {
							test = TEST_JB;
							dev->tspeeds.jb = attr.value("speed").toString().toInt();
							dev->testData.jb_time = attr.value("time").toString().toDouble();
						} else if (xml.name() == "ft") {
							test = TEST_FT;
							dev->tspeeds.ft = attr.value("speed").toString().toInt();
							dev->testData.ft_time = attr.value("time").toString().toDouble();
						} else if (xml.name() == "ta") {
							test = TEST_TA;
						//	dev->tspeeds.ta = attr.value("speed").toString().toInt();
							dev->testData.ta_time = attr.value("time").toString().toDouble();
						} else if ((test == TEST_TA) && (xml.name() == "zone")) {
							taZone = attr.value("idx").toString().toInt();
						}
					} else if (xml.name() == "device") {
						isDevice = 1;
					} else if (xml.name() == "media") {
						isMedia = 1;
					} else if (xml.name() == "tests") {
						isTests = 1;
					}
				}
				break;
			case QXmlStreamReader::EndElement:
				if (isQpxData && xml.name() == "qpxdata") {
					isQpxData = 0;
				} else if (isDevice && xml.name() == "device") {
					isDevice = 0;
				} else if (isMedia && xml.name() == "media") {
					isMedia = 0;
				} else if (isTests && xml.name() == "tests") {
					isTests = 0;
				} else if (isTests) {
					if (test == TEST_RT && xml.name() == "rt") {
						test = 0;
					} else if (test == TEST_WT && xml.name() == "wt") {
						test = 0;
					} else if (test == TEST_ERRC && xml.name() == "errc") {
						test = 0;
					} else if (test == TEST_JB && xml.name() == "jb") {
						test = 0;
					} else if (test == TEST_FT && xml.name() == "ft") {
						test = 0;
					} else if (test == TEST_TA && xml.name() == "ta") {
						test = 0;
					} else if (test == TEST_TA && xml.name() == "zone") {
						taZone = -1;
					}
				}
			case QXmlStreamReader::Invalid:
				qDebug() << COL_RED << "ResultsReader:  Invalid token @" << xml.characterOffset() << xml.name() << COL_NORM;
				break;
			case QXmlStreamReader::NoToken:
			case QXmlStreamReader::StartDocument:
			case QXmlStreamReader::EndDocument:
			case QXmlStreamReader::Characters:
			case QXmlStreamReader::Comment:
			case QXmlStreamReader::DTD:
			case QXmlStreamReader::EntityReference:
			case QXmlStreamReader::ProcessingInstruction:
				break;
	//		default:
				break;
		}
	}
	res = dataFound;
}


void ResultsWriter::run() {
	QXmlStreamWriter xml;
	res = false;

	if (!io || !io->isOpen() || !io->isWritable()) { return; }

	xml.setDevice(io);
	xml.setAutoFormatting(true);

	xml.writeStartDocument("1.0");
	xml.writeStartElement("qpxdata");
// writing metadata
// device info...
	xml.writeStartElement("device");
	xml.writeStartElement("identify");
	switch (dev->type) {
		case device::DevtypeLocal:
			xml.writeAttribute("type","local");
			break;
		case device::DevtypeTCP:
			xml.writeAttribute("type","tcp");
			xml.writeAttribute("host", dev->host);
			xml.writeAttribute("port", QString::number(dev->port));
			break;
		default:
			break;
	}
	xml.writeAttribute("path", dev->path);
//	xml.writeAttribute("id",   dev->id);
	xml.writeAttribute("ven",  dev->ven);
	xml.writeAttribute("dev",  dev->dev);
	xml.writeAttribute("sn",   dev->sn);
	xml.writeAttribute("fw",   dev->fw);
	xml.writeAttribute("tla",  dev->tla);
	xml.writeEndElement();

	xml.writeStartElement("info");
	xml.writeAttribute("buf",  dev->buf);
	xml.writeAttribute("iface",dev->iface);
	xml.writeAttribute("loader",dev->loader);
	xml.writeAttribute("cap",   QString("%1").arg(dev->cap,0,16));
	xml.writeAttribute("cap_rd",  QString("%1").arg(dev->cap_rd,0,16));
	xml.writeAttribute("cap_wr", QString("%1").arg(dev->cap_wr,0,16));
	xml.writeEndElement();

	if (dev->cap & CAP_DVD_CSS)
	{
		xml.writeStartElement("rpc");
		xml.writeAttribute("phase",  QString::number(dev->rpc_phase));
		xml.writeAttribute("region", QString::number(dev->rpc_reg));
		xml.writeAttribute("changes", QString::number(dev->rpc_ch));
		xml.writeAttribute("resets", QString::number(dev->rpc_rst));
		xml.writeEndElement();
	}

//	if (life_dn) {
		xml.writeStartElement("lifetime");
		xml.writeAttribute("count",  QString::number(dev->life_dn));
		xml.writeAttribute("cd_rd",  dev->life_cr);
		xml.writeAttribute("cd_wr",  dev->life_cw);
		xml.writeAttribute("dvd_rd", dev->life_dr);
		xml.writeAttribute("dvd_wr", dev->life_dw);
		xml.writeEndElement();
//	}

	xml.writeEndElement(); // device
	xml.writeStartElement("media");
// media info
	xml.writeStartElement("identify");
	xml.writeAttribute("type",      dev->media.type);
	xml.writeAttribute("category",  dev->media.category);
	xml.writeAttribute("mid",       dev->media.mid);
	xml.writeAttribute("label",     dev->media.label);
	xml.writeEndElement();

	xml.writeStartElement("capacity");
	xml.writeAttribute("read", QString::number(dev->media.creads));
	xml.writeAttribute("free", QString::number(dev->media.cfrees));
	xml.writeAttribute("tot",  QString::number(dev->media.ctots));

	xml.writeAttribute("read_m", QString::number(dev->media.creadm));
	xml.writeAttribute("free_m", QString::number(dev->media.cfreem));
	xml.writeAttribute("tot_m",  QString::number(dev->media.ctotm));

	xml.writeAttribute("read_msf", dev->media.creadmsf);
	xml.writeAttribute("free_msf", dev->media.cfreemsf);
	xml.writeAttribute("tot_msf",  dev->media.ctotmsf);
	xml.writeEndElement(); // capacity

	xml.writeStartElement("speeds");
	xml.writeAttribute("rspeeds",   dev->media.rspeeds.join(" "));
	xml.writeAttribute("wspeedsd",  dev->media.wspeedsd.join(" "));
	xml.writeAttribute("wspeedsm",  dev->media.wspeedsm.join(" "));
	xml.writeEndElement(); // misc

	xml.writeStartElement("misc");
	xml.writeAttribute("writer",    dev->media.writer);
	xml.writeAttribute("protection",dev->media.prot);
	xml.writeAttribute("regions",   dev->media.regions);
	xml.writeAttribute("gigarec",   QString("%1").arg(dev->media.grec,0,'f',1));

	xml.writeAttribute("spd1X",     QString::number(dev->media.spd1X));
	xml.writeAttribute("layers",    QString::number(dev->media.ilayers));
	xml.writeAttribute("erasable",  dev->media.erasable);

	xml.writeAttribute("dstate", dev->media.dstate);
	xml.writeAttribute("sstate", dev->media.sstate);
	xml.writeEndElement(); // misc
	xml.writeEndElement(); // media
// test speeds

	xml.writeStartElement("tests");

	if (dev->testData.rt.size()) {
		int s = dev->testData.rt.size();
		xml.writeStartElement("rt");
		xml.writeAttribute("speed", QString::number(dev->tspeeds.rt));
		xml.writeAttribute("time",  QString("%1").arg(dev->testData.rt_time,0,'f',2));

		for (int i=0; i<s; i++) {
			xml.writeTextElement("d", QString("%1,%2,%3")
				.arg(dev->testData.rt[i].lba)
				.arg(dev->testData.rt[i].spdx)
				.arg(dev->testData.rt[i].spdk));
		}
		xml.writeEndElement();
	}

	if (dev->testData.wt.size()) {
		int s = dev->testData.rt.size();
		xml.writeStartElement("wt");
		xml.writeAttribute("speed", QString::number(dev->tspeeds.wt));
		xml.writeAttribute("time",  QString("%1").arg(dev->testData.wt_time,0,'f',2));

		for (int i=0; i<s; i++) {
			xml.writeTextElement("d", QString("%1,%2,%3")
				.arg(dev->testData.wt[i].lba)
				.arg(dev->testData.wt[i].spdx)
				.arg(dev->testData.wt[i].spdk));
		}
		xml.writeEndElement();
	}

	if (dev->testData.errc.size()) {
		int s = dev->testData.errc.size();
		xml.writeStartElement("errc");
		xml.writeAttribute("speed", QString::number(dev->tspeeds.errc));
		xml.writeAttribute("time",  QString("%1").arg(dev->testData.errc_time,0,'f',2));
		xml.writeAttribute("data", QString::number(dev->media.tdata_errc));
		
		for (int i=0; i<s; i++) {
			xml.writeTextElement("d", QString("%1,%2,").arg(dev->testData.errc[i].raw.lba).arg(dev->testData.errc[i].raw.spdx,0,'f',2) +
				QString("%1,%2,%3,%4,%5,%6,%7,%8")
					.arg(dev->testData.errc[i].raw.err[0])
					.arg(dev->testData.errc[i].raw.err[1])
					.arg(dev->testData.errc[i].raw.err[2])
					.arg(dev->testData.errc[i].raw.err[3])
					.arg(dev->testData.errc[i].raw.err[4])
					.arg(dev->testData.errc[i].raw.err[5])
					.arg(dev->testData.errc[i].raw.err[6])
					.arg(dev->testData.errc[i].raw.err[7])
					);
		}
		xml.writeEndElement();
	}

	if (dev->testData.jb.size()) {
		int s = dev->testData.jb.size();
		xml.writeStartElement("jb");
		xml.writeAttribute("speed", QString::number(dev->tspeeds.jb));
		xml.writeAttribute("time",  QString("%1").arg(dev->testData.jb_time,0,'f',2));

		for (int i=0; i<s; i++) {
			xml.writeTextElement("d", QString("%1,%2,%3,%4")
					.arg(dev->testData.jb[i].lba)
					.arg(dev->testData.jb[i].spdx)
					.arg(dev->testData.jb[i].jitter)
					.arg(dev->testData.jb[i].asymm));
		}
		xml.writeEndElement();
	}

	if (dev->testData.ft.size()) {
		int s = dev->testData.ft.size();
		xml.writeStartElement("ft");
		xml.writeAttribute("speed", QString::number(dev->tspeeds.ft));
		xml.writeAttribute("time",  QString("%1").arg(dev->testData.ft_time,0,'f',2));

		for (int i=0; i<s; i++) {
			xml.writeTextElement("d", QString("%1,%2,%3,%4")
					.arg(dev->testData.ft[i].lba)
					.arg(dev->testData.ft[i].spdx)
					.arg(dev->testData.ft[i].fe)
					.arg(dev->testData.ft[i].te));
		}
		xml.writeEndElement();
	}

	if (dev->testData.ta[0].size()) {
		xml.writeStartElement("ta");
		xml.writeAttribute("time",  QString("%1").arg(dev->testData.ta_time,0,'f',2));

		for (int i=0; i<6; i++) {
			int s = dev->testData.ta[i].size();
			xml.writeStartElement("zone");
			xml.writeAttribute("idx", QString::number(i));

			for(int j=0;j<s; j++) {
				xml.writeTextElement("d", QString("%1,%2,%3")
					.arg(dev->testData.ta[i][j].idx)
					.arg(dev->testData.ta[i][j].pit)
					.arg(dev->testData.ta[i][j].land)
				);
			}
			xml.writeEndElement();
		}
		xml.writeEndElement();
	}

	xml.writeEndElement(); // tests
	xml.writeEndElement(); // qpxdata
	xml.writeEndDocument();

	res = true;
}

