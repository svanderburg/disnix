disnix-diagnose.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Spawn a remote shell session to diagnose a service' --libtool ./disnix-diagnose

disnix-diagnose.1.xml: disnix-diagnose.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-diagnose
noinst_HEADERS = diagnose.h
noinst_DATA = disnix-diagnose.1.xml
man1_MANS = disnix-diagnose.1

disnix_diagnose_SOURCES = diagnose.c main.c
disnix_diagnose_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libmanifest -I../libmain -I../libinterface -I../libmodel
disnix_diagnose_LDADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
