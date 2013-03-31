LIBTOOLFLAGS=--silent

plugin_ldflags = \
  -export-dynamic \
  -no-undefined \
  -avoid-version \
  -module

totempluginsdir=$(shell @PKG_CONFIG@ --variable=pluginsdir totem)
libdir=${totempluginsdir}/@PACKAGE@

lib_LTLIBRARIES=libdelete-play.la

libdelete_play_la_SOURCES=delete-play.c
libdelete_play_la_CFLAGS=$(DEPS_CFLAGS) -Wall
libdelete_play_la_LDFLAGS=$(DEPS_LIBS)$(plugin_ldflags) -version-info 1:0:0

delete_play_plugindir=$(libdir)
delete_play_plugin_DATA=delete-play.plugin

