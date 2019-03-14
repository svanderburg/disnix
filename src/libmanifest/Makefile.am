AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

pkglib_LTLIBRARIES = libmanifest.la
pkginclude_HEADERS = activationmapping.h distributionmapping.h distributionmapping-iterator.h snapshotmapping.h manifest.h

libmanifest_la_SOURCES = activationmapping.c distributionmapping.c distributionmapping-iterator.c snapshotmapping.c manifest.c
libmanifest_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libmodel -I../libinfrastructure
libmanifest_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmodel/libmodel.la ../libinfrastructure/libinfrastructure.la
