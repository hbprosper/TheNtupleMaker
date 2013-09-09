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
     
                stripped-classname + "_" + getByLabel + "." + method

     (B) Mechanism

     For each class, e.g., pat::Muon, we allocate a buffer that is
     initialized with the methods to be called. The return
     value of each method called (once, per event, for every instance 
     of an object of a given type) is stored in a variable whose address 
     is known to Root. The buffer object has two methods: init(...) and 
     fill(...). The init(...) method tells the buffer which methods of
     the associated class to call. The fill(...) method calls these
     methods and caches their return values. The commit() method of the
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
// $Id: TheNtupleMaker.cc,v 1.23 2013/07/05 07:15:14 prosper Exp $
// ---------------------------------------------------------------------------
#include <boost/regex.hpp>
#include <memory>
#include <iostream>
#include <fstream>
#include <cassert>
#include <set>
#include <time.h>
#include <stdlib.h>

#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "PhysicsTools/TheNtupleMaker/interface/treestream.h"
#include "PhysicsTools/TheNtupleMaker/interface/kit.h"
#include "PhysicsTools/TheNtupleMaker/interface/pluginfactory.h"
#include "PhysicsTools/TheNtupleMaker/interface/CurrentEvent.h"
#include "PhysicsTools/TheNtupleMaker/interface/Configuration.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"


#include "TROOT.h"
#include "TSystem.h"
#include "TMap.h"
#include "TString.h"
#include "TApplication.h"
// ---------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------
class TheNtupleMaker : public edm::EDAnalyzer 
{
public:
  explicit TheNtupleMaker(const edm::ParameterSet&);
  ~TheNtupleMaker();

private:
  virtual void beginJob();
  virtual void beginRun(const edm::Run&, const edm::EventSetup&);
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();

  void allocateBuffer();
  bool selectEvent(const edm::Event& iEvent);
  void shrinkBuffers();
  void updateTriggerBranches(int blockindex);

  // Object that models the output n-tuple.
  std::string ntuplename_;
  otreestream output;

  // Object that models the allocated buffers, one per object to be read.
  std::vector<BufferThing*> buffers;

  // addresses of buffers
  std::map<std::string, BufferThing*> buffermap;

  // map from variables to buffer location and count
  std::map<std::string, countvalue>   varmap;

  // map from object name to object index (used for selecting objects to be
  // stored)
  std::map<std::string, std::vector<int> >   indexmap;

  int DEBUG;

  std::string logfilename_;
  std::ofstream* log_;
  std::string analyzername_;
  std::string macroname_;

  int count_;
  int imalivecount_;
  int logger_;
  bool haltlogger_;
  bool macroEnabled_;

  TTree* ptree_;
  int inputCount_;

  // cache for HLT
  edm::InputTag triggerResults_;
  HLTConfigProvider HLTconfig_;
  bool HLTconfigured;
  std::vector<std::string> triggerNames_;

  // cache decoded config data
  std::vector<std::string> blockName_;
  std::vector<std::string> bufferName_;
  std::vector<std::string> label_;
  std::vector<std::string> prefix_;
  std::vector<int> maxcount_;
  std::vector<std::map<std::string, std::string> > parameters_;
  std::vector<std::vector<VariableDescriptor> > variables_;

  bool buffersInitialized;
};


TheNtupleMaker::TheNtupleMaker(const edm::ParameterSet& iConfig)
  : ntuplename_(iConfig.getUntrackedParameter<string>("ntupleName")), 
    output(otreestream(ntuplename_,
                       "Events", 
                       "created by TheNtupleMaker $Revision: 1.23 $")),
    logfilename_("TheNtupleMaker.log"),
    log_(new std::ofstream(logfilename_.c_str())),
    macroname_(""),
    count_(0),
    imalivecount_(1000),
    logger_(0),
    haltlogger_(false),
    macroEnabled_(false),
    inputCount_(0),
    HLTconfigured(false),
    buffersInitialized(false)
{
  cout << "\nBEGIN TheNtupleMaker Configuration" << endl;

  string cfg = string(iConfig.dump());

  // --------------------------------------------------------------------------
  // Add a provenance tree to ntuple
  // --------------------------------------------------------------------------
  TFile* file = output.file();
  ptree_ = new TTree("Provenance",
                     "created by TheNtupleMaker $Revision: 1.23 $");
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

  if ( cfg != "" ) ptree_->Branch("cfg", (void*)(cfg.c_str()), "cfg/C");

  //ptree_->Branch("inputcount", &inputCount_, "inputcount/I");

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
  int autosaveCount = 100;
  try
    {
      autosaveCount = iConfig.getUntrackedParameter<int>("autosaveCount");
    }
  catch (...)
    {}
  output.autosave(autosaveCount);

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

  // Get name of optional macro
  try
    {
      macroname_ = kit::nameonly(iConfig.
                                 getUntrackedParameter<string>("macroName"));
    }
  catch (...)
    {
      macroname_ = "";
    }

  // --------------------------------------------------------------------------

  macroEnabled_ = macroname_ != "";

  if ( macroEnabled_ )
    {
      // Try to load associated shared library 

      // First find shared lib
      string filestem = string("*") + macroname_ + string("*.so");
      string cmd = string("find . -name \"") + filestem + "\"";
      string shlib = kit::shell(cmd);
      if ( shlib == "" )
        {
          string errmess = string("\tfind ") + filestem + string(" failed");
          // Have a tantrum!
          throw cms::Exception("FileNotFound", errmess);
        }

      // Found shared library, so try to load it

      cout << "\t==> Load macro library " << shlib << endl;
      
      if ( gSystem->Load(shlib.c_str()) != 0 )
         throw cms::Exception("LoadFailed",
                              "\tfor shared library\n\t\t" + shlib);

      // Create commands to execute macro using CINT

      string treecmd("TTree* tree = (TTree*)0x%lx"); 
      gROOT->ProcessLine(Form(treecmd.c_str(),(unsigned long)(output.tree())));

      string mapcmd("map<string,countvalue>* "
                    "varmap = (map<string,countvalue>*)0x%lx");
      gROOT->ProcessLine(Form(mapcmd.c_str(), (unsigned long)(&varmap)));

      string imapcmd("map<string,vector<int> >* "
                     "indexmap = (map<string,vector<int> >*)0x%lx");
      gROOT->ProcessLine(Form(imapcmd.c_str(), (unsigned long)(&indexmap)));

      string macrocmd = macroname_ + string(" obj(tree,varmap,indexmap);");
      gROOT->ProcessLine(macrocmd.c_str());
    }

  if ( getenv("DBTheNtupleMaker") > 0 )
    DEBUG = atoi(getenv("DBTheNtupleMaker"));
  else
    DEBUG = 0;

  // Write to log file

  *log_ << "Created: " << ct << endl;
  log_->close();

  // --------------------------------------------------------------------------
  //
  // Decode buffer information and cache it. The buffers are created during
  // the first call to beginRun(). This is necessary for those buffers, such
  // as edmTriggerResultsHelper that need at least one event in memory.
  //
  // Each buffer descriptor (a series of strings) comprises
  // One line containing
  //    buffer     - name of buffer to allocate
  //    label      - used by getByLabel
  //    maxcount   - maximum number of objects to write out. If omitted,
  //                 assumed to be unity
  //    [prefix]   - prefix for names of n-tuple output variables
  //                 otherwise, take prefix = <buffer>_<label>
  // followed by the list of methods
  // 
  // Helper methods may optionally contain strings with the format
  //   parameter parameter-name = parameter-value
  //
  // --------------------------------------------------------------------------

  // We need several regular expressions for decoding
  boost::regex getmethod("[a-zA-Z][^ ]*[(].*[)][^ ]*|[a-zA-Z][a-zA-Z0-9]*$");
  boost::smatch matchmethod;

  boost::regex getparam("^ *param +");
  boost::smatch matchparam;

  boost::regex getvarprefix("(?<=/)[a-zA-Z0-9]+");
  boost::smatch matchvarprefix;

  boost::regex getlabel("[a-zA-Z0-9]+(?=/)");
  boost::smatch matchlabel;

  boost::regex getrange("[0-9]+[.][.]+[0-9]+");
  boost::smatch matchrange;

  boost::regex getstrarg("(?<=[(]\").+(?=\"[)])");
  boost::smatch matchstrarg;

  // get list of strings from "objects", decode them, and cache the results
  // NOTE: Within TNM, block and object are used interchangeably
  // --------------------------------------------------------------------------
  vector<string> vrecords;
  try
    {
      vrecords = iConfig.getUntrackedParameter<vector<string> >("objects");
    }
  catch (...)
    {
      vrecords = iConfig.getUntrackedParameter<vector<string> >("buffers");
    }

  for(unsigned ii=0; ii < vrecords.size(); ii++)
    {
      // We must distinguish between the block name and the buffer name
      // because multiple blocks can map to the same buffer name
      // The block name will be used as the name of the C++ struct
      // 
      string blockName = vrecords[ii];

      if ( DEBUG > 1 ) 
        cout << "block(" << blockName  << ")" << endl; 
      
      // Get description for current buffer of variables
      vector<string> bufferrecords = iConfig.
        getUntrackedParameter<vector<string> >(blockName);

      // Decode first record which should
      // contain the buffer name, getByLabel and max count, with
      // the exception of buffer edmEvent, which requires only the
      // first field
      string record = bufferrecords[0];

      // If current buffer is a Helper, create a parameter set specific to
      // the Helper 

      // Structure containing information about the methods to be called.
      vector<VariableDescriptor> var;

      vector<string> field;
      kit::split(record, field);

      string bufferName = field[0];

      if ( DEBUG > 0 )
        cout << "  buffer: " << bufferName << endl;
      
      // The first part of the branch name associated with a method is
      // the same as the buffer name, which basically identifies the
      // associated class

      string label("");
      string prefix = bufferName;
      string varprefix = "";
      int maxcount=1;

      // edmEventHelper does not use getByLabel since it is just a 
      // helper for edm::Event. So don't crash if there is no
      // getByLabel

      if ( bufferName.substr(0,8) != "edmEvent" )
        {
          if ( field.size() < 3 )
            // Have a tantrum!
            throw edm::Exception(edm::errors::Configuration,
                                 "cfg error: "
                                 "you need at least 3 fields in first line of"
                                 " each buffer\n"
                                 "\tyou blocks you stones you worse than "
                                 "senseless things...");
        }

      // getByLabel
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
                                 "cfg error: "
                                 "check getByLabel\n"
                                 "expected <label>/<prefix>");
        }

      // max object count to store
      if ( field.size() > 2 ) maxcount  = atoi(field[2].c_str());  

      // n-tuple variable prefix
      if ( field.size() > 3 ) 
        prefix = field[3]; 
      else if (label != "")
        // replace double colon with an "_"
        prefix += string("_") + kit::replace(label, "::", "_");

      // Modify variable name by (optional) varprefix
      if ( varprefix != "" ) prefix +=  "_" + varprefix;

      //DB
      if ( DEBUG > 1 )
        cout 
          << "   block " << blockName << "("
          << RED  
          << bufferName 
          << BLACK 
          << ")"
          << " label("    
          << BLUE 
          << label 
          << DEFAULT_COLOR 
          << ")"
          << " maxcount(" << maxcount << ")"
          << " prefix("   << prefix << ")"
          << endl;
      
      // Loop over requested methods for current buffer.
      // The first of the pair is the method name.
      // The second of the pair is the name of the associated output variable.
      // If the latter is omitted, it is assumed to be the same as the
      // method name.

      // Optionally, the buffer can have parameters given as a key value
      // pair, e.g.:
      //
      // param coneSize = 0.7
      // 
      std::map<std::string, std::string> parameters;

      unsigned int index = 0;
      while ( index < bufferrecords.size()-1 )
        {
          index++;

          string record = bufferrecords[index];
          if ( DEBUG > 0 )
            cout << "record:   (" << RED << record << DEFAULT_COLOR << ")" 
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
            // Yet another tantrum!
            throw edm::Exception(edm::errors::Configuration,
                                 "regex error: "
                                 "I can't get method name from \n" +
                                 record);
          string method = kit::strip(matchmethod[0]);
          if ( DEBUG > 0 )
            cout << "METHOD(" 
                 << BLUE << method << DEFAULT_COLOR 
                 << ")" << endl;

          // Get optional method alias name
          
          string varname = method;
          
          // If a method consists of method("somestring")
          // then make its default name "somestring",
          // except for prescale methods for which the string is
          // typically the same as that of a trigger. 
          // However, if the method is a compound method use the
          // standard default

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

          // Check for range syntax n...m in method. If found, replace
          // current method by a list of methods. For now, deal
          // with only a single range variable. Remember to update the
          // associated variable name.

          int startrange=0;
          int endrange=0;
          string range("");
          if ( boost::regex_search(method, matchrange, getrange) )
            {
              // decode range
              range = matchrange[0];
              if ( DEBUG > 0 )
                cout << " RANGE(" << BLUE << range << ") " 
                     << DEFAULT_COLOR
                     << method
                     << endl;
              string str = kit::replace(range, ".", " ");
              vector<string> vstr;
              kit::split(str, vstr);
              startrange = atof(vstr[0].c_str());
              endrange = atof(vstr[1].c_str());

             if ( DEBUG > 0 )
               cout << "  START: (" << BLUE << startrange << DEFAULT_COLOR
                    << ")" << endl
                    << "  END:   (" << BLUE << endrange << DEFAULT_COLOR
                    << ")" << endl;
            }
          
          if ( endrange == 0 )
            {
              // No range variable detected so just
              // add to vector of variables
              var.push_back(VariableDescriptor(rtype, method, varname));
              
              if ( DEBUG > 0 )
                cout << "   rtype:   " << RED   << rtype 
                     << DEFAULT_COLOR << endl
                     << "     method:  " << BLUE  << method 
                     << DEFAULT_COLOR << endl
                     << "       varname: " << GREEN << varname 
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

//                  if ( DEBUG > 0 )
//                     cout << "   method:   " << RED   << method 
//                          << DEFAULT_COLOR << endl
//                          << "     range:  " << BLUE  << range 
//                          << DEFAULT_COLOR << endl
//                          << "       number: " << GREEN << number 
//                          << DEFAULT_COLOR << endl;

                  method  = kit::replace(mname, range, number);
                  varname = vname + number; // update varname

                  // Add to vector of variables
                  var.push_back(VariableDescriptor(rtype, method, varname));
                                                   
              
                  if ( DEBUG > 0 )
                    cout << "   rtype:   " << RED   << rtype 
                         << DEFAULT_COLOR << endl
                         << "     method:  " << BLUE  << method 
                         << DEFAULT_COLOR << endl
                         << "       varname: " << GREEN << varname 
                         << DEFAULT_COLOR << endl;
                }
            }
        }
      
      // Decoding complete.

      // Cache decoded buffer information

      blockName_.push_back(blockName);
      bufferName_.push_back(bufferName);
      label_.push_back(label);
      parameters_.push_back(parameters);
      prefix_.push_back(prefix);
      variables_.push_back(var);
      maxcount_.push_back(maxcount);
    }
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
  for(unsigned i=0; i < buffers.size(); i++) buffers[i]->fill(iEvent, iSetup);

  //inputCount_++;
  count_++;
  if ( count_ % imalivecount_ == 0 )
    cout << "\t=> event count: " << count_ << endl;

  // Apply optional cuts

  if ( ! selectEvent(iEvent) ) return;

  //Event kept. Shrink buffers as needed. Shrinking is needed if only
  //certain objects of a given buffer have been selected.

  shrinkBuffers();

  // Copy data to output buffers

  output.commit();
}

bool
TheNtupleMaker::selectEvent(const edm::Event& event)
{
  bool keep = true;
  if ( ! macroEnabled_ ) return keep;

  // Initialize event variables in user macro
  // and reset indexmap
  gROOT->ProcessLineFast("obj.initialize();");

  // Call macro analyze method
  keep = (bool)gROOT->ProcessLineFast("obj.analyze();");  
      
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

void
TheNtupleMaker::shrinkBuffers()
{
  if ( ! macroEnabled_ ) return;

  // If requested, write out only selected objects
 
  // indexmap maps from buffer identifier (object variable name) to
  // object indices
  map<string, vector<int> >::iterator iter = indexmap.begin();

  if ( DEBUG > 0)
    cout << " ==> indexmap.size(): " << indexmap.size() << endl;
  //DEBUG = 1;
  for(iter=indexmap.begin(); iter != indexmap.end(); ++iter)
    {
      string name(iter->first);
      vector<int>& indices = iter->second;
      
      if ( buffermap.find(name) != buffermap.end() )
        {
          if (DEBUG > 0)
            {
              cout << "\t** BEFORE SHRINK( " << name << " ) count: " 
                   << buffermap[name]->count() << endl;
              for(unsigned int i=0; i < indices.size(); ++i)
                cout << "\t\t" << i << "\t" << indices[i] << endl;
            }

          buffermap[name]->shrink(indices);

          if (DEBUG > 0)
            {
              cout << "\t** AFTER SHRINK( " << name << " )  count: " 
                   << buffermap[name]->count() << endl;
            }
        }
      else
        throw edm::Exception(edm::errors::Configuration,
                             "object selection error: "
                             "bad buffer key name - " + name + 
                             "\n\tyou blocks you stones you worse "
                             "than senseless things\n");
    }
  //DEBUG = 0;
}


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

  for(unsigned int jj=0; jj < var.size(); jj++)
    {
      if ( DEBUG > 0 )
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
          if ( DEBUG > 0 )
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

      if ( DEBUG > 0 )
        cout << "\tREGEX(" << regex << ")" << endl;

      for(unsigned  i=0; i < triggerNames_.size(); i++)
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
          if ( DEBUG > 0 )
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
  if ( macroEnabled_ ) gROOT->ProcessLine("obj.beginJob();");
}


void 
TheNtupleMaker::beginRun(const edm::Run& run, 
                         const edm::EventSetup& eventsetup)
{
  // Initialize the HLT configuration every new
  // run
  // Based on some code from Josh

  if ( DEBUG > 0 )
    cout 
      << "BEGIN Run: " << run.run() << endl;

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

          // If buffers have been initialized, read triggerNames.txt
          // file and get saved trigger names

          if ( buffersInitialized ) 
            {
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

  // ---------------------------------------------------------------
  // The code below is executed once at beginRun time
  // ---------------------------------------------------------------
  if ( buffersInitialized ) return;

  buffersInitialized = true;

  // Write branches and variables to variables file 

  ofstream vout("variables.txt");
  time_t tt = time(0);
  string ct(ctime(&tt));
  vout << "tree: Events " << ct << endl;
  
  // ---------------------------------------------------------------
  // Create a buffers of appropriate type. Make sure to eliminate
  // duplicates, but warn that this is being done  
  // ---------------------------------------------------------------
  set<string> branchset;

  for(unsigned int i=0; i < blockName_.size(); i++)
    {
      if ( DEBUG > 0 )
        cout << "BLOCK NAME(" << blockName_[i] << ")" << endl;

      if ( bufferName_[i] == "edmTriggerResultsHelper" ) 
        updateTriggerBranches(i);

      // ---------------------------------------------------------------
      // Note: ->create(...) returns an auto_ptr to BufferThing. 
      // Ordinarily, an auto_ptr owns the object pointed to. But, a 
      // push_back makes a copy of the thing pushed back. The act of 
      // copying the auto_ptr, auto_ptr<BufferThing>, transfers 
      // ownership of the object from the auto_ptr to the vector. 
      // Consequently, in principle, it is up to the user of the 
      // vector to deallocate the memory occupied by the buffer 
      // objects when they are no longer needed. However, the plugin 
      // mechanism assumes (not unreasonably) that it is in charge of 
      // plugins and so is responsible for cleaning up allocated space.
      // ---------------------------------------------------------------
      if ( DEBUG > 0 )
        cout 
          << "  create buffer: " << bufferName_[i] << endl;

      // First cache block name, buffer name and get by label so that they are
      // available when the buffer is created.

      Configuration::instance().set(blockName_[i], 
                                    bufferName_[i], 
                                    label_[i], 
                                    parameters_[i]);

      buffers.push_back( BufferFactory::get()->create(bufferName_[i]) );
      if (buffers.back() == 0)
        throw cms::Exception("PluginLoadFailure")
          << "\taaargh!...let all the evil "
          << "that lurks in the mud hatch out\n"
          << "\tI'm unable to create buffer " + bufferName_[i]; 


      // ... and initialize it
      buffers.back()->init(output,
                           blockName_[i], 
                           label_[i], 
                           prefix_[i], 
                           variables_[i], 
                           maxcount_[i], 
                           vout,
                           branchset,
                           DEBUG);

      // cache addresses of buffers
      string key = buffers.back()->key();
      buffermap[key] = buffers.back();

      if ( DEBUG > 0 )
        {
          cout << DEFAULT_COLOR;
          cout << "  buffer: " << bufferName_[i] 
               << " created " << endl << endl;
          cout << "  object: " << key        << " address " << buffermap[key] 
               << endl << endl;
        }
    }
  vout.close();

  // Create ntuple analyzer template if requested

  if ( analyzername_ != "" )
    {
      string cmd("mkanalyzer.py "  + kit::nameonly(analyzername_));
      cout << cmd << endl;
      kit::shell(cmd);
    }

  // Cache variable addresses for each buffer
  // Make sure branch names are unique
  int index=0;
  cout << endl << endl << " BEGIN Branches " << endl;

  for(unsigned int i=0; i < buffers.size(); ++i)
    {
      vector<string>& vnames = buffers[i]->varnames();
      string objname = buffers[i]->key();
      cout << endl << i+1 
           << "\tobject: " << objname 
           << "\taddress: " << buffermap[objname] << endl;

      for(unsigned int ii=0; ii < vnames.size(); ++ii)
        {
          string name = vnames[ii];
          varmap[name] = buffers[i]->variable(name);
          index++;
          cout << "  " << index 
               << "\t" << name << endl;
        }
    }
  cout << " END Branches" << endl << endl;

  // Check for crash switch
  
  bool crash = true;
  try
    {
      crash = 
        (bool)Configuration::instance().
        getConfig()->getUntrackedParameter<int>("crashOnInvalidHandle");
    }
  catch (...)
    {}

  if ( crash )
    cout << "\t==> TheNtupleMaker will CRASH if a handle is invalid <==";
  else
    cout << "\t==> TheNtupleMaker will WARN if a handle is invalid <==";
  cout << endl << endl;

  cout << "END TheNtupleMaker Configuration" << endl;

  if ( DEBUG > 0 )
    cout 
      << "END Run: " << run.run() << endl;
}

// --- method called once each job just after ending the event loop  ----------
void 
TheNtupleMaker::endJob() 
{
  if ( macroEnabled_ ) gROOT->ProcessLine("obj.endJob();");

  output.close();
}

//define this as a plug-in
DEFINE_FWK_MODULE(TheNtupleMaker);
