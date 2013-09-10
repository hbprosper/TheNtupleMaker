#!/usr/bin/env python
#------------------------------------------------------------------------------
# File: mixsigbkg.py
# Description: Mix signal and background files and normalize the data
# Created: 06-Dec-2005 Harrison B. Prosper
# Updated: 20-Oct-2006 HBP & Daekwang Kau
#          02-Apr-2008 HBP Adapt for Serban
#------------------------------------------------------------------------------
import os, sys
from string import *
from time import time, ctime
from random import shuffle
from getopt import getopt, GetoptError
from math import *
#------------------------------------------------------------------------------
USAGE = '''
Usage:
   mixsigbkg.py <options> filename

   options:
         -h   print this
         -s   signal text file [sig.dat]
         -b   background text file [bkg.dat]
         -v   variables text file [use all variables in input text file
                                   but skip event weight etc.]
         -N   number of events/file [5000]

         The output files will be:
         
                            <filename>.dat
                            <filename>.var
'''
SHORTOPTIONS = 'hs:b:v:N:'
COUNT = 5000
SKIPVARS = ['','eventweight','weight','entry','target','eventcode', 'process']

#------------------------------------------------------------------------------
def error(message):
    print "** %s" % message
    sys.exit(0)

def usage():
    print USAGE
    sys.exit(0)

def nameonly(x):
    return os.path.splitext(os.path.basename(x))[0]
#------------------------------------------------------------------------------
# Decode command line using getopt module
#------------------------------------------------------------------------------
def decodeCommandLine():
    try:
        options, inputs = getopt(sys.argv[1:], SHORTOPTIONS)
    except GetoptError, m:
        print
        print m
        usage()

    # Need an output file name
    
    if len(inputs) == 0: usage()

    outfile = inputs[0]
    
    # Set defaults, then parse input line

    sigfile = 'sig.dat'
    bkgfile = 'bkg.dat'
    varfile = ''
    count   = COUNT
    
    for option, value in options:
        if option == "-h":
            usage()

        elif option == "-s":
            sigfile = value
            
        elif option == "-b":
            bkgfile = value
            
        elif option == "-v":
            varfile = value
            
        elif option == "-N":
            count = atoi(value)

    #---------------------------------------------------
    # Check that input files exist
    #---------------------------------------------------
    if not os.path.exists(sigfile): error("Can't find %s" % sigfile)
    if not os.path.exists(bkgfile): error("Can't find %s" % bkgfile)
    return (sigfile, bkgfile, varfile, count, outfile)
#------------------------------------------------------------------------------
def writeEventSet(filename, column, data):
    print "\tWriting file %s" % filename
    recs = []
    for row in data:
        entry = split(row)[column]
        recs.append(atoi(entry))
    recs.sort()
    recs = map(lambda x: "%16d\n" % x, recs)
    open(filename,'w').writelines(recs)
    
#------------------------------------------------------------------------------
# MAIN Program
#------------------------------------------------------------------------------
def main():

    sigfile, bkgfile, varfile, count, outfile = decodeCommandLine()

    names = {}
    names['name'] = nameonly(outfile)

    # Read signal file and add Target column

    print "Reading signal file: %s" % sigfile
    sigrec = map(lambda x: rstrip(x), open(sigfile).readlines())
    header = sigrec[0] + '\ttarget'
    sigrec = sigrec[1:]
    sigrec = map(lambda x: x + '\t1', sigrec)

    # Read background file and add Target column

    print "Reading background file: %s" % bkgfile
    bkgrec = map(lambda x: rstrip(x), open(bkgfile).readlines())
    bkgrec = bkgrec[1:]
    bkgrec = map(lambda x: x + '\t0', bkgrec)

    # Make sure we don't ask for more than the number of
    # events in each file
    
    count = min(count, min(len(sigrec), len(bkgrec)))

    # Concatenate signal + background records and shuffle them

    print "Mixing signal and background events..."

    records = sigrec[:count] + bkgrec[:count]
    shuffle(records)
    records = map(lambda x: x + '\n', records)

    #---------------------------------------------------
    # Get column names from header and create name to
    # index map
    #---------------------------------------------------
    entrycol = -1
    colnames = split(header)
    colmap = {}
    for index in range(len(colnames)):
        name = colnames[index]
        colmap[name] = index + 1
        if name == 'entry':
            entrycol = index

    if entrycol > -1:
        
        # Write event numbers in training and test sets

        writeEventSet("sig.train", entrycol, sigrec[:count])
        writeEventSet("bkg.train", entrycol, bkgrec[:count])    
        writeEventSet("sig.test",  entrycol, sigrec[count:])
        writeEventSet("bkg.test",  entrycol, bkgrec[count:])

    #---------------------------------------------------
    # Get list of possible input variables
    #---------------------------------------------------
    if varfile == '':
        # Use all variables in input files
        var = colnames
    else:
        # Use variables given in variables file
        if not os.path.exists(varfile): error("Can't find %s" % varfile)
        var = map(strip, open(varfile).readlines())

    # Skip any blank lines and non-physics data variables
    var = filter(lambda x: not lower(x) in SKIPVARS, var)
    nvar= len(var)
    
    #---------------------------------------------------
    # Convert data to float
    #---------------------------------------------------
    data = map(lambda row:
               map(atof, split(row)), records)

    #---------------------------------------------------
    # Compute mean and sigma for each variable
    #---------------------------------------------------
    mean   = nvar * [0.0]
    sigma  = nvar * [0.0]
    count = 0
    for row in data:
        for index in range(nvar):
            name = var[index]
            x = row[colmap[name] - 1]
            mean[index] += x
            sigma[index] += x * x

    #---------------------------------------------------
    # Write means and sigmas to variables file
    #---------------------------------------------------
    print "Write %(name)s.var" % names
    out = open("%(name)s.var" % names,"w")
    ndata = len(data)
    for index in range(nvar):
        mean[index]  = mean[index] / ndata
        sigma[index] = sigma[index] / ndata
        sigma[index] = sqrt(sigma[index] - mean[index]**2)
        if sigma[index] == 0: error("variable %s has zero variance" % \
                                        var[index])
            
        record = "%-16s\t%10.3e\t%10.3e\n" % \
                 (var[index], mean[index], sigma[index])
        out.write(record)
    out.close()

    #---------------------------------------------------
    # Normalize the data
    #---------------------------------------------------
    print "Write %(name)s.dat" % names
    out = open("%(name)s.dat" % names,"w")
    format = nvar * "%16s"
    
    header = format % tuple(var) + "%16s" % 'target\n'
    out.write(header)

    recs = nvar * [0]
    count = 0
    for row in data:
        if count % 2000 == 0: print count
        count += 1

        for index in range(nvar):
            name = var[index]
            k = colmap[name]-1
            x = (row[k] - mean[index]) / sigma[index]
            recs[index] = '%16.4e' % x
        record = joinfields(recs,"") + '%16d\n' % row[colmap['target']-1]
        out.write(record)
    out.close()
#------------------------------------------------------------------------------
main()
