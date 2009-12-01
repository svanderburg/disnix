AM_CPPFLAGS=-ggdb

bin_PROGRAMS = disnix-service disnix-client

disnix_service_SOURCES = hash.c methods.c signals.c disnix-service.c disnix-service-main.c 
disnix_service_LDFLAGS = -lgthread-2.0 -lcrypto

disnix_client_SOURCES = disnix-marshal.c disnix-client.c

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
