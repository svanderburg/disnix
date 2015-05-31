disnix-build.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Build store derivations on target machines in a network' --libtool ./disnix-build

disnix-build.1.xml: disnix-build.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-build
noinst_HEADERS = build.h
noinst_DATA = disnix-build.1.xml
man1_MANS = disnix-build.1

disnix_build_SOURCES = build.c main.c
disnix_build_LDADD = ../libdistderivation/libdistderivation.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_build_CFLAGS = -I../libdistderivation -I../libmain -I../libinterface

EXTRA_DIST = disnix-build.1 disnix-build.1.xml
