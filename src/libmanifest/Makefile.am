AM_CPPFLAGS=-ggdb

pkglib_LTLIBRARIES = libmanifest.la
noinst_HEADERS = activationmapping.h distributionmapping.h

libmanifest_la_SOURCES = activationmapping.c distributionmapping.c
libmanifest_la_CFLAGS = $(LIBXML2_CFLAGS) -I../libxmlutil
libmanifest_la_LIBADD = ../libxmlutil/libxmlutil.la
