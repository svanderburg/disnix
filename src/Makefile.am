SUBDIRS = libprocreact libmain libmodel libinterface libpkgmgmt libstatemgmt libinfrastructure libprofilemanifest collect-garbage query dbus-service libdistderivation libmanifest build distribute lock set activate visualize snapshot restore clean-snapshots delete-state capture-infra capture-manifest

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = disnix.pc

apidox:
	doxygen Doxyfile

EXTRA_DIST = disnix.pc.in
