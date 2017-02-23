disnix-visualize.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Generate a visualization graph of a manifest' --libtool ./disnix-visualize

disnix-visualize.1.xml: disnix-visualize.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-visualize
noinst_HEADERS = clustertable.h edgestable.h graph.h
noinst_DATA = disnix-visualize.1.xml
man1_MANS = disnix-visualize.1

disnix_visualize_SOURCES = main.c clustertable.c edgestable.c graph.c
disnix_visualize_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libmanifest -I../libmain -I../libmodel
disnix_visualize_LDADD = $(GLIB2_LIBS) ../libmanifest/libmanifest.la ../libmain/libmain.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
