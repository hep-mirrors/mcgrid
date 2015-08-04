#! /usr/bin/env python

"""This script reads a MadGraph PDF combination file (possibly created by
identifySubprocs.py) and builds a fastNLO steering file based on it.
"""

from __future__ import print_function
import argparse
import os.path

beam_specs = ['pp', 'ppbar', 'pbarp', 'pbarpbar']

parser = argparse.ArgumentParser(description=__doc__)

# Positional argument for specifying the PDF combination file
parser.add_argument('base_file_path', metavar='SOURCE_PATH',
                    help='The path to the MadGraph PDF combination file'
                    + ' on which the output fastNLO steering file should'
                    + ' be based.')

# Optional argument for specifying beams (fastNLO expects the parton IDs to
# reflect that correctly, whereas the source file always assumpes pp)
parser.add_argument('-b', '--beamtype',
                    choices=beam_specs,
                    help='The specification of the beams the fastNLO steering'
                    + ' file will be used for. The default is pp.')

# Optional argument for specifying the output file path
parser.add_argument('-o', '--target-path',
                    help='The path for the generated fastNLO steering file.')

args = parser.parse_args()

# Determine source file name
source_file_name = os.path.basename(args.base_file_path)

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

# Determine the steering file path
should_add_default_file_name = False
if args.target_path is None:
    steering_file_path = ''
    should_add_default_file_name = True
else:
    steering_file_path = args.target_path
    if os.path.isdir(args.target_path):
        should_add_default_file_name = True
if should_add_default_file_name:
    default_file_name = os.path.splitext(source_file_name)[0] + '.str'
    steering_file_path = os.path.join(steering_file_path, default_file_name)

# Parse the PDF combination file
subprocs = []
with open(args.base_file_path) as f:
    for idx, line in enumerate(f):
        # We are not interested in the CKM matrix flag in the first line
        if idx != 0 and line != '':
            # The rest of the lines are like this 'id #pairs pair1 pair2'
            # where '#pairs' is the number of pairs and each 'pairN' is a pair
            # of Ints representing the parton ID
            # We do not need the 'id', because it is already encoded as the
            # index in our list to be build
            pairs = []
            components = map(int, line.split()[1:])
            for i in range(int(components[0])):
                pairs.append(components[2*i+1:2*i+3])
            subprocs.append(pairs)

# Write the fastNLO steering file
with open(steering_file_path, 'w') as f:
    # Write header
    f.write('# -*-sh-*-\n')
    f.write('# ==================================================================== #\n')
    f.write('#\n')
    f.write('#   A steering file for creating a fastNLO table with MCgrid\n')
    f.write('#   Automatically generated from the PDF combination file\n')
    f.write('#\n')
    f.write('#     ' + source_file_name + '\n')
    f.write('#\n')
    f.write('# ==================================================================== #\n\n\n')

    f.write('# -------------------------------------------------------------------- #\n')
    f.write('# General settings \n')
    f.write('# -------------------------------------------------------------------- #\n\n')

    # Write some parameters that we cannot determine from MCgrid
    f.write('# NOTE: Please review the following settings and modify them if needed\n\n')

    f.write('# Unit of data cross sections (negative power of 10, e.g. 12->pb, 15->fb)\n')
    f.write('PublicationUnits              12\n\n')

    f.write('# Units of coeffients as passed to fastNLO (negative power of 10)\n')
    f.write('UnitsOfCoefficients           12\n\n')

    f.write('# Specify scale name and unit\n')
    f.write('ScaleDescriptionScale1        "Description [GeV]"\n\n')

    f.write('# Labels (symbol and unit) for the measurement dimension\n')
    f.write('DimensionLabels {\n')
    f.write('   "Quantity [Unit]"\n')
    f.write('}\n\n')

    # Write some commented parameters that might be uncommented to overwrite
    # defaults set from MCgrid
    f.write('## NOTE: The following settings can be uncommented to overwrite MCgrid defaults\n\n')

    f.write('## MCgrid default: Rivet histogram name\n')
    f.write('#ScenarioName                 Name\n\n')

    f.write('#ScenarioDescription {\n')
    f.write('#   "Description"\n')
    f.write('#   "RIVET_ID=ANALYSIS/HISTOGRAM"\n')
    f.write('#}\n\n')

    f.write('## MCgrid default: ["Sherpa"]\n')
    f.write('#CodeDescription {\n')
    f.write('#   "My Event Generator"\n')
    f.write('#}\n\n')

    f.write('## Number of decimal digits to store in the output table\n')
    f.write('#OutputPrecision              8\n\n')

    f.write('## Global output verbosity of fastNLO\n')
    f.write('# Possible values are DEBUG, MANUAL, INFO, WARNING, ERROR, SILENT\n')
    f.write('#GlobalVerbosity              ERROR\n\n')
    f.write('## Apply PDF reweighting for an optimized interpolation\n')
    f.write('#ApplyPDFReweighting          true\n\n')
    f.write('## Set limits for scale nodes to bin borders, if possible\n')
    f.write('#CheckScaleLimitsAgainstBins  true\n\n\n')

    f.write('# -------------------------------------------------------------------- #\n')
    f.write('# PDF combination settings \n')
    f.write('# -------------------------------------------------------------------- #\n\n')

    # Write stuff related to the PDF combinations
    pdfFormat = 'PDF{0}                          {1: 4}\n'
    for i in range(2):
        f.write(pdfFormat.format(i+1, beamsPDG[i]))
    f.write('\n')
    nSubprocsString = str(len(subprocs))
    f.write('NSubProcessesLO                  ' + nSubprocsString + '\n');
    f.write('NSubProcessesNLO                 ' + nSubprocsString + '\n');
    f.write('NSubProcessesNNLO                ' + nSubprocsString + '\n');
    f.write('IPDFdef3LO                       ' + nSubprocsString + '\n');
    f.write('IPDFdef3NLO                      ' + nSubprocsString + '\n');
    f.write('IPDFdef3NNLO                     ' + nSubprocsString + '\n\n');

    # Finally write actual PDF combinations
    for idx, label in enumerate(('PartonCombinationsLO', 'PartonCombinationsNLO', 'PartonCombinationsNNLO')):
        f.write(label +' {{\n')
        if idx == 0: 
            f.write('  # one line here!\n')
        else:
            f.write('\n')
        subprocIDFormat = '{0:3}'
        partonIDFormat = ' {0: 1}'
        for subprocID, pairs in enumerate(subprocs):
            f.write(subprocIDFormat.format(subprocID))
            for pair in pairs:
                for pairIdx, partonID in enumerate(pair):
                    f.write(partonIDFormat.format(beams[pairIdx] * partonID))
            f.write('\n')
        f.write('}}\n\n')

print('Successfully generated "' + steering_file_path + '". You should review it before use.')
