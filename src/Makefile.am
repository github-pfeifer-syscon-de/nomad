
bin_PROGRAMS = nomad

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
        -DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
        -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(X11_CFLAGS) \
	$(GLIBMM_CFLAGS) \
	$(GTKMM_CFLAGS) \
	$(RSVG_CFLAGS)

nomad_CFLAGS = -Wall
nomad_CXXFLAGS = -Wall \
	-Wconversion

nomad_LDFLAGS = @EXTRA_LDFLAGS@

nomad_LDADD =  \
	$(X11_LIBS) \
	$(GLIBMM_LIBS) \
	$(GTKMM_LIBS) \
	$(RSVG_LIBS)

nomad_SOURCES = resources.c \
	Config.cpp \
	Config.hpp \
	NomadApp.cpp \
	NomadApp.hpp \
	NomadWin.cpp \
	NomadWin.hpp \
	StringUtils.hpp \
	StringUtils.cpp \
	NomadTreeView.cpp \
	NomadTreeView.hpp \
	Capture.hpp \
	Capture.cpp \
	Shape.hpp \
	Shape.cpp \
	CairoShape.hpp \
	CairoShape.cpp \
	TextShape.hpp \
	TextShape.cpp \
	SvgShape.hpp \
	SvgShape.cpp \
	ScaleableShape.hpp \
	ScaleableShape.cpp \
	LocaleContext.hpp \
	LocaleContext.cpp \
	GenericCallback.hpp \
	GenericCallback.cpp \
	Preview.cpp \
	Preview.hpp

if HAVE_X11
nomad_SOURCES += \
	X11Capture.hpp \
	X11Capture.cpp
endif

if HAVE_WIN
# you might need to remove this if you have no SDK installed
#nomad_CFLAGS += -I "/C/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/um"
#nomad_CXXFLAGS += -I "/C/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/um"
nomad_SOURCES += \
	WinCapture.hpp \
	WinCapture.cpp \
	CWiaDataCallback.hpp \
	CWiaDataCallback.cpp \
	WinScan.hpp \
	WinScan.cpp
endif

