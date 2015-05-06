bin_PROGRAMS = disnix-clean-snapshots
noinst_HEADERS = clean-snapshots.h

disnix_clean_snapshots_SOURCES = clean-snapshots.c main.c
disnix_clean_snapshots_LDADD = ../libinfrastructure/libinfrastructure.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_clean_snapshots_CFLAGS = $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libinfrastructure -I../libmain -I../libinterface
