AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/disnix\""

pkglib_LTLIBRARIES = libinfrastructure.la
pkginclude_HEADERS = targetstable.h targets-iterator.h

libinfrastructure_la_SOURCES = targetstable.c targets-iterator.c
libinfrastructure_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libprocreact -I../libnixxml -I../libnixxml-glib -I../libmodel -I../libpkgmgmt
libinfrastructure_la_LIBADD = $(GLIB2_LIBS) $(LIBXML2_LIBS) $(LIBXSLT_LIBS) ../libprocreact/libprocreact.la ../libnixxml/libnixxml.la ../libnixxml-glib/libnixxml-glib.la ../libmodel/libmodel.la ../libpkgmgmt/libpkgmgmt.la
