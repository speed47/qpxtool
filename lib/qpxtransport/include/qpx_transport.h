//
// This is part of dvd+rw-tools by Andy Polyakov <appro@fy.chalmers.se>
//
// Use-it-on-your-own-risk, GPL bless...
//
// For further details see http://fy.chalmers.se/~appro/linux/DVD+RW/
//


//
// modified to use with QPxTool http://qpxtool.sf.net (C) 2005-2009,2012, Gennady "ShultZ" Kozlov
//

#include <string.h>

#ifndef __qpx_transport_h
#define __qpx_transport_h

#if defined(__unix) || defined(__unix__)

extern long getmsecs();
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#ifndef EMEDIUMTYPE
#define EMEDIUMTYPE	EINVAL
#endif
#ifndef	ENOMEDIUM
#define	ENOMEDIUM	ENODEV
#endif

//*
#elif defined(_WIN32)

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define ssize_t		LONG_PTR
#define off64_t		__int64

#if !defined(__MINGW32__)
#include "win32err.h"
#endif

#define poll(a,b,t)	Sleep(t)
#define getmsecs()	GetTickCount()

#include <locale.h>
#define ENV_LOCALE	".OCP"
//*/
#endif

#define CREAM_ON_ERRNO_NAKED(s)				\
    switch ((s)[12])					\
    {	case 0x04:	errno=EAGAIN;	break;		\
	case 0x20:	errno=ENODEV;	break;		\
	case 0x21:	if ((s)[13]==0)	errno=ENOSPC;	\
			else		errno=EINVAL;	\
			break;				\
	case 0x30:	errno=EMEDIUMTYPE;	break;	\
	case 0x3A:	errno=ENOMEDIUM;	break;	\
    }
#define CREAM_ON_ERRNO(s)	do { CREAM_ON_ERRNO_NAKED(s) } while(0)

#define	FATAL_START(er)	(0x80|(er))

#define ERRCODE_FIXED(s)      ((((s)[2]&0x0F)<<16)|((s)[12]<<8)|((s)[13]))
#define ERRCODE_DESCR(s)      ((((s)[1]&0x0F)<<16)|((s)[2]<<8)|((s)[3]))
#define ERRCODE(s)            ((s)[0] == 0x70 || (s)[0] == 0x71 ? \
                                ERRCODE_FIXED(s) : \
                               ((s)[0] == 0x72 || (s)[0] == 0x73 ? \
                                ERRCODE_DESCR(s) : 0))

//#define ERRCODE(s)	((((s)[2]&0x0F)<<16)|((s)[12]<<8)|((s)[13]))

#define	SK(errcode)	(((errcode)>>16)&0xF)
#define	ASC(errcode)	(((errcode)>>8)&0xFF)
#define ASCQ(errcode)	((errcode)&0xFF)

extern void sperror (const char *cmd, int err); //,  Scsi_Command *scsi);

class autofree {
    private:
	unsigned char *ptr;
    public:
	autofree();
	~autofree();
	unsigned char *operator=(unsigned char *str) { return ptr=str; }
	operator unsigned char *()		{ return ptr; }
};

#if defined(__linux) || defined(__GNU__)

//#include <sys/ioctl.h>
#if defined(__linux)
#include <linux/cdrom.h>
#elif defined(__GNU__)
#include <sys/cdrom.h>
#endif
//#include <mntent.h>
//#include <sys/wait.h>
//#include <sys/utsname.h>
#if defined(__linux)
#include <scsi/sg.h>
#endif
#if !defined(SG_FLAG_LUN_INHIBIT)
# if defined(SG_FLAG_UNUSED_LUN_INHIBIT)
#  define SG_FLAG_LUN_INHIBIT SG_FLAG_UNUSED_LUN_INHIBIT
# else
#  define SG_FLAG_LUN_INHIBIT 0
# endif
#endif
#ifndef CHECK_CONDITION
#define CHECK_CONDITION 0x01
#endif

typedef enum {	NONE=CGC_DATA_NONE,	// 3
				READ=CGC_DATA_READ,	// 2
				WRITE=CGC_DATA_WRITE	// 1
	     } Direction;
#ifdef SG_IO

static const int Dir_xlate [4] = {	// should have been defined
					// private in USE_SG_IO scope,
					// but it appears to be too
		0,			// implementation-dependent...
		SG_DXFER_TO_DEV,	// 1,CGC_DATA_WRITE
		SG_DXFER_FROM_DEV,	// 2,CGC_DATA_READ
		SG_DXFER_NONE	};	// 3,CGC_DATA_NONE

class USE_SG_IO {
private:
    int	yes_or_no;
public:
    USE_SG_IO();
    ~USE_SG_IO();
    operator int()			const	{ return yes_or_no; }
    int operator[] (Direction dir)	const	{ return Dir_xlate[dir]; }
};

static const class USE_SG_IO use_sg_io;

#endif

class Scsi_Command {
private:
//    long cmd_time;
    int fd,autoclose;
    char *filename;
    struct cdrom_generic_command cgc;
    union sense_union {
		struct request_sense	s;
		unsigned char		u[18];
    } _sense;
#ifdef SG_IO
    struct sg_io_hdr		sg_io;
#else
    struct { int cmd_len,timeout; }	sg_io;
#endif
public:
    Scsi_Command();
    Scsi_Command(int f);
    Scsi_Command(void*f);
    ~Scsi_Command();
    int associate (const char *file,const struct stat *ref);
    unsigned char &operator[] (size_t i);
	unsigned char &operator()(size_t i);
    unsigned char *sense();
    void timeout(int i);
    size_t residue();
    int transport(Direction dir=NONE,void *buf=NULL,size_t sz=0);
    int umount(int f);
    int is_reload_needed ();
};

#elif defined(__OpenBSD__) || defined(__NetBSD__)

#include <sys/ioctl.h>
#include <sys/scsiio.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/mount.h>

typedef off_t off64_t;
#define stat64   stat
#define fstat64  fstat
#define open64   open
#define pread64	 pread
#define pwrite64 pwrite
#define lseek64  lseek

typedef enum {	NONE=0,
				READ=SCCMD_READ,
				WRITE=SCCMD_WRITE
	     } Direction;

class Scsi_Command {
private:
    int fd,autoclose;
    char *filename;
    scsireq_t req;
public:
    Scsi_Command();
    Scsi_Command(int f);
    Scsi_Command(void*f);
    ~Scsi_Command();
    int associate (const char *file,const struct stat *ref);
    unsigned char &operator[] (size_t i);
    unsigned char &operator()(size_t i);
    unsigned char *sense();
    void timeout(int i);
    size_t residue();
    int transport(Direction dir=NONE,void *buf=NULL,size_t sz=0);
    int umount(int f);
    int is_reload_needed ();
};

#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)

#include <sys/ioctl.h>
#include <stdio.h>
#include <camlib.h>
#include <cam/scsi/scsi_message.h>
#include <cam/scsi/scsi_pass.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <dirent.h>

typedef off_t off64_t;
#define stat64   stat
#define fstat64  fstat
#define open64   open
#define pread64  pread
#define pwrite64 pwrite
#define lseek64  lseek

#define ioctl_fd (((struct cam_device *)ioctl_handle)->fd)

typedef enum {	NONE=CAM_DIR_NONE,
				READ=CAM_DIR_IN,
				WRITE=CAM_DIR_OUT
	     } Direction;

class Scsi_Command {
private:
    int fd,autoclose;
    char *filename;
    struct cam_device  *cam;
    union ccb		ccb;
public:
    Scsi_Command();
    Scsi_Command(int f);
    Scsi_Command(void *f);
    ~Scsi_Command();

    int associate (const char *file,const struct stat *ref);
    unsigned char &operator[] (size_t i);
    unsigned char &operator()(size_t i);
	unsigned char *sense();
	void timeout(int i);
	size_t residue();
	int transport(Direction dir=NONE,void *buf=NULL,size_t sz=0);
	int umount(int f);
#define RELOAD_NEVER_NEEDED	// according to Matthew Dillon
	int is_reload_needed ();
};

//*
#elif defined(_WIN32)

#if defined(__MINGW32__)
#include <ddk/ntddscsi.h>
#define FSCTL_LOCK_VOLUME	CTL_CODE(FILE_DEVICE_FILE_SYSTEM,6,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define FSCTL_UNLOCK_VOLUME	CTL_CODE(FILE_DEVICE_FILE_SYSTEM,7,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define FSCTL_DISMOUNT_VOLUME	CTL_CODE(FILE_DEVICE_FILE_SYSTEM,8,METHOD_BUFFERED,FILE_ANY_ACCESS)
#else
#include <winioctl.h>
#include <ntddscsi.h>
#endif

typedef enum {	NONE=SCSI_IOCTL_DATA_UNSPECIFIED,
				READ=SCSI_IOCTL_DATA_IN,
				WRITE=SCSI_IOCTL_DATA_OUT
	     } Direction;

typedef struct {
    SCSI_PASS_THROUGH_DIRECT	spt;
    unsigned char		sense[18];
} SPKG;

class Scsi_Command {
private:
    HANDLE fd;
    int    autoclose;
    char  *filename;
    SPKG   p;
public:
    Scsi_Command();
    Scsi_Command(void*f);
    ~Scsi_Command();
    int associate (const char *file,const struct stat *ref=NULL);
    unsigned char &operator[] (size_t i);
    unsigned char &operator()(size_t i);
    unsigned char *sense();
    void timeout(int i);
	size_t residue() { return 0; } // bogus
    int transport(Direction dir=NONE,void *buf=NULL,size_t sz=0);
    int umount (int f=-1);

#define RELOAD_NEVER_NEEDED
    int is_reload_needed ();
};
//*/

#elif defined (__APPLE__) && defined (__MACH__)

//
// This code targets Darwin Kernel Version 6.x, a.k.a. Mac OS X v10.2,
// or later, but upon initial release was tested only on PowerPC under
// Darwin Kernel Version 8.7.0, a.k.a. Mac OS X v10.4.7 (Tiger).
//

typedef off_t off64_t;
#define stat64   stat
#define fstat64  fstat
#define open64   open
#define pread64  pread
#define pwrite64 pwrite
#define lseek64  lseek

#include <sys/param.h>
#include <sys/mount.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/scsi/SCSITaskLib.h>

static int iokit_err   (IOReturn ioret,SCSITaskStatus stat,
			const unsigned char *sense);

// NB! ellipsis is GCC-ism, but conveniently Apple ships only gcc:-)
#define MMCIO(h,func,...)	({			\
    MMCDeviceInterface	**di = (MMCDeviceInterface **)h;\
    SCSITaskStatus	stat;				\
    union {						\
	SCSI_Sense_Data	s;				\
	unsigned char	u[18];				\
	}		sense;				\
    IOReturn		ioret;				\
    memset (&sense,0,sizeof(sense));			\
    ioret = (*di)->func(di,__VA_ARGS__,&stat,&sense.s);	\
    iokit_err (ioret,stat,sense.u);			})

typedef enum {	NONE=kSCSIDataTransfer_NoDataTransfer,
		READ=kSCSIDataTransfer_FromTargetToInitiator,
		WRITE=kSCSIDataTransfer_FromInitiatorToTarget
	     } Direction;

class Scsi_Command {
private:
    int				autoclose,_timeout;
    char			*filename;
    io_object_t			scsiob;
    IOCFPlugInInterface		**plugin;
    MMCDeviceInterface		**mmcdif;
    SCSITaskDeviceInterface	**taskif;
    unsigned char		cdb[16];
    union {
		SCSI_Sense_Data		s;
		unsigned char		u[18];
    } _sense;
    size_t			cdblen,resid;
public:
    Scsi_Command();
    Scsi_Command(void *f);
    ~Scsi_Command();

    int associate (const char *file,const struct stat *ref=NULL);

    unsigned char &operator[] (size_t i);
    unsigned char &operator() (size_t i);
    unsigned char *sense();
    void timeout(int i);
    size_t residue();
    int transport(Direction dir=NONE,void *buf=NULL,size_t sz=0);
    int umount(int f=-1);
#define RELOAD_NEVER_NEEDED
    int is_reload_needed(int not_used);
};

#else

#error "Unsupported OS"
#undef ERRCODE
#undef CREAM_ON_ERRNO
#undef CREAM_ON_ERRNO_NAKED

#endif

#endif

