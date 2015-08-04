lib_LTLIBRARIES = libmcgrid.la
libmcgrid_la_SOURCES = src/mcgrid.cpp src/banner.cpp src/sherpaFillInfo.hh src/grid.cpp src/grid_fnlo.cpp src/system.cpp src/banner.hh src/fillInfo.cpp src/mcgrid.hh src/grid.hh src/grid_fnlo.hh src/system.hh src/conventions.hh src/fillInfo.hh src/grid_appl.cpp src/mcgrid_pdf.cpp src/sherpaFillInfo.cpp src/genericFill.cpp src/grid_appl.hh src/sherpaFill.cpp
pkginclude_HEADERS = mcgrid/mcgrid.hh mcgrid/mcgrid_pdf.hh mcgrid/mcgrid_binned.hh

libmcgrid_la_LDFLAGS = -version-info 0:0:0 $(RIVET_LDFLAGS) $(APPLGRID_LDFLAGS) $(FASTNLO_LDFLAGS) $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LIBS) -fPIC -shared
libmcgrid_la_CPPFLAGS= $(RIVET_CPPFLAGS) $(APPLGRID_CPPFLAGS) $(FASTNLO_CPPFLAGS) $(BOOST_CPPFLAGS) -fPIC
libmcgrid_la_CXXFLAGS= $(RIVET_CXXFLAGS) $(APPLGRID_CXXFLAGS) $(FASTNLO_CXXFLAGS) $(BOOST_CXXFLAGS) -fPIC

ACLOCAL_AMFLAGS= -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = mcgrid.pc

EXTRA_DIST=manual scripts
