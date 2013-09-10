#ifndef CURRENTEVENT_H
#define CURRENTEVENT_H
//
// Package:    PhysicsTools/TheNtupleMaker
//             CurrentEvent.h
//
//             A singleton object to make event available to whoever needs it
//
// Original Author:  Harrison B. Prosper
//         Created:  Mon Mar  8, 2010
//
// $Id: CurrentEvent.h,v 1.1.1.1 2011/05/04 13:04:28 prosper Exp $

#include <map>
#include <vector>
#include <string>
//#include "FWCore/Framework/interface/Event.h"
#include "PhysicsTools/TheNtupleMaker/interface/BufferUtil.h"

namespace edm {
class Event;
class EventSetup;
}

/// A singleton class to cache event.
class CurrentEvent
{
public:
  ///
  static CurrentEvent& instance()
  {
    static CurrentEvent singleton;
    return singleton;
  }

  ///
  void set(const edm::Event& event, const edm::EventSetup& setup) 
  { 
    event_ = &event;
    setup_ = &setup;
  }

  ///
  const edm::Event* get() const { return event_; }

  ///
  const edm::EventSetup* getsetup() const { return setup_; }

private:
  CurrentEvent() {}        // prevent explicit creation
  ~CurrentEvent() {}                  
  CurrentEvent(const CurrentEvent&);             // prevent copy
  CurrentEvent& operator=(const CurrentEvent&);  // prevent assignment
  
  const edm::Event* event_;
  const edm::EventSetup* setup_;
};

#endif
