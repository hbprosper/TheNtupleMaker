#-------------------------------------------------------------------------
# Created: Thu Oct 15 10:31:07 2020 by mkntuplecfi.py
#-------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms
TNM = cms.EDAnalyzer(
    "TheNtupleMaker",
    #analyzerName = cms.untracked.string("analyzer.cc"),


    # NOTE: the names listed below will be the prefixes for
    #       the associated C++ variables created by mkanalyzer.py

    buffers = cms.untracked.vstring(
        'Ak7pfjets_rho',
        'Ak7pfjets_sigma',
        'Ak7pfjets_rhos',
        'Ak7pfjets_sigmas',
        'BeamSpot',
        'L1AcceptBunchCrossing',
        'L1EmParticle',
        'PFJet',
        'PFJet1'
    ),

    Ak7pfjets_rho = cms.untracked.vstring(
    'double                          ak7PFJets_rho                     1'
    ),

    Ak7pfjets_sigma = cms.untracked.vstring(
    'double                          ak7PFJets_sigma                   1'
    ),

    BeamSpot = cms.untracked.vstring(
    'reco::BeamSpot                  offlineBeamSpot                   1',
    #---------------------------------------------------------------------
        'double  z0()'
    ),

    L1AcceptBunchCrossing = cms.untracked.vstring(
    'L1AcceptBunchCrossing           scalersRawToDigi                 50',
    #---------------------------------------------------------------------
        'string  name()',
        'bool  empty()',
        'unsigned int  bunchCrossing()'
    ),

    Ak7pfjets_rhos = cms.untracked.vstring(
    'double                          ak7PFJets_rhos                   50'
    ),

    Ak7pfjets_sigmas = cms.untracked.vstring(
    'double                          ak7PFJets_sigmas                 50'
    ),

    L1EmParticle = cms.untracked.vstring(
    'l1extra::L1EmParticle           l1extraParticles_Isolated        50',
    #---------------------------------------------------------------------
        'double  pt()',
        'double  phi()',
        'double  eta()'
    ),

    PFJet = cms.untracked.vstring(
    'reco::PFJet                     ak5PFJets                        50',
    #---------------------------------------------------------------------
        'double  pt()',
        'double  phi()',
        'double  eta()',
        'float  chargedHadronEnergy()',
        'float  neutralHadronEnergy()',
        'int  chargedHadronMultiplicity()',
        'int  neutralHadronMultiplicity()',
        'int  nConstituents()'
    ),

    PFJet1 = cms.untracked.vstring(
    'reco::PFJet                     ak7PFJets                        50',
    #---------------------------------------------------------------------
        'double  pt()',
        'double  phi()',
        'double  eta()',
        'float  chargedHadronEnergy()',
        'float  neutralHadronEnergy()',
        'int  chargedHadronMultiplicity()',
        'int  neutralHadronMultiplicity()',
        'int  nConstituents()'
    )
)
