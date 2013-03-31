#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(totem-plugin-delete-play, 1.0.0, snmpenv@gmail.com)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/delete-play.c)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

PKG_CHECK_MODULES([DEPS], [libpeas-1.0 totem])

AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_FILES([Makefile src/Makefile])


AC_ENABLE_SHARED
AC_DISABLE_STATIC
LT_INIT

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT
