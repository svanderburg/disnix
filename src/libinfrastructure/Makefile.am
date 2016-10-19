AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/disnix\""

pkglib_LTLIBRARIES = libinfrastructure.la
pkginclude_HEADERS = infrastructure.h

libinfrastructure_la_SOURCES = infrastructure.c
libinfrastructure_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libxmlutil -I../libpkgmgmt
libinfrastructure_la_LIBADD = $(GLIB2_LIBS) $(LIBXML2_LIBS) $(LIBXSLT_LIBS) ../libxmlutil/libxmlutil.la ../libpkgmgmt/libpkgmgmt.la
