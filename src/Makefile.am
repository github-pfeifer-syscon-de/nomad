
bin_PROGRAMS = nomad

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
        -DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
        -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(X11_CFLAGS) \
	$(GLIBMM_CFLAGS) \
	$(GTKMM_CFLAGS) \
	$(GENERICIMG_CFLAGS) \
	$(HARU_CFLAGS) \
	$(RSVG_CFLAGS)

nomad_CFLAGS = -Wall
nomad_CXXFLAGS = -Wall \
	-Wconversion

nomad_LDFLAGS = @EXTRA_LDFLAGS@

nomad_LDADD =  \
	$(X11_LIBS) \
	$(GLIBMM_LIBS) \
	$(GTKMM_LIBS) \
	$(GENERICIMG_LIBS) \
	$(HARU_LIBS) \
	$(RSVG_LIBS)

nomad_SOURCES = resources.c \
	Config.cpp \
	Config.hpp \
	NomadApp.cpp \
	NomadApp.hpp \
	NomadWin.cpp \
	NomadWin.hpp \
	EditMode.cpp \
	EditMode.hpp \
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
	PenlWindow.hpp \
	PenlWindow.cpp \
	ScaleableShape.hpp \
	ScaleableShape.cpp \
	Preview.cpp \
	Preview.hpp

if HAVE_X11
nomad_SOURCES += \
	X11Capture.hpp \
	X11Capture.cpp
endif

if HAVE_WIN
nomad_SOURCES += \
	GenericCallback.hpp \
	GenericCallback.cpp \
	ScanDlg.hpp \
	ScanDlg.cpp \
	ScanPreview.hpp \
	ScanPreview.cpp \
	WinCapture.hpp \
	WinCapture.cpp \
	WiaDataCallback.hpp \
	WiaDataCallback.cpp \
	WiaProperty.hpp \
	WiaProperty.cpp \
	WiaDevice.hpp \
	WiaDevice.cpp \
	WiaScan.hpp \
	WiaScan.cpp
endif
