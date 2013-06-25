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
#include <string.h>
#include <assert.h>

static void 
eiio_cmyk2rgb(eiio_image_t *rgb, const eiio_image_t *cmyk)
{
	int y;

	assert(rgb->width == cmyk->width);
	assert(rgb->height == cmyk->height);

	for (y = 0; y < cmyk->height; ++y) {
		int x;
		for (x = 0; x < cmyk->width; ++x) {
			int pc = eiio_get_pixel(cmyk, x, y, EIIO_CHANNEL_C);
			int pm = eiio_get_pixel(cmyk, x, y, EIIO_CHANNEL_M);
			int py = eiio_get_pixel(cmyk, x, y, EIIO_CHANNEL_Y);
			int pk = eiio_get_pixel(cmyk, x, y, EIIO_CHANNEL_K);

			eiio_set_pixel(rgb, x, y, EIIO_CHANNEL_R, pk - ((255 - pc) * pk >> 8));
			eiio_set_pixel(rgb, x, y, EIIO_CHANNEL_G, pk - ((255 - pm) * pk >> 8));
			eiio_set_pixel(rgb, x, y, EIIO_CHANNEL_B, pk - ((255 - py) * pk >> 8));
		}
	}
}

#define EIIO_RGB2GRAY(r, g, b) \
	((6969 * (r) + 23434 * (g) + 2365 * (b)) / 32768)

static void 
eiio_gray2rgb(eiio_image_t *rgb, const eiio_image_t *gray)
{
	int x, y;

	assert(rgb->width == gray->width);
	assert(rgb->height == gray->height);

	for (y = 0; y < gray->height; ++y) {
		for (x = 0; x < gray->width; ++x) {
			int g = eiio_get_pixel(gray, x, y, EIIO_CHANNEL_GRAY);
			eiio_set_pixel(rgb, x, y, EIIO_CHANNEL_R, g);
			eiio_set_pixel(rgb, x, y, EIIO_CHANNEL_G, g);
			eiio_set_pixel(rgb, x, y, EIIO_CHANNEL_B, g);
		}
	}
}

void 
eiio_rgb(eiio_image_t *rgb, const eiio_image_t *src)
{
	switch (src->color) {
		case EIIO_COLOR_RGB:
			memmove(rgb->data, src->data,
					sizeof(int) * src->width * src->height * src->channels);
			break;
		case EIIO_COLOR_CMYK:
			eiio_cmyk2rgb(rgb, src);
			break;
		case EIIO_COLOR_GRAY:
			eiio_gray2rgb(rgb, src);
			break;
		default:
			eiio_warnings("eiio_rgb: unsupported color(%d)\n", (int)src->color);
			break;
	}
}
