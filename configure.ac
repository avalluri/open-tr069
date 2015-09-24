#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([open-tr069], [0.1], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src/evcpe.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wno-portability silent-rules subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# Checks for libraries.
AC_CHECK_LIB([dl], [dlopen dlclose dlsym dlerror])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([bzero inet_ntoa strchr strdup strerror strstr])

PKG_CHECK_MODULES(LIBEVENT, libevent)
AC_SUBST(LIBEVENT_CFLAGS)
AC_SUBST(LIBEVENT_LIBS)

AC_ARG_ENABLE(plugins-dir,
              [  --enable-plugins-dir specifiy the directory to load plugins from],
              [plugins_dir=$enableval], [plugins_dir="$ac_default_prefix/evcpe/plugins"])
AC_DEFINE_UNQUOTED(PLUGINS_DIR, ["$plugins_dir"], [Evcpe plugins location])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 embunit/Makefile
                 tests/Makefile])
AC_OUTPUT
