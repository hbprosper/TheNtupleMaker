//-----------------------------------------------------------------------------
// Package:     PhysicsTools
// Sub-Package: TheNtupleMaker
// Description: Add user-defined methods
// Created:     Tue Jan 19, 2010 HBP
// Updated:     Mon Mar 08, 2010 Sezen & HBP - add triggerBits class
//              Tue Aug 24, 2010 HBP - add HcalNoiseRBXHelper
//              Thu Sep 02, 2010 HBP - update to new version of HelperFor
//              Mon May 07, 2012 HBP - value and prescale of 
//                                     TriggerResultsHelper now returns -1
//                                     if a trigger does not exist
//$Revision: 1.10 $
//-----------------------------------------------------------------------------
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "PhysicsTools/TheNtupleMaker/interface/Buffer.h"
#include "PhysicsTools/TheNtupleMaker/interface/user.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
//-----------------------------------------------------------------------------
using namespace edm;
using namespace reco;
using namespace std;
//-----------------------------------------------------------------------------
/*
  The following variables are available to all helpers
  config            pointee to global configuration object
  hltconfig         pointer to current HLT configuration
  event             pointer to current event
  eventsetup        pointer to current event setup
  object            pointer to current helped object

  oindex            index of current helped object (dumb pointer)
  index             index of current helper object (dumb pointer)
  count             number of instances returned by helper/helped object
 */
//-----------------------------------------------------------------------------
namespace {
  void split(string str, vector<string>& vstr)
  {
    vstr.clear();
    istringstream stream(str);
    while ( stream )
      {
        string str;
        stream >> str;
        if ( stream ) vstr.push_back(str);
      }
  }
}

//-----------------------------------------------------------------------------
// TriggerResults helper
//-----------------------------------------------------------------------------
TriggerResultsHelper::TriggerResultsHelper() 
  : HelperFor<edm::TriggerResults>()
{
  std::cout << "TriggerResultsHelper:" << std::endl
            << "\tblockname:  " << blockname << std::endl
            << "\tbuffername: " << buffername << std::endl
            << "\tlabelname:  " << labelname << std::endl; 
}
    
TriggerResultsHelper::~TriggerResultsHelper() {}

///
int 
TriggerResultsHelper::value(std::string name) const
{
  if ( event == 0 )
    throw edm::Exception(edm::errors::Configuration,
                         "\nTriggerResultsHelper::value - " 
                         "event pointer is ZERO");
  
  // NB: use a reference to avoid expensive copying
  const edm::TriggerNames& tnames = event->triggerNames(*object);

  // Get bit associated with trigger name
  try
    {
      unsigned int bit = tnames.triggerIndex(name);
  
      // If trigger does not exist issue a warning

      if ( bit >= tnames.size() )
        {
          edm::LogWarning("TriggerNotFound")
            << "TriggerResultsHelper: " 
            << "trigger \"" + name + "\" NOT FOUND"
            << std::endl;
          return -1;
        }
      else
        return object->accept(bit) ? 1 : 0;
    }
  catch (...)
    {
      edm::LogWarning("TriggerFailure")
        << "TriggerResultsHelper: " 
        << "cannot access trigger \"" + name + "\""
        << std::endl;
      return -1;
    }
}

///
int
TriggerResultsHelper::prescale(std::string name)
{
  if ( event == 0 )
    throw edm::Exception(edm::errors::Configuration,
                         "\nTriggerResultsHelper - " 
                         "event pointer is ZERO");
  
  if ( hltconfig == 0 )
    { 
      edm::LogWarning("HLTConfigProviderNotInitialized")
        << "TriggerResultsHelper: " 
        << "HLTConfigProvider has not been initialized"
        << std::endl;
      return -1;
    }
  
  // NB: use a reference to avoid expensive copying
  try
    {
      const edm::TriggerNames& tnames = event->triggerNames(*object);

      // Get bit associated with trigger name
      unsigned int bit = tnames.triggerIndex(name);
  
      // If trigger does not exist issue a warning

      if ( bit >= tnames.size() )
        {
          edm::LogWarning("TriggerNotFound")
            << "TriggerResultsHelper: " 
            << "trigger \"" + name + "\" NOT FOUND"
            << std::endl;
          return -1;
        }
      else
        {
          if ( !event->isRealData() ) 
            return 1;
          else if (hltconfig->prescaleSet(*event, *eventsetup) != -1)
            return (int)hltconfig->prescaleValue(*event, *eventsetup, name);
          else
            return -1;
        }
    }
  catch (...)
    {
      edm::LogWarning("TriggerPrescaleFailure")
        << "TriggerResultsHelper: "
        << "cannot access prescale for trigger \"" + name + "\""
        << std::endl;
      return -1;
    }
}


//-----------------------------------------------------------------------------
// Event helper
//-----------------------------------------------------------------------------
///
EventHelper::EventHelper() : HelperFor<edm::Event>() 
{
  std::cout << "EventHelper:" << std::endl
            << "\tblockname:  " << blockname << std::endl
            << "\tbuffername: " << buffername << std::endl
            << "\tlabelname:  " << labelname << std::endl; 
}
    
EventHelper::~EventHelper() {}

///
int EventHelper::run() const { return object->id().run(); }

///
int EventHelper::event() const { return object->id().event(); }

///
int EventHelper::luminosityBlock() const { return object->luminosityBlock(); }

///
int EventHelper::bunchCrossing() const { return object->bunchCrossing(); }

///
int EventHelper::orbitNumber() const { return object->orbitNumber(); }

///
bool EventHelper::isRealData() const { return object->isRealData(); }

///
unsigned int EventHelper::unixTime() const {return object->time().unixTime();}

///
unsigned int EventHelper::nanosecondOffset() const 
// Note: change to microsecondOffset for version 4_2_0
{ return 0;} //object->time().nanosecondOffset(); }

//-----------------------------------------------------------------------------
// HcalNoiseRBX helper
//-----------------------------------------------------------------------------
HcalNoiseRBXCaloTower::HcalNoiseRBXCaloTower() 
  : HelperFor<reco::HcalNoiseRBX>() {}


void
HcalNoiseRBXCaloTower::analyzeObject()
{
  // NB: remember to clear!
  zside_.clear();
  ieta_.clear();
  iphi_.clear();
  hadEnergy_.clear();
  number_.clear();

  // loop over HPDs for given RBX
  for(std::vector<HcalNoiseHPD>::const_iterator 
        hpd = object->HPDsBegin(); hpd != object->HPDsEnd(); hpd++)
    {
      // loop over calo towers
      const RefVector<CaloTowerCollection> towers = hpd->caloTowers();
      for(unsigned int tower=0; tower < towers.size(); tower++)
        {
          zside_.push_back(towers[tower]->id().zside());
          ieta_.push_back(towers[tower]->id().ieta());
          iphi_.push_back(towers[tower]->id().iphi());
          hadEnergy_.push_back(towers[tower]->hadEnergy());
          number_.push_back(oindex); // dumb pointer to helped object
        }
    }
  // Important: Remember to update count
  count = zside_.size();
}
    
HcalNoiseRBXCaloTower::~HcalNoiseRBXCaloTower() {}

///
int 
HcalNoiseRBXCaloTower::zside() const
{
  if ( index < (int)zside_.size() ) 
    return zside_[index]; 
  else
    return -9999;
}

///
int 
HcalNoiseRBXCaloTower::ieta() const
{
  if ( index < (int)ieta_.size() ) 
    return ieta_[index];
  else
    return -9999;
}

///
int 
HcalNoiseRBXCaloTower::iphi() const
{
  if ( index < (int)iphi_.size() ) 
    return iphi_[index];
  else
    return -9999;
}

///
double 
HcalNoiseRBXCaloTower::hadEnergy() const
{
  if ( index < (int)hadEnergy_.size() ) 
    return hadEnergy_[index]; 
  else
    return -9999;
}

///
int 
HcalNoiseRBXCaloTower::number() const
{
  if ( index < (int)number_.size() ) 
    return number_[index];
  else
    return -9999;
}

//-----------------------------------------------------------------------------
// Synonyms:
//-----------------------------------------------------------------------------
triggerBits::triggerBits() : TriggerResultsHelper() {}
triggerBits::~triggerBits() {}
//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------
string
bufferLabel(string buffername)
{
  const edm::ParameterSet* config = Configuration::instance().getConfig();
  if ( config == 0 )
    throw cms::Exception("InvalidPointer", "bufferLabel, config = 0");

  vector<string> vrecords = config->
    getUntrackedParameter<vector<string> >(buffername);

  string label("");
  if ( vrecords.size() == 0 ) return label;

  string record = vrecords[0];
  vector<string> field;              
  split(record, field);
  if ( field.size() < 2 ) return label;

  label = field[1];
  return label;
}
// //-----------------------------------------------------------------------------
// doubleHelper::doubleHelper() : HelperFor<double>() {}
// doubleHelper::~doubleHelper() {}
// double   doubleHelper::value()  const { return *object; }

// //-----------------------------------------------------------------------------
// intHelper::intHelper() : HelperFor<int>() {}
// intHelper::~intHelper() {}
// int   intHelper::value()  const { return *object; }
