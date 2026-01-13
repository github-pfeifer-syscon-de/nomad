
TESTS = raw_test

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)."\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-I $(top_srcdir)/src/

AM_CFLAGS= \
	-Wall -Wpedantic

AM_CXXFLAGS = \
	-Wall -Wpedantic \
	$(GENERICIMG_CFLAGS) \
	$(LIBRAW_CFLAGS)

AM_LDFLAGS =

check_PROGRAMS = raw_test

raw_test_LDADD = \
	$(GENERICIMG_LIBS) \
	$(LIBRAW_LIBS)

raw_test_SOURCES = raw_test.cpp
