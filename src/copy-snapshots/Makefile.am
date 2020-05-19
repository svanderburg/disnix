disnix-copy-snapshots.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Copy a set of snapshots from or to a remote machine through a Disnix interface' --libtool ./disnix-copy-snapshots

disnix-copy-snapshots.1.xml: disnix-copy-snapshots.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-copy-snapshots
noinst_DATA = disnix-copy-snapshots.1.xml
man1_MANS = disnix-copy-snapshots.1

disnix_copy_snapshots_SOURCES = main.c
disnix_copy_snapshots_LDADD = ../libmain/libmain.la ../libstatemgmt/libstatemgmt.la
disnix_copy_snapshots_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libmain -I../libstatemgmt

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
