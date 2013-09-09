#!/usr/bin/env python
#------------------------------------------------------------------------------
# Create the skeleton of a user plugin
# Created: 27-Aug-2010 Harrison B. Prosper
#          22-Jul-2011 HBP - fix duplicate HelperFor bug
#          22-Apr-2012 HBP - use SINGLETON and COLLECTION keywords
#          03-May-2012 HBP - add methods automatically
#
#$Id: mkhelper.py,v 1.14 2013/07/05 21:01:54 prosper Exp $
#------------------------------------------------------------------------------
import os, sys, re
from string import atof, atoi, replace, lower,\
	 upper, joinfields, split, strip, find
from time import sleep, ctime
from glob import glob
from getopt     import getopt, GetoptError

from PhysicsTools.TheNtupleMaker.Lib import \
	 nameonly, \
	 cmsswProject, \
	 getauthor

from PhysicsTools.TheNtupleMaker.ReflexLib import \
         classMethods,\
         classDataMembers,\
         isFuntype
#------------------------------------------------------------------------------
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
if PACKAGE == None:
	print "Please run me in a package directory:"
	print "  $CMSSW_BASE/src/<subsystem>/<package>"
	sys.exit(0)
if SUBPACKAGE == None:
	print "Please run me in a package directory"
	print "  $CMSSW_BASE/src/<subsystem>/<package>"
	sys.exit(0)
#------------------------------------------------------------------------------
print "Subsystem: %s" % PACKAGE
print "Package:   %s" % SUBPACKAGE

PROJECTBASE = "%s/%s/%s"   % (LOCALBASE, PACKAGE, SUBPACKAGE)
PLUGINDIR = "%s/plugins"   % PROJECTBASE  
SRCDIR    = "%s/src"       % PROJECTBASE
INCDIR    = "%s/interface" % PROJECTBASE
AUTHOR    = getauthor()
#------------------------------------------------------------------------------
# Make sure these directories exist
if not os.path.exists(PLUGINDIR):
	print "\t** directory:\n\t** %s not found!" % PLUGINDIR
	sys.exit(0)
if not os.path.exists(SRCDIR):
	print "\t** directory:\n\t** %s not found!" % SRCDIR
	sys.exit(0)
if not os.path.exists(INCDIR):
	print "\t** directory:\n\t** %s not found!" % INCDIR
	sys.exit(0)
#------------------------------------------------------------------------------
# Load classmap.py
#------------------------------------------------------------------------------
# First try local release
cmd = 'find %s/%s/%s -name "classmap.py"' % (LOCALBASE, PACKAGE, SUBPACKAGE)
t = map(strip, os.popen(cmd).readlines())
if len(t) == 0:
	# try TheNtupleMaker
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
namespace   = re.compile(r'^[a-zA-Z]+::')
doublecolon = re.compile(r'::')
istemplate  = re.compile(r'(?<=\<).+(?=\>)')
stufftoskip = re.compile(r'clone|Clone|print|iterator')
signature   = re.compile(r'[a-zA-Z]+[\w]*\(.*\)')
arguments   = re.compile(r'(?<=\w\().*(?=\))')
constptr    = re.compile(r'^const .*\*$')
#------------------------------------------------------------------------------
shortOptions = "u"
def usage():
	print '''
Usage:
	mkhelper.py [options] <CMSSW class> s|c [postfix, default=Helper]

	s = singleton  (at most one  object  per event)
	c = collection (zero or more objects per event)

	options
	-u    undo effect of most recent call to mkhelper.py (experimental!)
	'''
	sys.exit(0)
#------------------------------------------------------------------------------
def wrpluginheader(names):
	template_header = '''#ifndef %(headername)s_H
#define %(headername)s_H
//-----------------------------------------------------------------------------
// Subsystem:   %(package)s
// Package:     %(subpackage)s
// Description: TheNtupleMaker helper class for %(classname)s
// Created:     %(time)s
// Author:      %(author)s      
//-----------------------------------------------------------------------------
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include "PhysicsTools/TheNtupleMaker/interface/HelperFor.h"
%(header)s
//-----------------------------------------------------------------------------
// Definitions:
//   helper:        object of class %(name)s
//   helped object: object of class %(classname)s
//
//
// The following variables are automatically defined and available to
//       all methods:
//
//         blockname          name of config. buffer object (config block) 
//         buffername         name of buffer in config block
//         labelname          name of label in config block (for getByLabel)
//         parameter          parameter (as key, value pairs)
//                            accessed as in the following example:
//
//                            string param = parameter("label");
//
//         0. hltconfig       pointer to HLTConfigProvider
//                            Note: this will be zero if HLTConfigProvider
//                                  has not been properly initialized
//
//         1. config          pointer to global ParameterSet object
//         2. event           pointer to the current event
//         3. object          pointer to the current helped object
//         4. oindex          index of current helped object
//
//         5. index           index of item(s) returned by helper.
//                            Note 1: an item is associated with all
//                                    helper methods (think of it as an
//                                    extension of the helped object)
//                                  
//                            Note 2: index may differ from oindex if,
//                                    for a given helped object, the count
//                                    variable (see below) differs from 1.
//
//         6. count           number of items per helped object (default=1)
//                            Note:
//                                  count = 0  ==> current helped object is
//                                                 to be skipped
//
//                                  count = 1  ==> current helped object is
//                                                 to be kept
//
//                                  count > 1  ==> current helped object is
//                                                 associated with "count"
//                                                 items, where each item
//                                                 is associated with all the
//                                                 helper methods
//
//       variables 0-6 are initialized by TheNtupleMaker.
//       variables 0-5 should not be changed.
//       variable    6 can be changed by the helper to control whether a
//                     helped object should be kept or generates more items
//-----------------------------------------------------------------------------
'''
	template_nonamespace = '''
/// A helper for %(classname)s.
class %(name)s : public HelperFor<%(classname)s>
{
public:
  ///
  %(name)s();

  virtual ~%(name)s();

  // Uncomment if helper class does some event-level analysis
  // virtual void analyzeEvent();

  // Uncomment if helper class does some object-level analysis
  // virtual void analyzeObject();

  // ---------------------------------------------------------
  // -- User access methods go here
  // ---------------------------------------------------------


private:
  // -- User internals


public:
  // ---------------------------------------------------------
  // -- Access Methods
  // ---------------------------------------------------------

  // WARNING: some methods may fail to compile because of coding
  //          problems in one of the CMSSW base classes. If so,
  //          just comment out the offending method and try again.
  
  %(methods)s
};
#endif
'''
	template_withnamespace = '''
namespace %(namespace)s
{
  /// A helper class for %(classname)s.
  class %(name)s : public HelperFor<%(classname)s>
  {
  public:
	///
	%(name)s();

	virtual ~%(name)s();

	// Uncomment if this class does some event-level analysis
	// virtual void analyzeEvent();
	 
	// Uncomment if this class does some object-level analysis
	// virtual void analyzeObject();

	// ---------------------------------------------------------
	// -- User access methods go here
	// ---------------------------------------------------------

	
  private:
    // -- User internals


  public:
    // ---------------------------------------------------------
    // -- Access Methods
    // ---------------------------------------------------------

	// WARNING: some methods may fail to compile because of coding
	//          problems in one of the CMSSW base classes. If so,
	//          just comment out the offending method and try again.
  
%(methods)s
  };
}
#endif
'''
	filename = "%(incdir)s/%(filename)s.h" % names
	if names['namespace'] == '':
		record = template_header + template_nonamespace
	else:
		record = template_header + template_withnamespace
	record = record % names
		
	out  = open(filename, "w")
	out.write(record)
	out.close()
	undofile = "%(incdir)s/.undo.%(filename)s.h" % names
	open(undofile,'w').write('\n')
#------------------------------------------------------------------------------
def wrplugincode(names):
	template = '''//-----------------------------------------------------------------------------
// Subsystem:   %(package)s
// Package:     %(subpackage)s
// Description: TheNtupleMaker helper class for %(classname)s
// Created:     %(time)s
// Author:      %(author)s      
//-----------------------------------------------------------------------------
#include "%(package)s/%(subpackage)s/interface/%(filename)s.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
//-----------------------------------------------------------------------------
using namespace std;
%(usingnamespace)s
//-----------------------------------------------------------------------------
// This constructor is called once per job
%(name)s::%(name)s()
  : HelperFor<%(classname)s>() {}
    
%(name)s::~%(name)s() {}

// -- Called once per event
//void %(name)s::analyzeEvent()
//{
//
//}

// -- Called once per object
//void %(name)s::analyzeObject()
//{
//
//}

// -- User access methods
//double %(name)s::someMethod()  const
//{
//  return  //-- some-value --
//}
''' % names

	
	filename = "%(srcdir)s/%(filename)s.cc" % names
		
	out  = open(filename, "w")
	out.write(template)
	out.close()
	undofile = "%(srcdir)s/.undo.%(filename)s.cc" % names
	open(undofile,'w').write('\n')
#------------------------------------------------------------------------------
def wrplugin(names):	
	template = '''// ----------------------------------------------------------------------------
// Created: %(time)s by mkhelper.py
// Author:  %(author)s      
// ----------------------------------------------------------------------------
#include "PhysicsTools/TheNtupleMaker/interface/UserBuffer.h"
#include "PhysicsTools/TheNtupleMaker/interface/pluginfactory.h"
#include "%(package)s/%(subpackage)s/interface/%(filename)s.h"
typedef UserBuffer<%(classname)s, %(fullname)s, %(ctype)s>
%(buffername)s_t;
DEFINE_EDM_PLUGIN(BufferFactory, %(buffername)s_t,
                  "%(buffername)s");\n''' % names

	undofile = "%(plugindir)s/.undo.userplugin_%(filename)s.cc" % names
	
	open(undofile,'w').write('\n')
	
	filename = "%(plugindir)s/userplugin_%(filename)s.cc" % names
	out  = open(filename, "w")
	out.write(template)
	out.close()
#------------------------------------------------------------------------------
def undo():
## 	print "\n\t\t*** UNDO is broken! Will be fixed soon...."
## 	print "\n\t\t*** Alas you must clean up your own mess!"
## 	sys.exit(0)
	names = {}
	names['subpackage'] = SUBPACKAGE
	names['plugindir']  = PLUGINDIR
	names['srcdir']     = SRCDIR
	names['incdir']     = INCDIR

	# undo userplugin
	t = glob("%(plugindir)s/.undo.userplugin_*.cc" % names)
	if len(t) > 0:
		undofile = replace(t[0], '.undo.', '')
		print "delete %s" % undofile
		os.system('rm -rf %s; rm -rf %s' % (undofile, t[0]))

	# undo plugins/BuildFile.xml
	t = glob("%(plugindir)s/.undo.BuildFile.xml" % names)
	if len(t) > 0:
		undofile = replace(t[0], '.undo.', '')
		print "restore %s" % undofile
		os.system('mv %s %s' % (t[0], undofile))

	# undo src/classes.h
	t = glob("%(srcdir)s/.undo.classes.h" % names)
	if len(t) > 0:
		undofile = replace(t[0], '.undo.', '')
		print "restore %s" % undofile
		os.system('mv %s %s' % (t[0], undofile))

	# undo src/classes_def.xml
	t = glob("%(srcdir)s/.undo.classes_def.xml" % names)
	if len(t) > 0:
		undofile = replace(t[0], '.undo.', '')
		print "restore %s" % undofile
		os.system('mv %s %s' % (t[0], undofile))
				
	# undo user plugin source
	t = glob("%(srcdir)s/.undo.*.cc" % names)
	if len(t) > 0:
		undofile = replace(t[0], '.undo.', '')
		print "delete %s" % undofile
		os.system('rm -rf %s; rm -rf %s' % (undofile, t[0]))

	# undo user plugin source
	t = glob("%(srcdir)s/.undo.*.cc" % names)
	if len(t) > 0:
		undofile = replace(t[0], '.undo.', '')
		print "restore %s" % undofile
		os.system('mv %s %s' % (t[0], undofile))

	# undo user plugin header
	t = glob("%(incdir)s/.undo.*.h" % names)
	if len(t) > 0:
		undofile = replace(t[0], '.undo.', '')
		print "delete %s" % undofile
		os.system('rm -rf %s; rm -rf %s' % (undofile, t[0]))

	# undo user plugin header
	t = glob("%(incdir)s/.undo.*.h" % names)
	if len(t) > 0:
		redofile = replace(t[0], '.undo.', '')
		print "restore %s" % undofile
		os.system('mv %s %s' % (t[0], undofile))

	sys.exit(0)
#------------------------------------------------------------------------------
def main():
	# Decode command line
	argv = sys.argv[1:]
	try:
		opts, vars = getopt(argv, shortOptions)
	except GetoptError, m:
		print
		print m
		usage()

	for option, value in opts:
		if option == "-H":
			usage()
		elif option == "-u":
			undo()

	# No options, check other arguments
	argc = len(argv)
	if argc < 2:
		usage()

	classname = argv[0]
	classtype = classname
	template  = ""
	
	# Check for templates
	t = istemplate.findall(classname)
	if len(t) > 0:
		template  = split(classname, '<')[0]
		classtype = t[0]

	#print "CLASS( %s ) TEMPLATE( %s )" % (classname, template)
	
	ctype = argv[1]
	if len(ctype) != 1:
		usage()

	if classname == "edm::Event":
		ctype = "s"
	elif not ctype in ['s','c', 'S', 'C']:
		usage()
	
	if argc > 2:
		postfix = argv[2]
	else:
		postfix = "Helper"

	# Find header associated with class
	if ClassToHeaderMap.has_key(classtype):
		header = ClassToHeaderMap[classtype]
	else:
		header = raw_input('Enter header for class "%s" starting at src: ' %
						   classtype)
		if header == "": sys.exit(0)
	if type(header) == type([]): header = header[0]
	
	# Check that header exists
	filename = "%s%s" % (LOCALBASE, header)
	if not os.path.exists(filename):
		filename = "%s%s" % (BASE, header)
		if not os.path.exists(filename):
			print "\t** can't find %s" % filename
			sys.exit(0)
		
	# Objects of this type can be singletons or form collections
	if ctype in ['s', 'S']:
		ctype = "SINGLETON"
	else:
		ctype = "COLLECTION"

	# if template is edm::ValueMap append name of class
	if template != "":
		template = split(template, "::")[-1]
		postfix  = template + postfix
		
	# Create names of helper class and associated buffer plugin
	nspace = namespace.findall(classtype)
	if len(nspace) == 0:
		nspace = ""
		nspacewithcolon = ""
		tab = ' '*2
	else:
		nspacewithcolon = nspace[0]
		nspace = doublecolon.sub("", nspacewithcolon)
		tab = ' '*4
		
	name  = namespace.sub("",classtype) + postfix      # remove namespace
	bname = doublecolon.sub("", classtype) + postfix   # remove "::"
	filename = doublecolon.sub("", bname)
	if nspacewithcolon != "":
		fullname = nspacewithcolon + name
	else:
		fullname = name

	# ----------------------------------------------------------------
	# Get class methods
	# ----------------------------------------------------------------
	db = {}
	classMethods(classtype, db)
	#db['baseclassnames'] = []
	#classDataMembers(classtype, db)

	# sort methods
	meths = {}
	for method, value in db['methods'].items():
		nom, clname, rtype, fullrtype, signature, methodcall, simpleArgs=value
		meths[lower(signature)] = (method, rtype,
								   fullrtype,
								   signature, methodcall, clname, simpleArgs)
	sigs = meths.keys()
	sigs.sort()

	methods = ''
	for key in sigs:
		method, rtype, fullrtype, signature, \
				methodcall, clname, simpleArgs = meths[key]
		#print "%s\t%s | %s" % (rtype, signature, methodcall)	
		
		if stufftoskip.search(method) != None: continue
		method = strip(method)

		if simpleArgs:
			c1 = ""
			c2 = ""
		else:
			c1 = "%s/**\n" % tab
			c2 = "\n%s*/"  % tab
			
		# If return type is a non-const pointer
		# then cast it explicitly to a non-const type
		cast = ""
		if fullrtype[-1] == "*":
			if rtype[:6] != "const ":
				cast = "(%s)" % rtype
			
		noms = {'tab': tab,
				'rtype': rtype,
				'cast' : cast,
				'classname': clname,
				'signature': signature,
				'methodcall': methodcall}

		m = '%(tab)s%(rtype)s %(signature)s const'\
			' { return %(cast)sobject->%(methodcall)s; }' % noms
		
		if len(m) > 79:
			m = '%(tab)s%(rtype)s %(signature)s const' % noms
			if len(m) > 79:
				m = '%(tab)s%(rtype)s\n%(tab)s%(signature)s const' % noms
			n = ' { return %(cast)sobject->%(methodcall)s; }' % noms
			if len(n+m) > 79:
				m = m + '\n%(tab)s' + strip(n)
				m = m % noms
		comment = "%(tab)s// from %(classname)s\n" % noms
		methods += "\n\n%s%s%s%s" % (comment, c1, m, c2)
	#----------------------------------
	
	print "class:  %s" % classname
	print "header: %s\n" % header
	print "\thelper class:  %s" % nspacewithcolon+name
	print "\thelper buffer: %s" % bname
	print ""
	print "\thelper code:   src/%s.cc" % filename
	print "\t               interface/%s.h" % filename
	print "\t               plugins/userplugin_%s.cc" % filename
	print
	
	names = {}
	names['filename']   = filename
	names['time']       = ctime(time())
	names['ctype']      = ctype
	names['name']       = name
	names['fullname']   = fullname
	names['headername'] = upper(bname)
	names['methods']    = methods
	
	if template == "":
		names['classname']  = classname
	else:
		names['classname']  = classname + " "
	names['classtype']  = classtype
	
	names['namespace']  = nspace
	if nspace != "":
		usingnspace = "using namespace %(namespace)s;" % names
	else:
		usingnspace = ""
	names['usingnamespace']  = usingnspace
	
	names['buffername'] = bname
	if template == "":
		names['header'] = '#include "%s"' % header
	else:
		names['header'] = '#include "DataFormats/Common/interface/ValueMap.h"'\
						  '\n#include "%s"' % header
	names['package']    = PACKAGE
	names['subpackage'] = SUBPACKAGE
	names['plugindir']  = PLUGINDIR
	names['srcdir']     = SRCDIR
	names['incdir']     = INCDIR
	names['author']     = AUTHOR
	#------------------------------------------------------------------------

	# delete previous .undo. files
	os.system('''
	rm -rf src/.undo.*
	rm -rf plugins/.undo.*
	rm -rf interface/.undo.*
	''')
	
	wrpluginheader(names)
	wrplugincode(names)
	wrplugin(names)

	#------------------------------------------------------------------------
	# Update BuildFile.xml in plugins directory
	#------------------------------------------------------------------------
	updated = False
	buildfile = "%s/BuildFile.xml" % PLUGINDIR
	undofile  = "%s/.undo.BuildFile.xml" % PLUGINDIR
	if os.path.exists(buildfile):
		os.system("cp %s %s" % (buildfile, undofile))
	else:
		updated = True
		out = open(buildfile, 'w')
		record = '''<use name="PhysicsTools/TheNtupleMaker"/>
<use name="FWCore/FWLite"/>
<use name="FWCore/PluginManager"/>

<use name="boost_regex"/>
<use name="boost_python"/>
<use name="rootrflx"/>
<use name="rootminuit"/>
<use name="rootmath"/>
<use name="f77compiler"/>
'''
		out.write(record)
		out.close()

		
	pkg = joinfields(split(header, '/')[:2],'/')
	record = open(buildfile).read()
	
	# Add package/sub-package required for helper

	if find(record, pkg) < 0:
		updated = True
		record += '\n<use name="%s"/>\n' % pkg

	pkg = '%(package)s/%(subpackage)s' % names
	if find(record, pkg) < 0:
		updated = True
		record += '\n<use name="%s"/>\n' % pkg
		
	if find(record, filename) < 0:
		updated = True
		record += '<library file="userplugin_%(filename)s.cc" '\
				  'name="%(filename)s">\n'\
				  '</library>\n' % names
	if updated:
		open(buildfile, 'w').write(record)
		print "\tupdated:       plugins/BuildFile.xml"

	#------------------------------------------------------------------------
	# Update classes.h
	#------------------------------------------------------------------------
	updated = False
	classesfile = "%s/classes.h" % SRCDIR
	undofile = "%s/.undo.classes.h" % SRCDIR
	
	if os.path.exists(classesfile):
		os.system("cp %s %s" % (classesfile, undofile))
	else:
		updated = True
		out = open(classesfile, 'w')
		record ='''//
//--------------------------------------------------------------------''' % \
		names
		out.write(record)
		out.close()
		os.system("cp %s %s" % (classesfile, undofile))

	record = open(classesfile).read()

	# update includes
	
	getincludes = re.compile("#include .*(?=\n)", re.M)
	includes = getincludes.findall(record)
	header = '%(package)s/%(subpackage)s/interface/%(filename)s.h' % names
	
	if len(includes) == 0:
		updated = True
		record += '\n#include "%s"' % header
	else:
		recincs  = joinfields(includes, '\n')
		if find(recincs, header) < 0:
			updated = True
			newrec = '%s\n#include "%s"\n' % (includes[-1], header)
			record = replace(record, includes[-1], newrec)

	# update namespace
	
	if find(record, "namespace") < 0:
		record += "\nnamespace\n{\n}\n"

	HelperFor = "HelperFor<%(classname)s>" % names
	
	if find(record, HelperFor) < 0:
		updated = True
		newrec = "  %s t_%s;\n}" % (HelperFor,
									names['buffername'])
		record = replace(record, "}", newrec)

	if updated:
		open(classesfile,'w').write(record)
		print "\tupdated:       src/classes.h"

	#------------------------------------------------------------------------
	# Update classes_def.xml
	#------------------------------------------------------------------------
	updated = False
	classesfile = "%s/classes_def.xml" % SRCDIR
	undofile = "%s/.undo.classes_def.xml" % SRCDIR
	if os.path.exists(classesfile):
		os.system("cp %s %s" % (classesfile, undofile))
	else:
		updated = True
		out = open(classesfile, 'w')
		record ='''<lcgdict>
</lcgdict>
		'''
		out.write(record)
		out.close()
		os.system("cp %s %s" % (classesfile, undofile))

	# Open existing classes_def.xml file
	record = open(classesfile).read()

	# remove </lcgdict>..it will be added back later
	record = replace(record, "</lcgdict>", "")
	record = rstrip(record) + "\n"
	
	# If an entry for the appropriate HelperFor
	# does not exist in the file, add one

	rec = ''
	if find(record, HelperFor) < 0:
		updated = True
		rec += ' <class name="%s"/>\n' % HelperFor
		rec = rec % names
		#print "\tADD %s" % HelperFor
		
	# If an entry for helper class does not exist, add one
	if find(record, fullname) < 0:
		updated = True
		rec +=' <class name="%(fullname)s"/>\n'
		rec = rec % names
	
	# Add back </lcgdict>
	if updated:
		record += rec + "</lcgdict>\n"
		open(classesfile,'w').write(record)
		print "\tupdated:       src/classes_def.xml"
		
	cmd = '''
	touch BuildFile.xml
	'''
	os.system(cmd)
	
#------------------------------------------------------------------------------
main()
