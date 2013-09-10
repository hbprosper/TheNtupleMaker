#ifndef RGS_H
#define RGS_H
//////////////////////////////////////////////////////////////////////////////
// File:    RGS.h
// Purpose: Declaration of RGS classes
// Created: 18-Aug-2000 Harrison B. Prosper, Chandigarh, India
//$Revision: 1.3 $
//////////////////////////////////////////////////////////////////////////////
#ifdef __WITH_CINT__
#include "TObject.h"
#endif

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include "TFile.h"

typedef std::vector<int>     vint;
typedef std::vector<vint>    vvint;
typedef std::vector<float>   vfloat;
typedef std::vector<vfloat>  vvfloat;
typedef std::vector<double>  vdouble;
typedef std::vector<vdouble> vvdouble;
typedef std::vector<std::string>  vstring;


// ERROR CODES

const int rSUCCESS = 0;
const int rFAILURE =-1;
const int rBADINDEX=-2;
const int rEOF     =-6;
const int rPYTHONERROR =-7;

enum CUTCODE {GT, LT, ABSGT, ABSLT, EQ, BOX, LADDER, END};

typedef std::map< std::string, int >  varmap;

static vdouble  vdNULL;


bool slurp_table(std::string filename,
                 std::vector<std::string>& header, 
                 std::vector<std::vector<double> >& data,
                 int start=0,
                 int count=0,
                 bool extend=false);

std::string rgsversion();
///
class RGS
{
public:
  ///
  RGS();

  ///
  RGS(std::string cutdatafilename, int start=0, int numrows=0);

  ///
  RGS(std::vector<std::string>& cutdatafilename, int start=0, int numrows=0);

  virtual ~RGS();

  /// If true, all is well.
  bool  good();

  /// Add a data file.
  void  add(std::string datafilename,
            int start=0, 
            int numrows=0,       // Read all rows  
            std::string weightname="Weight");
  
  /// Add one or more data files.
  void  add(std::vector<std::string>& datafilename,
            int start=0, 
            int numrows=0,
            std::string weightname="Weight");

  /// Run the RGS algorithm for specified cut variables and cut directions.
  void  run(vstring&  cutvar,  // Variables defining cuts 
            vstring&  cutdir,  // Cut direction (cut-type)
            int nprint=500);

  /// Run the RGS algorithm for specified cut variables and cut directions.
  void  run(std::string  varfile,  // file name of Variables file
            int nprint=500);

  /// Return the total count for the data file identified by dataindex.
  double    total(int dataindex);

  /// Return the count for the given data file and the given cut-point.
  double    count(int dataindex, int cutindex);

  /// Return all variables read from the cut file(s).
  vstring&  vars();

  /// Return number of cuts
  int       ncuts();

  /// Return cut values for cut-point identified by cutindex.
  vdouble   cuts(int cutindex);

  /// Return cut variable names
  vstring   cutvars();

  /// Return number of events for specified data file.
  int       ndata(int dataindex);

  /// Return values for data given data file and event.
  vdouble&  data(int dataindex, int event);

  /// Save counts to a root file
  TFile*    save(std::string filename, double lumi=1);
  
private:
  int         _status;

  vvdouble    _cutdata;
  varmap      _varmap;
  vstring     _var;
  vint        _cutcode;
  vint        _cutpointcount;

  vstring                   _searchname;
  std::vector< vvdouble >   _searchdata;
  vint                      _weightindex;
  vint        _index;
  vdouble     _totals;
  vvdouble    _counts;

  std::vector<std::vector<int> > _cutpointindex;

  void _init(vstring& filename, int start=0, int numrows=0);
  bool _boxcut(float x, int cutpoint, int jcut);
  bool _laddercut(vdouble& datarow, int cutpoint, int& cut);

#ifdef __WITH_CINT__
  public:
ClassDef(RGS,1)
#endif    

};

#endif
