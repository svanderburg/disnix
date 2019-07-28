pkglib_LTLIBRARIES = libprofilemanifest.la
pkginclude_HEADERS = profilemanifest.h profilemanifesttarget.h profilemanifesttargettable.h profilemanifesttarget-iterator.h profilelocking.h

libprofilemanifest_la_SOURCES = profilemanifest.c profilemanifesttarget.c profilemanifesttargettable.c profilemanifesttarget-iterator.c profilelocking.c
libprofilemanifest_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libmodel -I../libstatemgmt -I../libnixxml -I../libnixxml-glib -I../libmanifest -I../libinfrastructure
libprofilemanifest_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmodel/libmodel.la ../libstatemgmt/libstatemgmt.la ../libnixxml/libnixxml.la ../libnixxml-glib/libnixxml-glib.la ../libmanifest/libmanifest.la
