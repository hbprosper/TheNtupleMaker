// -*- C++ -*-
//
// Package:    PhysicsTools
// Class:      kit
// 
/**\class kit kit.cc 
   PhysicsTools/TheNtupleMaker/src/kit.cc

   Description: Some simple utilities

   Implementation:
   Common sense and a sense of beauty.
*/
//
// Original Author:  Harrison B. Prosper
//         Created:  Wed Jun 20 19:53:47 EDT 2007
//         Updated:  Sat Oct 25 2008 - make matchInDeltaR saner
// $Id: kit.cc,v 1.1.1.1 2011/05/04 13:04:29 prosper Exp $
//
//
//-----------------------------------------------------------------------------
#ifdef PROJECT_NAME
#include <Python.h>
#include <boost/python.hpp>
#endif
#include <sstream>
#include <cassert>
#include <string>
#include <cmath>
#include <iomanip>
#include <string>
#include <algorithm>
#include <map>
//-----------------------------------------------------------------------------
#include "TROOT.h"
#include "TSystem.h"
#include "TString.h"
#include "TList.h"
#include "TClass.h"
#include "TIterator.h"
#include "TFile.h"
#include "TInterpreter.h"
#ifdef PROJECT_NAME
#include "Math/SpecFuncMathCore.h"
#include "CLHEP/Random/RandGamma.h"
#include "HepPID/ParticleName.hh"
#include "PhysicsTools/TheNtupleMaker/interface/kit.h"
#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/Utilities/interface/Exception.h"
#else
#include "kit.h"
#endif
//-----------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------

const int MAXDEPTH=10;

static vector<TCanvas*> CANVAS;
static vector<TH1*> HIST;

float
kit::deltaPhi(float phi1, float phi2)
{
  float deltaphi = phi2 - phi1;
  if ( fabs(deltaphi) > M_PI ) deltaphi = 2 * M_PI - fabs(deltaphi);
  return deltaphi;
}

float 
kit::deltaR(float eta1, float phi1, float eta2, float phi2)
{
  float deltaeta = eta1 - eta2;
  float deltaphi = deltaPhi(phi1, phi2);
  return sqrt(deltaeta*deltaeta + deltaphi*deltaphi);
}

std::vector<kit::MatchedPair>
kit::deltaR(std::vector<PtThing>& v1,
            std::vector<PtThing>& v2)
{
  if ( v1.size() == 0 || v2.size() == 0 ) return vector<kit::MatchedPair>();

  vector<kit::MatchedPair> mp(v1.size(), kit::MatchedPair());
  vector<kit::MatchedPair> vp(v2.size(), kit::MatchedPair());

  for(unsigned i=0; i < v1.size(); i++)
    {
      for(unsigned j=0; j < v2.size(); j++)
        {
          vp[j].first = i;
          vp[j].second = j;
          vp[j].distance = v1[i].deltaR(v2[j]);
        }
      std::sort(vp.begin(), vp.end());
      mp[i].first = i;
      mp[i].second = vp[0].second;
      mp[i].distance = vp[0].distance;
    }
  return mp;
}

std::vector<kit::MatchedPair>
kit::deltaR(std::vector<double>& eta1,
            std::vector<double>& phi1,
            std::vector<double>& eta2,
            std::vector<double>& phi2,
            bool omit)
{
  if ( eta1.size() == 0 ) return vector<kit::MatchedPair>();
  if ( eta2.size() == 0 ) return vector<kit::MatchedPair>();
  if ( eta1.size() != phi1.size() ) return vector<kit::MatchedPair>();
  if ( eta2.size() != phi2.size() ) return vector<kit::MatchedPair>();

  vector<kit::MatchedPair> mp(eta1.size(), kit::MatchedPair());
  vector<kit::MatchedPair> vp(eta2.size(), kit::MatchedPair());

  for(unsigned i=0; i < eta1.size(); i++)
    {
      for(unsigned j=0; j < eta2.size(); j++)
        {
          if ( omit && ( i == j ) ) continue;
            
          vp[j].first = i;
          vp[j].second = j;
          vp[j].distance = kit::deltaR(eta1[i], phi1[i], eta2[j], phi2[j]);
        }
      std::sort(vp.begin(), vp.end());
      mp[i].first = i;
      mp[i].second = vp[0].second;
      mp[i].distance = vp[0].distance;
    }
  return mp;
}

#ifdef PROJECT_NAME
std::string 
kit::particleName(int id) 
{
  return HepPID::particleName(id);
}
#endif

// Extract name of a file without extension
string 
kit::nameonly(string filename)
{
  int i = filename.rfind("/");
  int j = filename.rfind(".");
  if ( j < 0 ) j = filename.size();
  return filename.substr(i+1,j-i-1);
}

string 
kit::strip(string line)
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

string 
kit::truncate(string s, string substr, int direction)
{
  if ( direction > 0 )
    {
      int i = s.rfind(substr);
      if ( i >= 0 )
        return s.substr(0,i);
      else
        return s;
    }
  else
    {
      int i = s.find(substr);
      if ( i >= 0 )
        return s.substr(i+1, s.size()-i-1);
      else
        return s;
    }
}


void
kit::bisplit(string str, string& left, string& right, string delim,
             int direction)
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

void 
kit::split(string str, vector<string>& vstr)
{
  vstr.clear();
  istringstream stream(str);
  while ( stream )
    {
      string str;
      stream >> str;
      if ( stream ) vstr.push_back(str);
    }
}

string
kit::replace(string& str, string oldstr, string newstr)
{
  return string(TString(str).ReplaceAll(oldstr, newstr).Data());
}

string 
kit::shell(string cmd)
{
  FILE* f = popen(cmd.c_str(),"r");
  int buffsize=8192;
  char s[8192];
  int n = fread(s,1,buffsize,f);
  pclose(f);
  string result = kit::strip(string(s).substr(0,n));
  return result;
}

// ---------------------------------------------
// Some histograms utilities
// ---------------------------------------------
namespace {
  const int LINEWIDTH= 2;
  const int TEXTFONT = 42;
  const int NDIV     = 505;
  const int WIDTH    = 500;
  const int HEIGHT   = 500;
  const double TEXTSIZE = 0.04;
  const double MARKERSIZE = 1.0;
}

void 
kit::setStyle()
{
  gROOT->SetStyle("Pub");
  gStyle->SetPalette(1);

  TStyle* style = gROOT->GetStyle("Pub");
  style->SetFrameBorderMode(0);
  style->SetCanvasBorderSize(0);
  style->SetCanvasBorderMode(0);
  style->SetCanvasColor(0);
  style->SetPadBorderMode(0);
  style->SetPadColor(0);

  // Margins

  style->SetPadTopMargin(0.05);
  style->SetPadBottomMargin(0.16);
  style->SetPadLeftMargin(0.20);
  style->SetPadRightMargin(0.10);

  // For the Global title:

  style->SetOptTitle(0);
  style->SetTitleFont(TEXTFONT);
  style->SetTitleColor(1);
  style->SetTitleTextColor(1);
  style->SetTitleFillColor(10);
  style->SetTitleFontSize(0.05);

  // For the axis titles:

  style->SetTitleColor(1, "XYZ");
  style->SetTitleFont(TEXTFONT, "XYZ");
  style->SetTitleSize(0.05, "XYZ");
  style->SetTitleXOffset(0.9);
  style->SetTitleYOffset(1.25);

  // For the axis labels:

  style->SetLabelColor(1, "XYZ");
  style->SetLabelFont(TEXTFONT, "XYZ");
  style->SetLabelOffset(0.007, "XYZ");
  style->SetLabelSize(0.05, "XYZ");

  // For the axis:

  style->SetAxisColor(1, "XYZ");
  style->SetStripDecimals(kTRUE);
  style->SetTickLength(0.03, "XYZ");
  style->SetNdivisions(NDIV, "XYZ");
  style->SetPadTickX(1);  
  style->SetPadTickY(1);

  style->cd();
}

vector<double> 
kit::contents(TH1* hist)
{
  vector<double> c(hist->GetNbinsX());
  for(int i=0; i < hist->GetNbinsX(); i++)
    c[i] = hist->GetBinContent(i+1);
  return c;
}

vector<double> 
kit::cdf(TH1* hist)
{
  vector<double> c(hist->GetNbinsX());
  c[0] = hist->GetBinContent(1);
  for(int i=1; i < hist->GetNbinsX(); i++)
    c[i] = c[i-1]+hist->GetBinContent(i+1);
  return c;
}

void
kit::setContents(TH1* hist, vector<double>& c)
{
  int n = hist->GetNbinsX();
  int nbin = n < (int)c.size() ? n : c.size();
  for(int i=0; i < nbin; i++) hist->SetBinContent(i+1, c[i]);
}

void
kit::setErrors(TH1* hist, vector<double>& err)
{
  int n = hist->GetNbinsX();
  int nbin = n < (int)err.size() ? n : err.size();
  for(int i=0; i < nbin; i++) hist->SetBinError(i+1, err[i]);
}

TH1F*
kit::divideHistograms(TH1* N, TH1* D, string ytitle)
{
  int nN = N->GetNbinsX();
  int nD = D->GetNbinsX();
  int nbin = nN < nD ? nN : nD;
  vector<double> n = kit::contents(N);
  vector<double> d = kit::contents(D);

  string nname(N->GetName());
  string dname(D->GetName());
  string hname = nname + "_" + dname;

  double xmin = N->GetXaxis()->GetXmin();
  double xmax = N->GetXaxis()->GetXmax();

  TH1F* h = new TH1F(hname.c_str(), "", nbin, xmin, xmax);

  // Set some reasonable defaults

  h->SetLineColor(kBlack);
  h->SetMarkerSize(1);
  h->SetMarkerStyle(20);

  h->GetXaxis()->CenterTitle();
  h->GetXaxis()->SetTitle(N->GetXaxis()->GetTitle());
  h->GetXaxis()->SetTitleOffset(1.3);
  h->SetNdivisions(504, "X");
  h->SetMarkerSize(1.0);

  h->GetYaxis()->CenterTitle();
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->GetYaxis()->SetTitleOffset(1.8);
  h->SetNdivisions(510, "Y");

  for(int i=0; i < nbin; i++) 
    {
      int bin = i+1;
      double p = 0;
      double e = 0;
      if ( (d[i] > 0) && (n[i] > 0) ) 
        {
          p = n[i] / d[i];
          // Use rough error calculation for now
          e = sqrt(d[i]*p*(1-p))/d[i];
        }
      h->SetBinContent(bin, p);
      h->SetBinError(bin, e);
    }
  return h;
}



void
kit::saveHistograms(string histfilename, 
                    TDirectory* dir, TFile* hfile, int depth)
{
  // Create output file

  if ( depth == 0 )
    {
      cout << "Saving histograms to " << histfilename << endl;
      hfile = new TFile(histfilename.c_str(), "RECREATE");
    }

  // Important!
  depth++;
  // Check recursion depth 
  if ( depth > 100 )
    {
      cout << "saveHistograms is lost in trees!" << endl;
      exit(0);
    }
  string tab(2*depth, ' ');

  // Loop over all histograms

  TList* list = dir->GetList();
  TListIter* it = (TListIter*)list->MakeIterator();

  while ( TObject* o = it->Next() )
    {
      dir->cd();
      
      if ( o->IsA()->InheritsFrom("TDirectory") )
        {
          TDirectory* d = (TDirectory*)o;
          cout << tab << "BEGIN " << d->GetName() << endl;
          kit::saveHistograms(histfilename, d, hfile, depth);
          cout << tab << "END " << d->GetName() << endl;
        }
      // Note: All histograms inherit from TH1
      else if ( o->IsA()->InheritsFrom("TH1") )
        {
          TH1* h = (TH1*)o;
          cout << tab << o->ClassName() 
               << "\t" << h->GetName()
               << "\t" << h->GetTitle()
               << endl;
          hfile->cd();
          h->Write("", TObject::kOverwrite);
        }
      else if ( o->IsA()->InheritsFrom("TCanvas") )
        {
          TCanvas* c = (TCanvas*)o;
          cout << tab << o->ClassName() 
               << "\t" << c->GetName()
               << "\t" << c->GetTitle()
               << endl;
          hfile->cd();
          c->Write("", TObject::kOverwrite);
        }
      else if ( o->IsA()->InheritsFrom("TGraph") )
        {
          TGraph* g = (TGraph*)o;
          cout << tab << o->ClassName() 
               << "\t" << g->GetName()
               << "\t" << g->GetTitle()
               << endl;
          hfile->cd();
          g->Write("", TObject::kOverwrite);
        }
    } // end of loop over keys

  hfile->Close();
  delete hfile;
}


TCanvas*
kit::canvas(string name, int n, int width, int height)
{
  kit::shell("mkdir -p figures");
  string cname  = "figures/fig_" + name;
  string ctitle = cname;
  int xoffset = 40*n;
  int yoffset = 10*n;
  return new TCanvas(cname.c_str(), 
                     ctitle.c_str(), 
                     xoffset, yoffset, width, height);
}
//-----------------------------------------------------------------------------
TLegend*
kit::legend(string title, int nlines, float xmin, float xmax, float ymax)
{    
  float ymin = ymax - nlines * 0.08;
  TLegend* l = new TLegend(xmin, ymin, xmax, ymax, title.c_str());
  l->SetFillColor(0);
  l->SetFillStyle(4000);
  l->SetBorderSize(0);
  l->SetTextSize(TEXTSIZE);
  l->SetTextFont(TEXTFONT);
  return l;
}
//-----------------------------------------------------------------------------
TH1F* 
kit::histogram(string hname,
               string xtitle,
               string ytitle,
               int    nbin,
               float  xmin,
               float  xmax,
               int    color,
               int    lstyle,
               int    lwidth)
{
  TH1F* h = new TH1F(hname.c_str(), "", nbin, xmin, xmax);

  h->SetLineColor(color);
  h->SetLineStyle(lstyle);
  h->SetLineWidth(lwidth);
 
  h->SetMarkerColor(kRed);
  h->SetMarkerStyle(20);
  h->SetMarkerSize(1.0);
 
  h->GetXaxis()->CenterTitle();
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->GetXaxis()->SetTitleOffset(1.3);

  h->GetYaxis()->CenterTitle();
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->GetYaxis()->SetTitleOffset(1.8);
  return h;
}
//-----------------------------------------------------------------------------
TH2F* 
kit::histogram(string hname,
               string xtitle,
               int    nbinx,
               float  xmin,
               float  xmax,
               string ytitle,
               int    nbiny,
               float  ymin,
               float  ymax,
               int    color,
               int    lstyle,
               int    lwidth)
{
  TH2F* h = new TH2F(hname.c_str(), "", nbinx, xmin, xmax, nbiny, ymin, ymax);

  h->SetLineColor(color);
  h->SetLineStyle(lstyle);
  h->SetLineWidth(lwidth);
 
  h->GetXaxis()->CenterTitle();
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->GetXaxis()->SetTitleOffset(1.3);

  h->GetYaxis()->CenterTitle();
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->GetYaxis()->SetTitleOffset(1.8);
  return h;
}
//-----------------------------------------------------------------------------
TGraph* 
kit::graph(vector<double>& x, 
           vector<double>& y,
           string xtitle,
           string ytitle,
           float  xmin,
           float  xmax,
           float  ymin,
           float  ymax,
           int    color,
           int    lstyle,
           int    lwidth)
{
  int nbins = x.size();
    
  TGraph* g = new TGraph(nbins, &x[0], &y[0]);
  g->SetLineColor(color);
  g->SetLineStyle(lstyle);
  g->SetLineWidth(lwidth);

  g->GetXaxis()->CenterTitle();
  g->GetXaxis()->SetTitle(xtitle.c_str());
  g->GetXaxis()->SetTitleOffset(1.3);
  g->GetHistogram()->SetAxisRange(xmin, xmax, "X");

  g->GetYaxis()->CenterTitle();
  g->GetYaxis()->SetTitle(ytitle.c_str());
  g->GetYaxis()->SetTitleOffset(1.8);
  g->GetHistogram()->SetAxisRange(ymin, ymax, "Y");
  return g;
}

void 
kit::plot(TCanvas* canvas, 
          TH1* h, string opt)
{
  if (!canvas) return;
  if (!h) return;

  TString tstr(opt.c_str());
  tstr.ToUpper();
  bool witherrors = tstr.Contains("E");
  bool save = tstr.Contains("S");

  canvas->cd();
  string option("");
  if (witherrors) option = "EP";
  h->Draw(option.c_str());
  canvas->Update();

  if (save) canvas->SaveAs(".gif");
}

void 
kit::plot(TCanvas* canvas, 
          vector<TH1*>& h, string opt)
{
  if (!canvas) return;
  if ( h.size() == 0 ) return;

  TString tstr(opt.c_str());
  tstr.ToUpper();
  bool witherrors = tstr.Contains("E");
  bool save = tstr.Contains("S");

  canvas->cd();
  if (witherrors)
    h[0]->Draw("EP");
  else
    h[0]->Draw();

  for(unsigned i=1; i < h.size(); i++) h[i]->Draw("same");
  canvas->Update();

  if (save) canvas->SaveAs(".gif");
}

// ---- STATISTICS

///////////////////////////////////////////////////////////////////////////////
// File:	poissongamma.cpp
// Description:	x = poissongamma(p,a,d)
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
//              Simple 1-D Application:
//              
//              d =  xsec * e_a * a + e_b * b 
//
//              where p_1 = xsec * e_a
//                    p_2 = e_b
//                    a_11= a
//                    a_21= b
//
//              and e_a, e_b are scale factors defined by
//              e = sigma^2 / estimate
//
// WARNING: The number of terms grows exponentially with the number of 
//          sources and the maximum bin count. Therefore, this routine
//          really only makes sense for rather few sources with modest
//          bin counts. When the number of sources is > 4 and/or the 
//          maximum observed bin count is large use poissongammamc,
//          which does the calculation by Monte Carlo integration.
//
// Created:     20-Dec-2001 Harrison B. Prosper
//              11-Mar-2004 HBP add additional interfaces
//              07-Jun-2005 HBP increase number of sources to 10! See warning
//                          above.
//              21-Jun-2005 HBP add poissongammamc
///////////////////////////////////////////////////////////////////////////////

const int MAXBUF = 10000; // Maximum count per bin
const int MAXSRC = 10;    // Maximum number of sources

// Global variables to avoid memory allocation.

static double cs[MAXSRC][MAXBUF];
static double ns[MAXSRC];
static double si[MAXSRC];
static double x [MAXSRC];
static double y [MAXSRC];

double 
kit::logpoisson(vdouble&	p,  // Weights "p_j" 
                vvdouble&	A,  // Counts  "A_ji" for up to 10 sources
                vdouble&	D,  // Counts  "D_i" for data.
                bool scale)     // Scale p_j if true  
{
  int N = p.size(); // Number of sources (N)
  int M = D.size(); // Number of bins    (M)
  vdouble x(N);

  // Scale counts if requested

  if ( scale )
    {
      for ( int j = 0; j < N; j++ )
        {
          double ns = 0;
          for ( int i = 0; i < M; i++ ) ns += A[j][i];
          x[j] = p[j]/(ns+M);
        }
    }
  else
    for ( int j = 0; j < N; j++ )
      x[j] = p[j];
    
  double q = 0;
  for ( int i = 0; i < M; i++ )
    {
      // Compute Poisson parameter for ith bin
      double d = 0;
      for ( int j = 0; j < N; j++ ) d += x[j] * A[j][i];
      double p = exp(-d);
      int N = (int)D[i];
      if ( N > 0 ) for (int n = 1; n <= N; n++) p *= d / n;
      q += log(p);
    }
  return q;
}

double 
kit::poissongamma(vdouble&	p,  // Weights "p_j" 
                  vvdouble&	a,  // Counts  "A_ji" for up to 10 sources
                  vdouble&	d,  // Counts  "D_i" for data.
                  bool returnlog,    // return log(P) if true
                  bool scale)     // Scale p_j if true  
{
  int N = p.size(); // Number of sources (N)
  int M = d.size(); // Number of bins    (M)

  if ( a.size() != (unsigned int)N )
    {
      std::cout << "**Error - poissongamma - mis-match in number of sources\n"
                << "size(p): " << N << " differs from size(A) = " << a.size()
                << std::endl;
      exit(0);
    }

  if ( a[0].size() != (unsigned int)M )
    {
      std::cout << "**Error - poissongamma - mis-match in number of sources\n"
                << "size(d): " << M << " differs from size(A[0]) = " 
                << a[0].size()
                << std::endl;
      exit(0);
    }

  if ( M < 1 ) return -1.0;
  
  if ( ( N < 2 ) || ( N > MAXSRC ) ) return -2.0;
  
  // Get total counts per source

  for ( int j = 0; j < N; j++ )
    {
      ns[j] = 0;
      for ( int i = 0; i < M; i++ ) ns[j] += a[j][i];
    }

  // Get data count

  double nd = 0.0; 					
  for ( int i = 0; i < M; i++ ) nd += d[i];

  // Scale counts

  for ( int j = 0; j < N; j++ )
    {
      if ( scale )
        x[j] = p[j] / (ns[j]+M);
      else
        x[j] = p[j];
      y[j] = x[j] / (1+x[j]);
    }

  // loop over the M terms of the product,
  // corresponding to the M bins

  double prob;
  if ( returnlog )
    prob = 0.0;
  else
    prob = 1.0;
  for ( int i = 0; i < M; i++)
    {
      double di = d[i];  // data count for bin i
      int D = (int)di;

      // compute terms of sum from zero to D
      
      for ( int j = 0; j < N; j++ )
        {
          si[j]    = a[j][i];  // "A_ji"
          cs[j][0] = pow(1+x[j],-(si[j]+1));
        }
      
      if ( D > 0 )
        {
          for ( int k = 1; k < D+1; k++ )
            for ( int j = 0; j < N; j++ )
              cs[j][k] = cs[j][k-1] * y[j] * (si[j]+k)/k;
        }

      // compute sum

      double sum = 0.0;
      switch (N)
        {
        case 2:
          for (int j = 0; j < D+1; j++)
            sum += 
              cs[0][j] *
              cs[1][D-j];
          break;

        case 3:
          for (int j = 0; j < D+1; j++)
            for (int k = 0; k < D+1-j; k++)
              sum += 
                cs[0][j] *
                cs[1][k] *
                cs[2][D-j-k];
          break;

        case 4:
          for (int j = 0; j < D+1; j++)
            for (int k = 0; k < D+1-j; k++)
              for (int l = 0; l < D+1-j-k; l++)
                sum += 
                  cs[0][j] *
                  cs[1][k] *
                  cs[2][l] *
                  cs[3][D-j-k-l];
          break;

        case 5:
          for (int j = 0; j < D+1; j++)
            for (int k = 0; k < D+1-j; k++)
              for (int l = 0; l < D+1-j-k; l++)
                for (int m = 0; m < D+1-j-k-l; m++)
                  sum += 
                    cs[0][j] *
                    cs[1][k] *
                    cs[2][l] *
                    cs[3][m] *
                    cs[4][D-j-k-l-m];
          break;

        case 6:
          for (int j = 0; j < D+1; j++)
            for (int k = 0; k < D+1-j; k++)
              for (int l = 0; l < D+1-j-k; l++)
                for (int m = 0; m < D+1-j-k-l; m++)
                  for (int m1 = 0; m1 < D+1-j-k-l-m; m1++)
                    sum += 
                      cs[0][j] *
                      cs[1][k] *
                      cs[2][l] *
                      cs[3][m] *
                      cs[4][m1] *
                      cs[5][D-j-k-l-m-m1];
          break;

        case 7:
          for (int j = 0; j < D+1; j++)
            for (int k = 0; k < D+1-j; k++)
              for (int l = 0; l < D+1-j-k; l++)
                for (int m = 0; m < D+1-j-k-l; m++)
                  for (int m1 = 0; m1 < D+1-j-k-l-m; m1++)
                    for (int m2 = 0; m2 < D+1-j-k-l-m-m1; m2++)
                      sum += 
                        cs[0][j] *
                        cs[1][k] *
                        cs[2][l] *
                        cs[3][m] *
                        cs[4][m1] *
                        cs[5][m2] *
                        cs[6][D-j-k-l-m-m1-m2];
          break;

        case 8:
          for (int j = 0; j < D+1; j++)
            for (int k = 0; k < D+1-j; k++)
              for (int l = 0; l < D+1-j-k; l++)
                for (int m = 0; m < D+1-j-k-l; m++)
                  for (int m1 = 0; m1 < D+1-j-k-l-m; m1++)
                    for (int m2 = 0; m2 < D+1-j-k-l-m-m1; m2++)
                      for (int m3 = 0; m3 < D+1-j-k-l-m-m1-m2; m3++)
                        sum += 
                          cs[0][j] *
                          cs[1][k] *
                          cs[2][l] *
                          cs[3][m] *
                          cs[4][m1] *
                          cs[5][m2] *
                          cs[6][m3] *
                          cs[7][D-j-k-l-m-m1-m2-m3];
          break;

        case 9:
          for (int j = 0; j < D+1; j++)
            for (int k = 0; k < D+1-j; k++)
              for (int l = 0; l < D+1-j-k; l++)
                for (int m = 0; m < D+1-j-k-l; m++)
                  for (int m1 = 0; m1 < D+1-j-k-l-m; m1++)
                    for (int m2 = 0; m2 < D+1-j-k-l-m-m1; m2++)
                      for (int m3 = 0; m3 < D+1-j-k-l-m-m1-m2; m3++)
                        for (int m4 = 0; m4 < D+1-j-k-l-m-m1-m2-m3; m4++)
                          sum += 
                            cs[0][j] *
                            cs[1][k] *
                            cs[2][l] *
                            cs[3][m] *
                            cs[4][m1] *
                            cs[5][m2] *
                            cs[6][m3] *
                            cs[7][m4] *
                            cs[8][D-j-k-l-m-m1-m2-m3-m4];
          break;

        case 10:
          for (int j = 0; j < D+1; j++)
            for (int k = 0; k < D+1-j; k++)
              for (int l = 0; l < D+1-j-k; l++)
                for (int m = 0; m < D+1-j-k-l; m++)
                  for (int m1 = 0; m1 < D+1-j-k-l-m; m1++)
                    for (int m2 = 0; m2 < D+1-j-k-l-m-m1; m2++)
                      for (int m3 = 0; m3 < D+1-j-k-l-m-m1-m2; m3++)
                        for (int m4 = 0; m4 < D+1-j-k-l-m-m1-m2-m3; m4++)
                          for (int m5 = 0; m5 < D+1-j-k-l-m-m1-m2-m3-m4; m5++)
                            sum += 
                              cs[0][j] *
                              cs[1][k] *
                              cs[2][l] *
                              cs[3][m] *
                              cs[4][m1] *
                              cs[5][m2] *
                              cs[6][m3] *
                              cs[7][m4] *
                              cs[8][m5] *
                              cs[9][D-j-k-l-m-m1-m2-m3-m4-m5];
          break;
        };
      if ( returnlog )
        prob += log(sum);
      else
        prob *= sum;
    }
  return prob;
}

// For simple 1-D application
//
// 
double 
kit::poissongamma1D(double xsec,
                    double Lum,
                    double sigmaLum,
                    double Bkg,
                    double sigmaBkg,
                    double Datum)
{
  // Compute scale factors and effective counts
  double eA = sigmaLum * sigmaLum / Lum;
  double Aeff = Lum / eA;

  if ( Bkg > 0.0 )
    {
      double eB = sigmaBkg * sigmaBkg / Bkg;
      double Beff = Bkg / eB;
      
      vdouble  p(2); p[0] = xsec * eA; p[1] = eB;
      vdouble  a(1), b(1), D(1); a[0] = Aeff; b[0] = Beff;
      vvdouble A(2); A[0] = a; A[1] = b; D[0] = Datum;

      return kit::poissongamma(p, A, D, false, false);
    }
  else
    {
      vdouble  p(1); p[0] = xsec * eA;
      vdouble  a(1), D(1); a[0] = Aeff;
      vvdouble A(1); A[0] = a; D[0] = Datum;

      return kit::poissongamma(p, A, D, false, false);
    }
}


double 
kit::poissongamma2(double	p1,
                   double     p2,
                   vdouble&   A1,
                   vdouble&   A2,
                   vdouble&	D,
                   bool returnlog,
                   bool scale)
{
  vdouble  p(2); p[0] = p1; p[1] = p2;
  vvdouble A(2); A[0] = A1; A[1] = A2;
  return poissongamma(p, A, D, returnlog, scale);
}

double 
kit::poissongamma5(double	p1,
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
                   bool returnlog,
                   bool scale)
{
  vdouble  p(5); p[0] = p1; p[1] = p2; p[2] = p3; p[3] = p4; p[4] = p5;
  vvdouble A(5); A[0] = A1; A[1] = A2; A[2] = A3; A[3] = A4; A[4] = A5;
  return kit::poissongamma(p, A, D, returnlog, scale);
}


//-----------------------------------------------------------------------------
// Compute Poisson X gamma density by Monte Carlo integration
//-----------------------------------------------------------------------------
//extern "C"
//{
//  float rngama_(float& p);
//}

static int TOTAL=0;
static vector<double> YIELD;

void 
kit::initpoissongammamc(vvdouble& a, // Counts  "A_ji" 
                        int total)   // Number of MC sampling points
{
  using namespace ROOT::Math;
  int N = a.size();    // Number of sources (N)
  int M = a[0].size(); // Number of bins    (M)
  TOTAL = total;
  YIELD.clear();
  YIELD.reserve(N*M*TOTAL);
  //Random<GSLRngMT> gslRan;

  for ( int i = 0; i < M; i++ )
    for (int k = 0; k < TOTAL; k++)
      for ( int j = 0; j < N; j++ )
        {
          //double q = (double)(a[j][i] + 1);
          //YIELD.push_back(gslRan.Gamma(_y+0.5, 1.0/_b));
        }
}

double 
kit::poissongammamc(vdouble&	p,  // Weights "p_j" 
                    vvdouble&	a,  // Counts  "A_ji"
                    vdouble&	d,  // Counts  "D_i" for data.
                    bool returnlog,
                    bool scale)   // Scale    p_j if true  
{
  int N = p.size(); // Number of sources (N)
  int M = d.size(); // Number of bins    (M)
  
  if ( a.size() != (unsigned int)N )
    {
      std::cout << "**Error - poissongamma - mis-match in number of sources\n"
                << "size(p): " << N << " differs from size(A) = " << a.size()
                << std::endl;
      exit(0);
    }

  if ( a[0].size() != (unsigned int)M )
    {
      std::cout << "**Error - poissongamma - mis-match in number of sources\n"
                << "size(d): " << M << " differs from size(A[0]) = " 
                << a[0].size()
                << std::endl;
      exit(0);
    }
  
  if ( M < 1 ) return -1.0;
  if ( ( N < 2 ) || ( N > MAXSRC ) ) return -2.0;
  
  if (YIELD.size() == 0) kit::initpoissongammamc(a);

  // Get total counts per source
  for ( int j = 0; j < N; j++ )
    {
      ns[j] = 0;
      for ( int i = 0; i < M; i++ ) ns[j] += a[j][i];
    }
  
  // Scale counts
  
  for ( int j = 0; j < N; j++ )
    {
      if ( scale )
        x[j] = p[j] / (ns[j]+M);
      else
        x[j] = p[j];
    }
  
  // Loop over each point in the swarm of points
  // that represents the prior
  
  int    index = 0;
  double prob;
  if ( returnlog )
    prob = 0.0;
  else
    prob = 1.0;
  for ( int i = 0; i < M; i++ )
    {
      double P = 0;
      for (int k = 0; k < TOTAL; k++)
        {
          // Compute Poisson parameter for ith bin
          double a = 0;
          for ( int j = 0; j < N; j++ )
            { 
              a += x[j] * YIELD[index];
              index++;
            }

          // Compute Poisson probability
          double q = exp(-a);
          int D = (int)d[i];
          if ( D > 0 ) for ( int n = 1; n <= D; n++ ) q *= a / n;

          P += q;
        }
      P /= TOTAL;
      if ( returnlog )
        prob += log(P);
      else
        prob *= P;
    }
  return prob;
}

