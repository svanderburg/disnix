disnix-activate.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Activate a configuration described in a manifest' --libtool ./disnix-activate

disnix-activate.1.xml: disnix-activate.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-activate
noinst_HEADERS = run-activate.h
noinst_DATA = disnix-activate.1.xml
man1_MANS = disnix-activate.1

disnix_activate_SOURCES = run-activate.c main.c
disnix_activate_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libnixxml -I../libinfrastructure -I../libmanifest -I../libmain -I../libmodel -I../libdeploy -I../libmigrate
disnix_activate_LDADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libmain/libmain.la ../libdeploy/libdeploy.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
