/*
 *
 * disc reader algo for DeadDiscReader
 * Copyright (C) 2007-2009, Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 *
 * initAllCSSKeys function from libdvdread
 *
 */

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>

#if (DVDCSS_KEY_CACHE > 0)
#include <fcntl.h>
#include <unistd.h>
#endif

#include <sys/time.h>

#include <qpx_mmc.h>
#include <sectmap.h>
#include <imgwriter.h>

#include <dvd_udf.h>

#include <reader.h>
#include <threads.h>
#include "reader_disc.h"

#define sector_sz 2048

#if (DVDCSS_KEY_CACHE > 0)
static int initCSSCache(drive_info *dev)
{
	char psz_buffer[PATH_MAX];
	char *psz_home;
	char *psz_cache;
	
	psz_home = getenv( "HOME" );
	if( psz_home == NULL )
		psz_home = getenv( "USERPROFILE" );

    /* Cache our keys in ${HOME}/.dvdcss/ */
    if( psz_home )
    {
		snprintf( psz_buffer, PATH_MAX, "%s/.dvdcss", psz_home );
		psz_buffer[PATH_MAX-1] = '\0';
		psz_cache = psz_buffer;
	}


	if( psz_cache != NULL )
    {
        /* Check that we can add the ID directory and the block filename */
        if( strlen( psz_cache ) + 1 + 32 + 1 + (DVD_KEY_SIZE * 2) + 10 + 1  > PATH_MAX )
        {
            printf( "CSS: cache directory name is too long\n" );
            psz_cache = NULL;
        }
    }

	if( !psz_cache ) goto nocache;

    /* If the cache is enabled, write the cache directory tag */
    if( psz_cache )
    {
        char *psz_tag = "Signature: 8a477f597d28d172789f06886806bc55\r\n"
            "# This file is a cache directory tag created by libdvdcss.\r\n"
            "# For information about cache directory tags, see:\r\n"
            "#   http://www.brynosaurus.com/cachedir/\r\n";
        char psz_tagfile[PATH_MAX + 1 + 12 + 1];
        int i_fd;

        sprintf( psz_tagfile, "%s/CACHEDIR.TAG", psz_cache );
        i_fd = open( psz_tagfile, O_RDWR|O_CREAT, 0644 );
        if( i_fd >= 0 )
        {
            write( i_fd, psz_tag, strlen(psz_tag) );
            close( i_fd );
        }
    }

    /* If the cache is enabled, extract a unique disc ID */
    if( psz_cache )
    {
        unsigned char p_sector[DVDCSS_BLOCK_SIZE];
       // char psz_debug[PATH_MAX + 30];
        char psz_key[1 + DVD_KEY_SIZE * 2 + 1];
        char *psz_title;
        unsigned char *psz_serial;
        int i;

        /* We read sector 0. If it starts with 0x000001ba (BE), we are
         * reading a VOB file, and we should not cache anything. */

        if (!read( dev, p_sector, 0, 1 ))
        {
            goto nocache;
        }

        if( p_sector[0] == 0x00 && p_sector[1] == 0x00
             && p_sector[2] == 0x01 && p_sector[3] == 0xba )
        {
            goto nocache;
        }

        /* The data we are looking for is at sector 16 (32768 bytes):
         *  - offset 40: disc title (32 uppercase chars)
         *  - offset 813: manufacturing date + serial no (16 digits) */

		if (!read( dev, p_sector, 16, 1 ))
        {
            goto nocache;
        }

        /* Get the disc title */
        psz_title = (char *)p_sector + 40;
        psz_title[32] = '\0';

        for( i = 0 ; i < 32 ; i++ )
        {
            if( psz_title[i] <= ' ' )
            {
                psz_title[i] = '\0';
                break;
            }
            else if( psz_title[i] == '/' || psz_title[i] == '\\' )
            {
                psz_title[i] = '-';
            }
        }

        /* Get the date + serial */
        psz_serial = p_sector + 813;
        psz_serial[16] = '\0';

        /* Check that all characters are digits, otherwise convert. */
        for( i = 0 ; i < 16 ; i++ )
        {
            if( psz_serial[i] < '0' || psz_serial[i] > '9' )
            {
                char psz_tmp[16 + 1];
                sprintf( psz_tmp,
                         "%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",
                         psz_serial[0], psz_serial[1], psz_serial[2],
                         psz_serial[3], psz_serial[4], psz_serial[5],
                         psz_serial[6], psz_serial[7] );
                memcpy( psz_serial, psz_tmp, 16 );
                break;
            }
        }

        /* Get disk key, since some discs have got same title, manufacturing
         * date and serial number, but different keys */
        if( dev->media.dvdcss.protection == 0x01 )
        {
             psz_key[0] = '-';
             for( i = 0; i < DVD_KEY_SIZE; i++ )
             {
                 sprintf( &psz_key[1+i*2], "%.2x", dev->media.dvdcss.DK[i] );
             }
             psz_key[1 + DVD_KEY_SIZE * 2] = '\0';
        }
        else
        {
             psz_key[0] = 0;
        }

        /* We have a disc name or ID, we can create the cache dir */
        i = sprintf( dev->media.dvdcss.psz_cachefile, "%s", psz_cache );
        if (mkdir( dev->media.dvdcss.psz_cachefile, 0755 ) < 0 && errno != EEXIST )
        {
            printf( "CSS: failed creating cache directory\n" );
            dev->media.dvdcss.psz_cachefile[0] = '\0';
            goto nocache;
        }

        i += sprintf( dev->media.dvdcss.psz_cachefile + i, "/%s-%s%s", psz_title,
                      psz_serial, psz_key );
        if ( mkdir( dev->media.dvdcss.psz_cachefile, 0755 ) < 0 && errno != EEXIST )
        {
            printf( "CSS: failed creating cache subdirectory\n" );
            dev->media.dvdcss.psz_cachefile[0] = '\0';
            goto nocache;
        }
        i += sprintf( dev->media.dvdcss.psz_cachefile + i, "/");

        /* Pointer to the filename we will use. */
        dev->media.dvdcss.psz_block = dev->media.dvdcss.psz_cachefile + i;

        printf( "CSS: using CSS key cache dir: %s\n", dev->media.dvdcss.psz_cachefile );
    }
nocache:
	return 0;
}
#endif


/* Loop over all titles and call dvdcss_title to crack the keys. */
static int initAllCSSKeys( drive_info *dev )
{
  struct timeval all_s, all_e;
  struct timeval t_s, t_e;
  char filename[ MAX_UDF_FILE_NAME_LEN ];
  unsigned int start, len;
  int title;
//  int stitle;
//  bool st0;

  udf_t udf;
/*        
  char *nokeys_str = getenv("DVDREAD_NOKEYS");
  if(nokeys_str != NULL)
    return 0;
*/
  udf.dev = dev;
  udf.cache = NULL;

  if (!dev->silent) {
	  printf( "libdvdread: Attempting to retrieve all CSS keys\n" );
  }

  gettimeofday(&all_s, NULL);
        
  for( title = 0; title < 100; title++ ) {
//	stitle=0; st0=0;
    gettimeofday( &t_s, NULL );
    if( title == 0 ) {
      sprintf( filename, "/VIDEO_TS/VIDEO_TS.VOB" );
    } else {
      sprintf( filename, "/VIDEO_TS/VTS_%02d_%d.VOB", title, 0 );
    }
    start = UDFFindFile( &udf, filename, &len );
    if( start != 0 && len != 0 ) {
      /* Perform CSS key cracking for this title. */
//	  st0=1;
	  if (!dev->silent) {
        printf( "libdvdread: Get key for %s at 0x%08x, len %10d\n", filename, start, len );
      }
      if( css_title( dev, (int)start ) < 0 ) {
	    if (!dev->silent) {
          printf( "libdvdread: Error cracking CSS key for %s (0x%08x)\n", filename, start);
        }
      }
      gettimeofday( &t_e, NULL );
	  if (!dev->silent) {
        printf( "libdvdread: Elapsed time %ld\n",  
                 (long int) t_e.tv_sec - t_s.tv_sec );
      }
	} 
            
    if( title == 0 ) continue;
    
//	for (stitle = 1; stitle<9; stitle++) {
	//	printf("stitle=%d\n", stitle);
		gettimeofday( &t_s, NULL );
		//sprintf( filename, "/VIDEO_TS/VTS_%02d_%d.VOB", title, stitle );
		sprintf( filename, "/VIDEO_TS/VTS_%02d_%d.VOB", title, 1 );
		start = UDFFindFile( &udf, filename, &len );
		if( start == 0 || len == 0 ) break;
            
		/* Perform CSS key cracking for this title. */
		if (!dev->silent) {
			printf( "libdvdread: Get key for %s at 0x%08x, len %10d\n", filename, start, len );
		}
		if( css_title( dev, (int)start ) < 0 ) {
			if (!dev->silent) {
				printf( "libdvdread: Error cracking CSS key for %s (0x%08x)!!\n", filename, start);
			}
		}
		gettimeofday( &t_e, NULL );
		if (!dev->silent) {
			printf( "libdvdread: Elapsed time %ld\n",  
               (long int) t_e.tv_sec - t_s.tv_sec );
		}
//	}
//	if( (!st0) && (stitle !=0) && (start == 0 || len == 0) ) break;
  }
  title--;
    
  if (!dev->silent) {
    printf( "libdvdread: Found %d VTS's\n", title );
  }
  gettimeofday(&all_e, NULL);
  if (!dev->silent) {
    printf( "libdvdread: Elapsed time %ld\n",  
             (long int) all_e.tv_sec - all_s.tv_sec );
  }
  FreeUDFCache( &udf, udf.cache);
  if (!dev->silent) {
	printf("Title keys:\n");

    dvd_title_t *p_title = dev->media.dvdcss.p_titles;
    while( p_title != NULL )
//		&& p_title->p_next != NULL
    {
		printf("STA: %8X,  KEY: %02X:%02X:%02X:%02X:%02X\n",p_title->i_startlb,
				p_title->p_key[0],
				p_title->p_key[1],
				p_title->p_key[2],
				p_title->p_key[3],
				p_title->p_key[4]);

		p_title = p_title->p_next;
    }
  }
  return 0;
}

void *read_disc(void* arg) {
    rdparm_t* parm  = (rdparm_t*) arg;
    drive_info* dev = parm->dev;
    smap*		map = parm->map;
    imgwriter*  iso = parm->iso;
    parm->stop=0;
    parm->running=1;
    parm->cnt_ok = 0;
    printf("%s: Starting reader thread #%d...\n", dev->device, parm->tidx);
	dvd_title_t *p_title = NULL;
	uint32_t	title_next = 0xFFFFFFFF;

	struct timeval t_s, t_e;
	int32_t    sects1X=75;
    uint32_t  lba = 0;
    int32_t	  scnt=1;
    int32_t	  ctry=0;
	bool	  descramble=0;

	dev->silent=0;

    if (dev->media.type & DISC_CD)  {
		printf("%s: media is CD\n",dev->device);
		scnt=15;
		sects1X=75;
	} else if (dev->media.type & DISC_DVD) {
		printf("%s: media is DVD\n",dev->device);
		scnt=16;
		sects1X=693;
		get_rpc_state(dev);
		read_disc_regions(dev);

		if (!dev->media.dvdcss.protection) {
			printf("DVD is NOT copy-protected\n");
		} else {
			switch (dev->media.dvdcss.protection) {
				case 0x01:
					printf("DVD is CSS-protected\n");
					break;
				case 0x02:
					printf("DVD is CPRM-protected\n");
					break;
				default:
					printf("Unknown DVD protection shceme!\n");
					break;
			}
			printf("Disc regions       : ");
			if (dev->media.dvdcss.regmask != 0xFF) {
				for (int i=0; i<8; i++)
					if (!((dev->media.dvdcss.regmask >> i) & 1))
						{printf("%d",i+1); /*dev->rpc.region = i+1;*/}
				printf("\n");
			} else {
				printf("Invalid region mask!\n");
			}
		}


		switch (dev->media.dvdcss.protection) {
			case 0: // unprotected DVD
				dev->media.dvdcss.method = DVDCSS_METHOD_NONE;
				break; 
			case 1: // CSS/CPPM protected DVD
				// just to auth to be able read data
//				scnt=1;
				dev->media.dvdcss.method = DVDCSS_METHOD_KEY;
				//dev->media.dvdcss.method = DVDCSS_METHOD_DISC;
				if (css_disckey(dev)) {
					printf("DVD auth failure!\n");
					goto exit_reader;
				}
				initAllCSSKeys( dev );
				break; 
			case 2: // CPRM - protected DVD
				// just to auth to be able read data
				dev->media.dvdcss.method = DVDCSS_METHOD_KEY;
				if (css_disckey(dev)) {
					printf("DVD auth failure!\n");
					goto exit_reader;
				}
				break; 
			default:
			//	printf("Unknown DVD protection scheme: %02X\n",dev->media.dvdcss.protection);
				dev->media.dvdcss.method = DVDCSS_METHOD_NONE;
				break;
		}
	} else {
		printf("Unsupported media!\n");
		goto exit_reader;
	}

    if (parm->pass >= PASS_RECOVER) scnt=1;

    map->lock();
    if (parm->pass < PASS_RECOVER) {
		map->get_next(&lba, BM_WAIT, &scnt);
	} else {
		bool rce,wce;
		printf("Recover pass - trying to disable read cache...\n");
		get_cache(dev,&rce,&wce);
		if (!rce) {
			printf("Read cache already disabled:)\n");
		} else {
			if (set_cache(dev,0,wce)) {
				printf("It seems drive does not support disabling read cache!\n");
			} else {
				printf("Read cache disabled successfully\n");
			}
		}
		map->get_next(&lba, BM_FAIL, &scnt);
	}
    map->set(lba, BM_READ, scnt);
    map->unlock();

	if (dev->media.dvdcss.protection == 0x01) {
		for (int i=0; i<DVD_KEY_SIZE; i++) dev->media.dvdcss.TK[i]=0;
		p_title = dev->media.dvdcss.p_titles;
		descramble = 0;
		if (!p_title) {
			title_next = 0xFFFFFFFF;
		} else {
			title_next = p_title->i_startlb;
		}
		printf("\nSetting title key: %02X:%02X:%02X:%02X:%02X  for %x-%x, CSS=%d\n",
			dev->media.dvdcss.TK[0],
			dev->media.dvdcss.TK[1],
			dev->media.dvdcss.TK[2],
			dev->media.dvdcss.TK[3],
			dev->media.dvdcss.TK[4],
			0, title_next,
			descramble);
	}

	gettimeofday(&t_e,NULL);
	seek(dev,0);
    while (lba < dev->media.capacity) {
		if (lba == 0xFFFFFFFF) goto exit_reader;
		t_s.tv_sec  = t_e.tv_sec;
		t_s.tv_usec = t_e.tv_usec;

		if (dev->media.type & DISC_CD)  {
			read(dev, dev->rd_buf, lba, scnt);
		} else if (dev->media.type & DISC_DVD) {
			if (dev->media.dvdcss.protection == 0x01) {

				if (lba >= title_next && p_title) {
				//	scnt = 16;
					while (lba >= title_next) {
						if (p_title->p_next) {
							title_next = p_title->p_next->i_startlb;
						} else {
							title_next = 0xFFFFFFFF;
						}
						if (lba >= title_next) p_title = p_title->p_next;
					}

					memcpy( dev->media.dvdcss.TK, p_title->p_key, sizeof(dvd_key_t) );
					if( ! memcmp( dev->media.dvdcss.TK, "\0\0\0\0\0", 5 ) ) descramble = 0; else descramble=1;
					printf("\nSetting title key: %02X:%02X:%02X:%02X:%02X  for %x-%x, CSS=%d\n",
							dev->media.dvdcss.TK[0],
							dev->media.dvdcss.TK[1],
							dev->media.dvdcss.TK[2],
							dev->media.dvdcss.TK[3],
							dev->media.dvdcss.TK[4],
							lba, title_next,
							descramble);
					p_title = p_title->p_next;
				}

				read_dvd(dev, dev->rd_buf, lba, scnt, descramble ? DVDCSS_READ_DECRYPT : 0);
			} else {
				read(dev, dev->rd_buf, lba, scnt);
			}
		}
		gettimeofday(&t_e,NULL);

		printf("%s: %5.2f X, lba %7d / %7d  ( %06x / %06x ) scnt=%d\r",
				dev->device,
				((float)scnt/(float)sects1X)/((t_e.tv_sec - t_s.tv_sec) + (t_e.tv_usec - t_s.tv_usec) / 1000000.0),
				lba, dev->media.capacity,
				lba, dev->media.capacity,
//				map->get_done() * 100 / dev->media.capacity,
				scnt);
		if ((dev->err & 0x0FFF00) == 0x23A00) {
			printf("%s: media removed!\n", dev->device);
    	    map->lock();
			map->set(lba, BM_WAIT, scnt);
			map->unlock();
			goto exit_reader;
		}

		switch (parm->pass) {
			case PASS_FIRST:
    	    case PASS_CONT:
				if (!dev->err) {
					if (iso->write(lba, scnt, sector_sz, (void*)dev->rd_buf) < scnt) {
						map->set(lba, BM_WAIT, scnt);
						goto exit_reader;
					}
					ctry=0;
				} else {
					ctry++;
				}
				break;
			case PASS_RECOVER0:
			case PASS_RECOVER1:
			default:
				if (!dev->err) {
					if (iso->write(lba, scnt, sector_sz, (void*)dev->rd_buf) < scnt) {
						map->set(lba, BM_WAIT, scnt);
						goto exit_reader;
					}
					ctry=0;
				} else {
					ctry++;
				}
				break;
		}
	
        map->lock();
// set current block state
		if (!ctry) {
			map->set(lba, BM_DONE, scnt);
			parm->cnt_ok+=scnt;
			lba+=scnt;
		} else {
			if (parm->pass < PASS_RECOVER)
				map->set(lba, BM_FAIL, scnt);
			if (ctry == parm->tries) ctry=0;
		}
		map->unlock();

		if (parm->stop) goto exit_reader;

		map->lock();
// get next block address
		if (parm->pass < PASS_RECOVER1) {
			if ((dev->media.type) & DISC_DVD && (dev->media.dvdcss.protection == 0x01)) {
				if ((lba!=title_next) && (lba+scnt > title_next) && p_title) {
					scnt = title_next - lba;
				} else {
					scnt = 16;
				}
			}

			map->get_next(&lba, BM_WAIT, &scnt);
			map->set(lba, BM_READ, scnt);
		} else {
			lba+=scnt;
			map->get_next(&lba, BM_FAIL, &scnt);	
			map->set(lba, BM_READ, scnt);
		}
		map->unlock();
    }
    
exit_reader:
	printf("%s: exit\n", dev->device);
    parm->running=0;
    parm->result=1;
    thread_exit(parm->result);
}

