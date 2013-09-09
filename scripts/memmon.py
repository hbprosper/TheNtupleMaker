#!/usr/bin/env python
#------------------------------------------------------------------------------
# File: memmon.py
# A memory monitor
# Created May 20 2011 Harrison B. Prosper
#$Id: memmon.py,v 1.1 2011/05/23 14:32:07 prosper Exp $
#------------------------------------------------------------------------------
import os, sys, re
from string import *
from time import sleep
from ROOT import *
#------------------------------------------------------------------------------
def main():
	
	print "\n\t== Memory Monitor ==\n"
	argv = sys.argv[1:]
	if len(argv) == 0:
		print '''
	Usage:
		  memmon.py <program-name> [ylim=50]

		  example:

		  memmon.py cmsRun
		'''
		sys.exit(0)

	program = argv[0]
	if len(argv) > 1:
		ylim = atof(argv[1])
	else:
		ylim = 50.0

	lastprog = ""
	dchange = 0.0
	vdiff = 0
	text = "\r%8s %8s %-10s %10s %10s %10s  %10s(%s)" % \
		   ("START", "TIME", "PROGRAM", "VSIZE", 
			"RSS", "VSIZEDIFF", "dVSIZE" , "%")
	print text


	gStyle.SetFrameBorderMode(0)
	gStyle.SetCanvasBorderSize(0)
	gStyle.SetCanvasBorderMode(0)
	gStyle.SetCanvasColor(0)
	gStyle.SetPadBorderMode(0)
	gStyle.SetPadColor(0)
	gStyle.SetLabelFont(62, "XYZ");
	gStyle.SetLabelOffset(0.007, "XYZ");
	gStyle.SetLabelSize(0.08, "XYZ");
	
	gStyle.cd()

	canvas = TCanvas("memmon", "%s Memory Monitor" % program, 0, 10, 800, 200)
	canvas.SetGridx()
	canvas.SetGridy()

	graph  = TGraph()
	graph.SetMinimum(-ylim)
	graph.SetMaximum(ylim)
	graph.SetLineColor(kBlue+2)
	graph.SetLineWidth(2)
	graph.SetPoint(1,0,-ylim)
	graph.SetPoint(2,0,+ylim)
	graph.SetPoint(3,0, 0)
	canvas.cd()
	graph.Draw("AL")
	canvas.Update()

	tick = 0
	while 1:
		t = split(os.popen('ps aux | grep "%s"' % program).read())
		if t == []:
			sleep(1)
			continue

		t1 = t[8]
		t2 = t[9]
		vmem = atoi(t[4])
		RSS  = atoi(t[5])
		prog = t[10]
		if prog == "grep":
			sleep(1)
			continue

		# start ticks when cmsRun begins
		if lastprog != program: tick = 0
		lastprog = prog

		if prog == program: 
			tick += 1
			if tick == 1:
				del graph
				graph  = TGraph()
				graph.SetMinimum(-ylim)
				graph.SetMaximum(ylim)
				graph.SetLineColor(kBlue+2)
				graph.SetLineWidth(2)
				graph.SetPoint(1,0,-ylim)
				graph.SetPoint(2,0,+ylim)
				graph.SetPoint(3,0, 0)

				canvas.cd()
				canvas.Clear()
				canvas.SetGridx()
				canvas.SetGridy()
				canvas.Update()
				graph.Draw("AL")
				canvas.Update()

			elif tick == 10:
				vmemstart = vmem
			elif tick > 10:
				vdiff = vmem - vmemstart
				dchange = 100*float(vdiff)/vmemstart
				graph.SetPoint(tick, tick, dchange)
				canvas.cd()
				graph.Draw("L")
				canvas.Update()

		else:
			mean = 0.0
			n = 0
			dchange = 0.0

		last = vmem
		text = "\r%8s %8s %-10s %10d %10d %10d  %10.4f" % \
			   (t1, t2, prog, vmem, RSS, vdiff, dchange)
		sys.stdout.write(text)
		sys.stdout.flush()

		sleep(1)
#------------------------------------------------------------------------------
try:
	main()
except KeyboardInterrupt:
	print "bye!"
