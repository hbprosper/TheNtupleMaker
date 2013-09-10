//-----------------------------------------------------------------------------
// File: testtreestream.cc
// Created: 03-Nov-2009 Harrison B. Prosper
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include "TRandom3.h"
#include "PhysicsTools/TheNtupleMaker/interface/treestream.h"
#include "PhysicsTools/TheNtupleMaker/interface/colors.h"
//-----------------------------------------------------------------------------
using namespace std;
//-----------------------------------------------------------------------------
int 
main(int argc, char** argv)
{

  bool doThing1, doThing2, doThing3;
  doThing1=doThing2=doThing3=false;

  for(int i=0; i < argc; i++)
    {
      string arg(argv[i]);
      if ( arg == "-1" )
        doThing1 = true;
      else if ( arg == "-2" )
        doThing2 = true;
      else if ( arg == "-3" )
        doThing3 = true;
    }

  if ( doThing1 )
    {
      vector<string> fname;
      fname.push_back("testtreestream.root");
      fname.push_back("testtreestream.root");

      itreestream input(fname, "Analysis GEN");

      input.ls();
      input.close();
    }

  if ( doThing2 )
    {
      cout << endl << "Processing testtreestream.C..." << endl << endl;

      vector<string> fname;
      fname.push_back("testtreestream.root");
      fname.push_back("testtreestream.root");

      itreestream input(fname, "Analysis GEN");

      int npmax=4000;
      vector<int> pid(npmax);
      input.select("Particle.PID", pid);

      int njmax=100;
      vector<float> jetpt(njmax);
      input.select("Jet.PT", jetpt);
      
      int nentries = input.size();
      cout << "Number of entries: " << nentries << endl;

      for(int row=0; row < nentries; row++)
        {
          input.read(row);
          if ( row % 20 == 0  )
            {
              cout << "Event: " 
                   << row << endl;
              cout << "\tnparticles: " << pid.size() << endl;
              for(int i=0; i < 5; i++)
                cout << "\t\t" << i << "\t" << pid[i] << endl;
 
              cout << "\tnjets:      " << jetpt.size() << endl;
              for(unsigned int i=0; i < jetpt.size(); i++)
                cout << "\t\tjetpt[" << i << "] " << jetpt[i] << endl;
              cout << endl;
            }
        }
      input.close();
    }

  if ( doThing3 )
    {
      TRandom3 random;

      otreestream out("test.root", "Events", "test treestream");

      double adouble=0;
      long along=0;
      int aint=0;
      out.add("adouble", adouble);
      //out.add("along", along);
      out.add("aint", aint);
      
      int nrows = 1000;
      char record[80];
      for(int i=0; i < nrows; i++)
        {
          adouble = random.Gaus(20);
          //along   = random.Integer(100000) % 7;
          aint    = random.Integer(20);
          
          if ( i % 100 == 0 )
            {
              //cout << "===========================================" << endl;
              sprintf(record, "%5d %10.3f %10ld %10d", 
                      i, adouble, along, aint);
              cout << record << endl;
            }
          out.commit();
        }
      out.close();
      
      // Now read
      itreestream inp("test.root", "Events");
      inp.select("adouble", adouble);
      //inp.select("along", along);
      inp.select("aint", aint);

      for(int i=0; i < nrows; i++)
        {
          inp.read(i);
          if ( i % 100 == 0 )
            {
              sprintf(record, "%5d %10.3f %10ld %10d", 
                      i, adouble, along, aint);
              cout << record << endl;
            }
        }
      inp.close();
    }
  return 0;
}

