// -*- C++ -*-
//
// Package:    PhysicsTools
// Class:      ClassMembers
// 
/**\class ClassMembers ClassMembers.cc 
   PhysicsTools/TheNtupleMaker/src/kit.cc

   Description: Some simple utilities

   Implementation:
   Common sense and a sense of beauty.
*/
//
// Original Author:  Harrison B. Prosper
//         Created:  Wed Jun 20 19:53:47 EDT 2007
//         Updated:  Sat Oct 25 2008 - make matchInDeltaR saner
// $Id: ClassMembers.cc,v 1.1.1.1 2011/05/04 13:04:29 prosper Exp $
//
//
//-----------------------------------------------------------------------------
#include <Python.h>
#include <boost/python.hpp>
#include <boost/regex.hpp>
#include <string>
//-----------------------------------------------------------------------------
#include "TString.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "PhysicsTools/TheNtupleMaker/interface/ClassMembers.h"
#include "PhysicsTools/TheNtupleMaker/interface/rfx.h"
//-----------------------------------------------------------------------------
using namespace std;
using namespace ROOT::Reflex;
//-----------------------------------------------------------------------------

namespace 
{
  string
  replace(string& str, string oldstr, string newstr)
  {
    return string(TString(str).ReplaceAll(oldstr, newstr).Data());
  }

  string subbasicstr(string& str)
  {
    return replace(str, "std::basic_string<char>", "std::string");
  }
  
  bool skipmethod(string& str)
  {
    string expr("TClass|TBuffer|TMember|operator|__get");
    vector<string> v = rfx::regex_findall(str, expr);
    return v.size() > 0;
  }
  
  string 
  strip(string line)
  {
    int l = line.size();
    if ( l == 0 ) return string("");
    int n = 0;
    while (((line[n] == 0)    ||
            (line[n] == ' ' ) ||
            (line[n] == '\n') ||
            (line[n] == '\t')) && n < l) n++;
    
    int m = l-1;
    while (((line[m] == 0)    ||
            (line[m] == ' ')  ||
            (line[m] == '\n') ||
            (line[m] == '\t')) && m > 0) m--;
  return line.substr(n,m-n+1);
  }

  void
  bisplit(string str, string& left, string& right, string delim,
          int direction=1)
  {
    left  = str;
    right = "";
    int i = 0;
    if ( direction > 0 )
      i = str.rfind(delim);
    else      
      i = str.find(delim);
    if ( i > 0 )
      {
        left  = str.substr(0, i);
        right = str.substr(i+delim.size(), str.size()-i-delim.size());
    }
  }
}

/** Recursively scan class and return all "getter" methods.
 */

ClassMembers::ClassMembers()
  : classname_(""),
    debug_(0),
    baseclasses_(vector<string>()),
    getters_(vector<string>()),
    setters_(vector<string>()),
    datamembers_(vector<string>()),
    signatures_(map<string, int>())
{}

ClassMembers::ClassMembers(string classname, int debug)
  : classname_(classname),
    debug_(debug),
    baseclasses_(vector<string>()),
    getters_(vector<string>()),
    setters_(vector<string>()),
    datamembers_(vector<string>()),
    signatures_(map<string, int>())
{
  get_getters_(classname);
  signatures_.clear();
  get_datamembers_(classname);
}

ClassMembers::~ClassMembers() {}

void
ClassMembers::get_getters_(string classname, int depth)
{
  using namespace Reflex;

  string spaces = "                                                          ";
  string tab = spaces.substr(0, 2*depth);
  if ( debug_ > 0 )
    cout << tab << "CLASS(" << classname  << ")" << endl;

  if ( depth > 10 ) 
    {
      cout << "ClassMembers::getmethods - yikes, lost in the trees!" 
           << endl;
      exit(0);
    }

  Type thing;
  Type c = thing.ByName(classname);
  int nfunctions = c.FunctionMemberSize();
  
  if ( debug_ > 0 )
    cout << tab << "  number of functions: " << nfunctions << endl;
 
  for(int i=0; i < nfunctions; i++)
    {
      Member m = c.FunctionMemberAt(i);
      if ( !m.IsPublic() ) continue;
      if ( !m.IsFunctionMember() ) continue;
      if ( m.IsConstructor() ) continue;
      if ( m.IsDestructor() ) continue;

      // we have a public method, decode it

      string mname = m.Name();
      string mtype = m.TypeOf().Name(SCOPED);

      if ( debug_ > 2 )
        cout << tab << "\t  "
             << "NAME(" << mname << ") "
             << "MTYPE(" << mtype << ")" << endl;

      // simplify edm::Refs

      boost::regex expr1("edm::Ref<std::vector<.+?(?=> >)");  
      boost::smatch match1;
      if ( boost::regex_search(mtype, match1, expr1) )
        {
          boost::regex expr2("edm::Ref<std::vector<.+?(?=>,)");  
          boost::smatch match2;
          if ( boost::regex_search(mtype, match2, expr2) )
            mtype = replace(mtype, match1[0], match2[0]);
        }

      // replace basic_str with std::string

      mtype = strip( subbasicstr(mtype) );

      // split method type into return type and argument list
      string rtype, args;
      bisplit(mtype, rtype, args, "(");
      args = "(" + args;
      rtype = strip(rtype);

      if ( debug_ > 2 )
        cout << tab << "\t "
             << " RTYPE(" << rtype << ")" 
             << " ARGS(" << args << ")" 
             << endl;

      args = replace(args, "(void)", "()");
      string signature = mname + args;

      // skip if this method already included

      if ( signatures_.find(signature) != signatures_.end() ) continue;
      signatures_[signature] = i;
      if ( debug_ > 2 )
        cout << tab << "\t  signature(" << signature << ")" << endl;

      // skip Root serialization methods
      string method = rtype + " " + signature;
      if ( skipmethod(method) )
        {
          if ( debug_ > 1 ) 
            cout << tab << "\t** skip: " << method << endl << endl;
          continue;
        }

      string record 
        = rtype + "|"
        + mname + "|"
        + args + "|"
        + classname;

      rtype = strip(rtype);
      if ( rtype.substr(0, 4) == "void" )
        setters_.push_back(record);
      else
        getters_.push_back(record);

      if ( debug_ > 0 )
        cout << tab << "\t  METHOD(" << record << ")" 
             << endl << endl;
    }
  
  // Loop over base classes, if present

  int nbase = c.BaseSize();
  if ( debug_ > 0 )
    cout << tab << "\t number of base classes: " << nbase << endl;

  for(int i=0; i < nbase; i++)
    {
      Type b = c.BaseAt(i).ToType();
      string basename = b.Name(SCOPED);
      baseclasses_.push_back(basename);
      get_getters_(basename, depth+1);
    }
}

void
ClassMembers::get_datamembers_(string classname, int depth)
{
  using namespace Reflex;

  string spaces = "                                                          ";
  string tab = spaces.substr(0, 2*depth);
  if ( debug_ > 0 )
    cout << tab << "CLASS(" << classname  << ")" << endl;

  if ( depth > 10 ) 
    {
      cout << "ClassMembers::getdatamembers - yikes, lost in the trees!" 
           << endl;
      exit(0);
    }

  Type thing;
  Type c = thing.ByName(classname);
  int nmembers = c.DataMemberSize();
  
  if ( debug_ > 0 )
    cout << tab << "  number of data members: " << nmembers << endl;
 
  for(int i=0; i < nmembers; i++)
    {
      Member m = c.DataMemberAt(i);
      if ( !m.IsPublic() ) continue;

      // we have a public data member, decode it

      string mname = m.Name();
      string mtype = m.TypeOf().Name(SCOPED);

      if ( debug_ > 2 )
        cout << tab << "\t  "
             << "NAME(" << mname << ") "
             << "MTYPE(" << mtype << ")" << endl;

      // simplify edm::Refs

      boost::regex expr1("edm::Ref<std::vector<.+?(?=> >)");  
      boost::smatch match1;
      if ( boost::regex_search(mtype, match1, expr1) )
        {
          boost::regex expr2("edm::Ref<std::vector<.+?(?=>,)");  
          boost::smatch match2;
          if ( boost::regex_search(mtype, match2, expr2) )
            mtype = replace(mtype, match1[0], match2[0]);
        }

      // replace basic_str with std::string

      mtype = strip( subbasicstr(mtype) );
      string signature = mtype;

      // skip if this data member already included

      if ( signatures_.find(signature) != signatures_.end() ) continue;
      signatures_[signature] = i;

//       string record 
//         = rtype + "|"
//         + mname + "|"
//         + args + "|"
//         + classname;

//       rtype = strip(rtype);
      datamembers_.push_back(mtype);
    }
  
  // Loop over base classes, if present

  int nbase = c.BaseSize();
  if ( debug_ > 0 )
    cout << tab << "\t number of base classes: " << nbase << endl;

  for(int i=0; i < nbase; i++)
    {
      Type b = c.BaseAt(i).ToType();
      string basename = b.Name(SCOPED);
      baseclasses_.push_back(basename);
      get_datamembers_(basename, depth+1);
    }
}
