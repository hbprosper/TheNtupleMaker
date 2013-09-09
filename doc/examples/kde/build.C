//-----------------------------------------------------------------------------
// File:        build.C
// Description: Model a 3-D density
// Created:     01-Nov-2009 Harrison B. Prosper, Panjab University, Chandigarh
//-----------------------------------------------------------------------------
void 
build()
{
  gROOT->Reset();

  cout  << "\n\n\t\t*** build KDE ***\n" << endl;
 
  // Load utilities
  gROOT->ProcessLine(".L util.C");

  setStyle();

  //------------------------------------
  // Get data
  //------------------------------------
  vector<double> pt;
  vector<double> eta;
  vector<double> phi;
  loaddata("jetdata.dat", 20000, pt, eta, phi);

  // Divide data into half: half to build KDE,
  // half to test it
  int Npoint = pt.size()/2;
  
  //------------------------------------
  // Book histograms
  // NB: The names of the histograms
  // should match (exactly) the names
  // of the variables passed to the
  // KDE object. See below.
  //------------------------------------
  int NBIN=50;

  TCanvas* cpt  = new TCanvas("fig_pt",  "Pt",    0,  10, 500, 500);
  TH1F* hpt  = bkhist("pt", "p_{T}", "", NBIN,  0, 100, kBlack, 1, kBlue);

  TCanvas* ceta = new TCanvas("fig_eta", "Eta", 400,  10, 500, 500);
  TH1F* heta = bkhist("eta", "#eta", "", NBIN, -4, 4, kBlack, 1, kBlue);

  TCanvas* cphi = new TCanvas("fig_phi", "Phi", 800,  10, 500, 500);
  TH1F* hphi = bkhist("phi", "#phi", "", NBIN, 0, 8, kBlack, 1, kBlue);

  //------------------------------------
  // Fill histograms
  // Use the second half of the data
  //------------------------------------
  for(int i=Npoint; i < (int)pt.size(); i++)
    {
      hpt->Fill(pt[i]);
      heta->Fill(eta[i]);
      hphi->Fill(phi[i]);

      if ( i % 100 == 0 )
        {
          cpt->cd();
          hpt->Draw("EP");
          cpt->Update();

          ceta->cd();
          heta->Draw("EP");
          ceta->Update();

          cphi->cd();
          hphi->Draw("EP");
          cphi->Update();
        }
    }

  //------------------------------------
  // Construct KDE using 1st half of
  // data.
  //------------------------------------
  cout << endl << "Constructing KDE with " << Npoint << " points" << endl;

  int ndim = 3;
  KDE kde("pt eta phi");

  vector<double> point(ndim);

  for (int i=0; i < Npoint; i++)
    {
      point[0] = pt[i];
      point[1] = eta[i];
      point[2] = phi[i];
      kde.add(point);
      if ( i % 100 == 0 ) cout << i << endl;
    }

  plotkde(kde, "p_{T}", cpt,   hpt);
  plotkde(kde, "#eta",  ceta,  heta);
  plotkde(kde, "#phi",  cphi,  hphi);

  //------------------------------------
  // Try to improve bandwidths
  //------------------------------------
  cout << "optimize" << endl;

  kde.optimize();

  plotkde(kde, "p_{T}", cpt,   hpt, "_opt");
  plotkde(kde, "#eta",  ceta,  heta, "_opt");
  plotkde(kde, "#phi",  cphi,  hphi, "_opt");

  //------------------------------------
  // Write a self-contained c++ function 
  // that encapsulates the KDE that has 
  // just been created
  //------------------------------------
  kde.write("jetdata");
}

void
plotkde(KDE& kde, string xtitle, TCanvas* c, TH1F* h, string opt="")
{
  //------------------------------------
  // Compute KDE at centers of bins, 
  // then overlay on histogram
  //------------------------------------
  double x[1000],  y[1000];
  
  int nbins = h->GetNbinsX();
  string name(h->GetName());
  cout << "Histogram: " << name << "\twith " << nbins << " bins" << endl;

  for(int j=0; j < nbins; j++)
    {
      int bin=j+1;
      double width;
      double norm;

      width = h->GetBinWidth(bin);
      norm  = h->Integral() * width;
      x[j]  = h->GetBinLowEdge(bin) + 0.5*width;
      y[j]  = norm * kde(x[j], name);
    }

  TGraph* g  = bkgraph(nbins, x,  y,  xtitle, "", 
                       h->GetXaxis()->GetBinLowEdge(1), 
                       h->GetXaxis()->GetBinUpEdge(nbins), 
                       kRed);
  c->cd();
  h->Draw("EP");
  g->Draw("L same");
  c->Update();

  string name = string(c->GetName())+opt+string(".gif");
  c->SaveAs(name.c_str());
}
