#ifndef KIT_H
#define KIT_H
// -*- C++ -*-
//
// Package:    PhysicsTools
// Class:      kit
// 
/**\class kit kit.cc 
   PhysicsTools/TheNtupleMaker/src/kit.cc

 Description: A class of utilities. These functions are placed in a class 
              so that Reflex can handle overloading automatically. This is
	      just a collection of boilderplate code, written over the years, 
              to lessen clutter.
 
 Implementation:
     As simple as possible
*/
//
// Original Author:  Harrison B. Prosper
//         Created:  Fri Apr 04 2008
// $Id: kit.h,v 1.1.1.1 2011/05/04 13:04:28 prosper Exp $
//
//$Revision: 1.1.1.1 $
//-----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <stdio.h>
//-----------------------------------------------------------------------------
#include "TLorentzVector.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TMath.h"
#ifdef PROJECT_NAME
#include "PhysicsTools/TheNtupleMaker/interface/colors.h"
#endif
//-----------------------------------------------------------------------------

typedef std::vector<double>  vdouble;
typedef std::vector<std::vector<double> > vvdouble;

struct kit
{
  ///
  struct MatchedPair
  {
    int first;
    int second;
    double distance;
    bool operator<(const MatchedPair& o) const 
    { return this->distance < o.distance; }
  };

  // Simple struct to collect together standard attributes

  // Note: Unlike a class, all attributes of a struct are public
  ///
  struct PtThing
  {
    PtThing() {}
    ~PtThing() {}

    /// Copy constructor.
    PtThing(const PtThing& rhs)
    {
      *this = rhs; // this will call assignment operator
    }

    /// Assignment. 
    PtThing& operator=(const PtThing& rhs)
    {
      index  = rhs.index;
      id     = rhs.id;
      type   = rhs.type;
      pt  = rhs.pt;
      eta = rhs.eta;
      phi = rhs.phi;
      et  = rhs.et;
      charge = rhs.charge;
      var = rhs.var;
      return *this;
    }

    /** Find \f$|Delta R = \sqrt{\Delta\phi^2+\Delta\eta^2}\f$ between this
        PtThing and the given.
     */
    float deltaR(PtThing& thing)
    {
      float deta = eta - thing.eta;
      float dphi = phi - thing.phi;
    
      // Make sure acute
      if ( fabs(dphi) > TMath::Pi() ) dphi = TMath::TwoPi() - fabs(dphi);
      return sqrt(deta*deta+dphi*dphi);
    }

    /// Compare direction of this PtThing with another using deltaR.
    bool matches(PtThing& thing, float drcut=0.4)
    {
      return deltaR(thing) < drcut;
    }

    int index;
    int id;
    std::string type;
    float pt;
    float eta;
    float phi;
    float et;
    float charge;
    
    /// Map for additional variables.
    std::map<std::string, float> var;
    
    /// To sort PtThings in descending pt.
    bool operator<(const PtThing& o) const { return o.pt < this->pt; }
  };
  
  /**
     Model a count as a function object. 
     Each time the object is 
     called, the count is incremented for the given string. The object is
     self-initializing; that is, there is no need for an explicit zeroing of
     the count. However, if you want the counts to be listed in a particular
     order, you can call the object, before you enter the event loop, for 
     each cut name but using a weight=0.
     <p>
     \code
     Usage:
     
     Count count;
     : :
     // increment count (weight=1 is the default)
     count("Selected Muon", weight);
     
     // To display the counts at the end, do
     : :
     count.ls();
     \endcode
  */
  class Count
  {
  public:
    ///
    Count() : _count(std::map<std::string, double>()),
              _name(std::vector<std::string>()),
              _hist(TH1F("counts", "",1,0,1)) 
    {
      _hist.SetBit(TH1::kCanRebin);
      _hist.SetStats(0);
    }

    ///
    ~Count(){}

    ///
    void operator()(std::string name, double weight=1) 
    {
      if ( _count.find(name) == _count.end() )
        { 
          _count[name] = 0;
          _name.push_back(name);
        }
      _count[name] += weight;
      _hist.Fill(name.c_str(), weight);
    }
   
    TH1F* hist() { return &_hist; }

    /// List counts (default = list to screen).
    void ls(std::ostream& os = std::cout)
    {
      for(unsigned int i=0; i < _name.size(); ++i)
        {
          
          std::string name = _name[i];
          double count  = _count[name];
          std::string rec = name + 
            "   .......................................................";

          rec = rec.substr(0,40);
          char record[80];
          sprintf(record,"%5d %-40s %10f", i+1, rec.c_str(), count);
          os << record << std::endl;
        }
    }
    
  private:
    std::map<std::string, double> _count;
    std::vector<std::string> _name;
    TH1F _hist;
  };

  // ------------------------------------------------------------------------

  ///
  enum PDGID
  {
    BOTTOM  = 5,
    TOP     = 6,
    
    ELECTRON=11,
    NUE     =12,
    MUON    =13,
    NUMU    =14,
    TAU     =15,
    NUTAU   =16,

    PHOTON  =22,
    ZBOSON  =23,
    WBOSON  =24,
    
    GLUINO   =1000021,
    CHI10    =1000022,
    CHI20    =1000023,
    CHI30    =1000025,
    CHI40    =1000035,
    CHI1P    =1000024,
    CHI2P    =1000037,
    GRAVITINO=1000039
  };

  ///
  static
  float deltaPhi(float phi1, float phi2);

  ///
  static
  float deltaR(float eta1, float phi1, float eta2, float phi2);

  
  ///
  static
  std::vector<MatchedPair>  deltaR(std::vector<PtThing>& v1, 
                                   std::vector<PtThing>& v2);

  ///
  static
  std::vector<MatchedPair>  deltaR(std::vector<double>& eta1,
                                   std::vector<double>& phi1, 
                                   std::vector<double>& eta2,
                                   std::vector<double>& phi2,
                                   bool omit=false);

#ifdef PROJECT_NAME
  ///
  static
  std::string         particleName(int PDGID);
#endif

  ///
  static
  std::string         nameonly(std::string name);

  ///
  static
  std::string         strip(std::string line);

  ///
  static
  std::string         truncate(std::string  s, 
                               std::string  substr,
                               int direction=1);

  ///
  static
  void                bisplit(std::string  s, 
                              std::string& left, 
                              std::string& right, 
                              std::string  delim,
                              int direction=1);
  ///
  static
  void                split(std::string str, 
                            std::vector<std::string>& vstr);

  ///
  static
  std::string         replace(std::string& str, 
                              std::string  oldstr, 
                              std::string  newstr);
  ///
  static
  std::string         shell(std::string cmd);

  // -------------------
  // HISTOGRAM UTILITIES
  // -------------------

  ///
  static
  void                setStyle();

  ///
  static
  std::vector<double> contents(TH1* hist);

  ///
  static
  std::vector<double> cdf(TH1* hist);

  ///
  static
  void                setContents(TH1* hist, std::vector<double>& c);

  ///
  static
  void                setErrors(TH1* hist, std::vector<double>& err);

  ///
  static
  TH1F*               divideHistograms(TH1* N, TH1* D, std::string ytitle);

  ///
  static
  void                saveHistograms(std::string histfilename, 
                                     TDirectory* dir=gDirectory, 
                                     TFile* hfile=0, int depth=0);
  ///
  static
  TCanvas*            canvas(std::string name, 
                             int n=0, 
                             int width=500,
                             int height=500);
  
  ///
  static
  TLegend*            legend(std::string title, int nlines, 
                             float xmin=0.62, 
                             float xmax=0.87, 
                             float ymax=0.93);
  
  ///
  static
  TH1F*               histogram(std::string hname,
                                std::string xtitle,
                                std::string ytitle,
                                int    nbin,
                                float  xmin,
                                float  xmax,
                                int    color=kBlack,
                                int    linestyle=1,
                                int    linewidth=2);
  
  ///
  static
  TH2F*               histogram(std::string hname,
                                std::string xtitle,
                                int    nbinx,
                                float  xmin,
                                float  xmax,
                                std::string ytitle,
                                int    nbiny,
                                float  ymin,
                                float  ymax,
                                int    color=kBlack,
                                int    linestyle=1,
                                int    linewidth=2);

  ///
  static
  TGraph*             graph(std::vector<double>& x, 
                            std::vector<double>& y,
                            std::string xtitle,
                            std::string ytitle,
                            float  xmin,
                            float  xmax,
                            float  ymin,
                            float  ymax,
                            int    color=kBlack,
                            int    linestyle=1,
                            int    linewidth=2);

  ///
  static
  void                plot(TCanvas* canvas, 
                           TH1* h1, std::string option="");

  ///
  static
  void                plot(TCanvas* canvas, 
                           std::vector<TH1*>& h, std::string option="");

  // Statistics

  /////////////////////////////////////////////////////////////////////////////
  // File:    poissongamma.hpp
  // Description:	x = poissongamma(p,a,d[,returnlog,[scale]])
  // 
  //              compute the marginal likelihood:
  //              a product of Poisson distributions
  //              and a prior that is a product of gamma distributions.
  //
  //              Model:
  //
  //              d_i = Sum p_j * a_ji,  i=1..M,   j=1..N
  // 
  //              D_i  is count corresponding to true mean d_i
  //              A_ji is count corresponding to true mean a_ji
  //
  //              D is a vector of M observed counts (that is, over M bins)
  //              p is a vector of N parameters (that is, over N sources)
  //              A is a vector of vectors of size N x M counts.
  //
  // WARNING: The number of terms grows exponentially with the number of 
  //          sources and the maximum bin count. Therefore, this routine
  //          really only makes sense for rather few sources with modest
  //          bin counts. When the number of sources is > 4 and/or the 
  //          maximum observed bin count is large use poissongammamc,
  //          which does the calculation by Monte Carlo integration.
  //
  // Created: 20-Dec-2001 Harrison B. Prosper
  //          11-Mar-2004 HBP change typedefs
  //          21-Jun-2005 HBP add poissongammasmc
  /////////////////////////////////////////////////////////////////////////////

  
  ///
  static
  double logpoisson(vdouble&	p,  // Weights "p_j" 
		    vvdouble&	A,  // Counts  "A_ji" for up to 10 sources
		    vdouble&	D,  // Counts  "D_i" for data.
		    bool scale=true); // Scale p_j if true  

  ///
  static
  double poissongamma(vdouble&	p,        // Weights "p_j" 
		      vvdouble&	A,        // Counts "A_ji" for up to 10 sources
		      vdouble&	D,        // Counts  "D_i" for data.
		      bool returnlog=false, // return log(P) if true
		      bool scale=true);     // Scale p_j if true  
  
  /// For simple 1-D application.
  static
  double poissongamma1D(double xsec,
			double Lum,
			double sigmaLum,
			double B,
			double sigmaB,
			double Datum);
  

  ///
  static
  double poissongamma2(double	  p1,
		       double     p2,
		       vdouble&   A1,
		       vdouble&   A2,
		       vdouble&	D,
		       bool returnlog=false,
		       bool scale=true);

  ///
  static
  double poissongamma5(double	p1,
		       double     p2,
		       double     p3,
		       double     p4,
		       double     p5,
		       vdouble&   A1,
		       vdouble&   A2,
		       vdouble&   A3,
		       vdouble&   A4,
		       vdouble&   A5,
		       vdouble&	D,
		       bool returnlog=false,
		       bool scale=true);

  ///
  static
  void initpoissongammamc(vvdouble& A,       // Counts  "A_ji" 
			  int total=10000);  // Number of MC sampling points

  ///
  static
  double poissongammamc(vdouble&	p,
			vvdouble&	A,
			vdouble&	D,
			bool returnlog=false,
			bool scale=true);

};


/*
std::ostream& operator<<(std::ostream& os, 
			 std::vector<reco::GenParticle*>& vp);

std::ostream& operator<<(std::ostream& os, 
			 reco::GenParticle* p);
*/

#endif
