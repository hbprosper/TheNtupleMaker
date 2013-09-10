#!/usr/bin/env python
# -----------------------------------------------------------------------------
#  File:        runrgs.py
#  Description: Run Random Grid Search to cuts that best separate signal from
#               background.
# -----------------------------------------------------------------------------
#  Created:     28-Apr-2009 Harrison B. Prosper
# -----------------------------------------------------------------------------
import os, sys, re
from ROOT import *
from string import *
# -----------------------------------------------------------------------------
def error(message):
	print "** %s" % message
	exit(0)
# -----------------------------------------------------------------------------
# Check that all files that are needed are present
VARFILE = "rgs.vars"
if not os.path.exists(VARFILE):
	error("unable to open variables file %s" % VARFILE)

CUTSFILE = "ttbar.dat"
if not os.path.exists(CUTSFILE):
	error("unable to open cuts file %s" % CUTSFILE)

SIGFILE = "ttbar.dat"
if not os.path.exists(SIGFILE):
	error("unable to open signal file %s" % SIGFILE)

BKGFILE = "qcd.dat"
if not os.path.exists(BKGFILE):
	error("unable to open background file %s" % BKGFILE)
# -------------------------------------------
def formatHist(h, color):
	NDIVX =-505
	NDIVY =-505
	MARKERSIZE=1.0

	h.SetMarkerSize(MARKERSIZE)
	h.SetMarkerStyle(20)
	h.SetMarkerColor(color)
    
	h.GetXaxis().CenterTitle()
	h.GetXaxis().SetTitle("efficiency (background)")
	h.GetXaxis().SetTitleOffset(1.2)
	h.GetXaxis().SetNdivisions(NDIVX)
    
	h.GetYaxis().CenterTitle()
	h.GetYaxis().SetTitle("efficiency (signal)")
	h.GetYaxis().SetTitleOffset(1.8)
	h.GetYaxis().SetNdivisions(NDIVY)
# -------------------------------------------
def efflimits(x):
	x1 = sum(x)/len(x)
	x2 = sum(map(lambda z: z*z, x))/len(x)
	x2 = sqrt(x2-x1*x1)
	xmax = x1 + x2
	k = int(xmax/0.05)
	xmax = k*0.05
	return (0.0, xmax)
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
def main():
	print "\n\t\t=== Random Grid Search ===\n"
	
	# -------------------------------------------------------------------------
	#  Load RGS class and style file
	# -------------------------------------------------------------------------
	gROOT.ProcessLine(".L RGS.cc+")
	gROOT.ProcessLine(".x style.C")

	# -------------------------------------------------------------------------
	#  Create RGS object
	# 
	#  Needed:
	#   A file of cut-points - usually a signal file, which ideally is
	#  not the same as the signal file on which the RGS algorithm is run.
	# -------------------------------------------------------------------------
	start = 0    
	maxcuts = 10000 #  maximum number of cut-points to consider

	print "\t==> create RGS object"
	rgs = RGS(CUTSFILE, start, maxcuts)
	
	# -------------------------------------------------------------------------
	#  Add signal and background data to RGS object
	#  Weight each event using the value in field "weight", if it exists. 
	# -------------------------------------------------------------------------
	numrows = 0 #  Load all the data from the files
	
	print "\t==> load background data"
	rgs.add(BKGFILE, start, numrows, "Weight")
	print
	
	print "\t==> load signal data"
	rgs.add(SIGFILE, start, numrows, "Weight")
	print
	
	# -------------------------------------------------------------------------
	#  Run!
	# -------------------------------------------------------------------------
	rgs.run(VARFILE)


	#  Save results to a root file
	rfile = rgs.save("rgs.root")
	
	# -------------------------------------------------------------------------
	#  Plot results
	# -------------------------------------------------------------------------
	crgs = TCanvas("fig_rgs", "RGS Distribution", 50, 50, 500, 500)

	nbins= 100
	bmax = 0.05
	smax = 0.20

	nh = 5
	hrgs = [0]*5
	color= [kBlue, kGreen, kYellow, kRed, kBlack]
	for i in xrange(nh):
		hname = "hrgs%d" % i
		hrgs[i] = TH2F(hname, "", nbins, 0, bmax, nbins, 0, smax)
		formatHist(hrgs[i], color[i])

	#  Plot

	btotal = rgs.total(0)   #  summed background weights
	stotal = rgs.total(1)   #  summed signal weights
	big = -1
	k = -1
	for i in xrange(maxcuts):
		b = rgs.count(0, i) #  background count after the ith cut-point
		s = rgs.count(1, i) #  signal count after the ith cut-point
		eb= b / btotal      #  background efficiency
		es= s / stotal      #  signal efficiency

		#  Here we can compute our signal significance measure
		#  We'll use the rather low-brow s/sqrt(b) measure!

		signif = 0.0
		if b > 0:  signif = s / sqrt(b)

		#  Plot es vs eb

		if signif < 2:
			hrgs[0].Fill(eb, es)
		elif signif < 5:
			hrgs[1].Fill(eb, es)
		elif signif < 8:
			hrgs[2].Fill(eb, es)
		elif signif < 11:
			hrgs[3].Fill(eb, es)
		else:
			hrgs[4].Fill(eb, es)

		if signif > big:
			k = i
			big = signif
			
		if i % 100 == 0:
			crgs.cd()
			hrgs[0].Draw()
			for j in xrange(1, nh): hrgs[j].Draw("same")
			crgs.Update()
			
	#  Get best cuts

	b = rgs.count(0, k) #  background count after the ith cut-point
	s = rgs.count(1, k) #  signal count after the ith cut-point
	eb= b / btotal      #  background efficiency
	es= s / stotal      #  signal efficiency
	signif = s / sqrt(b)

## 	out = open("bestcuts.txt", "w")
## 	record =\
## 		   "record: %d\n"\
## 		   "\tsignal:     %10.1f\n"\
## 		   "\tbackground: %10.1f\n"\
## 		   "\ts/sqrt(b):  %10.1f\n"\
## 		   "\teff_s:      %10.3f\n"\
## 		   "\teff_b:      %10.3f\n" % (k+2, s, b, signif, es, eb)
## 	print record
## 	out.write(record)

## 	cut = rgs.cuts(k)

## 	for i in xrange(cutvar.size()):
## 		record = "\t%-10s\t%s\t%10.1f" % (cutvar[i],
## 										  cutdir[i],
## 										  cut[i])
## 		print record
## 		out.write("%s\n" % record)
## 	out.close()

	crgs.cd()
	hrgs[0].Draw()
	for j in xrange(nh):
		hrgs[j].Draw("same")
		hrgs[j].Write("", TObject.kOverwrite)
	crgs.Update()
	crgs.SaveAs(".gif")
	crgs.Write()
	gSystem.Sleep(5000)
# -----------------------------------------------------------------------------
main()



