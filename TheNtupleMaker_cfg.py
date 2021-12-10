#$Revision: 1.1 $
import FWCore.ParameterSet.Config as cms

process = cms.Process("TheNtupleMaker")

process.load("FWCore.MessageService.MessageLogger_cfi")
# See TheNtupleMaker twiki for a brief explanation
process.MessageLogger.destinations = cms.untracked.vstring("cerr")
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.MessageLogger.cerr.default.limit = 5

# This is required in order to configure HLTConfigProducer
process.load("L1TriggerConfig.L1GtConfigProducers.L1GtConfig_cff")

maxEvents = 1001

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(maxEvents) )

process.source = cms.Source("PoolSource",
                            fileNames =
                            cms.untracked.vstring("file:reco.root"
                                            ))

#process.options = cms.untracked.PSet(
#    SkipEvent   = cms.untracked.vstring('ProductNotFound'),
#    wantSummary = cms.untracked.bool(True)
#)

# output file
process.load("CommonTools.UtilAlgos.TFileService_cfi")
process.TFileService.fileName = cms.string("ntuple.root")

process.load("PhysicsTools.TheNtupleMaker.ntuple_cfi")

process.p = cms.Path(process.TNM)
