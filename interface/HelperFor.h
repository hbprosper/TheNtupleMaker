#ifndef HELPERFOR_H
#define HELPERFOR_H
//-----------------------------------------------------------------------------
// Package:     PhysicsTools
// Sub-Package: TheNtupleMaker
// Description: Base class for helpers
// Created:     Aug, 2010 Harrison B. Prosper
//              01 May, 2011 HBP add param
// Updated:     05 Sep, 2013 HBP remove explicit dependency on Event.h
//                           HLTConfigProvider.h and ParameterSet.h
//$Revision: 1.7 $
//-----------------------------------------------------------------------------
#include <sstream>
#include <memory>
#include "PhysicsTools/TheNtupleMaker/interface/Configuration.h"
#include "PhysicsTools/TheNtupleMaker/interface/CurrentEvent.h"
//-----------------------------------------------------------------------------

// The headers for these must be included in the codes that include this
// header. These codes are typically helpers.

namespace edm {
class Event;
class ParameterSet;
}
class HLTConfigProvider;

/// Base class for helpers.
template <typename X>
class HelperFor
{
public:
  HelperFor() : blockname(Configuration::instance().getBlockname()),
                buffername(Configuration::instance().getBuffername()),
                labelname(Configuration::instance().getLabelname()),
                parameters(Configuration::instance().getParameters()),
                config(Configuration::instance().getConfig()),
                hltconfig(Configuration::instance().getHLTconfig()),

                event(0),      // pointer to current event
                eventsetup(0), // pointer to current event setup
                object(0),     // pointer to current helped object
                oindex(0),     // index of current helped object (dumb pointer)
                index(0),      // index of current helper object (dumb pointer)
                count(1)       // number of instances returned by helper

  {}

  virtual ~HelperFor() {}

  ///
  void cacheEvent()
  { 
    event      = CurrentEvent::instance().get();
    eventsetup = CurrentEvent::instance().getsetup();
    hltconfig  = Configuration::instance().getHLTconfig();
  }

  ///
  void cacheObject(const X& o, int n=0) 
  { 
    object = &o; 
    oindex = n; 
    count  = 1; 
  }

  /// return number of items per cached object
  int size() const { return count; }

  /// set index of item to be retrieved.
  void set(int n) { index = n; }

  // ---------------- can be overridden

  /// do some event-level analysis, if needed.
  virtual void analyzeEvent() {}

  /// do some object-level analysis, if needed.
  virtual void analyzeObject() {}

  /// do some post event-level analysis, if needed.
  virtual void flushEvent() {}

  // ---------------- available to user
  
  std::string blockname;
  std::string buffername;
  std::string labelname;
  std::map<std::string, std::string> parameters;

  
  template <typename T>
  T parameter(std::string key)
  {
    if ( parameters.find(key) != parameters.end() ) 
      {
        T a;
        std::istringstream iss(parameters[key]);
        iss >> a;
        return a;
      }
    return (T)0;
  }


  std::string parameter(std::string key)
  {
    if ( parameters.find(key) != parameters.end() ) 
      return parameters[key];
    else
      return "";
  }
  

  /// Pointer to ParameterSet initialized from config file.
  const edm::ParameterSet* config;

  ///
  const HLTConfigProvider* hltconfig;

  /// Parameter set local to buffer
  //const edm::ParameterSet  localconfig;

  /// Pointer to event.
  const edm::Event* event;

  /// Pointer to event.
  const edm::EventSetup* eventsetup;

  /// Pointer to current (helped) object
  const X* object;

  /// Index of current (helped) object (with count starting at zero).
  int oindex;

  /// Index of current helper object, which could differ from oindex..
  int index;

  /// 
  int count;
};


#endif

