#-------------------------------------------------------------------------
# Created: Tue Oct 13 22:11:56 2020 by mkntuplecfi.py
#-------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms
TNM = cms.EDAnalyzer(
    "TheNtupleMaker",
    #analyzerName = cms.untracked.string("analyzer.cc"),


    # NOTE: the names listed below will be the prefixes for
    #       the associated C++ variables created by mkanalyzer.py

    buffers = cms.untracked.vstring(
        'Fixedgridrhoall',
        'Fixedgridrhofastjetall',
        'BeamSpot',
        'L1EmParticle',
        'Jet',
        'Jet1'
    ),

    Fixedgridrhoall = cms.untracked.vstring(
    'double                          fixedGridRhoAll                   1'
    ),

    Fixedgridrhofastjetall = cms.untracked.vstring(
    'double                          fixedGridRhoFastjetAll            1'
    ),

    BeamSpot = cms.untracked.vstring(
    'reco::BeamSpot                  offlineBeamSpot                   1',
    #---------------------------------------------------------------------
        'double  x0()',
        'double  y0()',
        'double  z0()'
    ),

    L1EmParticle = cms.untracked.vstring(
    'l1extra::L1EmParticle           l1extraParticles_Isolated        50',
    #---------------------------------------------------------------------
        'int  charge()',
        'double  energy()',
        'double  pt()',
        'double  phi()',
        'double  eta()'
    ),

    Jet = cms.untracked.vstring(
    'pat::Jet                        slimmedJets                      50',
    #---------------------------------------------------------------------
        'double  pt()',
        'double  eta()',
        'float  jetArea()',
        'double  mass()'
    ),

    Jet1 = cms.untracked.vstring(
    'pat::Jet                        slimmedJetsAK8                   50',
    #---------------------------------------------------------------------
        'double  pt()',
        'double  eta()',
        'float  jetArea()',
        'double  mass()'
    )
)
