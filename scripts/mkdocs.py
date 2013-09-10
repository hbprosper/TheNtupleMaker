#!/usr/bin/env python
#---------------------------------------------------------------------------
# File:        mkdocs.py
# Description: Create html and txt files giving an exhaustive listing of
#              accessor methods
# Created:     23-Jan-2010 Harrison B. Prosper
# Updated:     15-Feb-2010 HBP, make it possible to be run anywhere
#              09-Mar-2010 HBP, add search of SimDataFormats
#              08-Aug-2010 HBP, fix search of user.h in TheNtupleMaker
#              26-Aug02919 HBP, get list of potential classes from
#                          python/classmap.py
#$Id: mkdocs.py,v 1.16 2011/05/07 18:39:14 prosper Exp $
#---------------------------------------------------------------------------
import os, sys, re
from ROOT import *
from string import *
from time import *
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
	print "Please run mkdocs.py in your sub-package directory"
	sys.exit(0)
	
print "Package:     %s" % PACKAGE
print "Sub-package: %s" % SUBPACKAGE

PROJECTBASE = "%s%s/%s"   % (LOCALBASE, PACKAGE, SUBPACKAGE)
PLUGINDIR = "%s/plugins"   % PROJECTBASE  
SRCDIR    = "%s/src"       % PROJECTBASE
INCDIR    = "%s/interface" % PROJECTBASE
#------------------------------------------------------------------------------
# Load classmap.py
# First try local release
cmd = 'find %s%s/%s -name "classmap.py"' % (LOCALBASE, PACKAGE, SUBPACKAGE)
t = map(strip, os.popen(cmd).readlines())
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
  mkdocs.py
'''
def usage():
	print USAGE
	sys.exit(0)
	
SHORTOPTIONS = 'hI:'
#----------------------------------------------------------------------------
# Load needed libraries
from PhysicsTools.TheNtupleMaker.AutoLoader import *
#gSystem.Load("libPhysicsToolsTheNtupleMaker")
#----------------------------------------------------------------------------
# Code to extract methods etc.
#----------------------------------------------------------------------------
BLACK     = '"black"'
DARKRED   = '"darkred"'
DARKGREEN = '"darkgreen"'
DARKBLUE  = '"darkblue"'
BLUE      = '"blue"'
#----------------------------------------------------------------------------
def color(c,s):
	return '<font color=%s>%s</font>' % (c,s)
#----------------------------------------------------------------------------
def boldcolor(c,s):
	return '<b><font color=%s>%s</font></b>' % (c,s)
#-----------------------------------------------------------------------------
def printMethods(db, out):
	classlist = db['classlist']
	for cdb in classlist:
		if not cdb.has_key('methods'): continue
		cname   = cdb['classname']
		methods = cdb['methods']
		out.write('\nAccessMethods: %s\n' % cname)
		for fullrtype, method in methods:
			out.write("  %s\n" % method)
#-----------------------------------------------------------------------------
def printDataMembers(db, out):
	classlist = db['classlist']
	for cdb in classlist:
		if not cdb.has_key('datamembers'): continue
		cname = cdb['classname']
		data  = cdb['datamembers']
		out.write('\nDataMembers:   %s\n' % cname)
		for t, datum in data:
			out.write("  %s\n" % datum)
#-----------------------------------------------------------------------------
def printHeader(db, out):
	cname    = db['classname']
	bname    = db['baseclassnames']
	header   = db['header']
	filestem = db['filestem']
	
	if cname == '':
		print "\t** %s not processed!" % header
		out.close()
		os.system("rm -rf txt/%s*" % filestem)
		sys.exit(0)

	# Write class name and its header file

	tab = 15*' '
	out.write('Class:         %s\n\n'  % cname)
	out.write('Header:        %s\n\n'  % header)

	# Write base class names
	
	if len(bname) > 1:
		delim = ''
		out.write('BaseClasses:   ')
		for i, fullname in enumerate(bname):
			out.write('%s%s\n' % (delim, fullname))
			delim = tab
			
		out.write("\n")
	out.write('Version:       %s\n' % db['version'])
	out.write('Created:       %s\tmkdocs.py\n' % ctime(time()))
#-----------------------------------------------------------------------------
def writeHTML(db, txtfilename):

	fullname = db['classname']
	filestem = db['filestem']

	record = open(txtfilename).read()
	record = convert2html(record)	
	cname  = split(split(fullname, '<')[0],'::').pop()

 	record = replace(record, 'Class:',
					 boldcolor(BLUE, 'Class:'))

 	record = replace(record, 'Header:',
					 boldcolor(BLUE, 'Header:'))
	
  	record = replace(record, 'BaseClasses:',
 					 boldcolor(BLUE, 'BaseClasses:'))

  	record = replace(record, 'AccessMethods:',
 					 boldcolor(DARKBLUE, 'AccessMethods:'))

  	record = replace(record, 'DataMembers:',
 					 boldcolor(DARKBLUE, 'DataMembers:'))

	htmlfile = "html/%s.%s.html" % (filestem, cname)
	out = open(htmlfile, 'w')
	out.write('<code>\n')
	out.write('\t<font size=+1 color=%s>\n' % BLACK)
	out.write(record)
	out.write('\t</font>\n')
	out.write('</code>\n')
	out.close()
#============================================================================
# Main Program
#  Read header list
#  for each header in list
#     decode header
#============================================================================
def main():

	print "mkdocs.py\n"
	
	fmap = {}
	for file in ClassToHeaderMap.values():
		if type(file) == type([]): continue
		fmap[file] = 0
	filelist = fmap.keys()
	filelist.sort()

	#-------------------------------------------------
	# Loop over header files to be scanned
	#-------------------------------------------------

	# Make sure html and txt directories exist
	
	os.system("mkdir -p html; mkdir -p txt")
	
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
				if find(fullname, '<') > -1: continue
				
				# Get methods and/or datamembers and write them out
				
				db['scopes'] = {}
				db['methods'] = {}
				db['datamembers'] = {}
				db['classname'] = fullname
				db['classlist'] = []
				db['baseclassnames'] = []
				db['signature'] = {}
				classMethods(fullname, db)
				db['baseclassnames'] = []
				classDataMembers(fullname, db)

				nmeth = len(db['methods'])
				ndata = len(db['datamembers'])
				
				if nmeth > 0 or ndata > 0:

					count += 1
					print "%5d\t%s" % (count, fullname)
					
					cname  = split(fullname,'::').pop()
					methfilename = "txt/%s.%s.txt" % (filestem, cname)

					out = open(methfilename, 'w')
					printHeader(db, out)
					printMethods(db, out)
					printDataMembers(db, out)
					out.close()
					
					writeHTML(db, methfilename)
				names.pop()
				
			elif group in ["class", "structclass"]:
				classname, basenames, template = getClassname(record)
				names.append(classname)

	# Create index.html

	print "\ncreating html/index.html.."
	os.system("mkindex.py")
	print "\n\tmkdocs.py is done!\n"
#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
main()










