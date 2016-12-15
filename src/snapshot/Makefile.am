disnix-snapshot.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Snapshots the state of components' --libtool ./disnix-snapshot

disnix-snapshot.1.xml: disnix-snapshot.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-snapshot
noinst_HEADERS = snapshot.h
noinst_DATA = disnix-snapshot.1.xml
man1_MANS = disnix-snapshot.1

disnix_snapshot_SOURCES = snapshot.c main.c
disnix_snapshot_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libmanifest -I../libmain -I../libinterface
disnix_snapshot_LDADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la

EXTRA_DIST = disnix-snapshot.1 disnix-snapshot.1.xml
