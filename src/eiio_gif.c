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

#if !EIIO_GIF
eiio_image_t *
eiio_read_gif(const void *blob, size_t blob_size)
{
	eiio_warnings("eiio_read_gif: not implemented\n");
	return NULL;
}
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eiio.h"
#include "eiio_internal.h"
#include "gif_lib.h"

#ifdef _MSC_VER
#  pragma comment(lib, "giflib.lib")
#endif

typedef struct {
	const unsigned char *blob;
	size_t offset;
	size_t blob_size;
} gif_blob_input_t;

static int 
read_blob(GifFileType *gif, GifByteType *data, int length) 
{
	int nb;

	gif_blob_input_t *blob_input = (gif_blob_input_t *)gif->UserData;
	if (blob_input->blob_size >= (size_t)length) {
		nb = length;
	} else {
		//nb = blob_input->blob_size;
		nb = 0;
	}

	if (nb != 0) {
		memmove(data, blob_input->blob + blob_input->offset, nb);
		blob_input->blob_size -= nb;
		blob_input->offset += nb;
	}

	return nb;
}

static void
eiio_gif_rows_free(GifRowType **prows)
{
	GifRowType *rows;
	int i;

	if (prows == NULL) {
		return;
	}
	rows = *prows;
	*prows = NULL;

	for (i = 0; rows[i] != NULL; ++i) {
		free(rows[i]);
		rows[i] = NULL;
	}
	free(rows);
}

static void 
eiio_gif_free(GifFileType **pgif, GifRowType **prows)
{
	eiio_gif_rows_free(prows);
	if (pgif && *pgif) {
		(*pgif)->SColorMap = NULL;
		DGifCloseFile(*pgif);
		*pgif = NULL;
	}
}

static GifRowType *
eiio_gif_read_rows(GifFileType *gif, int *transparent)
{
	static const int s_interlaced_offset[] = { 0, 4, 2, 1 };
	static const int s_interlaced_jumps[] = { 8, 8, 4, 2 };
	GifRowType *rows = NULL;
	GifRecordType record_type;
	int x, y, i, j;
	int top, left, width, height, ext_code;
	GifByteType *extension;
	size_t width_size;
	int frames = 0;

	*transparent = -1;

	rows = (GifRowType *)eiio_malloc(sizeof(GifRowType *) * (gif->SHeight + 1));
	if (rows == NULL) {
		return NULL;
	}
	width_size = gif->SWidth * sizeof(GifPixelType);

	for (y = 0; y < gif->SHeight; ++y) {
		rows[y] = (GifRowType)eiio_malloc(width_size);
		if (rows[y] == NULL) {
			eiio_gif_rows_free(&rows);
			return NULL;
		}
		if (y == 0) {
			for (x = 0; x < gif->SWidth; ++x) {
				rows[y][x] =  gif->SBackGroundColor;
			}
		} else {
			memmove(rows[y], rows[0], width_size);
		}
	}
	rows[gif->SHeight] = NULL; /* null terminater */

	do {
		if (DGifGetRecordType(gif, &record_type) == GIF_ERROR) {
			eiio_warnings("eiio_read_gif: malformed image\n");
			eiio_gif_rows_free(&rows);
			return NULL;
		}
		switch (record_type) {
		case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(gif) == GIF_ERROR) {
				eiio_warnings("eiio_read_gif: malformed image\n");
				eiio_gif_rows_free(&rows);
				return NULL;
			}
			top = gif->Image.Top;
			left = gif->Image.Left;
			width = gif->Image.Width;
			height = gif->Image.Height;

			if (gif->Image.Left + gif->Image.Width > gif->SWidth
				|| gif->Image.Top + gif->Image.Height > gif->SHeight) 
			{
				eiio_warnings("eiio_read_gif: malformed image\n");
				eiio_gif_rows_free(&rows);
				return NULL;
			}
			if (gif->Image.Interlace) {
				for (i = 0; i < 4; i++)
					for (j = top + s_interlaced_offset[i]; 
						j < top + height;
						j += s_interlaced_jumps[i]) 
					{
						if (DGifGetLine(gif, &rows[j][left], width) == GIF_ERROR) {
							eiio_warnings("eiio_read_gif: malformed image\n");
							eiio_gif_rows_free(&rows);
							return NULL;
						}
					}
			}
			else {
				for (i = 0; i < height; i++) {
					if (DGifGetLine(gif, &rows[top++][left], width) == GIF_ERROR) {
						eiio_warnings("eiio_read_gif: malformed image\n");
						eiio_gif_rows_free(&rows);
						return NULL;
					}
				}
			}
			frames = 1; // first frame
			break;
		case EXTENSION_RECORD_TYPE:
			extension = NULL;
			ext_code = 0;
			if (DGifGetExtension(gif, &ext_code, &extension) == GIF_ERROR) {
				eiio_warnings("eiio_read_gif: malformed image\n");
				eiio_gif_rows_free(&rows);
				return NULL;
			} else if (ext_code == 0xf9 
				&& extension[0] >= 4 
				&& (extension[1] & 0x1))
			{
				*transparent = extension[4];
			}

			while (extension != NULL) {
				if (DGifGetExtensionNext(gif, &extension) == GIF_ERROR) {
					eiio_warnings("eiio_read_gif: malformed image\n");
					eiio_gif_rows_free(&rows);
					return NULL;
				}
			}
			break;
		case TERMINATE_RECORD_TYPE:
			break;
		default:
			break;
		}
	} while (record_type != TERMINATE_RECORD_TYPE && frames == 0);

	if (frames <= 0) {
		eiio_gif_rows_free(&rows);
		return NULL;
	}

	return rows;
}

eiio_image_t *
eiio_read_gif(const void *blob, size_t blob_size)
{
	eiio_image_t *image;
	GifFileType *gif;
	GifRowType *rows;
	gif_blob_input_t blob_input;
	ColorMapObject *color_map;
	int x, y;
	eiio_uint8_t bg[3];
	int transparent = -1;
	
	blob_input.blob = (unsigned char *)blob;
	blob_input.blob_size = blob_size;
	blob_input.offset = 0;

	gif = DGifOpen(&blob_input, read_blob);
	if (gif == NULL) {
		eiio_warnings("eiio_read_gif: initialization failed\n");
		return NULL;
	}

	rows = eiio_gif_read_rows(gif, &transparent);

	if (rows == NULL) {
		/* failed free(ColorMap). FIXME */
		//DGifCloseFile(gif);
		return NULL;
	}
	color_map = gif->Image.ColorMap 
		? gif->Image.ColorMap: gif->SColorMap;

    if (color_map == NULL) {
		eiio_warnings("eiio_read_gif: Gif Image does not have a colormap\n");
		eiio_gif_free(&gif, &rows);
		return NULL;
	}

	image = eiio_image_alloc(gif->SWidth, gif->SHeight, EIIO_COLOR_RGB, eiio_malloc, free);
	if (image == NULL) {
		eiio_gif_free(&gif, &rows);
		return NULL;
	}

	eiio_get_background_color(&bg[0], &bg[1], &bg[2]);
	if (transparent >= 0) {
		for (y = 0; y < gif->SHeight; ++y) {
			for (x = 0; x < gif->SWidth; ++x) {
				int color_index = rows[y][x];
				if (color_index == transparent)	{
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_R, bg[0]);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_G, bg[1]);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_B, bg[2]);
				} else {
					GifColorType *entry = &color_map->Colors[color_index];
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_R, entry->Red);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_G, entry->Green);
					eiio_set_pixel(image, x, y, EIIO_CHANNEL_B, entry->Blue);
				}
			}
		}
	} else {
		for (y = 0; y < gif->SHeight; ++y) {
			for (x = 0; x < gif->SWidth; ++x) {
				GifColorType *entry = &color_map->Colors[rows[y][x]];
				eiio_set_pixel(image, x, y, EIIO_CHANNEL_R, entry->Red);
				eiio_set_pixel(image, x, y, EIIO_CHANNEL_G, entry->Green);
				eiio_set_pixel(image, x, y, EIIO_CHANNEL_B, entry->Blue);
			}
		}
	}

	eiio_gif_free(&gif, &rows);

	return image;
}

#endif
