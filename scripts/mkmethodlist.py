#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Filename: mkmethodlist.py
# Description: For given class, make an exhaustive listing of methods
# Created:  10-Doc-2009 HBP
# Updated:  03-Oct-2020 HBP fixes for ADL project
#$Id: mkmethodlist.py,v 1.16 2013/07/05 21:01:54 prosper Exp $
#-----------------------------------------------------------------------------
import os, sys, re
from sys    import exit
from glob   import glob
from time import sleep, ctime
from PhysicsTools.TheNtupleMaker.Lib import \
	 nameonly,\
	 readMethods,\
	 cmsswProject,\
         simpletype, \
         simpletypewithvoid
#-----------------------------------------------------------------------------
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
if PACKAGE == None:
	sys.exit("Please run me in the test directory of your sub-package")
	
print "Package:     %s" % PACKAGE
print "Sub-package: %s" % SUBPACKAGE
#------------------------------------------------------------------------------
# Load classmap.py
# First try local release
cmd = 'find -L %s%s/%s -name "classmap.py"' % (LOCALBASE, PACKAGE, SUBPACKAGE)
t   = list(map(str.strip, os.popen(cmd).readlines()))
if len(t) == 0:
	# try TheNtupleMaker
	cmd = 'find -L %sPhysicsTools/TheNtupleMaker -name "classmap.py"' % \
                LOCALBASE
	t   = list(map(str.strip, os.popen(cmd).readlines()))
	if len(t) == 0:
		sys.exit("\n\t** unable to locate classmap.py"\
		         "\t** try running mkclassmap.py to create it")
CLASSMAPFILE = t[0]
try:
	execfile(CLASSMAPFILE)
except:
	sys.exit("\n\t** unable to load classmap.py")
#------------------------------------------------------------------------------
def usage():
	sys.exit('''
Usage:
    mkmethodlist.py <txt-file1> [txt-file2 ...]
				 ''')
#------------------------------------------------------------------------------
striparg   = re.compile(r'\bconst |\w+::|\&')
stripcolon = re.compile(r':')
striptemplatepars = re.compile(r'\<.*\>')
stripconst = re.compile(r'\bconst ')
striprtypeless = re.compile(r'\bconst |\*$|&$|Ref$')
skipmethod = re.compile(r'clone')

DEBUG3 = 0
#----------------------------------------------------------------------------
def expandMethod(filename, fname1, delim, methlist):

	header, classname, basenames, methodlist = readMethods(filename)

	# Ok now, get simple methods

	for rtype, name, atype, record in methodlist:

		if DEBUG3 > 3:
                        print("\texpandMethod RTYPE( %s, %s )" % (rtype, name))

		# Take care not to overwrite these variables

		if DEBUG3 > 2:
			print("\tRTYPE( %s ) METHOD( %s ) ARG( %s )" % \
			      (rtype, name, atype))

		# Keep methods/datamembers that return simple types

		if rtype == "void": continue
		if simpletype.match(rtype) == None: continue
		if skipmethod.match(name) !=  None: continue		
			
		# atype = None for data members
		if atype == None:
			# data member
			fname2 = name
		else:
			# function member
			arg = striparg.sub("", atype)
			if simpletype.match(arg) == None: continue
			if ( atype == "void" ):
				fname2 = "%s()"   % name
			else:
				fname2 = "%s(%s)" % (name, atype)

		# ok, this looks like a good method or data member
		method = "%12s  %s%s%s" % (rtype, fname1, delim, fname2)
		if DEBUG3 > 0:
                        print("\t\tCOMPOUND METHOD( %s )" % strip(method))
		methlist.append(method)
#----------------------------------------------------------------------------
def mkmethodlist(filename):

	header, classname, basenames, methodlist = readMethods(filename)
	if DEBUG3 > 0:
		print("\theader: %s\t\n\t\tclassname: %s" % (header, classname))

	# Return if this is a template class

	if str.find(classname, "<") > -1:
		if DEBUG3 > 0:
			print("\tskipping template class: %s" % classname)
		return 0
		
	# ------------------------------------------------------------
	# Process simple methods
	# ------------------------------------------------------------
	compoundmethodlist = []	
	methods = []
	for rtype, name, atype, record in methodlist:

		# Take care are not to overwrite these variables

		if DEBUG3 > 2:
			print("\n\tRTYPE( %s ) METHOD( %s ) ARG( %s )" % \
			      (rtype, name, atype))

		# Skip some methods
		if rtype == "void": continue		
		if skipmethod.match(name) != None: continue

		# atype = None for data members
		if atype == None:
			# data member
			fname1 = name
		else:
			# function member
			arg = striparg.sub("", atype)
			if simpletypewithvoid.match(arg) == None: continue
			if ( atype == "void" ):
				fname1 = "%s()"   % name
			else:
				fname1 = "%s(%s)" % (name, atype)

		# Check for pointer or edm::Ref
		ispointer = rtype[-1]  == "*"
		isRef     = rtype[-3:] == "Ref"
		if ispointer or isRef:
			delim = '->'
		else:
			delim = '.'

		# check return type
		
		if simpletype.match(rtype) == None:
			# compound method
			compoundmethodlist.append((rtype, fname1, delim))
		else:
			# simple return type
			method = "%12s  %s" % (rtype, fname1)
			methods.append(method)
	  	
			if DEBUG3 > 0:
				print("\t\tSIMPLE METHOD( %s )" % method)

	# ------------------------------------------------------------
	# Process compound methods
	# ------------------------------------------------------------
	if DEBUG3 > 0:
		print("\n\tPROCESS COMPLEX RETURN TYPES\n")

	for rtype, fname1, delim in compoundmethodlist:

		# Get header for this class from ClassToHeaderMap
		cname = striprtypeless.sub("", rtype)
		if not (cname in ClassToHeaderMap):
			if DEBUG3 > 2:
				print("\t\t** header for class %s NOT found" %\
                                      cname)
			continue

		# get file that lists this class's methods and data members
		headerfile = ClassToHeaderMap[cname]
		if type(headerfile) == type([]): headerfile = headerfile[0]
		
		filestem = str.replace(headerfile, 'interface/', '')
		filestem = str.split(filestem, '.h')[0]
		filestem = str.replace(filestem, '/', '.')
		cname    = str.split(cname,'::').pop()
		txtfilename = "txt/%s.%s.txt" % (filestem, cname)
		if not os.path.exists(txtfilename):
			if DEBUG3 > 0:
				print("\t\t*** file %s NOT found" % txtfilename)
			continue

		if DEBUG3 > 1:
			print("\t\tTXT FILE( %s )" % txtfilename)

		expandMethod(txtfilename, fname1, delim, methods)
					
	if len(methods) == 0: return 0
	
	print("processed: %s\t%d" % (classname, len(methods)))

	# Write a summary document for this class
	
	astr = '%s\n' % '\n '.join(methods)
	classname = stripcolon.sub("", classname)
	headerfilename   = striptemplatepars.sub("", classname)
	open("methods/%s.txt" % headerfilename, 'w').write(astr)
	return len(methods)
#----------------------------------------------------------------------------
def main():
	if not os.path.exists("txt"):
		sys.exit("*** txt directory not found."\
		         "\n*** try running mkdocs.py to create it")
		
	filelist = sys.argv[1:]
	if len(filelist) == 0:
		filelist = glob("txt/*.txt")
	filelist.sort()
	
	if len(filelist) == 0:
		Usage()

	os.system("mkdir -p methods")
	count = 0
	for filename in filelist:
		count += mkmethodlist(filename)
	print("\n\ttotal number of access methods: %d\n" % count)
#----------------------------------------------------------------------------
main()

