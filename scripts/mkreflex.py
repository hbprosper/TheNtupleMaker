#!/usr/bin/env python
#---------------------------------------------------------------------------
# File:        mkreflex.py
# Description: Create files needed to make classes visible to Root Reflex
# Created:     28-Jul-2011 Harrison B. Prosper
#$Id: mkreflex.py,v 1.2 2013/07/05 21:01:54 prosper Exp $
#---------------------------------------------------------------------------
import os, sys, re
from ROOT import *
from string import atof, atoi, replace, lower,\
	 upper, joinfields, split, strip, find
from time import sleep, ctime
from getopt import getopt
from PhysicsTools.TheNtupleMaker.Lib import \
	 parseHeader,\
	 splitHeader,\
	 stripBlanklines,\
	 namespaceName,\
	 getClassname,\
	 convert2html,\
	 classMethods,\
	 classDataMembers,\
	 cmsswProject
#---------------------------------------------------------------------------
# Constants
#---------------------------------------------------------------------------
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
if PACKAGE == None:
	print "Please run mkdocs.py in your package directory"
	sys.exit(0)
	
print "Sub-System:     %s" % PACKAGE
print "package: %s" % SUBPACKAGE

PROJECTBASE = "%s%s/%s"   % (LOCALBASE, PACKAGE, SUBPACKAGE)
PLUGINDIR = "%s/plugins"   % PROJECTBASE  
SRCDIR    = "%s/src"       % PROJECTBASE
INCDIR    = "%s/interface" % PROJECTBASE
#------------------------------------------------------------------------------
# Load classmap.py
# First try local release
#cmd = 'find %s%s/%s -name "classmap.py"' % (LOCALBASE, PACKAGE, SUBPACKAGE)
#t = map(strip, os.popen(cmd).readlines())
t = []
if len(t) == 0:
	# try TheNtupleMaker
	cmd = 'find %sPhysicsTools/TheNtupleMaker -name "classmap.py"' % LOCALBASE
	t = map(strip, os.popen(cmd).readlines())
	if len(t) == 0:
		print "\n\t** unable to locate classmap.py"\
		  "\t** try running mkclassmap.py to create it"
		sys.exit(0)
CLASSMAPFILE = t[0]
try:
	execfile(CLASSMAPFILE)
except:
	print "\n\t** unable to load classmap.py"
	sys.exit(0)
#------------------------------------------------------------------------------
USAGE='''
Usage:
  mkreflex.py
'''
def usage():
	print USAGE
	sys.exit(0)
	
SHORTOPTIONS = 'hI:'
headers = re.compile('AnalysisDataFormats')

#----------------------------------------------------------------------------
# Load needed libraries
from PhysicsTools.TheNtupleMaker.AutoLoader import *
#gSystem.Load("libPhysicsToolsTheNtupleMaker")
#============================================================================
# Main Program
#  Read header list
#  for each header in list
#     decode header
#============================================================================
def main():

	print "mkreflex.py\n"
	
	fmap = {}
	for file in ClassToHeaderMap.values():
		if type(file) == type([]):
			for f in file:
				if headers.findall(f) != []:
					fmap[f] = 0
		else:
			if headers.findall(file) != []:
				fmap[file] = 0
	filelist = fmap.keys()
	filelist.sort()

	xmlrec = ''
	hdrrec = ''
	count = 0
	for index, filename in enumerate(filelist):

		# Create full pathname to header

		file = LOCALBASE + filename
		if not os.path.exists(file):
			file = BASE + filename
			if not os.path.exists(file):
				print "** file %s not found" % file
				continue
		file = os.path.abspath(file)

		# Scan header and parse it for classes
		
		record, items = parseHeader(file)
		
		if record == '': continue
		records = splitHeader(record)
		if len(records) == 0: continue

		# Now strip away path up to "/src/" in pathname of header
		
		header = file		
		k = rfind(header, "/src/") # search from right
		if k > 0: header = header[k+5:]

		##D
		print "\n%s" % header
		hdrrec += '#include "%s"\n' % header

		filestem = replace(header, 'interface/', '')
		filestem = split(filestem, '.h')[0]
		filestem = replace(filestem, '/', '.')

		# Initialize map to contain info about classes, methods & datamembers
		
		db = {'version':  VERSION,
			  'filestem': filestem,
			  'header':   header}

		names = []
		for irecord, (record, group, start, end) in enumerate(records):

			# Get actual record from items map

			key = strip(record)
			if items.has_key(key):
				record = items[key]
				if type(record) == type(()):
					record, extraRecord = record
			record = stripBlanklines(record)
			
			if group == "namespace":
				name = strip(namespaceName(record))
				if name != '': names.append(name)

			elif group == "endnamespace":
				if len(names) > 0: names.pop()
				
			elif group in ["endclass", "endstructclass"]:
				
				fullname = joinfields(names, "::")

				# For now ignore templates
				if find(fullname, '<') > -1:
					pass
				else:
					###D
					print "\t%s" % fullname

					xmlrec += '   <class name="%s"/>\n' % fullname
					
				if len(names) > 0: names.pop()
				
			elif group in ["class", "structclass"]:
				classname, basenames, template = getClassname(record)
				names.append(classname)


	record = '''#ifndef ANALYSISDATAFORMATS_H
#define ANALYSISDATAFORMATS_H
	
%s
#endif
	''' % hdrrec

	open("analysisDataFormats.h", "w").write(record)	
	open("analysisDataFormats.xml", "w").write(xmlrec)
#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
main()










