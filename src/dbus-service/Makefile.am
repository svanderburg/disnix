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
noinst_HEADERS = methods.h signals.h disnix-gtype-def.h disnix-instance.h operation.h disnix-client-operation.h disnix-service-main.h
noinst_DATA = disnix-client.1.xml disnix-service.8.xml
man1_MANS = disnix-client.1
man8_MANS = disnix-service.8

disnix_service_SOURCES = methods.c signals.c disnix-service.c disnix-service-main.c 

disnix_client_SOURCES = disnix-marshal.c disnix-client-operation.c disnix-client.c
disnix_client_CFLAGS = -I../libmain
disnix_client_LDADD = ../libmain/libmain.la

BUILT_SOURCES = disnix-service.h disnix-client.h disnix-marshal.h disnix-marshal.c

disnix-service.h: disnix.xml
	dbus-binding-tool --prefix=disnix --mode=glib-server --output=disnix-service.h disnix.xml

disnix-client.h: disnix.xml
	dbus-binding-tool --prefix=disnix --mode=glib-client --output=disnix-client.h disnix.xml

disnix-marshal.h: marshal.list
	glib-genmarshal --prefix=marshal marshal.list --header > disnix-marshal.h

disnix-marshal.c: marshal.list
	glib-genmarshal --prefix=marshal marshal.list --body > disnix-marshal.c

CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = disnix.xml marshal.list disnix-client.1 disnix-service.8 disnix-client.1.xml disnix-service.8.xml
