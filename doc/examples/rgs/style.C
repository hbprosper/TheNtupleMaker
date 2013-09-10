void style()
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

  gROOT->ForceStyle();
}
