include $(top_srcdir)/tests/Makefile.am.tests

INCLUDES = \
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	-I$(top_srcdir)/tests/testing	\
	$(PEAS_CFLAGS)			\
	$(PEAS_GTK_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)

# why does this pick up installed instead of not installed stuff??

noinst_PROGRAMS = $(TEST_PROGS)


progs_ldadd = \
	$(PEAS_LIBS)					\
	$(PEAS_GTK_LIBS)				\
	$(top_builddir)/libpeas/libpeas-1.0.la		\
	$(top_builddir)/libpeas-gtk/libpeas-gtk-1.0.la	\
	$(top_builddir)/tests/testing/libtesting-1.0.la

TEST_PROGS      += console
console_SOURCES  = console.c
console_LDADD    = $(progs_ldadd)
