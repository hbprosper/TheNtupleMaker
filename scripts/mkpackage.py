#!/usr/bin/env python
#------------------------------------------------------------------------------
# Create the skeleton of a user package
# Created: 03-Sep-2010 Harrison B. Prosper
#$Id: mkpackage.py,v 1.20 2013/07/05 21:01:54 prosper Exp $
#------------------------------------------------------------------------------
import os, sys, re
from string import atof, atoi, replace, lower,\
	 upper, joinfields, split, strip, find
from time import sleep, ctime
from PhysicsTools.TheNtupleMaker.Lib import \
	 nameonly, getauthor, cmsswProject
#------------------------------------------------------------------------------
#               CMS jargon
# PACAKGE    -> SUBSYSTEM
# SUBPACKAGE -> PACKAGE
#
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
if PACKAGE == None:
	print " The CMS directory structure is"
	print ""
	print "      $CMSSW_BASE/src/<subsystem>/<package>"
	print ""
	print "First create a <subsystem> directory in $CMSSW_BASE/src:"
	print "      mkdir <subsystem>"
	print 
	print "Then:"
	print "      cd <subsystem>"
	print ""
	print "      mkpackage.py <package>"
	sys.exit(0)
	
print "Subsystem:     %s" % PACKAGE
AUTHOR = getauthor()
#------------------------------------------------------------------------------
def usage():
	print '''
Usage:
	mkpackage.py <package>
	'''
	sys.exit(0)
#------------------------------------------------------------------------------
def main():
	argv = sys.argv[1:]
	argc = len(argv)
	if argc < 1:
		usage()

	SUBPACKAGE = argv[0]
	print "Package: %s" % SUBPACKAGE
	print "Author:      %s" % AUTHOR
	
	names = {}
	names['pkg']    = PACKAGE
	names['subpkg'] = SUBPACKAGE
	names['prog']   = lower(PACKAGE) + lower(SUBPACKAGE)
	names['author'] = AUTHOR
	names['mkntuple'] = 'PhysicsTools/TheNtupleMaker'
	names['lib'] = 'PhysicsToolsTheNtupleMaker'
	
	cmd = '''
	mkdir -p %(subpkg)s
	cd %(subpkg)s
	sed -e "s/%(lib)s/%(pkg)s%(subpkg)s/g" $CMSSW_BASE/src/%(mkntuple)s/BuildFile.xml  > .b
	sed -e "s|/UtilAlgos>|/UtilAlgos>\\n<use name="%(mkntuple)s"/>|g" .b > BuildFile.xml
	rm -rf .b
	
	mkdir -p interface
	mkdir -p python
	mkdir -p src
	mkdir -p test
	mkdir -p bin
	
	cp $CMSSW_BASE/src/PhysicsTools/TheNtupleMaker/python/classmap.py python
	mkdir -p plugins
	grep "<use " $CMSSW_BASE/src/%(mkntuple)s/plugins/BuildFile.xml > plugins/BuildFile.xml
	echo -e "<use name=\\"%(pkg)s/%(subpkg)s\\"/>" >> plugins/BuildFile.xml	

	sed -e "s/PhysicsTools.TheNtupleMaker/%(pkg)s.%(subpkg)s/g" $CMSSW_BASE/src/%(mkntuple)s/TheNtupleMaker_cfg.py  >  %(subpkg)s_cfg.py

	grep "<use " $CMSSW_BASE/src/%(mkntuple)s/bin/BuildFile.xml > bin/BuildFile.xml
	echo -e "<bin name=\\"%(prog)s\\" file=\\"%(prog)s.cc\\">" >> bin/BuildFile.xml
	echo -e "</bin>" >> bin/BuildFile.xml
 	echo "int main(int argc, char** argv) {return 0;}" > bin/%(prog)s.cc
	''' % names
	os.system(cmd)
#------------------------------------------------------------------------------
main()
