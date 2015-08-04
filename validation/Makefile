SHELL = /bin/sh

# general variables
EXAMPLES_PATH = ../examples
EXAMPLE_TARGET = plugin-applgrid-and-fastnlo install install-applgrid
PDF_SET = CT10
PDF_SET_TAG = 10800
SHERPA_FLAGS = PDF_LIBRARY=LHAPDFSherpa PDF_SET=$(PDF_SET) USE_PDF_ALPHAS=1

# Z production variables
CDF_ANALYSIS = CDF_2009_S8383952
CDF_PATH = $(EXAMPLES_PATH)/$(CDF_ANALYSIS)
CDF_HISTO = d02-x01-y01

.PHONY: z z-mcatnlo

# initialise a validation based on an example directory ($1) and run card file name ($2)
define initialise
cd $1 && $(MAKE) $(EXAMPLE_TARGET)
mkdir -p $@/mceg
-Sherpa -f $1/$2 -p $@/mceg -e 0 $(SHERPA_FLAGS) > $@/run_initialisation.log
-cd $@/mceg && ./makelibs
-Sherpa -f $1/$2 -p $@/mceg -e 0 $(SHERPA_FLAGS) > $@/run_initialisation.log
endef

define finalise
-rm Sherpa_References.tex
endef

# create grids based on an example directory ($1) and run card file name ($2)
define grids-create
cp $1/basic.str .
MCGRID_OUTPUT_PATH=$@/mcgrid Sherpa -e100k -f $1/$2 -p $@/mceg $(SHERPA_FLAGS) ANALYSIS_OUTPUT=${PWD}/$@/sherpa > $@/run_phasespace.log
MCGRID_OUTPUT_PATH=$@/mcgrid Sherpa -e100k -f $1/$2 -p $@/mceg $(SHERPA_FLAGS) ANALYSIS_OUTPUT=${PWD}/$@/sherpa > $@/run_fill.log
rm basic.str
endef

# convolute grids based on an analysis name ($1) and histogram ($2)
define grids-convolute
applgrid2yoda $@/mcgrid/MCgrid_$1/$2.root
fnlo-tk-yodaout $@/mcgrid/MCgrid_$1/$2.tab $(PDF_SET)
mv $2_$(PDF_SET)_0_1.0.yoda $@/appl.yoda
mv NLO_$(PDF_SET)_none.yoda $@/fnlo.yoda
endef

# reweight based on an example directory ($1) and run card file name ($2) 
define reweight
MCGRID_DISABLED=1 Sherpa -e100k -f $1/$2 -p $@/mceg $(SHERPA_FLAGS) PDF_VARIATIONS=$(PDF_SET)/0 ANALYSIS_OUTPUT=${PWD}/$@/sherpa > $@/run_reweight.log
mv $@/sherpa.MUR1_MUF1_PDF$(PDF_SET_TAG).yoda $@/reweight.yoda
endef

# plot based on a plot configuration file name ($1)
define plot
cd $@ && heppyplot ../plots/$1
endef

z:
	$(call initialise,$(CDF_PATH),Run.dat)
	$(call grids-create,$(CDF_PATH),Run.dat)
	$(call grids-convolute,$(CDF_ANALYSIS),$(CDF_HISTO))
	$(call reweight,$(CDF_PATH),Run.dat)
	$(call plot,Z_y.yaml)
	$(call finalise)

z-mcatnlo:
	$(call initialise,$(CDF_PATH),Run_MCatNLO.dat)
	$(call grids-create,$(CDF_PATH),Run_MCatNLO.dat)
	$(call grids-convolute,$(CDF_ANALYSIS),$(CDF_HISTO))
	$(call reweight,$(CDF_PATH),Run_MCatNLO.dat)
	$(call plot,Z_y-mcatnlo.yaml)
	$(call finalise)

clean:
	-rm -r z z-mcatnlo