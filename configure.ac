AC_PREREQ(2.61)
AC_INIT([disnix], [0.1])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.7.2 -Wall -Werror])

# Checks for programs
AC_PROG_CC
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG

# Checks for D-Bus libraries
PKG_CHECK_MODULES(GLIB_DBUS, [dbus-1 dbus-glib-1])

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
src/Makefile
scripts/Makefile
data/Makefile
man/Makefile
scripts/disnix-instantiate
])
AC_OUTPUT
