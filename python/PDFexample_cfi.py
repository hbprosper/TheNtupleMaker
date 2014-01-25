#-------------------------------------------------------------------------
# Created: Fri Jan 24 14:34:57 2014 by mkntuplecfi.py
#-------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms
demo =\
cms.EDAnalyzer("TheNtupleMaker",
               ntupleName = cms.untracked.string("ntuple.root"),
               analyzerName = cms.untracked.string("analyzer.cc"),


# NOTE: the names listed below will be the prefixes for
#       the associated C++ variables created by mkanalyzer.py
#       and the asscociated C++ structs.

               buffers =
               cms.untracked.
               vstring(
    'NNPDF21',
	'MSTW2008',
    'GenParticle'
    ),
               NNPDF21 =
               cms.untracked.
               vstring(
    'GenEventInfoProductHelper       generator                       100',
    #---------------------------------------------------------------------
	'param PDFSets = NNPDF21_100.LHgrid',
	'param nset = 1',
    'double  pdfweight()',
	'double  pdf1()',
	'double  pdf2()'
    ),

               MSTW2008 =
               cms.untracked.
               vstring(
    'GenEventInfoProductHelper       generator                       100',
    #---------------------------------------------------------------------
	'param PDFSets = MSTW2008lo68cl.LHgrid',
	'param nset = 2',
    'double  pdfweight()',
	'double  pdf1()',
	'double  pdf2()'
    ),			   
               GenParticle =
               cms.untracked.
               vstring(
    'recoGenParticle                 genParticlesPruned              200',
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
    'double  eta()',
    'double  mass()',
    'int  pdgId()',
    'int  status()'
    )
               )
