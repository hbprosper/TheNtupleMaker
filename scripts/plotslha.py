#!/usr/bin/env python
#------------------------------------------------------------------------------
# File: plotslha.py
# Description: plot SUSY mass spectrum or decays from an SLHA file
# Created: 22 Sep 2010 Harrison B. Prosper & Sezen Sekmen
#$Id: plotslha.py,v 1.17 2013/07/05 21:01:54 prosper Exp $
#------------------------------------------------------------------------------
import os, sys, re
from ROOT import *
from string import atof, atoi, replace, lower,\
	 upper, joinfields, split, strip, find
from time import sleep, ctime
from getopt import getopt, GetoptError
#------------------------------------------------------------------------------
OPTIONS= 'p:y:t:w:l' # command line options
RED    ="\x1b[0;31;48m"
DCOLOR ="\x1b[0m"	 # reset to default foreground color		 
			 
def usage():
	print '''
Usage
     ./plotslha.py [options] <slha file name>
	 
	 options:

	 -p PDGid             plot decays of given particle [plot masses]
	 -y [ymin:]ymax       range for y-axis              [automatic]
	 -l                   log(y) scale                  [linear]
	 -t "title"           title for plot                [filename]
	 -w canvas-width      canvas width in pixels        [1000]
'''
	sys.exit(0)
#------------------------------------------------------------------------------
LSCALE = 0.0135  # line length relative to x-range
LOGYMIN=-8.0     # minimum log10(y)
WIDTH  = 1000    # Default canvas width
HEIGHT = 500     # Default canvas height

# Font code = 10 * fontnumber + precision with fontnumber 1..14
# fontnumber= 1..14
# precision = 0 fast hardware fonts
# precision = 1 scalable and rotatable hardware fonts
# precision = 2 scalable and rotatable hardware fonts
# precision = 3 scalable and rotatable hardware fonts. Text size in pixels. 

Arial             = 43
FONTSIZE          = 18 # font size in pixels

TimesNewRomanBold = 33
LABEL_FONTSIZE    = 18
FONTSCALE         = 0.0027

TimesNewRoman2    = 12
TITLE_FONTSIZE    = 0.048

# Text alignment = 10 * horizontal + vertical
# For horizontal alignment:
#   1=left adjusted, 2=centered, 3=right adjusted
# For vertical alignment:
#   1=bottom adjusted, 2=centered, 3=top adjusted
ALIGN_LEFT_BOTTOM = 11
#------------------------------------------------------------------------------
PDGID = '''
1
2
3
4
5
6
11
12
13
14
15
16
21
22
23
24
25
35
36
37
39
1000001
2000001
1000002
2000002
1000003
2000003
1000004
2000004
1000005
2000005
1000006
2000006
1000011
2000011
1000012
1000013
2000013
1000014
1000015
2000015
1000016
1000021
1000022
1000023
1000025
1000035
1000024
1000037
'''
PDGID  = map(atoi, split(strip(PDGID)))

IGNORE = [5,
		  24,
		  1000003,
		  2000003,
		  1000004,
		  2000004,
		  1000013,
		  2000013,
		  1000014]
NAMES = {\
    '1'   : ('d',kBlack),
    '2'   : ('u',kBlack),
    '3'   : ('s',kBlack),
    '4'   : ('c',kBlack),
    '5'   : ('b',kBlack),
    '6'   : ('t',kBlack),
	
    '11'   : ('e^{#pm}',kBlack),
    '12'   : ('#nu_{e}',kBlack),
    '13'   : ('#mu^{#pm}',kBlack),
    '14'   : ('#nu_{#mu}',kBlack),
    '15'   : ('#tau^{#pm}',kBlack),
    '16'   : ('#nu_{#tau}',kBlack),
    '21'   : ('g',kBlack),
    '22'   : ('#gamma',kBlack),
    '23'   : ('Z^{0}',kBlack),
    '24'   : ('W^{#pm}',kBlack),
	
	'25'	: ('h',kViolet-6),
	'35'	: ('H',kViolet-6),
	'36'	: ('A',kViolet-6),
	'37'	: ('H^{#pm}',kViolet-6),
	
	'39'    : ('G',kMagenta),
	
	'1000001'	: ('#tilde{d}_{L}',kBlue),
	'2000001'	: ('#tilde{d}_{R}',kBlue),
	'1000002'	: ('#tilde{u}_{L}',kBlue),
	'2000002'	: ('#tilde{u}_{R}',kBlue),
	'1000003'	: ('#tilde{s}_{L}',kBlue),
	'2000003'	: ('#tilde{s}_{R}',kBlue),
	'1000004'	: ('#tilde{c}_{L}',kBlue),
	'2000004'	: ('#tilde{c}_{R}',kBlue),
	'1000005'	: ('#tilde{b}_{1}',kBlue),
	'2000005'	: ('#tilde{b}_{2}',kBlue),
	'1000006'	: ('#tilde{t}_{1}',kBlue),
	'2000006'	: ('#tilde{t}_{2}',kBlue),
	'1000011'	: ('#tilde{e}_{L}',kRed),
	'2000011'	: ('#tilde{e}_{R}',kRed),
	'1000012'	: ('#tilde{#nu}_{e_{L}}',kRed),
	'1000013'	: ('#tilde{#mu}_{L}',kRed),
	'2000013'	: ('#tilde{#mu}_{R}',kRed),
	'1000014'	: ('#tilde{#nu}_{#mu_{L}}',kRed),
	'1000015'	: ('#tilde{#tau}_{1}',kRed),
	'2000015'	: ('#tilde{#tau}_{2}',kRed),
	'1000016'	: ('#tilde{#nu}_{#tau_{L}}',kRed),
	'1000021'	: ('#tilde{g}',kCyan+4),
	'1000022'	: ('#tilde{#chi}_{1}^{0}',kGreen+4),
	'1000023'	: ('#tilde{#chi}_{2}^{0}',kGreen+4),
	'1000025'	: ('#tilde{#chi}_{3}^{0}',kGreen+4),
	'1000035'	: ('#tilde{#chi}_{4}^{0}',kGreen+4),
	'1000024'	: ('#tilde{#chi}_{1}^{#pm}',kGreen+3),
	'1000037'	: ('#tilde{#chi}_{2}^{#pm}',kGreen+3)}
# add integer keys
for key in NAMES.keys():
	NAMES[atoi(key)] = NAMES[key]
#------------------------------------------------------------------------------
def decodeCommandLine():
	argv = sys.argv[1:]
	if len(argv) < 1: usage()

	try:
		options, filenames = getopt(argv, OPTIONS)
	except GetoptError, m:
		print RED, "\n\t** %s" % m, DCOLOR
		usage()
		
	if len(filenames) == 0: usage()
	
	filename = filenames[0]
	if not os.path.exists(filename):
		print "\t*** I can't find \n\t%s" % filename
		sys.exit(0)

	# Check for command line options

	# Set defaults
	pdgid  = None
	title  = filename
	width  = 1000 # pixels
	ymin   = 0.0
	ymax   =-1.0
	setlogy   = False
	automatic = True
	squeeze   = True
	
	for option, value in options:
		if option == '-t':
			title = value
			
		elif option == '-p':
			pdgid = atoi(value)
			
			if not NAMES.has_key(pdgid):
				print "\t*** I'm a cyber ignoramous and don't understand"\
					  "\n\t*** this PDG id: %d" % pdgid
				sys.exit(0)
				
		elif option == '-y':

			automatic = False
			
			try:
				value = replace(value, ",",":") # perhaps a comma was used!
				t = split(value,":")
				if len(t) == 0:
					print "\t*** I can't make sense of ( %s %s )" % \
						  (option, value)
					sys.exit(0)
				if len(t) == 1:
					ymin = 0.0
					ymax = atof(value)
				else:
					ymin = atof(t[0])
					ymax = atof(t[1])

			except:
				print "\t*** I can't make sense of sense of ( %s %s )" % \
					  (option, value)
				sys.exit(0)
				
		elif option == '-l':
			setlogy = True
			
		elif option == '-s':
			squeeze = True
			
		elif option == '-w':
			try:
				width = atoi(value)
			except:
				print "\t*** I can't make sense of width( %s )" % value
				sys.exit(0)

	extras = {'automatic': automatic,
			  'setlogy'  : setlogy,
			  'squeeze'  : squeeze,
			  'pdgid'    : pdgid}
	
	return ((filename, title, width, ymin, ymax), extras)
#-----------------------------------------------------------------------------
def notDisplayed(pdgid, x, y, xmin, xmax, ymin, ymax):
	missed = False
	if x > 0.95 * xmax:
		missed = True
		message = RED +\
				  "** how embarassing, "\
				  "PDGid=%d, y=%e beyond x-max"+\
				  DCOLOR
		print message % (pdgid, y)

	if (y < ymin) or (y > ymax):
		missed = True
		message = RED +\
				  "** oops, "\
				  "PDGid=%d, y=%e out of y-range"+\
				  DCOLOR
		print message % (pdgid, y)
	return missed
#-----------------------------------------------------------------------------
def plotArea(extras, yymin, yymax, ymin, ymax):

	if extras['automatic']:
		print "\tymin and ymax set automatically"
		
		if extras['setlogy']:
			if yymin <= 0:
				yymin = 10**LOGYMIN
				print "\n\t** setting ymin = %e\n" % yymin 
				
			if yymax <= 0:
				print "\n\t** error *** ymax = %f...!!!\n" % yymax
				sys.exit(0)

			yymin1 = log10(yymin/10)
			yymin  = log10(yymin)
			yymax  = log10(yymax)
			yymid  =(yymax + yymin)/2
			yyhalf =(yymax - yymin)*0.65
			yymin= yymid - yyhalf
			if yymin < 0: yymin = yymin1
			yymax= yymid + yyhalf
			ymin = 10**yymin
			ymax = 10**yymax
		else:
			yymidpt =(yymax + yymin)/2
			yyhalfr =(yymax - yymin)*0.65
			yymin   = yymidpt - yyhalfr
			yymax   = yymidpt + yyhalfr
			ymin = max(yymin, 0.0)
			ymax = yymax
	else:
		if extras['setlogy']:
			if ymin <= 0:
				ymin = 10**LOGYMIN
				print "\n\t** setting ymin = %e\n" % ymin 
				
			if ymax <= 0:
				print "\n\t** error *** ymax = %f...!!!\n" % ymax
				sys.exit(0)
				
	print "\tymin: %10.2e" % ymin
	print "\tymax: %10.2e" % ymax
	print
	return (ymin, ymax)
#------------------------------------------------------------------------------
def setStyle():
   gStyle.SetOptTitle(0)
   gStyle.SetOptStat(0)
   
   gStyle.SetPadLeftMargin(0.09)
   gStyle.SetPadRightMargin(0.03)
   gStyle.SetTitleOffset(0.001, "Y")
   
   gStyle.SetFrameBorderMode(0)
   gStyle.SetPadBorderMode(0)
   gStyle.SetCanvasBorderMode(0)
   
   gStyle.SetCanvasColor(kWhite)
   gStyle.SetPadColor(kWhite)
   gStyle.SetFrameFillColor(kWhite)

   gStyle.SetTitleFont(Arial, "XY")
   gStyle.SetLabelFont(Arial, "XY")

   gStyle.SetTitleSize(FONTSIZE, "XY")
   gStyle.SetLabelSize(FONTSIZE, "XY")
#------------------------------------------------------------------------------
def makeHistogram(hname, xtitle, ytitle, nbin, xmin, xmax, ymin, ymax):	
   h = TH1F(hname, "", nbin, xmin, xmax)

   h.GetXaxis().CenterTitle()
   h.GetXaxis().SetTitle("")
   h.GetXaxis().SetTitleOffset(1.3)
   h.SetNdivisions(505, "X")
   
   h.GetYaxis().CenterTitle()
   h.GetYaxis().SetTitle(ytitle)
   h.GetYaxis().SetTitleOffset(0.8)
   h.SetNdivisions(510, "Y")

   h.SetMinimum(ymin)
   h.SetMaximum(ymax)

   for x in xtitle: h.Fill(x, 1)
   return h
#------------------------------------------------------------------------------
def getMassBlock(records, ymin, ymax, extras):
	block = {}
	token = "BLOCK MASS"	
	ltoken= len(token)

	found = False
	minmass = 1.e30
	maxmass =-1.e30
	for index, record in enumerate(records):
		
		# skip any other lines that start with a "#"
		if record[0] == "#": continue
		
		# Loop until we find the block

		record = upper(record) # make case insensitive
		
		if token == record[:ltoken]:
			found = True
			continue
		if not found: continue

		# we are in the block, so look for either:
		# a BLOCK or a DECAY section
		field = split(record)
		if len(field) > 0:
			if field[0] in ["BLOCK", "DECAY"]: break

		# field[0]: PDG id
		# field[1]: mass
		pdgid = atoi(field[0])
		if pdgid in IGNORE: continue
		
		mass  = abs(atof(field[1]))
		
		if not NAMES.has_key(pdgid):
			name = "UNK"
			color= kBlack
		else:
			name, color  = NAMES[pdgid]
		block[pdgid] = (mass, name, color)
		if mass > maxmass: maxmass = mass
		if mass < minmass: minmass = mass

	print "\tMinimum mass: %10.2e" % minmass
	print "\tMaximum mass: %10.2e" % maxmass
	
	ymin, ymax = plotArea(extras, minmass, maxmass, ymin, ymax)

	return (block, ymin, ymax)
#------------------------------------------------------------------------------
def getDecayBlock(records, ymin, ymax, extras):
	block = []
	pdgId = extras['pdgid']
	token = 'DECAY %-d' % pdgId	# left justify number

	minBF = 1.0
	maxBF = 0.0
	found = False
	for index, record in enumerate(records):

		# skip any other lines that start with a "#"
		if record[0] == "#": continue

		# Loop until we find the block
		record = upper(record) # make case insensitive
		field  = split(record)
		if len(field) > 1:
			keyword = '%-s %-s' % (field[0], field[1]) # left justify
		else:
			keyword = ""
			
		if token == keyword:
			print "\t%s" % strip(record)
			found = True

			name, color = NAMES[pdgId]
			block.append(pdgId)
			block.append(name)
			block.append(color)
			block.append([])
			continue
		
		if not found: continue

		# we are in the block, so look either for
		# a BLOCK or a DECAY section

		if len(field) > 0:
			if field[0] in ["BLOCK", "DECAY"]:
				break

		BF  = atof(field[0]) # branching fraction
		if BF <= 0.0:
			print "\n\t** negative or zero BF - ignored!!!!\n"
			print "\t** %s" % strip(record)
			continue
		
		NDA = atoi(field[1]) # number of daughters
		D   = []
		for i in range(NDA):
			pdgid = atoi(field[2+i])
			key = abs(pdgid)
			if not NAMES.has_key( key ):
				name = "UNK"
				color= kBlack
			else:
				# Fix name according to whether the PDG id is
				# positive or negative
				name, color = NAMES[key]
				if key < 7:
					if pdgid < 0:
						name = "#bar{%s}" % name
				elif find(name, "#pm") > -1:
					if key < 16:
						if pdgid > 0:
							name = replace(name, "#pm", "-")
						else:
							name = replace(name, "#pm", "+")
					else:
						if pdgid > 0:
							name = replace(name, "#pm", "+")
						else:
							name = replace(name, "#pm", "-")
				elif key > 1000000:
					if pdgid < 0:
						name = name + "^{*}"						
			D.append((pdgid, name, color))
			
		block[-1].append((BF, D)) # add to last element of block
		if BF > maxBF: maxBF = BF
		if BF < minBF: minBF = BF

	if maxBF < minBF:
		print "\n\t*** No data for PDG id: %d ***\n" % pdgId
		sys.exit(0)

	print "\tMinimum BF: %10.2e" % minBF
	print "\tMaximum BF: %10.2e" % maxBF
	
	ymin, ymax = plotArea(extras, minBF, maxBF, ymin, ymax)

	return (block, ymin, ymax)
#------------------------------------------------------------------------------
def plotMasses(block, output, ymin, ymax, **kargs):
   """ Plot SUSY mass spectrum
   """
   # Decode keyword arguments
   
   if kargs.has_key("title"):
	   title = kargs['title']
   else:
	   title = "SUSY Mass Spectrum"

   if kargs.has_key("width"):
	   width = kargs['width']
   else:
	   width = WIDTH
   height = HEIGHT

   if kargs.has_key("setlogy"):
	   setlogy = kargs['setlogy']
   else:
	   setlogy = False

   if kargs.has_key("squeeze"):
	   squeeze = kargs['squeeze']
   else:
	   squeeze = False	   
	   
   # Define plot area in user coordinates [xmin, xmax] x [ymin, ymax]
   
   xmin = 0.0;
   xmax = 1.0;
   xrange = xmax-xmin
   yrange = ymax-ymin

   # Map font size from pixels to plot coordinates
   
   fsize  = FONTSCALE * LABEL_FONTSIZE * (float(WIDTH) / width) 
   lsize  = 0.18*fsize
   space  = 0.18*fsize   

   # Determine how much x-space we need
   
   xoffset= lsize + space
   items  = []
   for pdgid in PDGID:
	   if not block.has_key(pdgid): continue
	   xoffset += lsize + 0.5*space
	   xoffset += 5*space
	   mass, name, color = block[pdgid]
	   items.append((pdgid, mass, name, color))

  # maybe we need to squeeze things a bit
   if squeeze:
	   xscale = 0.98 * xmax / xoffset
   else:
	   xscale = 1.0
   lsize *= xscale
   space *= xscale

   # Create canvas
   
   c = TCanvas('cm', "SUSY Mass Spectrum", 10, 10, width, height)
   c.SetGridy()
   if setlogy: c.SetLogy()

   # Create histogram

   h = makeHistogram("hm",
					 ['Higges',
					  'Squarks',
					  'Sleptons',
					  'Gauginos'],
					 "mass [GeV/c^{2}]",
					 4, xmin, xmax, ymin, ymax)
 
   # Plot title
   
   ptitle = TLatex()
   ptitle.SetTextAlign(ALIGN_LEFT_BOTTOM)
   ptitle.SetTextFont(TimesNewRoman2)
   ptitle.SetTextSize(TITLE_FONTSIZE)
   ptitle.SetTextColor(kBlack)

   # Lines and labels
   line = TLine()
   line.SetLineWidth(1)

   label = TLatex()
   label.SetTextAlign(ALIGN_LEFT_BOTTOM)
   label.SetTextFont(TimesNewRomanBold)
   label.SetTextSize(LABEL_FONTSIZE)

   # Now plot

   c.cd()
   h.Draw()
   if setlogy:
	   xpos = 0.05 * xrange + xmin
	   ypos = 10**(0.92 * log10(ymax/ymin) + log10(ymin))
   else:
	   xpos = 0.05 * xrange + xmin
	   ypos = 0.92 * yrange
   ptitle.DrawLatex(xpos, ypos, title)
   
   xoffset = lsize + 2*space
   for pdgid, mass, name, colour in items:
	   
	   if notDisplayed(pdgid, xoffset, mass, xmin, xmax, ymin, ymax): continue
		
	   line.SetLineColor( colour )
	   line.DrawLine(xoffset, mass, xoffset + lsize, mass)

	   xoffset += lsize + 0.5*space
	   label.SetTextColor(colour)
	   label.DrawLatex(xoffset, mass, name)
	   xoffset += 5*space

   c.Update()
   c.SaveAs( "%s_spectrum.eps" % output )
   c.SaveAs( "%s_spectrum.gif" % output )
   return (c, h) # needed to prevent plot from vanishing!
#------------------------------------------------------------------------------
def plotDecays(block, output, ymin, ymax, **kargs):
   """ Plot SUSY decays
   """
   # Decode keyword arguments
   
   if kargs.has_key("title"):
	   title = kargs['title']
   else:
	   title = "SUSY Decays"

   if kargs.has_key("width"):
	   width = kargs['width']
   else:
	   width = WIDTH
   height = HEIGHT
   
   if kargs.has_key("setlogy"):
	   setlogy = kargs['setlogy']
   else:
	   setlogy = False

   if kargs.has_key("squeeze"):
	   squeeze = kargs['squeeze']
   else:
	   squeeze = False	   
	   
   # get particle name, color, daughters and branching fraction
   # pname    - name of parent particle
   # pcolor   - display color of parent particle
   # BF       - branching fraction
   # chains   - decay chains: [(pdgid, name, color),...]
   
   pdgId, pname, pcolor, decays = block
   
   xmin = 0.0;
   xmax = 1.0;
   xrange = xmax-xmin
   yrange = ymax-ymin

   # Map font size from pixels to plot coordinates
   
   fsize  = FONTSCALE * LABEL_FONTSIZE * (float(WIDTH) / width) 
   lsize  = 0.18*fsize
   space  = 0.18*fsize

   # Determine how much x-space we need
   
   items  = []
   previousdecay = ""
   xoffset= lsize + 3*space
   for BF, D in decays:
	   # Omit conjugate decays
	   decay = ""
	   conjdecay = ""
	   for id, dname, dcolor in D:
		   decay += "%d" % id
		   conjdecay += "%d" % -id
	   if conjdecay == previousdecay: continue
	   previousdecay = decay

	   items.append((BF, D))
	   xoffset += lsize + space
	   for id, dname, dcolor in D:		   
		   # need a bit more space for boson characters
		   if dname[0] in ['W']: xoffset += 2*space
		   xoffset += 3*space
	   xoffset += space

   # maybe we need to squeeze things a bit
   if squeeze:
	   xscale = 0.98 * xmax / xoffset
   else:
	   xscale = 1.0
   lsize *= xscale
   space *= xscale
   
   # Create canvas
   
   c = TCanvas('cd', "SUSY Decays", 100, 100, width, height)
   c.SetGridy()
   if setlogy: c.SetLogy()
   
   # Create histogram

   h = makeHistogram("hd",
					 ['Thing 1', 
					  'Thing 2',
					  'Thing 3',
					  'Thing 4'],
					 "Branching Fraction",
					 4, xmin, xmax, ymin, ymax)
   canvascolor = gStyle.GetCanvasColor()
   h.GetXaxis().SetLabelColor(canvascolor) # suppress display of x-labels
   
   # Plot title
   
   ptitle = TLatex()
   ptitle.SetTextAlign(ALIGN_LEFT_BOTTOM)
   ptitle.SetTextFont(TimesNewRomanBold)
   ptitle.SetTextColor(pcolor)

   # Lines and labels
   line = TLine()
   line.SetLineWidth(1)

   label = TLatex()
   label.SetTextAlign(ALIGN_LEFT_BOTTOM)
   label.SetTextFont(TimesNewRomanBold)
   label.SetTextSize(LABEL_FONTSIZE)

   # Plot title

   c.cd()
   h.Draw()
   title += "    %s decays" % pname
   if setlogy:
	   xpos = 0.05 * xrange + xmin
	   ypos = 10**(0.92 * log10(ymax/ymin) + log10(ymin))
   else:
	   xpos = 0.05 * xrange + xmin
	   ypos = 0.92 * yrange
   ptitle.DrawLatex(xpos, ypos, title)

   # Plot items

   xoffset = lsize + 3*space
   for BF, D in items:
	   
	   pdgid, dname, color = D[0]
	   if notDisplayed(pdgid, xoffset, BF, xmin, xmax, ymin, ymax): continue
	   
	   line.SetLineColor( pcolor )
	   line.DrawLine( xoffset, BF, xoffset + lsize, BF )

	   xoffset += lsize + space
	   for id, dname, dcolor in D:		   
		   label.SetTextColor( dcolor )
		   label.DrawLatex(xoffset, BF, dname)
		   # need a bit more space for boson characters
		   if dname[0] in ['W']: xoffset += 2*space
		   xoffset += 3*space
	   xoffset += space

   c.Update()
   c.SaveAs( "%s_%d_decay.eps" % (output, pdgId) )
   c.SaveAs( "%s_%d_decay.gif" % (output, pdgId) )
   return (c, h) # needed to prevent plot from vanishing!
#------------------------------------------------------------------------------
def main():

	(filename, atitle, awidth, ymin, ymax), extras = decodeCommandLine()

	# Read SLHA file
	records  = map(strip, open(filename).readlines())
	
	n = rfind(filename, '.')
	output = filename[:n]
	
	# Plot either masses or decays

	setStyle()
	
	if extras['pdgid'] == None:
		block, ymin, ymax = getMassBlock(records, ymin, ymax, extras)

		c,h = plotMasses(block, output, ymin, ymax,
						 title=atitle, width=awidth,
						 setlogy=extras['setlogy'],
						 squeeze=true)
	else:
		block, ymin, ymax = getDecayBlock(records, ymin, ymax, extras)

		c,h = plotDecays(block, output, ymin, ymax,
						 title=atitle, width=awidth,
						 setlogy=extras['setlogy'],
						 squeeze=extras['squeeze'])

	print "\n\twarning: ...I shall self-destruct in 5 seconds..."
	sleep(5)
	print "\tBoom!!\n"
	#gApplication.Run()
#------------------------------------------------------------------------------
main()

