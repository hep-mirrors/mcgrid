#! /usr/bin/env python

"""
    identifySubprocs.py
    
    Reads COMIX .map files for all 2->N process subdirectories
    and builds a MadGraph PDF combination file for use with
    appl::lumi_pdf
  
"""

import glob
import os
import sys
import getopt

def usage():
  print 'Usage: '+sys.argv[0]+' [-b pp/ppbar/pbarp]'
  print '       Run from SHERPA Process directory'

# Arguments
try:
  opts, args = getopt.getopt(sys.argv[1:], "hb:", ["help", "beamtype="])
except getopt.GetoptError as err:
  # print help information and exit:
  print str(err) # will print something like "option -a not recognized"
  usage()
  sys.exit(2)
beamtype = "pp"
verbose = False
for o, a in opts:
  if o in ("-h", "--help"):
    usage()
    sys.exit()
  elif o in ("-b", "--beamtype"):
    beamtype = a
  else:
    assert False, "unhandled option"


# Determine beam types
if beamtype == "ppbar":
  beam1=1
  beam2=-1
elif beamtype == "pbarp":
  beam1=-1
  beam2=1
elif beamtype == "pp":
  beam1=1
  beam2=1
else:
  print "Error: Beamtype " + beamtype + " is unrecognised"
  sys.exit(2)

print "Using " + beamtype + " beams"

# Parton labelling
partons=['tb','bb','cb','sb','ub','db','G','d','u','s','c','b','t']
def LHA(id):
	return partons.index(id) - 6

# Define Subprocess and Subprocess map sets
subprocs=set()
maps=set()

os.chdir(os.getcwd())
if os.path.exists("./Comix/") is False:
  print "Error: Comix Directory not found! Please run in SHERPA Process/ Directory"
  sys.exit(2)

# Read all .alt files in subdirectories, identify basic subprocesses
# and add subprocs and subproc maps
for files in glob.glob("./Comix/*.map"):
    with open(files, 'r') as f:
  		line = f.readline()
  		instate=(files.split("__")[1],files.split("__")[2])

  		if (instate[0]=="j" or instate[1]=="j" ):
  			continue

  		target=(line.split()[1].split("__")[1],line.split()[1].split("__")[2])
  		subprocs.add(target)
		maps.add((instate,target))

# Print out subprocess summary
for element in subprocs:
	print element
	for sub in maps:
		if (sub[1] == element):
			print beam1*LHA(sub[0][0]), beam2*LHA(sub[0][1])

# Open target output file
outfile = open('subprocs.conf','w')
outfile.write('0\n')

# Print subprocesses to file
ielement=0
for element in subprocs:
	npairs = 0
	line = ""
	for sub in maps:
		if (sub[1] == element):
			line += str(beam1*LHA(sub[0][0])) + " "+ str(beam2*LHA(sub[0][1])) +" "
			npairs += 1
	outfile.write(str(ielement)+" "+str(npairs)+ " "+ line+"\n")
	ielement+=1

outfile.close()

print "lumi_pdf config written to subprocs.conf"