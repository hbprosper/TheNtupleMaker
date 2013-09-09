//-----------------------------------------------------------------------------
//  File:    RGS.cc
//  Purpose: Implements a version of rgsearch suitable for use in Python.
//  Created: 18-Aug-2000 Harrison B. Prosper, Chandigarh, India
//  Updated: 05-Apr-2002 HBP tidy up
//           17-May-2006 HBP use weightindex instead of a vector of weights
//           11-Aug-2012 HBP & Sezen - generalize
//-----------------------------------------------------------------------------
//$Revision: 1.3 $
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <set>

#ifdef PROJECT_NAME
#include "PhysicsTools/TheNtupleMaker/interface/RGS.h"
#else
#include "RGS.h"
#endif

#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"

using namespace std;

#ifdef __WITH_CINT__
ClassImp(RGS)
#endif

string rgsversion() 
{
  return string("RGS - $Revision: 1.3 $");
}

static int DEBUG = getenv("DBRGS") > 0 ? atoi(getenv("DBRGS")) : 0;

void error(string message)
{
  cerr << "read ** error *** " << message << endl;
}

bool inString(string strg, string str)
{
  int j = strg.find(str,0);
  return (j>-1) && (j<(int)strg.size());
}

// Extract name of a file without extension
string nameonly(string filename)
{
  int i = filename.rfind("/");
  int j = filename.rfind(".");
  if ( j < 0 ) j = filename.size();
  return filename.substr(i+1,j-i-1);
}


string strip(string line)
{
  int l = line.size();
  if ( l == 0 ) return string("");
  int n = 0;
  while (((line[n] == 0)    ||
          (line[n] == ' ' ) ||
          (line[n] == '\n') ||
          (line[n] == '\t')) && n < l) n++;
  
  int m = l-1;
  while (((line[m] == 0)    ||
          (line[m] == ' ')  ||
          (line[m] == '\n') ||
          (line[m] == '\t')) && m > 0) m--;
  return line.substr(n,m-n+1);
}
void bisplit(string str, string& left, string& right, string delim,
             int direction=-1)
{
  left  = str;
  right = "";
  int i = 0;
  if ( direction > 0 )
    i = str.rfind(delim);
  else      
    i = str.find(delim);
  if ( i > 0 )
    {
      left  = str.substr(0, i);
      right = str.substr(i+delim.size(), str.size()-i-delim.size());
    }
}


//----------------------------------------------------------------------------
// Description: Read from a text file. For use in Root.
// Created: 03-May-2005 Harrison B. Prosper
//----------------------------------------------------------------------------

bool slurp_table(string filename,
                 vector<string>& header, 
                 vector<vector<double> >& data,
                 int start,
                 int count,
                 bool extend)
{
  ifstream stream(filename.c_str());
  if ( ! stream.good() )
    { 
      error("slurp_table - unable to open "+filename);
      return false;
    }

  // Read header

  string line;
  getline(stream, line, '\n');
  istringstream inp(line);

  header.clear();
  while ( inp >> line ) header.push_back(line);

  // Skip the first "start" lines

  int n=0;
  for(int i=0; i < start; i++)
    {
      n++;
      if ( !getline(stream, line, '\n') ) break;
    }

  // Read "count" rows if count > 0, otherwise read all lines

  if ( extend )
    {
      int nrow=0;
      while ( getline(stream, line, '\n') )
        {
          istringstream inp2(line);
          for(int i=0; i < (int)header.size(); i++)
            {
              double x;
              inp2 >> x;
              data[nrow].push_back(x);
            }
          nrow++;
          if ( count <= 0 ) continue;
          if ( nrow >= count ) break;
        }
    }
  else
    {
      data.clear();
      data.reserve(100000);
      vector<double> d(header.size());
      int nrow=0;
      while ( getline(stream, line, '\n') )
        {	  
          istringstream inp2(line);      
          for(int i=0; i < (int)header.size(); i++) inp2 >> d[i];
          data.push_back(d);
          nrow++;
          if ( count <= 0 ) continue;
          if ( nrow >= count ) break;
        }
    }
  stream.close();
  return true;
}


//-----------------------------
// CONSTRUCTORS
//-----------------------------

RGS::RGS()
  : _status(0)
{}

RGS::RGS(vstring& filenames, int start, int numrows)
  : _status(0)
{
  // Definitions:
  //  Cut file
  //    n-tuple containing the variables that define the cuts.
  //
  //  Search file
  //    The file on which the cuts are to be applied
  //
  _init(filenames, start, numrows);
}

RGS::RGS(string filename, int start, int numrows)
  : _status(0)
{
  vstring filenames(1, filename);
  _init(filenames, start, numrows);
}

//-----------------------------
// DESTRUCTOR
//-----------------------------
RGS::~RGS(){}


//-----------------------------
// METHODS
//-----------------------------

bool
RGS::good() { return _status == 0; }


void 
RGS::add(string filename, 
         int    start, 
         int    numrows,
         string weightname)
{ 
  _searchname.push_back(nameonly(filename));  
  _searchdata.push_back(vvdouble());
  _weightindex.push_back(-1); // Index to weight field
  _status = 0;

  vector<string> var;
  if ( ! slurp_table(filename, var, _searchdata.back(), start, numrows) )
    {
      cout << "**Error** unable to read file " << filename << endl;
      _status = -1;
      return;
    }

  for(int i = 0; i < (int)var.size(); i++)
    {
      if ( weightname == var[i] )
        {
          _weightindex.back() = i;
          cout << "\tRGS will weight events with the variable "
               << weightname
               << " in column " << i << endl;
          break;
        }
    }
}

void 
RGS::add(vector<string>&   filename, 
         int start, int numrows,
         string   weightname)
{ 
  _searchname.push_back(nameonly(filename[0]));  
  _searchdata.push_back(vvdouble());
  _weightindex.push_back(-1); // Index to weight field
  _status = 0;

  int index=0;
  bool extend=false;
  for(int ifile=0; ifile < (int)filename.size(); ifile++)
    {
      vector<string> var;
      if ( ! slurp_table(filename[ifile], var, _searchdata.back(), 
                         start, numrows, extend) )
        {
          cout << "**Error** unable to read file " << filename[ifile] << endl;
          _status = -1;
          return;
        }
      extend = true;

      for(int i = 0; i < (int)var.size(); i++)
        {
          if ( weightname == var[i] ) _weightindex.back() = index;
          index++;
        }
    }
}

// Read and decode variables file, then call workhorse

void RGS::run(string varfile, // variables file name
              int nprint)
{
  // Cut directions/types
  //    >
  //    <
  //    >|
  //    <|
  //    <>
  //    ==
  //
  // Syntax of variables file:
  //
  // 1. uni-directional cut
  //      variable-name cut-direction
  //
  // 2. bi-directional (box) cut 
  //      variable-name <>
  //
  // 3. ladder cut
  //
  //      \ladder number-of-steps (i.e., cut-points)
  //          variable-name-1 cut-direction-1
  //          variable-name-2 cut-direction-2
  //                   : :
  //      \end

  // cutvar will contain the first field (the variable name)
  // cutdir will contain the remaining fields (either the cut-direction
  // or if cutvar is "\ladder" the the number of steps on the ladder
  //
  // Note: a ladder cut can comprise any combination of uni-directional cuts

  vstring cutvar;
  vstring cutdir;
  
  ifstream fin(varfile.c_str());
  if ( ! fin.good() )
    {
      cout << "*** RGS error *** unable to open file: " << varfile << endl;
      exit(0);
    }

  string line;
  while ( getline(fin, line) )
    {
      // skip blank lines and commented lines

      line = strip(line);
      if ( line == "" ) continue;
      if ( line.substr(0,1) == "/" ) continue;
      if ( line.substr(0,1) == "#" ) continue;
      if ( line.substr(0,1) == "%" ) continue;

      // split line into first field and the rest
      istringstream sin(line);
      string left, right;
      sin >> left;
      if ( left.substr(0,2) == "\e" ) 
        right="";
      else
        sin >> right;

      cutvar.push_back(left);
      cutdir.push_back(right);

      if ( DEBUG > 0 )
        cout << "varname(" << cutvar.back() << ") "
             << "cutdir(" << cutdir.back() << ") "
             << endl;
    }

  // ok, now do the real work
  run(cutvar, cutdir, nprint);
}

// The workhorse

void RGS::run(vstring&  cutvar,        // Variables defining cuts 
              vstring&  cutdir,        // Cut direction
              int nprint)
{
  if ( DEBUG > 0 )
    cout << endl << "RUN" << endl;

  // Make sure len of cutvar and cutdir are the same
  // Note: for ladder cuts, cutvar should contain the keyword 
  // (\ladder and \end) that delimit the ladder cuts.

  if ( cutvar.size() < cutdir.size() )
    {
      cout << "** Error-RGS::run ** Length(cutvar) != Length(cutdir)" << endl;
      exit(1);
    }

  // Indices from the cut variable to its column number 
  // in the data to be scanned.
  _index.clear();

  // The number of cut-points / cut
  _cutpointcount.clear();

  // The indices of cut-points per original cut-point 
  _cutpointindex.clear();

  _cutcode.clear();

  _status = 0;


  // Initial buffers for total number of events and
  // number of (possibly weighted) events passing cuts
  _totals.resize(_searchdata.size(),0.0);
  _counts.resize(_searchdata.size(),vdouble());
  for (int i = 0; i < (int)_counts.size(); i++)
    _counts[i].resize(_cutdata.size(), 0.0);

  // ----------------------------------------------------------
  // Decode cuts
  // ----------------------------------------------------------
  if ( DEBUG > 0 )
    cout << endl << "\tDECODE CUTS" << endl;

  int maxpoints = 1; // maximum number of simultaneous cuts-points to consider

  bool ladderActive = false; // Set to true while decoding a ladder 

  for (unsigned int i = 0; i < cutdir.size(); i++)
    {
      int code  =-1;
      int pointcount = 1; // number of cut-points/cut

      // Check for the start of a ladder cut
      if ( cutvar[i].substr(0, 2) == "\\l" ||
           cutvar[i].substr(0, 2) == "\\L" )
        {
          // Found start of a ladder block
          // Nested ladders are not allowed
          if ( ladderActive )
            {
              cout 
                << "** Error-RGS::run ** Nested ladders are not allowed"
                << endl;
              exit(0);
            }

          ladderActive = true;
          code = LADDER;
          istringstream sin(cutdir[i]);

          // Get number of steps/ladder
          sin >> pointcount;          
          if ( pointcount > maxpoints ) maxpoints = pointcount;
        }
      else if ( cutvar[i].substr(0, 2) == "\\e" ||
                cutvar[i].substr(0, 2) == "\\E" )
        {
          // Found end of a ladder
          ladderActive = false; // deactivate ladder state
          code = END;
        }
      else if ( inString(cutdir[i],"<>") )
        {
          // BOX cuts are not allowed in ladders (in current RGS version)
          if ( ladderActive )
            {
              cout 
                << "** Error-RGS::run ** Box cuts not implemented for ladders" 
                << endl;
              exit(0);
            }
          code = BOX;
          pointcount = 2; // two points needed for a box cut
          if ( pointcount > maxpoints ) maxpoints = pointcount;
        }
      else if ( inString(cutdir[i],">") )
        {
          if   ( inString(cutdir[i],"|") )
            code = ABSGT;
          else
            code = GT;
        }
      else if ( inString(cutdir[i],"<") )
        {
          if   ( inString(cutdir[i],"|") )
            code = ABSLT;
          else
            code = LT;
        }
      else if ( inString(cutdir[i],"=") )
        {
          code = EQ;
        }

      // Save cut code and number of simultaneous cut-points
      // for current cut
      _cutcode.push_back(code);
      _cutpointcount.push_back(pointcount);

      // Get index of current cut variable
      if ( cutvar[i].substr(0, 2) == "\\l" ||
           cutvar[i].substr(0, 2) == "\\L" )
        {
          _index.push_back(0); // Not used; so just set to any valid value
          cout << "begin ladder cut" << endl;
        }
      else if (cutvar[i].substr(0, 2) == "\\e" ||
               cutvar[i].substr(0, 2) == "\\E" )
        {
          _index.push_back(0); // Not used; so just set to any valid value
          cout << "end" << endl;
        }
      else
        {
          // Index is the column number
          if ( _varmap.find(cutvar[i]) != _varmap.end() )
            _index.push_back(_varmap[cutvar[i]]); // index map into data
          else
            {
              cout << "** cut variable " << cutvar[i] << " NOT found" << endl;
              exit(0);
            }
      
          if ( code == BOX ) cout << "box cut" << endl;
          cout << i << "\t" << _index.back() << "\t" 
               << cutvar[i] << "\t" << cutdir[i] 
               << " cut code " << code << endl;
        }
    } 
  if ( DEBUG > 0 )
    cout << "\tEND DECODE CUTS" << endl << endl;


  // Note: in RGS cut-set and cut-point are synonyms

  // -------------------------------------------------------------------------
  // Here we augment each cut-point so that we can handle both box and
  // ladder cuts. For each cut-point, randomly select maxpoints-1 more 
  // cut-points.
  // -------------------------------------------------------------------------

  if ( DEBUG > 1 )
    cout << "\tSAMPLE CUT-POINTS" << endl << endl;

  TRandom3 ran;

  for (int cutpoint = 0; cutpoint < (int)_cutdata.size(); cutpoint++)
    {
      // Reserve space for indices of cut-points
      _cutpointindex.push_back(vector<int>(maxpoints));

      // The first position is the index of the original cut-point
      _cutpointindex[cutpoint][0] = cutpoint;

      // Randomly select maxpoints-1 additional cut-points.
      // We use a set to ensure no "collisions" between cut points,
      // that is, no duplicates.
      set<int> bucket;
      bucket.insert(cutpoint);

      if ( DEBUG > 1 )
        cout << cutpoint << "\t";

      int j = 1;
      for(int i=0; i < 5*maxpoints; i++)
        {
          int jj = ran.Integer(_cutdata.size()-1);
          if ( bucket.find(jj) == bucket.end() )
            {
              // This randomly selected cut-point differs from
              // ones already selected, so add it to the selected set
              _cutpointindex[cutpoint][j] = jj;
              bucket.insert(jj);

              if ( DEBUG > 1 )
                cout << jj << "\t";

              j++;
              if ( j >= maxpoints ) break;
            }
        }
      if ( DEBUG > 1 )
        cout << endl;
    }
 
  if ( DEBUG > 1 )
    cout << "\tEND SAMPLE CUT-POINTS" << endl << endl;

  // -------------------------------------------------------------------------
  // Loop over files to be processed
  // -------------------------------------------------------------------------

  for (int file = 0; file < (int)_searchdata.size(); file++)
    {
      int  weightindex = _weightindex[file];

      // NB: use a reference to avoid expensive copy
      vvdouble& sdata  = _searchdata[file];

      cout << "\tProcessing dataset: " << _searchname[file] 
           << "\twith " << sdata.size() << " rows" << endl;

      if ( nprint > 0 )
        {
          cout << "\tApply cuts: " << endl;
          for (int cut = 0; cut < (int)cutvar.size(); cut++)
            {
              if ( _cutcode[cut] == BOX )
                cout << "\tbox cut" << endl;
              else if ( _cutcode[cut] == LADDER )
                {
                  cout << "\tladder cut" << endl;
                  continue;
                }
              else if ( _cutcode[cut] == END )
                {
                  cout << "\tend" << endl;
                  continue;
                }
              cout << "\t\t" << _var[_index[cut]] 
                   << "\t" << cutdir[cut] << endl;
            }
          cout << endl;
        }

 
      // Check whether to use event weighting
      ///////////////////////////////////////

      bool useEventWeight = weightindex > -1;

      // Loop over cut sets (points)
      //////////////////////////////
      
      int maxcuts = (int)_cutcode.size();

      for (int cutset = 0; cutset < (int)_cutdata.size(); ++cutset)
        {
#ifdef RGSDEBUG
          if ( DEBUG > 2 )
            cout << "\tCUT-POINT " << cutset 
                 << "\t========================== " << endl << endl;
#endif
          if ( (nprint > 0) && (cutset % nprint) == 0 )
            cout << "             " << cutset << endl;
          
          // Loop over rows of file to be processed
          /////////////////////////////////////////
          
          _counts[file][cutset] = 0; // Count per file per cut-point
          
          for (int row = 0; row < (int)sdata.size(); row++)
            {
#ifdef RGSDEBUG
              if ( DEBUG > 2 )
                {
                  cout << "\tROW " << row << endl;
                }
#endif
              // Loop over cut values. 

              // If we are processing a ladder cut then on exit
              // from _laddercut, cut should point to the end of
              // ladder 

              bool passed = true;

              int cut = 0;
              while ( cut < maxcuts )
                {
                  // -------------------------------------
                  // 1. get cut value for simple cut
                  // -------------------------------------
                  // column index in data vector
                  int jcut =_index[cut];

                  // datum value
                  float x  = sdata[row][jcut];

                  // cut value
                  float xcut =_cutdata[cutset][jcut];

#ifdef RGSDEBUG
                  if ( DEBUG > 2 )
                    {
                      switch (_cutcode[cut])
                        {
                        case GT:
                        case LT:
                        case ABSGT:
                        case ABSLT:
                        case EQ:
                          cout << "\t\t\t" 
                               << cutvar[cut] << "\t" << x << " "
                               << cutdir[cut] << " " << xcut << endl;
                          break;
                        default:
                          break;
                        }
                    }
#endif
                  // -------------------------------------
                  // 2. apply cut. If this is a box
                  // of ladder cut, we need to loop over
                  // multiple cut-points. The looping is 
                  // handled by the methods _boxcut and 
                  // _laddercut, respectively.
                  // -------------------------------------
                  switch (_cutcode[cut])
                    {
                    case GT:
                      passed = x > xcut;
                      break;
                      
                    case LT:
                      passed = x < xcut;
                      break;
                      
                    case ABSGT:
                      passed = abs(x) > abs(xcut);
                      break;
                      
                    case ABSLT:
                      passed = abs(x) < abs(xcut);
                      break;

                    case EQ:
                      passed = x == xcut;
                      break;

                    case BOX:
                      // Note: jcut, not cut
                      passed = _boxcut(x, cutset, jcut);
                      break;

                    case LADDER:
                      // Upon exit, cut should point to end of ladder
                      passed = _laddercut(sdata[row], cutset, cut);
                      break;
                      
                    default:
                      break;
                    }
                  
                  // If any cut fails, there is no point continuing.
                  if ( !passed ) break;

                  // IMPORTANT: remember of increment cut number
                  cut++;
                }
          
              // Keep a running sum of events that pass all cuts
              // of current cut-point

              float weight = 1.0;
              if ( useEventWeight ) weight = weight * sdata[row][weightindex];
              
              if ( cutset == 0 ) _totals[file] += weight;
          
              if ( passed ) _counts[file][cutset] += weight;

#ifdef RGSDEBUG
              if ( DEBUG > 2 )
                {
                  if ( passed )
                    cout << "\t\t\t\tPASSED" << endl << endl;
                  else
                    cout << "\t\t\t\tFAILED" << endl << endl;
                }
#endif
            }

        }
    }
}


// Box cut:    xcut_low < c < xcut_high
inline
bool RGS::_boxcut(float x, int cutpoint, int jcut)
{
  // get cut-points
  int cutpoint1 =_cutpointindex[cutpoint][0];
  int cutpoint2 =_cutpointindex[cutpoint][1];

  // get cut-values
  float xcut1   =_cutdata[cutpoint1][jcut];
  float xcut2   =_cutdata[cutpoint2][jcut];

  // get lower bound
  float xcutlow = min(xcut1, xcut2);

  bool passed = x > xcutlow;
  if ( passed ) 
    {
      // ok we've passed the lower bound, so check upper bound
      float xcuthigh = max(xcut1, xcut2);
      passed = x < xcuthigh;
    }
#ifdef RGSDEBUG
  if ( DEBUG > 2 )
    {
      float xcuthigh = max(xcut1, xcut2);
      cout << "\t\t   BOX" << endl;
      cout << "\t\t\t" 
           << _var[jcut] << "\t" << x << " > " << xcutlow << endl;
      cout << "\t\t\t" 
           << _var[jcut] << "\t" << x << " < " << xcuthigh << endl;
    }
#endif
  return passed;
}

// Ladder cut:    OR of cuts
// Loop over cut-points for current cut
// IMPORTANT: on exit, the cut number should point to the 
// end of the ladder indicated by the cutcode END.

inline
bool RGS::_laddercut(vdouble& datarow, int origcutpoint, int& cut)
{
#ifdef RGSDEBUG
  if ( DEBUG > 2 )
    {
      cout << "\t\t   LADDER" << endl;
    }
#endif

  // Loop over cut-points of ladder cut
  // ladderpassed will be true if at least one cut-point returns true

  bool ladderpassed = false;

  // Recall: _cutcode contains all cut codes including the codes
  // for keywords \ladder and \end

  int maxcuts = (int)_cutcode.size();

  // Get number of cut points to use for current ladder
  int pointcount = _cutpointcount[cut];

#ifdef RGSDEBUG
  if ( DEBUG > 2 )
    {
      cout << "\t\t     cut-points = " << pointcount << endl;
    }
#endif

  // Remember first cut of ladder
  int firstcut = cut;
  firstcut++;

  for(int ii=0; ii < pointcount; ++ii)
    {
#ifdef RGSDEBUG
      if ( DEBUG > 2 )
        {
          cout << "\t\t       point " << ii << endl;
        }
#endif
      // get cut-point associated with the original cut-point
      int cutpoint = _cutpointindex[origcutpoint][ii];

      bool endOfladder = false;
      bool passed = true;

      // loop over cuts, starting each time at first cut of ladder
      
      cut = firstcut;
      while ( cut < maxcuts )
        {
          // column index in data vector
          int jcut =_index[cut];

          // datum value
          float x  = datarow[jcut];

          // cut value
          float xcut =_cutdata[cutpoint][jcut];

#ifdef RGSDEBUG
          if ( DEBUG > 2 )
            {
              switch (_cutcode[cut])
                {
                case GT:
                  cout << "\t\t\t" 
                       << _var[jcut] << "\t" << x << " > " << xcut << endl;
                  break;
                case LT:
                  cout << "\t\t\t" 
                       << _var[jcut] << "\t" << x << " < " << xcut << endl;
                  break;
                case ABSGT:
                  cout << "\t\t\t" 
                       << _var[jcut] << "\t" << x << " >| " << xcut << endl;
                  break;
                case ABSLT:
                  cout << "\t\t\t" 
                       << _var[jcut] << "\t" << x << " <| " << xcut << endl;
                  break;
                case EQ:
                  cout << "\t\t\t" 
                       << _var[jcut] << "\t" << x << " == " << xcut << endl;
                  break;
                default:
                  break;
                }
            }
#endif
          switch (_cutcode[cut])
            {
            case GT:
              passed = x > xcut;
              break;
              
            case LT:
              passed = x < xcut;
              break;
              
            case ABSGT:
              passed = abs(x) > abs(xcut);
              break;
              
            case ABSLT:
              passed = abs(x) < abs(xcut);
              break;
              
            case EQ:
              passed = x == xcut;
              break;
              
            case END:
              endOfladder = true;
              break;

            default:
              break;
            }

          // If any cut fails, there is no point continuing to next cut
          if ( ! passed ) break;

          // break out of loop over cuts if we have reached end of ladder
          if ( endOfladder ) break;

          // IMPORTANT: Remember to increment cut number
          cut++;
        }

      // Make sure we are at the end of the ladder
      if ( ! endOfladder )
        {
          cut++;
          while ( cut < maxcuts )
            {
              if ( _cutcode[cut] == END ) break;
              cut++;
            }
        }
#ifdef RGSDEBUG
      if ( DEBUG > 2 )
        {
          if ( passed )
            cout << "\t\t\t\t\tpassed " << endl;
          else
            cout << "\t\t\t\t\tfailed " << endl;
        }
#endif
      // Take OR of cuts over cut-points
      ladderpassed = ladderpassed || passed;

      // If a cut succeeds, there is no need to continue
      if ( ladderpassed ) break;
    }

#ifdef RGSDEBUG
  if ( DEBUG > 2 )
    {
      cout << "\t\t   END" << endl;
    }
#endif
  return ladderpassed;
}


TFile*
RGS::save(string filename, double lumi)
{

  cout << endl << "Root file name: " << filename << endl;

  TFile* file = new TFile(filename.c_str(), "recreate"); 
  TTree* tree = new TTree("RGS", "RGS");

  // Declare a buffer of size, maxpoints x maxcuts, for writing out cut values
  int maxpoints = (int)_cutpointindex[0].size();
  int maxcuts   = (int)_cutcode.size();

  vector<vector<float> > cutvalue(maxcuts, vector<float>(maxpoints,0));

  // Note: _counts.size() = number of scanned files
  vector<float> count2(_counts.size());


  // Create branches for each cut. For box and ladder cuts, use fixed
  // length arrays of the appropriate size
  int cut = 0;
  while ( cut < maxcuts )
    {
      int jcut =_index[cut];
      string var = _var[jcut];
      char fmt[40];

      switch (_cutcode[cut])
        {
        case GT:
        case LT:
        case ABSGT:
        case ABSLT:
        case EQ:
          {
            sprintf(fmt, "%s/F", var.c_str());
            tree->Branch(var.c_str(), &cutvalue[cut][0], fmt);
            cout << "\t" << fmt << endl;
          }
          break;

        case BOX:
          {
            sprintf(fmt, "%s[2]/F", var.c_str());
            tree->Branch(var.c_str(), &cutvalue[cut][0], fmt);
            cout << "\t" << fmt << endl;
          }
        case LADDER:
          {

            // Get number of cut points to use for current ladder
            int pointcount = _cutpointcount[cut];

            cut++;
            while ( cut < maxcuts )
              {
                if ( _cutcode[cut] == END ) break;

                jcut =_index[cut];
                var  =_var[jcut];
                sprintf(fmt, "%s[%d]/F", var.c_str(), pointcount);
                tree->Branch(var.c_str(), &cutvalue[cut][0], fmt);
                cout << "\t" << fmt << endl;

                // IMPORTANT: Remember to increment cut number
                cut++;
              }
          }
          break;
                      
        default:
          break;
        }

      // IMPORTANT: increment loop counter
      cut++;
    }

  // Add one count variable for each file scanned
  for(unsigned int i=0; i < count2.size(); i++)
    {
      char fmt[40];
      char name[40];
      sprintf(name, "count%d", i);
      sprintf(fmt, "count%d/F", i);
      tree->Branch(name, &count2[i], fmt);
      cout << "\t" << fmt << endl;
    }

  // Now fill ntuple

  for (unsigned int cutpoint=0; cutpoint < _cutdata.size(); cutpoint++)
    {
      cut = 0;
      while ( cut < maxcuts )
        {
          int jcut =_index[cut];
          
          switch (_cutcode[cut])
            {
            case GT:
            case LT:
            case ABSGT:
            case ABSLT:
            case EQ:
              cutvalue[cut][0] = _cutdata[cutpoint][jcut];
              break;

            case BOX:
              {
                // Get the two cut-points
                int cutpoint1  =_cutpointindex[cutpoint][0];
                int cutpoint2  =_cutpointindex[cutpoint][1];
                // Get the cut values
                float xcut1 =_cutdata[cutpoint1][jcut];
                float xcut2 =_cutdata[cutpoint2][jcut];
                float xcutlow  = min(xcut1, xcut2);
                float xcuthigh = max(xcut1, xcut2);
                // Store in output buffers
                cutvalue[cut][0] = xcutlow;
                cutvalue[cut][1] = xcuthigh;
              }
              break;

            case LADDER:
              {
                // Get number of cut points to use for current ladder
                int pointcount = _cutpointcount[cut];

                // Remember first cut number
                int firstcut = cut;
                firstcut++;

                for(int i=0; i < pointcount; i++)
                  {
                    // Get cut-point index
                    int cutpoint_i  =_cutpointindex[cutpoint][i];
                    cut = firstcut;
                    while ( cut < maxcuts )
                      {
                        jcut =_index[cut];
          
                        // Get the cut value
                        float xcut =_cutdata[cutpoint_i][jcut];
                        // Store in output buffer
                        cutvalue[cut][i] = xcut;
                        cut++;
                      }
                  }
              }
              break;
              
            default:
              break;              
            }
          cut++;
        }

      // store counts
      for(unsigned int i=0; i < count2.size(); i++)
        count2[i] = _counts[i][cutpoint] * lumi;
      
      file->cd();
      tree->Fill();
    }
  file->cd();
  tree->AutoSave("SaveSelf");
  //file->Write("", TObject::kOverwrite);
  return file;
}

double
RGS::total(int index) 
{
  if ( index < 0 || index > (int)_totals.size()-1 )
    {
      _status = rBADINDEX;
      return 0;
    }
  return _totals[index];
}

double
RGS::count(int index, int cutindex) 
{
  if ( index < 0 || index > (int)_totals.size()-1 )
    {
      _status = rBADINDEX;
      return 0;
    }
  if ( cutindex < 0 || cutindex > (int)_cutdata.size()-1 )
    {
      _status = rBADINDEX;
      return 0;
    }
  return _counts[index][cutindex];
}

vdouble
RGS::cuts(int index)
{ 
  if ( index < 0 || index > (int)_cutdata.size()-1 )
    {
      _status = rBADINDEX;
      return vdNULL;
    }
  vdouble cut(_cutcode.size());
  for(unsigned i=0; i < _cutcode.size(); i++)
    cut[i] = _cutdata[index][_index[i]];
  return cut;
}

vstring
RGS::cutvars()   
{ 
  vstring cutvar(_cutcode.size());
  for(unsigned i=0; i < _cutcode.size(); i++)
    cutvar[i] = _var[_index[i]];
  return cutvar;
}

int
RGS::ncuts() { return _cutdata.size(); }

vstring&
RGS::vars()   { return _var; }

int
RGS::ndata(int index)
{ 
  if ( index < 0 || index > (int)_searchdata.size()-1 )
    {
      _status = rBADINDEX;
      return 0;
    }
  return _searchdata[index].size();
}

vdouble&
RGS::data(int index, int event)
{ 
  if ( index < 0 || index > (int)_searchdata.size()-1 )
    {
      _status = rBADINDEX;
      return vdNULL;
    }
  return _searchdata[index][event]; 
}

//--------------------------------
// PRIVATE METHODS
//--------------------------------

void
RGS::_init(vstring& filenames, int start, int numrows)
{
  _cutdata.clear();
  _cutcode.clear();
  _searchname.clear();
  _searchdata.clear();
  _weightindex.clear();
  _varmap.clear();
  _totals.clear();
  _counts.clear();

  _status = rSUCCESS;

  int index=0;
  bool extend=false;
  for(int ifile=0; ifile < (int)filenames.size(); ifile++)
    {
      vector<string> var;
      if ( ! slurp_table(filenames[ifile], 
                         var, _cutdata, start, numrows, extend) )
        {
          cout << "**Error** unable to read file " << filenames[ifile] << endl;
          _status = rFAILURE;
          return;
        }
      extend = true;
      
      for(int i = 0; i < (int)var.size(); i++)
        {
          _varmap[var[i]] = index; // Map variable names to ordinal value
          _var.push_back(var[i]);  // Map ordinal value to variable name
          index++;
        }
    }
}
