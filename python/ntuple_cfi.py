#-------------------------------------------------------------------------
# Created: Thu Dec  9 01:57:57 2021 by mkntuplecfi.py
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
        'Electron'
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
        'double  z0()',
        'double  sigmaZ()'
    ),

    Electron = cms.untracked.vstring(
    'pat::Electron                   slimmedElectrons                 50',
    #---------------------------------------------------------------------
        'int  charge()',
        'double  p()',
        'double  energy()',
        'double  et()',
        'double  px()',
        'double  py()',
        'double  pz()',
        'double  pt()',
        'double  phi()',
        'double  eta()'
    )
)
