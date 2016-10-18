pkglib_LTLIBRARIES = libpkgmgmt.la
pkginclude_HEADERS = package-management.h

AM_CPPFLAGS=-DLOCALSTATEDIR=\"$(localstatedir)\"

libpkgmgmt_la_SOURCES = package-management.c
libpkgmgmt_la_CFLAGS = $(GLIB2_CFLAGS)
libpkgmgmt_la_LIBADD = $(GLIB2_LIBS)
