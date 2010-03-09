AM_CPPFLAGS=-ggdb

pkglib_LTLIBRARIES = libdistderivation.la
noinst_HEADERS = derivationmapping.h

libdistderivation_la_SOURCES = derivationmapping.c
libdistderivation_la_CFLAGS = $(LIBXML2_CFLAGS) -I../libxmlutil
libdistderivation_la_LIBADD = ../libxmlutil/libxmlutil.la
