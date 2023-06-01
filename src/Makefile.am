
bin_PROGRAMS = nomad

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
        -DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
        -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(X11_CFLAGS) \
	$(GLIBMM_CFLAGS) \
	$(GTKMM_CFLAGS)

nomad_CFLAGS = -Wall
nomad_CXXFLAGS = -Wall \
	-Wconversion

nomad_LDFLAGS = @EXTRA_LDFLAGS@

nomad_LDADD =  \
	$(X11_LIBS) \
	$(GLIBMM_LIBS) \
	$(GTKMM_LIBS)

nomad_SOURCES = resources.c \
	NomadApp.cpp \
	NomadApp.hpp \
	NomadWin.cpp \
	NomadWin.hpp \
	StringUtils.hpp \
	NomadTreeView.cpp \
	NomadTreeView.hpp \
	Capture.hpp \
	Capture.cpp
