dummydir = "$(abs_top_srcdir)/.dummy-install/testing"

noinst_LTLIBRARIES = libtesting-1.0.la

INTROSPECTION_INCLUDES = \
	-I$(top_srcdir)				\
	-I$(srcdir)				\
	$(PEAS_CFLAGS)				\
	-DBUILD_DIR="\"$(abs_top_builddir)\""	\
	-DSRC_DIR="\"$(abs_top_srcdir)\""	\
	-DTESTING_COMPILATION

INCLUDES = \
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

libtesting_1_0_la_LDFLAGS = \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libtesting_1_0_la_LIBADD = \
	$(PEAS_LIBS)				\
	$(top_builddir)/libpeas/libpeas-1.0.la

libtesting_1_0_la_SOURCES = \
	testing.h			\
	testing-engine.h		\
	testing-engine.c		\
	testing-unimplementable.h	\
	testing-unimplementable.c

CLEANFILES = $(dummydir)

if ENABLE_GTK
INCLUDES += \
	$(PEAS_GTK_CFLAGS)	\
	-DENABLE_GTK

libtesting_1_0_la_LIBADD += \
	$(PEAS_GTK_LIBS)				\
	$(top_builddir)/libpeas-gtk/libpeas-gtk-1.0.la
endif

INCLUDES += $(INTROSPECTION_INCLUDES)

if HAVE_INTROSPECTION
  -include $(INTROSPECTION_MAKEFILE)
  INTROSPECTION_GIRS = Testing-1.0.gir
  INTROSPECTION_SCANNER_ARGS = \
	--add-include-path=$(top_builddir)/libpeas	\
	--c-include=testing/testing.h			\
	--warn-all
  INTROSPECTION_COMPILER_ARGS = --includedir=$(top_builddir)/libpeas

  introspection_sources = $(libtesting_1_0_la_SOURCES)

  Testing-1.0.gir: libtesting-1.0.la
  Testing_1_0_gir_INCLUDES = GObject-2.0 GIRepository-2.0 Peas-1.0
  Testing_1_0_gir_CFLAGS = $(INTROSPECTION_INCLUDES) -DTESTING_GIR_COMPILATION
  Testing_1_0_gir_LIBS = libtesting-1.0.la
  Testing_1_0_gir_FILES = $(addprefix $(srcdir)/,$(introspection_sources))

  girdir = $(dummydir)/gir-1.0
  gir_DATA = $(INTROSPECTION_GIRS)

  typelibdir = $(dummydir)/girepository-1.0
  typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

  CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
