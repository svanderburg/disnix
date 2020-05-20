disnix-query.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Query the installed services from machines' --libtool ./disnix-query

disnix-query.1.xml: disnix-query.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-query
noinst_HEADERS = query-installed.h
noinst_DATA = disnix-query.1.xml
man1_MANS = disnix-query.1

disnix_query_SOURCES = query-installed.c main.c
disnix_query_LDADD = ../libprocreact/libprocreact.la ../libinfrastructure/libinfrastructure.la ../libmain/libmain.la ../libpkgmgmt/libpkgmgmt.la ../libprofilemanifest/libprofilemanifest.la
disnix_query_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libnixxml -I../libinfrastructure -I../libmain -I../libpkgmgmt -I../libmodel -I../libprofilemanifest

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
