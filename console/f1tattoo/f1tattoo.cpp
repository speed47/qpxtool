/*
 * This file is part of the QPxTool project.
 * Copyright (C) 2005-2006,2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */


#include <stdio.h>
#include <stdlib.h>

#include <qpx_mmc.h>
#include <yamaha_features.h>

#ifdef USE_LIBPNG
#include <png.h>
#endif

#include "version.h"

const	uint32_t	FL_HELP			= 0x00000001;
const	uint32_t	FL_SCAN			= 0x00000002;
const	uint32_t	FL_DEVICE		= 0x00000004;
const	uint32_t	FL_VERBOSE		= 0x00000008;
const	uint32_t	FL_CURRENT		= 0x00000010;
const	uint32_t	FL_SUPPORTED	= 0x00000020;
const	uint32_t	FL_TATTOO_TEST	= 0x00000040;
const	uint32_t	FL_TATTOO_RAW	= 0x00000080;
#ifdef USE_LIBPNG
const	uint32_t	FL_TATTOO_PNG	= 0x00000100;
const	uint32_t	FL_TATTOO		= FL_TATTOO_RAW | FL_TATTOO_PNG | FL_TATTOO_TEST;
#else
const	uint32_t	FL_TATTOO		= FL_TATTOO_RAW | FL_TATTOO_TEST;
#endif

uint32_t	flags = 0;

int get_device_info(drive_info* drive)
{
	drive->ven_features=0;
	drive->chk_features=0;
	detect_capabilities(drive);
//	detect_check_capabilities(drive);
	determine_disc_type(drive);
	if (!isYamaha(drive)) {
		printf ("%s: drive not supported\n", drive->device);
		return 1;
	}
//	if (!yamaha_check_amqr(drive)) drive->ven_features|=YMH_AMQR;
//	if (!yamaha_check_forcespeed(drive)) drive->ven_features|=YMH_FORCESPEED;
	if (!yamaha_f1_get_tattoo(drive)) drive->ven_features|=YMH_TATTOO;

	if (flags & FL_SUPPORTED) {
		printf("\n** Supported features:\n");
//		printf("AudioMaster Q.R.    : %s\n", drive->ven_features & YMH_AMQR ? "YES" : "---");
//		printf("ForceSpeed          : %s\n", drive->ven_features & YMH_FORCESPEED ? "YES" : "---");
		printf("DiscT@2             : %s\n", drive->ven_features & YMH_TATTOO ? "YES" : "---");
	}

	if (flags & FL_CURRENT) {
		printf("\n** Current drive settings:\n");
	}
	if ((flags & (FL_CURRENT | FL_TATTOO | FL_TATTOO_TEST)) && (drive->ven_features & YMH_TATTOO)) {
		if (drive->yamaha.tattoo_rows) {
			printf("DiscT@2 info:\ninner: %d\nouter: %d\nimage: 3744x%d\n",
				drive->yamaha.tattoo_i,
				drive->yamaha.tattoo_o,
				drive->yamaha.tattoo_rows);
		} else {
			if (drive->media.type & DISC_CD)
				printf("Can't write DiscT@2 on inserted disc!\n");
			else
				printf("No disc found! Can't get DiscT@2 info!\n");
		}
	}
	return 0;
}

#ifdef USE_LIBPNG
static int my_png_get_image_width(png_structp png_ptr, png_infop info_ptr) {
#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4 
	return png_get_image_width(png_ptr, info_ptr);
#else
	return info_ptr->width;
#endif
}

static int my_png_get_image_height(png_structp png_ptr, png_infop info_ptr) {
#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4 
	return png_get_image_height(png_ptr, info_ptr);
#else
	return info_ptr->height;
#endif
}

static png_byte my_png_get_color_type(png_structp png_ptr, png_infop info_ptr)
{
#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4
	return png_get_color_type(png_ptr, info_ptr);
#else
	return info_ptr->color_type;
#endif
}

static png_uint_32 my_png_get_valid(png_structp png_ptr, png_infop info_ptr, png_uint_32 flags)
{
#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4
	return png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE);
#else
	return (info_ptr->valid & flags);
#endif
}

static int my_png_get_bit_depth(png_structp png_ptr, png_infop info_ptr)
{
#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4
	return png_get_bit_depth(png_ptr, info_ptr);
#else
	return info_ptr->bit_depth;
#endif
}

static int my_png_get_rowbytes(png_structp png_ptr, png_infop info_ptr)
{
#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4
	return png_get_rowbytes(png_ptr, info_ptr);
#else
	return info_ptr->rowbytes;
#endif
}

bool tattoo_read_png(unsigned char *buf, uint32_t rows, FILE *fp)
{
	png_byte	header[8];	// 8 is the maximum size that can be checked
	png_structp	png_ptr;
	png_infop	info_ptr;
	uint32_t	number_of_passes;
	png_bytep	png_row_pointer = NULL;
	unsigned char *raw_row_pointer;
//	unsigned char *tp = NULL;

//	int width;
	uint32_t row, col;
	int      c;
	int32_t  r,g,b;
	int num_palette;
#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4
	png_colorp palette;
#endif

	if (fread(header, 1, 8, fp) < 8) {
		printf("Error reading PNG header\n");
		fclose(fp);	
		return 1;
	}
	if (png_sig_cmp(header, 0, 8)) {
		printf("File not recognized as a PNG\n");
		fclose(fp);
		return 1;
	}
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);	
	if (!png_ptr) {
		printf("png_create_read_struct failed!\n");
		fclose(fp);
		return 1;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		printf("png_create_info_struct failed!\n");
		fclose(fp);
		return 1;
	}
	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("png_jmpbuf failed!\n");
		fclose(fp);
		return 1;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	printf("Image size: %ld x %ld\n",
					my_png_get_image_width(png_ptr, info_ptr),
					my_png_get_image_height(png_ptr, info_ptr));

	if (my_png_get_image_width(png_ptr, info_ptr) != 3744U || my_png_get_image_height(png_ptr, info_ptr) != rows ) {
		printf("Image should be 3744 x %d", rows);
		return 1;
	}

//	width = info_ptr->width;
//	height = info_ptr->height;
//	bit_depth = info_ptr->bit_depth;

#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4 
	number_of_passes = png_set_interlace_handling(png_ptr);
#else
	number_of_passes = png_set_interlace_handling(png_ptr);
#endif
	png_read_update_info(png_ptr, info_ptr);

	printf("Color type: [%d] ", my_png_get_color_type(png_ptr, info_ptr));
	switch (my_png_get_color_type(png_ptr, info_ptr)) {
	    case PNG_COLOR_TYPE_GRAY:
			printf("PNG_COLOR_TYPE_GRAY\n");
			break;
	    case PNG_COLOR_TYPE_PALETTE:
			printf("PNG_COLOR_TYPE_PALETTE\n");
			if (!(my_png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE))) {
				printf("PNG color type is indexed, but no palette found!");
				goto err_read_png;
			}
			break;
	    case PNG_COLOR_TYPE_RGB:
			printf("PNG_COLOR_TYPE_RGB\n");
			break;
	    case PNG_COLOR_TYPE_RGB_ALPHA:
			printf("PNG_COLOR_TYPE_RGB_ALPHA\n");
			break;
	    case PNG_COLOR_TYPE_GRAY_ALPHA:
			printf("PNG_COLOR_TYPE_GRAY_ALPHA\n");
			break;
	    default:
			printf("unlnown PNG color type!\n");
			goto err_read_png;
	}
	printf("Bit depth : %d\n", my_png_get_bit_depth(png_ptr, info_ptr));
	if (my_png_get_bit_depth(png_ptr, info_ptr) != 8) {
		printf("Unsupported bit depth!\n");
		goto err_read_png;
	}

#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4
	png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
#else
	num_palette = info_ptr->num_palette;
#endif
	if (my_png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE)) {
		printf("Palette   : %d colors\n", num_palette);
	} else {
		printf("Palette   : NO\n");
	}
	printf("ROW bytes : %ld\n", my_png_get_rowbytes(png_ptr, info_ptr));


	raw_row_pointer = buf;
	png_row_pointer = (png_byte*) malloc(my_png_get_rowbytes(png_ptr, info_ptr));
	for (row=0; row<rows; row++) {
		if (setjmp(png_jmpbuf(png_ptr))) {
			printf("png_jmpbuf failed!\n");
			goto err_read_png;
		}
		png_read_row(png_ptr, png_row_pointer, NULL);
		if (my_png_get_image_width(png_ptr, info_ptr) < 3744U)
			memset(raw_row_pointer, 0, 3744);

		switch (my_png_get_color_type(png_ptr, info_ptr)) {
		    case PNG_COLOR_TYPE_GRAY:
				for (col=0; col<my_png_get_image_width(png_ptr, info_ptr); col++) {
					raw_row_pointer[col] = png_row_pointer[col] ^ 0xFF;
//					memcpy(raw_row_pointer, png_row_pointer, 3744);
				}
				break;
		    case PNG_COLOR_TYPE_PALETTE:
				for (col=0; col<my_png_get_image_width(png_ptr, info_ptr); col++) {
					c = png_row_pointer[col];
#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4
					r = palette[c].red;
					g = palette[c].green;
					b = palette[c].blue;
#else
					r = info_ptr->palette[c].red;
					g = info_ptr->palette[c].green;
					b = info_ptr->palette[c].blue;
#endif
					c = (r*11 + g*16 + b*5) / 32;
					raw_row_pointer[col] = c ^ 0xFF;
				}
#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4
				png_set_PLTE(png_ptr, info_ptr, palette, num_palette);
#endif
				break;
		    case PNG_COLOR_TYPE_RGB:
				for (col=0; col< my_png_get_image_width(png_ptr, info_ptr); col++) {
					r = png_row_pointer[col*3];
					g = png_row_pointer[col*3+1];
					b = png_row_pointer[col*3+2];
					c = (r*11 + g*16 + b*5) / 32;
					raw_row_pointer[col] = c ^ 0xFF;
				}
				break;
		    case PNG_COLOR_TYPE_RGB_ALPHA:
				for (col=0; col<my_png_get_image_width(png_ptr, info_ptr); col++) {
					r = png_row_pointer[col*4];
					g = png_row_pointer[col*4+1];
					b = png_row_pointer[col*4+2];
					c = (r*11 + g*16 + b*5) / 32;
					raw_row_pointer[col] = c ^ 0xFF;
				}
				break;
		    case PNG_COLOR_TYPE_GRAY_ALPHA:
				for (col=0; col<my_png_get_image_width(png_ptr, info_ptr); col++) {
					raw_row_pointer[col] = png_row_pointer[col*2] ^ 0xFF;
				}
				break;
		}
		raw_row_pointer += 3744;
	}

//	if (tp) free(tp);
	if (png_row_pointer) free(png_row_pointer);
	return 1;

err_read_png:
//	if (tp) free(tp);
	if (png_row_pointer) free(png_row_pointer);
	return 0;
}
#endif

void usage(char* bin) {
	fprintf (stderr,"\nusage: %s [-d device] [optinos]\n",bin);
#ifdef USE_LIBPNG
	printf("PNG support: YES\n");
#else
	printf("PNG support: NO\n");
#endif
	printf("\t-l, --scanbus                list drives (scan IDE/SCSI bus)\n");
	printf("\t-h, --help                   show help\n");
	printf("\t-c, --current                show current drive settings\n");
	printf("\t-s, --supported              show features supported by drive\n");
	printf("\t--tattoo-raw <tattoo_file>   burn selected RAW image as DiscT@2\n");
	printf("\t--tattoo-png <tattoo_file>   burn selected PNG image as DiscT@2\n");
#ifdef USE_LIBPNG
	printf("\t                             WARNING: f1tattoo compiled without libpng\n");
#endif
	printf("\t--tattoo-test                burn tattoo test image\n");
	printf("\t-v, --verbose                be verbose\n");
}

int main(int argc, char* argv[])
{
	int	i;
	int	drvcnt=0;
	char	*device   = NULL;
	char	*tattoofn = NULL;
	unsigned char	*tattoobuf = NULL;
	FILE	*tattoof;
	drive_info* drive;
	bool fr=0;

	printf("**  DiscT@2 writer for Yamaha CRW-F1  v%s (c) 2005-2006,2009  Gennady \"ShultZ\" Kozlov  **\n", VERSION);

	for (i=1; i<argc; i++) {
//		printf("arg[%02d]: %s\n",i,argv[i]);
		if(!strcmp(argv[i],"-d")) {
				if(argc>(i+1)) {
					i++;
					flags |= FL_DEVICE;
					device = argv[i];
				} else {
					printf("Option %s needs parameter\n",argv[i]);
					exit (1);
				}
		}
		else if (!strcmp(argv[i],"-h")) flags |= FL_HELP;
		else if(!strcmp(argv[i],"--help")) flags |= FL_HELP;
		else if(!strcmp(argv[i],"-c")) flags |= FL_CURRENT;
		else if(!strcmp(argv[i],"--current")) flags |= FL_CURRENT;
		else if(!strcmp(argv[i],"-l")) flags |= FL_SCAN;
		else if(!strcmp(argv[i],"--scanbus")) flags |= FL_SCAN;
		else if(!strcmp(argv[i],"-s")) flags |= FL_SUPPORTED;
		else if(!strcmp(argv[i],"--supported")) flags |= FL_SUPPORTED;
		else if(!strcmp(argv[i],"-v")) flags |= FL_VERBOSE;
		else if(!strcmp(argv[i],"--verbose")) flags |= FL_VERBOSE;
		else if(!strcmp(argv[i],"--tattoo-test")) flags |= FL_TATTOO_TEST;
		else if(!strcmp(argv[i],"--tattoo-raw")) {
			flags |= FL_TATTOO_RAW;
			if (argc>(i+1)) {
				i++;
				tattoofn = argv[i];
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
		} else if(!strcmp(argv[i],"--tattoo-png")) {
#ifdef USE_LIBPNG
			flags |= FL_TATTOO_PNG;
			if (argc>(i+1)) {
				i++;
				tattoofn = argv[i];
			} else {
				printf("option %s needs parameter!\n", argv[i]);
				return 5;
			}
#else
			printf("Can't use PNG as input file: compiled without libpng");
#endif
		} else {
			printf("Illegal option: %s.\nUse -h for details\n",argv[i]);
			return 6;
		}
	}

	if (flags & FL_HELP) {
		usage(argv[0]);
		return 0;
	}
	if (!flags) {
		usage(argv[0]);
		return 1;
	}
	if (flags & FL_SCAN) {
		drvcnt = scanbus(DEV_YAMAHA);
		if (!drvcnt) printf("ERR: no drives found!\n");
		return 2;
	}
	if (!(flags & FL_DEVICE)) {
		printf("** ERR: no device selected\n");
		return 3;
	}

//	printf("____________________________\n");
	printf("Device : %s\n", device);
	drive = new drive_info(device);
	if (drive->err) {
		printf("%s: device open error!\n", argv[0]);
		delete drive;
		return 4;
	}
	inquiry(drive);
//	convert_to_ID(drive);
	printf("Vendor : '%s'\n",drive->ven);
	printf("Model  : '%s'\n",drive->dev);
	printf("F/W    : '%s'\n",drive->fw);
	if (!(flags & FL_VERBOSE)) drive->silent++;
	if (get_drive_serial_number(drive)) printf("Serial#: %s\n",drive->serial);

	if (flags) {
//	if (flags & FL_VERBOSE) {
		printf("\nf1tattoo flags : ");
		if (flags & FL_DEVICE)		printf(" DEVICE");
		if (flags & FL_HELP)		printf(" HELP");
		if (flags & FL_CURRENT)		printf(" CURRENT");
		if (flags & FL_SCAN)		printf(" SCAN");
		if (flags & FL_VERBOSE)		printf(" VERBOSE");
		if (flags & FL_SUPPORTED)	printf(" SUPPORTED");
		if (flags & FL_TATTOO)		printf(" TATTOO");
		if (flags & FL_TATTOO_TEST)	printf(" TATTOO_TEST");
		printf("\n\n");
	}
	get_device_info(drive);
//	printf("____________________________\n");

	if (flags & FL_TATTOO) {
		if (!(drive->ven_features & YMH_TATTOO)) {
			printf("Selected device doesn't have DiscT@2 feature!\n");
			delete drive;
			return 1;
		}
		if (flags & FL_TATTOO_TEST) {
			printf("%s: writing T@2 test image...\n", device);
			yamaha_f1_do_tattoo(drive, NULL, 0);
		} else {
			tattoof = fopen(tattoofn, "r");
			if (!tattoof) {
				printf("Can't open tattoo file: %s", tattoofn);
			} else {
				printf("Reading tattoo file...\n");
				tattoobuf = (unsigned char*) malloc (drive->yamaha.tattoo_rows * 3744);
#ifdef USE_LIBPNG
				if (flags & FL_TATTOO_PNG) {
					fr = tattoo_read_png(tattoobuf, drive->yamaha.tattoo_rows, tattoof);
				} else {
#endif
					memset(tattoobuf, 0, drive->yamaha.tattoo_rows * 3744);
					fr = (fread((void*)tattoobuf, 3744, drive->yamaha.tattoo_rows, tattoof) > 0);
#ifdef USE_LIBPNG
				}
#endif
				fclose(tattoof);
				if (fr) {
					yamaha_f1_do_tattoo(drive, tattoobuf, fr * 3744);
				} else {
					printf("Error reading T@2 image!\n");
				}
				free(tattoobuf);
			}
		}
	}
	if (!(flags & FL_VERBOSE)) drive->silent--;
	delete drive;
	return 0;
}

