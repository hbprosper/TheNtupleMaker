#!/usr/bin/env python
#---------------------------------------------------------------------------
# File:        mkclassmap.py
# Description: Create a map of classnames to headers
# Created:     26-Aug-2010 Harrison B. Prosper
#              31-Mar-2011 HBP - include typedefs
#              23-Apr-2012 HBP - use import to load class map
#$Id: mkclassmap.py,v 1.24 2013/07/05 23:02:36 prosper Exp $
#---------------------------------------------------------------------------
import os, sys, re
from ROOT import *
from string import atof, atoi, replace, lower,\
	 upper, joinfields, split, strip, find, rfind
from time import sleep, ctime, time
from glob import glob
from getopt     import getopt, GetoptError
from PhysicsTools.TheNtupleMaker.Lib import \
	 parseHeader,\
	 splitHeader,\
	 stripBlanklines,\
	 namespaceName,\
	 getClassname,\
	 cmsswProject,\
	 fixName
#---------------------------------------------------------------------------
# Constants
#---------------------------------------------------------------------------
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
if PACKAGE == None:
	print "Please run me in your package directory"
	sys.exit(0)

SCRAM_ARCH  = os.environ['SCRAM_ARCH']
PROJECTBASE = "%s/%s/%s"   % (LOCALBASE, PACKAGE, SUBPACKAGE)
PYTHONDIR   = "%s/python"   % PROJECTBASE

shortOptions = "Hu:"

USAGE='''
Usage:
  mkclassmap.py [options] [package1 [package2...]]

  options
  -u<package-path>   Update classmap by scanning given package-path

  example:
      mkclassmap.py -u$CMSSW_BASE/src/MyArea/MyAnalysis

  default packages:
                AnalysisDataFormats/* 
                DataFormats/* 
				SimDataFormats/*
				FWCore/Framework
				FWCore/FWLite
				FWCore/MessageLogger
				FWCore/ParameterSet
				FWCore/Utilities
				FWCore/Common
				PhysicsTools/TheNtupleMaker
				<user-sub-system>/<user-package>
'''
def usage():
	print USAGE
	sys.exit(0)
#------------------------------------------------------------------------------
# Decode command line
argv = sys.argv[1:]
try:
	opts, pkgs = getopt(argv, shortOptions)
except GetoptError, m:
	print
	print m
	usage()

Update = False
PKGBASE = BASE # default: scan $CMSSW_RELEASE_BASE
CLASSMAPFILE = '%s/classmap.py' % PYTHONDIR
subpkgs = pkgs

for option, value in opts:
	if option == "-H":
		usage()
		
	elif option == "-u":
		Update = True
		subpkgs += split(value)
		if os.path.exists(CLASSMAPFILE):
			execfile(CLASSMAPFILE)
			PKGBASE = ""
		else:
			Update = False
			PKGBASE = LOCALBASE
			subpkgs += ["%s/%s", (PACKAGE, SUBPACKAGE)]

SUBPACKAGELIST = ["AnalysisDataFormats/*",
				  "DataFormats/*",
				  "SimDataFormats/*",
				  "FWCore/Framework",
				  "FWCore/Common",
				  "FWCore/Utilities",
				  "FWCore/FWLite",
				  "FWCore/MessageLogger",
				  "FWCore/ParameterSet",
				  "PhysicsTools/TheNtupleMaker"]

if len(subpkgs) > 0:
	SUBPACKAGELIST += subpkgs

userpackage = "%s/%s" % (PACKAGE, SUBPACKAGE)
if not userpackage in SUBPACKAGELIST:
	SUBPACKAGELIST.append(userpackage)
#----------------------------------------------------------------------------
# subsystems to ignore

skipsubsystem = re.compile('Alignment|'\
						   'Histograms|'\
						   'Road|'\
						   'StdDict|'\
						   'Streamer|'\
						   'TestObj|'\
						   'VZero|'\
						   'Wrapped')

skipheader = re.compile('(classes|print).h$')
stripnamespace = re.compile('^[a-zA-Z0-9]+::')
#----------------------------------------------------------------------------
def addToMap(fullkey, header, cmap):
	if not cmap.has_key(fullkey):
		cmap[fullkey] = header
#============================================================================
# Main Program
#============================================================================
def main():
	
	print "mkclassmap.py\n"

	subpackagelist = SUBPACKAGELIST
	filelist = []
	if Update:
		for subpackage in subpackagelist:
			print "scan package: %s" % subpackage
			file = "%s/interface/*.h" % subpackage
			hlist = glob(file)
			hlist.sort()
			filelist += hlist
	else:

		for subpackage in subpackagelist:
			package, subpkg = split(subpackage,'/')
			if subpkg == "*":
				cmd = "%s%s" % (BASE, subpackage)
				subsystems = filter(lambda x: os.path.isdir(x), glob(cmd))
				if len(subsystems) == 0:
					cmd = "%s%s" % (PKGBASE, subpackage)
					subsystems = filter(lambda x: os.path.isdir(x), glob(cmd))
					
				subsystems = map(lambda x: split(x, '/').pop(), subsystems)
			else:
				subsystems = [split(subpackage, '/').pop()]

			for subsystem in subsystems:
				if skipsubsystem.match(subsystem) != None: continue
				
				dirpath = "%s%s/%s" % (PKGBASE, package, subsystem)
				if not os.path.exists(dirpath):
					dirpath = "%s%s/%s" % (LOCALBASE, package, subsystem)
					if not os.path.exists(dirpath):
						"** directory %s not found" % dirpath
						continue
				print "scan package: %s/%s" % (package, subsystem)
				file = "%s/interface/*.h" % dirpath
				hlist = glob(file)
				hlist.sort()
				filelist += hlist

		
	# Filter headers
	filelist = filter(lambda x: skipheader.search(x) == None, filelist)
	print
	print "scan %d headers for potentially useful classes" % len(filelist)
	
	#-------------------------------------------------
	# Loop over header files to be scanned
	#-------------------------------------------------
	cmap = {}
	count = 0
	for index, file in enumerate(filelist):
		if not os.path.exists(file):
			print "** file %s not found" % file
			continue

		file = os.path.abspath(file)
		if index % 100 == 0: print index

		# Scan header and parse it for classes

		record, items = parseHeader(file)
	
		if record == '':
			print "** failed on %s" % file
			continue
		records = splitHeader(record)
		if len(records) == 0: continue

		# Now strip away path up to "/src/" in pathname of header
		
		header = file
		k = rfind(header, "/src/") # search from right
		if k > 0: header = header[k+5:]

		cache = []
		names = []
		for irecord, (record, group, start, end) in enumerate(records):
			#print "GROUP(%s)" % group
			
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

				# Check for uninstantiated templates and
				# create keys

				if find(fullname, '<') > -1:
					tplate = True
					fullkey = split(fullname, '<')[0]
				else:
					tplate = False
					fullkey = fullname					

				cache.append((fullkey, header))

				count += 1
				print "%5d\t%s" % (count, fullkey)

				# remember to reset
				names.pop()
				
			elif group in ["class", "structclass"]:
				classname, basenames, template = getClassname(record)
				names.append(classname)

		# Update map
		
		for fullkey, header in cache:
			if Update:
				addToMap(fullkey, header, ClassToHeaderMap)
			else:
				addToMap(fullkey, header, cmap)

	
	# Write out class to header map

	if Update:
		print "updating classmap.py..."
		hmap = ClassToHeaderMap
	else:
		print "creating classmap.py..."
		hmap = cmap


		
## 	# define lib areas

## 	LOCALLIBAREA = "%s/lib/%s/" % (LOCALBASE[:-5], SCRAM_ARCH)
## 	LIBAREA = "%s/lib/%s/" % (BASE[:-5], SCRAM_ARCH)

	getmodule = re.compile('(?<=src/).*(?=/interface)')
## 	librecs = []
	recs = []
	keys = hmap.keys()
	keys.sort()
	for key in keys:
		key = fixName(key) # remove unnecessary spaces
		
		value = hmap[key]
		if type(value) == type(""):
			recs.append("'%s': '%s'" % (key, value))
		else:
			recs.append("'%s': %s"   % (key, value))

## 		# find the shared library in which the class typeinfo resides

## 		module = split(value,'/interface/')[0]
## 		library = "lib%s" % replace(module, '/', '')

## 		found = False
## 		filename = "%s%s.so" % (LOCALLIBAREA, library)
		
## 		if not os.path.exists(filename):
## 			filename = "%s%s.so" % (LIBAREA, library)
## 			if not os.path.exists(filename):
## 				print "\twarning: library %s not found"
## 				continue

		# add lib to list
## 		librecs.append("'%s': '%s'" % (key, library))

	record = joinfields(recs,',\n')		
	outfile = CLASSMAPFILE
	out = open(outfile,'w')
	out.write('# Created: %s\n' % ctime(time()))
	out.write('# Version: %s\n' % VERSION)
	out.write("ClassToHeaderMap = {\\\n")
	out.write(record+'\n')
	out.write("}\n\n")

## 	record = joinfields(librecs,',\n')
## 	out.write("ClassToLibraryMap = {\\\n")
## 	out.write(record+'\n')
## 	out.write("}\n")
	out.close()
#---------------------------------------------------------------------------
main()










