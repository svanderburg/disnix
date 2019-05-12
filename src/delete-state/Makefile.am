disnix-delete-state.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Deletes state of components that have become obsolete' --libtool ./disnix-delete-state

disnix-delete-state.1.xml: disnix-delete-state.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-delete-state
noinst_HEADERS = run-delete-state.h
noinst_DATA = disnix-delete-state.1.xml
man1_MANS = disnix-delete-state.1

disnix_delete_state_SOURCES = run-delete-state.c main.c
disnix_delete_state_CFLAGS = $(LIBXML2_CFLAGS) $(GLIB2_CFLAGS) -I../libprocreact -I../libnixxml -I../libinfrastructure -I../libmanifest -I../libmain -I../libinterface -I../libmodel -I../libmigrate
disnix_delete_state_LDADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la ../libmigrate/libmigrate.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
