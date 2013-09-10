//-----------------------------------------------------------------------------
// File:        util.C
// Description: Some simple stuff
// Created:     01-Nov-2009 Harrison B. Prosper, Panjab University, Chandigarh
//-----------------------------------------------------------------------------
void 
error(string message)
{
  cout << "** " << message << endl;
  exit(0);
}

TH1F* bkhist(string hname, string xtitle, string ytitle,
             int nbin, double xmin, double xmax,
             int color=kBlack, int lstyle=1, int mcolor=kBlue)
{
  TH1F* h = new TH1F(hname.c_str(), "", nbin, xmin, xmax);
  h->SetLineWidth(2);
  h->SetLineColor(color);
  h->SetLineStyle(lstyle);
  h->SetMarkerStyle(20);
  h->SetMarkerColor(mcolor);
  h->SetMarkerSize(1.0);

  h->GetXaxis()->CenterTitle();
  h->GetXaxis()->SetTitle(xtitle.c_str());
  h->GetXaxis()->SetTitleOffset(1.3);
  h->SetNdivisions(504, "X");
  
  h->GetYaxis()->CenterTitle();
  h->GetYaxis()->SetTitle(ytitle.c_str());
  h->GetYaxis()->SetTitleOffset(1.6);
  h->SetNdivisions(504, "Y");
  h->SetMinimum(0);
  return h;
}

TGraph* bkgraph(int npts, double* x, double* y,
                string xtitle, string ytitle,
                double xmin, double xmax,
                int color=kBlack, int lstyle=1)
{
  TGraph* g = new TGraph(npts, x, y);
  g->SetLineWidth(2);
  g->SetLineColor(color);
  g->SetLineStyle(lstyle);

  g->GetXaxis()->CenterTitle();
  g->GetXaxis()->SetTitle(xtitle.c_str());
  g->GetXaxis()->SetTitleOffset(0.3);
  g->GetHistogram()->SetNdivisions(504, "X");
  g->GetHistogram()->SetAxisRange(xmin, xmax, "X");

  g->GetYaxis()->CenterTitle();
  g->GetYaxis()->SetTitle(ytitle.c_str());
  g->GetYaxis()->SetTitleOffset(0.5);
  g->GetHistogram()->SetNdivisions(504, "Y");
  return g;
}

void 
loaddata(string filename,
         int np,
         vector<double>& x,
         vector<double>& y,
         vector<double>& z)
{
  ifstream inpv(filename.c_str());
  if ( ! inpv.good() ) error("unable to open " + filename);
  string npt, neta, nphi, ntagged;
  inpv >> npt >> neta >> nphi >> ntagged;
  
  double pt, eta, phi, tagged;
  
  int count=0;

  x.clear();
  y.clear();
  z.clear();

  count=0;
  while ( inpv >> pt >> eta >> phi >> tagged )
    { 
      x.push_back(pt);
      y.push_back(eta);
      z.push_back(phi);
      count++;
      if ( count >= np ) break;
    }
  inpv.close();
}

void setStyle()
{
  gROOT->SetStyle("Pub");
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
  style->SetTitleFont(42);
  style->SetTitleColor(1);
  style->SetTitleTextColor(1);
  style->SetTitleFillColor(10);
  style->SetTitleFontSize(0.05);

  // For the axis titles:

  style->SetTitleColor(1, "XYZ");
  style->SetTitleFont(42, "XYZ");
  style->SetTitleSize(0.05, "XYZ");
  style->SetTitleXOffset(0.9);
  style->SetTitleYOffset(1.25);

  // For the axis labels:

  style->SetLabelColor(1, "XYZ");
  style->SetLabelFont(42, "XYZ");
  style->SetLabelOffset(0.007, "XYZ");
  style->SetLabelSize(0.05, "XYZ");

  // For the axis:

  style->SetAxisColor(1, "XYZ");
  style->SetStripDecimals(kTRUE);
  style->SetTickLength(0.03, "XYZ");
  style->SetNdivisions(505, "XYZ");
  style->SetPadTickX(1);  
  style->SetPadTickY(1);

  style->cd();
}
