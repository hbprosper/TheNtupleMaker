//-----------------------------------------------------------------------------
// File:        example.C
// Description: Example of use of Random Grid Search algorithm to separate 
//              ttbar from light-jet QCD.
//-----------------------------------------------------------------------------
// Created:     28-Apr-2009 Harrison B. Prosper
//-----------------------------------------------------------------------------
#include <vector>
//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------
void 
error(string message)
{
  cout << "** " << message << endl;
  exit(0);
}

void formatHist(TH2F* h, int color)
{
  int NDIVX =-505;
  int NDIVY =-505;
  int MARKERSIZE=1.0;

  h->SetMarkerSize(MARKERSIZE);
  h->SetMarkerStyle(20);
  h->SetMarkerColor(color);
    
  h->GetXaxis()->CenterTitle();
  h->GetXaxis()->SetTitle("efficiency (QCD)");
  h->GetXaxis()->SetTitleOffset(1.2);
  h->GetXaxis()->SetNdivisions(NDIVX);
    
  h->GetYaxis()->CenterTitle();
  h->GetYaxis()->SetTitle("efficiency (t#bar{t})");
  h->GetYaxis()->SetTitleOffset(1.8);
  h->GetYaxis()->SetNdivisions(NDIVY);
}

//-----------------------------------------------------------------------------
// MAIN MACRO
//-----------------------------------------------------------------------------
void 
example()
{
  //---------------------------------------------------------------------------
  // Load RGS class and style file
  //---------------------------------------------------------------------------
  gROOT->ProcessLine(".L RGS.cc+");
  gROOT->ProcessLine(".x style.C");

  //---------------------------------------------------------------------------
  // Create RGS object
  //
  // Provide a file of cut-points - usually a signal file, which ideally is
  // not the same as the signal file on which the RGS algorithm is run.
  // But in this example, the cutfile and signal file are the same! But
  // we use only the first 10000 events as the cut-points.
  //---------------------------------------------------------------------------
  int start = 0;    
  int maxcuts = 10000;       // maximum number of cut-points to consider
  string cutfile("ttbar.dat");
  cout << "Create RGS object" << endl;
  RGS rgs(cutfile, start, maxcuts);

  //---------------------------------------------------------------------------
  // Add signal and background data to RGS object
  // Weight each event using the value in field "Weight", if it exists. 
  //---------------------------------------------------------------------------
  int numrows = 0; // Load all the data from the files

  cout << "Load background data" << endl;
  rgs.add("qcd.dat", start, numrows, "Weight");

  cout << "Load signal data" << endl;
  rgs.add("ttbar.dat", start, numrows, "Weight");

  //---------------------------------------------------------------------------
  // Run!
  //---------------------------------------------------------------------------
  rgs.run("rgs.vars");

  // Save results to a root file
  TFile* rfile = rgs.save("rgs.root");

  exit(0);

  //---------------------------------------------------------------------------
  // Plot results
  //---------------------------------------------------------------------------
  TCanvas* crgs = new TCanvas("fig_rgs", 
                              "RGS Distribution", 
                              50, 50, 500, 500);

  int nbins = 100;
  double bmax = 0.05;
  double smax = 0.25;

  int nh = 5;
  TH2F* hrgs[5];
  int  color[5] = {kBlue, kGreen, kYellow, kRed, kBlack};
  for(int i=0; i < nh; i++)
    {
      char hname[80];
      sprintf(hname, "hrgs%d", i);
      hrgs[i] = new TH2F(hname, "", nbins, 0, bmax, nbins, 0, smax);
      formatHist(hrgs[i], color[i]);
    }

  // Plot

  double btotal = rgs.total(0);   // summed background weights
  double stotal = rgs.total(1);   // summed signal weights
  double big = -1;
  int k = -1;
  for(int i=0; i < maxcuts; i++)
    {
      double b = rgs.count(0, i); // background count after the ith cut-point
      double s = rgs.count(1, i); // signal count after the ith cut-point
      double eb= b / btotal;      // background efficiency
      double es= s / stotal;      // signal efficiency

      // Here we can compute our signal significance measure
      // We'll use the rather low-brow s/sqrt(b) measure!

      double signif = 0.0;
      if ( b > 0 ) signif = s / sqrt(b);

      // Plot es vs eb

      if ( signif < 2 )
        hrgs[0]->Fill(eb, es);
      else if ( signif < 5 )
        hrgs[1]->Fill(eb, es);
      else if ( signif < 8 )
        hrgs[2]->Fill(eb, es);
      else if ( signif < 11 )
        hrgs[3]->Fill(eb, es);
      else
        hrgs[4]->Fill(eb, es);

      if ( signif > big )
        {
          k = i;
          big = signif;
        }
      if (i % 100 == 0)
        {
          crgs->cd();
          hrgs[0]->Draw();
          for(int j=1; j < nh; j++) hrgs[j]->Draw("same");
          crgs->Update();
        }
    }

  // Get best cuts

//   double b = rgs.count(0, k); // background count after the ith cut-point
//   double s = rgs.count(1, k); // signal count after the ith cut-point
//   double eb= b / btotal;      // background efficiency
//   double es= s / stotal;      // signal efficiency
//   double signif = s / sqrt(b);

//   ofstream out("bestcuts.txt");
//   char record[1024];
//   sprintf(record,
//           "record: %d\n"
//           "\tsignal:     %10.1f\n"
//           "\tbackground: %10.1f\n"
//           "\ts/sqrt(b):  %10.1f\n"
//           "\teff_s:      %10.3f\n"
//           "\teff_b:      %10.3f\n",
//           k+2, s, b, signif, es, eb);
//   cout << record << endl;
//   out << record << endl;

//   vector<double> cut = rgs.cuts(k);

//   for(int i=0; i < (unsigned)cutvar.size(); i++)
//     {
//       sprintf(record,
//               "\t%-10s\t%s\t%10.1f\n", 
//               cutvar[i].c_str(),
//               cutdir[i].c_str(),
//               cut[i]);
//       cout << record;
//       out << record;
//     }
//   out.close();

  crgs->cd();
  hrgs[0]->Draw();
  for(int j=1; j < nh; j++)
    {
      hrgs[j]->Draw("same");
      hrgs[j]->Write("", TObject::kOverwrite);
    }
  crgs->Update();
  crgs->SaveAs(".gif");
  crgs->Write();
  gSystem->Sleep(5000);
  exit(0);
}


