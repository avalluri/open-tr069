plugins_LTLIBRARIES = libworker.la
pluginsdir = $(pkglibdir)/plugins

libworker_la_SOURCES = worker.c
libworker_la_CFLAGS = -I$(abs_top_srcdir)/src/
libworker_la_LDFLAGS = -avoid-version -disable-static
libworker_la_LIBADD = $(abs_top_builddir)/src/libevcpe-common.la

AM_CFLAGS = "--tags -disable-static"
