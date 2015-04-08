pkglib_LTLIBRARIES = libmanifest.la
pkginclude_HEADERS = activationmapping.h distributionmapping.h snapshotmapping.h targets.h manifest.h

libmanifest_la_SOURCES = activationmapping.c distributionmapping.c snapshotmapping.c targets.c manifest.c
libmanifest_la_CFLAGS = $(LIBXML2_CFLAGS) -I../libxmlutil
libmanifest_la_LIBADD = ../libxmlutil/libxmlutil.la
