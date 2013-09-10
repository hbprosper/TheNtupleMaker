//-----------------------------------------------------------------------------
// Package:     PhysicsTools
// Sub-Package: TheNtupleMaker
// Description: TheNtupleMaker helper class for GenEventInfoProduct
// Created:     Wed Feb 16 01:43:26 2011
// Author:      Harrison B. Prosper      
//$Revision: 1.6 $
//-----------------------------------------------------------------------------
#include <stdlib.h>

#include "PhysicsTools/TheNtupleMaker/interface/GenEventInfoProductHelper.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "LHAPDF/LHAPDF.h"
//-----------------------------------------------------------------------------
using namespace std;
static bool DBGenEventInfo = getenv("DBGenEventInfoProductHelper") > 0; 
//-----------------------------------------------------------------------------
// This constructor is called once per job
GenEventInfoProductHelper::GenEventInfoProductHelper()
  : HelperFor<GenEventInfoProduct>() 
{
  std::cout << "GenEventInfoProductHelper:" << std::endl
            << "\tblockname:  " << blockname << std::endl
            << "\tbuffername: " << buffername << std::endl
            << "\tlabelname:  " << labelname << std::endl; 

  if ( config == 0 )
    throw cms::Exception("NullConfigPointer");

  // Get the PDF set
  pdfsetname_ = parameter("PDFSets");
  if ( pdfsetname_ == "" )
    throw cms::Exception("PDFSetsError") 
      << "Please specify PDFSets in buffer: " 
      << blockname << endl;

  string snset = parameter("nset");
  if ( snset == "" )
    throw cms::Exception("PDFSetsError") 
      << "Please specify a unique number for PDFSets in buffer: "
      << blockname << endl; 

  nset_ = atoi(snset.c_str());

  string snpdfset = parameter("numberOfPDFSets");
  if ( snpdfset == "" )
    throw cms::Exception("PDFSetsError") 
      << "Please specify numberOfPDFSets in buffer: "
      << blockname << endl; 

  npdfset_ = atoi(snpdfset.c_str());
  
  LHAPDF::initPDFSet(nset_, pdfsetname_);

  cout << endl << "\t==> using PDF set:      " << pdfsetname_ << endl;
  cout << endl << "\t==> number of PDF sets: " << npdfset_ << endl;

  pdf1_.clear();
  pdf2_.clear();
  pdfweight_.clear();
  pdfweightsum_.clear();
  for(int i=0; i <= npdfset_; ++i) 
    {
      pdf1_.push_back(0);
      pdf2_.push_back(0);
      pdfweight_.push_back(0);
      pdfweightsum_.push_back(0);
    }
  pdfweight_[0] = 1;
}
    
GenEventInfoProductHelper::~GenEventInfoProductHelper() 
{}


// -- Called once per object
void GenEventInfoProductHelper::analyzeObject()
{
  if ( object == 0 )
    throw cms::Exception("NullObjectPointer");
  if ( !object->hasPDF() ) return;

  const gen::PdfInfo* xpdf = object->pdf();
          
  double x1 = xpdf->x.first;
  double x2 = xpdf->x.second;
  double q  = xpdf->scalePDF;
  int id1   = xpdf->id.first;
  int id2   = xpdf->id.second;

  // Get pdf central values

  LHAPDF::usePDFMember(nset_, 0);
  pdf1_[0] = LHAPDF::xfx(nset_, x1, q, id1);
  pdf2_[0] = LHAPDF::xfx(nset_, x2, q, id2);
  
  if ( DBGenEventInfo )
    {
      char record[256];
      sprintf(record, 
              "    x1 : %10.2e     x2:  %10.2e\n"
              "pdf(x1): %10.2e pdf(x2): %10.2e\tq: %10.2e",
              x1, x2, pdf1_[0], pdf2_[0], q);
      cout << record << endl << endl;
    }

  double w0 = pdf1_[0] * pdf2_[0];
  for(int i=1; i <= npdfset_; ++i)
    {
      LHAPDF::usePDFMember(nset_, i);
      pdf1_[i] = LHAPDF::xfx(nset_, x1, q, id1);
      pdf2_[i] = LHAPDF::xfx(nset_, x2, q, id2);
      pdfweight_[i] = pdf1_[i] * pdf2_[i] / w0;
      pdfweightsum_[i] += pdfweight_[i];
    }
  count = npdfset_+1; // One more than the number of PDF sets
}

// -- Access Methods
double GenEventInfoProductHelper::pdfweight()  const
{
  if ( index <= npdfset_ )
    return  pdfweight_[index];
  else
    return -1;
}

double GenEventInfoProductHelper::pdfweightsum()  const
{
  if ( index <= npdfset_ )
    return  pdfweightsum_[index];
  else
    return -1;
}

double GenEventInfoProductHelper::pdf1()  const
{
  if ( index <= npdfset_ )
    return  pdf1_[index];
  else
    return -1;
}

double GenEventInfoProductHelper::pdf2()  const
{
  if ( index <= npdfset_ )
    return  pdf2_[index];
  else
    return -1;
}
