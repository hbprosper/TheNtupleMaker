#ifndef L1GLOBALTRIGGERREADOUTRECORDHELPER_H
#define L1GLOBALTRIGGERREADOUTRECORDHELPER_H
//-----------------------------------------------------------------------------
// Package:     PhysicsTools
// Sub-Package: TheNtupleMaker
// Description: TheNtupleMaker helper class for L1GlobalTriggerReadoutRecord
// Created:     Fri Oct  8 18:15:16 2010
// Author:      Harrison B. Prosper      
//$Revision: 1.1.1.1 $
//-----------------------------------------------------------------------------
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include "PhysicsTools/TheNtupleMaker/interface/HelperFor.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"
#include "L1Trigger/GlobalTriggerAnalyzer/interface/L1GtUtils.h"
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

/// A helper for L1GlobalTriggerReadoutRecord.
class L1GlobalTriggerReadoutRecordHelper : 
  public HelperFor<L1GlobalTriggerReadoutRecord>
{
public:
  ///
  L1GlobalTriggerReadoutRecordHelper();

  virtual ~L1GlobalTriggerReadoutRecordHelper();

  virtual void analyzeEvent();
  
  // -- Access Methods

  // -- Note: access methods must be declared const
  // -- code: -1 decision before mask
  // -- code: +1 decision after mask
  bool value(std::string triggerName, int code) const;
  ///
  int  prescale(std::string) const;
  
private:
  // -- internals
  L1GtUtils l1gtutils;
};
#endif
