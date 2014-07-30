#! /usr/bin/env python

"""
    identifySubprocs.py
    
    Reads AMEGIC .alt files for all 2->N process subdirectories
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

# Check directories
os.chdir(os.getcwd())

# Read all .alt files in subdirectories, identify basic subprocesses
# and add subprocs and subproc maps
nfiles=0
for files in glob.glob("./P2_*/*.alt"):
    print files
    with open(files, 'r') as f:
  		line = f.readline()
  		target=(line.split("__")[1],line.split("__")[2])
  		instate=(files.split("__")[1],files.split("__")[2])
  		subprocs.add(target)
		maps.add((instate,target))
    nfiles=nfiles+1

# No files found
if nfiles == 0:
  print "Error: No .alt files found in subdirectories! Please run in SHERPA Process/ Directory"
  sys.exit(2)

# Print out subprocess summary
for element in subprocs:
	print element
	print beam1*LHA(element[0]), beam2*LHA(element[1])
	for sub in maps:
		if (sub[1] == element):
			print beam1*LHA(sub[0][0]), beam2*LHA(sub[0][1])

# Open target output file
outfile = open('subprocs.conf','w')
outfile.write('0\n')

# Print subprocesses to file
ielement=0
for element in subprocs:
	line=str(beam1*LHA(element[0]))+" "+ str(beam2*LHA(element[1]))+ " "
	npairs = 1
	for sub in maps:
		if (sub[1] == element):
			line += str(beam1*LHA(sub[0][0])) + " "+ str(beam2*LHA(sub[0][1])) +" "
			npairs += 1
	outfile.write(str(ielement)+" "+str(npairs)+ " "+ line+"\n")
	ielement+=1

outfile.close()

print "lumi_pdf config written to subprocs.conf"