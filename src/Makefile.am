SUBDIRS = libmain libxmlutil libinfrastructure libinterface collect-garbage query dbus-service libdistderivation libmanifest build distribute activate visualize snapshot restore

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = disnix.pc

apidox:
	doxygen Doxyfile

EXTRA_DIST = disnix.pc.in
