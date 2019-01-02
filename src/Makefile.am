SUBDIRS = libprocreact libmain libmodel libinterface libpkgmgmt libstatemgmt libinfrastructure libprofilemanifest libmanifest libmigrate collect-garbage query dbus-service libdistderivation build distribute lock diagnose set activate visualize snapshot restore clean-snapshots delete-state capture-infra capture-manifest run-activity migrate

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = disnix.pc

apidox:
	doxygen Doxyfile

EXTRA_DIST = disnix.pc.in
