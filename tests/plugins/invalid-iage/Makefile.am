plugindir = "$(abs_top_srcdir)/.dummy-install/plugins"

INCLUDES =												\
	-I$(top_srcdir)									\
	$(PEAS_CFLAGS)									\
	$(WARN_CFLAGS)									\
	$(DISABLE_DEPRECATED)						\
	-DPLUGINDIR="\"$(plugindir)\""

plugin_LTLIBRARIES = libinvalid-iage.la

libinvalid_iage_la_SOURCES =  \
	invalid-iage-plugin.h	    	\
	invalid-iage-plugin.c

libinvalid_iage_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libinvalid_iage_la_LIBADD  = $(PEAS_LIBS)

plugin_DATA = invalid-iage.plugin

EXTRA_DIST = $(plugin_DATA)

CLEANFILES = $(plugindir)
