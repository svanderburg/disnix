disnix-delete-state.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Deletes state of components that have become obsolete' --libtool ./disnix-delete-state

disnix-delete-state.1.xml: disnix-delete-state.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-delete-state
noinst_HEADERS = delete-state.h
noinst_DATA = disnix-delete-state.1.xml
man1_MANS = disnix-delete-state.1

disnix_delete_state_SOURCES = delete-state.c main.c
disnix_delete_state_LDADD = ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_delete_state_CFLAGS = -I../libmanifest -I../libmain -I../libinterface

EXTRA_DIST = disnix-delete-state.1 disnix-delete-state.1.xml
