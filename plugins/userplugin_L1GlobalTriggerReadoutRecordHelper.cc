// ----------------------------------------------------------------------------
// Created: Fri Oct  8 18:15:16 2010 by mkuserplugin.py
// Author:      Harrison B. Prosper      
//$Revision: 1.2 $
// ----------------------------------------------------------------------------
#include "PhysicsTools/TheNtupleMaker/interface/UserBuffer.h"
#include "PhysicsTools/TheNtupleMaker/interface/pluginfactory.h"
#include "PhysicsTools/TheNtupleMaker/interface/L1GlobalTriggerReadoutRecordHelper.h"
typedef UserBuffer<L1GlobalTriggerReadoutRecord, L1GlobalTriggerReadoutRecordHelper, SINGLETON>
L1GlobalTriggerReadoutRecordHelper_t;
DEFINE_EDM_PLUGIN(BufferFactory, L1GlobalTriggerReadoutRecordHelper_t,
                  "L1GlobalTriggerReadoutRecordHelper");
