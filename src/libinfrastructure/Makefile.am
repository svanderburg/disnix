AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/disnix\""

pkglib_LTLIBRARIES = libinfrastructure.la
pkginclude_HEADERS = targets.h targets-iterator.h

libinfrastructure_la_SOURCES = targets.c targets-iterator.c
libinfrastructure_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libprocreact -I../libmodel -I../libpkgmgmt
libinfrastructure_la_LIBADD = $(GLIB2_LIBS) $(LIBXML2_LIBS) $(LIBXSLT_LIBS) ../libprocreact/libprocreact.la ../libmodel/libmodel.la ../libpkgmgmt/libpkgmgmt.la
