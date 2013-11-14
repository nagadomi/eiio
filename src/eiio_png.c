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

#if !EIIO_PNG
eiio_image_t *
eiio_read_png(const void *blob, size_t blob_size)
{
	eiio_warnings("eiio_read_png: not implemented");
	return NULL;
}
#else
#include <png.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>

#ifdef _MSC_VER
#  pragma comment(lib, "libpng.lib")
#  pragma comment(lib, "zlib.lib")
#endif

typedef struct {
	const unsigned char *blob;
	size_t offset;
	size_t blob_size;
} blob_input_t;

static void 
read_blob(png_structp png, png_bytep data, png_size_t length)
{
	blob_input_t *blob_input = (blob_input_t *)png_get_io_ptr(png);
	size_t nb = 0;

	if (length < blob_input->blob_size) {
		nb = length;
	} else {
		nb = blob_input->blob_size;
	}

	memmove(data, blob_input->blob + blob_input->offset, nb);
	blob_input->blob_size -= nb;
	blob_input->offset += nb;
}

static void 
eiio_png_rows_free(png_bytepp *prows)
{
	int i;
	png_bytepp rows;

	if (prows == NULL) {
		return;
	}
	rows = *prows;
	*prows = NULL;

	if (rows != NULL) {
		for (i = 0; rows[i] != NULL; ++i) {
			free(rows[i]);
			rows[i] = NULL;
		}
		free(rows);
	}
}

static png_bytepp 
eiio_png_read_rgb(const void *blob, size_t blob_size, int *width, int *height, int *channels)
{
	png_structp png = NULL;
	png_infop info = NULL;
	blob_input_t blob_input;
	int bit_depth, color_type;
	png_bytepp rows = NULL;
	png_uint_32 uwidth, uheight;
	int i;

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png == NULL){
		return NULL;
	}

	info = png_create_info_struct(png);
	if(info == NULL){
		png_destroy_read_struct(&png, NULL, NULL);
		return NULL;
	}

	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &info, NULL);
		eiio_png_rows_free(&rows);

		return NULL;
	}

	blob_input.blob = blob;
	blob_input.offset = 0;
	blob_input.blob_size = blob_size;
	png_set_read_fn(png, &blob_input, read_blob);

	*width = *height = 0;
	png_read_info(png, info);
	png_get_IHDR(png, info, &uwidth, &uheight, &bit_depth, &color_type, NULL, NULL, NULL);

	if (uwidth > 0x7fffffff || uheight > 0x7fffffff) {
		png_destroy_read_struct(&png, &info, NULL);
		eiio_png_rows_free(&rows);

		return NULL;
	}
	*width = (int)uwidth;
	*height = (int)uheight;	

	if (bit_depth == 16) {
		png_set_strip_16(png);
	}
	if ((color_type == PNG_COLOR_TYPE_GRAY 
		|| color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		&& bit_depth < 8) 
	{
		png_set_expand_gray_1_2_4_to_8(png);
		//png_set_gray_1_2_4_to_8(png);
	}

	if (bit_depth < 8) {
		png_set_packing(png);
	}

	if (color_type & PNG_COLOR_MASK_ALPHA) {
		*channels = 4;
	} else {
		*channels = 3;
	}

	if (color_type & PNG_COLOR_MASK_PALETTE) {
		png_set_palette_to_rgb(png);
	}

	if (color_type == PNG_COLOR_TYPE_GRAY
		|| color_type == PNG_COLOR_TYPE_GRAY_ALPHA) 
	{
		png_set_gray_to_rgb(png);
	}
	png_read_update_info(png, info);

	rows = (png_bytepp)eiio_malloc(sizeof(png_bytep) * (*height + 1));
	if (rows == NULL) {
		png_destroy_read_struct(&png, &info, NULL);
		return NULL;
	}
	for (i = 0; i < *height; ++i) {
		rows[i] = (png_bytep)eiio_malloc(sizeof(png_byte) * *width * *channels);
		if (rows[i] == NULL) {
			png_destroy_read_struct(&png, &info, NULL);
			eiio_png_rows_free(&rows);
			return NULL;
		}
	}
	rows[*height] = NULL; /* null terminater */

	png_read_image(png, rows);
	png_read_end(png, info);

	png_destroy_read_struct(&png, &info, NULL);

	return rows;
}

eiio_image_t *
eiio_read_png(const void *blob, size_t blob_size)
{
	eiio_image_t *image = NULL;
	int width, height, channels;
	int y;
	png_bytepp rows;
	
	rows = eiio_png_read_rgb(blob, blob_size, &width, &height, &channels);
	if (rows == NULL) {
		return NULL;
	}
	image = eiio_image_alloc(width, height, EIIO_COLOR_RGB, eiio_malloc, free);

	if (channels == 3) {
		for (y = 0; y < height; ++y) {
			int x;
			for (x = 0; x < width; ++x) {
				eiio_set_pixel(image, x, y, EIIO_CHANNEL_R, rows[y][x * 3 + 0]);
				eiio_set_pixel(image, x, y, EIIO_CHANNEL_G, rows[y][x * 3 + 1]);
				eiio_set_pixel(image, x, y, EIIO_CHANNEL_B, rows[y][x * 3 + 2]);
			}
		}
	} else { // 4 color + alpha png
		int bg[3];
		eiio_uint8_t bgu[3];
		int alpha;
		int bg_w;
		eiio_get_background_color(&bgu[0], &bgu[1], &bgu[2]);
		bg[0] = (int)bgu[0];
		bg[1] = (int)bgu[1];
		bg[2] = (int)bgu[2];
		
		for (y = 0; y < height; ++y) {
			int x;
			for (x = 0; x < width; ++x) {
				alpha = rows[y][x * 4 + 3];
				bg_w = 0xff - alpha;
				if (alpha != 0xff) {
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_R, (rows[y][x * 4 + 0] * alpha + bg[0] * bg_w) / 0xff);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_G, (rows[y][x * 4 + 1] * alpha + bg[1] * bg_w) / 0xff);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_B, (rows[y][x * 4 + 2] * alpha + bg[2] * bg_w) / 0xff);
				} else {
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_R, rows[y][x * 4 + 0]);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_G, rows[y][x * 4 + 1]);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_B, rows[y][x * 4 + 2]);
				}
			}
		}
	}
	eiio_png_rows_free(&rows);

	return image;
}

#endif
