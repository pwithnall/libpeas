SUBDIRS = plugins libpeas

if ENABLE_GTK
SUBDIRS += libpeas-gtk
endif

test test-gdb:
	@for subdir in $(SUBDIRS) ; do \
	   test "$$subdir" = "plugins" || \
	   ( cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $(@) ) || exit $? ; \
	 done

GTESTER_REPORT = gtester-report

# test-report: run tests and generate report
# perf-report: run tests with -m perf and generate report
# full-report: run tests with -m perf -m slow and generate report
test-report perf-report full-report:
	@export GTESTER_LOGDIR=`mktemp -d "$(abs_srcdir)/.testlogs-XXXXXX"` ; \
	 if test -d "$(top_srcdir)/.git"; then \
	   export REVISION="`git describe`" ;  \
	 else \
	   export REVISION="$(VERSION)" ; \
	 fi ; \
	 export TIMESTAMP=`date +%Y-%m-%dT%H:%M:%S%z` ; \
	 case $@ in \
	 test-report) test_options="-k";; \
	 perf-report) test_options="-k -m=perf";; \
	 full-report) test_options="-k -m=perf -m=slow";; \
	 esac ; \
	 echo '<?xml version="1.0"?>'              > $@.xml ; \
	 echo '<report-collection>'               >> $@.xml ; \
	 echo '<info>'                            >> $@.xml ; \
	 echo '  <package>$(PACKAGE)</package>'   >> $@.xml ; \
	 echo '  <version>$(VERSION)</version>'   >> $@.xml ; \
	 echo "  <revision>$$REVISION</revision>" >> $@.xml ; \
	 echo "  <date>$$TIMESTAMP</date>"        >> $@.xml ; \
	 echo '</info>'                           >> $@.xml ; \
	 for subdir in $(SUBDIRS) ; do \
	   test "$$subdir" = "plugins" || { \
	     export GTESTER_LOG=`mktemp "$$GTESTER_LOGDIR/log-XXXXXX"` ; \
	     export GTESTER_ARGS="--verbose $$test_options -o $$GTESTER_LOG" ; \
	     ( cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) generate-report ) ; \
	     sed '1,1s/^<?xml\b[^>?]*?>//' <"$$GTESTER_LOG" >> $@.xml ; \
	   } \
	 done ; \
	 echo >> $@.xml ; \
	 echo '</report-collection>' >> $@.xml ; \
	 $(GTESTER_REPORT) --version 2>/dev/null 1>&2 ; test "$$?" != 0 || $(GTESTER_REPORT) $@.xml >$@.html ; \
	 rm -rf "$$GTESTER_LOGDIR" ;

.PHONY: test test-gdb test-report perf-report full-report

check-local: test


HTML_REPORTS = \
	test-report.html 	\
	perf-report.html 	\
	full-report.html

XML_REPORTS = \
	test-report.xml 	\
	perf-report.xml 	\
	full-report.xml

CLEANFILES = $(HTML_REPORTS) $(XML_REPORTS)
