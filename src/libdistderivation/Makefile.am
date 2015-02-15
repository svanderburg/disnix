pkglib_LTLIBRARIES = libdistderivation.la
pkginclude_HEADERS = distributedderivation.h derivationmapping.h interfaces.h

libdistderivation_la_SOURCES = distributedderivation.c derivationmapping.c interfaces.c
libdistderivation_la_CFLAGS = $(LIBXML2_CFLAGS) -I../libxmlutil
libdistderivation_la_LIBADD = ../libxmlutil/libxmlutil.la
