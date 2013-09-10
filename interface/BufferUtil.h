#ifndef BUFFERUTIL_H
#define BUFFERUTIL_H
// ----------------------------------------------------------------------------
//
// Package:    PhysicsTools/TheNtupleMaker
//             BufferUtil.h
//
// Original Author:  Harrison B. Prosper
//         Created:  Tue Dec  8 15:40:26 CET 2009
//         Updated:  Sun Sep 19 HBP move some code from Buffer.h
//                   Thu Apr 28 HBP for variables not found return
//                   -9999
//                   Sun Apr 22 2012 HBP introduce Caller object and
//                                   ClassType
//                   Thu Jul 04 2013 HBP - add objectname to argument fof init
//                                   by default objectname = name of block
//                                   in config file
//
// $Id: BufferUtil.h,v 1.10 2013/07/05 07:15:14 prosper Exp $
// ----------------------------------------------------------------------------
#include <Python.h>
#include <boost/python/type_id.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "PhysicsTools/TheNtupleMaker/interface/treestream.h"
#include "PhysicsTools/TheNtupleMaker/interface/colors.h"
#include "PhysicsTools/TheNtupleMaker/interface/Method.h"
// ----------------------------------------------------------------------------
struct VariableDescriptor
{
  VariableDescriptor()
    : rtype(""),
      method(""),
      varname("")
  {}

  VariableDescriptor(std::string r, std::string m, std::string v)
    : rtype(r),
      method(m),
      varname(v)
  {}

  ~VariableDescriptor() {}

  std::string rtype;    // return type of method
  std::string method;   // method
  std::string name;     // stripped class name
  std::string varname;  // name derived from method
  int maxcount;
};

struct countvalue
{
  int* count;
  double* value;
};

/// Abstract base class for Buffer objects.
struct BufferThing
{
  virtual ~BufferThing();
      
  /// Initialize the buffer. 
  virtual void init(otreestream& out,
                    std::string objectname, 
                    std::string label, 
                    std::string prefix,
                    std::vector<VariableDescriptor>& var,
                    int  maxcount,
                    std::ofstream& log,
                    std::set<std::string>& branchset,
                    int debug=0)=0;
  
  /// Call requested methods of selected objects and fill buffer.
  virtual bool fill(const edm::Event& event, const edm::EventSetup& esetup)=0;
  ///
  virtual std::string& message()=0;
  ///
  virtual std::string name()=0;
  ///
  virtual void shrink(std::vector<int>& index)=0;
  ///
  virtual countvalue& variable(std::string name)=0;
  ///
  virtual std::vector<std::string>& varnames()=0;
  ///
  virtual int maxcount()=0;
  ///
  virtual int count()=0;
  ///
  virtual std::string key()=0;

};

///
enum BufferType
  {
    EVENT,            // Buffer for event
    RUNINFO,          // Buffer for RunInfo object
    HELPER,           // Buffer that performs complicated accesses
    DEFAULT
  };

///
enum ClassType
  {
    SINGLETON,        // At most one object per event
    COLLECTION,       // vector or SortedCollection
    CONTAINER         // Misc. containers (with size() and operator[](int)
  };


///
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
                      std::string& bufferkey,
                      std::set<std::string>& branchset,
                      int   debug);
// ----------------------------------------------------------------------------
// We need a few templates to make the code generic. 
// WARNING: keep code as short as possible to minimize code bloat due to 
// template instantiation diarrhoea
// ----------------------------------------------------------------------------
/** Model a variable.
    A (CMS) variable is a thing with a<br>
    1 - name
    2 - value (a vector of doubles)
    3 - a function to access data from the associated RECO or PAT object
*/
template <typename X>
struct Variable 
{
  /** Construct a variable.
      @param namen - name of variable
      @param count - maximum number of values/variable
      @param f - name of accessor function
   */
  Variable(std::string namen, int count, std::string f) 
    : name(namen),
      fname(f),
      value(std::vector<double>(count,0)),
      function(Method<X>(f))
  {}

  std::string         name;
  std::string         fname;
  std::vector<double> value;
  Method<X>           function;
};
// ----------------------------------------------------------------------------
template <typename X>
void initBuffer(otreestream& out,
                std::string& objectname,
                std::string& classname,
                std::string& label,
                std::string& label1,
                std::string& label2,
                std::string& prefix,
                std::vector<VariableDescriptor>& var,
                boost::ptr_vector<Variable<X> >&  variables,
                std::vector<std::string>&   varnames,
                std::map<std::string, countvalue>& varmap,
                int&  count,
                ClassType ctype,
                int   maxcount,
                std::ofstream& log,
                std::string&   bufferkey,
                std::set<std::string>& branchset,
                int   debug)
{
  initializeBuffer(out,
                   objectname,
                   classname,
                   label,
                   label1,
                   label2,
                   prefix,
                   var,
                   count,
                   ctype,
                   maxcount,
                   log,
                   bufferkey,
                   branchset,
                   debug);

  // Create a variable object for each method. We use a boost::ptr_vector
  // rather than a vector because a push_back on the latter can trigger
  // calls to the destructor of the pushed object. We don't want this to
  // happen for Variable<X> because it contains an object of type 
  // ClassFunction. A call to Variable<X>'s destructor will trigger a call
  // to the ClassFunction destructer, thereby deallocating memory at the wrong
  // time.

  variables.clear();
  for(unsigned i=0; i < var.size(); i++)
    variables.push_back(new Variable<X>(var[i].name, 
                                        var[i].maxcount,
                                        var[i].method));
  
  // Now add variables to output tree. This must be done after all
  // variables have been defined, because it is only then that their
  // addresses are guaranteed to be stable. (See above comment.)

  // Also cache variable names and addresses

  // We can use vectors for the following because an inadvertent call
  // to a destructor is innocuous.

  boost::regex getname("[^[]+");
  varnames.clear();
  varmap.clear();
  for(unsigned i=0; i < variables.size(); i++)
    {
      std::string fullname = variables[i].name;
      out.add(fullname, variables[i].value);

      boost::smatch m;
      boost::regex_search(fullname, m, getname);
      std::string name = m[0];
      varnames.push_back(name);

      countvalue v;
      v.count = &count;
      v.value = &(variables[i].value[0]);
      varmap[name] = v;
    }

  // Add counter variable
  {
    countvalue v;
    v.count = &count;
    v.value = 0;
    std::string counter = "n" + prefix; 
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

/// Function to handle getByLabel.
template <typename X>
bool getByLabel(const edm::Event& event, 
                X& handle, 
                std::string& label1, 
                std::string& label2, 
                std::string& message,
                BufferType buffertype,
                bool crash)
{ 
  // Try to do a getByLabel and fall on sword if it fails.
  try
    {
      if ( buffertype == RUNINFO )
        event.getRun().getByLabel(edm::InputTag(label1), handle);
      else
        {
          if ( label2 == "" )
            event.getByLabel(edm::InputTag(label1), handle);
          else
            event.getByLabel(edm::InputTag(label1, label2), handle);
        }
    }
  catch (cms::Exception& e)
    {
      // Complain
      std::ostringstream out;
      out << "full of sound and fury, signifying nothing" << std::endl
          << "Buffer::fill - label: \"" 
          << label1 << " " << label2 << "\" - class: " 
          << boost::python::type_id<X>().name() << " " 
          << e.explainSelf();
      edm::LogWarning("getByLabelFailure") << out.str() << std::endl;
      return false;
    }

//     if ( crash )
//       std::cout << "=== CRASH on InvalidHandle (" 
//                 << label1 << "/" << label2 << "/" 
//                 << boost::python::type_id<X>().name() << ")"
//                 << std::endl;
//     else
//       std::cout << "=== WARN on InvalidHandle (" 
//                 << label1 << "/" << label2 << "/" 
//                 << boost::python::type_id<X>().name() << ")"
//                 << std::endl;  

  // getByLabel succeeded, check that we have a valid handle,
  // otherwise complain bitterly
  
  if ( !handle.isValid() )
    {
      std::ostringstream out;
      out << "you blocks you stones you worse than"
          << " senseless things!" << std::endl
          << "Buffer::fill - label: \"" 
          << label1 << " " << label2 << "\" - class: " 
          << boost::python::type_id<X>().name();
      if ( crash )
        throw edm::Exception(edm::errors::Configuration, out.str());
      else
        edm::LogWarning("InvalidHandle") << out.str() << std::endl;
      return false;
    }
  return true;
}

// ----------------------------------------------------------------------------
/// Template function to handle call to methods.
template <typename X, typename Y>
void callMethods(int j, 
                 const X& object, 
                 boost::ptr_vector<Variable<Y> >& variables, 
                 int debug)
{
  for(unsigned i=0; i < variables.size(); i++)
    {
      if ( debug > 0 ) 
        std::cout << "\t" << j << "\tcall: "        
                  << variables[i].fname << std::endl;

      // ====> This is where the call to a method occurs     <====
      // ====> If a method fails, don't crash just complain  <====
      // ====> and continue
      try
        {
          variables[i].value[j] = variables[i].function(object);
        }
      catch (cms::Exception& e)
        {
          edm::LogWarning("MethodCallFailure") 
            << RED 
            << variables[i].function.name()
            << DEFAULT_COLOR 
            << std::endl
            << e.explainSelf()
            << std::endl;
          variables[i].value[j] = -9999; // return weird value
        }
      if ( debug > 0 ) 
        std::cout << "\t\t\tvalue = " 
                  << variables[i].value[j] 
                  << std::endl;
    }
}

// ----------------------------------------------------------------------------
/// Function object to handle getByLabel and callMethods.
template <typename X, typename Y, ClassType>
struct Caller
{
  Caller() {}
  ~Caller() {}

  bool operator()(const edm::Event& event, 
                  std::string& label1, 
                  std::string& label2, 
                  std::string& message,
                  BufferType buffertype,
                  bool crash,
                  boost::ptr_vector<Variable<Y> >& variables,
                  int& count,
                  int& maxcount,
                  int debug)
  {
    return true;
  }
};

// ------------------------------------------------
// Specialized function objects
// ------------------------------------------------
/// Handle singletons.
template <typename X>
struct Caller<X, X, SINGLETON>
{
  Caller() {}
  ~Caller() {}

  bool operator()(const edm::Event& event, 
                  std::string& label1, 
                  std::string& label2, 
                  std::string& message,
                  BufferType buffertype,
                  bool crash,
                  boost::ptr_vector<Variable<X> >& variables,
                  int& count,
                  int& maxcount,
                  int debug)
  {
    edm::Handle<X> handle;
    if ( ! getByLabel(event, handle, label1, label2, message,
                    buffertype, crash) )
      return false;
    
    // OK handle is valid, so extract data for all variables
    const X& object = *handle;
    callMethods(0, object, variables, debug);
    return true;
  }
};

/// Handle vector types
template <typename X>
struct Caller<X, X, COLLECTION>
{
  Caller() {}
  ~Caller() {}
  
  bool operator()(const edm::Event& event, 
                  std::string& label1, 
                  std::string& label2, 
                  std::string& message,
                  BufferType buffertype,
                  bool crash,
                  boost::ptr_vector<Variable<X> >& variables, 
                  int& count,
                  int& maxcount,
                  int debug)
  {
    edm::Handle< edm::View<X> > handle;
    if ( ! getByLabel(event, handle, label1, label2, message,
                      buffertype, crash) )
      return false;
    
    // OK handle is valid, so extract data for all variables.        
    // For the object count, use the smaller of handle size and maxcount.
    count = (int)handle->size() < maxcount ? handle->size() : maxcount;
    
    for(int j=0; j < count; j++)
      {
        const X& object = (*handle)[j];
        callMethods(j, object, variables, debug);
      }
    return true;
  }
};

/// Handle containers.
/**
   X is the container type
   Y is the containee type (the contained type)
 */
template <typename X, typename Y>
struct Caller<X, Y, CONTAINER>
{
  Caller() {}
  ~Caller() {}
  
  bool operator()(const edm::Event& event, 
                  std::string& label1, 
                  std::string& label2, 
                  std::string& message,
                  BufferType buffertype,
                  bool crash,
                  boost::ptr_vector<Variable<Y> >& variables, 
                  int& count,
                  int& maxcount,
                  int debug)
  {
    edm::Handle<X> handle;
    if ( ! getByLabel(event, handle, label1, label2, message,
                      buffertype, crash) )
      return false;
    
    // OK handle is valid, so extract data for all variables.        
    // For the object count, use the smaller of handle size and maxcount.
    count = (int)handle->size() < maxcount ? handle->size() : maxcount;
          
    for(int j=0; j < count; j++)
      {
        const Y& object = (*handle)[j];
        callMethods(j, object, variables, debug);
      }
     return true;
  }
};

#endif
  
