disnix-client.1: disnix-client.c
	$(HELP2MAN) --output=$@ --no-info --name 'Build store derivations on target machines in a network' --libtool ./disnix-client

disnix-client.1.xml: disnix-client.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

disnix-service.8: disnix-service-main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Build store derivations on target machines in a network' --section 8 --libtool ./disnix-service

disnix-service.8.xml: disnix-service.8
	$(SHELL) ../../maintenance/man2docbook.bash $<

AM_CPPFLAGS=-DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-service disnix-client
noinst_HEADERS = methods.h signaling.h logging.h locking.h jobmanagement.h disnix-client.h disnix-service.h
noinst_DATA = disnix-client.1.xml disnix-service.8.xml
man1_MANS = disnix-client.1
man8_MANS = disnix-service.8

disnix_service_SOURCES = methods.c signaling.c logging.c locking.c jobmanagement.c disnix-service.c disnix-service-main.c disnix-dbus.c
disnix_service_CFLAGS = $(GLIB2_CFLAGS) $(GIO2_CFLAGS) -I../libprocreact -I../libpkgmgmt -I../libstatemgmt -I../libprofilemanifest
disnix_service_LDADD = $(GLIB2_LIBS) $(GIO2_LIBS) ../libpkgmgmt/libpkgmgmt.la ../libstatemgmt/libstatemgmt.la ../libprofilemanifest/libprofilemanifest.la

disnix_client_SOURCES = disnix-client.c disnix-client-main.c disnix-dbus.c
disnix_client_CFLAGS = $(GLIB2_CFLAGS) $(GIO2_CFLAGS) -I../libmain
disnix_client_LDADD = $(GLIB2_LIBS) $(GIO2_LIBS) ../libmain/libmain.la

disnix-dbus.h: disnix.xml
	gdbus-codegen --generate-c-code=disnix-dbus disnix.xml

disnix-dbus.c: disnix.xml
	gdbus-codegen --generate-c-code=disnix-dbus disnix.xml

BUILT_SOURCES = disnix-dbus.c disnix-dbus.h

CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = disnix.xml disnix-client.1 disnix-service.8 disnix-client.1.xml disnix-service.8.xml
