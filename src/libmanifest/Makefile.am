AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

pkglib_LTLIBRARIES = libmanifest.la
pkginclude_HEADERS = servicemappingarray.h servicemapping-traverse.h profilemappingtable.h profilemapping-iterator.h snapshotmapping.h snapshotmappingarray.h snapshotmapping-traverse.h manifestservice.h servicestable.h interdependencymapping.h interdependencymappingarray.h manifest.h

libmanifest_la_SOURCES = servicemappingarray.c servicemapping-traverse.c profilemappingtable.c profilemapping-iterator.c snapshotmapping.c snapshotmappingarray.c snapshotmapping-traverse.c manifestservice.c servicestable.c interdependencymapping.c interdependencymappingarray.c manifest.c
libmanifest_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libnixxml -I../libnixxml-glib -I../libmodel -I../libinfrastructure
libmanifest_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmodel/libmodel.la ../libinfrastructure/libinfrastructure.la
