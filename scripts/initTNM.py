#!/usr/bin/env python
#------------------------------------------------------------------------------
# initialize TheNtupleMaker:
#  1. scripts/mkclassmap.py [CMG]
#  2. scripts/mkclasslist.py
#  3. scripts/mkplugins.py
#
# Created 2 Apr 2012 Harrison B. Prosper
#------------------------------------------------------------------------------
import os, sys
from string import atof, atoi, replace, lower,\
	 upper, joinfields, split, strip, find

pwd = split(os.environ["PWD"],"/")[-1]
if pwd != "TheNtupleMaker":
        print "\n\t* Please run this script from the TheNtupleMaker directory:"
        
        print "\t    python scripts/initTNM.py [CMG]"
        print
        sys.exit(0)

# Make scripts executable

os.system("chmod +x scripts/*.py")

if not os.environ.has_key("CMSSW_BASE"):
        print "\t* Please remember \n\t\tcmsenv\n"
        sys.exit(0)

# Add link to TheNtupleMaker

base = os.environ["CMSSW_BASE"]
os.system('''
mkdir -p %s/python/PhysicsTools
rm -rf %s/python/PhysicsTools/TheNtupleMaker
ln -s %s/src/PhysicsTools/TheNtupleMaker/python %s/python/PhysicsTools/TheNtupleMaker
''' % (base, base, base,base))

# Check if CMGTools installed

if len(sys.argv) > 1:
        cmgtools = "%s/src/AnalysisDataFormats/CMGTools" % base
        if os.path.exists(cmgtools):
                withCMG = "AnalysisDataFormats/CMGTools"
                print "\n\t* Initialize TheNtupleMaker with CMGTools *\n"
        else:
                print "\t* CMGTools was not found in local release %s" % \
                      os.environ['CMSSW_VERSION']
                print "\t* Please install and re-run this script\n"
                sys.exit(0)
else:
        withCMG = ""
        print "\n\t* Initialize TheNtupleMaker *\n"

# Now execute initialization commands

cmd = "scripts/mkclassmap.py " + withCMG
print cmd
os.system(cmd)

cmd = "scripts/mkclasslist.py"
print cmd
os.system(cmd)

cmd = "scripts/mkplugins.py"
print cmd
os.system(cmd)



