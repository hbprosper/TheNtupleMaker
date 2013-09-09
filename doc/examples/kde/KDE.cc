//----------------------------------------------------------------------------
//  File:    KDE.cc
//  Purpose: Estimate a multivariate density using Kernel Density Estimation.
// 
//           Method:
//              Use a Gaussian product kernel with the bandwidth in each
//              dimension set to
//
//              h = sigma * [4/(D+2)N]^[1/(D+4)]
//
//              where D is the dimensionality of the data. This bandwidth is
//              appropriate for a multivariate Gaussian density. For 
//              non-Gaussian densities this (default) bandwidth choice may 
//              be quite poor. However, the choice can be improved by calling
//              the optimize method, which uses Minuit to search for better
//              values for the bandwidths.
//
//           Interesting article:
//              Zhang, King and Hyndman, 2004,
//              Bandwidth Selection for Multivariate Kernel 
//              Density Estimation Using MCMC, available via google.
//
//  Created: 19-Aug-2007 Harrison B. Prosper - based on old Pde code from
//                       way back when!
//  Updated: 02-Nov-2009 HBP - Improve optimize() method
//                       Panjab University, Chandigarh, India
//$Revision: 1.2 $ 
//----------------------------------------------------------------------------
#ifdef PROJECT_NAME
#include "PhysicsTools/TheNtupleMaker/interface/KDE.h"
#else
#include "KDE.h"
#endif

#ifdef __WITH_CINT__
ClassImp(KDE)
#endif

#include <cmath>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <TMatrixD.h>
#include <TTimeStamp.h>
#include <TMath.h>
#include <TMinuit.h>
#include "Math/Random.h"
#include "Math/GSLRndmEngines.h"
//-----------------------------------------------------------------------------
using namespace std;

// Hide error and a few other utilities within an anonymous namespace
// so that they are visible only within this programming unit
namespace {
  const double ROOT2PI = sqrt(TMath::TwoPi());
  void error(string message)
  {
    cerr << "** error *** " << message << endl;
    exit(0);
  }

  //---------------------------------------------------------------------------
  // Function to be minimized (using Minuit) to find
  // improved estimates of the bandwidth.
  //
  // npar  is the number of parameters (bandwidths)
  // fval  will, upon exit, contain the current value of fcn
  // xval  contains, upon entry, the current values of the parameters
  //---------------------------------------------------------------------------

  static KDE* kdeptr=0;
  static int  NCALL=0;

  void fcn(int&    npar, 
           double* grad, 
           double& fval,
           double* xval,
           int     iflag)
  {
    for(int i=0; i < npar; i++)
      {
         // Check for crazy value!
        if ( xval[i] != xval[i] )
          { 
            cout << "*** ERROR fcn: xval[" << i << "] is a nan" << endl;
            exit(0);
          } 
      }

    // Pass current bandwidth parameters to KDE object
    // and remember to normalize the density

    vector<double> bandwidth(npar);
    copy(xval, xval+npar, bandwidth.begin());

    kdeptr->setbandwidth(bandwidth);
    kdeptr->normalize();

//     double lprior = kdeptr->logprior();

//     // Check for crazy value!
//     if ( lprior != lprior )
//       { 
//         cout << "*** ERROR fcn: lprior = " << lprior << endl;
//         exit(0);
//       }

//     double llike  = kdeptr->loglikelihood();

//     // Check for crazy value!
//     if ( llike != llike )
//       { 
//         cout << "*** ERROR fcn: llike = " << llike << endl;
//         exit(0);
//       }

//     double lpost = llike + lprior;
//     fval = -lpost;

    fval = kdeptr->chisq();

    double z=0;
    for(int i=0; i < npar; i++) z *= xval[i];
    if ( z > 0 ) fval += 1.0/(kdeptr->lambda() + pow(abs(z), 1.0/npar));
    
    // Check for crazy value!
    if ( fval != fval )
      { 
        cout << "*** ERROR fcn: fval = " << fval << endl;
        exit(0);
      }

    // Let user know fcn is alive

    NCALL++;
    if ( NCALL % 10 == 0 ) 
      cout << NCALL << "\tfval = " << fval << endl; 

    // Stupid code to avoid compiler warnings!
    int dumb = iflag; dumb = 0;
    double* evendumber = grad; evendumber = 0;
    dumb = dumb * (*evendumber);
  }

  // Split a string into a vector thereof

  vector<string> 
  split(string str)
  {
    vector<string> vstr;
    istringstream stream(str);
    while ( stream )
      {
        string str;
        stream >> str;
        if ( stream ) vstr.push_back(str);
      }
    return vstr;
  }
}

//-----------------------------------------------------------------------------
// Default constructor
//-----------------------------------------------------------------------------
KDE::KDE()
  : _vars(std::vector<std::string>()),
    _partition(0.9),
    _min(std::vector<double>()),
    _max(std::vector<double>()),
    _vmap(std::map<std::string, int>()),
    _data(std::vector<std::vector<double> >()),
    _D(0),
    _status(0),
    _norm(1),
    _initialized(false),
    _lambda(1.e-4),
    _ndata(0)
{}

//-----------------------------------------------------------------------------
// Regular constructor
//-----------------------------------------------------------------------------
KDE::KDE(std::string vars, double partition)
  : _vars(split(vars)),
    _partition(partition),
    _min(std::vector<double>(_vars.size(),1.e30)),
    _max(std::vector<double>(_vars.size(),-1.e30)),
    _vmap(std::map<std::string, int>()),
    _data(std::vector<std::vector<double> >()),
    _D(_vars.size()),
    _status(0),
    _norm(1.e-4),
    _initialized(false),
    _lambda(1),
    _ndata(0)
{
  cout << "KDE v2.0" << endl;
  cout << "variables" << endl;
  for(unsigned i=0; i < _vars.size(); i++)
    cout << "\t" << i << "\t" << _vars[i] << endl;
}

KDE::~KDE()
{}

bool
KDE::good() { return _status == 0; }

//-----------------------------------------------------------------------------
// Add a point to KDE
//-----------------------------------------------------------------------------
void
KDE::add(vector<double>& d)
{
  if ( _vars.size() == 0 ) 
    error("First call setvars(..) to set the variable names");

  if ( d.size() != _vars.size() )
    error("size of vector does not match number of variables");

  _data.push_back(d);

  for(unsigned i=0; i < _min.size(); i++)
    {
      if ( d[i] < _min[i] ) _min[i] = d[i];
      if ( d[i] > _max[i] ) _max[i] = d[i];
    }
}

//-----------------------------------------------------------------------------
// Compute n-D density projected onto specified axis
// We use a function object so that the KDE can
// be called like a function:
//
//    double y = kde(x, "A")
//-----------------------------------------------------------------------------
double
KDE::operator()(double x, string var)
{
  if ( !_initialized ) _init();

  if ( _vmap.find(var) == _vmap.end() )
    error("variable " + var + " not found");

  int dimension = _vmap[var];

  // Integrate over n-1 dimensions
  double p  = 0;
  for(int i=0; i < _ndata; i++)
    {
      double q = 1;
      for(int j=0; j < _D; j++)
        {
          if ( j == dimension )
            {
              double z = (x - _data[i][j])/_bandwidth[j];
              q *= exp(-0.5*z*z)/(_bandwidth[j] * ROOT2PI);
            }
          else
            {
              double za = (_min[j] - _data[i][j])/_bandwidth[j];
              double zb = (_max[j] - _data[i][j])/_bandwidth[j];
              q *= (TMath::Freq(zb) - TMath::Freq(za));
            }
        }      
      p += q;
    }
  p /= _ndata;
  p /= _norm;
  return p;
}

//-----------------------------------------------------------------------------
// Compute density at specified n-D point
//-----------------------------------------------------------------------------
double
KDE::operator()(vector<double>& x)
{
  if ( !_initialized ) _init();

  // Compute density at point x

  double p  = 0;
  for(int i=0; i < _ndata; i++)
    {
      double zz = 0;
      for(int j=0; j < _D; j++)
        {
          double y = (x[j] - _data[i][j])/_bandwidth[j];
          zz += y * y;
        }      
      p += exp(-0.5*zz);
    }
  double a = 1;
  for(int i=0; i < _D; i++) a *= _bandwidth[i] * ROOT2PI;
  a *= _norm * _ndata;
  return p/a;
}

//-----------------------------------------------------------------------------
// Aliases for operator()
//-----------------------------------------------------------------------------
double
KDE::density(double x, string var) { return (*this)(x, var); }

double
KDE::density(vector<double>& x) 
{ return (*this)(x); }

void
KDE::printbw()
{
  if ( !_initialized ) _init();

  for (int i=0; i < _D; i++)
    {
      cout << "\tbandwidth for " << _vars[i] << "\t---> "
           << _bandwidth[i] << endl;
    }
  cout << endl;
}

//-----------------------------------------------------------------------------
// Compute integral of n-D density over a hyper-rectangle
//-----------------------------------------------------------------------------
double
KDE::integral(vector<double>& a, vector<double>& b)
{
  if ( !_initialized ) _init();

  _status = 0;

  // Integrate over all N dimensions

  double p  = 0;
  for(int i=0; i < _ndata; i++)
    {
      double q = 1;
      for(int j=0; j < _D; j++)
        {
          double za = (a[j] - _data[i][j])/_bandwidth[j];
          double zb = (b[j] - _data[i][j])/_bandwidth[j];
          q *= (TMath::Freq(zb) - TMath::Freq(za));
        }
      p += q;
    }
  p /= _ndata;
  p /= _norm;
  return p;
}

//-----------------------------------------------------------------------------
// Normalize density to unity over the current domain whose bounds are
// specified by _min and _max
//-----------------------------------------------------------------------------
double
KDE::normalize()
{
  if ( !_initialized ) _init();
  _norm = 1.0; 
  double result = integral(_min, _max);
  if ( good() ) _norm = result;
  return _norm;
}

//-----------------------------------------------------------------------------
// Compute loglikelihood, log prior
//-----------------------------------------------------------------------------
double
KDE::logprior()
{
  double y = 0; 
  for(int i=0; i < _D; i++) y += _bandwidth[i] * _bandwidth[i];
  if ( y > 0 )
    return -_lambda * log(y);
  else
    return 0;
}

double
KDE::loglikelihood()
{
  double logl = 0; 
  for(unsigned i=_ndata; i < _data.size(); i++)
    {
      double y = density(_data[i]);
      logl += log(y);
    }
  return logl;
}

double
KDE::chisq()
{
  int ND = 0;
  double chi2 = 0;
  for(int i=0; i < _D; i++)
    {
      int nbins    = _hw[i].GetNbinsX(); 
      double xmin  = _hw[i].GetBinLowEdge(1);
      double width = _hw[i].GetBinWidth(1);
      double norm  = _hw[i].Integral()*width;
      string name(_hw[i].GetName());

      for(int bin=1; bin <= nbins; bin++)
        {
          double d = _hw[i].GetBinContent(bin);
          if ( d > 0 )
            {
              double x = xmin + (bin-0.5)*width;
              double f = norm * (*this)(x, name);
              double z = (f - d);
              if ( d > 0 )
                {
                  ND++;
                  chi2 += z*z / d;
                }
            }
        }
    }
  ND -= _D;
  if ( ND > 0 ) 
    return chi2/ND;
  else
    return 0;
}

int
KDE::optimize(double scale, int maxiter)
{
  if ( !_initialized ) _init();

  // Print initial parameters
  vector<double> dbandw(_D);
  cout << endl;
  for (int i=0; i < _D; i++)
    {
      dbandw[i] = _bandwidth[i];
      cout << "\tdefault bandwidth for " << _vars[i] << "\t---> "
           << _bandwidth[i] << endl;
    }
  cout << endl;

  // Make these are initialized

  NCALL = 0;
  kdeptr = this;
  
  // Parameters:
  // we have _D bandwidths
  
  TMinuit minuit(_D);
  minuit.SetFCN(fcn);
  
  // Set parameters
  int ierflag;
  for (int i=0; i < _D; i++)
    {
      double minbw = 0.0;
      double maxbw = _bandwidth[i]*scale;
      double step  = (maxbw-minbw)/(10*scale);
      minuit.mnparm(i, 
                    _vars[i].c_str(), 
                    _bandwidth[i], step, minbw, maxbw, ierflag); 
    }

  minuit.SetErrorDef(0.5);
  minuit.SetMaxIterations(maxiter);

  // Let's go!
  minuit.Migrad();

  // Check for convergence

  double fmin, fedm, errdef;
  int nvpar, nparx, status;
  minuit.mnstat(fmin, fedm, errdef, nvpar, nparx, status);
  if ( status == 3 )
    {
      cout << endl
           << "KDE optimization succeeded"
           << endl;

      // Get parameters
      for (int i=0; i < _D; i++)
        {
          double err;
          minuit.GetParameter(i, _bandwidth[i], err);
          cout << "\toptimized bandwidth for " << _vars[i]
               << "\t"
               << dbandw[i] 
               << "\t---> "
               << _bandwidth[i]
               << endl;
        }

      cout << endl;

      // Remember to normalize density within current domain
      // (same as that of the input 1-D histograms)
      normalize();
      return 0;
    }
  return -1;
}


//-----------------------------------------------------------------------------
// Write self-contained C++ function in a readable format
//-----------------------------------------------------------------------------
void 
KDE::write(string name)
{
  if ( !_initialized ) _init();

  int i = name.rfind(".");
  if ( i >= 0 ) name = name.substr(0,i);

  string filename = name + ".cpp";
  ofstream out(filename.c_str());
  char record[20*1024];

  out << 
    "//-----------------------------------------------------------------------------\n";
  out << "// File: " << filename << endl;
  out << "// Description: Kernel Density Estimate\n";
  out << "// \tVariables\n";
  for(unsigned i=0; i < _vars.size(); i++)
    out << "// \t" << i << "\t""" << _vars[i] << """\n";
  out << "//\n";

  TTimeStamp ts;
  string ctime(ts.AsString());
  ctime = ctime.substr(0,16);
  out << "// Created: " << ctime << " by KDE v2.0 Harrison B. Prosper\n";
  out << 
    "//-----------------------------------------------------------------------------\n";
  out << "#include <cmath>" << endl;
  out << "#include <vector>" << endl;
  out << "#include <map>" << endl;
  out << "#include <string>" << endl;
  out << "#include <algorithm>" << endl;
  out << "#include \"TMath.h\"" << endl;
  out << 
    "//-----------------------------------------------------------------------------\n";
  out << "using namespace std;" << endl << endl;

  out << "namespace {" << endl;
  out << "  // Number of dimensions" << endl;
  out << "  const int ndim=" << _D << ";" << endl << endl;

  out << "  // Number of points" << endl;
  out << "  const int ndata=" << _ndata << ";" << endl << endl;

  // Write out variables

  out << "  // Variables" << endl;
  string delim="\t ";
  out << "  const char* v[ndim]={" << endl;
  for (int i=0; i < _D; i++)
    {
      sprintf(record, "%s\"%s\"", delim.c_str(), _vars[i].c_str());
      out << "  " << record << endl;
      delim = "\t,";
    }
  out << "  };" << endl << endl;

  // Write out bandwidths

  out << "  // Bandwidths" << endl;
  delim="\t ";
  out << "  const double h[ndim]={" << endl;
  for (int i=0; i < _D; i++)
    {
      sprintf(record, "%s%10.5e", delim.c_str(), _bandwidth[i]);
      out << "  " << record << endl;
      delim = "\t,";
    }
  out << "  };" << endl << endl;


  // Write out bandwidths**2

  delim="\t ";
  out << "  const double hh[ndim]={" << endl;
  for (int i=0; i < _D; i++)
    {
      out << "  " << delim << "0.5/(h[" << i << "]*h[" << i << "])" << endl;
      delim = "\t,";
    }
  out << "  };" << endl << endl;

  // Write out normalization

  out << "  // Normalization" << endl;
  out << "  const double norm=" << _norm << ";" << endl;
  out << "  const double root2pi=sqrt(2*3.1415926536);" << endl;
  out << "  const double NORM=norm*ndata" << endl;

  delim="\t*";
  for (int i=0; i < _D; i++)
    {
      if (i < (_D-1))
        out << "  " << delim << "h[" << i << "]*root2pi"  << endl;
      else
        out << "  " << delim << "h[" << i << "]*root2pi;" << endl << endl;
    }

  // Write out data limits

  out << "  // Lower limits of data points" << endl;
  delim="\t ";
  out << "  const double lower[ndim]={" << endl;
  for (int i=0; i < _D; i++)
    {
      sprintf(record, "%s%10.5e", delim.c_str(), _min[i]);
      out << "  " << record << endl;
      delim = "\t,";
    }
  out << "  };" << endl << endl;

  out << "  // Upper limits of data points" << endl;
  delim="\t ";
  out << "  const double upper[ndim]={" << endl;
  for (int i=0; i < _D; i++)
    {
      sprintf(record, "%s%10.5e", delim.c_str(), _max[i]);
      out << "  " << record << endl;
      delim = "\t,";
    }
  out << "  };" << endl << endl;

  // Write out points
    
  out << "  // Points" << endl;
  out << "  const double data[ndata][ndim]={" << endl;

  for (int i=0; i < _ndata; i++)
    {
      delim="\t{";
      for(int j=0; j < _D; j++)
        {
          sprintf(record, "%s%10.5e", delim.c_str(), _data[i][j]); 
          out << record;
          delim = "\t,";
          if ( j != (_D-1) && (j+1) % 4 == 0 ) out << endl;
        }
      out << "}";
      if ( i < (_ndata-1) ) out << ",";
      out << endl;
    }
  out << "  };" << endl << endl;
    
  out << "  vector<double> z(ndim, 0);" << endl;
  out << "};" << endl << endl;

  // Write class and methods

  out << 
    "//--------------------------------------------------------------------\n";

  out << "class " << name << endl;
  out << "{"  << endl;
  out << "public:" << endl;
  out << "  " << name << "();" << endl;
  out << "  ~" << name << "() {}" << endl << endl;
  out << "  double operator()(std::vector<double>& x);" << endl;
  out << "  double operator()(double x, std::string v);" << endl;
  out << "  double density(std::vector<double>& x);" << endl;
  out << "  double density(double x, std::string v);" << endl;
  out << "  double integral(std::vector<double>& a, std::vector<double>& b);"
      << endl;
  out << "  std::vector<double> bandwidth();" << endl << endl;
  out << "private:" << endl;
  out << "  std::map<std::string, int> _vmap;" << endl;
  out << "};" << endl << endl;

  out << name << "::" << name << "()" << endl;
  out << "  : _vmap(std::map<std::string, int>())" << endl;
  out << "{" << endl;
  out << "  for(int i=0; i < ndim; i++) _vmap[v[i]] = i;" << endl;
  out << "}" << endl << endl;

  sprintf(record, "double %s::operator()(vector<double>& x)", name.c_str());
  out << record << endl;

  sprintf(record,
	  "{\n"
	  "  if ( x.size() != (unsigned)ndim ) return -1;\n"
	  "  double p = 0;\n"
	  "  for(int i=0; i < ndata; i++)\n"
	  "    {\n"
	  "      double zz = 0;\n"
	  "      for(int j=0; j < ndim; j++)\n"
	  "        {\n"
	  "          double y = x[j] - data[i][j];\n"
	  "          zz += hh[j] * y * y;\n"
	  "        }\n"      
	  "      p += exp(-zz);\n"
	  "    }\n"
	  "  return p / NORM;\n"
	  "}\n");
  out << record; 

  out << 
    "//--------------------------------------------------------------------\n";

  sprintf(record, "double %s::operator()(double x, std::string var)", 
	  name.c_str());
  out << record << endl;

  sprintf(record,
	  "{\n"
	  "  if ( _vmap.find(var) == _vmap.end() ) return -1;\n"
	  "  int dimension = _vmap[var];\n"
	  "  if ( (dimension < 0) || (dimension > ndim-1) ) return -1;\n"
	  "  double p = 0;\n"
	  "  for(int i=0; i < ndata; i++)\n"
	  "    {\n"
	  "      double q = 1;\n"
	  "      for(int j=0; j < ndim; j++)\n"
	  "        {\n"
	  "          if ( j == dimension )\n"
	  "            {\n"
	  "              double z = (x - data[i][j])/h[j];\n"
	  "              q *= exp(-0.5*z*z) / (h[j]*root2pi);\n"
	  "            }\n"
	  "          else\n"
	  "            {\n"
	  "              double za = (lower[j]-data[i][j])/h[j];\n"
	  "              double zb = (upper[j]-data[i][j])/h[j];\n"
	  "              q *= TMath::Freq(zb)-TMath::Freq(za);\n"
	  "            }\n"
	  "        }\n"      
	  "      p += q;\n"
	  "    }\n"
	  "  return p / (norm * ndata);\n"
	  "}\n");
  out << record; 


  sprintf(record, 
	  "double %s::density(vector<double>& x) {return (*this)(x);}", 
	  name.c_str());
  out << record << endl;


  sprintf(record, 
	  "double %s::density(double x, std::string var)"
	  "{return (*this)(x, var);}", 
	  name.c_str());
  out << record << endl;

  out << 
    "//--------------------------------------------------------------------\n";

  sprintf(record, 
	  "double %s::integral(vector<double>& a, vector<double>& b)", 
	  name.c_str());
  out << record << endl;

  sprintf(record,
	  "{\n"
	  "  double p = 0;\n"
	  "  for(int i=0; i < ndata; i++)\n"
	  "    {\n"
	  "      double q = 1;\n"
	  "      for(int j=0; j < ndim; j++)\n"
	  "        {\n"
	  "          double za = (a[j]-data[i][j])/h[j];\n"
	  "          double zb = (b[j]-data[i][j])/h[j];\n"
	  "          q *= TMath::Freq(zb)-TMath::Freq(za);\n"
	  "        }\n"
	  "      p += q;\n"
	  "    }\n"
	  "  return p / (norm * ndata);\n"
	  "}\n");
  out << record; 

  out << 
    "//--------------------------------------------------------------------\n";

  sprintf(record, "vector<double> %s::bandwidth()", name.c_str());
  out << record << endl;
  sprintf(record,
	  "{\n"
	  "  vector<double> v(ndim);\n"
	  "  copy(h, h + ndim, v.begin());\n"
          "  return v;\n"
	  "}\n");
  out << record; 
  out.close();
}

//-----------------------------------------------------------------------------
// PRIVATE METHODS
//-----------------------------------------------------------------------------
// Initialize stuff
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void
KDE::_init()
{
  _z.clear();
  _mean.clear();
  _stddev.clear();
  _bandwidth.clear();
  _vmap.clear();

  if ( (int)_vars.size() == 0 )
    error("first call setvars(..) to set variable names");

  if ( _data.size() == 0 ) 
    error("first call add(..) to add points to KDE");

  // Divide data into two partitions:
  // 1. for KDE
  // 2. for likelihood
  
  _ndata = static_cast<int>(_partition * _data.size());

  cout << endl << "Data Partition: " << _ndata 
       << "/" << _data.size() - _ndata << endl;

  // IMPORTANT: set this flag here to prevent recursive call to _init()

  _initialized = true;
  
  if ( (int)_vars.size() != _D )
    {
      _vars.clear();
      for(int i=0; i < _D; i++)
        {
          char key[16];
          sprintf(key, "x%d", i+1);
          _vars.push_back(key);
        }
    }

  vector<double> d(_D,0);

  for(int i=0; i < _D; i++)
    {
      _z.push_back(0);
      _mean.push_back(0);
      _stddev.push_back(0);
      _bandwidth.push_back(0);
      _vmap[_vars[i]] = i;
    }
  
  // Compute means
  
  for(int i=0; i < (int)_ndata; i++)
    for(int j=0; j < _D; j++) _mean[j] += _data[i][j];

  for(int j=0; j < (int)_mean.size(); j++) _mean[j] /= _ndata;
  
  // Compute standard deviations

  for(int i=0; i < (int)_ndata; i++)
    for(int j=0; j < _D; j++)
      {
        double z = _data[i][j] - _mean[j];
        _stddev[j] += z*z;
      }

  for(int j=0; j < _D; j++)
    {
      _stddev[j] = sqrt(_stddev[j]/_ndata);
      if ( _stddev[j] != _stddev[j] ) error("_init() _stddev[j] is a nan");
    }

  // Compute default bandwidths

  double f = pow( 4.0/( (_D+2)*_ndata ), 1.0/(_D+4));
  for(int j=0; j < _D; j++)
    {
      _bandwidth[j] = f * _stddev[j];
      if ( _bandwidth[j] != _bandwidth[j] ) 
        error("_init() _bandwidth[j] is a nan");
    }
  
  // Print initial parameters

  for (int i=0; i < _D; i++)
    cout << "\tdefault bandwidth for " << _vars[i] << "\t---> "
         << _bandwidth[i] << endl;
  cout << endl;
  
  // Normalize density
  
  normalize();
  
  for(int j=0; j < _D; j++)
    if ( _bandwidth[j] != _bandwidth[j] ) 
      error("_init() *** _bandwidth[j] is a nan!");


  // histogram data

  _hw.clear();
  int nbins = 25;
  for(int j=0; j < _D; j++)
    {
      double xmin = TMath::Max(_min[j], _mean[j] - 2.5*_stddev[j]);
      double xmax = _mean[j] + 2.5*_stddev[j];
      _hw.push_back(TH1F(_vars[j].c_str(), "", nbins, xmin, xmax));
    }
 
  for(int i=0; i < (int)_ndata; i++)
    for(int j=0; j < _D; j++)
      _hw[j].Fill(_data[i][j]);
}

