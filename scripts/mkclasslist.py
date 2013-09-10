#!/usr/bin/env python
#------------------------------------------------------------------------------
# create classlist.txt
# Created: 05-Jan-2010 Harrison B. Prosper
# Updated: 08-Aug-2010 HBP minor fix before release
#          25-Aug-2010 HBP add a few more classes (by hand)
#          31-Mar-2012 HBP use directories defined by classmap.py
#                      simplify classes.txt format to one class per line
#          15-Apr-2012 HBP use exclusionlist.txt to exclude specific
#                      classes
#          18-Apr-2012 HBP use inclusionlist.txt to include specific
#                      classes not picked up automatically
#$Id: mkclasslist.py,v 1.23 2013/07/05 21:01:54 prosper Exp $
#------------------------------------------------------------------------------
import os, sys, re
from ROOT import *
from string import atof, atoi, replace, lower,\
	 upper, joinfields, split, strip, find
from time import sleep, ctime
from PhysicsTools.TheNtupleMaker.Lib import cmsswProject, fixName
from PhysicsTools.TheNtupleMaker.ReflexLib import findHeaders, getFullname
from PhysicsTools.TheNtupleMaker.classmap import ClassToHeaderMap
#------------------------------------------------------------------------------
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
cwd = os.path.basename(os.environ['PWD'])
if PACKAGE == None:
	print "\n\t** Must be run from package directory"
	sys.exit(0)
#------------------------------------------------------------------------------
# Extract getByLabel strings using a non-greedy search
getclass  = re.compile(r'(?<=edm::Wrapper\<).+(?=\> *")')
getfields = re.compile(r'(?<=")[^ ]*?(?=")')
isvector  = re.compile(r'(?<=^std::vector\<).*(?=\>)')
isvoid    = re.compile(r'\(void\)')
isint     = re.compile(r'\((unsigned )?(long |short )?int\)')
#------------------------------------------------------------------------------
def getClass(rec):
	if find(rec, '<class name') < 0: return None

	# fix string before getting class
	rec = strip(rec)
	rec = replace(rec, "&lt;", "<")
	rec = replace(rec, "&gt;", ">")
	classname = getclass.findall(rec)
	if len(classname) == 0: return None
	classname = strip(classname[0])
	if classname == "*": return None
	return classname
#------------------------------------------------------------------------------
def isVector(fullname):
	t = isvector.findall(fullname)
	if len(t) > 0:
		return (True, strip(t[0]))
	else:
		return (False, fullname)
#------------------------------------------------------------------------------
# Exclude a bunch of complicated stuff
exlist = '''
edmNew
RefVector
PtrVector
Collection
edm::Ptr
edm::Ref
edm::FwdRef
edm::PtrVector
edm::SortedCollection
edm::EDCollection
edm::DetSet
edm::AssociationMap
edm::Association
edm::ValueMap
edm::RangeMap
edm::OwnVector
edm::Lazy
std::vector<edm::Fwd
std::vector<edm::Ref
std::vector<edm::Ptr
std::pair
std::map
std::set
'''
exlist = joinfields(split(strip(exlist),'\n'),'|')
# read classes to exclude
delim  = ''
exlistfile = "plugins/exclusionlist.txt"
if os.path.exists(exlistfile):
	exclusionlist = split(strip(open(exlistfile).read()), '\n')
	for x in exclusionlist:
		exlist += '|%s' % x
exlist = replace(exlist, "<", "\<") # need to "escape" some characters
exlist = replace(exlist, ">", "\>")
skipme = re.compile(exlist)

# read classes to include that are not included automatically
inclusionlist = []
inlistfile = "plugins/inclusionlist.txt"
if os.path.exists(inlistfile):
	inclusionlist = map(strip, split(strip(open(inlistfile).read()), '\n'))
#------------------------------------------------------------------------------
argv = sys.argv[1:]
argc = len(argv)

# Construct list of directories to be searched from 
# the contents of classmap.py

values = ClassToHeaderMap.values()
values.sort()
records = {}
for value in values:
	t = split(value, "/interface/")
	records[t[0]] = 0
records = records.keys()
records.sort()

# Get list of classes from classes_def.xml
# Assume that those bounded by Wrappers are the
# ones that can potentially be stored in a Root file

wclasses = []
for record in records:
	xmlfile = "%s/%s/src/classes_def.xml" % (LOCALBASE, record)
	if not os.path.exists(xmlfile):
		xmlfile = "%s/%s/src/classes_def.xml" % (BASE, record)
		if not os.path.exists(xmlfile): continue

	recs = os.popen('grep "edm::Wrapper" %s' % xmlfile).readlines()
	for rec in recs:
		classname = getClass(rec)
		if classname == None: continue
		wclasses.append(classname)
		
# ---------------------------------------
# Add classes listed in inclusionlist.txt
# ---------------------------------------
for x in inclusionlist:
	if not x in wclasses:
		wclasses.append(x)
wclasses.sort()
#------------------------------------------------------------------------------
# Get list of classes
#from PhysicsTools.TheNtupleMaker.AutoLoader import *

print "\t==> writing plugins/classlist.txt ..."
tname = {}
for classname in wclasses:

	# Find singletons and collections

	# Get actual name of classes
	fullname = getFullname(classname)
	fullname = fixName(fullname)
	
	# Skip a bunch of complicated stuff...

	if skipme.match(fullname) != None:
		continue

	# Keep classes for which a header can be identified
	headers = findHeaders(fullname)
	if len(headers) == 0:
		#print "** could not identify header for class (%s)" % fullname
		continue
	
	# Make sure that containers supersede singletons in the
	# following sense:
	# vector<A> is kept but not A if both are present

	# Since classname may be a vector, check it first before the fullname
	yes, name = isVector(classname)
	if not yes:
		yes, name = isVector(fullname)
	if yes:
		# add to map
		ctype = 'collection'
		name = fixName(name)
		tname[name] = (ctype, name)

	# If this is not a collection modeled as a
	# STL vector, skip it
	elif find(classname, "Collection") > -1:
		continue
	
	else:
		# This is not a  vector type. Keep it only if
		# "name" is not present in the map.
		name = classname
		if not tname.has_key(name):
			# add to map
			ctype = 'singleton'
			name = fixName(name)
			tname[name] = (ctype, name)

# write out classes
values = tname.values()
values.sort()

records = []
for index, (ctype, classname) in enumerate(values):
	print "%4d %s\t%s" % (index+1, ctype, classname)
	records.append("%s\t%s\n" % (ctype, classname))
print "\t==> number of classes: %d" % len(records) 
open("plugins/classlist.txt",'w').writelines(records)

