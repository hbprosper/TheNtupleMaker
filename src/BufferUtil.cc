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
#include "PhysicsTools/TheNtupleMaker/interface/kit.h"
#include "PhysicsTools/TheNtupleMaker/interface/treestream.h"
#include "PhysicsTools/TheNtupleMaker/interface/pluginfactory.h"
#include "PhysicsTools/TheNtupleMaker/interface/BufferUtil.h"
//-----------------------------------------------------------------------------

/// Function to initialize a Buffer.
void initializeBuffer(otreestream& out,
                      std::string& objectname,  
                      std::string& classname,
                      std::string& label,
                      std::string& label1,
                      std::string& label2,
                      std::string& prefix,
                      std::vector<VariableDescriptor>& var,
                      int&  count,
                      ClassType ctype,
                      int   maxcount,
                      std::ofstream& log,
                      std::string&   bufferkey,
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

  // Type name of object that exports methods, that is, whose methods
  // return values that can be stored in the n-tuple

   if ( debug > 0 )
    {
      switch (ctype)
        {
        case SINGLETON:
          std::cout << RED  << "SINGLETON BUFFER FOR( " << classname << " )" 
                    << BLACK << std::endl;
          break;

        case COLLECTION:
          std::cout << RED  << "COLLECTION BUFFER FOR( " << classname 
                    << " )" 
                    << BLACK << std::endl;
          break;

        case CONTAINER:
          std::cout << RED  << "CONTAINER BUFFER FOR( " << classname << " )" 
                    << BLACK << std::endl;
        }
    }

  // Split getByLabel into its component parts

  label1 = label;
  int i = label.find("_");
  if ( i > 0 )
    {
      label1 = label.substr(0,i);
      label2 = label.substr(i+1, label.size()-i-1);
      label  = label1 + ", " + label2;
    }

  // Just return if no variables were specified
  
  if ( var.size() == 0 ) 
    {
      std::cout 
        << "** Warning! Buffer::init - no variables defined for class " 
        << classname
        << std::endl
        << "** and getByLabel \"" << label << "\""
        << std::endl;
      return;
    }
  

  // buffer key is used in shrink buffer method of TNM
  bufferkey = objectname;

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

  std::string counter("");

  bool isArray = maxcount > 1; 
  if ( isArray )
    {
      // Add leaf counter to tree
      counter = "n" + prefix;        
      out.add(counter, count);
      std::cout << "      counter: " << counter 
                << "\taddress( " << &count << ")" << std::endl;
      log << "int/" 
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
      std::string branchname = prefix + "." + varname;

      // check for uniqueness
      if ( branchset.find(branchname) != branchset.end() )
        {
          edm::LogWarning("BranchNotUnique")
            << "\t...Fee fi fo fum" 
            << std::endl
            << "\t...I smell the blood of an Englishman"
            << std::endl
            << "\t...Be he alive, or be he dead"
            << std::endl
            << "\t...I'll grind his bones to make my bread"
            << std::endl
            << "This branch (" 
            << BOLDRED << branchname 
            << DEFAULT_COLOR << ") is NOT unique!"
            << std::endl;
          continue;
        }
      branchset.insert(branchname);

      log << rtype << "/" 
          << branchname  << "/"
          << objectname + "_" + varname << "/"
          << maxcount 
          << std::endl;
        
      if ( isArray ) branchname += "[" + counter + "]";
      
      // Note: nvar <= i
      var[nvar].name = branchname;
      var[nvar].maxcount = maxcount;
      var[nvar].method = method;

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

