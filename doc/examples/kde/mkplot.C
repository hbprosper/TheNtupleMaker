void 
mkplot()
{
  gROOT->ProcessLine(".L jetdata.cpp+");
  gROOT->ProcessLine(".L util.C");
  gROOT->ProcessLine(".L plot.C");
  plot();
}
