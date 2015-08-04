# Determine Rivet analysis name
MAKEFILEPATH := $(abspath $(lastword $(MAKEFILE_LIST)))
ANALYSIS := MCgrid_$(notdir $(patsubst %/,%,$(dir $(MAKEFILEPATH))))

# Rivet flags
RFLAGS=$(shell rivet-config --cppflags)
RLDFLAGS=$(shell rivet-config --ldflags)
RDATA=$(shell rivet-config --datadir)
RLIBS=$(shell rivet-config --libdir)

# APPLgrid flags
ASHARE=$(shell applgrid-config --share)

# MCgrid flags
MCFLAGS=$(shell pkg-config mcgrid --cflags)
MCLDFLAGS=$(shell pkg-config mcgrid --libs)

# combine flags
PLGFLAGS=$(MCFLAGS) $(RFLAGS) 
PLGLDFLAGS=$(MCLDFLAGS) $(RLDFLAGS) 

# plugin input/output and data
RTARGET = analysis/Rivet$(ANALYSIS)Plugins.so
RSOURCES = $(wildcard ./analysis/*.cc)
RYODAFILE = $(wildcard ./analysis/*.yoda)
RDATAFILES = $(wildcard ./analysis/*.info) $(wildcard ./analysis/*.plot) $(RYODAFILE)

.PHONY: all plugin-applgrid plugin-fastnlo plugin-applgrid-and-fastnlo clean install install-applgrid

plugin-applgrid-and-fastnlo:
	rivet-buildplugin $(RTARGET) $(RSOURCES) $(PLGFLAGS) $(PLGLDFLAGS) -DUSE_APPL=1 -DUSE_FNLO=1

plugin-applgrid:
	rivet-buildplugin $(RTARGET) $(RSOURCES) $(PLGFLAGS) $(PLGLDFLAGS) -DUSE_APPL=1

plugin-fastnlo:
	rivet-buildplugin $(RTARGET) $(RSOURCES) $(PLGFLAGS) $(PLGLDFLAGS) -DUSE_FNLO=1
	
clean:
	rm -f $(RTARGET)

install:
	cp $(RDATAFILES) $(RDATA)
	cp $(RTARGET) $(RLIBS)

install-applgrid:
	cp ./subprocesses/*.config $(ASHARE)

# Temporarily hide the data file before plotting as rivet-mkhtml will otherwise normalise against data
# which is not very useful if we want to compare the tiny deviations between grid and generator
# (on the other hand, data must be available for the Rivet init phase, so it has to be installed)
comparison-plot:
	rm -f $(RDATA)/$(ANALYSIS).yoda
	rivet-mkhtml *yoda
	cp $(RYODAFILE) $(RDATA)
