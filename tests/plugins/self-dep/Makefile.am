plugindir = "$(abs_top_srcdir)/.dummy-install/plugins"

INCLUDES =												\
	-I$(top_srcdir)									\
	$(PEAS_CFLAGS)									\
	$(WARN_CFLAGS)									\
	$(DISABLE_DEPRECATED)						\
	-DPLUGINDIR="\"$(plugindir)\""

plugin_LTLIBRARIES = libself-dep.la

libself_dep_la_SOURCES =  \
	self-dep-plugin.h	    	\
	self-dep-plugin.c

libself_dep_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libself_dep_la_LIBADD  = $(PEAS_LIBS)

plugin_DATA = self-dep.plugin

EXTRA_DIST = $(plugin_DATA)

CLEANFILES = $(plugindir)
