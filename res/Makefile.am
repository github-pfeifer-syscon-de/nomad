## build resources
##   the autoreconf complains about the $(shell) construct, fix see configure.ac  -Wno-portability.
##   But this seems the best way to keep this portable

PKGCONFIG = pkg-config
GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)

NOMAD_RESOURCE_XML = nomad.gresources.xml
RESOURCE_C = ../src/resources.c
NOMAD_RESOURCES = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=. --generate-dependencies $(NOMAD_RESOURCE_XML))

all:$(RESOURCE_C)

$(RESOURCE_C): $(NOMAD_RESOURCE_XML) $(NOMAD_RESOURCES)
	$(GLIB_COMPILE_RESOURCES) --target=$(RESOURCE_C) --sourcedir=. --generate-source $(NOMAD_RESOURCE_XML)

# The desktop files
desktopdir = $(datadir)/applications
desktop_DATA=nomad.desktop

# application icon
icon_basename=nomad
pixmapdir=$(datadir)/icons/hicolor/64x64/apps
pixmap_DATA=$(icon_basename).png
appicondir=$(datadir)/icons/hicolor/scalable/apps
appicon_DATA=$(icon_basename).svg

# include source for build
EXTRA_DIST = $(NOMAD_RESOURCE_XML) $(NOMAD_RESOURCES) $(desktop_DATA) $(pixmap_DATA) $(appicon_DATA)

clean-local:
	-rm -rf $(RESOURCE_C)
