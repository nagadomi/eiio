#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(eiio, 0.5.6, nagadomi@nurs.or.jp)
EIIO_MAJOR=0
EIIO_MINOR=5
EIIO_REVISION=6
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_PATH_PROG([TEST], [test], [notfound])
if test "" = "$CFLAGS"; then
    CFLAGS=" "
fi
if test "" = "$CPPFLAGS"; then
    CPPFLAGS=" "
fi
if test "" = "$LDFLAGS"; then
    LDFLAGS=" "
fi

CFLAGS=" $CFLAGS -Wall -Wextra -Wformat=2 -Werror=format-security -Wdisabled-optimization -Winline -Wno-unused-parameter "
CFLAGS=" $CFLAGS -D_FORTIFY_SOURCE=2 -fstack-protector --param=ssp-buffer-size=4 "

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P

# Checks for libraries.
LT_INIT([win32-dll])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

AC_CHECK_LIB(ssp, main)

AX_ARG_ENABLE(debug,
        AC_HELP_STRING([--enable-debug], [enable debug symbol and assertion. ]), [no],
	[eiio_strict=1;CFLAGS="$CFLAGS -g "],
        [eiio_strict=0;CFLAGS="$CFLAGS -DNDEBUG -ffast-math -O3 -funroll-loops "])

eiio_arch=native
AC_ARG_WITH(arch,
	AS_HELP_STRING([--with-arch],
	[processor architecture. native/generic. (default is native)]),
	[eiio_arch=${withval}],
	[])
case $eiio_arch in
  yes|"")
    eiio_arch=native
    ;;
  no)
    eiio_arch=generic
    ;;
esac
if test "$eiio_arch" != "generic"; then
   CFLAGS=" $CFLAGS -march=$eiio_arch "
   AC_MSG_NOTICE(adding -march=$eiio_arch)
fi

AX_ARG_ENABLE(png,
        AC_HELP_STRING([--disable-png], [ disable libpng support]), [yes],
        [AC_CHECK_HEADERS(
		[png.h],
		[eiio_png=1;
		AC_CHECK_LIB(png, png_create_read_struct)],
		[AC_MSG_ERROR(png.h not found. please either install libpng-dev or use the --disable-png option.)])
	],	
	[eiio_png=0]
)

AX_ARG_ENABLE(jpeg,
        AC_HELP_STRING([--disable-jpeg], [ disable libjpeg support]), [yes],
        [AC_CHECK_HEADERS(
		[jpeglib.h],
		[eiio_jpeg=1;
		AC_CHECK_LIB(jpeg, jpeg_std_error)],
		[AC_MSG_ERROR(jpeglib.h not found. please either install libjpeg-dev or use the --disable-jpeg option.)])
	],	
	[eiio_jpeg=0]
)

AX_ARG_ENABLE(gif,
        AC_HELP_STRING([--disable-gif], [ disable giflib support]), [yes],
	[AC_CHECK_HEADERS(
		[gif_lib.h],
		[eiio_gif=1;
		AC_CHECK_LIB(gif, DGifOpen)],
		[AC_MSG_ERROR(gif_lib.h not found. please either install libgif-dev or use the --disable-gif option.)])
	],
	[eiio_gif=0]
)

eiio_ffmpeg=0
#AX_ARG_ENABLE(ffmpeg,
#        AS_HELP_STRING([--enable-ffmpeg],[ eanble libavformat and libavcodec support]), [no],
#        [AC_CHECK_HEADERS(
#        [libavcodec/avcodec.h libavformat/avformat.h libavutil/avutil.h libswscale/swscale.h],
#		[eiio_ffmpeg=1
#        AC_CHECK_LIB(avformat, main, [], [AC_MSG_ERROR(please install libavformat-dev.)])
#        AC_CHECK_LIB(avcodec, main, [], [AC_MSG_ERROR(please install libavcodec-dev.)])
#        AC_CHECK_LIB(swscale, main, [], [AC_MSG_ERROR(please install libswscale-dev.)])
#        ],
#		[AC_MSG_ERROR(ffmpeg headers not found. please install libavformat-dev libavcodec-dev libavtuil-dev.)])
#	],
#	[eiio_ffmpeg=0]
#)

AC_ARG_WITH(pkgconfigdir,
	AC_HELP_STRING([--with-pkgconfigdir],
	[Use the specified pkgconfig dir (default is libdir/pkgconfig)]),
	[pkgconfigdir=${withval}],
	[pkgconfigdir='${libdir}/pkgconfig'])
AC_MSG_NOTICE([pkgconfig directory is ${pkgconfigdir}])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])

# host
AC_CANONICAL_HOST
case $host in
  *-*-mingw* | *-*-cygwin*)
    LDFLAGS="$LDFLAGS -Wl,--stack,10485760 "
    LIBS=" $LIBS -lstdc++ -lgdi32 "
    ;;
  *-linux*)
    LDFLAGS=" $LDFLAGS -Wl,-z,relro"
    ;;
  *)
    ;;
esac

CXXFLAGS=$CFLAGS

AC_SUBST(eiio_arch)
AC_SUBST(eiio_strict)
AC_SUBST(eiio_jpeg)
AC_SUBST(eiio_png)
AC_SUBST(eiio_gif)
AC_SUBST(eiio_ffmpeg)
AC_SUBST(pkgconfigdir)
AC_SUBST(EIIO_MAJOR)
AC_SUBST(EIIO_MINOR)
AC_SUBST(EIIO_REVISION)

AC_CONFIG_FILES([
src/eiio.pc
src/eiio_config_auto.h
src/Makefile
vcproj/Makefile
Makefile
])

AC_OUTPUT

echo "libeiio $EIIO_MAJOR.$EIIO_MINOR.$EIIO_REVISION:"
echo "   Processor Architecture .......... : $eiio_arch"
echo "   Building with JPEG support....... : $eiio_jpeg"
echo "   Building with PNG  support....... : $eiio_png"
echo "   Building with GIF  support....... : $eiio_gif"
