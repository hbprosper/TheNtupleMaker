#!/usr/bin/env python
#------------------------------------------------------------------------------
#- File: pgammafit.py
#- Description: Example of use of PoissonGammaFit
#               Fit a mixture of QCD (QCD+Z), W+jets, top (tt, tqb, tb, tW)
#               to (MC) data. The data for the sources are specified as
#               2-d histograms as are the data. The example is the expectation,
#               after cuts, for an integrated luminosity of 10/pb. The MC
#               data were provided by Lukas V. Use HistogramCache to retrieve
#               histograms.
#- Created: 20-Sep-2010 Harrison B. Prosper
#           11-Feb-2011 HBP - remove dependence of kit.h
#$Revision: 1.5 $
#------------------------------------------------------------------------------
from string import *
from ROOT import *
from time import sleep
import os, sys, re
#------------------------------------------------------------------------------
sources = '''
QCD
W
top
'''
# Histogram names
sources = split(strip(sources))
hname   = map(lambda x: "h%s" % x, sources)
hnameD  = "hdata"
getcount= re.compile(r'[a-zA-Z]+ +[0-9.]+')
TEXTFONT=42
vdouble = vector("double")
#------------------------------------------------------------------------------
# Some Root utilities
#------------------------------------------------------------------------------
CSCALE = 1
ESCALE = 1
def unfoldContents(h):
	c = vdouble()
	e = vdouble()
	for i in range(h.GetNbinsX()):
		binx = i+1
		for j in range(h.GetNbinsY()):
			biny = j+1
			c.push_back( h.GetBinContent(binx, biny) * CSCALE )
			e.push_back( h.GetBinError(binx, biny) * CSCALE/ESCALE )
	return (c, e)

def setContents(h, c):
	for i in range(h.GetNbinsX()):
		bin =  i + 1
		h.SetBinContent(bin, c[i])

def histogram(hname, xtitle, ytitle, nbins, xmin, xmax):
	h = TH1F(hname, "", nbins, xmin, xmax)

	# Set some reasonable defaults

	h.SetLineColor(kBlue)
	h.SetMarkerSize(1)
	h.SetMarkerColor(kRed)
	h.SetMarkerStyle(20)
	
	h.GetXaxis().CenterTitle()
	h.GetXaxis().SetTitle(xtitle)
	h.GetXaxis().SetTitleOffset(1.3)
	h.SetNdivisions(510, "X")
	h.SetMarkerSize(1.0)
	
	h.GetYaxis().CenterTitle()
	h.GetYaxis().SetTitle(ytitle)
	h.GetYaxis().SetTitleOffset(1.4)
	h.SetNdivisions(504, "Y")
	return h

def setStyle():
	style = TStyle("CMSstyle","CMS Style")
	style.SetPalette(1)
	
	#  For the canvas:
	style.SetCanvasBorderMode(0)
	style.SetCanvasColor(kWhite)
	style.SetCanvasDefH(500) # Height of canvas
	style.SetCanvasDefW(500) # Width of canvas
	style.SetCanvasDefX(0)   # Position on screen
	style.SetCanvasDefY(0)
	
	#  For the Pad:
	style.SetPadBorderMode(0)
	style.SetPadColor(kWhite)
	style.SetPadGridX(kFALSE)
	style.SetPadGridY(kFALSE)
	style.SetGridColor(kGreen)
	style.SetGridStyle(3)
	style.SetGridWidth(1)
    
	#  For the frame:
	style.SetFrameBorderMode(0)
	style.SetFrameBorderSize(1)
	style.SetFrameFillColor(0)
	style.SetFrameFillStyle(0)
	style.SetFrameLineColor(1)
	style.SetFrameLineStyle(1)
	style.SetFrameLineWidth(1)
  
	#  For the histo:
	style.SetHistLineColor(1)
	style.SetHistLineStyle(0)
	style.SetHistLineWidth(2)

	style.SetEndErrorSize(2)
	style.SetErrorX(0.)
    
	style.SetMarkerSize(0.5)
	style.SetMarkerStyle(20)
	
	# For the fit/function:
	style.SetOptFit(1)
	style.SetFitFormat("5.4g")
	style.SetFuncColor(2)
	style.SetFuncStyle(1)
	style.SetFuncWidth(1)
	
	# For the date:
	style.SetOptDate(0)

	#  For the statistics box:
	style.SetOptFile(0)
	style.SetOptStat("")
	style.SetStatColor(kWhite)
	style.SetStatFont(TEXTFONT)
	style.SetStatFontSize(0.03)
	style.SetStatTextColor(1)
	style.SetStatFormat("6.4g")
	style.SetStatBorderSize(1)
	style.SetStatH(0.2)
	style.SetStatW(0.3)
    
	#  Margins:
	style.SetPadTopMargin(0.05)
	style.SetPadBottomMargin(0.16)
	style.SetPadLeftMargin(0.20)
	style.SetPadRightMargin(0.10)
	
	#  For the Global title:
	style.SetOptTitle(0) 
	style.SetTitleFont(TEXTFONT)
	style.SetTitleColor(1)
	style.SetTitleTextColor(1)
	style.SetTitleFillColor(10)
	style.SetTitleFontSize(0.05)
	
	#  For the axis titles:
	style.SetTitleColor(1, "XYZ")
	style.SetTitleFont(TEXTFONT, "XYZ")
	style.SetTitleSize(0.05, "XYZ")
	style.SetTitleXOffset(1.25)    # (0.9)
	style.SetTitleYOffset(1.45)    # (1.25)

	#  For the axis labels:
	style.SetLabelColor(1, "XYZ")
	style.SetLabelFont(TEXTFONT, "XYZ")
	style.SetLabelOffset(0.007, "XYZ")
	style.SetLabelSize(0.05, "XYZ")
	
	#  For the axis:
	style.SetAxisColor(1, "XYZ")
	style.SetStripDecimals(kTRUE)
	style.SetTickLength(0.03, "XYZ")
	style.SetNdivisions(510, "XYZ")
	#  To get tick marks on the opposite side of the frame
	style.SetPadTickX(1)  
	style.SetPadTickY(1)
	
	#  Change for log plots:
	style.SetOptLogx(0)
	style.SetOptLogy(0)
	style.SetOptLogz(0)
	
	#  Postscript options:
	style.SetPaperSize(20.,20.)
	style.cd()
	return style
#------------------------------------------------------------------------------
def main():

	# Load classes

	gROOT.ProcessLine(".L PoissonGammaFit.cc+")
	gROOT.ProcessLine(".L HistogramCache.cc+")
	
	# Read expected counts from the counts.txt file and place them in a map
	# This ugly code is an example of how to do a lot in one line! Not
	# really recommended!
	
	counts = map(lambda x: (x[0], atof(x[1])),
				 map(split, getcount.findall(open("counts.txt").read())))
	count = {}
	for key, value in counts: count[key] = value

	# Set some standard style and create a canvas

	setStyle()
 	canvas = TCanvas("fig_fitresults",
					 "m_T vs b_tag", 
					 50, 50, 500, 500)

	# Read histograms into memory
	
	filename = "histograms.root"
	hcache   = HistogramCache(filename)

	# Get data histogram and plot it
	
	hdata = hcache.histogram(hnameD)
	canvas.cd()
	hdata.Draw("lego2")
	canvas.Update()
	sleep(0.5)
	
	# Get data distribution. Since it is 2-d,
	# unfold into a 1-d vector, D, create a 1-d histogram and plot it

	# 1. Unfold 2-d histogram into a 1-D vector, D
	D, dD = unfoldContents(hdata)

	# 2. Create 1-d histogram
	h1d = []
	h1d.append( histogram("hdata1d", "bin", "", len(D), 0, len(D)) )
	setContents(h1d[-1], D)

	# 3. Plot it
	canvas.cd()
	h1d[-1].Draw("EP")
	canvas.Update()
	sleep(0.5)
				
	# Get distribution of sources, also unfold into 1-d vectors, A[j]

	h = []
	A = [] # vector<vector<double> >
	dA= [] # vector<vector<double> >
	for i, source in enumerate(sources):
		h.append( hcache.histogram(hname[i]) ) # get histogram
		canvas.cd()
		h[-1].Draw("lego2")
		canvas.Update()
		sleep(0.5)

		A.append(unfoldContents(h[-1]))

		h1d.append( histogram("%s1d" % hname[i],
							  hname[i], "", len(D), 0, len(D)) )
		setContents(h1d[-1], A[i][0])
		h1d[-1].SetFillColor(i+1)
		canvas.cd()
		h1d[-1].Draw()
		canvas.Update()
		sleep(0.5)

	# Construct a PoissonGamma object and fit...
	
	total = sum(D)
	print "total data count: %d" % total
	
	print "Fit..."
	pgfit = PoissonGammaFit(D)
	for a, da in A:
		pgfit.add(a, False, da) # add a source, but with a floating yield
										  
	# do fit
	guess = vdouble(len(sources), total/3)
	pgfit.execute(guess)
	
	if not pgfit.good():
		print "Fit failed"
		sys.exit(0)

	# Get mode and width of posterior density, which, because PoissonGammaFit
	# uses a flat prior for the yields, are the same as those of the
	# Poisson/gamma marginal density. 
	
	mode  = pgfit.mode()
	error = pgfit.width()
	logevidence = pgfit.logEvidence()

	# Print results
	
	print
	print "%-10s %10s\t%10s    %10s" %  \
		  ('source', "true count", " estimated count", "")
	for index, s in enumerate(sources):
		print "%-10s %10.f\t%10.0f +/-%-10.0f" % \
			  (s, count[s], mode[index], error[index])
#------------------------------------------------------------------------------
main()

