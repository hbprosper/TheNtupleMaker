#ifndef EVENTBUFFER_H
#define EVENTBUFFER_H
#ifdef PROJECT_NAME
#include "PhysicsTools/TheNtupleMaker/interface/treestream.h"
#else
#include "treestream.h"
#endif
struct eventBuffer 
{
  void saveObjects() {}
  itreestream* input;
  otreestream* output;
};
#endif
