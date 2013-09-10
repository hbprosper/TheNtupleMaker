// -*- C++ -*-
//
// Package:    PhysicsTools
// Class:      rfx
// 
/**\class rfx rfx.cc 
   PhysicsTools/TheNtupleMaker/src/rfx.cc

   Description: Some Reflex utilities

   Implementation:
   Common sense and a sense of beauty.
*/
//
// Original Author:  Harrison B. Prosper
//         Created:  Wed Jun 20 19:53:47 EDT 2007
//         Updated:  Sat Oct 25 2008 - make matchInDeltaR saner
// $Id: rfx.cc,v 1.5 2012/05/05 22:03:57 prosper Exp $
//
//
//-----------------------------------------------------------------------------
#include <Python.h>
#include <boost/python.hpp>
#include <boost/regex.hpp>

#include <sstream>
#include <cassert>
#include <string>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
//-----------------------------------------------------------------------------
#include "TROOT.h"
#include "TSystem.h"
#include "TString.h"
#include "TList.h"
#include "TClass.h"
#include "TIterator.h"
#include "TFile.h"
#include "TInterpreter.h"

#include "FWCore/Utilities/interface/Exception.h"
#include "PhysicsTools/TheNtupleMaker/interface/rfx.h"
#include "PhysicsTools/TheNtupleMaker/interface/colors.h"
//-----------------------------------------------------------------------------
using namespace std;
using namespace ROOT::Reflex;
//-----------------------------------------------------------------------------
static bool DBgetMethod = getenv("DBgetMethod") > 0 ? true : false; 
static bool DBdecodeMethod = getenv("DBdecodeMethod") > 0 ? true : false; 
static bool DBdecodeArguments = getenv("DBdecodeArguments") > 0 ? true :false; 
//-----------------------------------------------------------------------------
string 
rfx::strip(string line)
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

string
rfx::replace(string& str, string oldstr, string newstr)
{
  return string(TString(str).ReplaceAll(oldstr, newstr).Data());
}

void
rfx::bisplit(string str, string& left, string& right, string delim,
             int direction)
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

vector<string>
rfx::regex_findall(string& str, string expr)
{
  vector<string> v;
  // Get all matches
  boost::regex regexp(expr);
  int start = 0;
  int end = (int)str.size();
  for (;;)
    {
      boost::smatch matches;
      if ( boost::regex_search(str.substr(start, end), 
                               matches, regexp,
                               boost::match_partial) )
        {
          v.push_back(matches[0]);
          int index=0;
          start 
            += (int)(matches.position(index)) 
            +  (int)(matches[0].length()) + 1; 
          if ( start >= end ) break;
        }
      else
        break;
    }
  return v;
} 

string
rfx::regex_search(string& str, string expr) 
{
  boost::regex regexp(expr);
  boost::smatch match;
  if ( boost::regex_search(str, match, regexp) )
    return match[0];
  else
    return "";
}

string
rfx::regex_sub(string& str, string expr, string rstr) 
{
  boost::regex regexp(expr);
  boost::smatch match;
  if ( boost::regex_search(str, match, regexp) )
    return replace(str, match[0], rstr);
  else
    return str;
}

unsigned int
rfx::memcheck(std::string program)
{
  string cmd("ps aux | grep "+program);
  FILE* f = popen(cmd.c_str(),"r");
  int buffsize=8192;
  char s[8192];
  int n = fread(s,1,buffsize,f);
  pclose(f);
  istringstream inp(rfx::strip(string(s).substr(0,n)));
  string a;
  unsigned int k;
  inp >> a >> a >> a >> a >> k;
  return k;
}

// ---------------------------------------------------------------------------
// Reflex stuff
// ---------------------------------------------------------------------------

bool 
rfx::isCompoundMethod(std::string expression, std::string& delim)
{
  // method could be of the form
  // y = method1(...)->method2(...) or
  // y = method1(...).method2(...)
  // y = method1(...).datamember
  // y = datamember.method2(...)
  // y = datamember->method2(...)
  boost::regex expr("(?<=[)a-zA-Z_0-9]) *([-][>]|[.]) *(?=[a-zA-Z])");
  boost::smatch what;
  bool yes = boost::regex_search(expression, what, expr);
  if ( yes ) delim = what[0];
  return yes;
}

string
rfx::fullname(std::string classname)
{

  Type t = Type::ByName(classname);
  return t.FinalType().Name(SCOPED);
}

void 
rfx::getScopes(std::string classname, 
               std::vector<std::string>& names, 
               int depth)
{
  depth++;
  if ( depth > 20 )
    {
      throw cms::Exception("LostInTrees")
        << "\tgetScopes: yikes! I'm lost in the trees" << std::endl;
    }
  names.push_back(classname);

  Type c = Type::ByName(classname);
  for (unsigned i=0; i < c.BaseSize(); i++)
    {
      string basename = c.BaseAt(i).ToType().Name(SCOPED);
      rfx::getScopes(basename, names, depth);
    }
}

Reflex::Member
rfx::getMethod(std::string classname,
               std::string methodname,
               std::string args)
{
  using namespace Reflex;
  vector<string> names;
  rfx::getScopes(classname, names);

  boost::regex expr(args);
  boost::smatch what;

  if ( DBgetMethod )
    cout << "getMethod - \n"
         << "  classname<" << classname << ">\n"
         << "    method<" << methodname << ">\n" 
         << "      args<" << args << ">" 
         << endl;

  for(unsigned i=0; i < names.size(); i++)
    {

      if ( DBgetMethod )
        cout << i << "\tSCOPE( " << names[i] << " )" << endl;

      Type t = Type::ByName(names[i]); 

      for(unsigned j=0; j < t.FunctionMemberSize(); j++)
        {
          Member m = t.FunctionMemberAt(j);
          if ( !m.IsPublic() ) continue;
          if ( !m.IsFunctionMember() ) continue;
          if ( m.IsConstructor() ) continue;
          if ( m.IsDestructor() ) continue;
 
          // Check method name
          string name = m.Name();

          if ( DBgetMethod )
            cout << "\t\tname: " << name << endl;

          if ( methodname != name ) continue;

          // Now check signature
          string signature = m.TypeOf().Name(SCOPED);

          if ( DBgetMethod )
            cout << "\t\t\tsignature: " 
                 << RED << signature 
                 << DEFAULT_COLOR << endl
                 << "\t\t\targsregex: " << args << endl;

          if ( boost::regex_search(signature, what, expr) ) 
            {
              if ( DBgetMethod )
                cout << "\t\t\t\t** matched **" << endl;

              // We have a match
              return m;
            }
          else
            {
              // The number of required arguments may be zero.
              // If it is try to match the signature (void)
              int nargs = m.FunctionParameterSize(true);
              if ( nargs == 0 )
                {
                  if ( DBgetMethod )
                    cout << "\t\t\t#required: " 
                         << RED << 0 
                         << DEFAULT_COLOR << endl;

                  signature = string("(void)");
                  if ( boost::regex_search(signature, what, expr) ) 
                    {
                      if ( DBgetMethod )
                        cout << RED 
                             << "\t\t\t\t** matched **"
                             << DEFAULT_COLOR
                             << endl;
                      
                      // We have a match
                      return m;
                    }
                }
            }
        }
    }
  return Member();
}

Reflex::Member
rfx::getDataMember(std::string classname,
                   std::string datumname)
{
  using namespace Reflex;
  vector<string> names;
  rfx::getScopes(classname, names);

  for(unsigned i=0; i < names.size(); i++)
    {
      //DB
      //cout << i << "\tSCOPE( " << names[i] << " )" << endl;

      Type t = Type::ByName(names[i]); 

      for(unsigned j=0; j < t.DataMemberSize(); j++)
        {
          Member m = t.DataMemberAt(j);
          if ( !m.IsPublic() ) continue;
 
          // Check member name
          string name = m.Name();

          //DB
          //cout << "\t\t" << name << endl;
          if ( datumname == name )
            {
              return m;
            }
        }
    }
  return Member();
}

bool
rfx::memberValid(Member& m)
{
  return m.DeclaringType().Name() != "";
}

int
rfx::simpleType(string name)
{
  // Codes:
  // 0 non-simple 
  // 1 float
  // 2 integer
  // 3 string
  // 4 void

  vector<boost::regex> regex;

  // Regex for reals
  regex.push_back(boost::regex("double|float"));

  // Regex for integers
  regex.push_back(boost::regex("(unsigned )?(short|int|long|size_t)"));

  // Regex for strings
  regex.push_back(boost::regex("string"));

  // Regex for bools
  regex.push_back(boost::regex("bool"));

  // Regex for voids
  regex.push_back(boost::regex("void"));

  int code=0;
  for(unsigned int i=0; i < regex.size(); ++i)
    {
      boost::smatch what;
      if ( boost::regex_search(name, what, regex[i]) )
        {
          code = i + 1;
          break;
        }
    }
  return code;
}

void
rfx::decodeArguments(std::string  args,
                     std::string& argsregex,
                     std::vector<rfx::ValueThing*>& vars)
{
  //DB
  if ( DBdecodeArguments )
    cout << "decodeArguments - ARGS(" << args << ")" << endl;

  // Split string into argument fields

  bool isString = false;
  vector<string> arglist;
  string str("");
  for(unsigned i=1; i < args.size(); i++)
    { 
      if ( isString )
        {
          str += args[i];
          if ( args[i] == '"' ) isString = false;
        }
      else if ( args[i] == '"' )
        {
          str += args[i];
          isString = true;
        }
      else if ( args[i] == ',' )
        {
          arglist.push_back(str);
          str = "";
        }
      else if ( args[i] == ')' )
        {
          arglist.push_back(str);
          str = "";
        }
      else
        {
          str += args[i];
        }
    }

  // Create regex for arguments and note the type

  // Regex for strings
  vector<string> atype;
  vector<boost::regex> expr;
  expr.push_back(boost::regex("\".*\"")); 
  atype.push_back("(std::string|std::basic_string<char>)");

  // Regex for reals
  expr.push_back(boost::regex("[-+]?[0-9]*[.][0-9]*([eE][-+]?[0-9]+)?"));
  atype.push_back("(double|float)");

  // Regex for integers
  expr.push_back(boost::regex("[-+]?[0-9]+"));
  atype.push_back("((unsigned )?(short|int|long|size_t))");

  // Regex for bools
  expr.push_back(boost::regex("\b(false|true)\b"));
  atype.push_back("(bool)");

  // Regex for voids
  expr.push_back(boost::regex("\bvoid\b"));
  atype.push_back("(void)");

  boost::smatch what;

  vector<int> vartype(expr.size(),-1);
                      
  argsregex = string("");
  string delim("[(]");
  for(unsigned i=0; i < arglist.size(); i++)
    {
      string str = rfx::strip(arglist[i]);
      //DB
      if ( DBdecodeArguments )
        cout << "\targ(" << str << ")" << endl;

      if ( str == "" ) // Check for void
        {
          vartype[i] = 4;
          argsregex += delim + atype[4];
        }
      else
        {
          for(unsigned j=0; j < expr.size(); j++)
            {
              //DB
              if ( DBdecodeArguments )
                cout << "\texpr(" << expr[j] << ")" << endl;
              if ( boost::regex_search(str, what, expr[j]) )
                {
                  vartype[i] = j;
                  argsregex += delim + atype[j];
                  break;
                }
            }
        }
      // Make sure we had a match
      if ( vartype[i] < 0 )
        {
          throw cms::Exception("ArgDecodeFailure")
            << "rfx::decodeArguments - failed on: " << args << endl;
        }
      delim = ", ";

      //DB
      if ( DBdecodeArguments )
        cout << "\ttype: " << vartype[i] << endl;

      string s;
      double d;
      long l;
      bool b;
      istringstream inp(str);

      switch (vartype[i])
        {
        case 0:
          str = str.substr(1, str.size()-2); // remove quotes
          vars.push_back(new rfx::Value<string>(str));
          break;

        case 1:
          inp >> d;
          vars.push_back(new rfx::Value<double>(d));
          break;

        case 2:
          inp >> l;
          vars.push_back(new rfx::Value<long>(l));
          break;

        case 3:
          inp >> b;
          vars.push_back(new rfx::Value<bool>(b));
          break;

        case 4:
          break;
        };
    }
  //if ( argsregex != "" ) argsregex += "[)]";
}


void*
rfx::invokeMethod(FunctionDescriptor& fd, void* address)
{
  void* raddr=0;
  if ( address == 0 ) return raddr;

  // Model instance of class
  Object object(fd.otype, address);

//   // Call method on class instance
  fd.method.Invoke(object, &fd.robject, fd.args);

  // Get address of returned object
  raddr = fd.robject.Address();

  // If this is a pointer or reference, return address of object pointed to
  if ( fd.pointer || fd.reference ) raddr = *static_cast<void**>(raddr);

 return raddr;
}


void*
rfx::datamemberValue(string& classname, void* address, string& membername)
{
  void* raddr=0;
  if ( address == 0 ) return raddr;
  
  // Model instance of class
  Type ctype = Type::ByName(classname);
  Object object(ctype, address);

  // Model datamember and get address of its value
  Object value = object.Get(membername);
  raddr  = value.Address();

  // If this is a pointer or reference, return address of object pointed to
  Type atype = value.TypeOf();
  if ( atype.IsPointer() || atype.IsReference() ) 
    raddr = *static_cast<void**>(raddr);
  return raddr;
}


void
rfx::deallocateMemory(Member& method, void* address)
{
  method.TypeOf().ReturnType().FinalType().Deallocate(address);
}


std::string
rfx::returnTypeName(Member& method)
{
  Type rtype = method.TypeOf().ReturnType().FinalType();
  if ( rtype.IsPointer() )
    return rtype.ToType().Name(SCOPED);
  else
    return rtype.Name(SCOPED);
}

Type
rfx::returnType(Member& method, int& code)
{
  Type rtype = method.TypeOf().ReturnType().FinalType();
  if ( rtype.IsReference() )
    {
      code = 3;
      return rtype;
    }
  else if ( rtype.IsPointer() )
    {
      code = 2;
      return rtype.ToType();
    }
  else
    {
      code = 1;
      return rtype;
    }
}

bool
rfx::returnsPointer(Member& method)
{
  Type rtype = method.TypeOf().ReturnType().FinalType();
  return rtype.IsPointer();
}

Member
rfx::getReturnedObjectMethod(Member& method, std::string name)
{
  string rname = rfx::returnTypeName(method);
  return rfx::getMethod(rname, name);
}

Member
rfx::getisAvailable(Member& method)
{
  return getReturnedObjectMethod(method, "isAvailable");
}

Member
rfx::getisNull(Member& method)
{
  return getReturnedObjectMethod(method, "isNull");
}


void
rfx::decodeMethod(FunctionDescriptor& fd)
{
  if ( DBdecodeMethod )
    cout << "decodeMethod - function( " << fd.expression << " )" << endl;
  
  // Regex to extract name of method
  boost::regex exprName("[a-zA-Z][a-zA-Z0-9_]*(-[>])? *(?=[(])");

  // Regex to extract arguments
  boost::regex exprArgs("(?<=[a-zA-Z0-9>_]) *[(].*[)]");
  
  boost::smatch what;

  // Extract name of method
  if ( !boost::regex_search(fd.expression, what, exprName) )
    {
      throw cms::Exception("RegexFailure")
        << "Method - "
        << "unable to get name from: " 
        << RED << fd.expression << DEFAULT_COLOR << endl;
    }
  string methodname = what[0];
  
  // Extract arguments of method
  if ( !boost::regex_search(fd.expression, what, exprArgs) )
    {
      throw cms::Exception("RegexFailure")
        << "Method - "
        << "unable to decode arguments in method:\n\t" 
        << RED << fd.expression << DEFAULT_COLOR << endl;
    }
  string methodargs = what[0];

  if ( DBdecodeMethod )
    cout << "decodeMethod - args( " << methodargs << " )" << endl;
  
  int nparams = fd.method.FunctionParameterSize();
  for(int i=0; i < nparams; ++i)
    {
      string pname = fd.method.FunctionParameterNameAt(i);
      cout << "\tparam[" << i << "]:\t" << pname << endl; 
    }

  // Now get argument regex
  string argsregex("");
  rfx::decodeArguments(methodargs, 
                       argsregex,
                       fd.values);
  if ( argsregex == "" )
    {
      throw cms::Exception("DecodeFailure")
        << "Method - "
        << "unable to decode arguments: " 
        << RED << methodargs << DEFAULT_COLOR << endl;
    }

  if ( DBdecodeMethod )
    cout << "decodeMethod - argsregex( " << argsregex << ") " << endl;

  // Search for method that matches both name and signature
  fd.method = rfx::getMethod(fd.classname, methodname, argsregex);
  if ( !rfx::memberValid(fd.method) )
    {
      throw cms::Exception("decodMethodeFailure")
        << "\tgetMethod is unable to find a method that matches the"
        << "\n\tregular expression\n\t"
        << RED 
        << methodname 
        << argsregex 
        << DEFAULT_COLOR
        << "\n\tin class: " << BLUE << fd.classname << DEFAULT_COLOR
        << endl;
    }

  // We have a method, so get address of arguments and associated
  // values
  for(unsigned i=0; i < fd.values.size(); i++)
    {
      void* addr = fd.values[i]->address();

      if ( DBdecodeMethod )
        cout << "decodeMethod arg[" << i << "]: " 
             << addr << ": "
             << fd.values[i]->str() << endl;
      fd.args.push_back( addr );
    }
  return;
}

