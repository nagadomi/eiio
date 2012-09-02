/*
Copyright (c) 2011 nagadomi <nagadomi@nurs.or.jp>

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

#if EIIO_WINDOWS
#include <stdio.h>
eiio_image_t *eiio_read_bitmap_handle(HBITMAP hDIB)
{
	BITMAP bmp;
	HDC hDC;
	BITMAPINFO info;
	RGBQUAD *data;
	int y, x, height;
	eiio_image_t *image = NULL;
	
	memset(&info, 0, sizeof(info));
	
	GetObject(hDIB, sizeof(BITMAP), &bmp);
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = bmp.bmWidth;
	info.bmiHeader.biHeight = bmp.bmHeight;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biBitCount = 32;
	
	hDC = GetDC(NULL);
	GetDIBits(hDC, hDIB, 0, bmp.bmHeight, NULL, &info, DIB_RGB_COLORS);
	data = (RGBQUAD*)eiio_malloc(info.bmiHeader.biSizeImage);
	GetDIBits(hDC, hDIB, 0, bmp.bmHeight, (LPVOID)data, &info, DIB_RGB_COLORS);
	
	image = eiio_image_alloc(bmp.bmWidth, bmp.bmHeight,
							 EIIO_COLOR_RGB, eiio_malloc, free);
	height = bmp.bmHeight;
	for (y = 0; y < bmp.bmHeight; ++y) {
		for (x = 0; x < bmp.bmWidth; ++x) {
			const RGBQUAD *p = &data[(y * bmp.bmWidth) + x];
			eiio_set_pixel(image, x, height - y - 1, EIIO_CHANNEL_R, (int)p->rgbRed);
			eiio_set_pixel(image, x, height - y - 1, EIIO_CHANNEL_G, (int)p->rgbGreen);
			eiio_set_pixel(image, x, height - y - 1, EIIO_CHANNEL_B, (int)p->rgbBlue);
		}
	}
	free(data);
	ReleaseDC(NULL, hDC);
	
	return image;
}

#endif
