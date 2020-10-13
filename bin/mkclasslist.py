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
#          29-Aug-2020 HBP adapt to Python 3 (for ADL project)
#          30-Sep-2020 HBP adapt for ADL
#------------------------------------------------------------------------------
import os, sys, re
import PhysicsTools.TheNtupleMaker.AutoLoader
import ROOT
from time import sleep, ctime
from glob import glob

from PhysicsTools.TheNtupleMaker.Lib import cmsswProject, \
        fixName, findHeaders, getFullname
from PhysicsTools.TheNtupleMaker.classmap import ClassToHeaderMap

#------------------------------------------------------------------------------
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
cwd = os.path.basename(os.environ['PWD'])
if PACKAGE == None:
	sys.exit("\n\t** Must be run from package directory")
#------------------------------------------------------------------------------
# Extract getByToken strings using a non-greedy search
getclass  = re.compile(r'(?<=edm::Wrapper\<).+(?=\> *")')
getfields = re.compile(r'(?<=")[^ ]*?(?=")')
isvector  = re.compile(r'(?<=^std::vector\<).*(?=\>)')
isvoid    = re.compile(r'\(void\)')
isint     = re.compile(r'\((unsigned )?(long |short )?int\)')
skip      = re.compile('edm::helpers|edm::refhelper|TemplatedSecondary')
def skipme(rec):
	return len(str.skip.findall(rec))>0 
#------------------------------------------------------------------------------
def getClass(rec):
	if str.find(rec, '<class name') < 0: return None

	# fix string before getting class
	rec = str.strip(rec)
	rec = str.replace(rec, "&lt;", "<")
	rec = str.replace(rec, "&gt;", ">")
	classname = getclass.findall(rec)
	if len(classname) == 0: return None
	classname = str.strip(classname[0])
	if classname[-1] == "*": return None
	return classname
#------------------------------------------------------------------------------
def isVector(fullname):
	t = isvector.findall(fullname)
	if len(t) > 0:
		return (True, str.strip(t[0]))
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
edm::helpers
edm::refhelper
TemplatedSecondary
'''
exlist = '|'.join(str.split(str.strip(exlist),'\n'))
# read classes to exclude
delim  = ''
exlistfile = "plugins/exclusionlist.txt"
if os.path.exists(exlistfile):
	exclusionlist = str.split(str.strip(open(exlistfile).read()), '\n')
	for x in exclusionlist:
		exlist += '|%s' % x
exlist = str.replace(exlist, "<", "\<") # need to "escape" some characters
exlist = str.replace(exlist, ">", "\>")
skipme = re.compile(exlist)

# read classes to include that are not included automatically
inclusionlist = []
inlistfile = "plugins/inclusionlist.txt"
if os.path.exists(inlistfile):
	inclusionlist = [str.strip(x) for x in 
                         str.split(str.strip(open(inlistfile).read()), '\n')]

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
# the contents of classmap.py

values = ClassToHeaderMap.values()
values.sort()
records = {}
for value in values:
	t = str.split(value, "/interface/")
	records[t[0]] = 0
records = records.keys()
records.sort()

# Get list of classes from classes_def*.xml
# Assume that those bounded by Wrappers are the
# ones that can potentially be stored in a Root file

wclassesmap = {}
count = 0
for record in records:
	xmlfiles = glob("%s/%s/src/classes_def*.xml" % (LOCALBASE, record))
	xmlfiles += glob("%s/%s/src/classes_def*.xml" % (BASE, record))
	for xmlfile in xmlfiles:
		recs = os.popen('grep "edm::Wrapper" %s' % xmlfile).readlines()
		for rec in recs:
			classname = getClass(rec)
			if classname == None: continue
			if not (classname in wclassesmap):
				wclassesmap[classname] = 1
				count += 1
				#print "\t%5d ==> %s" % (count, classname)
wclasses = wclassesmap.keys()
#print

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

print("getting list of classes known to TNM...")
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
		print("** could not identify header for class (%s)" % fullname)
		continue	
	pkg = str.split(str.split(headers[0], "/interface")[0], '/src/')[-1]

	# Make sure that containers supersede singletons in the
	# following sense:
	# vector<A> is kept but not A if both are present

	# Since classname may be a vector, check it first before the fullname
	yes, name = isVector(classname)
	if not yes:
		yes, name = isVector(fullname)
	if yes:
		ctype = 'collection'
		name = fixName(name)
		tname[name] = (pkg, name, ctype)

	# If this is not a collection modeled as a
	# STL vector, skip it
	elif str.find(classname, "Collection") > -1:
		continue
	
	else:
		# This is not a  vector type. keep it only
		# if "name" not yet in map
		name = classname
		if not tname.has_key(name):
			ctype = 'singleton'
			name  = fixName(name)
			tname[name] = (pkg, name, ctype)

        ###DB
        #print(tname[name])
# write out classes
values = tname.values()
values.sort()
cmap = {}
records = []
for index, (pkg, classname, ctype) in enumerate(values):
	if classname[-1] == "*": continue
	key =  '%s%s' % (pkg, classname)
	if key in cmap: continue
	cmap[key] = 1
	print("%4d %s\t%s\t%s" % (index+1, pkg, classname, ctype))
	records.append("%s\t%s\t%s\n" % (pkg, classname, ctype))
open("plugins/classlist.txt",'w').writelines(records)

