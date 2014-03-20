/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2007-2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#if defined (__unix) || defined (__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <dlfcn.h>
#elif defined (_WIN32)
#include <windows.h>

#define dlopen(n,f) LoadLibraryA(n)
#define dlsym(l,n) 	GetProcAddress(l,n)
#define dlclose(l)	FreeLibrary(l)

char serr[255];

char* dlerror()
{
	int err = GetLastError();
	if (!err) return 0;
	sprintf(serr, "Library load error %d", err);
	return serr;
};

#endif

#include <sys/types.h>
#include <dirent.h>


//#include <qpx_mmc.h>
#include "qpx_scan.h"

#include <qpx_writer.h>
#include <plextor_features.h>

#define FALLBACK_PLUGIN_NAME "C2P"

qscanner::qscanner(drive_info* idev) {
	dev=idev;
	writer=NULL;
	plugin=NULL;
	pluginlib=NULL;
	plugin_create=NULL;
	plugin_destroy=NULL;
	attached=0;
	listed = 0;
	speed = -1;
	lba_sta=0;
	lba_end=-1;
	tchar=-1;
}

qscanner::~qscanner() {
	if (attached) plugin_detach();
}

void qscanner::setTestSpeed(int ispeed) { speed = ispeed; }

bool qscanner::setTestWrite(bool simul) {
	if (dev->media.type & DISC_DVDplus) {
		if (isPlextor(dev)) {
			if (isPlextorLockPresent(dev) || !plextor_px755_do_auth(dev) ) {
				printf("Turning PLEXTOR DVD+R(W) TestWrite %s\n", simul ? "ON" : "OFF");
				WT_simul = 0;
				dev->plextor.testwrite_dvdplus = simul;
				return plextor_set_testwrite_dvdplus(dev);
			} else {
				printf("Found locked PLEXTOR drive. Can't handle DVD+R(W) TestWrite!\n");
				return 1;
			}
		} else {
			if (simul) {
				printf("TestWrite on DVD+R(W) supported on PLEXTOR drives only!\n");
				WT_simul = 0;
				return 1;
			} else {
				WT_simul = 0;
				return 0;
			}
		}
	} else {
		if ((dev->media.type & DISC_CD) && (dev->capabilities & CAP_TEST_WRITE_CD)) {
			printf("Turning TestWrite (CD) %s\n", simul ? "ON" : "OFF");
			WT_simul = simul;
			return 0;
		} else if ((dev->media.type & DISC_DVDminus) && (dev->capabilities & CAP_TEST_WRITE_DVD)) {
			printf("Turning TestWrite (DVD) %s\n", simul ? "ON" : "OFF");
			WT_simul = simul;
			return 0;
		} else {
			WT_simul = 0;
			if (simul) {
				if ((dev->media.type & DISC_DVDRAM)) {
					printf("DVD-RAM media doesn't support TestWrite!\n");
				} else {				
					printf("Drive doesn't support TestWrite on this media!\n");
				}
				return 1;
			} else {
				return 0;
			}
		}
	}
}

void qscanner::stop() {
	stop_req=1;
	if (writer) writer->stop();
};

void qscanner::stat() {
	stat_req=1;
};


int qscanner::run(char *test) {
	int r=-1;
	stop_req=0;
	stat_req=0;
	if (!dev->media.type) {
		printf("No media detected!\n");
		return 1;
	}
	if (!(dev->media.type & (DISC_CD | DISC_DVD | DISC_BD))) {
		printf("Unsupported media!\n");
		return 2;
	}
//	set_speed(dev,speed);
	if (!strcmp(test, "rt")) {
		if (lba_end<0 || lba_end>dev->media.capacity) lba_end = dev->media.capacity-1;
		r=run_rd_transfer();
	} else if (!strcmp(test, "wt")) {
		if (lba_end<0 || lba_end>dev->media.capacity_total) lba_end = dev->media.capacity_total-1;
		r=run_wr_transfer();
	} else if (!strcmp(test, "errc")) {
		if (lba_end<0 || lba_end>dev->media.capacity) lba_end = dev->media.capacity-1;
		if (dev->media.type & DISC_CD) {
			r=run_cd_errc();
		} else if (dev->media.type & DISC_DVD) {
			r=run_dvd_errc();
		} else if (dev->media.type & DISC_BD) {
			r=run_bd_errc();
		}
	} else if (!strcmp(test, "jb")) {
		if (lba_end<0 || lba_end>dev->media.capacity) lba_end = dev->media.capacity-1;
		if (dev->media.type & DISC_CD) {
			r=run_cd_jb();
		} else if (dev->media.type & DISC_DVD) {
			r=run_dvd_jb();
		}
	} else if (!strcmp(test, "ft")) {
		if (lba_end<0 || lba_end>dev->media.capacity_total) lba_end = dev->media.capacity_total-1;
		r=run_fete();
	} else if (!strcmp(test, "ta")) {
		if (dev->media.type & DISC_CD) {
			r=run_cd_ta();
		} else if (dev->media.type & DISC_DVD) {
			r=run_dvd_ta();
		}
	}
	return r;
}

int qscanner::check_test(unsigned int test) {
	if (!attached) return -1;
	return plugin->check_test(test);
}

int qscanner::errc_data() {
	if (!attached) return -1;
	return plugin->errc_data();
}

int* qscanner::get_test_speeds(unsigned int test) {
	if (!attached) return NULL;
	return plugin->get_test_speeds(test);
}
	
int qscanner::plugins_probe(bool test, bool probe_enable) {
	char *pname;
	char *ppath;
	DIR  *dir;
	struct dirent *dentry;
	int i;
	int r=1;
	for (i=0; strlen(ppaths[i]) && !attached; i++) {
		ppath = (char*) ppaths[i];
		if (!dev->silent)
			printf("Looking for plugins in %s...\n", ppath);
		dir = opendir(ppath);
		if (dir) {
			dentry = readdir(dir);
			while(dentry && !attached) {
				if (!strncmp(dentry->d_name,"libqscan_",9)) {
					if (!dev->silent) printf("FOUND: %s\n", dentry->d_name);
					pname = (char*) malloc (strlen(dentry->d_name) + strlen(ppath) +2 );
#ifdef _WIN32
					sprintf(pname, "%s\\%s", ppath, dentry->d_name);
#else
					sprintf(pname, "%s/%s", ppath, dentry->d_name);
#endif
					plugin_attach(pname, probe_enable, 0, !test);
					if (attached) {
						r=0;
						if (test) {
							plugin_detach();
						} else {
							if (!strcmp(plugin->name(), FALLBACK_PLUGIN_NAME)) {
							//	printf("Found fallback plugin, return...\n");
								plugin_detach();
								r=1;
							}
						}
					}
					free(pname);
				}
				dentry = readdir(dir);
			}
			closedir(dir);
		}
	}
	return r;
}

int qscanner::plugin_attach_fallback() { return plugin_attach(FALLBACK_PLUGIN_NAME); } 

int qscanner::plugin_attach(char* name) {
	char *pname;
	char *ppath;
	DIR  *dir;
	struct dirent *dentry;
	int i;
	int r=1;
	if (attached || !name) return 2;

	for (i=0; strlen(ppaths[i]) && !attached; i++) {
		ppath = (char*) ppaths[i];
		if (!dev->silent) printf("Looking for plugins in %s...\n", ppath);
		dir = opendir(ppath);
		if (dir) {
			dentry = readdir(dir);
			while(dentry && !attached) {
				if (!strncmp(dentry->d_name,"libqscan_",9)) {
					if (!dev->silent) printf("FOUND: %s\n", dentry->d_name);
					pname = (char*) malloc (strlen(dentry->d_name) + strlen(ppath) +2 );
#ifdef _WIN32
					sprintf(pname, "%s\\%s", ppath, dentry->d_name);
#else
					sprintf(pname, "%s/%s", ppath, dentry->d_name);
#endif
					plugin_attach(pname, 0, 1, 1);

					if (attached) {
						if ( strcmp(plugin->name(), name)) plugin_detach();
						else r=0;
					}
					free(pname);
				}
				dentry = readdir(dir);
			}
			closedir(dir);
		}
	}
	if (!attached)
		printf("Can't find plugin '%s'\n", name);
	else
		if (!strcmp(plugin->name(), FALLBACK_PLUGIN_NAME)) {
			printf("Fallback plugin loaded: '%s'\n", name);
		} else {
			printf("Forced plugin loaded: '%s'\n", name);
		}
	return r;
}

int qscanner::plugin_attach(char* pname, bool probe_enable, bool no_detach, bool silent) {
	bool blacklisted=0;
	drivedesc* devlist; 
	if (attached) return 2;
	listed=0;

//	pluginlib = dlopen( pname, RTLD_NOW | RTLD_GLOBAL);
	pluginlib = dlopen( pname, RTLD_LAZY | RTLD_GLOBAL);
	if (!pluginlib) {
		printf("0 dlopen err: %s\n",dlerror());
//		printf("can't open library!\n");
		goto plugin_attach_liberr;
	}
//	if (!dev->silent) printf("pluginlib = %p\n", pluginlib);
#ifndef _WIN32
	if (dlerror()) {
		printf("1 dlopen err: %s\n",dlerror());
		goto plugin_attach_err;
	}
#endif
	if (!dev->silent) printf("plugin lib opened: %s\n",pname);
	plugin_create = (scan_plugin* (*) (drive_info*)) dlsym(pluginlib, "plugin_create");
#ifndef _WIN32
	if (dlerror()) {
#else
	if (!plugin_create) {
#endif
		printf("error searching symbol \"plugin_create\" : %s\n",dlerror());
		goto plugin_attach_err;
	}
//	printf("symbol \"plugin_create\" found!\n");
	//*(void **) (&plugin_destroy) = dlsym(pluginlib, "plugin_destroy");
	plugin_destroy = (void (*) (scan_plugin*)) dlsym(pluginlib, "plugin_destroy");
#ifndef _WIN32
	if (dlerror()) {
#else
	if (!plugin_destroy) {
#endif
		printf("error searching symbol \"plugin_destroy\" : %s\n", dlerror());
		goto plugin_attach_err;
	}
//	printf("symbol \"plugin_destroy\" found!\n");
/*
	slist = (drivedesc*) dlsym(pluginlib, "devlist");
	e = dlerror();
	if (e){
		printf("%s\nerror searching symbol \"devlist\"\n",e);
		goto plugin_attach_err;
	}
*/
//	printf("creating plugin\n");
	plugin = plugin_create(dev);
//	printf("plugin info\n");
	if (!silent) printf("Found plugin: %s (%s)\n",plugin->name(),plugin->desc());

	if (plugin->blklist) {
		devlist = plugin->blklist;
		if (!dev->silent) {
			printf("Devices in blacklist:\n");
			for(int d=0; devlist[d].ven_ID>0; d++) {
				printf("  %s %s*\n",devlist[d].ven,devlist[d].dev);
			}
		}
	
		for(int d=0; !blacklisted && devlist[d].ven_ID>0; d++) {
			if (!strncmp(dev->ven, devlist[d].ven, strlen(devlist[d].ven)) && !strncmp(dev->dev,devlist[d].dev, strlen(devlist[d].dev)))
				blacklisted=1;
		}

		devlist = NULL;
		if (blacklisted) {
			printf("Plugin %s: device '%s' '%s' blacklisted! Detaching plugin...\n", plugin->name(), dev->ven, dev->dev);
			attached=1;
			plugin_detach();
			return 1;
		}
	}

	if (!probe_enable && plugin->devlist) {
		devlist = plugin->devlist;
		dev->chk_features = 0;

		if (!dev->silent) {
			printf("Devices supported by this plugin:\n");
			for(int d=0; devlist[d].ven_ID>0 && devlist[d].dev_ID>0;d++) {
				printf("  %s %s\n",devlist[d].ven,devlist[d].dev);
			}
		}

		for(int d=0; !listed && devlist[d].ven_ID>0 && devlist[d].dev_ID>0;d++) {
			if (!strncmp(dev->ven, devlist[d].ven, strlen(devlist[d].ven)) && !strncmp(dev->dev,devlist[d].dev, strlen(devlist[d].dev))) {
				dev->ven_ID = devlist[d].ven_ID;
				dev->dev_ID = devlist[d].dev_ID;
				dev->chk_features = devlist[d].tests;
				listed = 1;
				if (!silent) printf("device listed as: %s %s\n",devlist[d].ven,devlist[d].dev);
			}
		}
	}

	if (!no_detach && !listed && (!probe_enable || plugin->probe_drive() == DEV_FAIL)) {
		if (!dev->silent) {
			if (probe_enable)
				printf("Device probe failed! detaching plugin\n");
			else
				printf("Device not listed! detaching plugin\n");
		}
		attached=1;
		plugin_detach();
		return 1;
	}
	attached=1;
	if (!dev->silent) printf("plugin attached: %s\n", pname);
	return 0;

plugin_attach_err:
	dlclose(pluginlib);

plugin_attach_liberr:
	attached=0;
	printf("error attaching scan plugin %s\n", pname);
	plugin=NULL;
	plugin_create=NULL;
	plugin_destroy=NULL;
	listed=0;
	return -1;
}

void qscanner::plugin_detach() {
	if (!dev->silent) printf("detaching plugin...\n");
	if (!attached) return;
//	if (plugin_destroy!=NULL && plugin!=NULL)
	(*plugin_destroy) (plugin);
	attached=0;
	dlclose(pluginlib);
	plugin=NULL;
	pluginlib=NULL;
	plugin_create=NULL;
	plugin_destroy=NULL;
}

//int qscanner::plugin_info() {}

const char* qscanner::plugin_name() {
	if (!attached) return NULL;
	return plugin->name();
}

const char* qscanner::plugin_desc() {
	if (!attached) return NULL;
	return plugin->desc();
}

