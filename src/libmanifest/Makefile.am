AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

pkglib_LTLIBRARIES = libmanifest.la
pkginclude_HEADERS = interdependencymapping.h \
	interdependencymappingarray.h \
	manifest.h \
	manifestservice.h \
	manifestservicestable.h \
	mappingparameters.h \
	profilemappingtable.h \
	profilemapping-iterator.h \
	servicemapping.h \
	servicemapping-traverse.h \
	servicemappingarray.h \
	snapshotmapping.h \
	snapshotmappingarray.h \
	snapshotmapping-traverse.h

libmanifest_la_SOURCES = interdependencymapping.c \
	interdependencymappingarray.c \
	manifest.c \
	manifestservice.c \
	manifestservicestable.c \
	mappingparameters.c \
	profilemappingtable.c \
	profilemapping-iterator.c \
	servicemapping.c \
	servicemappingarray.c \
	servicemapping-traverse.c \
	snapshotmapping.c \
	snapshotmappingarray.c \
	snapshotmapping-traverse.c

libmanifest_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libnixxml -I../libnixxml-glib -I../libmodel -I../libinfrastructure
libmanifest_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmodel/libmodel.la ../libinfrastructure/libinfrastructure.la
