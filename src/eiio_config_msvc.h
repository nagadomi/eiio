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

#ifndef EIIO_CONFIG_MSVC_H
#define EIIO_CONFIG_MSVC_H

#define EIIO_JPEG 1
#define EIIO_PNG  1
#define EIIO_GIF  0
#define EIIO_BMP  0
#define EIIO_FFMPEG 0

#ifndef NDEBUG
#define EIIO_STRICT 1
#else
#define EIIO_STRICT 0
#endif

#if EIIO_JPEG
#  pragma comment(lib, "libjpeg.lib")
#endif
#if EIIO_PNG
#  pragma comment(lib, "libpng15.lib")
#  pragma comment(lib, "zlib1.lib")
#endif
#if EIIO_GIF
#  pragma comment(lib, "libungif.lib")
#endif

#endif
