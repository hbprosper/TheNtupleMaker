#!/usr/bin/env python
#------------------------------------------------------------------------------
# initialize TheNtupleMaker:
#  1. scripts/mkclassmap.py [CMG]
#  2. scripts/mkclasslist.py
#
# Created  2 Apr 2012 Harrison B. Prosper
#         24 Jan 2014 HBP: call scram set lhapdffull
#         30 Sep 2020 HBP; remove setting of lhapdfull since it no longer 
#                     exists  
#------------------------------------------------------------------------------
import os, sys
#------------------------------------------------------------------------------
pwd = str.split(os.environ["PWD"],"/")[-1]
if pwd != "TheNtupleMaker":
        print("\n\t* Please run this script from the TheNtupleMaker directory")
        sys.exit("\t    python scripts/initTNM.py [CMG]")

# Make scripts executable

os.system("chmod +x scripts/*.py")

if "CMSSW_BASE" not in os.environ:
        sys.exit("\t* Please remember \n\t\tcmsenv\n")

# Add link to TheNtupleMaker

base = os.environ["CMSSW_BASE"]
os.system('''
mkdir -p %s/python/PhysicsTools
rm -rf %s/python/PhysicsTools/TheNtupleMaker
ln -s %s/src/PhysicsTools/TheNtupleMaker/python %s/python/PhysicsTools/TheNtupleMaker
''' % (base, base, base, base))

# Check if CMGTools installed

if len(sys.argv) > 1:
        cmgtools = "%s/src/AnalysisDataFormats/CMGTools" % base
        if os.path.exists(cmgtools):
                withCMG = "AnalysisDataFormats/CMGTools"
                print("\n\t* Initialize TheNtupleMaker with CMGTools *\n")
        else:
                print("\t* CMGTools was not found in local release %s" % \
                      os.environ['CMSSW_VERSION'])
                sys.exit("\t* Please install and re-run this script\n")
else:
        withCMG = ""
        print("\n\t* Initialize TheNtupleMaker *\n")

# Now execute initialization commands

cmd = "bin/mkclassmap.py " + withCMG
print(cmd)
os.system(cmd)

cmd = "bin/mkclasslist.py"
print(cmd)
os.system(cmd)




