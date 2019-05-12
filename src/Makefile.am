SUBDIRS = libprocreact libnixxml libnixxml-glib libmain libmodel libinterface libpkgmgmt libstatemgmt libinfrastructure libprofilemanifest libmanifest libmigrate libdeploy libdistderivation libbuild collect-garbage query dbus-service build distribute lock diagnose set activate visualize snapshot restore clean-snapshots delete-state capture-infra capture-manifest run-activity migrate deploy

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = disnix.pc

apidox:
	doxygen Doxyfile

EXTRA_DIST = disnix.pc.in
