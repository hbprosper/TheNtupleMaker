#ifndef GENPARTICLEHELPER_H
#define GENPARTICLEHELPER_H
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
//                                     move classes to separate files
//$Revision: 1.1.1.1 $
//-----------------------------------------------------------------------------
#include <algorithm>
#include <iostream>
#include <map>

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "PhysicsTools/TheNtupleMaker/interface/HelperFor.h"
//-----------------------------------------------------------------------------
namespace reco
{
  //---------------------------------------------------------------------------
  /// A helper class for reco::GenParticle.
  class GenParticleHelper : public HelperFor<reco::GenParticle>
  {
  public:
    ///
    GenParticleHelper();

    virtual ~GenParticleHelper();

    /// 
    virtual void analyzeEvent();
    /// 
    virtual void analyzeObject();
    ///
    int   charge() const;
    ///
    int   pdgId() const;
    ///
    int   status() const;
    ///
    double   energy() const;
    ///
    double   pt() const;
    ///
    double   eta() const;
    ///
    double   phi() const;
    ///
    double   mass() const;
    ///
    int firstMother() const;
    ///
    int lastMother()  const;
    ///
    int firstDaughter() const;
    ///
    int lastDaughter()  const;
    
  private:
    // Filled once per cached object
    std::vector<int> mothers_;
    std::vector<int> daughters_;

    // Filled once per event
    std::map<std::string, int> amap;
  };
}
// ----------------------------------------------------------------------------
// Synonyms
// ----------------------------------------------------------------------------
class GParticle : public reco::GenParticleHelper
{
public:
  GParticle();
  virtual ~GParticle();
};

#endif

