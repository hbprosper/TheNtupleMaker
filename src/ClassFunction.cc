// -*- C++ -*-
//
// Package:    PhysicsTools
// Class:      ClassFunction
// 
/**\class ClassFunction ClassFunction.cc 
   PhysicsTools/TheNtupleMaker/src/ClassFunction.cc

   Description: model simple or compound methods

   Implementation:
   Common sense and a sense of beauty.
*/
//
// Original Author:  Harrison B. Prosper
//         Created:  Tue Dec  8 15:40:26 CET 2009
//                   Wed Oct 20 HBP - go back to logging all warnings
//                   Sat May 21 2011 HBP - clear memory occupied by objects
//                                         returned by value
// 
// $Id: ClassFunction.cc,v 1.3 2012/05/04 20:54:37 prosper Exp $
//-----------------------------------------------------------------------------
#include <Python.h>
#include <boost/python.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <string>
#include <stdlib.h>
//-----------------------------------------------------------------------------
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "PhysicsTools/TheNtupleMaker/interface/ClassFunction.h"
#include "PhysicsTools/TheNtupleMaker/interface/colors.h"
#include "PhysicsTools/TheNtupleMaker/interface/CurrentEvent.h"
//-----------------------------------------------------------------------------
using namespace std;
using namespace ROOT::Reflex;
//-----------------------------------------------------------------------------
static bool DBClassFunction = getenv("DBClassFunction")>0 ? true : false; 
//static bool FirstCallToFM=true;

namespace {
  string scrunch(string filename)
  {
    filename = rfx::replace(filename, " ", "");
    filename = rfx::replace(filename, ">>", "> >");
    return filename;
  }
}

RunToTypeMap ClassFunction::donotcall = RunToTypeMap();

ClassFunction::ClassFunction()
  : classname_(""),
    expression_("")
{}


ClassFunction::~ClassFunction() 
{ 
  for(unsigned depth=0; depth < fd_.size(); ++depth)
    {
      FunctionDescriptor& fd = fd_[depth]; // NB: get an alias NOT a copy!

      if ( DBClassFunction )
        cout << "       - deallocate memory for: " 
             << BLUE 
             << fd.classname << "::" << fd.expression
             << DEFAULT_COLOR
             << endl;

      fd.rtype.Deallocate(fd.robject.Address());

      for(unsigned j=0; j < fd.values.size(); ++j) delete fd.values[j];
    }
}

// Model an instantiable method using the Reflex tools

ClassFunction::ClassFunction(std::string classname, 
                             std::string expression)
  : classname_(scrunch(classname)),
    expression_(expression)
{
  classname = classname_; // added 25 Apr 2012

//   if ( FirstCallToFM )
//     {
//       FirstCallToFM = false;
//       cout << endl << "\t==> Using ClassFunction to call methods <=="
//            << endl << endl;
//     }

  if ( DBClassFunction )
    cout << endl 
         << "BEGIN ClassFunction - " 
         << classname_ + "::" << expression_ << endl;

  if ( classname_ == "" ) 
    throw cms::Exception("InvalidClassname") << "null classname" << endl;

  if ( expression_ == "" ) 
    throw cms::Exception("InvalidExpression") << "null expression" << endl;

  //--------------------------------------------------------------------------
  // Split method into its parts. Surely, even for CMS, a maximum 
  // indirection depth of 20 is sufficient!
  //--------------------------------------------------------------------------
  int maxDepth = 20;

  // This flag has to be true at the end of this routine, otherwise something
  // is wrong.

  bool done = false;

  //--------------------------------------------------------------------------
  // Note:
  // classname  - type name of parent object
  // expression - method/datamember of parent object to be invoked
  // retname    - type name of returned object or data member
  // fd         - a vector of function descriptors
  //--------------------------------------------------------------------------

  for(int depth=0; depth < maxDepth; ++depth)
    {
      if ( classname == "" )
        throw cms::Exception("InvalidClassname") << "null classname" << endl;

      if ( expression == "" )
        throw cms::Exception("InvalidExpression") << "null expression" << endl;

      // Allocate a descriptor. We use the descriptor to cache everything 
      // that is needed to render the calling of the method as efficient 
      // as possible.

      if ( DBClassFunction )
        cout << BLUE
             << "       - allocate function descriptor " << depth
             << DEFAULT_COLOR << endl; 
      
      fd_.push_back( FunctionDescriptor() );
      FunctionDescriptor& fd = fd_[depth]; // NB: get an alias NOT a copy!
      
      string retname(""); // Name of returned type
      fd.classname   = classname;
      fd.expression  = expression;
      fd.otype       = Type::ByName(fd.classname); // Model parent object
      if ( fd.otype.Name() == "" )
        throw cms::Exception("NullClassName") 
          << "ClassFunction cannot find type information for class "
          << classname
          << " so sadly can't call "
          << expression
          << endl;
      
      fd.datamember  = false;
      fd.simple      = false;
      fd.byvalue     = false;
      fd.pointer     = false;
      fd.reference   = false;
      fd.smartpointer= false;
      fd.isAvailable = false;
      fd.isNull      = false;

      // If method is compound, split it in two and set "expression"
      // to the first part and "expr2" to the remainder

      string delim(""); // delimeter between expression and expr2
      string expr2("");
      if ( rfx::isCompoundMethod(expression, delim) ) 
        rfx::bisplit(expression, fd.expression, expr2, delim);

      if ( DBClassFunction )
        cout << "       - method/datamember " << ": " 
             << fd.classname << "::" 
             << BLUE << fd.expression << DEFAULT_COLOR << endl
             << "       - join (" << delim << ")"<< endl;

      // Determine whether this is a function or data member
  
      boost::regex dregex("^[a-zA-Z_]+[a-zA-Z0-9_:]*[(]");
      boost::smatch dmatch;

      fd.datamember = ! boost::regex_search(fd.expression, dmatch, dregex);

      if ( fd.datamember )    
        {
          //-------------------------------------------------------------------
          // This seems to be a data member
          //-------------------------------------------------------------------
          if ( DBClassFunction )
            cout << "       - is " << RED << "data member" 
                 << DEFAULT_COLOR << endl;

          // Get a model of it

          fd.method = rfx::getDataMember(fd.classname, fd.expression);

          // Fall on sword if we did not find a valid data member

          if ( !rfx::memberValid(fd.method) )
            throw cms::Exception("getDataMemberFailure")
              << " can't decode data member: " 
              << fd.classname << "::" 
              << fd.expression << endl;

          // We have a valid data member. Get type allowing for 
          // the possibility that values can be returned 
          // by value, pointer or reference.
          // 1. by value     - a copy of the object is returned
          // 2. by pointer   - a variable (the pointer) containing the 
          //                   address of the object is returned
          // 3. by reference - the object itself is returned

          fd.rtype = fd.method.TypeOf();     // Model type of data member
          if ( fd.rtype.IsFundamental() )
            fd.simple = true;

          if ( fd.rtype.IsPointer() )
            fd.pointer = true;

          else if ( fd.rtype.IsReference() )
            fd.reference = true;

          fd.byvalue = !(fd.pointer || fd.reference);

          // Get type name of data member
          // Note: for data members, the rtype variable
          // isn't the final type in the sense that it
          // could still include the "*" or "&" appended to the
          // type name. However, for methods rtype is the final
          // type.
            
          fd.rname = fd.rtype.Name(SCOPED+FINAL);
          if ( DBClassFunction )
            cout << "       - datamember type: " 
                 << BLUE
                 << retname
                 << DEFAULT_COLOR << endl;

          if ( fd.pointer || fd.reference )
            // remove "*" or "&" at end of name
            fd.rname = fd.rname.substr(0, fd.rname.size()-1);

          // Fall on sword if we cannot get data member type name

          if ( fd.rname == "" )
            throw cms::Exception("datamemberTypeFailure")
              << " can't get type for data member "
              << fd.classname << "::"
              << fd.expression << endl;

          if ( DBClassFunction )
            cout << "       - datamember type (confirmation): " 
                 << fd.rname << endl;
        }
      else
        {
          //-------------------------------------------------------------------
          // This seems to be a method
          //-------------------------------------------------------------------
          if ( DBClassFunction )
            cout << "       - is " << RED << "method " 
                 << DEFAULT_COLOR << endl;
          
          // Decode method and return a Reflex model of it in fd.method

          rfx::decodeMethod(fd);

         // Fall on sword if we did not find a valid method

          if ( !rfx::memberValid(fd.method) )
            throw cms::Exception("decodeMethodFailure")
              << " can't decode method: " 
              << fd.classname << "::" 
              << fd.expression << endl;

          // We have a valid method so get a model of its return type
          // Note: again, allow for the possibility that the value can be
          // returned by value, pointer or reference.

          fd.rtype   = fd.method.TypeOf().ReturnType().FinalType();
          fd.simple  = fd.rtype.IsFundamental();
          fd.pointer = fd.rtype.IsPointer();
          fd.reference = fd.rtype.IsReference();
          fd.byvalue = !(fd.pointer || fd.reference);

          if ( fd.pointer ) fd.rtype = fd.rtype.ToType();

          // Get type name of returned object

          fd.rname = fd.rtype.Name(SCOPED+FINAL);
          if ( fd.rname == "" )
            throw cms::Exception("returnTypeFailure")
              << " can't get return type for method " 
              << fd.method.Name() << endl;

          
          if ( DBClassFunction )
            cout << "       - return type: " << fd.rname << endl;

          // This could be an isAvailable method

          boost::regex aregex("^isAvailable[(]");
          boost::smatch amatch;
          fd.isAvailable = boost::regex_search(fd.expression, amatch, aregex);

          // This method could be an isNull method

          boost::regex nregex("^isNull[(]");
          boost::smatch nmatch;
          fd.isNull = boost::regex_search(fd.expression, nmatch, nregex);
        }

      //-----------------------------------------------------------------------
      // We have a valid method or data member. 
      //-----------------------------------------------------------------------

      // The return type or data member could be a smart pointer
      if ( !fd.simple)
        {
          Member m = rfx::getisNull(fd.method);
          fd.smartpointer = rfx::memberValid(m);
        }

      if ( fd.smartpointer )
        {
          // The data member or the return type is a smart pointer, so
          // insert a call to isAvailable()

          expr2 = string("isAvailable()") + delim + expr2;

          if ( DBClassFunction )
            cout << "       - return type: " 
                 << RED << "smart pointer" 
                 << DEFAULT_COLOR << endl;
        }
      else if ( fd.isAvailable )
        {
          // This is an isAvailable method, so insert a call to isNull

          fd.rname = fd.classname; // same classname as current smart pointer
          expr2 = string("isNull()") + delim + expr2;
        }
      else if ( fd.isNull )
        {
          // This is an isNull method, so insert a call to get

          fd.rname = fd.classname; // same classname as current smart pointer
          expr2 = string("get()") + delim + expr2;
        }

      // Memory is needed by Reflex to store the return values from functions.
      // We need to reserve the right amount of space for each object
      // returned, which could of course be a fundamental (that is, simple)
      // type. We free all reserved memory in ClassFunction's destructor.

      fd.robject = Object(fd.rtype, fd.rtype.Allocate());

      // set return type code

      fd.rcode = Tools::FundamentalType(fd.rtype);
      if ( DBClassFunction )
           cout << "       - return code: " 
                 << RED << fd.rcode 
                 << DEFAULT_COLOR << endl;

      // If the return type is simple, then we need to break out of this
      // loop because the analysis of the method is complete. However, if
      // the method is either isAvailable or isNull we must continue.
            
      if ( fd.simple )
        {
          if ( !fd.isAvailable )
            {
              if ( !fd.isNull )
                {
                  // This ClassFunction should always arrive here!
                  done = true;
                  if ( DBClassFunction )
                    cout << "END ClassFunction - " 
                         << classname_ + "::" << expression_ << endl << endl;
                  break;
                }
            }
        }

      // The return type is not simple or the method is either isAvailable or
      // isNull. We therefore, need to continue: the 2nd part of the compound 
      // method becomes the expression on the next round and the return 
      // type becomes the next classname.
 
      expression = expr2;
      classname  = fd.rname;
    }

  if ( ! done )
    throw cms::Exception("ClassFunctionFailure")
      << " **** I can't understand this method: " 
      << classname_ << "::" << expression_ << endl
      << " **** make sure it returns a simple type"
      << endl;
}

bool 
ClassFunction::doNotCall(FunctionDescriptor& fd)
{
  const edm::Event* event = CurrentEvent::instance().get();
  if ( event == 0 ) return false;
  int run = event->id().run();
  if ( ClassFunction::donotcall.find(run) != 
       ClassFunction::donotcall.end() )
    {
      if ( ClassFunction::donotcall[run].find(fd.rname) !=
           ClassFunction::donotcall[run].end() )

        if ( DBClassFunction )
          cout << "==> Skipping method " 
               << RED 
               << classname_ << "::" 
               << expression_
               << DEFAULT_COLOR
               << endl;
      return true;
    }
  return false;
}

void
ClassFunction::updatedoNotCall(FunctionDescriptor& fd)
{
  const edm::Event* event = CurrentEvent::instance().get();
  if ( event == 0 ) return;
  int run = event->id().run();
  if ( ClassFunction::donotcall.find(run) == 
       ClassFunction::donotcall.end() )
    {
      ClassFunction::donotcall[run] = map<string, int>();
    }
  ClassFunction::donotcall[run][fd.rname] = 0;
  if ( DBClassFunction )
    cout << "==> Adding " << RED << fd.rname << DEFAULT_COLOR 
         << " to doNotCall list" << endl;
}

double
ClassFunction::invoke(void* address)
{
#ifdef DEBUG
  if ( DBClassFunction )
    cout << "BEGIN ClassFunction::invoke" << endl;
#endif

  void* raddr = 0;
  double value= 0;
  long double longvalue=0;
  value_ = 0;
  longvalue_ = 0;


  // Keep track of objects returned by value because we need to
  // call their destructors

  vector<FunctionDescriptor*> condemned;

  // Loop over each part of method

  for(unsigned int depth=0; depth < fd_.size(); ++depth)
    {
      FunctionDescriptor& fd = fd_[depth]; // NB: get an alias NOT a copy!

//       // Check the return type of current method. If it points to an object
//       // that is on the doNotCall list for the current run, then don't call
//       // this method. The assumption is that if a collection is missing
//       // it is missing for the entire run.

//       if ( fd.pointer || fd.smartpointer )
//         {
//           if ( doNotCall(fd) ) 
//             {
//               raddr = 0;
//               value_ = 0;
//               longvalue_ = 0;
//               break;
//             }
//         }

      execute(fd, address, raddr, value, longvalue);
 
      if ( fd.simple )
        {
          //---------------------------------------
          // Fundamental return type
          //---------------------------------------
          // This is a fundamental type returned from
          // either a regular method or:
          // 1. a bool from the isAvailable() method of a smart pointer
          // 2. a bool from the isNull() method of a smart pointer
#ifdef DEBUG
          if ( DBClassFunction )
            cout << "\tClassFunction::invoke - FUNCTION:     " 
                 << BLUE << fd.method.Name() << DEFAULT_COLOR << endl
                 << "\t                       - RETURN TYPE:  " 
                 << RED << "FUNDAMENTAL" << DEFAULT_COLOR << endl
                 << "\t                       - RETURN VALUE: "
                 << RED
                 << value_
                 << DEFAULT_COLOR << endl;
#endif
          // This could be an isAvailable method. If so,
          // check its return value

          if ( fd.isAvailable )
            {
              bool available = (bool)value;
              if ( available )
                {
#ifdef DEBUG
                  if ( DBClassFunction )
                    cout << "\tClassFunction::invoke - isAvailable returns: " 
                         << RED << "TRUE" << DEFAULT_COLOR << endl;
#endif
                }
              else
                {
                  // The collection is not available, so return a null pointer
                  edm::LogWarning("CollectionNotFound") << "\t" 
                                                        << classname_ 
                                                        << "::" 
                                                        << expression_ 
                                                        << endl;  
                  value = 0;
                  //updatedoNotCall(fd);
                  break; // break out of loop
                }
            }

          // This could be an isNull method. If so, check its return value

          else if ( fd.isNull )
            {
              bool null = (bool)value;
              if ( null )
                {
                  // The collection is not available, so return a null pointer
                  edm::LogWarning("NullSmartPointer") << "\t" 
                                                      << classname_ 
                                                      << "::" 
                                                      << expression_ 
                                                      << endl;  
                  value = 0;
                  break; // break out of loop
                }
              else
                {
#ifdef DEBUG
                  if ( DBClassFunction )
                    cout << "\tClassFunction::invoke - isNull returns: " 
                         << RED << "FALSE" << DEFAULT_COLOR << endl;
#endif
                }
            }
        }
      else 
        {
          //---------------------------------------
          // Non-fundamental return type
          //---------------------------------------

          // Keep track of objects returned by value. We need to call
          // their destructors explicitly.
          if ( fd.byvalue ) condemned.push_back(&fd);
          
#ifdef DEBUG
          if ( DBClassFunction )
            cout << "\tClassFunction::invoke - FUNCTION:       " 
                 << BLUE << fd.method.Name() << DEFAULT_COLOR << endl
                 << "\t                       - RETURN TYPE:    " 
                 << RED << "NON-FUNDAMENTAL" << DEFAULT_COLOR << endl
                 << "\t                       - RETURN ADDRESS: "
                 << RED
                 << raddr
                 << DEFAULT_COLOR << endl;
#endif
          if ( fd.pointer )
            {
              if ( raddr == 0 )
                {
                  edm::LogWarning("NullPointer") << "\t" 
                                                 << classname_ 
                                                 << "::" 
                                                 << expression_ 
                                                 << endl; 
                  value = 0;
                  break; // break out of loop
                }
            }

          // Return address becomes object address in next call
          address = raddr;
        }
    }

  // Cache return value
  raddr_     = raddr;
  value_     = value;
  longvalue_ = longvalue;

  // Explicitly destroy objects that were returned by value
  for(unsigned int i=0; i < condemned.size(); ++i)
    {
      // get a reference not a copy
      FunctionDescriptor& fd = *condemned[i];

      // call object's destructor, but keep memory reserved when
      // ClassFunction was initialized.
      fd.rtype.Destruct(fd.robject.Address(), false);
    }

#ifdef DEBUG
  if ( DBClassFunction )
    cout << "END ClassFunction::invoke" << endl << endl;
#endif

  return value_;
}

void* 
ClassFunction::raddress() { return raddr_; }

void
ClassFunction::execute(FunctionDescriptor& fd, 
                       void*   address, 
                       void*&  raddr,
                       double& value,
                       long double& longvalue)
{
  // address     address of object whose method/data member is being called
  // raddr       address of return object or value
  
  
#ifdef DEBUG
      if ( DBClassFunction )
        cout << "\tClassFunction::execute: " 
             << fd.classname << "::" 
             << fd.expression << endl;
#endif

  if ( fd.datamember )
    raddr = rfx::datamemberValue(fd.classname, address, fd.expression);
  else
    raddr = rfx::invokeMethod(fd, address);

  // If address is zero, bail out

  if ( raddr == 0 ) return;

  // If the function does not return a fundamental type then just return

  if ( fd.rcode == kNOTFUNDAMENTAL ) return;

  // Ok the function's return type is fundamental, so map it to a double
  // or a long double

  switch( fd.rcode )
    {
      // most common fundamental types
    case kDOUBLE:
      value = *static_cast<double*>(raddr);
      longvalue = value;
      break;
	
    case kFLOAT:
      value = static_cast<double>(*static_cast<float*>(raddr));
      longvalue = value;
      break;
      
    case kINT:
      value = static_cast<double>(*static_cast<int*>(raddr));
      longvalue = value;
      break;	  
      
    case kUNSIGNED_INT:
      value = static_cast<double>(*static_cast<unsigned int*>(raddr));
      longvalue = value;
      break;	
      
    case kUNSIGNED_SHORT_INT:
      value = static_cast<double>(*static_cast<unsigned short int*>(raddr));
      longvalue = value;
      break;
      
    case kBOOL:
      value = static_cast<double>(*static_cast<bool*>(raddr));
      longvalue = value;
      break;	    

    case kUNSIGNED_LONG_INT:
      value = static_cast<double>(*static_cast<unsigned long int*>(raddr));
      longvalue = value;
      break;

      // less common simple types

    case kCHAR:
      value = static_cast<double>(*static_cast<char*>(raddr));
      longvalue = value;
      break;
      
    case kSIGNED_CHAR:
      value = static_cast<double>(*static_cast<unsigned char*>(raddr));
      longvalue = value;
      break;
    
    case kSHORT_INT:
      value = static_cast<double>(*static_cast<short int*>(raddr));
      longvalue = value;
      break;	    
    
    case kLONG_INT:
      value = static_cast<double>(*static_cast<long int*>(raddr));
      longvalue = value;
      break;
    
    case kUNSIGNED_CHAR:
      value = static_cast<double>(*static_cast<unsigned char*>(raddr));
      longvalue = value;
      break;
      
      // long longs
      
    case kULONGLONG:
      longvalue = static_cast<long double>
        (*static_cast<unsigned long long*>(raddr));
      break;
    
    case kLONG_DOUBLE:
      longvalue = *static_cast<long double*>(raddr);
      break;	
    case kLONGLONG:
      longvalue = static_cast<long double>(*static_cast<long long*>(raddr));
      break;
        
    default:
      // Should never get here!
      edm::LogWarning("SHOULD_NEVER_GET_HERE") << "\t" 
                                               << fd.classname 
                                               << "::" 
                                               << fd.expression 
                                               << endl; 
    }
}

long double
ClassFunction::invokeLong(void* address) 
{
  invoke(address);
  return longvalue_; 
}

double
ClassFunction::operator()(void* address) { return invoke(address); }

std::string
ClassFunction::str() const
{
  ostringstream os;

  os << classname_ << "::" << expression_ << endl;

  for(unsigned int depth=0; depth < fd_.size(); ++depth)
    {
      const FunctionDescriptor& fd = fd_[depth];
      os << "  " << depth << endl;
      os << "\tclassname:  " << fd.classname << endl;
      os << "\texpression: " << fd.expression << endl;
      if ( fd.values.size() > (unsigned)0 )
        {
          os << "\t\tvalues: " << endl;
          for(unsigned i=0; i < fd.values.size(); i++)
            {
              os << "\t\t" 
                 << i 
                 << "  " << fd.args[i]
                 << ": " << fd.values[i]->str()  << endl;
            }
        }
    }
  return os.str();
}


std::ostream&
operator<<(std::ostream& os, const ClassFunction& o)
{
  os << o.str();
  return os;
}


