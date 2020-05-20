disnix-lock.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Notifies services to lock or unlock themselves' --libtool ./disnix-lock

disnix-lock.1.xml: disnix-lock.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-lock
noinst_HEADERS = lock-or-unlock.h
noinst_DATA = disnix-lock.1.xml
man1_MANS = disnix-lock.1

disnix_lock_SOURCES = lock-or-unlock.c main.c
disnix_lock_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libmanifest -I../libmain -I../libmodel -I../libdeploy
disnix_lock_LDADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libmain/libmain.la ../libpkgmgmt/libpkgmgmt.la ../libdeploy/libdeploy.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
