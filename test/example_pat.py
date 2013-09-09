#$Revision: 1.6 $ example.py
#------------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
							fileNames =
							cms.untracked.vstring("file:pat.root"
												  )
							)
#-------------------------------------------------------------------------
# Created: Sun Feb  7 22:35:29 2010 by mkntuplecfi.py
# $Revison:$
#-------------------------------------------------------------------------
process.demo =\
cms.EDAnalyzer("TheNtupleMaker",

			   # Name of output n-tuple
               ntupleName = cms.untracked.string("ntuple_pat.root"),

			   # List of buffers to allocate.
			   #----------------------------------------------------------
			   # The names in "buffers" are arbitrary so long as each agrees
			   # with the name of a vstring block. For example, since the
			   # vstring "buffers" contains the name "recoBeamSpot", the code
			   # expects a vstring block of that name to exist.
			   # However, the names within a vstring block, for example,
			   # edmEventHelper, must correspond to a Buffer plugin.
			   # (See plugins.cc and userplugins.cc in the plugins dir.)
			   #----------------------------------------------------------
               buffers =
               cms.untracked.
               vstring(
	'edmEventHelper',
	'GenEventInfoProduct',
    'GenRunInfoProduct',
    'recoBeamSpot',
	'patMET',
    'patMuon',
 	'patElectron',
	'recoGenParticle',
 	'recoGenParticleHelper',
 	'edmTriggerResultsHelper'
    ),
			   #----------------------------------------------------------
			   # Format of 1st line:
			   #   buffer-name         getByLabel         maximum-count
			   #
			   # Format of subsequent lines:
			   #   [return-type] method [alias]
			   #----------------------------------------------------------
               edmEventHelper =
               cms.untracked.
               vstring(
    'edmEventHelper                  info',
    #---------------------------------------------------------------------
	'   bool  isRealData()',
    '   int   run()',
	'   int   event()',
	'   int   luminosityBlock()',
	'   int   bunchCrossing()'
    ),				   
               GenEventInfoProduct =
               cms.untracked.
               vstring(
    'GenEventInfoProduct             generator                         1',
    #---------------------------------------------------------------------
    ' double   weight()'
    ),
               GenRunInfoProduct =
               cms.untracked.
               vstring(
    'GenRunInfoProduct               generator                         1',
    #---------------------------------------------------------------------
    ' double   externalXSecLO().value()',
    ' double   externalXSecNLO().value()',
    ' double   filterEfficiency()',
    ' double   internalXSec().value()'
    ),			   
               recoBeamSpot =
               cms.untracked.
               vstring(
    "recoBeamSpot                    offlineBeamSpot                   1",
    #---------------------------------------------------------------------
    " double   x0()",
    " double   y0()",
    " double   z0()"
    ),
               patMET =
               cms.untracked.
               vstring(
    "patMET                         layer1METsAK5                    50",
    #---------------------------------------------------------------------
    " double   et()",
    " double   phi()",
    " double   pt()"
    ),

               patMuon =
               cms.untracked.
               vstring(
    "patMuon                         cleanLayer1Muons                50",
    #---------------------------------------------------------------------
    "    int   charge()",
    " double   energy()",
    " double   et()",
    " double   eta()",
    " double   p()",
    " double   phi()",
    " double   pt()",
    "  float   caloIso()",
    "  float   ecalIso()",
    " double   ecalIsoDeposit()->candEnergy()",
    "  float   hcalIso()",
    "  float   trackIso()",
    "   bool   isCaloMuon()",
    "   bool   isGlobalMuon()",
    "   bool   isStandAloneMuon()",
    "   bool   isTrackerMuon()"
    ),

               patElectron =
               cms.untracked.
               vstring(
    "patElectron                      cleanLayer1Electrons            50",
    #---------------------------------------------------------------------
    "    int   charge()",
    " double   energy()",
    " double   et()",
    " double   eta()",
    " double   p()",
    " double   phi()",
    " double   pt()",
	" double   gsfTrack()->d0()",
	" double   gsfTrack()->phi()"
    ),

			   recoGenParticle =
               cms.untracked.
               vstring(
    "recoGenParticle               genParticles                    4000",
    #---------------------------------------------------------------------
    "    int   charge()",
    "    int   pdgId()",
    "    int   status()",
    " double   pt()",
    " double   eta()",
    " double   phi()",
    " double   mass()"
	),
			   recoGenParticleHelper =
               cms.untracked.
               vstring(
    "recoGenParticleHelper           genParticles                    4000",
    #---------------------------------------------------------------------
	"    int   firstMother()",
	"    int   lastMother()",
	"    int   firstDaughter()",
	"    int   lastDaughter()"
    ),
               edmTriggerResultsHelper =
               cms.untracked.
               vstring(
    "edmTriggerResultsHelper         TriggerResults                     1",
    #---------------------------------------------------------------------
    '   bool   value("HLT_L1Jet15")',
	'   bool   value("HLT_Jet30")',
	'   bool   value("HLT_Jet50")',
	'   bool   value("HLT_Jet80")',
	'   bool   value("HLT_Mu9")'
    )			   			   			   
               )

process.p = cms.Path(process.demo)
