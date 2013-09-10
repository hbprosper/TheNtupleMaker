#$Revision: 1.1.1.1 $ example.py
#------------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
							fileNames =
							cms.untracked.vstring("file:test.root")
							)
process.me = \
cms.EDAnalyzer("TestMethod",
               electronLabel = cms.untracked.string("selectedPatElectronsAK5")
               )

process.p = cms.Path(process.me)
