include $(top_srcdir)/tests/Makefile.am.tests

INCLUDES = \
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	-I$(top_srcdir)/tests/testing	\
	$(PEAS_CFLAGS)			\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)

# why does this pick up installed instead of not installed stuff??

noinst_PROGRAMS = $(TEST_PROGS)

progs_ldadd = \
	$(PEAS_LIBS)					\
	$(top_builddir)/libpeas/libpeas-1.0.la		\
	$(top_builddir)/tests/testing/libtesting-1.0.la


TEST_PROGS     += engine
engine_SOURCES  = engine.c
engine_LDADD    = $(progs_ldadd)

TEST_PROGS        += extension
extension_SOURCES  = extension.c
extension_LDADD    = $(progs_ldadd)

TEST_PROGS            += extension-set
extension_set_SOURCES  = extension-set.c
extension_set_LDADD    = $(progs_ldadd)
