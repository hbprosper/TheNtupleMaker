// -*- C++ -*-
//
// Package:    PhysicsTools/TheNtupleMaker
// Class:      MethodTBase
// 
/**\class MethodTBase MethodTBase.cc 
   PhysicsTools/TheNtupleMaker/src/MethodBase.cc
   Description: Base class for MethodT

   Model function members, that is, methods.
    This is the base class of the template class MethodT. The functions that 
    can be modeled must return simple types and have either no arguments or
    arguments comprising simple types. The functions can be of the form:
    - y = method1(...)->method2(...) or
    - y = method1(...).method2(...) or
    - y = method1(..).variable

   Implementation:
   Common sense and a sense of beauty.
*/
//
// Original Author:  Harrison B. Prosper
//         Created:  Tue Dec  8 15:40:26 CET 2009
// $Id: MethodTBase.cc,v 1.1.1.1 2011/05/04 13:04:29 prosper Exp $
//
// If using Python, include its header first to avoid annoying compiler
// complaints.
// ---------------------------------------------------------------------------
#include <Python.h>
#include <boost/python/type_id.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <stdlib.h>

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CommonTools/Utils/src/ExpressionPtr.h"
#include "CommonTools/Utils/src/ExpressionBase.h"
#include "CommonTools/Utils/interface/expressionParser.h"
#include "PhysicsTools/TheNtupleMaker/interface/rfx.h"
#include "PhysicsTools/TheNtupleMaker/interface/MethodTBase.h"
#include "TROOT.h"
// ---------------------------------------------------------------------------
using namespace ROOT::Reflex;
using namespace std;
// ---------------------------------------------------------------------------
namespace {
  // Ctrl[attribute;foreground;backgroundm
  // \x1b 0,1,...   30+color   40+color
  const std::string BLACK("\x1b[0;30;48m");
  const std::string RED("\x1b[0;31;48m");
  const std::string GREEN("\x1b[0;32;48m");
  const std::string YELLOW("\x1b[0;33;48m");
  const std::string BLUE("\x1b[0;34;48m");
  const std::string MAGENTA("\x1b[0;35;48m");
  const std::string CYAN("\x1b[0;36;48m");
  const std::string WHITE("\x1b[0;37;48m");
}

MethodTBase::MethodTBase() {}
  
MethodTBase::~MethodTBase() {}

MethodTBase::MethodTBase(std::string classname, 
                         std::string expression,
                         reco::parser::ExpressionPtr expr)
    : classname_(classname),
      expression_(expression),
      expr1_(expr),
      expression1_(""),
      expression2_(""),
      compoundMethod_(false),
      checkReturn_(false),
      checkisNull_(false),
      callisAvailable_(""),
      callisNull_(""),
      setReturnAddress_("double* x = (double*)0x%x"),
      setObjectAddress_("")
{
  if ( getenv("DEBUGMETHOD") > 0 )
    debug_ = atoi(getenv("DEBUGMETHOD"));
  else
    debug_ = 0;
  
  if ( debug_ > 0 )
    std::cout << "Expression( " 
              << BLUE << expression_ <<  BLACK << " )" << std::endl;
  
  std::string delim("");
  compoundMethod_ = rfx::isCompoundMethod(expression_, delim);
  
  // If method is compound, use CINT to evaluate it
  // otherwise use the expression parser
    
  if ( compoundMethod_ )
    {
      if ( debug_ > 0 )
        std::cout << "Method - " 
                  << RED << "compound method" 
                  << BLACK 
                  << std::endl;
     
      if ( debug_ > 0 )
        std::cout << "         delimeter: (" << delim << ")" << std::endl;
      
      rfx::bisplit(expression_, expression1_, expression2_, delim);
      
      // Get return type of 1st part of compound method
      // and determine if it is a pointer 

      FunctionDescriptor fd;
      fd.classname = classname_;
      fd.expression= expression1_; // part 1 of a possible compound method

      rfx::decodeMethod(fd);

      if ( !rfx::memberValid(fd.method) )
        throw cms::Exception("memberValidFailure") 
          << "I'm too stupid to decode \"" 
          << fd.expression
          << "\"\n" << endl;
      
      // buffer for commands
      char cmd[1024];

      // Command to set address of object
      sprintf(cmd, "gROOT->Reset(); %s* o = (%s*)0x%s", 
              classname_.c_str(), classname_.c_str(), "%x");
      setObjectAddress_ = std::string(cmd);
      
      // Check return type 
      if ( rfx::returnsPointer(fd.method) )
        {
          // Returns a simple pointer
          checkReturn_ = true;
          sprintf(cmd, "o->%s", expression1_.c_str());
        }
      else
        {
          // Check for return of objects with an isNull() method
          Reflex::Member isNull = rfx::getisNull(fd.method); 
          if ( rfx::memberValid(isNull) )
            {
              checkReturn_ = true;
              checkisNull_ = true;
 
              sprintf(cmd, "o->%s.isAvailable()", expression1_.c_str());
              callisAvailable_ = std::string(cmd);

              sprintf(cmd, "o->%s.isNull()", expression1_.c_str());
              callisNull_ = std::string(cmd);
            }
          else
            {
              sprintf(cmd, "%s", "");
            }
        }

      callMethod1_ = std::string(cmd);      
      sprintf(cmd, "*x = o->%s", expression_.c_str());
      callCompoundMethod_ = std::string(cmd);
    }
  else
    {
      if ( debug_ > 0 )
        std::cout << "Method - " 
                  << RED << "simple method" 
                  << BLACK 
                  << std::endl;
      
      // This is a simple method, so use
      // expression parser to evaluate it
      
      expression1_ = expression_;
    }

  if ( debug_ > 0 )
    std::cout << "Method - 1: " << expression1_ << std::endl
              << "         2: " << expression2_ << std::endl;
}

std::string MethodTBase::name() const
{
  return classname_ + "::" + expression_;
}

double MethodTBase::invoke(ROOT::Reflex::Object& object, void* address)
{
  if ( !compoundMethod_ )
    {
      if ( debug_ > 0 )
        //DB
        std::cout << "simpleMethod(" << classname_ << "): " 
                  << GREEN 
                  << expression_
                  << BLACK
                  << std::endl;

      double x = expr1_->value(object);
      
      if ( debug_ > 0 )
        //DB
        std::cout << "  return: " 
                  << RED 
                  << x
                  << BLACK
                  << std::endl;
      return x;
    }
  else
    {
      if ( debug_ > 0 )
        std::cout << "compoundMethod(" << classname_ << "): " 
                  << RED 
                  << expression_
                  << BLACK
                  << std::endl
                  << " setObjectAddress: " 
                  << RED 
                  << setObjectAddress_
                  << BLACK
                  << std::endl;
      
      // Initialize pointer to address of object
      gROOT->ProcessLine(Form(setObjectAddress_.c_str(), address));
      
      if ( checkReturn_ )
        {          
          if ( checkisNull_ )
            {
              if ( debug_ > 0 )
                {
                  //DB
                  std::cout << "   call isAvailable():" << endl
                            << RED
                            << "\t\t"
                            << callisAvailable_
                            << BLACK
                            << std::endl;
                  gROOT->ProcessLine(callisAvailable_.c_str());
                }

              // check if collection is available
              bool available =(bool)gROOT->ProcessLineFast(callisAvailable_.
                                                             c_str());
              if ( ! available )
                {
                  edm::LogWarning("CollectionNotFound") 
                    << "\t" << classname_ << "::" << expression_;
                  return 0;
                }

              if ( debug_ > 0 )
                {
                  //DB
                  std::cout << "   call isNull():" << endl
                            << RED
                            << "\t\t"
                            << callisNull_
                            << BLACK
                            << std::endl;
                  gROOT->ProcessLine(callisNull_.c_str());
                }

              bool null =(bool)gROOT->ProcessLineFast(callisNull_.c_str());
              
              if ( debug_ > 0 )
                //DB
                std::cout << "        return: " 
                          << GREEN 
                          << null
                          << BLACK
                          << std::endl;

              if ( null )
                {
                  edm::LogWarning("NullSmartPointer") 
                    << "\t" << classname_ << "::" << expression_;
                  return 0;
                }
            }
          else
            {
              void* y = 0;
              y = (void*)gROOT->ProcessLineFast(callMethod1_.c_str());
              if ( y == 0 ) 
                {
                  edm::LogWarning("NullPointer") 
                    << "\t" << classname_ << "::" << expression_;
                  return 0;
                }
            }
        }
      
      // We have a valid pointer, so proceed

      double x = 0;
      gROOT->ProcessLine(Form(setReturnAddress_.c_str(), &x));
      
      if ( debug_ > 0 )
        {
          //DB
          std::cout << "     callCompound:" << endl 
                    << RED 
                    << "\t\t"
                    << setReturnAddress_ << endl
                    << "\t\t"
                    << callCompoundMethod_
                    << BLACK
                    << std::endl;
          gROOT->ProcessLine(callCompoundMethod_.c_str());
        }

      try
        {
          gROOT->ProcessLineFast(callCompoundMethod_.c_str());
        }
      catch (...)
        {}
      
      if ( debug_ > 0 )
        //DB
        std::cout << "       return: " 
                  << RED 
                  << x
                  << BLACK
                  << std::endl;
      return x;
    }
}
