disnix-run-activity.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Directly executes a Disnix deployment activity' --libtool ./disnix-run-activity

disnix-run-activity.1.xml: disnix-run-activity.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

AM_CPPFLAGS=-DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-run-activity
noinst_HEADERS = run-activity.h
noinst_DATA = disnix-run-activity.1.xml
man1_MANS = disnix-run-activity.1

disnix_run_activity_SOURCES = run-activity.c main.c
disnix_run_activity_CFLAGS = $(GLIB2_CFLAGS) $(GIO2_CFLAGS) -I../libmain -I../libprocreact -I../libpkgmgmt -I../libstatemgmt -I../libprofilemanifest
disnix_run_activity_LDADD = $(GLIB2_LIBS) $(GIO2_LIBS) ../libmain/libmain.la ../libpkgmgmt/libpkgmgmt.la ../libstatemgmt/libstatemgmt.la ../libprofilemanifest/libprofilemanifest.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
