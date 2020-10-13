#-----------------------------------------------------------------------------
import sys
import ROOT
#-----------------------------------------------------------------------------
def fatal(message):
	sys.exit("** error ** %s" % message)
#-----------------------------------------------------------------------------
def enableAutoLoader():
	print("enabling autoloader...")
	if not (ROOT.gSystem.Load( "libFWCoreFWLite" ) == 0):
		fatal("unable to load FWCoreFWLite")
        try:
	        ROOT.FWLiteEnabler.enable()
        except:
                ROOT.AutoLibraryLoader.enable()
#-----------------------------------------------------------------------------
enableAutoLoader()

vint         = ROOT.vector("int")
vfloat       = ROOT.vector("float")
vdouble      = ROOT.vector("double")
vstring      = ROOT.vector("string")
vvint        = ROOT.vector("vector<int>")
vvfloat      = ROOT.vector("vector<float>")
vvdouble     = ROOT.vector("vector<double>")




