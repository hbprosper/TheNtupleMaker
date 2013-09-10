//----------------------------------------------------------------------------
// File:    GEvent.cc
// Purpose: Simple interface to GenParticles
// Created: 16-Feb-2010 Harrison B. Prosper
//          Based on code written in 2005
// Updated: 28-May-2010 HBP - add first and last mothers
//$Revision: 1.1.1.1 $
//----------------------------------------------------------------------------
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include "TLorentzVector.h"
#include "PhysicsTools/TheNtupleMaker/interface/GEvent.h"
#include "PhysicsTools/TheNtupleMaker/interface/kit.h"
//----------------------------------------------------------------------------

using namespace std;

// Hide in an anonymous namespace

namespace {

  const int MAXDEPTH=99;

  struct PID
  {
    int pid;
    int index;
    bool operator<(const PID& o) const { return abs(this->pid) < abs(o.pid); }
  };
}

GEvent::GEvent()
  : nhep(0),

    firstMother(std::vector<int>(MAXSIZE, 0)),
    lastMother(std::vector<int>(MAXSIZE, 0)),

    firstDaughter(std::vector<int>(MAXSIZE, 0)),
    lastDaughter (std::vector<int>(MAXSIZE, 0)),

    pdgId  (std::vector<int>(MAXSIZE, 0)),
    status (std::vector<int>(MAXSIZE, 0)),
    pt     (std::vector<float>(MAXSIZE, 0)),
    eta    (std::vector<float>(MAXSIZE, 0)),
    phi    (std::vector<float>(MAXSIZE, 0)),
    mass   (std::vector<float>(MAXSIZE, 0)),
    charge (std::vector<float>(MAXSIZE,  0))
{}

GEvent::~GEvent() {}

vector<int>
GEvent::daughters(int index, int istat)
{
  nhep = pdgId.size();
  std::vector<int> d;
  if ( index < 0 || index >= nhep ) return d;

  std::vector<PID> vpid;
  // Find daughters. 
  // The first and last indices specify the range within which
  // the daughters are to be found. It is necessary to check the mother
  // of each potential daughter to be sure we have the correct daughters

  for (int dindex = firstDaughter[index]; dindex <= lastDaughter[index]; 
       dindex++)
    {
      if ( status[dindex] != istat ) continue;
      if ( !(firstMother[dindex] == index ||
             lastMother [dindex] == index) ) continue;
      vpid.push_back(PID());
      vpid.back().index = dindex;
      vpid.back().pid = pdgId[dindex];
    }
  std::sort(vpid.begin(), vpid.end());
  for(unsigned i=0; i < vpid.size(); i++) d.push_back(vpid[i].index);
  return d;
}

int
GEvent::find(int id, int start)
{
  nhep = pdgId.size();
  for(int index=start; index < nhep; index++)
    {
      int pid = pdgId[index];
      if ( id == SUSY )
        {
          int apid = abs(pid);
          if ( apid > SUSY && apid <= GRAVITINO )
            return index;
        }
      else if ( id == pid )
        return index;
    }
  return -1;
}

string
GEvent::name(int index)
{
  if ( index < 0 || index >= nhep ) return "";
  return kit::particleName(pdgId[index]);
}

//---------------------------------------------------------------------------
// Purpose: Print Gen Event in a tree format
// Created: 06-Sep-2004 Harrison B. Prosper
//---------------------------------------------------------------------------
void 
GEvent::printTable(std::ostream& stream, int maxrows)
{
  nhep = pdgId.size();

  char record[512];
  sprintf(record, "%-4s %-16s %4s %4s %4s %4s %10s %10s %10s %10s %6s", 
          " ", "name", "m1", "m2", "d1", "d2", 
          "pt", "eta", "phi", "mass", "status");
  stream << record << endl;
  int nrows = maxrows < nhep ? maxrows : nhep;
  for(int i=0; i < nrows; i++)
    {
      sprintf(record,
              "%4d %-16s %4d %4d %4d %4d %10.2e %10.2e %10.2e %10.2e %6d", 
              i, kit::particleName(pdgId[i]).c_str(), 
              firstMother[i], 
              lastMother[i], 
              firstDaughter[i], 
              lastDaughter[i],
              pt[i],
              eta[i],
              phi[i],
              mass[i],
              status[i]);
      stream << record << endl;
    }
}

std::string
GEvent::table(int maxcount)
{
  std::ostringstream os;
  printTable(os, maxcount);
  return os.str();
}

//---------------------------------------------------------------------------
// Purpose: Print Gen Event in a tree format
// Created: 06-Sep-2004 Harrison B. Prosper
//---------------------------------------------------------------------------
void 
GEvent::printTree(std::ostream& stream,
                  int           index,
                  int           printLevel,
                  int           maxdepth,
                  int           depth)
{
  nhep = pdgId.size(); // Need number of particles

  depth++;  

  if ( index < 0 ) return;
  if ( index >= nhep )  return;
  if ( depth >  (maxdepth <= 0 ? MAXDEPTH : maxdepth) ) return;    
  
  int ppid = pdgId[index];

  char record[255];
  if ( depth > 1 )
    {
      for (int i = 1; i < depth; i++) stream << "---";
      stream << "  ";
    }
  sprintf(record, "%-16s", kit::strip(kit::particleName(ppid)).c_str());
  stream << record; 
  
  if ( printLevel > 0 )
    {
      stream.setf(std::ios::right);  // right justify  
      stream.precision(4);
      stream << "\t"	    
             << "(" 
             << pt[index]  << ", "
             << eta[index] << ", "  
             << phi[index] << ", "
             << mass[index]
             << ")";
    }
  if ( printLevel > 1 )
    {
      stream << "\t" 
             << pdgId[index] <<  " "
             << index
             << status[index];
    }
  stream << std::endl;

  
  if ( firstDaughter[index] < 0 ) return;

  // Find daughters. 
  // The first and last indices specify the range within which
  // the daughters are to be found. It is necessary to check the mother
  // of each potential daughter to be sure we have the correct daughters

  std::vector<PID> d;
  for (int dindex = firstDaughter[index]; dindex <= lastDaughter[index]; 
       dindex++)
    {
      if ( status[dindex] != 3 ) continue;
      if ( !(firstMother[dindex] == index ||
             lastMother [dindex] == index) ) continue;
      
      PID p; 
      p.index = dindex;
      p.pid   = pdgId[dindex];
      d.push_back(p);
    }
  // Sort them in order of increasing PDG ID.
  std::sort(d.begin(), d.end());
  
  // Loop over daughters

  for (unsigned i=0; i < d.size(); i++)
    printTree(stream,
              d[i].index,
              printLevel,
              maxdepth,
              depth);
}

std::string
GEvent::tree(int      index,
             int      printLevel,
             int      maxdepth,
             int      depth)
{
  std::ostringstream os;
  if ( index < 0 )
    {
      printTree(os, 0, printLevel, maxdepth, depth);
      printTree(os, 1, printLevel, maxdepth, depth);
    }
  else
    printTree(os, index, printLevel, maxdepth, depth);
  
  return os.str();
}
