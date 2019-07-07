AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

pkglib_LTLIBRARIES = libmanifest.la
pkginclude_HEADERS = servicemappingarray.h distributionmappingtable.h distributionmapping-iterator.h snapshotmappingarray.h servicestable.h interdependencymappingarray.h manifest.h

libmanifest_la_SOURCES = servicemappingarray.c distributionmappingtable.c distributionmapping-iterator.c snapshotmappingarray.c servicestable.c interdependencymappingarray.c manifest.c
libmanifest_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libnixxml -I../libnixxml-glib -I../libmodel -I../libinfrastructure
libmanifest_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmodel/libmodel.la ../libinfrastructure/libinfrastructure.la
