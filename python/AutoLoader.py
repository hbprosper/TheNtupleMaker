#-----------------------------------------------------------------------------
#$Revision: 1.1.1.1 $
import sys
from ROOT import *
#-----------------------------------------------------------------------------
def fatal(message):
	print "** error ** %s" % message
	sys.exit(0)
#-----------------------------------------------------------------------------
def enableAutoLoader():
	print "enabling autoloader..."
	if not (gSystem.Load( "libFWCoreFWLite" ) == 0):
		fatal("unable to load FWCoreFWLite")        
	AutoLibraryLoader.enable()
#-----------------------------------------------------------------------------
enableAutoLoader()

vint         = vector("int")
vfloat       = vector("float")
vdouble      = vector("double")
vstring      = vector("string")
vvdouble     = vector("vector<double>")




