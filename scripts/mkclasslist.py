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
#          11-Dec-2014 HBP Adapt to CMSSW_7_2_3
#          18-May-2016 HBP Adapt to public data. if classlist_default.txt 
#                      exists in plugins, then use this list of classes. 
#$Id: mkclasslist.py,v 1.23 2013/07/05 21:01:54 prosper Exp $
#------------------------------------------------------------------------------
import os, sys, re
from ROOT import *
from string import atof, atoi, replace, lower,\
	 upper, joinfields, split, strip, find
from time import sleep, ctime
from glob import glob
from PhysicsTools.TheNtupleMaker.Lib import cmsswProject, fixName
from PhysicsTools.TheNtupleMaker.ReflexLib import findHeaders, getFullname
from PhysicsTools.TheNtupleMaker.classmap import ClassToHeaderMap
#------------------------------------------------------------------------------
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
PWD = os.environ['PWD']
if PACKAGE == None:
	print "\n\t** Must be run from package directory"
	sys.exit(0)
#------------------------------------------------------------------------------
# Extract getByLabel strings using a non-greedy search
getclass  = re.compile(r'(?<=edm::Wrapper\<).+(?=\> *")')
getfields = re.compile(r'(?<=")[^ ]*?(?=")')
isvector  = re.compile(r'(?<=^std::vector\<).*(?=\>)|'\
			       '(?<=^vector\<).*(?=\>)')
isvoid    = re.compile(r'\(void\)')
isint     = re.compile(r'\((unsigned )?(long |short )?int\)')
skip = re.compile('edm::helpers|edm::refhelper|TemplatedSecondary')
def skipme(rec):
	return len(skip.findall(rec))>0 
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
	if classname[-1] == "*": return None
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
edm::helpers
edm::refhelper
TemplatedSecondary
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

# more stuff to skip
skip = re.compile('edm::helpers|'\
			  'edm::refhelper|'\
			  'edm::DetSet|'\
			  'IPTagInfo|'\
			  'std::pair|'\
			  'Point.D|'\
			  'PattRecoTree|'\
			  'TemplatedSecondary')
def skipmore(rec):
	return len(skip.findall(rec))>0 
#------------------------------------------------------------------------------
argv = sys.argv[1:]
argc = len(argv)

# Construct list of directories to be searched from 
# the contents of classmap.py, but if classlist_default.txt
# exists, use the classes list therein

classlist_default = '%s/plugins/classlist_default.txt' % PWD
if os.path.exists(classlist_default):
	print "\n\t=== using classes in plugins/classlist_default.txt ===\n"
	wclasses = map(strip, open(classlist_default).readlines())
else:
	values = ClassToHeaderMap.values()
	values.sort()

	records = {}
	for value in values:
		t = split(value, "/interface/")
		records[t[0]] = 0
	records = records.keys()
	records.sort()

	# Get list of classes from classes_def*.xml
	# Assume that those bounded by Wrappers are the
	# ones that can potentially be stored in a Root file

	wclassesmap = {}
	count = 0
	for record in records:
		xmlfiles = glob("%s/%s/src/classes_def*.xml" % \
					(LOCALBASE, record))
		xmlfiles += glob("%s/%s/src/classes_def*.xml" % \
					 (BASE, record))
		for xmlfile in xmlfiles:
			recs = os.popen('grep "edm::Wrapper" %s' % \
						xmlfile).readlines()
			for rec in recs:
				classname = getClass(rec)
				if classname == None: continue
				if not wclassesmap.has_key(classname):
					wclassesmap[classname] = 1
					count += 1
	wclasses = wclassesmap.keys()

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

print "list of classes known to TNM:"
tname = {}
for classname in wclasses:

	# Find singletons and collections

	# Get actual name of classes
	fullname = getFullname(classname)
	fullname = fixName(fullname)

	# Skip a bunch of complicated stuff...

	if skipme.match(fullname) != None:
		continue
	if skipmore(fullname): 
		continue

	# Keep classes for which a header can be identified
	headers = findHeaders(fullname)
	if len(headers) == 0:
		#print "** could not identify header for class (%s)" % fullname
		continue	
	pkg = split(split(headers[0], "/interface")[0], '/src/')[-1]

	# Make sure that containers supersede singletons in the
	# following sense:
	# vector<A> is kept but not A if both are present

	# Since classname may be a vector, check it first before the fullname
	yes, name = isVector(classname)

	no = not yes
	if no:
		yes, name = isVector(fullname)
	if yes:
		ctype = 'collection'
		name = fixName(name)
		tname[name] = (pkg, name, ctype)

	# If this is not a collection modeled as a
	# STL vector, skip it : CHECK IF THIS WORKS
	elif find(classname, "SortedCollection") > -1:
		ctype = 'collection'
		name = fixName(name)
		tname[name] = (pkg, name, ctype)	
	else:
		# This is not a  vector type. keep it only
		# if "name" not yet in map
		name = classname
		if not tname.has_key(name):
			ctype = 'singleton'
			name  = fixName(name)
			tname[name] = (pkg, name, ctype)

# write out classes
values = tname.values()
values.sort()
cmap = {}
records = []
for index, (pkg, classname, ctype) in enumerate(values):
	if classname[-1] == "*": continue
	key =  '%s%s' % (pkg, classname)
	if cmap.has_key(key): continue
	cmap[key] = 1
	print "%4d %s\t%s\t%s" % (index+1, pkg, classname, ctype)
	records.append("%s\t%s\t%s\n" % (pkg, classname, ctype))
open("plugins/classlist.txt",'w').writelines(records)

