/*
Copyright (c) 2010+ nagadomi <nagadomi@nurs.or.jp>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef EIIO_H
#define EIIO_H
#ifdef _MSC_VER
#  ifdef _DEBUG
#    define _CRTDBG_MAP_ALLOC
#    include <crtdbg.h>
#  endif
#endif
#include "eiio_config.h"
#include <stdlib.h>
#if EIIO_WINDOWS
#  include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*eiio_outofmemory_t)(void);

eiio_outofmemory_t eiio_set_outofmemory_func(eiio_outofmemory_t func);
eiio_outofmemory_t eiio_get_outofmemory_func(void);
	
void *eiio_malloc(size_t s);
typedef unsigned char eiio_uint8_t;

typedef enum {
	EIIO_FORMAT_UNKNOWN,
	EIIO_FORMAT_JPEG,
	EIIO_FORMAT_PNG,
	EIIO_FORMAT_GIF,
	EIIO_FORMAT_BMP
} eiio_format_e;

typedef enum {
	EIIO_COLOR_GRAY,
	EIIO_COLOR_RGB,
	EIIO_COLOR_CMYK
} eiio_color_e;

#define	EIIO_CHANNEL_R    0
#define	EIIO_CHANNEL_G    1
#define	EIIO_CHANNEL_B    2
#define	EIIO_CHANNEL_C    0
#define	EIIO_CHANNEL_M    1
#define	EIIO_CHANNEL_Y    2
#define	EIIO_CHANNEL_K    3
#define	EIIO_CHANNEL_GRAY 0

typedef void *(*eiio_malloc_t)(size_t mem_size);
typedef void (*eiio_free_t)(void *mem);

typedef struct {
	int width;
	int height;
	int channels;
	eiio_color_e color;
	eiio_free_t free_func;
	eiio_uint8_t *data;
} eiio_image_t;


eiio_image_t *eiio_read_file(const char *filename);
eiio_image_t *eiio_read_blob(const void *blob, size_t blob_size);

void eiio_set_background_color(eiio_uint8_t r, eiio_uint8_t g, eiio_uint8_t b);
void eiio_get_background_color(eiio_uint8_t *r, eiio_uint8_t *g, eiio_uint8_t *b);

void eiio_set_pixel_strict(eiio_image_t *image,
						   int x, int y, int channel,
						   eiio_uint8_t pixel);


eiio_uint8_t eiio_get_pixel_strict(const eiio_image_t *image,
							  int x, int y, int channel);


eiio_uint8_t eiio_write_file(const char *filename,
							 const eiio_image_t *image,
							 eiio_format_e format);
	
eiio_uint8_t eiio_write_blob(void *blob, size_t *blob_size,
							 const eiio_image_t *image,
							 eiio_format_e format);

void eiio_image_free(eiio_image_t **image);
eiio_image_t *eiio_image_alloc(int width, int height,
							   eiio_color_e color,
							   eiio_malloc_t malloc_func,
							   eiio_free_t free_func);

#if EIIO_STRICT
#  define eiio_set_pixel(image, x, y, channel, pixel) \
	eiio_set_pixel_strict(image, x, y, channel, pixel)

#  define eiio_get_pixel(image, x, y, channel) \
	eiio_get_pixel_strict(image, x, y, channel)
#else
#  define eiio_set_pixel(image, x, y, channel, pixel) \
	((image)->data[(y) * ((image)->width * (image)->channels)	\
	+ (x) * (image)->channels + (channel)] = (pixel))

#  define eiio_get_pixel(image, x, y, channel) \
	((image)->data[(y) * ((image)->width * (image)->channels)	\
	+ (x) * (image)->channels + (channel)])
#endif


/* Video API*/
#if EIIO_FFMPEG

typedef struct eiio_video eiio_video_t;

eiio_video_t *eiio_video_open(const char *file);
eiio_image_t *eiio_video_next(eiio_video_t *video);
int64_t eiio_video_frames(eiio_video_t *video);
int eiio_video_seek(eiio_video_t *video, int64_t frame);
void eiio_video_close(eiio_video_t **video);
int eiio_video_set_size(eiio_video_t *video, int height, int width);

#endif

/* Windows BITMAP */	
#if EIIO_WINDOWS

eiio_image_t *eiio_read_bitmap_handle(HBITMAP hDIB);

#endif

#ifdef __cplusplus
}
#endif

#endif
