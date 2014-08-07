#! /usr/bin/env python

"""
    identifySubprocs.py
    
    Reads COMIX/AMEGIC info files for all 2->N processes
    and builds a MadGraph PDF combination file for use with
    appl::lumi_pdf
  
"""

import glob
import os
import sys
import getopt
import shutil

# *************** Database Unpack ******************

import sqlite3
import sys

def unpack(procDir,procDB):
  # Fetch process DB
  sql_conn = sqlite3.connect(procDB)
  sql_cur = sql_conn.cursor()
  
  # query
  query="SELECT * FROM path;"
  sql_cur.execute(query)
  
  # Write out packed files
  for idx, row in enumerate(sql_cur):
    
    name = row[0].split('/')[-1]
    cont = row[1]
    
    flname = "{0}/{1}".format(procDir,name)
    output = open(flname, "wt")
    output.write(str(cont))
    
    output.close()

# **************************************************


def usage():
  print 'Usage: '+sys.argv[0]+' <Process Database> [-b pp/ppbar/pbarp]'

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

# Make directory if not already present
os.chdir(os.getcwd())
if not os.path.exists("./tmp"):
  os.makedirs("./tmp")

# unpack database
unpack("tmp",sys.argv[1])

# First check for AMEGIC alt files

# Read all .alt files, identify basic subprocesses
# and add subprocs and subproc maps
nfiles=0
for files in glob.glob("./tmp/*.alt"):
  with open(files, 'r') as f:
    line = f.readline()
    target=(line.split("__")[1],line.split("__")[2])
    instate=(files.split("__")[1],files.split("__")[2])
    subprocs.add(target)
    maps.add((instate,target))
    maps.add((target,target))
  nfiles=nfiles+1

# No files found
if nfiles == 0:
  print "No Amegic files found, searching for COMIX maps..."

  # Read all .map files, identify basic subprocesses
  # and add subprocs and subproc maps
  for files in glob.glob("./tmp/*.map"):
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

# Cleanup
shutil.rmtree("tmp")
print "lumi_pdf config written to subprocs.conf"