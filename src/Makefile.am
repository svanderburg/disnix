SUBDIRS = libmain libxmlutil libinfrastructure libinterface libpkgmgmt libstatemgmt collect-garbage query dbus-service libdistderivation libmanifest build distribute lock set activate visualize snapshot restore clean-snapshots delete-state capture-infra

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = disnix.pc

apidox:
	doxygen Doxyfile

EXTRA_DIST = disnix.pc.in
