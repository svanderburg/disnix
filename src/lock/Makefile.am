disnix-lock.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Notifies services to lock or unlock themselves' --libtool ./disnix-lock

disnix-lock.1.xml: disnix-lock.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-lock
noinst_HEADERS = locking.h
noinst_DATA = disnix-lock.1.xml
man1_MANS = disnix-lock.1

disnix_lock_SOURCES = locking.c main.c
disnix_lock_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libmanifest -I../libmain -I../libinterface
disnix_lock_LDADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la

EXTRA_DIST = disnix-lock.1 disnix-lock.1.xml
