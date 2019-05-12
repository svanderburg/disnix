pkglib_LTLIBRARIES = libmigrate.la
pkginclude_HEADERS = restore.h snapshot.h delete-state.h migrate.h datamigrationflags.h

libmigrate_la_SOURCES = restore.c snapshot.c delete-state.c migrate.c
libmigrate_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libinfrastructure -I../libmanifest -I../libnixxml -I../libmodel -I../libinterface
libmigrate_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libinterface/libinterface.la
