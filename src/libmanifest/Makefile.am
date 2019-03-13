AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

pkglib_LTLIBRARIES = libmanifest.la
pkginclude_HEADERS = activationmapping.h distributionmapping.h snapshotmapping.h targets.h targets-iterator.h manifest.h

libmanifest_la_SOURCES = activationmapping.c distributionmapping.c snapshotmapping.c targets.c targets-iterator.c manifest.c
libmanifest_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libmodel
libmanifest_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmodel/libmodel.la
