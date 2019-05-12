disnix-snapshot.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Snapshots the state of components' --libtool ./disnix-snapshot

disnix-snapshot.1.xml: disnix-snapshot.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-snapshot
noinst_HEADERS = run-snapshot.h
noinst_DATA = disnix-snapshot.1.xml
man1_MANS = disnix-snapshot.1

disnix_snapshot_SOURCES = run-snapshot.c main.c
disnix_snapshot_CFLAGS = $(LIBXML2_CFLAGS) $(GLIB2_CFLAGS) -I../libprocreact -I../libnixxml -I../libinfrastructure -I../libmanifest -I../libmodel -I../libmain -I../libmigrate
disnix_snapshot_LDADD = ../libmain/libmain.la ../libmigrate/libmigrate.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
