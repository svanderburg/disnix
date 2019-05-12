disnix-migrate.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Migrates state from services that have been moved from one machine to another' --libtool ./disnix-migrate

disnix-migrate.1.xml: disnix-migrate.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-migrate
noinst_HEADERS = run-migrate.h
noinst_DATA = disnix-migrate.1.xml
man1_MANS = disnix-migrate.1

disnix_migrate_SOURCES = run-migrate.c main.c
disnix_migrate_CFLAGS = $(LIBXML2_CFLAGS) $(GLIB2_CFLAGS) -I../libprocreact -I../libnixxml -I../libinfrastructure -I../libmanifest -I../libmodel  -I../libmain -I../libmigrate
disnix_migrate_LDADD = ../libmain/libmain.la ../libmigrate/libmigrate.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
