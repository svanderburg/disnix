AC_PREREQ(2.61)
AC_INIT([disnix], [0.1])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.7.2 -Wall -Werror])

# Checks for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
PKG_PROG_PKG_CONFIG

# Checks for D-Bus libraries
PKG_CHECK_MODULES(GLIB_DBUS, [dbus-1 dbus-glib-1])

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

# Check for OpenSSL stuff
AC_CHECK_HEADERS([openssl/sha.h])
AC_CHECK_LIB([crypto], [SHA256_Init])

# Output
AC_CONFIG_FILES([
Makefile
conf/Makefile
src/Makefile
src/build/Makefile
src/collect-garbage/Makefile
src/query/Makefile
src/dbus-service/Makefile
src/libxmlutil/Makefile
src/libdistderivation/Makefile
src/libmanifest/Makefile
src/libinfrastructure/Makefile
src/distribute/Makefile
src/activate/Makefile
scripts/Makefile
data/Makefile
man/Makefile
scripts/disnix-instantiate
scripts/disnix-manifest
scripts/disnix-gendist-roundrobin
])
AC_OUTPUT
