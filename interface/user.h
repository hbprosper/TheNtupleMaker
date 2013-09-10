#ifndef USER_H
#define USER_H
//-----------------------------------------------------------------------------
// Package:     PhysicsTools
// Sub-Package: TheNtupleMaker
// Description: Add user-defined methods
// Created:     Tue Jan 19, 2010 HBP
// Updated:     Mon Mar 08, 2010 Sezen & HBP - add triggerBits class
//              Thu Apr 08, 2010 Sezen & HBP - add GParticle class
//              Tue Aug 24, 2010 HBP - add HcalNoiseRBXCaloTower
//                                   - add TriggerResultsHelper
//                                   - add GenParticleHelper
//              Thu Sep 02, 2010 HBP - move HelpFor to separate file
//              Sat Jun 25, 2011 HBP - add template class for ints etc.
//              Thu Jul 04, 2013 HBP - add BasType bool
//$Revision: 1.9 $
//-----------------------------------------------------------------------------
#include <algorithm>
#include <iostream>
#include <map>

#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
//#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/METReco/interface/HcalNoiseRBX.h"
#include "PhysicsTools/TheNtupleMaker/interface/HelperFor.h"
//-----------------------------------------------------------------------------
namespace reco
{
  //---------------------------------------------------------------------------
  /// Helper class for HcalNoisRBX that unpacks the associated CaloTowers.
  class HcalNoiseRBXCaloTower : public HelperFor<reco::HcalNoiseRBX>
  {
  public:
    ///
    HcalNoiseRBXCaloTower();
    
    virtual ~HcalNoiseRBXCaloTower();

    ///
    virtual void analyzeObject();

    /// get the z-side of the tower (1/-1)
    int zside() const;
    
    /// get the tower ieta
    int ieta() const;

    /// get the tower iphi
    int iphi() const;    
    /// 
    double hadEnergy() const;
    ///
    int number() const;

  private:
    std::vector<int> zside_;
    std::vector<int> ieta_;
    std::vector<int> iphi_;
    std::vector<double> hadEnergy_;
    std::vector<int> number_;
  };
}

namespace edm 
{
  //---------------------------------------------------------------------------
  /// Helper class for TriggerResults.
  class TriggerResultsHelper : public HelperFor<edm::TriggerResults>
  {
  public:
    ///
    TriggerResultsHelper();
    
    virtual ~TriggerResultsHelper();
    
    ///
    int value(std::string tname) const;

    ///
    int prescale(std::string tname);
  };

  //---------------------------------------------------------------------------
  /// Helper class for edm::Event.
  class EventHelper : public HelperFor<edm::Event>
  {
  public:
    ///
    EventHelper();    
    
    virtual ~EventHelper();
    
    ///
    int run() const;
    ///
    int event() const;
    ///
    int luminosityBlock() const;
    ///
    int bunchCrossing() const;
    ///
    int orbitNumber() const;
    ///
    bool isRealData() const;
    ///
    unsigned int unixTime() const;
    ///
    unsigned int nanosecondOffset() const;
  }; 
}

// ----------------------------------------------------------------------------
template <typename X>
class BasicType : public HelperFor<X>
{
public:
  BasicType() : HelperFor<X>() {}
  virtual ~BasicType() {}
};

template <>
class BasicType<int> : public HelperFor<int>
{
public:
  BasicType() : HelperFor<int>() {}
  virtual ~BasicType() {}
  int value() const {return *object;}
};

template <>
class BasicType<short> : public HelperFor<short>
{
public:
  BasicType() : HelperFor<short>() {}
  virtual ~BasicType() {}
  short value() const {return *object;}
};


template <>
class BasicType<unsigned int> : public HelperFor<unsigned int>
{
public:
  BasicType() : HelperFor<unsigned int>() {}
  virtual ~BasicType() {}
  unsigned int value() const {return *object;}
};

template <>
class BasicType<unsigned short> : public HelperFor<unsigned short>
{
public:
  BasicType() : HelperFor<unsigned short>() {}
  virtual ~BasicType() {}
  unsigned short value() const {return *object;}
};

template <>
class BasicType<unsigned long> : public HelperFor<unsigned long>
{
public:
  BasicType() : HelperFor<unsigned long>() {}
  virtual ~BasicType() {}
  unsigned long value() const {return *object;}
};

template <>
class BasicType<long> : public HelperFor<long>
{
public:
  BasicType() : HelperFor<long>() {}
  virtual ~BasicType() {}
  long value() const {return *object;}
};

template <>
class BasicType<float> : public HelperFor<float>
{
public:
  BasicType() : HelperFor<float>() {}
  virtual ~BasicType() {}
  float value() const {return *object;}
};

template <>
class BasicType<double> : public HelperFor<double>
{
public:
  BasicType() : HelperFor<double>() {}
  virtual ~BasicType() {}
  double value() const {return *object;}
};

template <>
class BasicType<bool> : public HelperFor<bool>
{
public:
  BasicType() : HelperFor<bool>() {}
  virtual ~BasicType() {}
  double value() const {return *object;}
};

// template <typename VALUE, typename KEY>
// class VMap : public HelperFor<edm::ValueMap<VALUE> >
// {
// public:
//   VMap() : HelperFor<edm::ValueMap<VALUE> >() {}
//   virtual ~VMAP() {}
//   VALUE get() {ptr()->
//   //X value() const {return (X)0;}
//   //X value() const {return get();}
// };


// ----------------------------------------------------------------------------
// Synonym:
// ----------------------------------------------------------------------------
class triggerBits : public edm::TriggerResultsHelper
{
public:
  triggerBits();
  virtual ~triggerBits();
};

// ----------------------------------------------------------------------------
// utilities
// ----------------------------------------------------------------------------
/// return buffer getByLabel
std::string bufferLabel(std::string buffername);

#endif

