#!/usr/bin/env python
#---------------------------------------------------------------------------
# File:        findhdr.py
# Description: find header for given class using classmap.py
# Created:     04-Sep-2010 Harrison B. Prosper
#---------------------------------------------------------------------------
import os, sys, re
from ROOT import *
from string import *
from time import *
from glob import glob
from getopt     import getopt, GetoptError
from PhysicsTools.TheNtupleMaker.Lib import cmsswProject
#---------------------------------------------------------------------------
# Constants
#-----------------------------------------------------------------------------
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
USAGE='''
Usage:
  findhdr.py <full-classname>

  uses classmap.py (created using mkclassmap.py)
'''
def usage():
	print USAGE
	sys.exit(0)
#------------------------------------------------------------------------------
# Load classmap.py
#------------------------------------------------------------------------------
# First try local release
t = []
searchdir = LOCALBASE
if PACKAGE != None:
	searchdir += "/%s" % PACKAGE
	if SUBPACKAGE != None:
		searchdir += "/%s" % SUBPACKAGE
		
cmd = 'find %s -name "classmap.py"' % searchdir
t = map(strip, os.popen(cmd).readlines())

# If that fails, use the classmap.py in TheNtupleMaker
if len(t) == 0:
	cmd = 'find %s/PhysicsTools/TheNtupleMaker -name "classmap.py"' % LOCALBASE
	t = map(strip, os.popen(cmd).readlines())
	if len(t) == 0:
		print "\n\t** unable to locate classmap.py"\
		  "\t** try running mkclassmap.py to create it"
		sys.exit(0)

mapfile = t[0]
try:
	execfile(mapfile)
except:
	print "\n\t** unable to load classmap.py"
	sys.exit(0)
#------------------------------------------------------------------------------
def main():
	argv = sys.argv[1:]
	if len(argv) < 1:
		usage()
	classname = argv[0]
	if not ClassToHeaderMap.has_key(classname):
		return
	else:
		if classname[-1] != "*":			
			print "\t%s" % ClassToHeaderMap[classname]
		else:
			for x in ClassToHeaderMap[classname]:
				print "\t%s" % x
#------------------------------------------------------------------------------
main()










