disnix-distribute.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Distributes intra-dependency closures of services to target machines' --libtool ./disnix-distribute

disnix-distribute.1.xml: disnix-distribute.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-distribute
noinst_HEADERS = distribute.h
noinst_DATA = disnix-distribute.1.xml
man1_MANS = disnix-distribute.1

disnix_distribute_SOURCES = distribute.c main.c
disnix_distribute_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libmanifest -I../libmain -I../libinterface
disnix_distribute_LDADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la

EXTRA_DIST = disnix-distribute.1 disnix-distribute.1.xml
