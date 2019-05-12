disnix-clean-snapshots.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Delete older snapshots stored in a network machines' --libtool ./disnix-clean-snapshots

disnix-clean-snapshots.1.xml: disnix-clean-snapshots.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-clean-snapshots
noinst_HEADERS = clean-snapshots.h
noinst_DATA = disnix-clean-snapshots.1.xml
man1_MANS = disnix-clean-snapshots.1

disnix_clean_snapshots_SOURCES = clean-snapshots.c main.c
disnix_clean_snapshots_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libprocreact -I../libnixxml -I../libinfrastructure -I../libmain -I../libinterface -I../libmodel
disnix_clean_snapshots_LDADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libinfrastructure/libinfrastructure.la ../libmain/libmain.la ../libinterface/libinterface.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
