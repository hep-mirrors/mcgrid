lib_LTLIBRARIES = libmcgrid.la
libmcgrid_la_SOURCES = src/mcgrid.cpp src/mcgrid_pdf.cpp src/genericFill.cpp src/sherpaFill.cpp
pkginclude_HEADERS = mcgrid/mcgrid.hh mcgrid/mcgrid_pdf.hh mcgrid/mcgrid_binned.hh

libmcgrid_la_LDFLAGS = -version-info 0:0:0 $(RIVET_LDFLAGS) $(APPLGRID_LDFLAGS) $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LIBS) -fPIC -shared
libmcgrid_la_CPPFLAGS= $(RIVET_CPPFLAGS) $(APPLGRID_CPPFLAGS) $(BOOST_CPPFLAGS) -fPIC
libmcgrid_la_CXXFLAGS= $(RIVET_CXXFLAGS) $(APPLGRID_CXXFLAGS) $(BOOST_CXXFLAGS) -fPIC

ACLOCAL_AMFLAGS='-Im4' 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = mcgrid.pc

EXTRA_DIST=manual scripts