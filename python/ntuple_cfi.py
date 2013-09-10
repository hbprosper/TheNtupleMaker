#-------------------------------------------------------------------------
# Created: Tue Aug 24 22:06:57 2010 by mkntuplecfi.py
#-------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms
demo =\
cms.EDAnalyzer("TheNtupleMaker",
               ntupleName = cms.untracked.string("ntuple_reco.root"),
               analyzerName = cms.untracked.string("analyzer_reco.cc"),

               buffers =
               cms.untracked.
               vstring(
	'edmEvent',
    'HcalNoiseSummary',
	'recoHcalNoiseRBXCaloTower',
    'recoCaloJet',
    'recoCaloMET',
    'recoCaloMET1',
    'recoHcalNoiseRBX',
    'recoVertex',
	'edmTriggerResultsHelper'
    ),
               edmEvent =
               cms.untracked.
               vstring(
    'edmEvent',
    #---------------------------------------------------------------------
    '   int   isRealData()',
    '   int   id().run()',
	'   int   id().event()',
	'   int   luminosityBlock()',
	'   int   bunchCrossing()',
	'   unsigned int time().unixTime()',
	'   unsigned int time().nanosecondOffset()'
    ),			   
               HcalNoiseSummary =
               cms.untracked.
               vstring(
    'HcalNoiseSummary                hcalnoise                         1',
    #---------------------------------------------------------------------
    '   bool   passHighLevelNoiseFilter()',
    '   bool   passLooseNoiseFilter()'
    ),
               recoHcalNoiseRBXCaloTower =
               cms.untracked.
               vstring(
    'recoHcalNoiseRBXCaloTower      hcalnoise                       1000',
    #---------------------------------------------------------------------
    '   int    ieta()',
	'   int    iphi()',
	'   double hadEnergy()',
	'   int    number()'
    ),
               recoCaloJet =
               cms.untracked.
               vstring(
    'recoCaloJet                     ak5CaloJets                     500',
    #---------------------------------------------------------------------
    ' double   energy()',
    ' double   eta()',
    '  float   etaetaMoment()',
    ' double   phi()',
    '  float   phiphiMoment()',
    ' double   pt()',
    '  float   emEnergyFraction()',
    '    int   n90()'
    ),
               recoCaloMET =
               cms.untracked.
               vstring(
    'recoCaloMET                     met                             500',
    #---------------------------------------------------------------------
    ' double   et()',
    ' double   sumEt()'
    ),
               recoCaloMET1 =
               cms.untracked.
               vstring(
    'recoCaloMET                     metNoHF                         500',
    #---------------------------------------------------------------------
    ' double   et()',
    ' double   sumEt()'
    ),
               recoHcalNoiseRBX =
               cms.untracked.
               vstring(
    'recoHcalNoiseRBX                hcalnoise                       500',
    #---------------------------------------------------------------------
    ' double   caloTowerEmE()',
    ' double   caloTowerHadE()',
    '    int   idnumber()'
    ),
               recoVertex =
               cms.untracked.
               vstring(
    'recoVertex                      offlinePrimaryVertices          500',
    #---------------------------------------------------------------------
    ' double   chi2()',
    ' double   ndof()',
    ' size_t   tracksSize()',
    ' double   x()',
    ' double   y()',
    ' double   z()'
    ),
               edmTriggerResultsHelper =
               cms.untracked.
               vstring(
    "edmTriggerResultsHelper          TriggerResults::HLT               1",
    #---------------------------------------------------------------------
    '   bool   value("HLT_Jet15U")   Jet15U',
	'   bool   value("HLT_Jet30U")   Jet30U',
	'   bool   value("HLT_Jet50U")   Jet50U',
	'   bool   value("HLT_L1Jet6U")  L1Jet6U'	
    )			   			   
               )
	 
