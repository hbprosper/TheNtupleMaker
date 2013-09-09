#ifndef GENEVENTINFOPRODUCTHELPER_H
#define GENEVENTINFOPRODUCTHELPER_H
//-----------------------------------------------------------------------------
// Package:     PhysicsTools
// Sub-Package: TheNtupleMaker
// Description: TheNtupleMaker helper class for GenEventInfoProduct
// Created:     Wed Feb 16 01:43:26 2011
// Author:      Harrison B. Prosper      
//$Revision: 1.2 $
//-----------------------------------------------------------------------------
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include "PhysicsTools/TheNtupleMaker/interface/HelperFor.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
//-----------------------------------------------------------------------------
// Note: The following variables are automatically defined and available to
//       all methods:
//         1. config          pointer to ParameterSet object
//         2. event           pointer to the current event
//         3. object          pointer to the current object
//         4. oindex          index of current object
//         5. index           index of item(s) returned by helper 
//         6. count           count per object (default = 1)
//       Items 1-6 are initialized by TheNtupleMaker. The count can be changed from
//       its default value of 1 by the helper. However, items 1-5 should not
//       be changed.
//-----------------------------------------------------------------------------

/// A helper for GenEventInfoProduct.
class GenEventInfoProductHelper : public HelperFor<GenEventInfoProduct>
{
public:
  ///
  GenEventInfoProductHelper();

  virtual ~GenEventInfoProductHelper();
  ///
  virtual void analyzeObject();
  
  ///
  double pdf1() const;

  ///
  double pdf2() const;

  ///
  double pdfweight() const;
  
  ///
  double pdfweightsum() const;
  
private:
  // -- internals
  std::string pdfsetname_;
  int nset_;
  int npdfset_;
  std::vector<double> pdf1_;
  std::vector<double> pdf2_;
  std::vector<double> pdfweight_;
  std::vector<double> pdfweightsum_;
};
#endif
