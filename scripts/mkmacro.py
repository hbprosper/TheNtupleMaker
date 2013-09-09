#!/usr/bin/env python
#------------------------------------------------------------------------------
# File: mkusermacro.py
# Description: Create ntuple selector.h file
# Created: 06-Mar-2010 Harrison B. Prosper
# Updated: 05-Oct-2010 HBP - clean up
#          12-Mar-2011 HBP - give user option to add variables
#          07-May-2012 HBP - fix object selection bug
#          13-May-2012 HBP - add more comments, include object counters
#          05-Jul-2013 HBP - defer much of the work to mkanalyzer.py
#
#$Id: mkmacro.py,v 1.9 2013/07/05 21:01:54 prosper Exp $
#------------------------------------------------------------------------------
import os, sys, re, posixpath
from string import atof, atoi, replace, lower,\
	 upper, joinfields, split, strip, find
from time import sleep, ctime
from glob import glob
#------------------------------------------------------------------------------
# Functions
#------------------------------------------------------------------------------
getvno = re.compile(r'[0-9]+$')

def usage():
	print '''
	Usage:
	   mkmacro.py <usermacro-name> [variables.txt]
	'''
	sys.exit(0)
	
def nameonly(s):
	return posixpath.splitext(posixpath.split(s)[1])[0]

def join(left, a, right):
	s = ""
	for x in a:
		s = s + "%s%s%s" % (left, x, right)
	return s

def getauthor():
	regex  = re.compile(r'(?<=[0-9]:)[A-Z]+[a-zA-Z. ]+')
	record = strip(os.popen("getent passwd $USER").read())
	author = "Shakepeare's ghost"
	if record != "":
		t = regex.findall(record)
		if len(t) > 0: author = t[0]
	return author
#------------------------------------------------------------------------------
AUTHOR = getauthor()

if os.environ.has_key("CMSSW_BASE"):
	CMSSW_BASE = os.environ["CMSSW_BASE"]
	PACKAGE = "%s/src/PhysicsTools/TheNtupleMaker" % CMSSW_BASE

HEADER=\
'''#ifndef %(NAME)s_H
#define %(NAME)s_H
//-----------------------------------------------------------------------------
// File:        %(name)s.h
// Description: user macro called by TheNtupleMaker
// Created:     %(time)s by mkmacro.py
// Author:      %(author)s
//
// Note:    To make your macro %(name)s known to TheNtupleMaker add the
//          line
//
//               macroName = cms.untracked.string("%(name)s.cc"),
//
//          to ntuple_cfi.py.
//
// WARNING: It is better not to edit this header.
//
//-----------------------------------------------------------------------------
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>

#include "TTree.h"
//-----------------------------------------------------------------------------
// TNM: for internal use only
struct countvalue
{
  int*    count;
  double* value;
};
typedef std::map<std::string, countvalue> VarMap;
typedef std::map<std::string, std::vector<int> > IndexMap;

/**----------------------------------------------------------------------------
  All user-defined variables and functions should be declared in this struct
  in the program %(name)s.cc, not in this header.
  
  This coding technique avoids the need to modify the header.
  An object of this type, called "local", is allocated in beginJob().
 ----------------------------------------------------------------------------*/

class %(name)sInternal;

// ----------------------------------------------------------------------------
class %(name)s
{
public:
  %(name)s() {}
  %(name)s(TTree* tree_, VarMap* varmap_, IndexMap* indexmap_)
    : tree(tree_), varmap(varmap_), indexmap(indexmap_) {}
  ~%(name)s() {}

  void beginJob();
  void initialize() { initialize_(); }
  bool analyze();
  void endJob();

  void select(std::string name)
  {
    (*indexmap)[name] = std::vector<int>();
  }
  
  void select(std::string name, int index)
  {
    if ( indexmap->find(name) == indexmap->end() ) return;
    (*indexmap)[name].push_back(index);
  }

private:
  TTree*    tree;
  VarMap*   varmap;
  IndexMap* indexmap;
  %(name)sInternal* local;
  void initialize_();
};
#endif
'''
HEADER_INIT =\
'''#ifndef %(NAME)sINITIALIZER_H
#define %(NAME)sINITIALIZER_H
//-----------------------------------------------------------------------------
// File:        %(name)sInitializer.h
// Description: Initialize variables for user macro
// Created:     %(time)s by mkmacro.py
// Author:      %(author)s
//-----------------------------------------------------------------------------
#include "%(name)s.h"
%(decl)s



// ------------------------------------------------------------------------
// --- Initialize variables
// ------------------------------------------------------------------------
void %(name)s::initialize_()
{
  using namespace std;
  using namespace evt;
  
  // clear object selection map every event
	
  for(IndexMap::iterator
    item=indexmap->begin(); 
    item != indexmap->end();
	++item)
	item->second.clear();	
	
%(impl)s
}

#endif
'''

MACRO=\
		  '''//-----------------------------------------------------------------------------
// File:        %(name)s.cc
// Description: user macro
// Created:     %(time)s by mkmacro.py
// Author:      %(author)s
//-----------------------------------------------------------------------------
#include "%(name)sInitializer.h"
using namespace std;
using namespace evt;
/**----------------------------------------------------------------------------
  All user-defined variables and functions should be declared in this struct.
  This coding technique avoids the need to modify the header %(name)s.h.
  An object of this type, called "local", is allocated in beginJob().

  
  IMPORTANT: If some of these variables are to be added to the ntuple,
             make sure the type of each variable matches the type format
			 specified in the call to tree->Branch(...). See example below.
 ----------------------------------------------------------------------------*/
struct %(name)sInternal
{
  int counter;

  //float HT;
};

/**----------------------------------------------------------------------------
  call these functions to select the specified objects

  example:
  
    select("jet");    to be called from beginJob()
 
  and
  
    select("jet", i); to be called from analyze() for every object
  
    to be kept
 ----------------------------------------------------------------------------*/
void %(name)s::beginJob()
{
  local = new %(name)sInternal();
  local->counter = 0;

  // Add a float variable to ntuple
  // Note match between the type of the variable HT (a float) and its
  // format specifier ("HT/F")
  //tree->Branch("HT", &local->HT, "HT/F");

  // Define objects that are to be selected in analyze()
  //select("jet");
}

void %(name)s::endJob()
{
  if ( local ) delete local;
}
//-----------------------------------------------------------------------------

bool %(name)s::analyze()
{
  local->counter++;

  // Uncomment if you want to fill the structs
  // fillObjects();
  
  // compute variables
  // apply cuts etc.

  //local->HT = 0;
  //for(int i=0; i < njet; ++i)
  //{
  //  if ( !(Jet_pt[i] > 100) ) continue;
  //  if ( !(Jet_pt[i] < 400) ) continue;
  //
  //  select("Jet", i);
  //  local->HT += Jet_pt[i];
  //} 
  
  //if ( miserable-event )
  //  return false;
  //else
  return true;
}
'''
COMPILE=\
		  '''#!/usr/bin/env python
import os, sys
from ROOT import *
#------------------------------------------------------------------------------
if os.environ.has_key("CMSSW_BASE"):
	rbase= os.environ["CMSSW_RELEASE_BASE"]
	base = os.environ["CMSSW_BASE"]
	arch = os.environ["SCRAM_ARCH"]
	
	incp = "-I" + base + "/src"
	gSystem.AddIncludePath(incp)

	incp = "-I" + rbase + "/src"
	gSystem.AddIncludePath(incp)

	libp = "-L" + base + "/lib/" + arch + " -lPhysicsToolsTheNtupleMaker" 
	gSystem.AddLinkedLibs(libp)

	libp = "-L" + rbase + "/lib/" + arch 
	gSystem.AddLinkedLibs(libp)
	
	gROOT.ProcessLine(".L %(name)s.C+");
'''
COMPILE=\
		  '''#------------------------------------------------------------------------------
# Created: %(time)s
#------------------------------------------------------------------------------
name	:= %(name)s
AT      := @ # leave blank for verbose printout
#------------------------------------------------------------------------------
CINT	:= rootcint
CXX     := g++
LDSHARED:= g++
#------------------------------------------------------------------------------
DEBUG	:= -ggdb
CPPFLAGS:= -I. $(filter-out -std=c++0x,$(shell root-config --cflags))
CXXFLAGS:= $(DEBUG) -pipe -O2 -fPIC -Wall

OS	:= $(shell uname -s)
ifeq ($(OS),Darwin)
	LDFLAGS	:= -dynamiclib
else
	LDFLAGS := -shared
endif
#------------------------------------------------------------------------------
LIBS	:= $(shell root-config --glibs)
#------------------------------------------------------------------------------
header  := $(name).h
linkdef	:= $(name)Linkdef.h
cinthdr := $(name)Dictionary.h
cintsrc	:= $(name)Dictionary.cc
cintobj	:= $(name)Dictionary.o
cppsrc 	:= $(name).cc
cppobj  := $(name).o

objects	:= $(cintobj) $(cppobj) 
library	:= lib$(name).so
#-----------------------------------------------------------------------
lib:	$(library)

$(library)	: $(objects)
	@echo "---> Linking $@"
	$(AT)$(LDSHARED) $(LDFLAGS) $+ $(LIBS) -o $@

$(cppobj)	: $(cppsrc)
	@echo "---> Compiling `basename $<`" 
	$(AT)$(CXX)	$(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(cintobj)	: $(cintsrc)
	@echo "---> Compiling `basename $<`"
	$(AT)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(cintsrc) : $(header) $(linkdef)
	@echo "---> Generating dictionary `basename $@`"
	$(AT)$(CINT) -f $@ -c $(CPPFLAGS) $+

$(linkdef)	:
	@echo "---> Creating $(linkdef)"
	@echo -e "#include <map>"  >	$(linkdef)
	@echo -e "#include <string>" >>	$(linkdef)
	@echo -e "#include <vector>" >>	$(linkdef)
	@echo -e "#ifdef __CINT__" >> $(linkdef)
	@echo -e "#pragma link off all globals;" >> $(linkdef)
	@echo -e "#pragma link off all classes;" >> $(linkdef)
	@echo -e "#pragma link off all functions;" >> $(linkdef)
	@echo -e "#pragma link C++ class countvalue+;" >> $(linkdef)
	@echo -e "#pragma link C++ class map<string, countvalue>+;" \
	>> $(linkdef)
	@echo -e "#pragma link C++ class map<string, vector<int> >+;" \
	>> $(linkdef)
	@echo -e "#pragma link C++ class %(name)s+;" \
	>> $(linkdef)	
	@echo -e "#endif" >> $(linkdef)

clean   :
	rm -rf $(cintsrc) $(cinthdr) $(objects) $(linkdef)

veryclean   :
	rm -rf $(cintsrc) $(cinthdr) $(objects) $(linkdef) $(library)
'''
#------------------------------------------------------------------------------
def main():
	print "\n\tmkmacro.py"

	# Decode command line

	argv = sys.argv[1:]
	argc = len(argv)
	if argc < 1: usage()

	name = nameonly(argv[0])
	if argc > 1:
		varfilename = argv[1]
	else:
		varfilename = "variables.txt"
	if not os.path.exists(varfilename):
		print "\t** error ** can't find variable file: %s" % varfilename
		sys.exit(0)


	# use mkanalyzer in macro mode to do the heavy lifting
	cmd = 'mkanalyzer.py %s %s macromode' % (name, varfilename)
	os.system(cmd)
	
	if not os.path.exists('%s_decl.h' % name):
		print '** unable to open file %s_decl.h' % name
		sys.exit(0)
	decl = open('%s_decl.h' % name).read()
	
	if not os.path.exists('%s_decl.h' % name):
		print '** unable to open file %s_decl.h' % name
		sys.exit(0)
	impl = open('%s_impl.h' % name).read()

	os.system('rm %s_*l.h' % name)
	
	# Create C++ codes

	names = {'name'  : name,
			 'NAME'  : upper(name),
			 'time'  : ctime(),
			 'decl'  : decl,
			 'impl'  : impl,
			 'author': AUTHOR,
			 's': '%s'
			 }

	os.system('mkdir -p %s' % name)
	outfilename = "%(name)s/%(name)s.h" % names
	record = HEADER % names
	open(outfilename, "w").write(record)


	outfilename = "%(name)s/%(name)sInitializer.h" % names
	record = HEADER_INIT % names
	open(outfilename, "w").write(record)

	outfilename = "%(name)s/%(name)s.cc" % names
	if os.path.exists(outfilename):
		print "\t** %s already exists...please delete or rename first!" % \
		outfilename
	else:
		record = MACRO % names	
		open(outfilename, "w").write(record)

	record = COMPILE % names
	outfilename = "%(name)s/Makefile" % names
	open(outfilename, "w").write(record)

	print "\tto compile and link the macro %(name)s\n\tdo" % names
	print "\t\tcd %(name)s" % names
	print "\t\tmake\n" % names
#------------------------------------------------------------------------------
main()

