// -*- C++ -*-
//
// Package:    TheNtupleMaker
// Class:      TheNtupleMaker
// 
/**\class TheNtupleMaker TheNtupleMaker.cc 
          PhysicsTools/TheNtupleMaker/src/TheNtupleMaker.cc

 Description: Creates a simple n-tuple from RECO or PAT files.

 Implementation:

     A sense of beauty and common sense

     (A) Introduction

     This analyzer creates simple Root n-tuples using information
     specified in a configuration file. In the configuration file, one
     specifies a buffer for each object type to be read from the input RECO
     or PAT file, the methods to be called and optionally the names that are 
     to be given to the corresponding variables in the output n-tuple. The
     default naming scheme is
     
      stripped-classname + "_" + getByToken label + "." + stripped-method name

     (B) Mechanism

     For each class, e.g., pat::Muon, we allocate a buffer that is
     initialized with the methods to be called. The return
     value of each method called (once, per event, for every instance 
     of an object of a given type) is stored in a variable whose address 
     is known to Root. The buffer object has two methods: init(...) and 
     get(...). The init(...) method tells the buffer which methods of
     the associated class to call. The get(...) method calls these
     methods and caches their return values. The Fill() method of the
     n-tuple object writes the cached values to the n-tuple.

     The buffers are allocated dynamically via the CMS plugin mechanism. (The
     plugin factory and plugins are defined in pluginfactory.cc and 
     plugins*.cc, respectively). The list of buffer objects to be 
     allocated is specified in the configuration file, under the Python list 
     "buffers".
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
//                   Thu Oct 08 2020 HBP - rebuild using ROOT JIT compiler
//                   Thu Dec 09 2021 HBP - save generated code to jit_code.cc
//
// $Id: TheNtupleMaker.cc,v 1.23 2013/07/05 07:15:14 prosper Exp $
// ---------------------------------------------------------------------------
#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cassert>
#include <map>
#include <time.h>
#include <stdlib.h>

#include "PhysicsTools/TheNtupleMaker/interface/TheNtupleMaker.h"
#include "PhysicsTools/TheNtupleMaker/interface/CurrentEvent.h"
#include "PhysicsTools/TheNtupleMaker/interface/Configuration.h"
#include "PhysicsTools/TheNtupleMaker/interface/kit.h"
#include "DataFormats/PatCandidates/interface/Flags.h"

// File service for output ROOT files
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "TStopwatch.h"
// ---------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// These variables/objects can be global. They do not have to be private
// variables of TheNtupleMaker
// ---------------------------------------------------------------------------
namespace {
  // output ROOT file (n-tuple).
  edm::Service<TFileService> fs;
  
  TTree* tree;                     /// Output tree
  
  // allocated buffers, one per object to be read.
  std::vector<BufferThing*> buffers;
  
  TStopwatch swatch;  
  int        maxEvents(-1);
  
  // addresses of buffers
  std::map<std::string, BufferThing*> buffermap;
  
  std::string ntuplename_;
  std::string analyzername_;
  std::string macroname_;
  bool includeLabel_(true);
  
  int count_(0);
  int imalivecount_(1000);
  int logger_(0);
  bool haltlogger_(false);
  bool macroEnabled_(false);
  
  TTree* ptree_;
  //int inputCount_;
  
  // for HLT
  edm::InputTag triggerResults_;
  HLTConfigProvider HLTconfig_;
  bool HLTconfigured(false);
  std::vector<std::string> triggerNames_;
  
  // cache decoded config data
  std::vector<std::map<std::string, std::string> > parameters_;
  std::vector<std::vector<VariableDescriptor> > variables_;

  std::string DirectoryName;

};
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

TheNtupleMaker::TheNtupleMaker(const edm::ParameterSet& iConfig)
{
  if ( getenv("DBTheNtupleMaker") > 0 )
      DEBUG = atoi(getenv("DBTheNtupleMaker"));

  cout << BOLDYELLOW << "\nConfiguring TheNtupleMaker"
       << endl 
       << DEFAULT_COLOR << endl; 

  DirectoryName = iConfig.getParameter<string>("@module_label");

  tree = fs->make<TTree>("Events", 
			 string("created by TheNtupleMaker " 
				+ TNM_VERSION).c_str());

  string config = string(iConfig.dump());

  // --------------------------------------------------------------------------
  // Add a provenance tree to ntuple
  // --------------------------------------------------------------------------
  TFile* file = tree->GetCurrentFile();
  ptree_ = new TTree("Provenance",
                     string("created by TheNtupleMaker " 
			    + TNM_VERSION).c_str());
  string cmsver("unknown");
  if ( getenv("CMSSW_VERSION") > 0 ) cmsver = string(getenv("CMSSW_VERSION"));
  ptree_->Branch("cmssw_version", (void*)(cmsver.c_str()), "cmssw_version/C");

  time_t tt = time(0);
  string ct(ctime(&tt));
  ptree_->Branch("date", (void*)(ct.c_str()), "date/C");

  string hostname("unknown");
  if ( getenv("HOSTNAME") > 0 ) hostname = string(getenv("HOSTNAME"));
  ptree_->Branch("hostname", (void*)(hostname.c_str()), "hostname/C");

  string username("unknown");
  if ( getenv("USER") > 0 ) username = string(getenv("USER"));
  ptree_->Branch("username", (void*)(username.c_str()), "username/C");

  if ( config != "" ) ptree_->Branch("cfg", (void*)(config.c_str()), "cfg/C");

  // Ok, fill this branch
  file->cd();
  ptree_->Fill();

  // --------------------------------------------------------------------------
  // Cache global configuration
  Configuration::instance().set(iConfig);
    
  // Get optional configuration parameters

  try
    {
      triggerResults_ 
        = iConfig.getParameter<edm::InputTag>("HLTriggerResults");
    }
  catch (...)
    {
      triggerResults_ = edm::InputTag("TriggerResults", "", "HLT");
    }

  try
    {
      imalivecount_ = iConfig.getUntrackedParameter<int>("imaliveCount");
    }
  catch (...)
    {}

  // Save Tree header and branch buffers after every autosaveCount Mbytes
  // have been written to file. Default is 100 Mbytes.
  // int autosaveCount = 100;
  // try
  //   {
  //     autosaveCount = iConfig.getUntrackedParameter<int>("autosaveCount");
  //   }
  // catch (...)
  //   {}
  //output.autosave(autosaveCount);

  try
    {
      logger_ = iConfig.getUntrackedParameter<int>("loggerCount");
      haltlogger_ = logger_ >= 0;
    }
  catch (...)
    {}

  // Get optional analyzer name
  try
    {
      analyzername_ = iConfig.getUntrackedParameter<string>("analyzerName");
    }
  catch (...)
    {
      analyzername_ = "";
    }

  // Get name of optional macro (to be replaced with ADL)
  try
    {
      macroname_ = kit::nameonly(iConfig.
                                 getUntrackedParameter<string>("macroName"));
    }
  catch (...)
    {
      macroname_ = "";
    }

  // The default is to include the label in the branch name
  try
    {
      includeLabel_ = iConfig.
	getUntrackedParameter<bool>("includeLabel");
    }
  catch (...)
    {
      includeLabel_ = true;
    }

  
  // --------------------------------------------------------------------------

  macroEnabled_ = macroname_ != "";

  // if ( macroEnabled_ )
  //   {
  //     // Try to load associated shared library 

  //     // First find shared lib
  //     string filestem = string("*") + macroname_ + string("*.so");
  //     string cmd = string("find . -name \"") + filestem + "\"";
  //     string shlib = kit::shell(cmd);
  //     if ( shlib == "" )
  //       {
  //         string errmess = string("\tfind ") + filestem + string(" failed");
  //         // Have a tantrum!
  //         throw cms::Exception("FileNotFound", errmess);
  //       }

  //     // Found shared library, so try to load it

  //     cout << "\t==> Load macro library " << shlib << endl;
      
  //     if ( gSystem->Load(shlib.c_str()) != 0 )
  //        throw cms::Exception("LoadFailed",
  //                             "\tfor shared library\n\t\t" + shlib);

  //     // Create commands to execute macro using CINT

  //     string treecmd("TTree* tree = (TTree*)0x%lx"); 
  //     gROOT->ProcessLine(Form(treecmd.c_str(),(unsigned long)(output.tree())));

  //     string mapcmd("map<string,countvalue>* "
  //                   "varmap = (map<string,countvalue>*)0x%lx");
  //     gROOT->ProcessLine(Form(mapcmd.c_str(), (unsigned long)(&varmap)));

  //     string imapcmd("map<string,vector<int> >* "
  //                    "indexmap = (map<string,vector<int> >*)0x%lx");
  //     gROOT->ProcessLine(Form(imapcmd.c_str(), (unsigned long)(&indexmap)));

  //     string macrocmd = macroname_ + string(" obj(tree,varmap,indexmap);");
  //     gROOT->ProcessLine(macrocmd.c_str());
  //   }
  // --------------------------------------------------------------------------
  // Decode buffer information and cache info. The buffers are created in this
  // constructor. However, some variables for triggers are added in
  // beginRun() as edmTriggerResultsHelper needs at least one event in memory.
  //
  // Each buffer (aka block) descriptor comprises a string containing
  //    className  - EDM class (or simple type)
  //    label      - getByToken iput tag
  //    maxcount   - maximum number of objects to write out. If omitted,
  //                 assumed to be unity and therefore a singleton object.
  //    [prefix]   - prefix for names of n-tuple output variables
  //                 otherwise take prefix = <bufferName>_<label>, where
  //                 bufferName is derived from className: reco::PFJets
  //                 becomes recoPFJets
  //    Followed by a list of methods. However, if className is a simple
  //    type, there are no methods.
  // 
  // Helper methods may optionally contain strings with the format
  //   parameter parameter-name = parameter-value
  //
  // --------------------------------------------------------------------------

  // We need several regular expressions for decoding
  boost::regex getmethod("[a-zA-Z][^ ]*[(].*[)][^ ]*|[a-zA-Z][a-zA-Z0-9]*$");
  boost::smatch matchmethod;

  boost::regex  getparam("^ *param +");
  boost::smatch matchparam;

  boost::regex  getvarprefix("(?<=/)[a-zA-Z0-9]+");
  boost::smatch matchvarprefix;

  boost::regex  getlabel("[a-zA-Z0-9]+(?=/)");
  boost::smatch matchlabel;

  boost::regex  getrange("[0-9]+[.][.]+[0-9]+");
  boost::smatch matchrange;

  boost::regex  getstrarg("(?<=[(]\").+(?=\"[)])");
  boost::smatch matchstrarg;

  boost::regex  getnspace("^(edm|reco|pat|l1extra|trigger)");
  boost::smatch matchnspace;
  // --------------------------------------------------------------------------
  // get list of strings from buffer, decode them, and cache the results
  // NOTE: within TNM, block and buffer are used interchangeably
  // --------------------------------------------------------------------------

  std::vector<std::string> className_;
  std::vector<std::string> blockName_;
  std::vector<std::string> bufferName_;
  std::vector<std::string> label_;
  std::vector<std::string> prefix_;
  std::vector<int> maxcount_;

  vector<string> vrecords = iConfig.
    getUntrackedParameter<vector<string> >("buffers");

  for(unsigned ii=0; ii < vrecords.size(); ii++)
    {
      // The buffer (block) name will be used as the name of the C++ struct, 
      // if one is created.
      string blockName = vrecords[ii];

      if ( DEBUG > 0 ) 
        cout << BOLDWHITE << "BUFFER(" << blockName  << ")" 
	     << DEFAULT_COLOR 
	     << endl; 
      
      // Get description of variables for current buffer
      vector<string> bufferrecords = iConfig.
        getUntrackedParameter<vector<string> >(blockName);

      // ----------------------------------------------------------------
      // Decode first record which should contain the EDM class name,
      // getByToken label and max count, with the exception of 
      // buffer Event, which requires only the first field, 
      // namely edm::Event or if we have a simple type.
      // ----------------------------------------------------------------
      string record = bufferrecords[0];

      // If current buffer is a Helper, create a parameter set specific
      // to the Helper. 

      // Structure containing information about the methods to be called
      // for current buffer.
      vector<VariableDescriptor> var;

      // Split first record into fields
      vector<string> field;
      kit::split(record, field);

      // The bufferName is the className stripped of "::"
      string className = field[0]; // EDM classname
      string bufferName= kit::replace(className, "::", "");

      if ( DEBUG > 0 )
        cout << "  className: " 
	     << BOLDCYAN << className
	     << DEFAULT_COLOR
	     << "\tbufferName: " 
	     << BOLDYELLOW << bufferName
	     << DEFAULT_COLOR 
	     << endl;
      
      // if bufferName is a simple type, then there are no methods.
      boost::regex getsimpletype("^(float|double|int|long|unsigned"
				 "|size_t|short|bool|char|string|std::string)");
      boost::smatch matchtype;
      std::string bufferName_lower = bufferName;
      boost::algorithm::to_lower(bufferName_lower);
      bool simpletype = boost::regex_search(bufferName_lower, 
					    matchtype, getsimpletype);

      // ----------------------------------------------------------------
      // In the ROOT 5 version of TNM, the bufferName is given, not 
      // the className, so we need to extract the className from the 
      // bufferName to maintain backwards compatibility by checking for
      // known namespaces. If none is found then assume that the className
      // is the same as the bufferName
      // ----------------------------------------------------------------
      if ( className == bufferName )
	{
	  if ( !simpletype )
	    {
	      if ( boost::regex_search(bufferName, matchnspace, getnspace) )
		{
		  string nspace = matchnspace[0];
		  className = kit::replace(bufferName, 
					   nspace, nspace+string("::"));
		}
	    }
	}

      if ( DEBUG > 0 )
        cout << "  className: " 
	     << BOLDGREEN << className
	     << DEFAULT_COLOR
	     << "\tbufferName: " 
	     << BOLDYELLOW << bufferName
	     << DEFAULT_COLOR 
	     << endl;
      
      // The bufferName is the first part of the branch name associated 
      // with a method unless bufferName is a simple type.
      string prefix = "";
      if ( !simpletype ) prefix = bufferName;
      string varprefix = "";
      string label("");
      int maxcount=1;

      // edm::EventHelper does not use getByToken since it is just a 
      // helper for edm::Event. So don't crash if there is no label
      if ( bufferName.substr(0,8) != "edmEvent" )
        {
          if ( field.size() < 3 )
            // Have a tantrum!
            throw edm::Exception(edm::errors::Configuration,
                                 "cfg error: " + BOLDRED + 
                                 "you need at least 3 fields in first line of"
                                 " each buffer\n"
                                 "\tyou blocks you stones you worse than "
                                 "senseless things..." + DEFAULT_COLOR);
        }

      // getByToken label (category)
      if ( field.size() > 1 ) label = field[1];
      if ( boost::regex_search(record, matchvarprefix, getvarprefix) )
        {
          varprefix = kit::strip(matchvarprefix[0]);
          if ( boost::regex_search(record, matchlabel, getlabel) )
            {
              field[1] = matchlabel[0];
              label = field[1];
            }
          else
            // Have a tantrum!
            throw edm::Exception(edm::errors::Configuration,
                                 "cfg error: " + BOLDRED +
                                 "check getByToken, "
                                 "expected <label>[/<prefix>]"+DEFAULT_COLOR);
        }

      // max object count to store. this also determines whether we have
      // a singleton or a vector
      if ( field.size() > 2 ) maxcount  = atoi(field[2].c_str());  

      // n-tuple variable prefix
      if ( field.size() > 3 ) 
        prefix = field[3];
      else if ( prefix == "" ) // a simple type
	// replace double colon with an "_"
	prefix = kit::replace(label, "::", "_");       
      else if ( includeLabel_ && label != "" )
	    // replace double colon with an "_"
	prefix += string("_") + kit::replace(label, "::", "_");
      
      // Modify variable name by (optional) varprefix
      if ( varprefix != "" ) prefix +=  "_" + varprefix;

      if ( DEBUG > 1 )
        cout 
          << "   block " << blockName << "("
          << BOLDCYAN  
          << bufferName 
          << DEFAULT_COLOR 
          << ")"
          << " label("    
          << BOLDGREEN 
          << label 
          << DEFAULT_COLOR 
          << ")"
          << " maxcount(" << maxcount << ")"
          << " prefix("   << prefix << ")"
          << endl;
      
      // ----------------------------------------------------------------------
      // Loop over requested methods for current buffer.
      // The first of the pair is the method name. The second element 
      // of the pair is the name of the associated output variable.
      // If the latter is omitted, it is assumed to be the same as the
      // method name.
      //
      // Optionally, the buffer can have parameters given as a key value
      // pair, e.g.:
      //
      // param coneSize = 0.7
      // ----------------------------------------------------------------------

      std::map<std::string, std::string> parameters;

     if ( simpletype )
	{
	  var.push_back(VariableDescriptor(bufferName, "", ""));
	}
      else
	{
	  // We have a non-simple type, so we expect methods or attributes.
	  unsigned int index = 0;
	  while ( index < bufferrecords.size()-1 )
	    {
	      index++;

	      string record = bufferrecords[index];
	      if ( DEBUG > 1 )
		cout << "\trecord:   (" << BOLDRED << record 
		     << DEFAULT_COLOR << ")" 
		     << endl;

	      // Check for a Helper parameter
	      if ( boost::regex_search(record, matchparam, getparam) )
		{
		  std::string param = kit::replace(record,
						   matchparam[0], "");
		  string key, value;
		  kit::bisplit(param, key, value, "=");
		  key   = kit::strip(key);
		  value = kit::strip(value);
		  parameters[key] = value;
		  continue;
		}

	      // Get method
	      if ( ! boost::regex_search(record, matchmethod, getmethod) ) 
		// Throw another tantrum!
		throw edm::Exception(edm::errors::Configuration,
				     "regex error: " + BOLDRED +
				     "I can't get method name from \n" +
				     DEFAULT_COLOR +
				     record);
	      string method = kit::strip(matchmethod[0]);
	      if ( DEBUG > 2 )
		cout << "METHOD(" 
		     << BOLDYELLOW << method << DEFAULT_COLOR 
		     << ")" << endl;

	      // Get optional method alias name
	      string varname = method;
	      // --------------------------------------------------------------
	      // If a method consists of method("somestring")
	      // then make its default name "somestring",
	      // except for prescale methods for which the string is
	      // typically the same as that of a trigger. 
	      // However, if the method is a compound method use the
	      // standard default
	      // --------------------------------------------------------------
	      if ( boost::regex_search(method, matchstrarg, getstrarg) ) 
		{
		  varname = matchstrarg[0];
		  if ( method.substr(0,8) == "prescale" )
		    varname = "prescale" + varname;
		}

	      // Check for an alias
	      // The left sub-string is the return type of the method
	      // The right sub-string is the optional alias

	      string left, right;
	      kit::bisplit(record, left, right, method);

	      string rtype = kit::strip(left);
	      right = kit::strip(right);
	      if ( right != "" )  varname = right;

	      // --------------------------------------------------------------
	      // Check for range syntax n...m in method. If found, replace
	      // current method by a list of methods. For now, deal
	      // with only a single range variable. Remember to update the
	      // associated variable name.
	      // --------------------------------------------------------------
	      int startrange=0;
	      int endrange=0;
	      string range("");
	      if ( boost::regex_search(method, matchrange, getrange) )
		{
		  // decode range
		  range = matchrange[0];
		  if ( DEBUG > 0 )
		    cout << " RANGE(" << BOLDBLUE << range << ") " 
			 << DEFAULT_COLOR
			 << method
			 << endl;
		  string str = kit::replace(range, ".", " ");
		  vector<string> vstr;
		  kit::split(str, vstr);
		  startrange = atoi(vstr[0].c_str());
		  endrange   = atoi(vstr[1].c_str());

		  if ( DEBUG > 0 )
		    cout << "  START: (" << BOLDBLUE << startrange 
			 << DEFAULT_COLOR
			 << ")" << endl
			 << "  END:   (" << BOLDBLUE << endrange 
			 << DEFAULT_COLOR
			 << ")" << endl;
		}
          
	      if ( endrange == 0 )
		{
		  // No range variable detected so just
		  // add to vector of variables
		  var.push_back(VariableDescriptor(rtype, method, varname));
              
		  if ( DEBUG > 0 )
		    cout << "   rtype:   " << BOLDRED   << rtype 
			 << DEFAULT_COLOR << endl
			 << "     method:  " << BOLDCYAN  << method 
			 << DEFAULT_COLOR << endl
			 << "       varname: " << BOLDGREEN << varname 
			 << DEFAULT_COLOR << endl;
		}
	      else
		{
		  // Range variable detected

		  string vname = kit::replace(varname, range, "");
		  string mname = method;

		  for(int ind=startrange; ind <= endrange; ind++)
		    {
		      char number[80];
		      sprintf(number, "%d", ind);

		      if ( DEBUG > 2 )
			cout << "   method:   " << BOLDRED   << method 
			     << DEFAULT_COLOR << endl
			     << "     range:  " << BOLDCYAN  << range 
			     << DEFAULT_COLOR << endl
			     << "       number: " << BOLDGREEN << number 
			     << DEFAULT_COLOR << endl;

		      method  = kit::replace(mname, range, number);
		      varname = vname + number; // update varname

		      // Add to vector of variables
		      var.push_back(VariableDescriptor(rtype, method, varname));
                                                   
		      if ( DEBUG > 0 )
			cout << "   rtype:   " << RED   << rtype 
			     << DEFAULT_COLOR << endl
			     << "     method:  " << CYAN  << method 
			     << DEFAULT_COLOR << endl
			     << "       varname: " << GREEN << varname 
			     << DEFAULT_COLOR << endl;
		    }
		}
	    }
	}
      // ----------------------------------------------------------------------
      // Decoding of current buffer complete.
      // Cache decoded buffer information
      // ----------------------------------------------------------------------
      className_.push_back(className);
      blockName_.push_back(blockName);
      bufferName_.push_back(bufferName);
      label_.push_back(label);
      parameters_.push_back(parameters);
      prefix_.push_back(prefix);
      variables_.push_back(var);
      maxcount_.push_back(maxcount);
    }

  // ---------------------------------------------------------------
  // Create branchnames.  Make sure to eliminate
  // duplicates, but warn that this is being done 
  // Block name is same as object name. variables_[i].branchname 
  // will contain the branch name
  // ---------------------------------------------------------------
  // Write branches and variables to variables file 
  
  ofstream vout("variables.txt");
  if ( DirectoryName !=  "")
    vout << "Tree: " 
	 << DirectoryName << "/" 
	 << tree->GetName() << " " 
	 << ct << endl;
  else
    vout << "Tree: " 
	 << tree->GetName() << " " 
	 << ct << endl;

  map<string, int> branchcount;
  for(size_t ii=0; ii < blockName_.size(); ii++)
    createBranchnames(blockName_[ii],
		      prefix_[ii],
		      maxcount_[ii],
		      variables_[ii],
		      vout,
		      branchcount);
  vout.close();

  // --------------------------------------------------------------
  // Create ntuple analyzer template if requested
  // --------------------------------------------------------------
  // if ( analyzername_ != "" )
  //   {
  //     string cmd("mkanalyzer.py "  + kit::nameonly(analyzername_));
  //     cout << cmd << endl;
  //     kit::shell(cmd);
  //   }
  
  // ---------------------------------------------------------------
  // Compile TheNtupleMaker header
  // ---------------------------------------------------------------
  ofstream fout("jit_code.cc");

  char code[8000];
  sprintf(code,
	  "#include \"PhysicsTools/TheNtupleMaker/interface/"
	  "TheNtupleMaker.h\"\n");

  fout << "// ------------------------------------------------" 
       << "---------------" << endl;
  fout << "// Created: TheNtupleMaker " << ct;
  fout << "// ------------------------------------------------"	
       << "---------------" << endl;
  fout << code;
  fout << "// ------------------------------------------------" 
       << "---------------" << endl;

  cout << endl
       << "compiling TNM header:"
       << endl << endl;
  cout << BOLDCYAN << code << DEFAULT_COLOR;
  gROOT->ProcessLine(code);
  cout << endl << endl<< "done compiling TNM header" << endl;

  // --------------------------------------------------------------
  // Loop over buffers (blocks), create an instance of each and
  // compile it.
  // --------------------------------------------------------------
  long unsigned int objectaddr;
  sprintf(code, "long unsigned int* objectaddr=0;\n");
  fout << "// pointer to return object addresses to TNM" << endl;
  fout << code;
  gROOT->ProcessLine(code);
  
  for(int ii=0; ii < (int)blockName_.size(); ii++)
    {
      cout << "compiling " << blockName_[ii] << endl;

      if ( maxcount_[ii] > 1 )
	sprintf(code, 
		"Buffer< std::vector<%s> > \tbuffer%d;\n",
		className_[ii].c_str(), ii);
      else
	sprintf(code, 
		"Buffer< %s > \tbuffer%d;\n",
		className_[ii].c_str(), ii);
      
      if ( DEBUG < 0 ) cout << "   " 
	   << GREEN << code 
	   << DEFAULT_COLOR;
      
      fout << endl << endl 
           << "// ====> BLOCK( " << blockName_[ii] << " )" 
           << endl << endl;
      fout << code;
      gROOT->ProcessLine(code);
      
      sprintf(code, 
	      "objectaddr  = (long unsigned int*)%s;\n"
	      "*objectaddr = (long unsigned int)&buffer%d;\n", 
	      "0x%lx", ii);

      gROOT->ProcessLine(Form(code, &objectaddr));
	  
      // Cache buffer address
      buffers.push_back((BufferThing*)objectaddr);
      
      // Compile variables of current buffer
      BufferThing* pbuffer = buffers.back();
      for(int jj=0; jj < (int)variables_[ii].size(); jj++)
	{
	  sprintf(code, 
		  "Variable< %s, %s > \tvar%d%d(\"%s\", %d, \"%s\");\n",
		  className_[ii].c_str(),
		  variables_[ii][jj].rtype.c_str(),
		  ii, jj,
		  variables_[ii][jj].method.c_str(),
		  maxcount_[ii],
		  variables_[ii][jj].branchname.c_str());

	  if ( DEBUG < 0 ) cout << "\t" 
	       << CYAN << code 
	       << DEFAULT_COLOR;

          fout << code;
	  gROOT->ProcessLine(code);

          // the getter code for current variable should be available
	  string getter_code;
          ifstream fin(".jit_code.cc");
    	  while (getline(fin, getter_code))
	   {
              fout << getter_code << endl;
	   }
          fin.close();

	  sprintf(code, 
		  "objectaddr  = (long unsigned int*)%s;\n"
		  "*objectaddr = (long unsigned int)&var%d%d;\n", 
		  "0x%lx", ii, jj);

	  gROOT->ProcessLine(Form(code, &objectaddr));
	      
	  // Cache variable address in buffer
	  pbuffer->add((VariableThing*)objectaddr);
	}
      
      // Now initialize buffer
      pbuffer->init(this, label_[ii]);
    }

  fout.close();

  // // Check for crash switch
  
  // bool crash = false;
  // try
  //   {
  //     crash = 
  //       (bool)Configuration::instance().
  //       getConfig()->getUntrackedParameter<int>("crashOnInvalidHandle");
  //   }
  // catch (...)
  //   {}

  // if ( crash )
  //   cout << "\t==> TheNtupleMaker will CRASH if a handle is invalid <==";
  // else
  //   cout << "\t==> TheNtupleMaker will WARN if a product is not found <==";
  // cout << endl << endl;

  cout << endl << "END TheNtupleMaker Configuration" 
       << endl << endl;
}


TheNtupleMaker::~TheNtupleMaker() {}

// ----------------------------------------------------------------------------
// member functions
// ----------------------------------------------------------------------------

// ------------ method called to for each event  ------------
void
TheNtupleMaker::analyze(const edm::Event& iEvent, 
                        const edm::EventSetup& iSetup)
{
  // Cache current event and event setup
  CurrentEvent::instance().set(iEvent, iSetup);

  // Call methods for each buffer
  for(size_t i=0; i < buffers.size(); i++) buffers[i]->get(iEvent);

  //inputCount_++;
  count_++;
  if ( count_ % imalivecount_ == 0 )
    cout << "\t=> event count: " << count_ << endl;

  // Apply optional cuts

  //if ( ! selectEvent(iEvent) ) return;

  // Fill output ntuple

  tree->Fill();
}

bool
TheNtupleMaker::selectEvent(const edm::Event& event)
{
  bool keep = true;
  if ( ! macroEnabled_ ) return keep;

  // Initialize event variables in user macro
  // and reset indexmap
  // gROOT->ProcessLineFast("obj.initialize();");

  // Call macro analyze method
  //keep = (bool)gROOT->ProcessLineFast("obj.analyze();");  
      
  if ( DEBUG )
    {
      if ( keep )
        cout << "\t\t** KEEP EVENT(" 
             << "Run: " << event.id().run() 
             << "\tEvent: " << event.id().event() << ")"
             << endl;
      else
        cout << "\t\t** SKIP EVENT("
             << "Run: " << event.id().run() 
             << "\tEvent: " << event.id().event() << ")"
             << endl;
    }

  return keep;
}

// void
// TheNtupleMaker::shrinkBuffers()
// {
//   if ( ! macroEnabled_ ) return;

//   // If requested, write out only selected objects
 
//   // indexmap maps from buffer identifier (object variable name) to
//   // object indices
//   map<string, vector<int> >::iterator iter = indexmap.begin();

//   if ( DEBUG > 0)
//     cout << " ==> indexmap.size(): " << indexmap.size() << endl;
//   //DEBUG = 1;
//   for(iter=indexmap.begin(); iter != indexmap.end(); ++iter)
//     {
//       string name(iter->first);
//       vector<int>& indices = iter->second;
      
//       if ( buffermap.find(name) != buffermap.end() )
//         {
//           if (DEBUG > 0)
//             {
//               cout << "\t** BEFORE SHRINK( " << name << " ) count: " 
//                    << buffermap[name]->count() << endl;
//               for(unsigned int i=0; i < indices.size(); ++i)
//                 cout << "\t\t" << i << "\t" << indices[i] << endl;
//             }

//           buffermap[name]->shrink(indices);

//           if (DEBUG > 0)
//             {
//               cout << "\t** AFTER SHRINK( " << name << " )  count: " 
//                    << buffermap[name]->count() << endl;
//             }
//         }
//       else
//         throw edm::Exception(edm::errors::Configuration,
//                              "object selection error: "
//                              "bad buffer key name - " + name + 
//                              "\n\tyou blocks you stones you worse "
//                              "than senseless things\n");
//     }
//   //DEBUG = 0;
// }


void 
TheNtupleMaker::updateTriggerBranches(int index)
{
  // 1. Search for wildcard characters in trigger name
  // 2. Search list of trigger names and add any matched name
  //    to list of branches to be created.

  // check for wildcard character(s)
  boost::regex getwild("[.]|"
                       "[*]|"
                       "[+]|"
                       "[?]|"
                       "[(]|"
                       "[)]|"
                       "[<]|"
                       "[>]|"
                       "[[!]]|"
                       "[[$]]|"
                       "[[^]]");
  boost::smatch matchwild;

  // extract string from method
  boost::regex getstrarg("(?<=[(]\").+(?=\"[)])");
  boost::smatch matchstrarg;

  // get current list of trigger variables
  vector<VariableDescriptor>& var    = variables_[index];
  vector<VariableDescriptor>  varnew;

  for(size_t jj=0; jj < var.size(); jj++)
    {
      if ( DEBUG > 2 )
        cout << "\tMETHOD("  << var[jj].method << ")" << endl;

      // extract string
      if ( !boost::regex_search(var[jj].method, 
                                matchstrarg, 
                                getstrarg) )
        throw edm::Exception(edm::errors::Configuration,
                             "cannot extract string from: " +
                             var[jj].method);
      string argstr = matchstrarg[0];

      // check for wild cards in argument string
      if ( !boost::regex_search(argstr, matchwild, getwild) )
        {
          // no wild card characters found so just add to list
          // of variables and continue
          varnew.push_back(var[jj]);
          if ( DEBUG > 2 )
            cout << "\tVARNAME(" << var[jj].varname << ")" << endl;

          continue;
        }

      // wildcard found, search trigger names

      // Now search for matches in trigger list
      // First create a regular expression

      boost::smatch matchre;
      string regex = kit::replace(argstr, "*", ".*");
      regex = kit::replace(regex, "..*", ".*");
      boost::regex getname(regex);
      boost::smatch matchname;

      if ( DEBUG > 2 )
        cout << "\tREGEX(" << regex << ")" << endl;

      for(size_t i=0; i < triggerNames_.size(); i++)
        {
          if ( !boost::regex_search(triggerNames_[i], matchname, getname) )
            continue;

          // found match, so update varnew
          varnew.push_back(var[jj]);
          if ( var[jj].method.substr(0,8) == "prescale" )
            varnew.back().varname = "prescale" + triggerNames_[i];            
          else
            varnew.back().varname = triggerNames_[i];
          varnew.back().method = kit::replace(varnew.back().method,
                                              argstr, triggerNames_[i]);
          if ( DEBUG > 2 )
            cout << "\t\tMATCH(" << varnew.back().method << ")" << endl;
        }
    }

  // update variables
  variables_[index] = varnew;
}

// --- method called once each job just before starting event loop  -----------
void 
TheNtupleMaker::beginJob()
{
  //if ( macroEnabled_ ) gROOT->ProcessLine("obj.beginJob();");
  swatch.Start();
}


void 
TheNtupleMaker::beginRun(const edm::Run& run, 
                         const edm::EventSetup& eventsetup)
{
  // Initialize the HLT configuration every new run
  // Based on some code from Josh

  if ( DEBUG > 0 )
    cout 
      << "BEGIN Run: " << BOLDYELLOW << run.run() << DEFAULT_COLOR << endl;

  try
    {
      bool HLTchanged=true; // follow HLTTrigReport 

      if ( HLTconfig_.init(run, 
                           eventsetup, 
                           triggerResults_.process(), 
                           HLTchanged) )
        {
          HLTconfigured = true;

          if ( HLTchanged )
            {
              edm::LogInfo("HLTConfigChanged") 
                << "The HLT configuration has changed"
                << std::endl;
	    }          

          // Update trigger names file

          triggerNames_ = HLTconfig_.triggerNames();

          unsigned int startrun = run.run();
          unsigned int endrun   = run.run();
          set<string> nameset;

	  ifstream fin("triggerNames.txt");
	  if ( fin.good() )
	    {
	      int n;
	      string line;
	      try
		{
		  fin >> line >> line >> startrun; // get start run
		  fin >> line >> line >> n;
		  fin >> line >> line >> n;
		}
	      catch (...)
		{}
	      
	      while ( fin >> line ) nameset.insert(line);
	      fin.close();
	    }

          // Write out updated trigger names
          ofstream fout("triggerNames.txt");
          fout << "FIRST RUN:     " << startrun << endl;
          fout << "LAST  RUN:     " << endrun   << endl;
          for(unsigned int  i=0; i < triggerNames_.size(); i++)
            nameset.insert(triggerNames_[i]);
          fout << "TRIGGER COUNT: " << nameset.size()   << endl;

          for(set<string>::iterator it=nameset.begin();
              it != nameset.end(); it++)
            fout << "\t" << *it << std::endl;
          fout.close();
        }
      else
        {
          HLTconfigured = false;

          edm::LogWarning("HLTConfigFailure") 
            << "The HLT configuration failed"
            << std::endl;
        }
    }
  catch (...)
    {
      HLTconfigured = false;
      edm::LogWarning("HLTConfigInitFailure") 
        << "Problem initializing HLT configuration"
        << std::endl
        << "\ttry including the line: "
        << BOLDRED 
        << "process.load(\""
        << "L1TriggerConfig.L1GtConfigProducers.L1GtConfig_cff\")"
        << DEFAULT_COLOR
        << std::endl
        << "in your configuration file"
        << std::endl;
    }

  // Update HLT config pointer

  if ( HLTconfigured ) 
    Configuration::instance().set(&HLTconfig_);
  else
    Configuration::instance().set((HLTConfigProvider*)0);

  if ( DEBUG > 0 )
    cout 
      << "END Run: " << run.run() << endl;
}

void 
TheNtupleMaker::createBranchnames(std::string blockName,    
				  std::string prefix,
				  int maxcount,
				  std::vector<VariableDescriptor>& var,
				  ofstream& vout,
				  map<string, int>& branchcount)
{
  // Define regular expressions to check for compound methods; 
  // i.e., methods of the form 
  // 1. y = a()->b()
  // 2. y = a().b()
  // 3. y = a().b 
  // 4. y = a()->b 
  // 5. y = a.b()
  // 6. y = a->b()

  boost::regex stripargs("[(].*?[)]");
  boost::regex stripme("-[>]|[.]|\"|[(]|[)]| |,|[<]|[>]");
  boost::regex stripcolon("[a-zA-Z]+::");
  boost::regex strip3_("___");
  boost::regex strip2_("__");
  boost::regex strip2_atend("_$");

  // Just return if no variables were specified
  
  if ( var.size() == 0 ) 
    {
      std::cout 
        << "** Warning! createBranchnames - no variables defined for  " 
        << blockName
        << std::endl;
      return;
    }
  
  if ( DEBUG > 0 )
    std::cout << BOLDRED << blockName << DEFAULT_COLOR << std::endl;

  // Define variables destined for the output tree  
  // For every method, create the associated n-tuple branch name

  int nvar = 0;
  for(size_t i=0; i < var.size(); i++)
    {    
      std::string rtype   = var[i].rtype;
      std::string method  = var[i].method;
      std::string varname = var[i].varname;
      if ( DEBUG > 1 )
        std::cout << BOLDCYAN
                  << "   varname before(" << varname << ")"
                  << DEFAULT_COLOR
                  << std::endl;

      // Replace "->", ".", "(", ")" and quotes by "_"
      varname = boost::regex_replace(varname, stripme,  "_");

      // Replace "___" by "_"
      varname = boost::regex_replace(varname, strip3_,  "_");

      // Replace "__" by "_"
      varname = boost::regex_replace(varname, strip2_,  "_");

      // Strip away possible "_" at the end 
      varname = boost::regex_replace(varname, strip2_atend,  "");

      if ( DEBUG > 1 )
        std::cout << "        "
                  << BOLDYELLOW 
                  << "   varname after (" << varname << ")"
                  << DEFAULT_COLOR
                  << std::endl;      

      // create branch name, but be mindful that we may be
      // dealing with a simple time.
      std::string  branchname = prefix;
      if ( varname != "") branchname += "." + varname;

      // check for uniqueness
      if ( branchcount.find(branchname) == branchcount.end() )
	{
	  // unique name
	  branchcount[branchname] = 1;
	}
      else
        {
	  // non-unique name
	  int  nn = branchcount[branchname];
	  char num[12];
	  sprintf(num, "%d", nn);
	  nn++;
	  branchcount[branchname] = nn;
          branchname = prefix + string(num) + "_" + "." + varname;
        }

      // check if we need to dress type with vector<...>
      string dressed_rtype = rtype;
      if ( maxcount > 1 ) 
	dressed_rtype = string("vector<") + rtype + string(">");

      if ( varname == "" )
	{
	  vout << dressed_rtype << "/" 
	       << branchname << "/"
	       << blockName  << "/"
	       << maxcount 
	       << std::endl;
	}
      else
	{
	  vout << dressed_rtype << "/" 
	       << branchname  << "/"
	       << blockName + "_" + varname << "/"
	       << maxcount 
	       << std::endl;
	}
      
      // Note: nvar <= i
      var[nvar].branchname   = branchname;
      var[nvar].maxcount   = maxcount;
      var[nvar].method   = method;

      if ( DEBUG > 0 )
	std::cout << "   " << nvar << ":\t" << BOLDGREEN 
		  << branchname 
		  << "\t" << DEFAULT_COLOR << method << std::endl;
      
      // update variable count
      nvar++;
    }

  // update size of var, if necessary
  if ( nvar < (int)var.size() ) var.resize(nvar);
}

// --- method called once each job just after ending the event loop  ----------
void 
TheNtupleMaker::endJob() 
{
  char cmd[80];
  float ms = swatch.RealTime();
  if ( maxEvents > 0 )
    {
      ms = 1000 * ms / maxEvents;
      sprintf(cmd, "%8.3f", ms);
      cout << endl << BOLDYELLOW
	   << "time/event: " << cmd << " ms" << DEFAULT_COLOR
	   << endl << endl;
    }
  else
    {
      ms /= 60;
      sprintf(cmd, "%8.3f", ms);
      cout << endl << BOLDYELLOW
	   << "elapsed time: " << cmd << " minutes" << DEFAULT_COLOR
	   << endl << endl;
    }

  //if ( macroEnabled_ ) gROOT->ProcessLine("obj.endJob();");

  //output.close();
}

TTree* TheNtupleMaker::getTree() { return tree; }

//define this as a plug-in
DEFINE_FWK_MODULE(TheNtupleMaker);
