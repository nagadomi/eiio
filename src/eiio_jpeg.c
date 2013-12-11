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

#if !EIIO_JPEG
eiio_image_t *eiio_read_jpeg(const void *blob, size_t blob_size)
{
	eiio_warnings("eiio_read_jpg: not implemented\n");
	return NULL;
}
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>
#include <jpeglib.h>

#ifdef _MSC_VER
#  pragma comment(lib, "libjpeg.lib")
#endif

typedef struct {
	struct jpeg_source_mgr pub;
	JOCTET *buffer;
} memory_src_mgr;

METHODDEF(void)
init_memory_source (j_decompress_ptr cinfo)
{
}

METHODDEF(boolean)
fill_memory_buffer(j_decompress_ptr cinfo)
{
  return TRUE;
}

METHODDEF(void)
skip_memory_data (j_decompress_ptr cinfo, long num_bytes)
{
  memory_src_mgr *src = (memory_src_mgr *) cinfo->src;

  src->pub.next_input_byte += (size_t) num_bytes;
  src->pub.bytes_in_buffer -= (size_t) num_bytes;
}

METHODDEF(void)
term_memory_source (j_decompress_ptr cinfo)
{
}

static void
jpeg_memory_src(j_decompress_ptr cinfo, const void *blob, size_t blob_size)
{
	memory_src_mgr *src;

	if (cinfo->src == NULL) {
		src = (memory_src_mgr *)(*cinfo->mem->alloc_small)
			((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(memory_src_mgr));
		cinfo->src = (struct jpeg_source_mgr *)src;
	}
	src = (memory_src_mgr *)cinfo->src;
	src->pub.init_source = init_memory_source;
	src->pub.fill_input_buffer = fill_memory_buffer;
	src->pub.skip_input_data = skip_memory_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart;
	src->pub.term_source = term_memory_source;
	src->pub.bytes_in_buffer = blob_size;
	src->pub.next_input_byte = blob;
}

static void 
jpeg_error(j_common_ptr cinfo)
{
	cinfo->err->output_message(cinfo);
	longjmp(*(jmp_buf*)cinfo->client_data, 1);
}

eiio_image_t *
eiio_read_jpeg(const void *blob, size_t blob_size)
{
	eiio_image_t *image = NULL;
	jmp_buf context;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY rows = NULL;
	JSAMPLE *image_data = NULL;
	J_COLOR_SPACE color_space;
	int image_width, image_height, y;
	unsigned int i;

	memset(&cinfo, 0, sizeof(cinfo));
	memset(&jerr, 0, sizeof(jerr));

	// decompress
	cinfo.client_data = (void*)&context;
	cinfo.err = jpeg_std_error(&jerr);
	cinfo.err->error_exit = jpeg_error;
	jpeg_create_decompress(&cinfo);

	if (setjmp(*(jmp_buf*)cinfo.client_data)) {
		// error
		jpeg_destroy_decompress(&cinfo);
		if (rows != NULL) {
			free(rows);
		}
		if (image_data != NULL) {
			free(image_data);
		}

		return NULL;
	}

	jpeg_memory_src(&cinfo, blob, blob_size);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	color_space = cinfo.out_color_space;
	//image_width = cinfo.image_width;
	//image_height = cinfo.image_height;
	image_width = cinfo.output_width;
	image_height = cinfo.output_height;

	rows = (JSAMPARRAY)eiio_malloc(
		sizeof(JSAMPROW) * cinfo.output_height
	);
	image_data = (JSAMPLE *)eiio_malloc(
		sizeof(JSAMPLE) 
			* cinfo.output_components 
			* cinfo.output_width
			* cinfo.output_height
	);
	
	for (i = 0; i < cinfo.output_height; ++i) {
		rows[i] = (JSAMPROW)&image_data[i * cinfo.output_components * cinfo.output_width];
	}
	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(
			&cinfo,
			&rows[cinfo.output_scanline],
			cinfo.output_height - cinfo.output_scanline
		);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	switch (color_space) {
	case JCS_RGB:
		image = eiio_image_alloc(image_width, image_height, EIIO_COLOR_RGB, eiio_malloc, free);
		if (image) {
			for (y = 0; y < image_height; ++y) {
				int x;
				JSAMPLE *row = rows[y];
				for (x = 0; x < image_width; ++x) {
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_R, *row++);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_G, *row++);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_B, *row++);
				}
			}
		}
		break;
	case JCS_GRAYSCALE:
		image = eiio_image_alloc(image_width, image_height, EIIO_COLOR_GRAY, eiio_malloc, free);
		if (image) {
			for (y = 0; y < image_height; ++y) {
				int x;
				JSAMPLE *row = rows[y];
				for (x = 0; x < image_width; ++x) {
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_GRAY, *row++);
				}
			}
		}
		break;
	case JCS_CMYK:
		image = eiio_image_alloc(image_width, image_height, EIIO_COLOR_CMYK, eiio_malloc, free);
		if (image) {
			for (y = 0; y < image_height; ++y) {
				int x;
				JSAMPLE *row = rows[y];
				for (x = 0; x < image_width; ++x) {
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_C, *row++);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_M, *row++);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_Y, *row++);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_K, *row++);
				}
			}
		}
		break;
	default:
		eiio_warnings("eiio_read_jpeg: unsupported color(%d)\n", color_space);
		break;
	}
	free(rows);
	free(image_data);

	if (image) {
		if (image->color != EIIO_COLOR_RGB) {
			eiio_image_t *rgb = eiio_image_alloc(
				image->width, image->height, EIIO_COLOR_RGB, eiio_malloc, free);
		
			eiio_rgb(rgb, image);

			eiio_image_free(&image);
			image = rgb;
		}
	}

	return image;
}


#endif
