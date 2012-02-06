AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/disnix\""

pkglib_LTLIBRARIES = libinfrastructure.la
pkginclude_HEADERS = infrastructure.h

libinfrastructure_la_SOURCES = infrastructure.c
libinfrastructure_la_CFLAGS = $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libxmlutil
libinfrastructure_la_LIBADD = ../libxmlutil/libxmlutil.la $(LIBXML2_LIBS) $(LIBXSLT_LIBS)