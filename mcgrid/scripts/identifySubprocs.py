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
import argparse
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

beam_specs = ['pp', 'ppbar', 'pbarp', 'pbarpbar']

parser = argparse.ArgumentParser(description=__doc__)

# Positional argument for specifying the PDF combination file
parser.add_argument('process_database', metavar='PROCESS_DATABASE',
                    help='The path to the sqlite process database file written out by Sherpa')

# Optional argument for specifying beams
parser.add_argument('-b', '--beamtype',
                    choices=beam_specs,
                    help='The specification of the beams the fastNLO steering'
                    + ' file will be used for. The default is pp.')

# Optional argument for specifying the output file path
parser.add_argument('-o', '--target-path',
                    default='subprocs.config',
                    help='The path for the generated MadGraph PDF combination file.')

args = parser.parse_args()

# Warn the user if the default beams are used
if args.beamtype is None:
    print('WARNING: The default beams are used (pp). Use the --beamtype'
          + ' option to change this.')
    args.beamtype = 'pp'

# Translate the beam spec in a pair of signs
if args.beamtype == 'pp':
    beams = (1, 1)
elif args.beamtype == 'ppbar':
    beams = (1, -1)
elif args.beamtype == 'pbarp':
    beams = (-1, 1)
elif args.beamtype == 'pbarpbar':
    beams = (-1, -1)
else:
    raise Exception('The beam spec could not be translated into a pair of signs')
beamsPDG = (beams[0] * 2212, beams[1] * 2212)

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
unpack("tmp",args.process_database)

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
			print beams[0]*LHA(sub[0][0]), beams[1]*LHA(sub[0][1])

# Open target output file
outfile = open(args.target_path,'w')
outfile.write('0\n')

# Print subprocesses to file
ielement=0
for element in subprocs:
	npairs = 0
	line = ""
	for sub in maps:
		if (sub[1] == element):
			line += str(beams[0]*LHA(sub[0][0])) + " "+ str(beams[1]*LHA(sub[0][1])) +" "
			npairs += 1
	outfile.write(str(ielement)+" "+str(npairs)+ " "+ line+"\n")
	ielement+=1

outfile.close()

# Cleanup
shutil.rmtree("tmp")
print "lumi_pdf config written to subprocs.config"
