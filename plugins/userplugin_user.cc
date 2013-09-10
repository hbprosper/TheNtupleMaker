//-----------------------------------------------------------------------------
// Package:     PhysicsTools
// Sub-Package: TheNtupleMaker
// Description: Add user-defined methods
// Created:     Tue Jan 19, 2010 HBP
// Updated:     Mon Mar 08, 2010 Sezen & HBP - add triggerBits class
//              Thu Apr 08, 2010 Sezen & HBP - add GParticle class
//              Thu Aug 25, 2010 HBP - rename classes
//              Fri Jun 24, 2011 HBP - add sint and vint
//              Fri Jun 25, 2011 HBP - add suint and vuint
//$Revision: 1.6 $
//-----------------------------------------------------------------------------
#include "PhysicsTools/TheNtupleMaker/interface/UserBuffer.h"
#include "PhysicsTools/TheNtupleMaker/interface/pluginfactory.h"
#include "PhysicsTools/TheNtupleMaker/interface/user.h"
#include "PhysicsTools/TheNtupleMaker/interface/GenParticleHelper.h"
//-----------------------------------------------------------------------------
typedef UserBuffer<reco::GenParticle, 
                   reco::GenParticleHelper, COLLECTION> 
recoGenParticleHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, recoGenParticleHelper_t, 
                  "recoGenParticleHelper");

typedef UserBuffer<reco::HcalNoiseRBX, 
                   reco::HcalNoiseRBXCaloTower, COLLECTION> 
recoHcalNoiseRBXCaloTower_t;
DEFINE_EDM_PLUGIN(BufferFactory, recoHcalNoiseRBXCaloTower_t, 
                  "recoHcalNoiseRBXCaloTower");

typedef UserBuffer<edm::TriggerResults, 
                   edm::TriggerResultsHelper, SINGLETON> 
edmTriggerResultsHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, edmTriggerResultsHelper_t, 
                  "edmTriggerResultsHelper");

typedef UserBuffer<edm::Event, edm::EventHelper, SINGLETON> edmEventHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, edmEventHelper_t, "edmEventHelper");

// Fundamental types

typedef UserBuffer<float, BasicType<float>, SINGLETON> floatHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, floatHelper_t, "sfloat");

typedef UserBuffer<float, BasicType<float>, COLLECTION> vfloatHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, vfloatHelper_t, "vfloat");

typedef UserBuffer<double, BasicType<double>, SINGLETON> doubleHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, doubleHelper_t, "sdouble");

typedef UserBuffer<double, BasicType<double>, COLLECTION> vdoubleHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, vdoubleHelper_t, "vdouble");

typedef UserBuffer<int, BasicType<int>, SINGLETON> intHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, intHelper_t, "sint");

typedef UserBuffer<int, BasicType<int>, COLLECTION> vintHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, vintHelper_t, "vint");

typedef UserBuffer<unsigned int, BasicType<unsigned int>, SINGLETON> 
uintHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, uintHelper_t, "suint");

typedef UserBuffer<unsigned int, BasicType<unsigned int>, COLLECTION> 
vuintHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, vuintHelper_t, "vuint");

typedef UserBuffer<bool, BasicType<bool>, SINGLETON> 
boolHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, boolHelper_t, "sbool");

typedef UserBuffer<bool, BasicType<bool>, COLLECTION> 
vboolHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, vboolHelper_t, "vbool");
