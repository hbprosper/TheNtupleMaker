//-----------------------------------------------------------------------------
// Package:     PhysicsTools
// Sub-Package: TheNtupleMaker
// Source:      BufferUtil.cc
// Original Author:  Harrison B. Prosper
//         Created:  Tue Dec  8 15:40:26 CET 2009
//         Updated:  Sun Sep 19 HBP move initBuffer from Buffer.h
//                   Thu Feb 17 HBP change definition of isArray (maxcount > 1)
//                   Wed Jul 20 HBP handle BasicType
//                   25-Apr-2012 HBP fix objectname when it is a template
//                   Thu Jul 04 2013 HBP - add objectname to argument fof init
//                                   by default objectname = name of block
//                                   in config file. This way the C++ object
//                                   name in the variables.txt file will be
//                                   the same as that specified in the config
//                                   file.
//                   Fri Jan 24 2014 HBP - get number of members in PDF set 
//                                   using LHAPDF::numberPDF().
//                                   If a branchname is not unique, make it
//                                   unique by adding buffer name to branch
//                                   name.
//                   Wed Dec 17 2014 HBP - split off creation of branchnames
//                                   from initializeBuffer
//
// $Id: BufferUtil.cc,v 1.6 2013/07/05 07:15:14 prosper Exp $
//-----------------------------------------------------------------------------
#include <Python.h>
#include <boost/python/type_id.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <string>
#include <set>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"

#include "PhysicsTools/TheNtupleMaker/interface/BufferUtil.h"
#include "PhysicsTools/TheNtupleMaker/interface/kit.h"
#include "PhysicsTools/TheNtupleMaker/interface/treestream.h"
#include "PhysicsTools/TheNtupleMaker/interface/pluginfactory.h"

//-----------------------------------------------------------------------------
void createBranchnames(std::string& objectname,  
		       std::string& prefix,
		       std::string& counter,
		       std::vector<VariableDescriptor>& var,
		       int   maxcount,
		       std::ofstream& vout,
		       std::set<std::string>& branchset,
		       int   debug)
{
  // Define regular expressions to check for compound methods; i.e., methods
  // of the form 
  // 1. y = a()->b()
  // 2. y = a().b()
  // 3. y = a().b 
  // 4. y = a()->b 
  // 5. y = a.b()
  // 6. y = a->b()

  boost::regex stripargs("[(].*?[)]");
  boost::regex stripme("-[>]|[.]|\"|[(]|[)]| |,|[<]|[>]");
  boost::regex stripcolon("[a-zA-Z]+::");
  boost::regex strip3_("___");
  boost::regex strip2_("__");
  boost::regex strip2_atend("_$");

  // Just return if no variables were specified
  
  if ( var.size() == 0 ) 
    {
      std::cout 
        << "** Warning! createBranchnames - no variables defined for  " 
        << objectname
        << std::endl;
      return;
    }
  
  // Define variables destined for the output tree
  
  std::cout << "   n-tuple variables:" << std::endl;
  
  // Root is able to store vector<..> types. However, since we want the 
  // resulting n-tuple to be as simple as possible, we'll handle the mapping 
  // to and from vectors ourselves.

  // If we have a vector variable, create a counter variable for it.
  // Root calls this a "leaf counter". A vector variable is generally a
  // collection. However, a helper for a singleton object could expand
  // into a vector variable. Likewise, a helper class for a collection object,
  // say a collection of pat::Jets, could map these objects to a single 
  // instance of each variable, for example to HT. If so, we shall assume 
  // that the n-tuple variable is to be a simple non-array type.

  counter = std::string("");

  bool isArray = maxcount > 1; 
  if ( isArray )
    {
      // Add leaf counter to tree
      counter = "n" + prefix;        
      std::cout << "      counter: " << counter << std::endl;
      vout << "int/" 
	   << counter << "/"
	   << "n"+objectname << "/"
	   << 1 << " *" 
	   << std::endl;
    }
  
  // For every method, create the associated n-tuple variable name
  int nvar = 0;
  for(unsigned i=0; i < var.size(); i++)
    {    
      std::string rtype   = var[i].rtype;
      std::string method  = var[i].method;
      std::string varname = var[i].varname;
      if ( debug > 0 )
        std::cout << BLUE
                  << " varname before(" << varname << ")"
                  << BLACK
                  << std::endl;

      // Replace "->", ".", "(", ")" and quotes by "_"
      varname = boost::regex_replace(varname, stripme,  "_");

      // Replace "___" by "_"
      varname = boost::regex_replace(varname, strip3_,  "_");

      // Replace "__" by "_"
      varname = boost::regex_replace(varname, strip2_,  "_");

      // Strip away possible "_" at the end 
      varname = boost::regex_replace(varname, strip2_atend,  "");

      if ( debug > 0 )
        std::cout << "        "
                  << RED 
                  << " varname after (" << varname << ")"
                  << BLACK
                  << std::endl;      

      // create branch name 
      std::string  branchname = prefix + "." + varname;

      // check for uniqueness
      if ( branchset.find(branchname) != branchset.end() )
        {
	  // the branchname is not unique, so make it so
          std::string newbranchname =
            prefix + "_" + objectname + "." + varname;
            
          edm::LogWarning("BranchNotUnique")
            << "Fee fi fo fum\n"
            << "I smell the blood of an Englishman\n"
            << "Be he alive, or be he dead\n"
            << "I'll grind his bones to make my bread\n"
            << "This branch (" 
            << BOLDRED << branchname 
            << DEFAULT_COLOR << ") is NOT unique!\n"
            << "Change name to \n"
            << BOLDBLUE << newbranchname
            << DEFAULT_COLOR 
            << std::endl;
          branchname = newbranchname;
        }
      branchset.insert(branchname);

      vout << rtype << "/" 
	   << branchname  << "/"
	   << objectname + "_" + varname << "/"
	   << maxcount 
	   << std::endl;
        
      if ( isArray ) branchname += "[" + counter + "]";
      
      // Note: nvar <= i
      var[nvar].name     = branchname;
      var[nvar].maxcount = maxcount;
      var[nvar].method   = method;

      if ( debug > 0 )
        std::cout << "   " << nvar << ":\t" << branchname 
                  << std::endl
                  << "\t\t" << method << std::endl;

      // update variable count
      nvar++;
    }

  // update size of var, if necessary
  if ( nvar < (int)var.size() ) var.resize(nvar);
}

void init0(std::string& classname, bool& skipme)
{
  // We need to skip these classes, if we are running over real data
  boost::regex getname("GenEvent|GenParticle|"
		       "GenJet|GenRun|genPart|generator|"
		       "LHEEventProduct|"
		       "PileupSummaryInfo");
  boost::smatch m;
  skipme = boost::regex_search(classname, m, getname);
}

void init1(std::string& classname,
	   std::string& label,
	   std::string& label1,
	   std::string& label2,
	   bool& crash,
	   BufferType& buffertype)
{
  // Split Label into its component parts
  label1 = label;
  int i = label.find("_");
  if ( i > 0 )
    {
      label1 = label.substr(0,i);      
      label2 = label.substr(i+1, label.size()-i-1);
      label  = label1 + ", " + label2;
    }

  std::cout << "\t=== Initialize Buffer for ("
	    << classname << ")"
	    << std::endl;

  // Get optional crash switch
  try
    {
      crash =
	(bool)(Configuration::instance().
	       getConfig()->
	       getUntrackedParameter<int>("crashOnInvalidHandle"));
    }
  catch (...)
    {
      crash = true;
    }

  if ( crash )
    std::cout << "\t=== CRASH on InvalidHandle ("
	      << classname << ")"
	      << std::endl;
  else
    std::cout << "\t=== WARN on InvalidHandle ("
	      << classname << ")"
	      << std::endl;

  // Is this a RunInfo object?
  // Data for these classes must be extracted using the getRun()
  // method of the event object.
  // Definition: An extractable object is one that can be extracted from an
  // event using getByLabel

  boost::regex re("RunInfo");
  boost::smatch match;
  if ( boost::regex_search(classname, match, re) ) buffertype = RUNINFO;
}


void initCache(std::string& counter,
	       std::vector<std::string>& fullname,
	       std::vector<double*>& value,
	       int& count,
	       std::vector<std::string>& varnames,
	       std::map<std::string, countvalue>& varmap)
{
  // We can use vectors for the following because an inadvertent call
  // to a destructor is innocuous.

  boost::regex getname("[^[]+");
  varnames.clear();
  varmap.clear();
  for(unsigned i=0; i < fullname.size(); i++)
    {
      boost::smatch m;
      boost::regex_search(fullname[i], m, getname);
      std::string name = m[0];
      varnames.push_back(name);

      countvalue v;
      v.count = &count;
      v.value = value[i];
      varmap[name] = v;
    }

  // Add counter variable
  {
    countvalue v;
    v.count = &count;
    v.value = 0;
    varnames.push_back(counter);
    varmap[counter] = v;
  }

  {
    countvalue v;
    v.count = 0;
    v.value = 0;
    varmap["NONE"] = v;
  }
}
