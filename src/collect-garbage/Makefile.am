disnix-collect-garbage.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Delete garbage from a network of machines' --libtool ./disnix-collect-garbage

disnix-collect-garbage.1.xml: disnix-collect-garbage.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-collect-garbage
noinst_HEADERS = collect-garbage.h
noinst_DATA = disnix-collect-garbage.1.xml
man1_MANS = disnix-collect-garbage.1

disnix_collect_garbage_SOURCES = collect-garbage.c main.c
disnix_collect_garbage_LDADD = ../libprocreact/libprocreact.la ../libinfrastructure/libinfrastructure.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_collect_garbage_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libprocreact -I../libinfrastructure -I../libmain -I../libinterface -I../libxmlutil

EXTRA_DIST = disnix-collect-garbage.1 disnix-collect-garbage.1.xml
