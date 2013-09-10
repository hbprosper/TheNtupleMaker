#ifndef METHODT_H
#define METHODT_H
//-----------------------------------------------------------------------------
//
// Package:    PhysicsTools/TheNtupleMaker
//             MethodT.h
//
// Original Author:  Harrison B. Prosper
//         Created:  Tue Dec  8 15:40:26 CET 2009
// $Id: MethodT.h,v 1.1.1.1 2011/05/04 13:04:28 prosper Exp $
//
//-----------------------------------------------------------------------------
// If using Python, include its header first to avoid annoying compiler
// complaints.
#include <Python.h>
#include <boost/python/type_id.hpp>
#include <string>
#include "CommonTools/Utils/src/ExpressionPtr.h"
#include "CommonTools/Utils/src/ExpressionBase.h"
#include "CommonTools/Utils/interface/expressionParser.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "PhysicsTools/TheNtupleMaker/interface/MethodTBase.h"
#include "PhysicsTools/TheNtupleMaker/interface/rfx.h"
#include "Reflex/Object.h"
#include "Reflex/Type.h"
//-----------------------------------------------------------------------------
template <typename T>
reco::parser::ExpressionPtr parserPtr(std::string expression)
{
  std::string delim("");
  reco::parser::ExpressionPtr ptr;
  if ( !rfx::isCompoundMethod(expression, delim) )
    if( !reco::parser::expressionParser<T>(expression, ptr) ) 
      {
        std::string message("Since I'm a cyber ignoramous, "
                            "I'm too stupid to understand ");
        message += expression + std::string("\n");
        throw cms::Exception("ExpressionError", message);
      } 
  return ptr;
}

/** Model function members, that is, methods.
    The functions that 
    can be modeled must return simple types and have either no arguments or
    arguments comprising simple types. The functions can be of the form:
    - y = method1(...)->method2(...) or
    - y = method1(...).method2(...) or
    - y = method1(..).variable
*/
template <typename T>
class MethodT : public MethodTBase 
{
public:
  MethodT() {}
  
  ///
  MethodT(const std::string expression)
    : MethodTBase(std::string(boost::python::type_id<T>().name()),
                  std::string(expression),
                  parserPtr<T>(expression)),
      type_(ROOT::Reflex::Type::ByTypeInfo(typeid(T)))
  {}

  ~MethodT() {}

  ///
  double operator()(const T& t)
  {
    ROOT::Reflex::Object object(type_, const_cast<T*>(&t));
    return invoke(object, (void*)(&t));
  }
private:
  ROOT::Reflex::Type type_;
};

#endif
