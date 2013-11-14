/*
Copyright (c) 2010 nagadomi <nagadomi@nurs.or.jp>

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
#include "eiio.h"
#include "eiio_internal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static eiio_outofmemory_t eiio_outofmemory_func = NULL;

eiio_outofmemory_t
eiio_set_outofmemory_func(eiio_outofmemory_t func)
{
	eiio_outofmemory_t old_func = eiio_outofmemory_func;
	eiio_outofmemory_func = func;
	
	return old_func;
}

eiio_outofmemory_t
eiio_get_outofmemory_func(void)
{
	return eiio_outofmemory_func;
}

static void *
eiio_outofmemory(void)
{
	if (eiio_outofmemory_func) {
		(*eiio_outofmemory_func)();
	} else {
		fprintf(stderr, "eiio_alloc: out of memory\n");
		abort();
	}
	
	return NULL;
}

void *
eiio_malloc(size_t s)
{
	void *mem = malloc(s);
	
	if (mem == NULL) {
		return eiio_outofmemory();
	}

	return mem;
}

void 
eiio_warnings(const char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}

static int
eiio_color_channels(eiio_color_e color)
{
	int channels = 0;

	switch (color) {
    case EIIO_COLOR_GRAY:
		channels = 1;
		break;
    case EIIO_COLOR_RGB:
		channels = 3;
		break;
    case EIIO_COLOR_CMYK:
		channels = 4;
		break;
	default:
		eiio_warnings("eiio_color_channels: unsupported color(%d)\n", (int)color);
		channels = -1;
		break;
	}

	return channels;
}

static eiio_format_e
eiio_format(const char *data)
{
	if (memcmp(data, "\xff\xd8", 2) == 0) {
		return EIIO_FORMAT_JPEG;
	}
	if (memcmp(data, "\x89PNG", 4) == 0) {
		return EIIO_FORMAT_PNG;
	}
	if (memcmp(data, "GIF", 3) == 0) {
		return EIIO_FORMAT_GIF;
	}
	if (memcmp(data, "BM", 2) == 0) {
		return EIIO_FORMAT_BMP;
	}

	return EIIO_FORMAT_UNKNOWN;
}

eiio_image_t *
eiio_read_file(const char *filename)
{
	eiio_image_t *image = NULL;
	FILE *fp;
	size_t file_size, len, offset;
	unsigned char *blob;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	if (file_size <= 0) {
		fclose(fp);
		return NULL;
	}

	blob = (unsigned char *)eiio_malloc(file_size);
	if (blob == NULL) {
		fclose(fp);
		return NULL;
	}

	rewind(fp); 
	offset = 0;
	while ((len = fread(blob + offset,
		sizeof(unsigned char),
		file_size - offset, fp)) > 0) 
	{
		offset += len;
		if (feof(fp) || offset >= file_size) {
			break;
		}
	}
	fclose(fp);

	if (offset == file_size) {
		image = eiio_read_blob(blob, file_size);
	}

	free(blob);

	return image;
}

eiio_image_t *
eiio_read_blob(const void *blob, size_t blob_size)
{
	eiio_image_t *image = NULL;
	eiio_format_e format;

	if (blob_size <= 3) {
		eiio_warnings("eiio_read_blob: blob is too small\n");
		return NULL;
	}

	format = eiio_format(blob);
	switch (format) {
    case EIIO_FORMAT_JPEG:
		image = eiio_read_jpeg(blob, blob_size);
		break;
    case EIIO_FORMAT_PNG:
		image = eiio_read_png(blob, blob_size);
		break;
	case EIIO_FORMAT_GIF:
		image = eiio_read_gif(blob, blob_size);
		break;
	case EIIO_FORMAT_BMP:
		image = eiio_read_bmp(blob, blob_size);
		break;
	default:
		break;
	}

	return image;
}

eiio_uint8_t
eiio_write_file(const char *filename,
				const eiio_image_t *image,
				eiio_format_e format)
{
	/* TODO */
	eiio_warnings("eiio_write_file: not implemented\n");
	return -1;
}

eiio_uint8_t
eiio_write_blob(void *blob, size_t *blob_size,
				const eiio_image_t *image,
				eiio_format_e format)
{
	/* TODO */
	eiio_warnings("eiio_write_blob: not implemented\n");
	return -1;
}

void 
eiio_set_pixel_strict(eiio_image_t *image,
					  int x, int y, int channel,
					  eiio_uint8_t pixel)
{
	assert(x >= 0);
	assert(x < image->width);
	assert(y >= 0);
	assert(y < image->height);

	image->data[
		(y * image->width * image->channels) 
			+ (x * image->channels) + channel
	] = pixel;
}

eiio_uint8_t
eiio_get_pixel_strict(const eiio_image_t *image, int x, int y, int channel)
{
	assert(x >= 0);
	assert(x < image->width);
	assert(y >= 0);
	assert(y < image->height);

	return image->data[
		(y * image->width * image->channels) 
			+ (x * image->channels) + channel
	];
}

eiio_image_t *
eiio_image_alloc(int width, int height,
				 eiio_color_e color,
				 eiio_malloc_t malloc_func,
				 eiio_free_t free_func)
{
	int channels;
	eiio_image_t *image;

	channels = eiio_color_channels(color);
	if (channels < 0) {
		return NULL;
	}
	image = (eiio_image_t *)(*malloc_func)(sizeof(eiio_image_t));
	if (image == NULL) {
		return NULL;
	}
	image->free_func = free_func;
	image->width = width;
	image->height = height;
	image->channels = channels;
	image->color = color;
	image->data = (eiio_uint8_t *)(*malloc_func)(
		sizeof(eiio_uint8_t) * width * height * channels);

	if (image->data == NULL) {
		(*free_func)(image);
		return NULL;
	}

	memset(image->data, 0, width * height * channels);

	return image;
}

void 
eiio_image_free(eiio_image_t **image)
{
	if (*image != NULL) {
		eiio_free_t free_func = (*image)->free_func;

		(*free_func)((*image)->data);
		(*free_func)(*image);
		*image = NULL;
	}
}


static eiio_uint8_t g_background_color[3] = { 128, 128, 128 };

void 
eiio_set_background_color(eiio_uint8_t r, eiio_uint8_t g, eiio_uint8_t b)
{
	g_background_color[0] = r;
	g_background_color[1] = g;
	g_background_color[2] = b;
}

void 
eiio_get_background_color(eiio_uint8_t *r, eiio_uint8_t *g, eiio_uint8_t *b)
{
	*r = g_background_color[0];
	*g = g_background_color[1];
	*b = g_background_color[2];
}

