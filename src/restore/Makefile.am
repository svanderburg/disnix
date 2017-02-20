disnix-restore.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Restores the state of components' --libtool ./disnix-restore

disnix-restore.1.xml: disnix-restore.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-restore
noinst_HEADERS = restore.h
noinst_DATA = disnix-restore.1.xml
man1_MANS = disnix-restore.1

disnix_restore_SOURCES = restore.c main.c
disnix_restore_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libmanifest -I../libmain -I../libinterface
disnix_restore_LDADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
