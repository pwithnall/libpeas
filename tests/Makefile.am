SUBDIRS = plugins testing libpeas

if ENABLE_GTK
SUBDIRS += libpeas-gtk
endif

EXTRA_DIST = Makefile.am.tests

test test-gdb test-report perf-report full-report:
	@for subdir in $(SUBDIRS); do \
	   test "$$subdir" = "testing" -o "$$subdir" = "plugins" || \
	   ( cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $(@) ) || exit $?; \
	 done

.PHONY: test test-gdb test-report perf-report full-report
