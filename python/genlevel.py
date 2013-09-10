#-------------------------------------------------------------------------
# Created: Sat Sep 18 17:35:14 2010
#-------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms
demo =\
cms.EDAnalyzer("TheNtupleMaker",
               ntupleName = cms.untracked.string("ntuple.root"),
               analyzerName = cms.untracked.string("analyzer.cc"),

               buffers =
               cms.untracked.
               vstring(
	'GenEventInfoProduct',
	'GenEventInfoProductHelper',
    'recoGenParticleHelper'
    ),
              GenEventInfoProduct =
              cms.untracked.
              vstring(
   'GenEventInfoProduct             generator                         1',
   #---------------------------------------------------------------------
   ' double   weight()',
   ' double   pdf()->x.first     x1',
   ' double   pdf()->x.second    x2',
   ' double   pdf()->xPDF.first  pdf1',
   ' double   pdf()->xPDF.second pdf2',
   ' double   pdf()->scalePDF    scalePDF',
   ' int      pdf()->id.first    id1',
   ' int      pdf()->id.second   id2'
   ),
			   # Object to get PDF values and weights
			   # 0 - central values
			   # 1 to N (N=44 for CTEQ) +/- "1-sigma" shifts
			   
			   PDFSet = cms.untracked.string("cteq66.LHgrid"),
			   numberPDFSets = cms.untracked.int32(44),
			   
			   GenEventInfoProductHelper =
			   cms.untracked.
			   vstring(
   'GenEventInfoProductHelper       generator                        50',
   #---------------------------------------------------------------------
   ' double   pdf1()',
   ' double   pdf2()',
   ' double   pdfweight()',
   ' double   pdfweightsum()'
   ),
              recoGenParticleHelper =
              cms.untracked.
              vstring(
   "recoGenParticleHelper           genParticles                     500",
   #---------------------------------------------------------------------
   "    int   firstMother()",
   "    int   lastMother()",
   "    int   firstDaughter()",
   "    int   lastDaughter()",
   "    int   charge()",
   "    int   pdgId()",
   "    int   status()",
   " double   pt()",
   " double   eta()",
   " double   phi()",
   " double   mass()"
   )
               )
