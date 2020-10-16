#!/usr/bin/env python
#-----------------------------------------------------------------------------
# File:        mkntuplecfi.py
# Description: Gui to create cfi fragment for TheNtupleMaker
# Created:     06-Jan-2010 Harrison B. Prosper
# Updated:     16-Jan-2010 HBP - simplify command line
#              12-Feb-2010 HBP - Change to single quotes
#              06-Jun-2010 HBP - always highlight selected methods when a class
#                          is selected
#              08-Sep-2010 HBP - adapt to extended listing in methods files
#              18-Sep-2010 HBP - improve find
#              03-Oct-2010 HBP - add checkbox and migrate selection to
#                          selected methods only if at least one method and
#                          getbylabel is chosen.
#              02-Dec-2010 HBP - something in Pat changed (of course!) in 3_8_7
#                                requiring a different getbranch regex...sigh!
#              30-May-2011 HBP - use absolute path to methods directory
#              28-Jul-2011 HBP - handle simple types
#              24-Mar-2012 HBP - change ntuplecfi to ntuple_cfi
#              05-Jul-2013 HBP - simplify names of buffers
#              30-Sep-2020 HBP - update to Python 3
#-----------------------------------------------------------------------------
#$Id: mkntuplecfi.py,v 1.23 2013/07/05 23:02:36 prosper Exp $
#-----------------------------------------------------------------------------
import sys, os, re, platform
import ROOT
from ROOT import \
        kLHintsTop, kLHintsBottom, kLHintsLeft, kLHintsRight,\
        kLHintsExpandX, kLHintsExpandY, kTRUE, kFALSE,\
        kFDOpen, kFDSave, kSunkenFrame,\
        TGLayoutHints, TGWindow, TGMainFrame, TGMenuBar,\
        TGPopupMenu, TGVerticalFrame, TGToolBar, TGPictureButton,\
        TGHorizontalFrame, TGLabel, TGListBox, TGStatusBar,\
        TGHorizontal3DLine, TGPicturePool, TGProgressBar, TTimer,\
        TGTab, TFileDialog, THelpDialog, TGTextEntry, TGHProgressBar
from ROOT import root, Connection, Slot, itreestream
import PhysicsTools.TheNtupleMaker.AutoLoader
from PhysicsTools.TheNtupleMaker.Lib import cmsswProject, fixName, getFullname

from time import ctime, sleep
from glob import glob
from array import array
#------------------------------------------------------------------------------
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
PKGDIR = '%s%s/%s' % (LOCALBASE, PACKAGE, SUBPACKAGE)
PYDIR  = '%s/python' % PKGDIR
if not os.path.exists(PYDIR):
	PYDIR = "."	

# Load class list, if it exists
PLUGINSDIR= "%s/plugins" % PKGDIR
#------------------------------------------------------------------------------
def usage():
	sys.exit("""
Usage:
      mkntuplecfi.py [root-file-dir]
   
      root-file-dir     directory containing root files. If omitted,
		                  the default is the current directory
		""")

ARGV = sys.argv[1:]
if len(ARGV) > 0 and ARGV[0] == "?": usage()

# Make sure CMSSW is set up
if "CMSSW_BASE" not in os.environ:
	sys.exit("\t*** please set up CMSSW first\n")

BASE = os.environ["PWD"]
rev  = "2.0.0"
VERSION        = \
"""
mkntuplecfi.py %s
Python         %s
Root           %s
""" % (rev,
       platform.python_version(),
       ROOT.gROOT.GetVersion())

ICONDIR   = "%s/icons" % os.environ["ROOTSYS"]
METHODDIR = "methods"
TXTDIR    = "txt"

# Make sure methods directory and txt directories exist

if not os.path.exists(METHODDIR):
	if not os.path.exists(TXTDIR):
		cmd = "mkvomit.py"
		print("\n%s\n" % cmd)
		os.system(cmd)
                
	if not os.path.exists(TXTDIR):
		sys.exit("\t** error ** unable to run mkvomit.py")
		
	cmd = "mkmethodlist.py %s/*.txt" % TXTDIR
	print("\n%s\n" % cmd)
	os.system(cmd)

# Now get full path to methods directory
METHODDIR = str.strip(os.popen("find `pwd` -name %s" % METHODDIR).read())
print(VERSION)

#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
CLASSLISTFILE = "%sPhysicsTools/TheNtupleMaker/plugins/classlist.txt" % \
				LOCALBASE
stripstd = re.compile(r'\bstd::')
stripnsp = re.compile(r'\b[a-zA-Z0-9_]+::')

CLASSMAP = {}
if os.path.exists(CLASSLISTFILE):
	records = [x.split() for x in  open(CLASSLISTFILE).readlines()]
	recs    = [(x[-1], ' '.join(x[1:-1])) for x in records]
	for ctype, name in recs:
		fullname = getFullname(name)
		name     = stripstd.sub('',name)
		fullname = stripstd.sub('',fullname)
		
		if ctype == "collection":
			if name[-1] == ">": name += " "
			if fullname[-1] == ">": fullname += " "
			name = "vector<%s>" % name
			fullname = "vector<%s>" % fullname
		name = fixName(name)
		fullname = fixName(fullname)
		
		CLASSMAP[name] = name
		CLASSMAP[fullname] = name

	for x in ["double", "float", "long", "int", "short", "bool", "char",
                  "std::string", "string",
                  "unsigned long", "unsigned int", "unsigned short"]:
		CLASSMAP[x] = x
		x = "vector<%s>" % x
		CLASSMAP[x] = x
#-----------------------------------------------------------------------------
WIDTH          = 750            # Width of GUI in pixels
HEIGHT         = 500            # Height of GUI in pixels
# StatusBar components
STATUS_PARTS   = array('i')     # Status bar division in percentage
STATUS_PARTS.append(23)
STATUS_PARTS.append(77)
CFI_NAME = str.lower(SUBPACKAGE)
CFI_NAME = "ntuple_cfi"         # Set default config file name
CFI_LIS  = "%s.lis" % CFI_NAME
CFI_PY   = "%s.py"  % CFI_NAME
#-----------------------------------------------------------------------------
# We need a few simple regular expressions to extract sub-strings
#-----------------------------------------------------------------------------

# Extract branches ending in
getbranch = re.compile(r'(?<=\_).+?\.[ \t]+/ edm::Wrapper.+',re.M)

# Extract class name from branch name
getclass  = re.compile(r'(?<=Wrapper\<).+(?=\>)')

# Extract getByToken string from branch name
getlabel  = re.compile(r'.+(?=\_\_)|.+(?=\_)')

getmethod = re.compile(r'[a-zA-Z][^\s]*[(].*[)]')

stmethods = re.compile(r' (pt|eta|phi'\
                       '|energy|px|py|pz|p|et'\
                       '|charge)[(]')

isomethods= re.compile(r'.+Iso')

ismethods = re.compile(r'is.+')

isRootFile= re.compile(r'[.]root$')

isSimpleType = re.compile('^(unsigned long|long'\
			  '|unsigned int|int'\
			  '|unsigned short|short'\
			  '|bool|float|double|char|std::string|string)')

isSimpleVectorType = re.compile(r'(?<=vector\<)'\
                                '(unsigned long|long'\
                                '|unsigned int|int'\
                                '|unsigned short|short'\
                                '|bool|float|double|char|std::string|string)'\
				'(?=\>)')

isSimpleVectorVectorType = re.compile(r'(?<=vector\<vector\<)'\
                                '(unsigned long|long'\
                                '|unsigned int|int'\
                                '|unsigned short|short'\
                                '|bool|float|double|char|std::string|string)'\
				'(?=\>)')
vectorValue  = re.compile(r'(?<=vector\<).+?(?=\>)')
methodname   = re.compile(r'(?<= ).+(?=[(])')

# Strip away :: and vector<..>
stripname   = re.compile(r'::|vector\<|\>| ')

def makeUnique(alist):
	m = set(alist)
        l = list(m)
        l.sort()
	return l

def isVector(name):
	return str.find(name, "vector<") > -1 or \
		str.find(name, "vdouble") > -1

def sortMethods(methods):
	# Standard methods
	meths = []
	for m in methods:
		if stmethods.search(m) == None: continue
		meths.append(m)
		
	# the rest
	for m in methods:
		if stmethods.search(m)  != None: continue
		meths.append(m)
	return meths
#-----------------------------------------------------------------------------
# GUI widget Layouts
#-----------------------------------------------------------------------------
TOP          = TGLayoutHints(kLHintsTop)
LEFT         = TGLayoutHints(kLHintsLeft)
RIGHT        = TGLayoutHints(kLHintsRight)
LEFT_X       = TGLayoutHints(kLHintsLeft | kLHintsExpandX)
LEFT_X       = TGLayoutHints(kLHintsLeft | kLHintsExpandX)
TOP_LEFT     = TGLayoutHints(kLHintsTop  | kLHintsLeft)
TOP_LEFT_PADDED     = TGLayoutHints(kLHintsTop  | kLHintsLeft, 5, 5, 2, 2)
TOP_X        = TGLayoutHints(kLHintsTop  | kLHintsExpandX)
TOP_LEFT_X   = TGLayoutHints(kLHintsTop  | kLHintsLeft | kLHintsExpandX)
TOP_LEFT_X_Y = TGLayoutHints(kLHintsTop  | kLHintsLeft | kLHintsExpandX |
							 kLHintsExpandY)
TOP_RIGHT    = TGLayoutHints(kLHintsTop  | kLHintsRight)
TOP_X_Y      = TGLayoutHints(kLHintsTop  | kLHintsExpandX | kLHintsExpandY)


# Menu codes and callbacks

M_CALLBACK         = {} # Map between menu items and callbacks
# Each menu item is identified by a unique code
# and is associated with a unique callback, specified
# by name

M_FILE_OPEN        = 1;  M_CALLBACK[M_FILE_OPEN]  = "self.open()"
M_FILE_SAVE        = 2;  M_CALLBACK[M_FILE_SAVE]  = "self.save()"
M_FILE_EXIT        = 6;  M_CALLBACK[M_FILE_EXIT]  = "self.exit()"
M_EDIT_CLEAR       = 21; M_CALLBACK[M_EDIT_CLEAR] = "self.clear()"
M_EDIT_SELECT      = 22; M_CALLBACK[M_EDIT_SELECT]= "self.select()"
M_EDIT_UNSELECT    = 23; M_CALLBACK[M_EDIT_UNSELECT]= "self.unselect()"
M_EDIT_UNDO        = 24; M_CALLBACK[M_EDIT_UNDO]  = "self.undo()"
M_HELP_ABOUT       = 51; M_CALLBACK[M_HELP_ABOUT] = "self.about()"
M_HELP_USAGE       = 52; M_CALLBACK[M_HELP_USAGE] = "self.usage()"

L_CLASSBOX         = 1
L_LABELBOX         = 2
L_METHODBOX        = 3

B_OPEN             = 1
B_SAVE             = 2
B_OK               = 3

K_MENUBAR_HEIGHT   = 26    # pixels
K_TOOLBAR_HEIGHT   = 32
K_STATUSBAR_HEIGHT = 26
K_LISTBOX_WIDTH    = WIDTH/3
K_LISTBOX_HEIGHT   = 450

K_PROG_MAX         = 20.0
K_MAX_COUNT        = 50
K_MAX_LINES        = 255

# Help

HelpUsage = \
"""
Detailed instructins are given here:
https://github.com/hbprosper/TheNtupleMaker/blob/master/docs/TheNtupleMaker.pdf
"""

HU_WIDTH  = 600
HU_HEIGHT = 250

HelpAbout = \
"""
%s

\tHarrison B. Prosper and Sezen Sekmen
\tCMS Experiment
\tCERN, Geneva, Switzerland
\te-mail: harry.prosper@cern.ch

\tPermanent Address
\t\tDepartment of Physics
\t\tFlorida State University
\t\tTallahassee, FL 32306, USA
\t\te-mail: harry@hep.fsu.edu
""" % VERSION

HA_WIDTH  = 350
HA_HEIGHT = 300

BLACK       = root.Color("black")
WHITE       = root.Color("white")

RED         = root.Color("red")
ORANGE      = root.Color("orange")
YELLOW      = root.Color("yellow")
GREEN       = root.Color("green")
BLUE        = root.Color("blue")

DARKRED     = root.Color("darkred")
LIGHTYELLOW = root.Color("lightyellow")
LIGHTGREEN  = root.Color("lightgreen")

P_METHODS  = 0
P_SELECTED = 1
P_COLOR = (LIGHTYELLOW, LIGHTGREEN)

#-----------------------------------------------------------------------------
# (A) Root Graphical User Interfaces (GUI)
#
#   A Root GUI is a double tree of widgets (that is, graphical objects) with
#   which the user can interact in order to orchestrate a series of actions.
#   One tree describes the child-to-parent relationships, while the other
#   describes the parent-to-child relationships. The latter describes the
#   graphical layout of widgets. In the Root GUI system the two trees are not
#   isomorphic. For example, the child-to-parent relationship of a TGPopupMenu
#   is TGPopupMenu -> TGWindow, however, TGMenuBar -> TGPopupMenu is 
#   a typical parent-to-child relationship.
#
#   o A child-to-parent relationship is defined by the child widget when the
#     latter is created.
#
#   o A parent-to-child relationship, that is, a specific layout of a widget
#     within another, is defined by the parent after it has been created
#     using its AddFrame method in which the child is specified.
#
#   Each widget can emit one or more signals, usually triggered by some user
#   manipulation of, or action on, the widget. For example, clicking on a
#   tab of a  TGTab, that is, a notebook, will cause the notebook to emit
#   the signal "Selected(Int_t)" along with the identity of the selected tab.
#   Signals are connected to "Slots", that is, actions. This is how a user
#   GUI interaction can be made to yield one or more actions. Any signal can be
#   connected to any slot. Indeed, the relationship between signals and slots
#   can be many-many. In practice, a slot is modeled as a procedure such as
#   a method of the GUI class. In this GUI, the listboxs' Selected(Int_t)
#   signal is connected to the corresponding listbox method of the GUI so 
#   that whenever an item in the listbox is selected that method is invoked.
#   
#   In summary, a Root GUI is a (double) tree of widgets with which the user
#   can interact, whose signals---usually generated by user interactions---are
#   connected to slots, that is, actions modeled as procedures.
#
# (B) This GUI
#
#   We model this GUI as a TApplication that contains a TGMainFrame. Here is
#   the layout hierarchy (that is, the parent-to-child relationships).
#
#   window                   (TGWindow)
#
#     main                      (TGMainFrame)
#
#       menuBar                    (TGMenuBar)
#         menuFile                    (TGPopupMenu)
#         menuEdit                    (TGPopupMenu)
#         menuHelp                    (TGPopupMenu)
#
#       vframe                     (TGVerticalFrame)
#         toolBar                    (TGToolBar)
#           openButton                  (TGPictureButton)
#           saveButton                  (TGPictureButton)
#           progressBar                 (TGHProgressBar)
#
#         hframe                     (TGHorizontalFrame)
#           vcframe                     (TGVerticalFrame)
#             clabel                      (TGLabel)
#             classBox                    (TGListBox)
#
#           vmframe                     (TGVerticalFrame)
#             mlabel                      (TGLabel)
#             methodBox                   (TGListBox)
#
#           vlframe                     (TGVerticalFrame)
#             llabel                      (TGLabel)
#             labelBox                    (TGListBox)
#
#         statusBar                  (TGSTatusBar)
#-----------------------------------------------------------------------------

class Gui:
	"""
	gui = Gui(title, OpenDir)
	"""

	def __init__(self, window, name, opendir):

                # Directory containing lists of methods
		self.methodDir  = METHODDIR
                
                # Initial directory for open file dialog
		self.openDir    = opendir
                
                # Initial directory for save file dialog
		self.saveDir    = opendir
		self.iconDir    = ICONDIR
                
		# List of Signal/Slot connections
		self.connection = []

		#-------------------------------------------------------------
		# Create main frame
		#-------------------------------------------------------------
		# Establish a connection between the "CloseWindow()"
		# signal and the GUI's "close" slot, modeled as a method.
		# When the main frame issues the signal CloseWindow() this
		# triggers a call to the close method of the class Gui.

		self.window = window
		self.main = TGMainFrame(self.window, WIDTH, HEIGHT)
		self.defaultColor = self.main.GetDefaultFrameBackground()
		self.connection.append(Connection(self.main, "CloseWindow()",
						  self,      "close"))

		#-------------------------------------------------------------
		# Create menu bar
		#-------------------------------------------------------------
		# MenuBar's parent is main, a fact made explicit in MenuBar's
		# constructor. This establishes a child-to-parent relationship.
		# We also define how MenuBar is to displayed within main and
                # thereby establish a parent-to-child relationship. This is
                # done by invoking main's AddFrame method, wherein we also
                # specify how the MenuBar is to be positioned within main.
		#-------------------------------------------------------------
		# The hint objects are used to place and group
		# widgets with respect to each other.

		self.menuBar = TGMenuBar(self.main)
		self.main.AddFrame(self.menuBar, TOP_X)

		self.H3DBar  = TGHorizontal3DLine(self.main)
		self.main.AddFrame(self.H3DBar, TOP_X)

		#-------------------------------------------------------------
		# Create menus
		#-------------------------------------------------------------
		self.menuFile = TGPopupMenu(self.window)
		self.menuBar.AddPopup("&File", self.menuFile, TOP_LEFT_PADDED)
		self.menuFile.AddEntry("&Open", M_FILE_OPEN)
		self.menuFile.AddEntry("&Save", M_FILE_SAVE)
		self.menuFile.AddSeparator()
		self.menuFile.AddEntry("E&xit",M_FILE_EXIT)        
		self.connection.append(Connection(self.menuFile,
                                                  "Activated(Int_t)",
						  self,          "menu"))

		self.menuEdit = TGPopupMenu(self.window)
		self.menuBar.AddPopup("&Edit", self.menuEdit, TOP_LEFT_PADDED)
		self.menuEdit.AddEntry("Select   All Methods", M_EDIT_SELECT)
		self.menuEdit.AddEntry("UnSelect All Methods", M_EDIT_UNSELECT)
		self.menuEdit.AddSeparator()
		self.menuEdit.AddEntry("&UnSelect Class", M_EDIT_CLEAR)
		self.connection.append(Connection(self.menuEdit,
                                                  "Activated(Int_t)",
						  self,          "menu"))

		self.menuHelp = TGPopupMenu(self.window)
		self.menuBar.AddPopup("&Help", self.menuHelp, TOP_LEFT_PADDED)
		self.menuHelp.AddEntry("Usage", M_HELP_USAGE)
		self.menuHelp.AddEntry("About", M_HELP_ABOUT)
		self.menuHelp.AddSeparator()
		self.connection.append(Connection(self.menuHelp,
                                                  "Activated(Int_t)",
						  self,          "menu"))

		#-------------------------------------------------------------
		# Add vertical frame to contain notebook, toolbar and
		# status window
		#-------------------------------------------------------------
		self.vframe = TGVerticalFrame(self.main, 1, 1)
		self.main.AddFrame(self.vframe, TOP_X_Y)

		#-------------------------------------------------------------
		# Add horizontal frame to hold toolbar
		#-------------------------------------------------------------
		self.toolBar = TGHorizontalFrame(self.vframe, 1, 1)
		self.vframe.AddFrame(self.toolBar, TOP_X)

		#-------------------------------------------------------------
		# Add picture buttons
		#-------------------------------------------------------------
		BUTTON_LAYOUT = TGLayoutHints(kLHintsLeft, 10, 10, 2, 2)
		self.picpool  = TGPicturePool(ROOT.gClient, ICONDIR)
		if self.picpool:
			self.openIcon   = self.picpool.GetPicture("open.xpm")
			self.openButton = TGPictureButton(self.toolBar,
							  self.openIcon,
							  B_OPEN)
			self.openButton.SetToolTipText("Open an EDM file")
			self.toolBar.AddFrame(self.openButton, BUTTON_LAYOUT)
			self.connection.append(Connection(self.openButton,
							  "Clicked()",
							  self, "open"))

			self.saveIcon   = self.picpool.GetPicture("save.xpm")
			self.saveButton = TGPictureButton(self.toolBar,
							  self.saveIcon,
							  B_SAVE)
			self.saveButton.SetToolTipText("Save "\
                                                       "configuration "\
                                                       "file fragment")
			self.toolBar.AddFrame(self.saveButton, BUTTON_LAYOUT)
			self.connection.append(Connection(self.saveButton,
							  "Clicked()",
							  self, "save"))

		#-------------------------------------------------------------
		# Add a find window
		#-------------------------------------------------------------
		self.findlabel = TGLabel(self.toolBar, "Find method:")
		FINDLABEL_LAYOUT = TGLayoutHints(kLHintsLeft,2,2,5,5)
		self.toolBar.AddFrame(self.findlabel, FINDLABEL_LAYOUT)
		self.findBox = TGTextEntry(self.toolBar)
		self.findBox.SetToolTipText("type, then hit return to continue")
		self.findBox.SetTextColor(DARKRED)
		self.findcurrentPos = 0
		
		FIND_LAYOUT  = TGLayoutHints(kLHintsLeft,2,2,2,2)
		self.toolBar.AddFrame(self.findBox, FIND_LAYOUT)
		self.connection.append(Connection(self.findBox,
						  "ReturnPressed()",
						  self, "find"))
			
		#-------------------------------------------------------------
		# Add a horizontal progress bar
		#-------------------------------------------------------------
		self.progressBar = TGHProgressBar(self.toolBar,
						  TGProgressBar.kFancy, 1)
		self.progressBar.SetBarColor("green")
		self.progressBar.SetRange(0, K_PROG_MAX)
		PB_LAYOUT = TGLayoutHints(kLHintsLeft |
					  kLHintsExpandX |
					  kLHintsExpandY, 10, 10, 2, 2)
		self.toolBar.AddFrame(self.progressBar, PB_LAYOUT)

		# Set up a timer for progress bar
		self.progTimer = TTimer(500)
		self.connection.append(Connection(self.progTimer,
						  "Timeout()",
						  self, "progTimeout"))

		#-------------------------------------------------------------
		# Add a notebook with two pages
		# 1. The Methods page lists available methods
		# 2. The Selected Methods page lists selected methods
		#-------------------------------------------------------------
		self.noteBook = TGTab(self.vframe, 1, 1)
		self.vframe.AddFrame(self.noteBook, TOP_X_Y)
	        self.connection.append(Connection(self.noteBook,
                                                  "Selected(Int_t)",
					          self, "setPage"))

		self.pages = []
		self.pages.append(self.noteBook.AddTab("Methods"))
		self.pages.append(self.noteBook.AddTab("Selected Methods"))
		self.currentPage = 0
		self.noteBook.SetTab("Methods")
		tab = self.noteBook.GetTabTab("Methods")
		tab.ChangeBackground(LIGHTYELLOW)
		
		#-------------------------------------------------------------
		# Add horizontal frame to contain labels and listboxes
		#-------------------------------------------------------------
		self.hframe    = [0,0]
		self.clabel    = [0,0]
		self.mlabel    = [0,0]
		self.llabel    = [0,0]
		self.vcframe   = [0,0]
		self.vmframe   = [0,0]
		self.vlframe   = [0,0]
		self.classBox  = [0,0]
		self.labelBox  = [0,0]
		self.methodBox = [0,0]
		
		for ID in [0,1]:
			idoffset = ID*10
			
			self.hframe[ID] = TGHorizontalFrame(self.pages[ID],
                                                            1, 1)
			self.pages[ID].AddFrame(self.hframe[ID], TOP_X_Y)

			# Create a labeled list box by creating a vertical
                        # frame, inserting it into the horizontal frame,
                        # and then inserting the label followed by the listbox
                        # into the vertical frame
			
			self.vcframe[ID] = TGVerticalFrame(self.hframe[ID],
                                                           1, 1)
			self.hframe[ID].AddFrame(self.vcframe[ID], TOP_X_Y)
			self.hframe[ID].ChangeBackground(YELLOW)
			
			self.clabel[ID] = TGLabel(self.vcframe[ID], "Classes")
			self.vcframe[ID].AddFrame(self.clabel[ID], TOP_X)

			self.classBox[ID] = TGListBox(self.vcframe[ID],
						      L_CLASSBOX+idoffset,
						      kSunkenFrame)
			self.classBox[ID].Resize(K_LISTBOX_WIDTH,
                                                 K_LISTBOX_HEIGHT)
			self.vcframe[ID].AddFrame(self.classBox[ID], TOP_X_Y)
			self.connection.append(Connection(self.classBox[ID],
							  "Selected(Int_t)",
							  self,
							  "classListBox%d" % \
                                                          (ID+1)))
			
			# Create a list box for methods

			self.vmframe[ID] = TGVerticalFrame(self.hframe[ID],
                                                           1, 1)
			self.hframe[ID].AddFrame(self.vmframe[ID], TOP_X_Y)
			
			self.mlabel[ID] = TGLabel(self.vmframe[ID], "Methods")
			self.vmframe[ID].AddFrame(self.mlabel[ID], TOP_X)
			
			self.methodBox[ID] = TGListBox(self.vmframe[ID],
						       L_METHODBOX+idoffset,
						       kSunkenFrame)
			self.methodBox[ID].Resize(K_LISTBOX_WIDTH,
                                                  K_LISTBOX_HEIGHT)
			self.methodBox[ID].SetMultipleSelections()
			self.vmframe[ID].AddFrame(self.methodBox[ID], TOP_X_Y)
			self.connection.append(Connection(self.methodBox[ID],
							  "Selected(Int_t)",
							  self,
							  "methodListBox%d" % \
                                                          (ID+1)))

			# Create a list box for getByLabel string

			self.vlframe[ID] = TGVerticalFrame(self.hframe[ID],
                                                           1, 1)
			self.hframe[ID].AddFrame(self.vlframe[ID], TOP_X_Y)
			
			self.llabel[ID] = TGLabel(self.vlframe[ID], "Category")
			self.vlframe[ID].AddFrame(self.llabel[ID], TOP_X)
			
			self.labelBox[ID] = TGListBox(self.vlframe[ID],
						      L_LABELBOX+idoffset,
						      kSunkenFrame)
			self.labelBox[ID].Resize(K_LISTBOX_WIDTH,
                                                 K_LISTBOX_HEIGHT)
			self.labelBox[ID].SetMultipleSelections()
			self.vlframe[ID].AddFrame(self.labelBox[ID], TOP_X_Y)
			self.connection.append(Connection(self.labelBox[ID],
							  "Selected(Int_t)",
							  self,
							  "labelListBox%d" % \
                                                          (ID+1)))
		#--------------------------------------------------------------
		# Create a status bar, divided into two parts
		#--------------------------------------------------------------
		self.statusBar = TGStatusBar(self.vframe, 1, 1)
		status_parts = array('i')
		status_parts.append(23)
		status_parts.append(77)
		self.statusBar.SetParts(status_parts, len(status_parts))
		self.vframe.AddFrame(self.statusBar, TOP_X)

		self.mapWindows()

	def __del__(self):
		pass

#----------------------------------------------------------------------------
# Methods
#----------------------------------------------------------------------------
	def mapWindows(self):

		self.main.SetWindowName("Make n-tuple configuration fragment")

		# Map all subwindows of main frame
		self.main.MapSubwindows()

		# Initialize layout        
		self.main.Resize(self.main.GetDefaultSize())

		# Finally map the main frame (render GUI visible)        
		self.main.MapWindow()

#---------------------------------------------------------------------------
	def loadData(self, filename):

		# List root file

		self.statusBar.SetText("Listing file ...", 0)
		self.statusBar.SetText(filename, 1)

		self.progTimer.Start()

		stream = itreestream(filename, "Events")
		record = stream.str()

		stream.close()
		
		stream = itreestream(filename, "Runs")
		record += stream.str()
		stream.close()
		sleep(2)

		self.progTimer.Stop()
		self.progressBar.Reset()

		# Get branches
		recs = getbranch.findall(record)
		if len(recs) == 0:
			self.statusBar.SetText("** Error", 0)
			self.statusBar.SetText("No branches found", 1)
			return
		self.statusBar.SetText("Done!", 0)

		records = []
		for ii, x in enumerate(recs):
			t = str.split(x, '/')
			c = str.strip(t[1]) # Wrapped class name

			# Get label name
			t = str.split(t[0], '_')
			label = t[0]
			if t[1] != '': label += "_%s" % t[1]

                        # Get class name
			cname = fixName(str.strip(getclass.findall(c)[0]))
			cname = stripstd.sub('',cname)

                        # check for simple types.
			s = isSimpleType.findall(cname)
			v = isSimpleVectorType.findall(cname)
			vv= isSimpleVectorVectorType.findall(cname)
			simpleType = len(s) > 0 or len(v) > 0 or len(vv) > 0

                        # do nothing if this is a simple type
                        if simpleType:
                                pass

			# Display class only if it is in classlist.txt
			elif cname in CLASSMAP: 
                                cname = CLASSMAP[cname]
			else:
                                continue

			t = (label, cname, simpleType)
			records.append(t)

		# Create a map to keep track of selections
		
		self.cmap = {}
		self.previousID = -1

		for label, cname, simpleType in records:

                        # Initialize cmap
                        if cname not in self.cmap:

                                if simpleType:
                                        method  = "none"
                                        methods = [method]
                                        # set "method" selection to True
                                        mmap = {method : True}
                                else:
                                        # a complex type, so search for file 
                                        # containing list of methods associated
                                        # with cname
                                        fname = "%s/%s.txt" % \
                                                (self.methodDir,
                                                 stripname.sub("", cname))

                                        if not os.path.exists(fname):
                                                print("** file %s not found" % \
                                                      fname)
                                                continue
                                        self.statusBar.SetText(fname, 1)

                                        t = [x.strip() for x in 
                                             open(fname).readlines()]
                                        methods = sortMethods(
                                                list(filter(lambda x:
                                                            x != "", t)))
                                        mmap = {}
                                        for method in methods: 
                                                mmap[method] = False

                                # initialize map
                                self.cmap[cname] = {'selected': False,
                                                    'labels': {},
                                                    'simpleType': simpleType,
                                                    'methods': mmap,
                                                    'sortedmethods': methods}

                                
			self.cmap[cname]['labels'][label] = False

		# Fill classBox on Methods page
		self.statusBar.SetText("Number of classes/simple types", 0)
		cnames = list(self.cmap.keys())
		self.statusBar.SetText("%d" % len(cnames), 1)
		cnames.sort()

		for ID in [P_METHODS, P_SELECTED]:
			self.classBox[ID].RemoveAll()
			self.labelBox[ID].RemoveAll()
			self.methodBox[ID].RemoveAll()
		
		for index, entry in enumerate(cnames):
			self.classBox[P_METHODS].AddEntry(entry, index)
		self.classBox[P_METHODS].Layout()
		
#---------------------------------------------------------------------------
# Slots
#---------------------------------------------------------------------------

#---------------------------------------------------------------------------
# Responds to: Selected(Int_t)  issued by Edit/Clear

	def clear(self):
		pageNumber = self.noteBook.GetCurrent()
 		if pageNumber == P_SELECTED: return

		# We are in methods page
		
		cid = self.classBox[pageNumber].GetSelected()
		cname = self.classBox[pageNumber].GetEntry(cid).GetText().Data()
		self.cmap[cname]['selected'] = False
		self.classBox[P_METHODS].Select(cid, kFALSE)
		self.labelBox[P_METHODS].RemoveAll()
		self.methodBox[P_METHODS].RemoveAll()		
#---------------------------------------------------------------------------
# Responds to: Selected(Int_t)  issued by Notebook

	def setPage(self, ID):
		
		# Before changing current tab's color,
                # re-color previous tab to the
		# TGMainframe's default color

		tab = self.noteBook.GetTabTab(self.currentPage)
		tab.ChangeBackground(self.main.GetDefaultFrameBackground())

		# Now re-color current tab
                
		self.currentPage = ID
		tab = self.noteBook.GetTabTab(self.currentPage)
		tab.ChangeBackground(P_COLOR[ID])

		# return if this is the methods page
		
		if ID == P_METHODS: return
		
		# This is the selected methods page, so update it
		# Note which class was selected

		cid = self.classBox[P_SELECTED].GetSelected()
		if cid < 0:
			cname = ''
		else:
			cname = self.classBox[P_SELECTED].\
                                GetEntry(cid).GetText().Data()

		self.classBox[P_SELECTED].RemoveAll()
		self.labelBox[P_SELECTED].RemoveAll()
		self.methodBox[P_SELECTED].RemoveAll()

		# List selected classes
		
		cnames = list(self.cmap.keys())
		cnames.sort()
		for index, entry in enumerate(cnames):

			if not self.cmap[entry]['selected']: continue

			self.classBox[P_SELECTED].AddEntry(entry, index)

			if entry != cname: continue

			# The current class was selected in
                        # Selected Methods page
			
			self.classBox[P_SELECTED].Select(index)

			# List selected categories
		
			labels= self.cmap[cname]['labels']
			names = list(labels.keys())
			names.sort()
			self.labelBox[P_SELECTED].RemoveAll()
			for ind, name in enumerate(names):
				if not labels[name]: continue
				self.labelBox[P_SELECTED].AddEntry(name, ind)
			self.labelBox[P_SELECTED].Layout()

			# List selected methods
		
			methods = self.cmap[cname]['methods']
			names   = self.cmap[cname]['sortedmethods']

			self.methodBox[P_SELECTED].RemoveAll()
			for ind, name in enumerate(names):
				if not methods[name]: continue
				self.methodBox[P_SELECTED].AddEntry(name, ind)
				
			self.methodBox[P_SELECTED].Layout()
			
		self.classBox[P_SELECTED].Layout()
		
#---------------------------------------------------------------------------
# Responds to: Timeout()    issued by Timer

	def progTimeout(self):
		if self.progressBar.GetPosition() < K_PROG_MAX:
			self.progressBar.Increment(1.0)
		else:
			self.progressBar.Reset()
                # Force immediate redraw
		root.GetRoot().NeedRedraw(self.progressBar) 
				
#---------------------------------------------------------------------------
# Responds to: CloseWindow()    issued by Main

	def close(self):
		ROOT.gApplication.Terminate()

#---------------------------------------------------------------------------
# Responds to: Activated(Int_t) issued by menuFile, menuEdit, menuHelp

	def menu(self, id):
		if id in M_CALLBACK:
			exec(M_CALLBACK[id])
		else:
			print "Unrecognized menu id = %d" % id

#---------------------------------------------------------------------------
# Responds to: Selected(Int_t)

	def classListBox1(self, ID):	
		pageNumber = self.noteBook.GetCurrent()
		cname = self.classBox[pageNumber].GetEntry(ID).GetText().Data()
		
		self.methodBox[pageNumber].RemoveAll()
		self.labelBox[pageNumber].RemoveAll()

		methods= self.cmap[cname]['methods']
		labels = self.cmap[cname]['labels']
		
		# Check if previously highlighted
		
		if self.previousID == ID:
			self.previousID = -1
			self.cmap[cname]['selected'] = False

			for index, name in enumerate(list(methods.keys())):
				self.cmap[cname]['methods'][name] = False

			for index, name in enumerate(list(labels.keys())):
				self.cmap[cname]['labels'][name] = False

			self.classBox[pageNumber].Select(ID, kFALSE)
			self.methodBox[pageNumber].Layout()
			self.labelBox[pageNumber].Layout()
			self.classBox[pageNumber].\
                                GetEntry(ID).SetBackgroundColor(WHITE)
			return
		
		self.previousID = ID
	
		# List methods
		
		names   = self.cmap[cname]['sortedmethods']
		for index, name in enumerate(names):
			self.methodBox[pageNumber].AddEntry(name, index)
			if self.cmap[cname]['methods'][name]:
				self.methodBox[pageNumber].Select(index)
		self.methodBox[pageNumber].Layout()

		# List categories
		
		names = list(labels.keys())
		names.sort()
		for index, name in enumerate(names):
			self.labelBox[pageNumber].AddEntry(name, index)
			if self.cmap[cname]['labels'][name]:
				self.labelBox[pageNumber].Select(index)
		self.labelBox[pageNumber].Layout()

#---------------------------------------------------------------------------
# Responds to: Selected(Int_t)

	def classListBox2(self, ID):		
		pageNumber = self.noteBook.GetCurrent()
		cname = self.classBox[pageNumber].GetEntry(ID).GetText().Data()
		
		self.methodBox[pageNumber].RemoveAll()
		self.labelBox[pageNumber].RemoveAll()

		methods = self.cmap[cname]['methods']
		labels  = self.cmap[cname]['labels']

		self.cmap[cname]['selected'] = True
	
		# List methods
		
		names   = self.cmap[cname]['sortedmethods']
		for index, name in enumerate(names):
			if not methods[name]: continue
			self.methodBox[pageNumber].AddEntry(name, index)
		self.methodBox[pageNumber].Layout()

		# List getByLabels
		
		names = list(labels.keys())
		names.sort()
		for index, name in enumerate(names):
			if not labels[name]: continue
			self.labelBox[pageNumber].AddEntry(name, index)
		self.labelBox[pageNumber].Layout()		
#---------------------------------------------------------------------------
	def methodListBox1(self, ID):
		pageNumber = self.noteBook.GetCurrent()

		# This is the Methods page, so flag method as selected	
			
		cid   = self.classBox[pageNumber].GetSelected()
		cname = self.classBox[pageNumber].GetEntry(cid).GetText().Data()
		name  = self.methodBox[pageNumber].GetEntry(ID).GetText().Data()
		on    = self.cmap[cname]['methods'][name]
		self.cmap[cname]['methods'][name] = not on
		self.setSelectedClass(cname)
		
	def methodListBox2(self, ID):
		pageNumber = self.noteBook.GetCurrent()
		# Disable method selection in Selected Methods page
		self.methodBox[pageNumber].Select(ID, kFALSE)
		return
#---------------------------------------------------------------------------
	def labelListBox1(self, ID):
		pageNumber = self.noteBook.GetCurrent()
	
		# This is the Methods page, so flag label as selected
		
		cid   = self.classBox[pageNumber].GetSelected()
		cname = self.classBox[pageNumber].GetEntry(cid).GetText().Data()
		name  = self.labelBox[pageNumber].GetEntry(ID).GetText().Data()
		on    = self.cmap[cname]['labels'][name]
		self.cmap[cname]['labels'][name] = not on
		self.setSelectedClass(cname)
		
	def labelListBox2(self, ID):
		pageNumber = self.noteBook.GetCurrent()
		# Disable label selection in Selected Methods page		
		self.labelBox[pageNumber].Select(ID, kFALSE)
		return
#---------------------------------------------------------------------------
	def setSelectedClass(self, cname):
		# A class is selected if and only if at least one
		# method and one label has been selected.

		methodSelected = False
		itemmap = self.cmap[cname]['methods']
		for name in list(itemmap.keys()):
			if not itemmap[name]: continue
			# a method has been selected
			methodSelected = True
			break
		
		labelSelected = False
		itemmap = self.cmap[cname]['labels']
		for name in list(itemmap.keys()):
			if not itemmap[name]: continue
			# a category has been selected
			labelSelected = True
			break
		
		self.cmap[cname]['selected'] = methodSelected and labelSelected
		pageNumber = self.noteBook.GetCurrent()
		cid = self.classBox[pageNumber].GetSelected()
		if self.cmap[cname]['selected']:
			self.classBox[pageNumber].\
                                GetEntry(cid).SetBackgroundColor(YELLOW)
		else:
			self.classBox[pageNumber].\
                                GetEntry(cid).SetBackgroundColor(WHITE)

		self.classBox[pageNumber].Layout()
#---------------------------------------------------------------------------
	def select(self):

		# If this is the selected methods page, do nothing
		
		pageNumber = self.noteBook.GetCurrent()
		if pageNumber == P_SELECTED: return

		cid   = self.classBox[pageNumber].GetSelected()
		if cid < 0: return

		# Select all methods

		cname = self.classBox[pageNumber].GetEntry(cid).GetText().Data()
		nentries = len(self.cmap[cname]['methods'])

		for id in range(nentries):
			name = self.methodBox[pageNumber].\
                               GetEntry(id).GetText().Data()
			self.cmap[cname]['methods'][name] = True
			self.methodBox[pageNumber].Select(id)
		self.methodBox[pageNumber].Layout()
#---------------------------------------------------------------------------
	def unselect(self):

		# If this is the selected methods page, do nothing
		
		pageNumber = self.noteBook.GetCurrent()
		if pageNumber == P_SELECTED: return

		cid   = self.classBox[pageNumber].GetSelected()
		if cid < 0: return
		
		# Un-select all methods

		cname = self.classBox[pageNumber].GetEntry(cid).GetText().Data()
		nentries = len(self.cmap[cname]['methods'])

		for id in range(nentries):
			name = self.methodBox[pageNumber].\
                               GetEntry(id).GetText().Data()
			self.cmap[cname]['methods'][name] = False
			self.methodBox[pageNumber].Select(id, kFALSE)
		self.methodBox[pageNumber].Layout()
#---------------------------------------------------------------------------
	def open(self):
		fdialog = TFileDialog(self.window,
				      self.main,
				      kFDOpen,
				      self.openDir)

		filename = fdialog.Filename()
		self.openDir  = fdialog.IniDir()

		if isRootFile.search(filename) == None:
			THelpDialog(self.window,
				    "Warning",
				    "Please select a root file!", 230, 50)
			return

		self.loadData(filename)
#---------------------------------------------------------------------------
	def undo(self):
		self.notdone()
#---------------------------------------------------------------------------
	def find(self):

		# If this is the selected methods page, do nothing
		
		pageNumber = self.noteBook.GetCurrent()
		if pageNumber == P_SELECTED: return

		cid   = self.classBox[pageNumber].GetSelected()
		if cid < 0: return

		origstr   = self.findBox.GetText()
		searchstr = str.lower(origstr)
		searchstr = str.replace(searchstr,'(','[(]')
		searchstr = str.replace(searchstr,')','[)]')
		searchstr = str.replace(searchstr,'->','[-][>]')
		searchstr = str.replace(searchstr,'*','.*')
		
		findit    = re.compile(searchstr)		
		cname     = self.classBox[pageNumber].\
                            GetEntry(cid).GetText().Data()
		nentries  = len(self.cmap[cname]['methods'])
		start     = self.findcurrentPos + 1
		
		for ID in range(start, nentries):
			self.findcurrentPos = ID
			name = self.methodBox[pageNumber].\
                               GetEntry(ID).GetText().Data()
			lowername  = str.lower(name)
			if findit.search(lowername) == None: continue
			self.methodBox[pageNumber].SetTopEntry(ID)
                        #print("REGEX(%s) FIND( %s )" % (searchstr, name))
			break

		if self.findcurrentPos >= (nentries-1):
			self.findcurrentPos = 0
			THelpDialog(self.window,
				    "Warning",
				    "Method %s not found!" % origstr, 250, 40)
						
		self.methodBox[pageNumber].Layout()
		
#---------------------------------------------------------------------------
# Save selected class/label/methods to config file fragment
#---------------------------------------------------------------------------
	def save(self):

		# Get info to be written out
		blocknamemap = {}
		cnames = list(self.cmap.keys())
		cnames.sort()

		blocks  = []
		databuf = {}
		for index, cname in enumerate(cnames):

                        item = self.cmap[cname]

                        # if this class/simple type not selected skip
                        if not item['selected']: continue

                        simpleType = item['simpleType']

                        #print("STEP(1) %s %s" % (cname, simpleType))

			# Current class selected			
			# Get selected categories (i.e., labels)
			labels = []
			selected = item['labels']
			for label in list(selected.keys()):
				if not selected[label]: continue
				labels.append(label)

                        #print("STEP(1.2) %s / %s" % (cname, labels))

			# If no label selected for this class, complain
			if labels == []:
				message = "Either:\ngo boil your head...\n" \
					"or choose category for type:\n%s" % \
                                        cname
				THelpDialog(self.window, "Warning",
                                            message, 250, 100)
				return

                        #print("STEP(1.3)")

			# If this class is a vector, set the maximum
                        # count to K_MAX_COUNT,
			# otherwise assume we have a singleton.

			if isVector(cname):
				maxcount = K_MAX_COUNT
			else:
				maxcount = 1
			
			# Get selected methods. if method == "none",
                        # there are no methods because we have a simple type.

			methods = []	
			selected = item['methods']
			sortedmethods = item['sortedmethods']
			for method in sortedmethods:
				if not selected[method]: continue
				methods.append(method)

                        #print("STEP(2)")

			# vstring seems to have a limit on the number of
                        # strings, which implies a maximum number of 
                        # variables/buffer

			if len(methods) > K_MAX_LINES - 1:
				THelpDialog(self.window,
					    "Warning!",
					    "maximum variable count %d "\
                                            "reached" % K_MAX_LINES,
							320, 40)
				methods = methods[:K_MAX_LINES-1]

			# If the current class is associated with
                        # multiple categories (that is, labels) make 
                        # sure each buffer block has a unique name

                        #print("STEP(2.1)")

			labels.sort()

			# For now ignore AssociationVectors
			avectorType= cname.find("edm::AssociationVector") > -1
			if avectorType: continue

                        #print("STEP(3) %s / %s" % (cname, simpleType))

			# create a blockName from cname
                        if simpleType:
                                t = vectorValue.findall(cname)
                                if len(t) > 0: cname = t[0]
                                blockName = labels[0]
                                blockName = blockName.capitalize()
                                # strip away possible vector decoration
                        else:
                                # complex type
                                # strip away possible vector decoration
                                t = vectorValue.findall(cname)
                                if len(t) > 0: cname = t[0]
                                blockName = str.replace(cname, 'Helper', '')
                                blockName = stripnsp.sub("", blockName)

                        #print("STEP(4) %s / %s / %s" % \
                        #      (cname, blockName, labels))
                        
			blocks.append(blockName)
			databuf[blockName] = {}
			databuf[blockName]['type']    = cname
			databuf[blockName]['label']   = labels[0]
			databuf[blockName]['count']   = maxcount
			databuf[blockName]['methods'] = methods

                        #print("STEP(5) %s -> %s" % (cname, blockName))

                        # if we have more labels, make sure blockName
                        # is unique
			for index in range(1, len(labels)):
                                if simpleType:
                                        blockname = labels[index].capitalize()
                                else:
                                        blockname = "%s%d" % (blockName, index)
				blocks.append(blockname)
				databuf[blockname] = {}
				databuf[blockname]['type']    = cname
				databuf[blockname]['label']   = labels[index]
				databuf[blockname]['count']   = maxcount
				databuf[blockname]['methods'] = methods
                
                                #print("STEP(6) %s -> %s" % (cname, blockName))

                # -------------------------------------------------
		# Now write out info
                # -------------------------------------------------
		self.saveDir = PYDIR
		fdialog = TFileDialog(self.window,
				      self.main,
				      kFDSave,
				      self.saveDir,
				      CFI_PY)

		self.saveDir = fdialog.IniDir()
		filename = fdialog.Filename()
		
		self.statusBar.SetText("Saving to file", 0)
		self.statusBar.SetText(filename, 1)
		
		tab  = ' '*4
		out = open(filename, "w")
		out.write('#------------------------------------'\
				  '-------------------------------------\n')
		out.write("# Created: %s by mkntuplecfi.py\n" % ctime())
		out.write('#------------------------------------'\
				  '-------------------------------------\n')
		out.write("import FWCore.ParameterSet.Config as cms\n")
		out.write('TNM = cms.EDAnalyzer(\n')
                out.write('%s"TheNtupleMaker",\n' % tab)
		#out.write('%sntupleName   = cms.untracked.'\
                #          'string("ntuple.root"),\n' % tab)
		out.write('%s#analyzerName = cms.untracked.'\
                          'string("analyzer.cc"),\n\n' % tab)

		out.write('\n')
		out.write('%s# NOTE: the names listed below '\
                          'will be the prefixes for\n'\
			  '%s#       the associated C++ variables created by '\
			  'mkanalyzer.py\n' % (tab, tab))
		out.write('\n')
		out.write('%sbuffers = cms.untracked.vstring(\n' % tab)
		tab1 = tab
		
		# 1. Write out list of blocks/buffers
			
		tab  = 4*' '
		tab2 = 8*' '
                delim = tab2
		for index, blockName in enumerate(blocks):
			out.write("%s'%s'" % (delim, blockName))
			delim = ",\n%s" % tab2
		out.write('\n%s),\n\n' % tab1)

		# 2. For each buffer, write out associated methods
		
		prefix = tab1
		for index, blockName in enumerate(blocks):

			#print("STEP(7)", blockName)

                        cname   = databuf[blockName]['type']
			label   = databuf[blockName]['label']
			maxcount= databuf[blockName]['count']
			methods = databuf[blockName]['methods']

			out.write('%s%s = cms.untracked.vstring(\n' % \
                                  (prefix, blockName))
			
			prefix = ",\n\n%s" % tab1
			
			# Write block header
			
                        hasMethods = methods[0] != "none"
                        if hasMethods:
                                postfix = ",\n"
                        else:
                                postfix = ""

			record = "%s'%-31s %-31s %3d'%s" % \
				(tab1, cname, label, maxcount, postfix)
			out.write(record)
			
			# Loop over methods for current block

                        if hasMethods:
                                out.write('%s#--------------------------------'\
                                          '---------------------------'\
                                          '----------\n' % \
                                          tab1)
                                delim = tab2
                                for index, method in enumerate(methods):
                                        self.statusBar.SetText(method, 1)
                                        record = "%s'%s'" % (delim, method)
                                        delim  = ",\n%s"  % tab2
                                        out.write(record)
                        out.write('\n%s)' % tab1)
				
		out.write("\n)\n")
		out.close()
		sleep(1)
		self.statusBar.SetText("Done!", 0)
		self.statusBar.SetText("", 1)
					
	def usage(self):
		THelpDialog(self.window, "Usage",
                            HelpUsage, HU_WIDTH, HU_HEIGHT)

	def about(self):
		THelpDialog(self.window, "About",
                            HelpAbout, HA_WIDTH, HA_HEIGHT)

	def exit(self):
		self.close()

	def notdone(self):
		print "Not done"
		THelpDialog(self.window,"Not Yet Done!",
                            "\tThou lump of foul deformity!",
			    220, 30)

	def Run(self):
		ROOT.gApplication.Run()

#------------------------------------------------------------------------------
#---- Main program ------------------------------------------------------------
#------------------------------------------------------------------------------
def main():
	if len(ARGV) > 0:
		opendir = ARGV[0]
	else:
		opendir = "."
	gui = Gui(root.GetRoot(), "mkntuplecfi", opendir)
	gui.Run()
#------------------------------------------------------------------------------
main()



