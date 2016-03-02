SHELL = /bin/sh

# general variables
EXAMPLES_PATH = ../examples
STANDARD_TARGET = plugin-applgrid-and-fastnlo install install-applgrid
SCALE_LOG_GRIDS_TARGET = plugin-applgrid-scale-log-grids install install-applgrid
PDF_SET = CT10
PDF_SET_TAG = 10800
SHERPA_FLAGS = PDF_LIBRARY=LHAPDFSherpa PDF_SET=$(PDF_SET) USE_PDF_ALPHAS=1

# Z production variables
CDF_ANALYSIS = CDF_2009_S8383952
CDF_PATH = $(EXAMPLES_PATH)/$(CDF_ANALYSIS)
CDF_HISTO = d02-x01-y01

.PHONY: z z-mcatnlo z-scale-log-grids z-scale-log-grids-up z-scale-log-grids-down

# initialise a validation based on an example directory ($1) and a target ($2)
define initialise-plugin
cd $1 && $(MAKE) $2
endef

# initialise a validation based on an example directory ($1) and a run card file name ($2)
define initialise-sherpa
mkdir -p $@/mceg
-Sherpa -f $1/$2 -p $@/mceg -e 0 $(SHERPA_FLAGS) > $@/run_initialisation.log
-cd $@/mceg && ./makelibs
-Sherpa -f $1/$2 -p $@/mceg -e 0 $(SHERPA_FLAGS) > $@/run_initialisation.log
endef

define finalise
-rm Sherpa_References.tex
endef

# create grids based on an example directory ($1), a run card file name ($2), Sherpa arguments ($3) and an MCgrid path suffix ($4)
define grids-create
cp $1/basic.str .
MCGRID_OUTPUT_PATH=$@/mcgrid$4 Sherpa -e100k -f $1/$2 -p $@/mceg $(SHERPA_FLAGS) ANALYSIS_OUTPUT=${PWD}/$@/sherpa $3 > $@/run_phasespace.log
MCGRID_OUTPUT_PATH=$@/mcgrid$4 Sherpa -e100k -f $1/$2 -p $@/mceg $(SHERPA_FLAGS) ANALYSIS_OUTPUT=${PWD}/$@/sherpa $3 > $@/run_fill.log
rm basic.str
endef

# run sherpa based on an example directory ($1), a run card file name ($2) and Sherpa arguments ($3)
define sherpa
MCGRID_DISABLED=1 Sherpa -e100k -f $1/$2 -p $@/mceg $(SHERPA_FLAGS) ANALYSIS_OUTPUT=${PWD}/$@/sherpa-dedicated $3 > $@/run_dedicated.log
endef

# convolute grids based on an analysis name ($1), a histogram ($2), an MCgrid path suffix ($3) and an applgrid2yoda argument ($4)
define appl-grid-convolute
applgrid2yoda $4 $@/mcgrid$3/MCgrid_$1/$2.root
mv $2_$(PDF_SET)_0_*.yoda $@/appl$3.yoda
endef
define fnlo-grid-convolute
fnlo-tk-yodaout $@/mcgrid$3/MCgrid_$1/$2.tab $(PDF_SET)
mv NLO_$(PDF_SET)_none.yoda $@/fnlo$3.yoda
endef

# reweight based on an example directory ($1) and run card file name ($2) 
define reweight
MCGRID_DISABLED=1 Sherpa -e100k -f $1/$2 -p $@/mceg $(SHERPA_FLAGS) PDF_VARIATIONS=$(PDF_SET)/0 ANALYSIS_OUTPUT=${PWD}/$@/sherpa > $@/run_reweight.log
mv $@/sherpa.MUR1_MUF1_PDF$(PDF_SET_TAG).yoda $@/reweight.yoda
endef

# plot based on a plot configuration file name ($1)
define plot
cd $@ && ../plots/$1
endef

z:
	$(call initialise-plugin,$(CDF_PATH),$(STANDARD_TARGET))
	$(call initialise-sherpa,$(CDF_PATH),Run.dat)
	$(call grids-create,$(CDF_PATH),Run.dat,,)
	$(call appl-grid-convolute,$(CDF_ANALYSIS),$(CDF_HISTO),,)
	$(call fnlo-grid-convolute,$(CDF_ANALYSIS),$(CDF_HISTO),)
	$(call reweight,$(CDF_PATH),Run.dat)
	$(call plot,Z_y.py)
	$(call finalise)

z-mcatnlo:
	$(call initialise-plugin,$(CDF_PATH),$(STANDARD_TARGET))
	$(call initialise-sherpa,$(CDF_PATH),Run_MCatNLO.dat)
	$(call grids-create,$(CDF_PATH),Run_MCatNLO.dat,,)
	$(call appl-grid-convolute,$(CDF_ANALYSIS),$(CDF_HISTO),,)
	$(call fnlo-grid-convolute,$(CDF_ANALYSIS),$(CDF_HISTO),)
	$(call reweight,$(CDF_PATH),Run_MCatNLO.dat)
	$(call plot,Z_y.py)
	$(call finalise)

z-scale-log-grids:
	$(call initialise-plugin,$(CDF_PATH),$(SCALE_LOG_GRIDS_TARGET))
	$(call initialise-sherpa,$(CDF_PATH),Run.dat)
	$(call grids-create,$(CDF_PATH),Run.dat,,)
	$(call appl-grid-convolute,$(CDF_ANALYSIS),$(CDF_HISTO),,)
	$(call reweight,$(CDF_PATH),Run.dat)
	$(call plot,Z_y.py)
	$(call finalise)

z-scale-log-grids-up:
	# initialise scale logs plugin and sherpa
	$(call initialise-plugin,$(CDF_PATH),$(SCALE_LOG_GRIDS_TARGET))
	$(call initialise-sherpa,$(CDF_PATH),Run.dat)

        # create and convolute grids with the scale logs plugin
	$(call grids-create,$(CDF_PATH),Run.dat,,-scalelogs)
	$(call appl-grid-convolute,$(CDF_ANALYSIS),$(CDF_HISTO),-scalelogs,--scale-factors 2.0)

        # initialise standard plugin and create and convolute grids using it
	$(call initialise-plugin,$(CDF_PATH),$(STANDARD_TARGET))
	$(call grids-create,$(CDF_PATH),Run.dat,,)
	$(call appl-grid-convolute,$(CDF_ANALYSIS),$(CDF_HISTO),,--scale-factors 2.0)

	# make dedicated prediction
	$(call sherpa,$(CDF_PATH),Run.dat,SCALEFACTOR:=4.0)

	$(call plot,Z_y.py)
	$(call finalise)

z-scale-log-grids-down:
	# initialise scale logs plugin and sherpa
	$(call initialise-plugin,$(CDF_PATH),$(SCALE_LOG_GRIDS_TARGET))
	#(call initialise-sherpa,$(CDF_PATH),Run.dat)

        # create and convolute grids with the scale logs plugin
	$(call grids-create,$(CDF_PATH),Run.dat,,-scalelogs)
	#(call appl-grid-convolute,$(CDF_ANALYSIS),$(CDF_HISTO),-scalelogs,--scale-factors 0.5)

        # initialise standard plugin and create and convolute grids using it
	#(call initialise-plugin,$(CDF_PATH),$(STANDARD_TARGET))
	#(call grids-create,$(CDF_PATH),Run.dat,,)
	#(call appl-grid-convolute,$(CDF_ANALYSIS),$(CDF_HISTO),,--scale-factors 0.5)

	# make dedicated prediction
	#(call sherpa,$(CDF_PATH),Run.dat,SCALEFACTOR:=0.25)

	#(call plot,Z_y.py)
	#(call finalise)

clean:
	-rm -r z
	-rm -r z-mcatnlo
	-rm -r z-scale-log-grids
	-rm -r z-scale-log-grids-up
	-rm -r z-scale-log-grids-down
