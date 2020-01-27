//
// This is part of dvd+rw-tools by Andy Polyakov <appro@fy.chalmers.se>
//
// Use-it-on-your-own-risk, GPL bless...
//
// For further details see http://fy.chalmers.se/~appro/linux/DVD+RW/
//


//
// modified to use with QPxTool http://qpxtool.sf.net (C) 2005-2009, Gennady "ShultZ" Kozlov
//

#include <sense.h>
#include "colors.h"
#include "qpx_transport.h"

const unsigned char scsi_command_size[8] =
{
	6, 10, 10, 12,
	16, 12, 10, 10
};

#define COMMAND_SIZE(opcode) scsi_command_size[((opcode) >> 5) & 7]


#if defined(__unix) || defined(__unix__)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#include <sys/types.h>
//#include <sys/stat.h>
#ifdef open64
#undef open64
#endif
#include <fcntl.h>
//#include <poll.h>
#include <sys/time.h>


long getmsecs()
{
	struct timeval tv;
	gettimeofday (&tv,NULL);
	return tv.tv_sec*1000+tv.tv_usec/1000;
}

long getusecs()
{
	struct timeval tv;
    gettimeofday (&tv,NULL);
	return tv.tv_sec*1000000+tv.tv_usec;
}

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
#include <stdio.h>

#define EINVAL		ERROR_BAD_ARGUMENTS
#define ENOMEM		ERROR_OUTOFMEMORY
#define EMEDIUMTYPE	ERROR_MEDIA_INCOMPATIBLE
#define ENOMEDIUM	ERROR_MEDIA_OFFLINE
#define ENODEV		ERROR_BAD_COMMAND
#define EAGAIN		ERROR_NOT_READY
#define ENOSPC		ERROR_DISK_FULL
#define EIO			ERROR_NOT_SUPPORTED
#define ENXIO		ERROR_GEN_FAILURE

static class _win32_errno {
    public:
	operator int()		{ return GetLastError(); }
	int operator=(int e)	{ SetLastError(e); return e; }
} _sys_errno;

#ifdef errno
#undef errno
#endif

#define errno _sys_errno

inline void perror (const char *str)
{ LPVOID lpMsgBuf;

    FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		0, // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);
    if (str)
		fprintf (stderr, COL_RED "%s: %s" COL_NORM,str,lpMsgBuf);
    else
		fprintf (stderr, COL_RED "%s" COL_NORM,lpMsgBuf);

    LocalFree(lpMsgBuf);
}

#define exit(e)		ExitProcess(e)

#endif // _WIN32

void sperror (const char *cmd,int err) //,  Scsi_Command *scsi)
{
	int saved_errno=errno;
	char sense_str[255];
	sense2str(err, sense_str);

	if (err==-1) {
		fprintf (stderr, COL_RED "\n:-( unable to %s : [%d] " COL_NORM, cmd, saved_errno);
		errno=saved_errno, perror (NULL);
	} else
		fprintf (stderr, COL_RED "\n:-[ %s failed with SK=%Xh/ASC=%02Xh/ACQ=%02Xh]: %s\n" COL_NORM,
			cmd, SK(err), ASC(err), ASCQ(err),sense_str);
}

autofree::autofree()
	{ ptr=NULL; }
autofree::~autofree()
	{ if (ptr) free(ptr); }

#if defined(__linux) || defined(__GNU__)

#include <limits.h>
#if defined(__linux)
#include <linux/cdrom.h>
#elif defined(__GNU__)
#include <sys/cdrom.h>
#endif
#include <sys/ioctl.h>
#include <mntent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>
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

#ifdef SG_IO

USE_SG_IO::USE_SG_IO()
{
	struct utsname buf;
	uname (&buf);
	// was CDROM_SEND_PACKET declared dead in 2.5?
	yes_or_no=(strcmp(buf.release,"2.5.43")>=0);
}

USE_SG_IO::~USE_SG_IO(){}

#endif
Scsi_Command::Scsi_Command()		{ fd=-1, autoclose=1; filename=NULL; }
Scsi_Command::Scsi_Command(int f)	{ fd=f,  autoclose=0; filename=NULL; }
Scsi_Command::Scsi_Command(void*f)	{ fd=(long)f, autoclose=0; filename=NULL; }
Scsi_Command::~Scsi_Command()
{
	if (fd>=0 && autoclose) close(fd),fd=-1;
	if (filename) free(filename),filename=NULL;
}

int Scsi_Command::associate (const char *file,const struct stat *ref=NULL)
{
	struct stat sb;
	/*
	 * O_RDWR is expected to provide for none set-root-uid
	 * execution under Linux kernel 2.6[.8]. Under 2.4 it
	 * falls down to O_RDONLY...
	 */
	if ((fd=open (file,O_RDWR|O_NONBLOCK)) < 0 &&
	  (fd=open (file,O_RDONLY|O_NONBLOCK)) < 0) return 0;
	if (fstat(fd,&sb) < 0) return 0;
	if (!S_ISBLK(sb.st_mode)) { errno=ENOTBLK;return 0; }
	if (ref && (!S_ISBLK(ref->st_mode) || ref->st_rdev!=sb.st_rdev))
		{ errno=ENXIO; return 0; }
	filename=strdup(file);
	return 1;
}

int Scsi_Command::transport(Direction dir,void *buf,size_t sz)
{
	int ret = 0;
#ifdef SG_IO
#define KERNEL_BROKEN 0
	if (use_sg_io)
	{
		sg_io.dxferp		= buf;
		sg_io.dxfer_len		= sz;
		sg_io.dxfer_direction	= use_sg_io[dir];
		/* cmd length fix */
//		printf("opcode: %02X, cmd.len: %d\n",sg_io.cmdp[0],sg_io.cmd_len);
		sg_io.cmd_len = (sg_io.cmd_len < COMMAND_SIZE(sg_io.cmdp[0])) ? COMMAND_SIZE(sg_io.cmdp[0]):sg_io.cmd_len;
//		printf("new cmd.len: %d\n",sg_io.cmd_len);

		if (ioctl (fd,SG_IO,&sg_io)) return -1;
#if !KERNEL_BROKEN
		if ((sg_io.info&SG_INFO_OK_MASK) != SG_INFO_OK)
#else
		if (sg_io.status)
#endif
		{
			errno=EIO; ret=-1;
#if !KERNEL_BROKEN
			if (sg_io.masked_status&CHECK_CONDITION)
#endif
			{
				ret = ERRCODE(sg_io.sbp);
				if (ret==0) ret=-1;
				else CREAM_ON_ERRNO(sg_io.sbp);
			}
		}
		return ret;
	}
	else
#undef KERNEL_BROKEN
#endif
	{
		cgc.buffer		= (unsigned char *)buf;
		cgc.buflen		= sz;
		cgc.data_direction	= dir;
#if !defined(__GNU__)
		if (ioctl (fd,CDROM_SEND_PACKET,&cgc))
		{
			ret = ERRCODE(_sense.u);
			if (ret==0) ret=-1;
		}
#endif
	}
	return ret;
}

unsigned char &Scsi_Command::operator[] (size_t i)
{
	if (i==0)
	{
		memset(&cgc,0,sizeof(cgc)), memset(&_sense,0,sizeof(_sense));
		cgc.quiet = 1;
		cgc.sense = &_sense.s;
#ifdef SG_IO
		if (use_sg_io)
		{
			memset(&sg_io,0,sizeof(sg_io));
			sg_io.interface_id= 'S';
			sg_io.mx_sb_len	= sizeof(_sense);
			sg_io.cmdp	= cgc.cmd;
			sg_io.sbp	= _sense.u;
			sg_io.flags	= SG_FLAG_LUN_INHIBIT|SG_FLAG_DIRECT_IO;
		}
#endif
	}
	sg_io.cmd_len = i+1;
	return cgc.cmd[i];
}

unsigned char &Scsi_Command::operator()(size_t i)	{ return _sense.u[i]; }
unsigned char *Scsi_Command::sense()	{ return _sense.u;    }

void Scsi_Command::timeout(int i) { cgc.timeout=sg_io.timeout=i*1000; }
#ifdef SG_IO
size_t Scsi_Command::residue() { return use_sg_io?sg_io.resid:0; }
#else
size_t Scsi_Command::residue() { return 0; }
#endif

int Scsi_Command::umount(int f)
{
	struct stat    fsb,msb;
	struct mntent *mb;
	FILE          *fp;
	pid_t          pid,rpid;
	int            ret=0,rval;

	if (f==-1) f=fd;
	if (fstat (f,&fsb) < 0)				return -1;
	if ((fp=setmntent ("/proc/mounts","r"))==NULL)	return -1;

	while ((mb=getmntent (fp))!=NULL)
	{
		if (stat (mb->mnt_fsname,&msb) < 0) continue; // corrupted line?
		if (msb.st_rdev == fsb.st_rdev)
		{
			ret = -1;
			if ((pid = fork()) == (pid_t)-1) break;
			if (pid == 0) execl ("/bin/umount","umount",mb->mnt_dir,NULL);
			while (1)
			{
				rpid = waitpid (pid,&rval,0);
				if (rpid == (pid_t)-1) 
				{
					if (errno==EINTR) continue;
					else break;
				}
				else if (rpid != pid)
				{
					errno = ECHILD;
					break;
				}
				if (WIFEXITED(rval))
				{
					if (WEXITSTATUS(rval) == 0) ret=0;
					else errno=EBUSY; // most likely
					break;
				}
				else
				{
					errno = ENOLINK; // some phony errno
					break;
				}
			}
		break;
		}
	}
	endmntent (fp);
	return ret;
}

int Scsi_Command::is_reload_needed ()
{ return ioctl (fd,CDROM_MEDIA_CHANGED,CDSL_CURRENT) == 0; }

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

Scsi_Command::Scsi_Command()	{ fd=-1, autoclose=1; filename=NULL; }
Scsi_Command::Scsi_Command(int f)	{ fd=f,  autoclose=0; filename=NULL; }
Scsi_Command::Scsi_Command(void*f){ fd=(long)f, autoclose=0; filename=NULL; }
Scsi_Command::~Scsi_Command()
{
	if (fd>=0 && autoclose) close(fd),fd=-1;
	if (filename) free(filename),filename=NULL;
}

int Scsi_Command::associate (const char *file,const struct stat *ref)
{
	struct stat sb;

	fd=open(file,O_RDWR|O_NONBLOCK);
	// this is --^^^^^^-- why we have to run set-root-uid...

	if (fd < 0)					return 0;
	if (fstat(fd,&sb) < 0)				return 0;
	if (!S_ISCHR(sb.st_mode))	{ errno=EINVAL; return 0; }

	if (ref && (!S_ISCHR(ref->st_mode) || ref->st_rdev!=sb.st_rdev))
	{   errno=ENXIO; return 0;   }

	filename=strdup(file);

	return 1;
}

unsigned char &Scsi_Command::operator[] (size_t i)
{
	if (i==0)
	{   memset(&req,0,sizeof(req));
	    req.flags = SCCMD_ESCAPE;
	    req.timeout = 30000;
	    req.senselen = 18; //sizeof(req.sense);
	}
	req.cmdlen = i+1;
	return req.cmd[i];
}

unsigned char &Scsi_Command::operator()(size_t i)	{ return req.sense[i]; }

unsigned char *Scsi_Command::sense()		{ return req.sense;    }

void Scsi_Command::timeout(int i)			{ req.timeout=i*1000; }
size_t Scsi_Command::residue()			{ return req.datalen-req.datalen_used; }
int Scsi_Command::transport(Direction dir,void *buf,size_t sz)
{
	int ret=0;
	/* cmd length fix */
//	printf("CMD: (%2d)  %02x\n", req.cmdlen, req.cmd[0]);
	req.cmdlen = (req.cmdlen < COMMAND_SIZE(req.cmd[0])) ? COMMAND_SIZE(req.cmd[0]):req.cmdlen;

	req.databuf = (caddr_t)buf;
	req.datalen = sz;
	req.flags |= dir;
	if (ioctl (fd,SCIOCCOMMAND,&req) < 0)	return -1;
	if (req.retsts==SCCMD_OK)		return 0;

	errno=EIO; ret=-1;
	if (req.retsts==SCCMD_SENSE)
	{
		ret = ERRCODE(req.sense);
	    if (ret==0) ret=-1;
	    else	CREAM_ON_ERRNO(req.sense);
	}
	return ret;
}

// this code is basically redundant... indeed, we normally want to
// open device O_RDWR, but we can't do that as long as it's mounted.
// in other words, whenever this routine is invoked, device is not
// mounted, so that it could as well just return 0;
int Scsi_Command::umount(int f)
{
	struct stat    fsb,msb;
#if defined(__NetBSD__)
    struct statvfs *mntbuf;
#else
    struct statfs *mntbuf;
#endif
    int            ret=0,mntsize,i;

	if (f==-1) f=fd;

	if (fstat (f,&fsb) < 0)				return -1;
	if ((mntsize=getmntinfo(&mntbuf,MNT_NOWAIT))==0)return -1;

	for (i=0;i<mntsize;i++)
	{
		char rdev[MNAMELEN+1],*slash,*rslash;

	    mntbuf[i].f_mntfromname[MNAMELEN-1]='\0';	// paranoia
	    if ((slash=strrchr (mntbuf[i].f_mntfromname,'/'))==NULL) continue;
	    strcpy (rdev,mntbuf[i].f_mntfromname); // rdev is 1 byte larger!
	    rslash = strrchr  (rdev,'/');
	    *(rslash+1) = 'r', strcpy (rslash+2,slash+1);
	    if (stat (rdev,&msb) < 0) continue;
	    if (msb.st_rdev == fsb.st_rdev)
	    {
			ret=unmount (mntbuf[i].f_mntonname,0);
			break;
        }
	}
	return ret;
}

int Scsi_Command::is_reload_needed ()
    {	return 1;   }

#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)

#include <sys/ioctl.h>
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

Scsi_Command::Scsi_Command()
{	cam=NULL, fd=-1, autoclose=1; filename=NULL;   }

Scsi_Command::Scsi_Command(int f)
{
	char pass[32];	// periph_name is 16 chars long

	cam=NULL, fd=-1, autoclose=1, filename=NULL;

	memset (&ccb,0,sizeof(ccb));
	ccb.ccb_h.func_code = XPT_GDEVLIST;
	if (ioctl (f,CAMGETPASSTHRU,&ccb) < 0) return;

	sprintf (pass,"/dev/%.15s%u",ccb.cgdl.periph_name,ccb.cgdl.unit_number);
	cam=cam_open_pass (pass,O_RDWR,NULL);
}

Scsi_Command::Scsi_Command(void *f)
{	cam=(struct cam_device *)f, autoclose=0; fd=-1; filename=NULL;  }

Scsi_Command::~Scsi_Command()
{
	if (cam && autoclose)	cam_close_device(cam), cam=NULL;
	if (fd>=0)		close(fd);
	if (filename)		free(filename), filename=NULL;
}

int Scsi_Command::associate (const char *file,const struct stat *ref)
{
	struct stat sb;
	char pass[32];		// periph_name is 16 chars long

	fd=open(file,O_RDONLY|O_NONBLOCK);

	// all if (ref) code is actually redundant, it never runs
	// as long as RELOAD_NEVER_NEEDED...
	if (ref && fd<0 && errno==EPERM)
	{
		// expectedly we would get here if file is /dev/passN
	    if (stat(file,&sb) < 0)		return 0;
	    if (!S_ISCHR(ref->st_mode) || ref->st_rdev!=sb.st_rdev)
		return (errno=ENXIO,0);
	    fd=open(file,O_RDWR);
	}

	if (fd < 0)				return 0;
	if (fstat(fd,&sb) < 0)			return 0;
	if (!S_ISCHR(sb.st_mode))		return (errno=EINVAL,0);

	if (ref && (!S_ISCHR(ref->st_mode) || ref->st_rdev!=sb.st_rdev))
	    return (errno=ENXIO,0);

	memset (&ccb,0,sizeof(ccb));
	ccb.ccb_h.func_code = XPT_GDEVLIST;
	if (ioctl(fd,CAMGETPASSTHRU,&ccb)<0)	return (close(fd),fd=-1,0);

	sprintf (pass,"/dev/%.15s%u",ccb.cgdl.periph_name,ccb.cgdl.unit_number);
	cam=cam_open_pass (pass,O_RDWR,NULL);
	if (cam==NULL)				return (close(fd),fd=-1,0);

	filename=strdup(file);

	return 1;
}

unsigned char& Scsi_Command::operator[] (size_t i)
{
	if (i==0)
	{
		memset(&ccb,0,sizeof(ccb));
		ccb.ccb_h.path_id    = cam->path_id;
		ccb.ccb_h.target_id  = cam->target_id;
		ccb.ccb_h.target_lun = cam->target_lun;
		cam_fill_csio (&(ccb.csio),

		1,				// retries
		NULL,				// cbfncp
		CAM_DEV_QFRZDIS,		// flags
		MSG_SIMPLE_Q_TAG,		// tag_action
		NULL,				// data_ptr
		0,				// dxfer_len
		sizeof(ccb.csio.sense_data),	// sense_len
		0,				// cdb_len
		30*1000);			// timeout
	}
	ccb.csio.cdb_len = i+1;
	return ccb.csio.cdb_io.cdb_bytes[i];
}

unsigned char& Scsi_Command::operator()(size_t i) 
	{ return ((unsigned char *)&ccb.csio.sense_data)[i]; }

unsigned char *Scsi_Command::sense()	{ return (unsigned char*)&ccb.csio.sense_data;    }

void Scsi_Command::timeout(int i)	{ ccb.ccb_h.timeout=i*1000; }
size_t Scsi_Command::residue()	{ return ccb.csio.resid; }
int Scsi_Command::transport(Direction dir,void *buf,size_t sz)
{
	int ret=0;

	ccb.csio.ccb_h.flags |= dir;
	ccb.csio.data_ptr  = (u_int8_t *)buf;
	ccb.csio.dxfer_len = sz;

	if ((ret = cam_send_ccb(cam, &ccb)) < 0)
	    return -1;

	if ((ccb.ccb_h.status & CAM_STATUS_MASK) == CAM_REQ_CMP)
	    return 0;

	/* cmd length fix */
//	printf("CMD: (%2d)  %02x\n", ccb.csio.cdb_len, ccb.csio.cdb_io.cdb_bytes[0]);
	ccb.csio.cdb_len = (ccb.csio.cdb_len < COMMAND_SIZE(ccb.csio.cdb_io.cdb_bytes[0])) ? COMMAND_SIZE(ccb.csio.cdb_io.cdb_bytes[0]):ccb.csio.cdb_len;

	unsigned char  *sense=(unsigned char *)&ccb.csio.sense_data;

	errno = EIO;
	// FreeBSD 5-CURRENT since 2003-08-24, including 5.2 fails to
	// pull sense data automatically, at least for ATAPI transport,
	// so I reach for it myself...
	if ((ccb.csio.scsi_status==SCSI_STATUS_CHECK_COND) &&
	    !(ccb.ccb_h.status&CAM_AUTOSNS_VALID))
	{   u_int8_t  _sense[18];
	    u_int32_t resid=ccb.csio.resid;

	    memset(_sense,0,sizeof(_sense));

	    operator[](0)      = 0x03;	// REQUEST SENSE
	    ccb.csio.cdb_io.cdb_bytes[4] = sizeof(_sense);
	    ccb.csio.cdb_len   = 6;
	    ccb.csio.ccb_h.flags |= CAM_DIR_IN|CAM_DIS_AUTOSENSE;
	    ccb.csio.data_ptr  = _sense;
	    ccb.csio.dxfer_len = sizeof(_sense);
	    ccb.csio.sense_len = 0;
	    ret = cam_send_ccb(cam, &ccb);

	    ccb.csio.resid = resid;
	    if (ret<0)	return -1;
	    if ((ccb.ccb_h.status&CAM_STATUS_MASK) != CAM_REQ_CMP)
		return errno=EIO,-1;

	    memcpy(sense,_sense,sizeof(_sense));
	}

	ret = ERRCODE(sense);
	if (ret == 0)	ret = -1;
	else		CREAM_ON_ERRNO(sense);

	return ret;
}

int Scsi_Command::umount(int f)
{
	struct stat    fsb,msb;
    struct statfs *mntbuf;
    int            ret=0,mntsize,i;

	if (f==-1) f=fd;

	if (fstat (f,&fsb) < 0)				return -1;
	if ((mntsize=getmntinfo(&mntbuf,MNT_NOWAIT))==0)return -1;

	for (i=0;i<mntsize;i++)
	{
		if (stat (mntbuf[i].f_mntfromname,&msb) < 0) continue;
	    if (msb.st_rdev == fsb.st_rdev)
	    {
			ret=unmount (mntbuf[i].f_mntonname,0);
			break;
	    }
	}
	return ret;
}

int Scsi_Command::is_reload_needed ()
	{  return 0;   }

//*
#elif defined(_WIN32)

Scsi_Command::Scsi_Command()		{ fd=INVALID_HANDLE_VALUE; autoclose=1; filename=NULL; }
Scsi_Command::Scsi_Command(void*f)	{ fd=f, autoclose=0; filename=NULL; }
Scsi_Command::~Scsi_Command()
{
	DWORD junk;
	if (fd!=INVALID_HANDLE_VALUE && autoclose)
	{
		if (autoclose>1)
		DeviceIoControl(fd,FSCTL_UNLOCK_VOLUME, 
			NULL,0,NULL,0,&junk,NULL);
		CloseHandle (fd),fd=INVALID_HANDLE_VALUE;
	}
	if (filename) free(filename),filename=NULL;
}

int	Scsi_Command::associate (const char *file,const struct stat *ref)
{
	char dev[32];
	sprintf(dev,"%.*s\\",sizeof(dev)-2,file);
	if (GetDriveType(dev)!=DRIVE_CDROM)
		return errno=EINVAL,0;
	sprintf(dev,"\\\\.\\%.*s",sizeof(dev)-5,file);
	fd=CreateFile (dev,GENERIC_WRITE|GENERIC_READ,
			   FILE_SHARE_READ|FILE_SHARE_WRITE,
			   NULL,OPEN_EXISTING,0,NULL);
	if (fd!=INVALID_HANDLE_VALUE)
		filename=strdup(dev);
	return fd!=INVALID_HANDLE_VALUE;
}


unsigned char &Scsi_Command::operator[] (size_t i)
{
	if (i==0)
	{
		memset(&p,0,sizeof(p));
	    p.spt.Length = sizeof(p.spt);
		p.spt.DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
		p.spt.TimeOutValue = 30;
		p.spt.SenseInfoLength = sizeof(p.sense);
		p.spt.SenseInfoOffset = offsetof(SPKG,sense);
	}
	p.spt.CdbLength = i+1;
	return p.spt.Cdb[i];
}

unsigned char &Scsi_Command::operator()(size_t i)	{ return p.sense[i]; }
unsigned char *Scsi_Command::sense() { return p.sense; };

void 	Scsi_Command::timeout(int i)	{ p.spt.TimeOutValue=i; }
int 	Scsi_Command::transport(Direction dir,void *buf,size_t sz)
{
	DWORD bytes;
	int   ret=0;

	/* cmd length fix */
//	printf("CMD: (%2d)  %02x\n", p.spt.CdbLength, p.spt.Cdb[0]);
	p.spt.CdbLength = (p.spt.CdbLength < COMMAND_SIZE(p.spt.Cdb[0])) ? COMMAND_SIZE(p.spt.Cdb[0]):p.spt.CdbLength;

	p.spt.DataBuffer = buf;
	p.spt.DataTransferLength = sz;
	p.spt.DataIn = dir;

	if (DeviceIoControl (fd,IOCTL_SCSI_PASS_THROUGH_DIRECT,
				&p,sizeof(p.spt),
				&p,sizeof(p),
				&bytes,FALSE) == 0) return -1;

	if (p.sense[0]&0x70)
	{
		SetLastError (ERROR_GEN_FAILURE);
		ret = ERRCODE(p.sense);
		if (ret==0) ret=-1;
		else CREAM_ON_ERRNO(p.sense);
	}
#if 0
	else if (p.spt.Cdb[0] == 0x00)	// TEST UNIT READY
	{ unsigned char _sense[18];

	    operator[](0)   = 0x03;	// REQUEST SENSE
	    p.spt.Cdb[4]    = sizeof(_sense);
	    p.spt.CdbLength = 6;

	    p.spt.DataBuffer = _sense;
	    p.spt.DataTransferLength = sizeof(_sense);
	    p.spt.DataIn = READ;

	    if (DeviceIoControl (fd,IOCTL_SCSI_PASS_THROUGH_DIRECT,
				&p,sizeof(p.spt),
				&p,sizeof(p),
				&bytes,FALSE) == 0) return -1;

	    if ((ret = ERRCODE(_sense))) CREAM_ON_ERRNO(_sense);
	}
#endif
	return ret;
}

int	Scsi_Command::umount (int f)
{
	DWORD junk;
	HANDLE h = (f==-1) ? fd : (HANDLE)f;

	if (DeviceIoControl(h,FSCTL_LOCK_VOLUME,NULL,0,NULL,0,&junk,NULL) &&
		DeviceIoControl(h,FSCTL_DISMOUNT_VOLUME,NULL,0,NULL,0,&junk,NULL))
	{
		if (h==fd) autoclose++;
		return 0;
	}
	return -1;
}

int	Scsi_Command::is_reload_needed ()	{   return 0;   }

//*/

#elif defined (__APPLE__) && defined (__MACH__)


static int iokit_err   (IOReturn ioret,SCSITaskStatus stat,
			const unsigned char *sense)
{ int ret=-1;

    if      (ioret==kIOReturnSuccess)		ret = 0;
    else if (ioret==kIOReturnNoDevice)		errno = ENXIO;
    else if (ioret==kIOReturnNoMemory)		errno = ENOMEM;
    else if (ioret==kIOReturnExclusiveAccess)	errno = EBUSY;
    else					errno = EIO;

    if (ret) return ret;

    if (stat==kSCSITaskStatus_CHECK_CONDITION)
    {	ret = ERRCODE(sense);
	if (ret==0)	errno=EIO, ret=-1;
	else		CREAM_ON_ERRNO(sense);
    }
    else if (stat!=kSCSITaskStatus_GOOD)
	errno = EIO, ret = -1;

  return ret;
}

Scsi_Command::Scsi_Command()
{
	scsiob=IO_OBJECT_NULL, plugin=NULL, mmcdif=NULL, taskif=NULL;
	autoclose=1; filename=NULL;
}

Scsi_Command::Scsi_Command(void *f)
{	
	taskif = (SCSITaskDeviceInterface **)f, autoclose=0; filename=NULL;
}

Scsi_Command::~Scsi_Command()
{
	if (autoclose)
	{   if (taskif)		(*taskif)->ReleaseExclusiveAccess(taskif),
			(*taskif)->Release(taskif),       taskif=NULL;
		if (mmcdif)		(*mmcdif)->Release(mmcdif),       mmcdif=NULL;
		if (plugin)		IODestroyPlugInInterface(plugin), plugin=NULL;
		if (scsiob)		IOObjectRelease(scsiob), scsiob=IO_OBJECT_NULL;
	}
	if (filename)		free(filename), filename=NULL;
}

int Scsi_Command::associate (const char *file,const struct stat *ref)
{
	struct stat		sb;
    io_object_t		scsiob=IO_OBJECT_NULL,parent;
    CFMutableDictionaryRef	match,bsddev;
    CFNumberRef		num;
    int				i;

	if (ref)			sb = *ref;
	else if (stat(file,&sb))	return 0;

	if (!(S_ISBLK(sb.st_mode) || S_ISCHR(sb.st_mode)))
	    return !(errno=ENOTBLK);

	if ((match = CFDictionaryCreateMutable(kCFAllocatorDefault,0,
				&kCFTypeDictionaryKeyCallBacks,
				&kCFTypeDictionaryValueCallBacks))
	    == NULL)	return !(errno=ENOMEM);
	if ((bsddev = CFDictionaryCreateMutable(kCFAllocatorDefault,0,
				&kCFTypeDictionaryKeyCallBacks,
				&kCFTypeDictionaryValueCallBacks))
	    == NULL)	return CFRelease(match),!(errno=ENOMEM);

	i = major(sb.st_rdev);
	num = CFNumberCreate(kCFAllocatorDefault,kCFNumberIntType,&i);
	CFDictionarySetValue(bsddev,CFSTR("BSD Major"),num);
	CFRelease(num);

	i = minor(sb.st_rdev);
	num = CFNumberCreate(kCFAllocatorDefault,kCFNumberIntType,&i);
	CFDictionarySetValue(bsddev,CFSTR("BSD Minor"),num);
	CFRelease(num);

	CFDictionarySetValue(match,CFSTR(kIOPropertyMatchKey),bsddev);
	CFRelease(bsddev);

	if ((scsiob = IOServiceGetMatchingService(kIOMasterPortDefault,match))
	    == IO_OBJECT_NULL)	return !(errno=ENXIO);

	// traverse up to "SCSITaskAuthoringDevice"
	kern_return_t	kret;
	while ((kret=IORegistryEntryGetParentEntry(scsiob,kIOServicePlane,
				&parent)) == kIOReturnSuccess)
	{
		CFStringRef	uclient;
		const char	*s;
		int		cmp;

	    IOObjectRelease(scsiob);
	    scsiob = parent;
	    uclient = (CFStringRef)IORegistryEntryCreateCFProperty(scsiob,
				CFSTR(kIOPropertySCSITaskDeviceCategory),
				kCFAllocatorDefault,0);
	    if (uclient)
	    {
			s = CFStringGetCStringPtr(uclient,kCFStringEncodingMacRoman);
			cmp = strcmp(s,kIOPropertySCSITaskAuthoringDevice);
			CFRelease(uclient);
			if (cmp==0)	break;
	    }
	}
	if (kret!=kIOReturnSuccess)
	{
		if (scsiob!=IO_OBJECT_NULL)	IOObjectRelease(scsiob);
	    return !(errno=ENXIO);
	}

	SInt32	score=0;
	if (IOCreatePlugInInterfaceForService(scsiob,
				kIOMMCDeviceUserClientTypeID,
				kIOCFPlugInInterfaceID,
				&plugin,&score) != kIOReturnSuccess)
	{
		IOObjectRelease(scsiob);
	    return !(errno=ENXIO);
	}
	if ((*plugin)->QueryInterface(plugin,
				CFUUIDGetUUIDBytes(kIOMMCDeviceInterfaceID),
				(void**)&mmcdif) != S_OK)
	{
		IODestroyPlugInInterface(plugin),	plugin=NULL;
	    IOObjectRelease(scsiob);
	    return !(errno=ENXIO);
	}
	if ((taskif = (*mmcdif)->GetSCSITaskDeviceInterface(mmcdif)) == NULL)
	{
		(*mmcdif)->Release(mmcdif),		mmcdif=NULL;
	    IODestroyPlugInInterface(plugin),	plugin=NULL;
	    IOObjectRelease(scsiob);
	    return !(errno=ENXIO);
	}

	//
	// Note that in order to ObtainExclusiveAccess no corresponding
	// /dev/[r]diskN may remain open by that time. For reference,
	// acquiring exclusive access temporarily removes BSD block
	// storage device from I/O registry as well as corresponding
	// /dev entries.
	//
	if ((*taskif)->ObtainExclusiveAccess(taskif) != kIOReturnSuccess)
	{
		(*taskif)->Release(taskif),		taskif=NULL;
	    (*mmcdif)->Release(mmcdif),		mmcdif=NULL;
	    IODestroyPlugInInterface(plugin),	plugin=NULL;
	    IOObjectRelease(scsiob),		scsiob=IO_OBJECT_NULL;
	    return !(errno=EBUSY);
	}

	filename=strdup(file);

	return 1;
}

unsigned char& Scsi_Command::operator[] (size_t i)
{
	if (i==0)
	{
		memset (cdb,0,sizeof(cdb));
	    memset (&_sense,0,sizeof(_sense));
	    _timeout = 30;
	}
	cdblen = i+1;
	return cdb[i];
}

unsigned char& Scsi_Command::operator()(size_t i)	{ return _sense.u[i]; }
unsigned char* Scsi_Command::sense()		{ return _sense.u;    }

void Scsi_Command::timeout(int i)			{ _timeout=i;         }
size_t Scsi_Command::residue()			{ return resid;       }

int Scsi_Command::transport(Direction dir,void *buf,size_t sz)
{
	int		ret=0;
	SCSITaskInterface **cmd;
	SCSITaskStatus	stat;
	UInt64		bytes;
	IOVirtualRange	range = { (IOVirtualAddress)buf, sz };

	/* cmd length fix */
//	printf("CMD: (%2d)  %02x\n", cdblen, cdb[0]);
	cdblen = (cdblen < COMMAND_SIZE(cdb[0])) ? COMMAND_SIZE(cdb[0]):cdblen;

	resid = sz;
	cmd = (*taskif)->CreateSCSITask(taskif);
	if (cmd==NULL)	return (errno=ENOMEM),-1;

	(*cmd)->SetCommandDescriptorBlock(cmd,cdb,cdblen);
	(*cmd)->SetScatterGatherEntries(cmd,&range,1,sz,dir);
	(*cmd)->SetTimeoutDuration(cmd,_timeout*1000);

	if ((*cmd)->ExecuteTaskSync(cmd,&_sense.s,&stat,&bytes)
	    != kIOReturnSuccess)
	    errno=EIO, ret=-1;
	else if (stat==kSCSITaskStatus_GOOD)
	{
		resid = sz - bytes;   
	}
	else if (stat==kSCSITaskStatus_CHECK_CONDITION)
	{
		ret = ERRCODE(_sense.u);
	    if (ret==0)	errno=EIO, ret=-1;
	    else	CREAM_ON_ERRNO(_sense.u);
	}
	else
	{
		//SCSIServiceResponse resp;
	    //(*taskif)->GetSCSIServiceResponse(taskif,&resp);
	    errno=EIO, ret=-1;
	}

	(*cmd)->Release(cmd);
	return ret;
}


int Scsi_Command::umount(int f)
{
	struct stat	sb;
    dev_t	ref;
    int		i,n;

	if (f>=0)
	{
		if (fstat (f,&sb))		return -1;
	    if (!S_ISCHR(sb.st_mode))	return errno=ENOTBLK,-1;
	    ref = sb.st_rdev;
	    // /dev/rdiskN and /dev/diskN have same st_rdev
	}
	else
	{
		char bsdname [16];
		CFStringRef devname = (CFStringRef)IORegistryEntrySearchCFProperty (
					scsiob,kIOServicePlane,
					CFSTR("BSD Name"),
					kCFAllocatorDefault,
					kIORegistryIterateRecursively);
	    if (devname==NULL)	return 0;	// already exclusive

	    sprintf (bsdname,"/dev/%.*s",(int)(sizeof(bsdname)-6),
				CFStringGetCStringPtr (devname,0));
	    CFRelease (devname);

	    if (stat (bsdname,&sb))			return -1;
	    ref = sb.st_rdev;
	}

	if ((n=getfsstat (NULL,0,MNT_NOWAIT)) < 0)	return -1;
	n += 4, n *= sizeof(struct statfs);

	struct statfs *p = (struct statfs *)alloca(n);
	if ((n=getfsstat (p,n,MNT_NOWAIT)) < 0)		return -1;

	for (i=0;i<n;i++,p++)
	{
		if (stat (p->f_mntfromname,&sb)==0 &&
		S_ISBLK(sb.st_mode) &&
		sb.st_rdev==ref)
#if 0	    // looks neat, but causes irritaing popups on console...
		return unmount (p->f_mntonname,0);
#else
	    {
			int	ret=0,rval;
			pid_t	pid,rpid;

			ret = -1;
			if ((pid = fork()) == (pid_t)-1)	return -1;
			if (pid == 0)	// if diskutil will be proven broken,
					// don't allow growisofs to be used as
					// attack vector...
					setuid (getuid ()),
					execl  ("/usr/sbin/diskutil",
						"diskutil","unmount",
						p->f_mntonname,(void*)NULL),
					exit (errno);
			while (1)
			{
				rpid = waitpid (pid,&rval,0);
				if (rpid == (pid_t)-1)
				{
					if (errno==EINTR)	continue;
					else			break;
				}
				else if (rpid != pid)
				{	errno = ECHILD;
					break;
				}
				if (WIFEXITED(rval))
				{
					if (WEXITSTATUS(rval) == 0)	ret=0;
					else				errno=EBUSY; // most likely
					break;
				}
				else if (WIFSTOPPED(rval) || WIFCONTINUED(rval))
					continue;
				else
				{	errno = ENOLINK;	// some phony errno
					break;
				}
			}

		// diskutil(8) seem to unmount only volfs-managed
		// media, so I check if it managed to unmount and
		// try the system call if it didn't...
			if (ret==0)
			{
				struct statfs fs;

				if (statfs (p->f_mntonname,&fs)==0 &&
				!strcmp (fs.f_mntfromname,p->f_mntfromname))
				return unmount (p->f_mntonname,0);
			}
			return ret;
	    }
#endif
	}

    return 0; // not mounted?	
}

#define RELOAD_NEVER_NEEDED
int Scsi_Command::is_reload_needed(int not_used)
{	return 0;   }


#else
#error "Unsupported OS"
#endif

