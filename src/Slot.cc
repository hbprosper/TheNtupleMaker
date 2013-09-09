///////////////////////////////////////////////////////////////////////
// File:    Slot.cc
// Purpose: Models a slot to which a signal can be connected.
// Created: Summer-2002 Harrison B. Prosper
// Updated: 05-Jun-2008 HBP Adapt to CMS
//          14-Apr-2011 HBP use unsigned long
///////////////////////////////////////////////////////////////////////
//$Revision: 1.2 $

#include <Python.h>
#include <iostream>
#include <iomanip>
#include "PhysicsTools/TheNtupleMaker/interface/Slot.h"

using namespace std;

// Make this class known to Root

#ifdef __WITH_CINT__
ClassImp(Slot)
#endif

// RootCint requires a default constructor

Slot::Slot() {}

Slot::Slot(PyObject* object, const char *method)  	   
  : _object(object),
    _mstr(method),
    _method(std::vector<char>(_mstr.size()+1,0))
{
  copy(_mstr.begin(), _mstr.end(), _method.begin());
}

Slot::~Slot() 
{}

// SLOTS
////////

void Slot::handleSignal(int id)
{
  char ip[4] = {"(i)"};

  PyObject* result = PyObject_CallMethod(_object, &_method[0], ip, id);
  if ( PyErr_Occurred() ) PyErr_Clear();

  // Decrement reference count. Use XDECREF to ignore NULL references
  
  Py_XDECREF(result);
}

void Slot::handleSignal()
{
  PyObject* result = PyObject_CallMethod(_object, &_method[0], NULL);
  if ( PyErr_Occurred() ) PyErr_Clear();

  // Decrement reference count. Use XDECREF to ignore NULL references
  
  Py_XDECREF(result);
}


