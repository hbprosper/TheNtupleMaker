//-----------------------------------------------------------------------------
// File:        addvars.cc
// Description: user macro
// Created:     Sat May  5 23:32:08 2012 by mkmacro.py
// Author:      Harry Prosper
//-----------------------------------------------------------------------------
#include "addvars.h"
//-----------------------------------------------------------------------------
using namespace std;

struct addvarsInternal
{
  int counter;
  float HT;
};

void addvars::beginJob()
{
  local = new addvarsInternal();
  local->counter = 0;
  std::cout << "BEGIN JOB addvars tree " << tree << std::endl;

  // Add a float variable to ntuple
  tree->Branch("HT", &local->HT, "HT/F");

  select("jet");
}

void addvars::endJob()
{
  if ( local ) delete local;
}


bool addvars::analyze()
{

  // compute variables
  // apply cuts etc.

  local->HT = 0;
  for(unsigned int i=0; i < jet_pt.size();++i) 
    {
      if ( !(jet_pt[i] > 100) ) continue;
      if ( !(jet_pt[i] < 400) ) continue;

      local->HT += jet_pt[i];

      std::cout << "\t===> select jet " << i << " with pt " << jet_pt[i]
                << std::endl;

      select("jet", i);
    }
  
  //if ( miserable_event )
  //  return false;
  //else
  return true;
}
