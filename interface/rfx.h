#ifndef RFX_H
#define RFX_H
// -*- C++ -*-
//
// Package:    PhysicsTools
// Class:      rfx
// 
/**\class rfx rfx.cc 
   PhysicsTools/TheNtupleMaker/src/rfx.cc

 Description: A class of Reflex utilities. These functions are placed in a 
 class so that Reflex can handle overloading automatically.
 
 Implementation:
     As simple as possible
*/
//
// Original Author:  Harrison B. Prosper
//         Created:  Fri Apr 04 2008
// $Id: rfx.h,v 1.4 2012/05/04 20:54:34 prosper Exp $
//
//$Revision: 1.4 $
//-----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <stdio.h>
//-----------------------------------------------------------------------------
#include "Reflex/Object.h"
#include "Reflex/Base.h"
#include "Reflex/Type.h"
#include "Reflex/Member.h"
//-----------------------------------------------------------------------------
namespace {
  std::vector<void*> DEFARGS;
}

class FunctionDescriptor;

struct rfx
{

  ///
  struct ValueThing
  {
    virtual ~ValueThing() {}
    virtual std::string str()=0;
    virtual void*       address()=0;
  };

  ///
  template <typename X>
  struct Value : public ValueThing
  {
    Value(X& x) : x_(x) {}
    ~Value() {}

    std::string str() 
    {
      std::ostringstream os;
      os << x_;
      return os.str();
    }
    void* address() { return static_cast<void*>( &x_ ); }

    X x_;
  };

  static
  unsigned int   memcheck(std::string program);

  ///
  static
  bool           isCompoundMethod(std::string expression, std::string& delim);


  ///
  static
  std::string    fullname(std::string classname);

  ///
  static
  void           getScopes(std::string classname, 
                           std::vector<std::string>& names, 
                           int depth=0);
  ///
  static
  Reflex::Member getMethod(std::string classname,
                           std::string methodname,
                           std::string args=std::string("(void)"));

  ///
  static
  Reflex::Member getDataMember(std::string classname,
                               std::string membername);
  ///
  static
  bool           memberValid(Reflex::Member& member);

  ///
  static
  bool           returnsPointer(Reflex::Member& method);

  ///
  static
  int            simpleType(std::string name);

  ///
  static
  Reflex::Member getReturnedObjectMethod(Reflex::Member& method,
                                         std::string name);

  ///
  static
  Reflex::Member getisAvailable(Reflex::Member& method);

  ///
  static
  Reflex::Member getisNull(Reflex::Member& method);

  ///
  static
  void           decodeArguments(std::string  args,
                                 std::string& argsregex,
                                 std::vector<rfx::ValueThing*>& values);

  ///
  static
  void           decodeMethod(FunctionDescriptor& fd);

  ///
  static
  void*          datamemberValue(std::string& classname, void* address, 
                                 std::string& membername);
  ///
  static
  void*          invokeMethod(FunctionDescriptor& fd, void* address); 

  ///
  static
  void           deallocateMemory(Reflex::Member& member, void* address);

  ///
  static
  std::string    returnTypeName(Reflex::Member& method);

  ///
  static
  Reflex::Type   returnType(Reflex::Member& method, int& code);

  ///
  static
  std::string    strip(std::string line);

  ///
  static
  std::string    replace(std::string& str, 
                         std::string oldstr, std::string newstr);
  ///
  static
  void           bisplit(std::string  s, 
                         std::string& left, 
                         std::string& right, 
                         std::string  delim,
                         int direction=1);
  ///
  static
  std::vector<std::string> regex_findall(std::string& str, std::string expr);
  
  ///
  static
  std::string              regex_search(std::string& str, std::string expr); 

  ///
  static
  std::string              regex_sub(std::string& str, 
                                     std::string expr, 
                                     std::string rstr); 
};

 /// Describe attributes of a function or data member.
struct FunctionDescriptor
{
  std::string        classname;          /// Name of parent class
  std::string        expression;         /// Function expression
  Reflex::Type       otype;              /// Object type
  Reflex::Member     method;             /// Model of function
  std::vector<rfx::ValueThing*>  values; /// Argument descriptors
  std::vector<void*> args;               /// Arguments
  
  Reflex::Type              rtype;       /// Models return type
  Reflex::EFUNDAMENTALTYPE  rcode;       /// Return type code
  Reflex::Object            robject;     /// Models returned object
  std::string               rname;       /// Name of return type
  
  bool               datamember;     /// True if this is a data member

  bool               simple;         /// True if return type is simple
  bool               byvalue;        /// True if return is by value
  bool               pointer;        /// True if return is by pointer
  bool               reference;      /// True if return is by reference   
  bool               smartpointer;   /// True if return is by smart pointer

  bool               isAvailable;    /// True if this is isAvailable()
  bool               isNull;         /// True if this is isAvailable()
};


#endif
