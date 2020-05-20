disnix-capture-infra.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Captures the container configurations of machines and generates an infrastructure expression from it' --libtool ./disnix-capture-infra

disnix-capture-infra.1.xml: disnix-capture-infra.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-capture-infra
noinst_HEADERS = capture-infra.h
noinst_DATA = disnix-capture-infra.1.xml
man1_MANS = disnix-capture-infra.1

disnix_capture_infra_SOURCES = capture-infra.c main.c
disnix_capture_infra_LDADD = ../libprocreact/libprocreact.la ../libinfrastructure/libinfrastructure.la ../libmain/libmain.la ../libstatemgmt/libstatemgmt.la
disnix_capture_infra_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libprocreact -I../libnixxml -I../libnixxml-glib -I../libinfrastructure -I../libmain -I../libstatemgmt -I../libmodel

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
