//-----------------------------------------------------------------------------
// File: plugins_t.cc
// Created: Dec-2009 Harrison B. Prosper
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "PhysicsTools/TheNtupleMaker/interface/pluginfactory.h"
//-----------------------------------------------------------------------------
using namespace std;
using namespace edmplugin;

void fatal(string message)
{
  cout << "** error ** " << message << endl;
  exit(0);
}

using namespace std;

//-----------------------------------------------------------------------------
int 
main(int argc, char** argv)
{
  cout << "testbufplugins" << endl;
  cout << "\tenabling autoloader..." << endl;
  AutoLibraryLoader::enable();
  cout << "\t\tdone!\n" << endl;

  std::auto_ptr<BufferThing> b4(BufferFactory::get()->create("recoTrack"));
  assert(b4.get() != 0);

  std::auto_ptr<BufferThing> b1(BufferFactory::get()->create("patMuon"));
  assert(b1.get() != 0);

  std::auto_ptr<BufferThing> b2(BufferFactory::get()->create("patElectron"));
  assert(b2.get() != 0);

  std::auto_ptr<BufferThing> b3(BufferFactory::get()->create("recoBeamSpot"));
  assert(b3.get() != 0);

  return 0;
}

