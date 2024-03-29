#ifndef THENTUPLEMAKER_H
#define THENTUPLEMAKER_H
/**
   PhysicsTools/TheNtupleMaker
   Description: Automatically create an n-tuple from CMS EDM files.
*/
//
// Original Author:  Sezen SEKMEN & Harrison B. Prosper
//         Created:  Tue Dec  8 15:40:26 CET 2009
//         Updated:  Sun Jan 17 HBP - add log file
//                   Sun Jun 06 HBP - add variables.txt file
//                   Sun Nov 07 HBP - add event setup to fill
//                   Sat Mar 12 2011 HBP - change selectorname to usermacroname
//                   Wed May 04 2011 HBP - change name to TheNtupleMaker
//                   Fri Jun 24 2011 HBP - get provenance info using 
//                                   getenv
//                   Wed Jul 20 2011 HBP - add trigger configuration set up
//                                   in beginRun()
//                   Fri Jul 22 2011 HBP - make buffer name and get by label
//                                   available to buffers
//                   Mon Aug 08 2011 HBP - allow global alias
//                   Sun May 06 2012 HBP - fix macro invokation
//                   Mon May 07 2012 HBP - remove SelectedObjectMap.h 
//                   Sun May 13 2012 HBP - move creation of buffers to
//                                   beginRun()
//                                   Implement wildcard for triggers and
//                                   range processing for all variables
//                   Thu Jul 04 2013 HBP - fix variables.txt
//                   Wed Dec 17 2014 HBP - split branchname creation from
//                                   buffer creation (in anticipation of
//                                   move to Root 6)
//
//                   Thu Oct 08 2020 HBP - rebuild using ROOT JIT compiler
//                   Thu Oct 15 2020 HBP - make it possible to store bools.
// $Id: TheNtupleMaker.cc,v 1.23 2013/07/05 07:15:14 prosper Exp $
// ---------------------------------------------------------------------------
#include <boost/regex.hpp>
#include <boost/python/type_id.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "PhysicsTools/TheNtupleMaker/interface/colors.h"
#include "PhysicsTools/TheNtupleMaker/interface/tnmutil.h"

#include "TROOT.h"
#include "TTree.h"
#include "TInterpreter.h"
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
namespace {
  const std::string TNM_VERSION("7.0.2 December 2021");
  char TNM_RECORD[10000];
  int DEBUG(0);
};
class VariableDescriptor;
// ----------------------------------------------------------------------------
class TheNtupleMaker : public edm::EDAnalyzer 
{
public:
  explicit TheNtupleMaker(const edm::ParameterSet&);
  virtual ~TheNtupleMaker();
  virtual void beginJob();
  virtual void beginRun(const edm::Run&, const edm::EventSetup&);
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();

  template <typename T>
  edm::EDGetTokenT<T> getToken(std::string label)
  {
    // Split Label into its component parts
    std::string label1 = label;
    std::string label2("");
    int i = label.find("_");
    if ( i > 0 )
      {
	label1 = label.substr(0,i);      
	label2 = label.substr(i+1, label.size()-i-1);
      }
    return consumes<T>(edm::InputTag(label1, label2));
  }

  TTree* getTree();
  
private:
  bool selectEvent(const edm::Event& iEvent);
  void updateTriggerBranches(int blockindex);
  void createBranchnames(std::string blockName,  
			 std::string prefix,
			 int maxcount,
			 std::vector<VariableDescriptor>& var,
			 std::ofstream& vout,
			 std::map<std::string, int>& bcount);
};
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// The two abstract classes below are needed to obtain polymorphic behavior
// for Buffers and Variables.

/// Abstract base class for Variable objects.
struct VariableThing
{
  virtual ~VariableThing() {};
  virtual void init(TTree* tree)=0;
  virtual void get(const void* address)=0;
  static int count;
};
int VariableThing::count = 0;

class TheNtupleMaker;

/// Abstract base class for Buffer objects.
struct BufferThing
{
  virtual ~BufferThing() {}
  virtual void add(VariableThing* v)=0;
  virtual void init(TheNtupleMaker* eda, std::string label)=0;
  virtual void get(const edm::Event& event)=0;
  static int count;
};
int BufferThing::count = 0;

/// Descriptor for variable: return type, method (and signature) and branchname.
struct VariableDescriptor
{
  VariableDescriptor()
    : rtype(""),
      method(""),
      varname(""),
      branchname(""),
      maxcount(0)
  {}

  VariableDescriptor(std::string r, std::string m, std::string v)
    : rtype(r),
      method(m),
      varname(v),
      branchname(""),
      maxcount(0)
  {}

  ~VariableDescriptor() {}

  std::string rtype;       /// Return type of method
  std::string method;      /// Method plus signature
  std::string varname;     /// Name of variable derived from method
  std::string branchname;  /// Name of branch associated with method
  int         maxcount;    /// Maximum count associated with method
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// We need a few templates to make the code generic. 
// ----------------------------------------------------------------------------

/** Model a variable.
    X     - object type (this could also be a simple type or a vector of such)
    RTYPE - return type
*/
template <typename X, typename RTYPE>
  struct Variable : public VariableThing
{

  virtual ~Variable() {}
  
  /** 
      @brief Model a variable as an object comprising a method, a branch name 
      and a mechanism for calling the method.
      @param method_      - name of getter method
      @param maxcount_    - maximum number of values/variable
      @param branchname_  - name of branch associated with getter
   */
  Variable(std::string method_, int maxcount_, std::string branchname_)
  {
    // create name of getter class
    sprintf(TNM_RECORD, "Getter%d", count);
    getter_classname  = std::string(TNM_RECORD);

    // create name of getter instance
    sprintf(TNM_RECORD, "getter%d", count);
    getter_objectname = std::string(TNM_RECORD);

    // value is the buffer to receive return data from getter
    value      = std::vector<RTYPE>(maxcount_);

    method     = method_;
    maxcount   = maxcount_;
    branchname = branchname_;
    otype      = boost::python::type_id<X>().name();
    rtype      = boost::python::type_id<RTYPE>().name();
    // --------------------------------------------------
    // write getter class to handle call to method.
    // Note, however, that X can be either a simple type, 
    // an object or a vector of simple types or objects.
    // we need sightly different code for each case.
    // --------------------------------------------------
    std::string code = tnm_write_code(getter_classname,
				      getter_objectname,
				      method,
				      otype,
				      rtype,
				      maxcount,
				      count);
    getter_code = code;
    std::ofstream fout(".jit_code.cc");
    fout << code << std::endl;
    fout.close();

    if ( DEBUG < 0 )
      {
	std::cout << BOLDYELLOW 
		  << "--------------------------------------------------------"
		  << DEFAULT_COLOR << std::endl;
	std::cout << BOLDGREEN << code << DEFAULT_COLOR << std::endl;
      }
	
    // --------------------------------------------------
    // compile with JIT compiler and return address of
    // getter class instance
    // --------------------------------------------------
    gROOT->ProcessLine(Form(code.c_str(), &address));

    // --------------------------------------------------
    // get object representing getter class
    // --------------------------------------------------
    getter_class = TClass::GetClass(getter_classname.c_str());
    if ( !getter_class )
      // Have a tantrum!
      throw edm::Exception(edm::errors::Configuration,
			   "cfg error: "
			   + BOLDRED +
			   "unable to get " + getter_classname
			   + DEFAULT_COLOR);

    // --------------------------------------------------
    // get instance of "get" method of getter class
    // --------------------------------------------------
    getter = getter_class->GetMethod("get", "0,0");
    if ( !getter )
      // Have another tantrum!
      throw edm::Exception(edm::errors::Configuration,
			   "cfg error: " + BOLDRED +
			   "unable to get get method of " + getter_classname
    			   + DEFAULT_COLOR);
    // IMPORTANT: update!
    count++;
  }

  /** Execute get method of getter class to retrieve 
      data from specified EDM object.
      @param objaddress - address of EDM object
   */
  virtual void get(const void* objaddress)
  {
    // address of EDM object
    long unsigned int oaddr = (long unsigned int)objaddress;

    // address of buffer to receive data
    long unsigned int vaddr = (long unsigned int)&value;

    // address of getter class instance
    void* object = (void*)address;

    // execute call to "get". since we have already compiled the code
    // above, the call should be fast.
    const void* args[] = {&oaddr, &vaddr};
    gInterpreter->ExecuteWithArgsAndReturn(getter, object, args, 2);
  }

  virtual void init(TTree* tree)
  {
    // assume that maxcount > 1 => a collection and a singleton otherwise
    if ( maxcount > 1 )
      tree->Branch(branchname.c_str(), &value);
    else if ( maxcount == 1 )
      {
	// hack to get around taking the address of a "temporary".
	// while it is true that value can be of zero length, the
	// address of value[0] does not change during the job.
	// therefore, it is safe to use it. the const is needed to
	// make the compiler happy, but non-const is needed to 
	// make ROOT happy!
	const RTYPE& t = value[0]; // a reference not a copy
	tree->Branch(branchname.c_str(), (RTYPE*)&t);
      }
    else
      throw edm::Exception(edm::errors::Configuration,
			   "cfg error: "
			   + BOLDRED +
			   "maxcount < 1 for " + branchname
			   + DEFAULT_COLOR);
  }

  std::string branchname;          /// name of branch associated with method
  std::string otype;               /// object type (could also be a simple type)
  std::string rtype;               /// return type of method
  std::string method;              /// method
  std::string getter_classname;    /// name of getter class
  std::string getter_objectname;   /// name of getter instance
  std::string getter_code;         /// code associated with variable
  int         maxcount;            /// maximum count/variable  
  std::vector<RTYPE> value;        /// buffer for returned values

  TClass*  getter_class;           /// pointer to getter class
  TMethod* getter;                 /// pointer to get method of getter class
  long unsigned int address;       /// adress of getter class instance
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
/**
   @brief A buffer comprises one or more variables, each associated with a 
   method unless the variable is a simple type.
 */
template <typename T>
struct Buffer : public BufferThing
{
  Buffer() :
    tree(0),
    var(std::vector<VariableThing*>()),
    label("")
  {
    count++;
  }
  
  virtual ~Buffer()
  {
  }

  /** 
     @brief Add a Variable to the buffer.
     @param v  Pointer to a Variable.
     Variables are owned by ROOT.
   */
  virtual void add(VariableThing* v)
  {
    var.push_back( v );    
  }
  /**
     @brief Initialize the buffer after all Variables have been added.
  */
  virtual void init(TheNtupleMaker* eda, std::string label_)
  {
    token = eda->getToken<T>(label_);
    tree  = eda->getTree();
    label = label_;
    // initiaize every variable
    for(size_t c=0; c < var.size(); c++) var[c]->init(tree);
  }
  
  /// Get data from associated object by calling specified methods.
  virtual void get(const edm::Event& event)
  {
    edm::Handle<T> object;
    try
      {
	event.getByToken(token, object);
	const T* pobject = &(*object);
	for(size_t c=0; c < var.size(); c++) var[c]->get( pobject );
      }
    catch (...)
      {
	edm::LogWarning("getByTokenFailure") 
	  << RED << "unable to get product "
	  << boost::python::type_id<T>().name()
	  << " with label " << label
	  << DEFAULT_COLOR
	  << std::endl;
      }
  }
  
  edm::EDGetTokenT<T> token;
  TTree* tree;
  std::vector<VariableThing*> var; /// Models variables associated with buffer
  std::string label;
};

#endif
