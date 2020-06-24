disnix-copy-closure.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Copy a closure from or to a remote machine through a Disnix interface' --libtool ./disnix-copy-closure

disnix-copy-closure.1.xml: disnix-copy-closure.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-copy-closure
noinst_DATA = disnix-copy-closure.1.xml
man1_MANS = disnix-copy-closure.1

disnix_copy_closure_SOURCES = main.c
disnix_copy_closure_LDADD = ../libmain/libmain.la ../libpkgmgmt/libpkgmgmt.la
disnix_copy_closure_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libmain -I../libpkgmgmt -I../libprocreact

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
