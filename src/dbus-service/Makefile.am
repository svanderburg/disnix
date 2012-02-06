AM_CPPFLAGS=-DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-service disnix-client
noinst_HEADERS = methods.h signals.h disnix-gtype-def.h disnix-instance.h operation.h disnix-client-operation.h

disnix_service_SOURCES = methods.c signals.c disnix-service.c disnix-service-main.c 

disnix_client_SOURCES = disnix-marshal.c disnix-client-operation.c disnix-client.c

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
EXTRA_DIST = disnix.xml marshal.list
