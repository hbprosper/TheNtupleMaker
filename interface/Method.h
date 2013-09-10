#ifndef METHOD_H
#define METHOD_H
//-----------------------------------------------------------------------------
//
// Package:    PhysicsTools/TheNtupleMaker
//             Method.h
//
// Original Author:  Harrison B. Prosper
//         Created:  Tue Dec  8 15:40:26 CET 2009
// $Id: Method.h,v 1.2 2011/05/23 08:46:57 prosper Exp $
//
// If using Python, include its header first to avoid annoying compiler
// complaints.
//-----------------------------------------------------------------------------
#include <Python.h>
#include <boost/python/type_id.hpp>
#include <string>
#include <iostream>
#include "PhysicsTools/TheNtupleMaker/interface/ClassFunction.h"
//-----------------------------------------------------------------------------
/// Model function methods.

template <typename T>
class Method : public ClassFunction
{
public:
  ///
  Method() {}
  
  ///
  Method(std::string expression)
    : ClassFunction(boost::python::type_id<T>().name(), expression),
      name_(boost::python::type_id<T>().name()+std::string("::")+expression)
  {}
  
  virtual ~Method() {}

  ///
  std::string name() const { return name_; }
  
  ///
  double operator()(const T& object) {return invoke((void*)(&object)); }
  
  ///
  double operator()(T& object) { return (*this)((const T)(object)); }
  
private:
  std::string name_;
};

#endif
