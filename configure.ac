AC_PREREQ(2.61)
AC_INIT([disnix], m4_esyscmd([echo -n $(cat ./version)$VERSION_SUFFIX]))
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.9 tar-pax -Wall -Werror -Wno-extra-portability])

# Checks for installed programs
AC_DEFUN([NEED_PROG],
[
AC_PATH_PROG($1, $2)
if test -z "$$1"; then
    AC_MSG_ERROR([$2 is required])
fi
])

# Checks for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
PKG_PROG_PKG_CONFIG
NEED_PROG(bash, bash)
NEED_PROG(getopt, getopt)
NEED_PROG(nix_store, nix-store)
NEED_PROG(dysnomia, dysnomia)
AC_PATH_PROG(xmllint, xmllint, false)
AC_PATH_PROG(xsltproc, xsltproc, false)
AC_PATH_PROG(dblatex, dblatex)
AC_PATH_PROG(HELP2MAN, help2man, false)
AC_PATH_PROG(DOCLIFTER, doclifter, false)

# Checks for glib libraries
GLIB2_REQUIRED=2.26.0
PKG_CHECK_MODULES(GLIB2, glib-2.0 >= $GLIB2_REQUIRED)
AC_SUBST(GLIB2_CFLAGS)
AC_SUBST(GLIB2_LIBS)

PKG_CHECK_MODULES(GIO2, gio-unix-2.0 >= $GLIB2_REQUIRED)
AC_SUBST(GIO2_CFLAGS)
AC_SUBST(GIO2_LIBS)

# Checks for libxml2 libraries
LIBXML2_REQUIRED=2.5.10
PKG_CHECK_MODULES(LIBXML2, libxml-2.0 >= $LIBXML2_REQUIRED)
AC_SUBST(LIBXML2_CFLAGS)
AC_SUBST(LIBXML2_LIBS)

# Checks for libxslt libraries
LIBXSLT_REQUIRED=1.1.10
PKG_CHECK_MODULES(LIBXSLT, libxslt >= $LIBXSLT_REQUIRED)
AC_SUBST(LIBXSLT_CFLAGS)
AC_SUBST(LIBXSLT_LIBS)

# Checks for Docbook stuff
AC_ARG_WITH(docbook-rng, AC_HELP_STRING([--with-docbook-rng=PATH],
  [path of the DocBook RelaxNG schema]),
  docbookrng=$withval, docbookrng=/docbook-rng-missing)
AC_SUBST(docbookrng)

AC_ARG_WITH(docbook-xsl, AC_HELP_STRING([--with-docbook-xsl=PATH],
  [path of the DocBook XSL stylesheets]),
  docbookxsl=$withval, docbookxsl=/docbook-xsl-missing)
AC_SUBST(docbookxsl)

# Add D-Bus specific flags
CFLAGS="$CFLAGS $GLIB_DBUS_CFLAGS"
LIBS="$LIBS $GLIB_DBUS_LIBS"

AC_ARG_WITH(dbus-sys, [  --with-dbus-sys=<dir>   where D-BUS system.d directory is])

if ! test -z "$with_dbus_sys" ; then
    DBUS_SYS_DIR="$with_dbus_sys"
else
    DBUS_SYS_DIR="$sysconfdir/dbus-1/system.d"
fi
AC_SUBST(DBUS_SYS_DIR)

# State should be stored in /nix/var, unless the user overrides it explicitly.
test "$localstatedir" = '${prefix}/var' && localstatedir=/nix/var

# Output
AC_CONFIG_FILES([
Makefile
conf/Makefile
src/Makefile
src/disnix.pc
src/build/Makefile
src/collect-garbage/Makefile
src/diagnose/Makefile
src/query/Makefile
src/dbus-service/Makefile
src/libprocreact/Makefile
src/libmodel/Makefile
src/libdistderivation/Makefile
src/libmain/Makefile
src/libmanifest/Makefile
src/libinfrastructure/Makefile
src/libinterface/Makefile
src/libpkgmgmt/Makefile
src/libstatemgmt/Makefile
src/libprofilemanifest/Makefile
src/distribute/Makefile
src/lock/Makefile
src/set/Makefile
src/activate/Makefile
src/visualize/Makefile
src/snapshot/Makefile
src/restore/Makefile
src/clean-snapshots/Makefile
src/delete-state/Makefile
src/capture-infra/Makefile
src/capture-manifest/Makefile
src/run-activity/Makefile
scripts/Makefile
data/Makefile
maintenance/Makefile
scripts/disnix-copy-closure
scripts/disnix-copy-snapshots
scripts/disnix-delegate
scripts/disnix-deploy
scripts/disnix-env
scripts/disnix-instantiate
scripts/disnix-manifest
scripts/disnix-migrate
scripts/disnix-gendist-roundrobin
scripts/disnix-reconstruct
scripts/disnix-ssh-client
doc/Makefile
doc/manual/Makefile
init.d/Makefile
])
AC_OUTPUT
