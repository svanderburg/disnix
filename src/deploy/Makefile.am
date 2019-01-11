disnix-deploy.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Deploys a prebuilt Disnix manifest' --libtool ./disnix-deploy

disnix-deploy.1.xml: disnix-deploy.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-deploy
noinst_HEADERS = run-deploy.h
noinst_DATA = disnix-deploy.1.xml
man1_MANS = disnix-deploy.1

disnix_deploy_SOURCES = run-deploy.c main.c
disnix_deploy_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libmanifest -I../libmodel  -I../libmain -I../libmigrate -I../libdeploy
disnix_deploy_LDADD = ../libmain/libmain.la ../libmigrate/libmigrate.la ../libdeploy/libdeploy.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
