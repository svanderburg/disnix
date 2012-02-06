pkglib_LTLIBRARIES = libmanifest.la
pkginclude_HEADERS = activationmapping.h distributionmapping.h targets.h

libmanifest_la_SOURCES = activationmapping.c distributionmapping.c targets.c
libmanifest_la_CFLAGS = $(LIBXML2_CFLAGS) -I../libxmlutil
libmanifest_la_LIBADD = ../libxmlutil/libxmlutil.la
