#!/usr/bin/env python2

import matplotlib.pyplot as plt
import heppyplotlib as hpl

file_names = [base_name + ".yoda" for base_name in "sherpa", "reweight", "fnlo", "appl"]
labels = ["Sherpa", "Reweighted", "MCgrid/fastNLO", "MCgrid/APPLgrid"]

hpl.ratioplot(file_names, "/MCgrid_CDF_2009_S8383952/d02-x01-y01", labels=labels, errors_enabled=False)

plt.savefig("Z_y.pdf")