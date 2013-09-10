#---------------------------------------------------------------------------
# File: ReflexLib.py
# Description: A collection of simple Reflex utilities
# Created: 25-Apr-2012 Harrison B. Prosper
#$Revision: 1.5 $
#---------------------------------------------------------------------------
from ROOT import *
from string import atof, atoi, replace, lower, \
	 upper, joinfields, split, strip, find
import os, sys, re, posixpath
from PhysicsTools.TheNtupleMaker.Lib import fixName, getwords
try:
	from PhysicsTools.TheNtupleMaker.classmap import ClassToHeaderMap
except:
	print "*** unable to import ClassToHeaderMap"
	print "*** run mkclassmap.py to create classmap.py"
	sys.exit(0)
#------------------------------------------------------------------------------
isfun     = re.compile(r'[0-9]+|bool|short|int|long|float|double'\
					   '|unsigned short|unsigned int|unsigned long|size_t')
isFuntype = re.compile(r'[0-9]+|bool|short|int|long|float|double'\
					   '|unsigned short|unsigned int|unsigned long'\
					   '|std::string|char|const char|size_t')
isSTL     = re.compile(r'std::(vector|set|map|pair)')
skipthis  = re.compile('operator|'\
					   '__get|'\
					   'const_iterator|'\
					   'iterator')
#------------------------------------------------------------------------------
def findHeaders(name):
	hdrs = {}
	pkgs = {}
	# break name into words, but skip fundamental types
	# and the main STL types
	words = getwords.findall(name)
	classlist = []
	for word in words:
		if isfun.match(word) != None:
			continue
		elif isSTL.match(word) != None:
			continue
		else:
			if not word in classlist:
				classlist.append(word)

	# for each word, scan the header map for a header file
	headers = []
	skipme = False
	for cname in classlist:
		# Find associated header
		if ClassToHeaderMap.has_key(cname):
			header = ClassToHeaderMap[cname]
			if type(header) == type([]): header = header[0]
			if not header in headers:
				headers.append(header)
		elif isfun.match(cname) != None:
			continue
	return headers
#----------------------------------------------------------------------------
# Routine to load appropriate library
#----------------------------------------------------------------------------
if not os.environ.has_key("SCRAM_ARCH"):
	print "*** SCRAM_ARCH not defined"
	print "*** need to set up CMS environment"
	sys.exit(0)

SCRAM_ARCH = os.environ["SCRAM_ARCH"]
LOCALLIBAREA = "%s/lib/%s/" % (os.environ["CMSSW_BASE"], SCRAM_ARCH)
LIBAREA = "%s/lib/%s/" % (os.environ["CMSSW_RELEASE_BASE"], SCRAM_ARCH)

LOADED_LIBS = {}

def loadLibrary(name):
## 	if len(LOADED_LIBS) == 0:
## 		import PhysicsTools.TheNtupleMaker.AutoLoader
## 		LOADED_LIBS[name] = 1
## 		return
	name = fixName(name) # remove unnecessary spaces

	if not ClassToHeaderMap.has_key(name): return

	# construct library name from header:
	# lib<subsystem><package>
	
	library = "lib%s%s" % tuple(split(ClassToHeaderMap[name],'/')[:2])
	
	if LOADED_LIBS.has_key(library): return
	
	LOADED_LIBS[library] = 0

	try:
		gSystem.Load(library)
	except:
		print "** failed to load %s for %s" % (library, name)
#----------------------------------------------------------------------------
skipmethod = re.compile(r'TClass|TBuffer|TMember|operator|^__')
reftype    = re.compile(r'(?<=edm::Ref\<std::vector\<)(?P<name>.+?)(?=\>,)')
basicstr   = re.compile(r'std::basic_string\<char\>')
vsqueeze   = re.compile(r'(?<=[^>]) +\>')
#----------------------------------------------------------------------------
FINAL   = 1
SCOPED  = 4
def classMethods(classname, db, depth=0):
	loadLibrary(classname)

	depth += 1
	if depth == 1:
		db['scopes'] = {}
		db['methods'] = {}
		db['datamembers'] = {}
		db['classname'] = classname
		db['classlist'] = []
		db['baseclassnames'] = []
		db['signature'] = {}
		db['private'] = {}
		
	if depth > 20:
		print "lost in trees"
		return
	tab = "  " * (depth-1)

	cdb = {'classname': classname,
		   'methods': []}

	thing = Reflex.Type()
	c = thing.ByName(classname)
	n = c.FunctionMemberSize()

	###D
	#print "CLASS( %s )" % classname
	
	for i in xrange(n):
		m = c.FunctionMemberAt(i)
		if not m.IsFunctionMember(): continue
		name  = m.Name()

		if not m.IsPublic():
			f     = m.TypeOf()
			mtype = f.Name(SCOPED)
			sig   = '%s %s'
			#print "\tname (%s) PRIVATE" % sig
			db['private'][sig] = 1
			continue

		if m.IsConstructor(): continue
		if m.IsDestructor():  continue

		#DEBUG = name in ['overlap']
		DEBUG = False
		
		# skip some obviously "skipable" stuff
		if skipthis.match(name) != None: continue
		
		f     = m.TypeOf()
		mtype = f.Name(SCOPED)

		# ignore methods that conflict with private methods
		sig   = '%s %s'
		if db['private'].has_key(sig):
			print "**warning - %s is a private method of %s" % (name,
																classname)
			continue
		
		mtype = strip(basicstr.sub("std::string", mtype))
		mtype = replace(mtype, "std::string >", "std::string>")
		
		t = split(mtype, '(')
		if DEBUG:
			print "NAME( %s )" % name
			print "\ttest len(t)", t
		if len(t) != 2:  continue

		# Get return type
		rtype = strip(t[0])
		# Skip setters
		rtype = strip(rtype)
		if DEBUG:
			print "\ttest for void"
		if rtype in ['void', 'void*']: continue

		# Get arguments
		args  = "(%s" % strip(t[1])
		args  = replace(args, "(void)", "()")
		argscall = args
		funargs = True
		
		if args != "()":
			
			# decode parameters
			nargs  = m.FunctionParameterSize(False)
			nrargs = m.FunctionParameterSize(True)
			ndefargs = nargs - nrargs
			
			argtypes = map(strip, split(args[1:-1], ','))
			#print name, argtypes, nargs, nrargs
			
			if len(argtypes) != nargs:
				print "**** ARGLENGTH MISMATCH (%s) (%s)" % (name, mtype)
				#sys.exit(0)
				continue

			# skip functions with non-fundamental type arguments
			for argtype in argtypes:
				if isFuntype.match(argtype): continue
				funargs = False
				break
			if DEBUG: print "\ttest for fundamental args"
			
			#print "\nNAME(%s) ARGS(%s) %d, %d" % \
			#	  (name, argtypes, nargs, ndefargs)
			argscall = '('
			args = "("
			delim= ""

			# required arguments
			for iarg in xrange(nargs):
				argname = m.FunctionParameterNameAt(iarg)
				argtype = argtypes[iarg]
				if iarg < nrargs:
					value = ""
				else:
					value = "=%s" % m.FunctionParameterDefaultAt(iarg)

				# If no argument given, make one
				if argname == "":
					argname = "x%d" % iarg
					
				if DEBUG:
					print "\tARGTYPE(%s) ARG(%s%s)" % (argtype, argname, value)
				
				args += "%s%s %s%s" % (delim, argtype, argname, value)
				argscall += "%s%s" % (delim, argname)
				delim = ", "

			args += ")"
			argscall += ")"
			
		# Check return type
		
		isconst = f.ReturnType().IsConst()
			
		# In C++ there is no overloading across scopes
		# only within scopes
		if db['scopes'].has_key(name):
			# This method is potentially an overload.
			# If we are not in the same scope, however, it cannot
			# overload the existing method
			scope = db['scopes'][name]
			if DEBUG: print "\ttest for scopes"
			if  scope != classname: continue
		db['scopes'][name] = classname

		signature  = name + args
		methodcall = name + argscall
		
		if DEBUG: print "SIG(%s) CALL(%s)" % (signature, methodcall)
		
		# Expand typedefs, but check first for pointers and
		# references
		fullrtype = rtype
		if rtype[-1] in ['*','&']:
			r = thing.ByName(rtype[:-1])
			if r.IsTypedef():
				fullrtype = "%s%s" % (r.Name(SCOPED+FINAL), rtype[-1])
				#rtype = fullrtype # Fri Jan 29
		else:
			r = thing.ByName(rtype)
			if r.IsTypedef():
				fullrtype = r.Name(SCOPED+FINAL)			

		if isconst: rtype = "const %s" % rtype
			
		rtype     = strip(basicstr.sub("std::string", rtype))
		fullrtype = strip(basicstr.sub("std::string", fullrtype))
		signature = basicstr.sub("std::string", signature)
		str = "%s  %s" % (rtype, signature)
		if DEBUG: print "\ttest for skipmethod(%s)" % str
		if skipmethod.search(str) != None: continue

		m = reftype.findall(str)
		if len(m) != 0:
			for x in m:
				cname = "%sRef" % x
				cmd = re.compile(r"edm::Ref\<.+?,%s\> \>" % x)
				rtype = cmd.sub(cname, rtype)
				signature = cmd.sub(cname, signature)

		# Ok, now added to methods list
		rtype = vsqueeze.sub(">", rtype)
		signature = vsqueeze.sub(">", signature)
		method    = "%32s  %s" % (rtype, signature)

		# Important: make sure we don't have duplicates
		if db['methods'].has_key(method):
			continue
		db['methods'][method] = (name, classname,
								 rtype, fullrtype,
								 signature,
								 methodcall,
								 funargs)
		
		cdb['methods'].append((fullrtype, method))

	db['classlist'].append( cdb )

	nb = c.BaseSize()
	for i in xrange(nb):
		b = c.BaseAt(i).ToType()
		basename = b.Name(SCOPED)
		db['baseclassnames'].append(basename)
		classMethods(basename, db, depth)
#----------------------------------------------------------------------------
def classDataMembers(classname, db, depth=0):
	loadLibrary(classname)
	
	depth += 1
	if depth > 20:
		print "lost in trees"
		return
	tab = "  " * (depth-1)

	cdb = {'classname': classname,
		   'datamembers': []}

	thing = Reflex.Type()
	c = thing.ByName(classname)
	n = c.DataMemberSize()

	for i in xrange(n):
		m = c.DataMemberAt(i)
		if not m.IsPublic(): continue

		name  = m.Name()
		dtype = m.TypeOf()
		nametype = dtype.Name(SCOPED+FINAL)
		nametype  = strip(basicstr.sub("std::string", nametype))
		db['scopes'][name] = classname
		signature = name

		# Ok, now added to datamembers list
		nametype = vsqueeze.sub(">", nametype)
		signature= vsqueeze.sub(">", signature)
		member   = "%32s  %s" % (nametype, name)

		if db['datamembers'].has_key(member):
			continue
		db['datamembers'][member] = classname
		cdb['datamembers'].append((nametype, member))
	db['classlist'].append( cdb )

	# scan bases classes
	nb = c.BaseSize()
	for i in xrange(nb):
		b = c.BaseAt(i).ToType()
		basename = b.Name(SCOPED)
		db['baseclassnames'].append(basename)
		classDataMembers(basename, db, depth)
#------------------------------------------------------------------------------
# Get fully qualified name of class
def getFullname(classname):
	loadLibrary(classname)
	t = Reflex.Type.ByName(classname)
	cname = t.FinalType().Name(7)
	if cname != "": classname = cname
	return classname
#------------------------------------------------------------------------------
# Define a container as a class that contains the methods
# size() and operator[](unsigned ? long ?int)
def isContainer(classname):
	loadLibrary(classname)
	
	t = Reflex.Type.ByName(classname)
	t = t.FinalType()
	n = t.FunctionMemberSize()
	print "%s\t%d" % (classname, n)
	
	has_size = False
	has_oper = False
	for i in xrange(n):
		m = t.FunctionMemberAt(i)
		if not m.IsPublic(): continue
		if not m.IsFunctionMember(): continue
		if m.IsConstructor(): continue
		if m.IsDestructor(): continue
		name = m.Name()
		
		if name == "size":
			# check signature
			fname = m.TypeOf().Name(3)
			if isvoid.search(fname) != None:
				has_size = True
		elif name == "operator[]":
			fname = m.TypeOf().Name(3)
			print "%s SIG(%s)" % (name, fname)
			if isint.search(fname) != None:
				has_oper = True
			
		iscon = has_size and has_oper
		if iscon: return True
		
	return False
