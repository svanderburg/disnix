AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/disnix\""

pkglib_LTLIBRARIES = libinfrastructure.la
pkginclude_HEADERS = target.h targetstable.h targets-iterator.h targetpropertiestable.h containerstable.h

libinfrastructure_la_SOURCES = target.c targetstable.c targets-iterator.c targetpropertiestable.c containerstable.c
libinfrastructure_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libprocreact -I../libnixxml -I../libnixxml-glib -I../libmodel -I../libpkgmgmt
libinfrastructure_la_LIBADD = $(GLIB2_LIBS) $(LIBXML2_LIBS) $(LIBXSLT_LIBS) ../libprocreact/libprocreact.la ../libnixxml/libnixxml.la ../libnixxml-glib/libnixxml-glib.la ../libmodel/libmodel.la ../libpkgmgmt/libpkgmgmt.la
