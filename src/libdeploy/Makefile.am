pkglib_LTLIBRARIES = libdeploy.la
pkginclude_HEADERS = distribute.h locking.h set-profiles.h transition.h activate.h deploy.h deploymentflags.h

libdeploy_la_SOURCES = distribute.c locking.c set-profiles.c transition.c activate.c deploy.c
libdeploy_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libinfrastructure -I../libmanifest -I../libnixxml -I../libmodel -I../libpkgmgmt -I../libstatemgmt -I../libmigrate
libdeploy_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libpkgmgmt/libpkgmgmt.la ../libstatemgmt/libstatemgmt.la ../libmigrate/libmigrate.la
