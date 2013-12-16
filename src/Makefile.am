EXTRA_DIST = eiio_config_msvc.h

lib_LTLIBRARIES = libeiio.la 
nodist_libeiio_la_SOURCES = eiio_config_auto.h
libeiio_la_SOURCES = \
eiio.c \
eiio_bmp.c \
eiio_color.c \
eiio_gif.c \
eiio_jpeg.c \
eiio_png.c \
eiio_ffmpeg.c \
eiio_bitmap_handle.c \
eiio.h \
eiio_config.h \
eiio_internal.h

libeiio_la_LIBADD = 
libeiio_la_CFLAGS = 
libeiio_la_LDFLAGS = -no-undefined -version-number @EIIO_MAJOR@:@EIIO_MINOR@:@EIIO_REVISION@ -rpath "$(libdir)"
nodist_include_HEADERS = eiio_config_auto.h
include_HEADERS = eiio.h eiio_config.h

pkgconfigdir = @pkgconfigdir@
pkgconfig_DATA = eiio.pc

CLEANFILES = eiio.pc
