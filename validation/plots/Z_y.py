#!/usr/bin/env python2

import os.path
import matplotlib.pyplot as plt
import heppyplotlib as hpl

file_names = []
labels = []
styles = []


def add_yoda(base_name, label, style=None):
    file_name = base_name + '.yoda'
    if os.path.exists(file_name):
        file_names.append(file_name)
        labels.append(label)
        if style is not None:
            styles.append(style)
        else:
            styles.append({})
        return True
    return False

if not add_yoda("sherpa-dedicated", "Dedicated"):
    add_yoda("sherpa", "Sherpa")
add_yoda("reweight-pdf", "Reweighted")
add_yoda("reweight-scale", "Reweighted")
add_yoda("appl", "MCgrid/APPLgrid")
add_yoda("appl-scalelogs", "MCgrid/APPLgrid (scalelogs)", style={'linestyle': '--'})
add_yoda("fnlo", "MCgrid/fastNLO")

print "Plotting ", labels

axes = hpl.ratioplot(file_names, "/MCgrid_CDF_2009_S8383952/d02-x01-y01", labels=labels, errors_enabled=False, styles=styles)[0]
axes[1].set_ylim(0.99, 1.01)
plt.savefig("Z_y.pdf")
