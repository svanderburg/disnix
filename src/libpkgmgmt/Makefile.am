pkglib_LTLIBRARIES = libpkgmgmt.la
pkginclude_HEADERS = package-management.h copy-closure.h

AM_CPPFLAGS=-DLOCALSTATEDIR=\"$(localstatedir)\"

libpkgmgmt_la_SOURCES = package-management.c copy-closure.c
libpkgmgmt_la_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libinterface
libpkgmgmt_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libinterface/libinterface.la
