plugindir = "$(abs_top_srcdir)/.dummy-install/plugins"

INCLUDES =												\
	-I$(top_srcdir)									\
	$(PEAS_CFLAGS)									\
	$(WARN_CFLAGS)									\
	$(DISABLE_DEPRECATED)						\
	-DPLUGINDIR="\"$(plugindir)\""

plugin_LTLIBRARIES = libunavailable.la

libunavailable_la_SOURCES =	\
	unavailable-plugin.h			\
	unavailable-plugin.c

libunavailable_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libunavailable_la_LIBADD  = $(PEAS_LIBS)

plugin_DATA = unavailable.plugin

EXTRA_DIST = $(plugin_DATA)

CLEANFILES = $(plugindir)
