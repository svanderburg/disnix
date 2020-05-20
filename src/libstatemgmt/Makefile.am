pkglib_LTLIBRARIES = libstatemgmt.la
pkginclude_HEADERS = state-management.h remote-state-management.h copy-snapshots.h

libstatemgmt_la_SOURCES = state-management.c remote-state-management.c copy-snapshots.c
libstatemgmt_la_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact
libstatemgmt_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la
