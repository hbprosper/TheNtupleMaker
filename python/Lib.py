#---------------------------------------------------------------------------
# File: Lib.py
# Description: A collection of simple utilities, many culled
#              from either xml2boost.py or header2xml.
# Created: 19-May-2006 Harrison B. Prosper
#          18-Sep-2010 HBP use updated version of convert2html
#          31-Mar-2012 HBP parseHeader now extract typedefs 
#          29-Aug-2020 HBP adapt to Python 3 (ADL project)
#          30-Sep-2020 HBP fix stuff for ADL
#---------------------------------------------------------------------------
import os, sys, re, posixpath, ROOT
from pprint import PrettyPrinter
from xml.etree.ElementTree import ElementTree
from xml.parsers.expat import ExpatError
try:
        from PhysicsTools.TheNtupleMaker.classmap import ClassToHeaderMap
except:
        pass
#---------------------------------------------------------------------------
if not ('SCRAM_ARCH' in os.environ):
	print("*** SCRAM_ARCH not defined")
	sys.exit("*** need to set up CMS environment: cmsenv")
#---------------------------------------------------------------------------
pp          = PrettyPrinter()
SCRAM_ARCH  = os.environ["SCRAM_ARCH"]
LOCALLIBAREA= "%s/lib/%s/" % (os.environ["CMSSW_BASE"], SCRAM_ARCH)
LIBAREA = "%s/lib/%s/" % (os.environ["CMSSW_RELEASE_BASE"], SCRAM_ARCH)
#--------------------------------------------------------------------------
WEIRD       = '<|<@&@>|>'
AMPERSAND   = '&amp;'
LT          = '&lt;'
GT          = '&gt;'
TILDE       = '&atilde;'
TAB         = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'
NEWLINE     = '<br>'
SPACE       = '&nbsp;'
QUOTE       = '&quot;'
APOSTROPHE  = '&apos;'
NULL        = ''
#---------------------------------------------------------------------------
ALPHA       = re.compile(r'[A-Z]')
findlb      = re.compile(r'\(',re.M)
findrb      = re.compile(r'\)',re.M)
findlt      = re.compile(r'\<',re.M)
findgt      = re.compile(r'\>',re.M)
word        = re.compile(r'\b\w+\b')
swords      = re.compile(r'"[^"]*"|\b\w+\b')
swords2     = re.compile(r'\(\s*\*\s*\w+\s*\)|"[^"]*"|\b\w+\b')
#---------------------------------------------------------------------------
# Regular expressions used for tidying up use of "&", "*", "," "!=" "=" 
# "+", "-" and "/"
#---------------------------------------------------------------------------
adjustamp   = re.compile('(?P<adjust>[ \t]+[&][ \t]*)',re.M)
adjustast2  = re.compile('(?P<adjust>[ \t]+[*][ \t]*[*][ \t]*)',re.M)
adjustast   = re.compile('(?P<adjust>[ \t]+[*](?![*]|/)[ \t]*)',re.M)
adjustcom   = re.compile('(?P<adjust>[ \t]*[,][ \t]*)',re.M)
adjusteql   = re.compile('(?P<adjust>[ \t]+[=][ \t]*)',re.M)
adjustnoteql= re.compile('(?P<adjust>[ \t]+[!][=][ \t]*)',re.M)
adjustplus  = re.compile('(?P<adjust>[ \t]+[+][ \t]*)',re.M)
adjustminus = re.compile('(?P<adjust>[ \t]+[-][ \t]*)',re.M)
adjustdiv   = re.compile('(?P<adjust>[ \t]+[/][ \t]*(?![/]))',re.M)

adjustlt    = re.compile('(?P<adjust>[ \t]+<[ \t]*)',re.M)
adjustlpar  = re.compile('(?P<adjust>\([ \t]*)',re.M)
adjustrpar  = re.compile('(?P<adjust>[ \t]+\))',re.M)
adjustlbra  = re.compile('(?P<adjust>\[[ \t]*)',re.M)
adjustrbra  = re.compile('(?P<adjust>[ \t]+\])',re.M)

adjustptr   = re.compile('(?P<adjust>\*)(?=[A-Za-z]+)',re.M)
adjustref   = re.compile('(?P<adjust>\&)(?=[A-Za-z]+)',re.M)

fixstupid   = re.compile('(?<=\w)(?P<delim>\*\*|\*|&)(?=\w)',re.M)

# Pick up leading blank lines

leadingblanks= re.compile('(?P<leadingblanks>(^[ \t]*\n)+)',re.M)

callback    = re.compile('(?P<callback>([\w\*\&:_]+\s+)+'\
                           '\(\s*(\w+::)?\s*\*\s*\w+\s*\)\s*\(.*\))')

callback2   = re.compile('(?P<callback>([\w\*\&:_]+\s+)+'\
                           '(\(\s*\*\s*)?'\
                           '(\w+::)?(?P<name>\w+)\s*(\)\s*)?\(.*\))')

callbackName= re.compile('(?P<name>\(\s*(\w+::)?\s*\*\s*\w+\s*\))')

stripTypedef= re.compile(r'^\s*\btypedef\s+')
findSpace   = re.compile("\s+",re.M)

# To extract template parameters

templatepar = re.compile('\w[^<]*<(?P<par>[^>]*)>')

templatecmd = '(?P<template>template'\
	'\s*(<.*>|([^\{\n]+\n)+)\s*)'
istemplate  = re.compile(templatecmd)

#---------------------------------------------------------------------------
# fundamental types
#---------------------------------------------------------------------------
FTYPES = '''
char           Char_t;
unsigned char  UChar_t;
short          Short_t;
unsigned short UShort_t;
long           Int_t;       //Signed integer 4 bytes
unsigned long  UInt_t;      //Unsigned integer 4 bytes
int            Int_t;       //Signed integer 4 bytes (int)
unsigned int   UInt_t;      //Unsigned integer 4 bytes (unsigned int)
int            Seek_t;      //File pointer (int)
long           Long_t;      //Signed long integer 8 bytes (long)
unsigned long  ULong_t;     //Unsigned long integer 8 bytes (unsigned long)
int            Seek_t;      //File pointer (int)
long           Long_t;      //Signed long integer 4 bytes (long)
unsigned long  ULong_t;     //Unsigned long integer 4 bytes (unsigned long)
float          Float_t;     //Float 4 bytes (float)
float          Float16_t;   //Float 4 bytes written with a truncated mantissa
double         Double_t;    //Double 8 bytes
double         Double32_t;  //Double 8 bytes, written as a 4 bytes float
char           Text_t;      //General string (char)
bool           Bool_t;      //Boolean (0=false, 1=true) (bool)
unsigned char  Byte_t;      //Byte (8 bits) (unsigned char)
short          Version_t;   //Class version identifier (short)
const char     Option_t;    //Option string (const char)
int            Ssiz_t;      //String size (int)
float          Real_t;      //TVector and TMatrix element type (float)
long long      Long64_t;    //Portable signed long integer 8 bytes
unsigned long long ULong64_t;//Portable unsigned long integer 8 bytes
double         Axis_t;      //Axis values type (double)
double         Stat_t;      //Statistics type (double)
short          Font_t;      //Font number (short)
short          Style_t;     //Style number (short)
short          Marker_t;    //Marker number (short)
short          Width_t;     //Line width (short)
short          Color_t;     //Color number (short)
short          SCoord_t;    //Screen coordinates (short)
double         Coord_t;     //Pad world coordinates (double)
float          Angle_t;     //Graphics angle (float)
float          Size_t;      //Attribute size (float)
'''
FTYPES    = FTYPES.split("\n")
ttypes    = [x.split() for x in FTYPES]
ftypes    = set()
rtypes    = set()
for t in ttypes:
        if len(t) == 0: continue
        fields = []
        for x in t:
                fields.append(x)
                if x.find('_t;') > 0:
                        break

        atype = ' '.join(fields[:-1])
        ftypes.add(atype)
                
        rtype = str.strip(fields[-1].split(';')[0])
        rtypes.add(rtype)
        
ftypes = list(ftypes)
rtypes = list(rtypes)
ftypes.sort()
rtypes.sort()
ftypes = 'size_t|string|std::string|'\
        + '|'.join(ftypes) + '|' + '|'.join(rtypes) 

isFuntype  = re.compile('[0-9]+|void|%s' % ftypes)

# To check for simple types

simpletype = re.compile('(?P<name>\\b(%s)\\b)' % ftypes)

simpletypewithvoid = re.compile('(?P<name>\\b(void|%s)\\b)' % ftypes)

isSTL      = re.compile(r'std::(vector|set|map|pair)')
#----------------------------------------------------------------------------
skipmethod = re.compile(r'TClass|TBuffer|TMember|operator|^__')
reftype    = re.compile(r'(?<=edm::Ref\<std::vector\<)(?P<name>.+?)(?=\>,)')
basicstr   = re.compile(r'std::basic_string\<char\>')
vsqueeze   = re.compile(r'(?<=[^>]) +\>')
skipthis   = re.compile('operator|'\
		        '__get|'\
		        'const_iterator|'\
		        'iterator')
#---------------------------------------------------------------------------
# Given a base class element, return name of base class, minus possible
# template decoration.
#---------------------------------------------------------------------------
getbasename = re.compile(r'(public |private |protected )? *'\
                         '(?P<name>[a-zA-Z].*)')

def extractbaseclassName(e):
	name = e.attrib['name']
	m = getbasename.match(name)
	if m == None: fatal("Can't get name from %s name" % name)
	name = m.group('name')
	return name
#---------------------------------------------------------------------------
# Given template <class X, etc.> extract [X,...]
#---------------------------------------------------------------------------
findtemplatepars = re.compile('(?<=class )(?P<par>\w+)')
def templateParameters(element):
	if element == None: return []
	if type(element) == type(""):
		template = element
	else:
		if not ('template' in element.attrib): return []        
		template = element.attrib['template']
	return findtemplatepars.findall(str.replace(template,
                                                    'typename','class'))

squeezeEq = re.compile('\s*=\s*')
def templateParList(record):
	m = templatepar.search(record)
	if m == None: fatal("templateParList - "\
                            "can't get template parameters from\n\t" + record)
	template = m.group("par")
	template = squeezeEq.sub('=', template)
	template = str.replace(str.replace(template,'class ',''),
                               'typename ','')
	template = [str.strip(str.split(x,'=')[0])
                    for x in str.split(template,',')]
	return template

#---------------------------------------------------------------------------
def nameonly(s):
	return posixpath.splitext(posixpath.split(s)[1])[0]
#---------------------------------------------------------------------------
def decodeTypedef(record):
	record = str.p(stripTypedef.sub("",record))
        # Hack to handle a few crazy typedefs 12/20/09
	i = str.find(record, '(') 
	if i > 0:
		i -= 1
	else:
		i = str.rfind(record,' ')
	if record[-1] == ';':
		key  = str.strip(record[i+1:-1])
	else:
		key  = str.strip(record[i+1:])
	value= standardizeName(str.strip(record[:i]))
	return (key, value)

#---------------------------------------------------------------------------
# Extract (return type, method name, arguments, qual) of a function by
# a static analysis of string.
# This code is beyond ugly, but it seems to work most of the time!
#---------------------------------------------------------------------------
def argOK(field):
	nlb = findlb.findall(field)
	nrb = findrb.findall(field)
	nlt = findlt.findall(field)
	ngt = findgt.findall(field)
	return len(nlb) == len(nrb) and len(nlt) == len(ngt)

def decodeMethodName(method):
        DO = 0

        method = adjustlpar.sub(" (",method)
        method = adjustrpar.sub(") ",method)
        method = adjustlbra.sub(" [",method)
        method = adjustrbra.sub("] ",method)

        # Fix some stupidities
        #(see TIterator*GetIteratorOnAlleaves in TTree.h)

        m = fixstupid.search(method)
        if m != None:
                delim = m.group('delim')
                method= fixstupid.sub(delim+' ', method)

        # Change contiguous newlines etc, into a space

        method = findSpace.sub(' ',method)

        # Split up method/function

        j = str.rfind(method,')')+1
        a = method[:j]
        a = str.replace(a,' [','[')
        a = str.replace(a,' <','<')
        t = str.split(a)
        # Get rid of possible ";"
        qual = str.replace(str.strip(method[j:]),';','') 
        qual = str.replace(qual,'throwPP','throw()')

        # ---------------------------------------
        # Get arguments: I know, I know, this code
        # is hideous in the extreme!
        #
        # First find start of argument list (...)
        # Look out for functions of the form
        # name<...(...) > method(...)
        # ---------------------------------------
        k = -1
        if DO: print(t)
        for n, s in enumerate(t):

                if s[0] == "(":

                        if n > 0:
                                # Do check only for non-ops
                                if str.find(t[n-1],'operator') < -1: 
                                        if str.find(t[n-1],'<') > 0:
                                                if str.find(t[n-1], '>') < 0:
                                                        continue
                        k = n
                        break
        if DO: print(k)

        if k < 0: return ("","","","","")

        t[k] = t[k][1:]
        n = len(t)
        t[n-1] = t[n-1][:len(t[n-1])-1]
        args = ' '.join(t[k:])
        if args == "": args = 'void'

        if DO > 1: print("args( %s )" % args)

        # Split at "," making sure we have valid arguments

        fields = args.split(',')
        if len(fields) > 1:
                field   = fields[0]
                cfields = fields[1:]
                fields  = []
                for i, s in enumerate(cfields):
                        if argOK(field):                
                                fields.append(field)
                                field = s
                        else:
                                field = field + "," + s
                fields.append(field)

        fields = [x.strip() for x in fields]
        if DO:
                print("FIELDS")
                pp.pprint(fields)

        arglist = []
        varlist = []
        for i, field in enumerate(fields):
                # See for example TGFontTypeComboBox
                field = adjustptr.sub("* ", field) 
                field = adjustref.sub("& ", field)

                # WARNING: Changed 30-Oct-2005 *** HBP
                # Use find rather than rfind to avoid problem with
                # arguments such as: const char* training = "Entry$%2==0"

                j = str.find(field,'=') # Check for default parameters
                defaultPar = j > -1
                if defaultPar:
                        a = str.strip(field[:j])
                        value = str.replace(standardizeName(
                                str.strip(field[j+1:])),' (','(')
                else:
                        a = str.strip(field)

                var = ''

                # Check for callbacks

                if callback.match(a) != None:

                        ca = callbackName.sub('CALLB_',a)
                        cretype, cname, carglist, cvarlist, cqual = \
                                decodeMethodName(ca)
                        cname = str.replace(callbackName.search(a).
                                            group('name'),' ','')
                        a = "%s %s(%s) %s" % (cretype,
                                              cname,
                                              ', '.join(carglist,', '),
                                              cqual)
                else:

                        # determine  which field is the variable and which
                        # is the type.
                        #
                        # Yikes...what awful code...!

                        b = str.split(a)
                        n = len(b)
                        if n == 1:
                                a = b[0] # This is easy!
                                var = '' # Variable name
                        else:
                                first = b[0]
                                last  = b[-1]
                                var = last

                                # Could have
                                #[const] type1 [type2] [const(&|*)] [varname]

                                if last[-1] in ['*','&'] or \
                                   last == 'const' or \
                                   simpletype.match(last) != None:
                                        var = ''
                                        pass

                                elif first == 'const':

                                        if n == 2:
                                                # This must be of the form
                                                # const <type>
                                                var = ''
                                                pass

                                        elif simpletype.match(last) != None:
                                                # Last field is a simple type so
                                                # assume it is part of the type
                                                var = ''
                                                pass

                                        else:
                                                # Last field is not a simple
                                                # type. If it ends with
                                                # either & or * assume it
                                                # is part of the type,
                                                # otherwise assume it is
                                                # a name.
                                                if last[-1] in  ['*','&']:
                                                        var = ''
                                                        pass
                                                else:
                                                        # Assume last is name
                                                        a = ' '.join(b[:-1]) 
                                                        var = b[-1]

                                elif simpletype.match(last) == None:
                                        # Assume last is name
                                        a = ''.join(b[:-1]) 
                                        var = b[-1]

                                # Check for array parameter aaa[n].
                                # But note that
                                # aaa could be a type!
                                if str.find(last,'[') > -1 and \
                                   str.find(last,']') > -1:
                                        if a[-1] != ']':
                                                i = str.find(last,'[')
                                                j = str.rfind(last,']')+1
                                                a = a + last[i:j]

                                        if DO:
                                                print("%d, LAST( %s ) -> %s" % \
                                                      (i, last, a))

                a = str.strip(a)

                # Check for defaulted parameter
                if defaultPar:
                        if a == 'char': value = '"%s"' % value[1:-1]
                        a = a + '=%s' % value

                #---------------------------
                # Thank goodness we're done!
                #---------------------------
                arglist.append(a)
                varlist.append(var)

        # check for void
        if len(arglist) > 0:
                if arglist[0] == 'void':
                        arglist = []
                        varlist = []

        if DO: print("ARGLIST( %s )" % arglist)

        # This may be an operator; needs special handling
        q = -1
        for n, s in enumerate(t[:q]):
                if str.find(s,"operator") > -1:
                        q = n
                        break
        if q < 0:
                name  = t[k-1]
                rtype = ' '.join(t[:k-1])
        else:
                name  = ''.join(t[q:k])
                name  = str.replace(name," [","[")
                name  = str.replace(name," (","(")
                rtype = ''.join(t[:q])
        return (rtype,name,arglist,varlist,qual)
#---------------------------------------------------------------------------
# Check for callback
#---------------------------------------------------------------------------
findcbname = re.compile('\(\s*(\w+::)?\s*\*\s*(?P<name>[\w_]+)\)\s*\(')
scrunch    = re.compile(' +(?=(\*|\[))')
def decodeCallback(record):
	if str.find(record,'(') > 0 and str.find(record,')') > 0:
		record = ' '.join(str.split(record),' ')
		record = str.replace(record,'typedef ','')
		record = str.replace(record,';','')
		record = standardizeName(record)
		g = findcbname.search(record)
		if g != None:
			name = g.group('name')
			i = str.find(record, '(')
			j = str.rfind(record, '(')
			record = record[:i] + name + record[j:]
		record = scrunch.sub("", record)
		return decodeMethodName(record)
	return None
#---------------------------------------------------------------------------
getmethodname = re.compile(r"[a-zA-Z][\w\_\<\>:,]*(?=\()")
getmethodargs = re.compile(r"(?<=\().*(?=\))")
def readMethods(txtfilename):
        records = [str.strip(x) for x in open(txtfilename).readlines()]

        methods   = []
        basenames = []
        classname = header = None
        isMethod  = False 
        isBase    = False
        isDatum   = False

        for rec in records:
                t = str.split(rec)

                if len(t) > 0:
                        token = t[0]
                else:
                        token = None

                if   token == "Class:":
                        classname = t[1]

                elif token == "Header:":
                        header = t[1]

                elif token == "BaseClasses:":
                        isBase = True
                        basename = t[1]
                        basenames.append(basename)

                elif token == "AccessMethods:":
                        isMethod = True
                        continue

                elif token == "DataMembers:":
                        isDatum = True
                        continue		

                elif isBase:
                        if rec != "":
                                basenames.append(rec)
                        else:
                                isBase = False

                elif isMethod:
                        if rec != "":
                                t = getmethodname.findall(rec)
                                if len(t) == 0:
                                        continue

                                name  = str.strip(t[0])
                                getmethodrtype = re.compile(r'.*(?=%s\()' % \
                                                            name)
                                rtype = str.strip(getmethodrtype.
                                                  findall(rec)[0])
                                atype = str.strip(getmethodargs.findall(rec)[0])
                                if atype == "": atype = 'void'
                                methods.append((rtype, name, atype, rec))
                        else:
                                isMethod = False

                elif isDatum:
                        if rec != "":
                                t = str.split(rec)
                                if len(t) == 0: continue

                                name  = t[-1]
                                rtype = ' '.join(t[:-1])
                                methods.append((rtype, name, None, rec))
                        else:
                                isDatum = False

        return (header, classname, basenames, methods)
#---------------------------------------------------------------------------
def stripBlanklines(record):
	"""
	Strip away leading and trailng blank lines.
	"""
	m = leadingblanks.match(record) # Important: Use match not search!
	if m:
		record = record[m.end():]
	return str.rstrip(record)
#---------------------------------------------------------------------------
# Tidy up identifiers
#---------------------------------------------------------------------------
def convert2html(record):
	record = str.replace(record,'&', AMPERSAND)
	record = str.replace(record,'<', LT)
	record = str.replace(record,'>', GT)
	record = str.replace(record,'~', TILDE)
	record = str.replace(record,"\t",TAB)
	record = str.replace(record,"\n",NEWLINE)
	record = str.replace(record," ", SPACE)
	record = str.replace(record,'"', QUOTE)
	record = str.replace(record,"'", APOSTROPHE)
	record = str.replace(record,'\0',NULL)
	return record
#---------------------------------------------------------------------------
def convertFromhtml(record):
	record = str.replace(record, AMPERSAND,'&')
	record = str.replace(record, LT,'<')
	record = str.replace(record, GT,'>')
	record = str.replace(record, TILDE, '~')
	record = str.replace(record, TAB,"\t")
	record = str.replace(record, NEWLINE, "\n")
	record = str.replace(record, SPACE, " ")
	record = str.replace(record, QUOTE, '"')
	record = str.replace(record, APOSTROPHE, "'")
	#record = replace(record, NULL, '\0')
	return record
#---------------------------------------------------------------------------
def standardizeName(name):
	# Do more intelligently in a future life!
	name = adjustast2.sub('** ',name) # fix before next line
	name = adjustast.sub('* ',name)
	name = adjustamp.sub('& ',name)
	name = adjustcom.sub(', ',name)
	name = adjusteql.sub('=', name)
	name = adjustplus.sub('+', name)
	name = adjustminus.sub('-', name)
	name = adjustdiv.sub('/', name)
	name = adjustnoteql.sub('!=', name)
	name = adjustlt.sub("<",name)

	name = str.replace(name,"* )", "*)")
	name = str.replace(name,"& )", "&)")
	name = str.replace(name,"* >", "*>")
	name = str.replace(name,"& >", "&>")
	name = str.replace(name,"< ",  "<")
	return str.strip(name)
#---------------------------------------------------------------------------
def standardizeOpName(name):
	i = str.find(name,'operator')
	if i < 0: return name
	j = str.find(name[i:],'(') + i
	astr = name[i+8:j]
	if word.search(astr) == None:
		name = name[:i] + 'operator%s' % str.strip(astr) + name[j:]
	return name
#---------------------------------------------------------------------------
# Read an XML file .... duh!
#---------------------------------------------------------------------------
rowcol = re.compile('line (?P<row>[0-9]+), column (?P<col>[0-9]+)',re.M)
def readXmlFile(filename):
	try:
		return ElementTree(file=filename).getroot()
	except:
		return None
#---------------------------------------------------------------------------
def validateXmlFile(filename):
	try:
		ElementTree(file=filename).getroot()
		return ''
	except ExpatError as error:
		errStr = "\n\tBadly formed XML in file\n\t%s" % filename
		astr   = "\t\t%s\n" % error
		errStr = errStr + "\n" + astr
		m = rowcol.search(astr)
		if m != None:
			row = m.group('row')
			if row != None:
				row = int(row)
				records = open(filename).readlines()
				startrow = max(0,row-3)
				endrow   = row
				for record in records[startrow:endrow]:
					errStr = errStr + "\n" + \
                                                str.rstrip(record)
				col = m.group('col')
				if col != None:
					col = atoi(col)-1
					errStr = errStr + "\n" + \
                                                ' '*col + '^'
		return errStr
	except:
		# Fall on sword
		fatal("validateXMLFile - "\
                      "Unknown problem with XML file\n\t%s" % \
		      filename)
#---------------------------------------------------------------------------
# Make a key by stripping away qualifiers such as 'const', '*', etc.
#---------------------------------------------------------------------------
makekey = re.compile(r'\bstruct\b|\bconst\b|\bstatic\b|\bvirtual\b|[&]|[*]|')
def stripName(arg, regex=makekey):
	key = str.split(arg,'=')[0] # In case this comes with a value
	return str.strip(regex.sub('', key))
#---------------------------------------------------------------------------
# Rename a templated identifier to a valid Python identifier
#---------------------------------------------------------------------------
findvector = re.compile(r'\bvector\b')
findlist   = re.compile(r'\blist\b')
findmap    = re.compile(r'\bmap\b')
findarray  = re.compile(r'\barray\b')
findpair   = re.compile(r'\bpair\b')
findset    = re.compile(r'\bset\b')
findconst  = re.compile(r'\bconst\b')
findunsigned  = re.compile(r'\bunsigned\b')
findcontainer=re.compile(r'\b(?P<name>vector|list|map|array|pair|set)\b')
cstring    = re.compile(r'(\bconst )?char *\*')
#---------------------------------------------------------------------------
def renameIdentifier(identifier):
	identifier = cstring.sub("cstring", identifier)
	identifier = str.replace(identifier,'std::','')
	identifier = str.replace(identifier,'boostutil::','')
	identifier = str.replace(identifier,'::','_')
	#identifier = findunsigned.sub('u', identifier)
	i = str.find(identifier,'<')
	j = str.rfind(identifier,'>')

	if  i < j and i > -1:
		left   = identifier[:i]
		middle = identifier[i:j+1]
		right  = identifier[j+1:]

		left = stripName(left)        
		left = findvector.sub('v', left)
		left = findlist.sub('l', left)
		left = findmap.sub('m', left)
		left = findarray.sub('a', left)
		left = findpair.sub('p', left)

		middle = findconst.sub('c', middle)
		middle = findvector.sub('v', middle)
		middle = findlist.sub('l', middle)
		middle = findmap.sub('m', middle)
		middle = findarray.sub('a', middle)
		middle = findpair.sub('p', middle)

		middle = replace(middle,'<','')
		middle = replace(middle,'>','')
		middle = replace(middle,',','_')
		middle = replace(middle,'*','ptr')
		middle = replace(middle,'&','ref')

		identifier = left + middle        
		identifier = ''.join(str.split(identifier))
	else:
		m = findcontainer.search(identifier)
		if m != None:
			identifier = findcontainer.sub(m.group('name')[0], \
                                                       identifier)
	return identifier
#---------------------------------------------------------------------------
def expandpath(astr):
	return str.strip(os.popen("find %s -maxdepth 0 2>/dev/null" % \
                                  astr).read())
#---------------------------------------------------------------------------
def libExists(libdirs, lib):
	path = ''
	for ldir in libdirs:
		fullname = "%s/%s" % (ldir, lib)
		if expandpath(fullname) != '': return True
	return False
#---------------------------------------------------------------------------
striplib = re.compile(r'\.so.*$')
def libDependencies(libpaths, libdirs, lib):

	# Get pathname of library

	k = 0
	for i, lpath in enumerate(libpaths):
		k = i
		path = "%s/%s" % (lpath, lib)
		if os.path.exists(path):
			break
		else:
			path = ''
	if path == '': return ([], [])

	# Get lib dependencies

	command = "symlist.py -l %s" % path
	records = os.popen(command).readlines()

	ldflags = [libdirs[k]]
	liblist = []
	for record in records:
		t = str.split(record)
		if len(t) != 2: continue
		lib, area = t
		if area[0:4] == '/lib': continue
		if area[0:8] == '/usr/lib': continue

		k = 0
		for i, lpath in enumerate(libpaths):
			k = i
			path = "%s/%s" % (lpath, lib)
			if os.path.exists(path):
				break
			else:
				path = ''
		if path == '':
			print("** Warning: %s not along "\
                              "any path given in LibDirs" % lib)
			continue

		lib = striplib.sub("", lib[3:])

		uniqueList(ldflags, libdirs[k])
		uniqueList(liblist, lib)

	return (ldflags, liblist)
#---------------------------------------------------------------------------
def unique(alist, value):
	if not value in alist: alist.append(value)
def uniqueList(alist, value):
	if not value in alist: alist.append(value)
#---------------------------------------------------------------------------
# Look for files on given list of search paths
#---------------------------------------------------------------------------
def findFile(infile, incs):
	afile = strip(os.popen("find %s 2>/dev/null" % infile).readline())    
	if afile != "":
		filepath = afile
		for includepath in incs:
			tmp = filepath.split(includepath + '/', 1)
			if len(tmp) == 2:
				base, afile = tmp
			else:
				base = ''
				afile = tmp[0]

			if base == '': return (filepath, afile)
		return (filepath, filepath)

	afile = infile
	for includepath in incs:
		filepath = includepath + "/" + afile
		filepath = str.strip(os.popen("find %s 2>/dev/null" % \
                                              filepath).readline())
		if filepath != "": return (filepath, afile)

		filepath = includepath + "/include/" + afile
		filepath = str.strip(os.popen("find %s 2>/dev/null" % \
                                              filepath).readline())
		if filepath != "": return (filepath, afile)

	return ("","")
#---------------------------------------------------------------------------
def findHeaders(name):
	hdrs = {}
	pkgs = {}
	# break name into words, but skip fundamental types
	# and the main STL types
	words = getwords.findall(name)
	classlist = []
	for word in words:
		if isFuntype.match(word) != None:
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
		if cname in ClassToHeaderMap:
			header = ClassToHeaderMap[cname]
			if type(header) == type([]): header = header[0]
			if not header in headers:
				headers.append(header)
		elif isFuntype.match(cname) != None:
			continue
	return headers
#---------------------------------------------------------------------------
# Extract method signature (name + args)
#---------------------------------------------------------------------------
def methodSignature(classname,record):
	methtype,name,args,vars,qual = decodeMethodName(record)
	return "%s::%s(%s)" % (classname, name, ', '.join(args))

def methodFullSignature(record):
	methtype,name,args,vars,qual = decodeMethodName(record)
	astr = "%s %s(%s) %s" % (methtype, name, ', '.join(args), qual)
	return strip(astr)

#---------------------------------------------------------------------------
# Get fully qualified name of class
def getFullname(classname):
    t = ROOT.TClass(classname, ROOT.kTRUE)
    #t = Reflex.Type.ByName(classname)
    #cname = t.FinalType().Name(7)
    cname = t.GetName()
    if cname != "": classname = cname
    return classname
#---------------------------------------------------------------------------
# Expand macros then environment variables then shell commands
#---------------------------------------------------------------------------
envar = re.compile('(?P<env>[$]\w+)',re.M)
shell = re.compile('(?P<shell>[$]\(shell [^\)]+\))',re.M)
def expandNames(record, macros={}, what=1):

	# Expand macros

	for key, value in macros.items():
		record = str.replace(record,'$(%s)' % key, value)

	if what < 2: return record

	# Expand environment variables

	tokens = []
	for astr, group, start, end in findAll(envar,record):
		env = astr[1:]
		if env in os.environ:
			tokens.append((os.environ[env],start,end))
	if len(tokens) > 0:
		record = splice(record, tokens,'') # Suppress newline

	if what < 3: return record

	# Expand shell commands

	tokens = []
	for astr,group,start,end in findAll(shell,record):
		command = astr[7:len(astr)-1]
		stdout, stdin, stderr = popen3(command)            
		errors = str.strip(stderr.read())
		if len(errors) == 0:
			tokens.append((stdout.read(),start,end))
                        # Suppress newline
	if len(tokens) > 0: record = splice(record, tokens,'')
	return record
#---------------------------------------------------------------------------
def hidepath(afile, pathmap=None):
	if pathmap == None:
		pathmap = os.environ
		prefix = '$'
	else:
		prefix = ''

	key = ''
	value = ''
	for k, v in pathmap.items():
		if v == '': continue
		if v == '.': continue
		if k == 'PWD': continue

		if v == afile[:len(v)]:
			if len(v) > len(value):
				key = k
				value = v
	if key == '': return afile
	n = len(value)
	return '%s%s%s' % (prefix, key, afile[n:])
#---------------------------------------------------------------------------
# Look for classes in current element and recursively within each class
#---------------------------------------------------------------------------
def findClassElements(elem, classes, depth=0):
	if elem == None: return

	depth += 1
	if depth >= 20:
		fatal("lost in tree...I'm bailing!\n\t\t%s" % elem.tag)

	# Loop over elements

	for element in elem.getchildren():
		if element.tag == 'namespace':
			findClassElements(element, classes, depth)
		elif element.tag == 'section':
			findClassElements(element, classes, depth)
		elif element.tag == 'class':
			classes.append(element)
			findClassElements(element, classes, depth)
	return
#---------------------------------------------------------------------------
# Look for typedefs in current element and recursively
#---------------------------------------------------------------------------
def findTypedefElements(elem, elements, depth=0, namespace=[]):
	depth += 1
	if depth >= 20:
		fatal("lost in tree...I'm bailing!\n\t\t%s" % elem.tag)

	# Loop over elements

	for element in elem.getchildren():
		if element.tag == 'namespace':
			if element.attrib.has_key('name'):
				namespace.append(element.attrib['name'])

			findTypedefElements(element, elements, depth, namespace)

			if element.attrib.has_key('name'):
				namespace.pop()

		elif element.tag == 'class':
			namespace.append(element.attrib['name'])

			findTypedefElements(element, elements, depth, namespace)

			namespace.pop()

		elif element.tag == 'section':

			findTypedefElements(element, elements, depth,
                                            namespace)          

		elif element.tag == 'typedef':
			key, value = decodeTypedef(element.text)
			namespace.append(key)
			fullname = joinfields(namespace,'::')
			namespace.pop()
			element.attrib = {'name':key, 'fullname': fullname,
                                          'value':value}
			elements.append(element)
	return
#---------------------------------------------------------------------------
# Look for namespaces
#---------------------------------------------------------------------------
def listNamespaces(elem, names, depth=0, namespace=[]):
        depth += 1
        if depth >= 20:
                fatal("lost in tree...I'm bailing!\n\t\t%s" % elem.tag)

        # Loop over elements

        for element in elem.getchildren():
                if element.tag == 'namespace':
                        if 'name' in element.attrib:

                                namespace.append(element.attrib['name'])
                                astr = joinfields(namespace,'::')
                                if not astr in names: names.append(astr)

                        listNamespaces(element, names, depth, namespace)

                        if 'name' in element.attrib:
                                namespace.pop()
        return
#---------------------------------------------------------------------------
def baseclassNames(element):
	bases = element.findall('baseclass')
	nlist = []
	for e in bases:
		name = extractbaseclassName(e)
		nlist.append(name)
	return nlist
#---------------------------------------------------------------------------
DEBUG1 = 0
stripincdir    = re.compile(r'interface/|include/')
striptemplate  = re.compile(r'\<.*$')
def extractHeaders(BASE, include, db, depth=0):

	if include in db['include']: return

	# consider only DataFormat includes

	if str.find(include, 'DataFormats') < 0: return

	depth += 1
	if depth > 50:
		print("extractHeaders: woe is me..lost in trees!")
		return
	tab = (depth-1)*'  '


	##D
	if DEBUG1 > 0: print("%sINCLUDE( %s )" % (tab, include))

	# construct xml filename from that of include

	xmlstr = stripincdir.sub("xml/", str.split(include, ".")[0])
	xmlfile= "%s/src/%s.xml" % (BASE, xmlstr) 

	# get associated xml object

	if not os.path.exists(xmlfile):
		##D
		if DEBUG1 > 0: print("\t\tNOT found")
		return

	##D
	if DEBUG1 > 0: print("%s  XMLFILE( %s )" % (tab, xmlfile))

	# xml file exists - get xml object

	headerxml = readXmlFile(xmlfile)
	if headerxml == None:
		print("\t\t** can't read xml file: %s" % xmlfile)
		return

	classes = []
	findClassElements(headerxml, classes)
	for e in classes:
		fname  = e.attrib['fullname']
		fname  = striptemplate.sub("", fname)
		if fname in db['class']: continue

		if DEBUG1 > 0: print("%s    CLASS( %s )" % (tab, fname))
		db['class'][fname] = (include, e)

		# Get methods of current class

		if not (fname in db['method']): db['method'][fname] = []

		sections = e.findall('section')
		for s in sections:
			if 'name' in s.attrib:
				if s.attrib['name'] in \
                      ['private','protected']: continue

			# Get methods
			meths = s.findall("method1") + s.findall("method2")
			for m in meths: db['method'][fname].append(m)

	# Get typedefs

	if not (include in db['typedef']): db['typedef'][include] = {}
	telem = []
	findTypedefElements(headerxml, telem)
	for e in telem:
		name  = e.attrib['name']
		if name in db['typedef'][include]: continue
		value = e.attrib['value']
		db['typedef'][include][name] = value
		if DEBUG1 > 0:
			print("%s      TYPEDEF( %s -> %s )" % \
                              (tab, name, value))

	db['include'][include] = (headerxml, classes)

	# Recurse!
	for e in headerxml.findall('include'):
		include = e.attrib['name'][1:-1]
		if include in db['include']: continue
		if str.find(include, 'DataFormats') < 0: continue

		extractHeaders(BASE, include, db, depth)		
#---------------------------------------------------------------------------
def populateDB(BASE, include, db):
	if DEBUG1 > 0:
		print("\nBEGIN( populateDB ) %s" % include)


	# Make sure xml database is initialized

	if not ('headerlist' in db): db['headerlist'] = []
	if not ('include'    in db): db['include'] = {}
	if not ('class'      in db): db['class']   = {}
	if not ('typedef'    in db): db['typedef'] = {}
	if not ('method'     in db): db['method']  = {}

	extractHeaders(BASE, include, db)

	# Loop over headers and get typedefs for each

	includes = list(db['include'].keys())
	for include in includes:

		headerxml, classes = db['include'][include]

		# Expand typedefs by including those from
		# headers within current header

		inclist = headerxml.findall('include')
		for incxml in inclist:
			inc = incxml.attrib['name'][1:-1]
			if not (inc in db['include']): continue
			hdr, cls = db['include'][inc]
			telem = []
			findTypedefElements(hdr, telem)
			for e in telem:
				name  = e.attrib['name']
				if name in db['typedef'][include]: continue
				value = e.attrib['value']
				db['typedef'][include][name] = value

#===========================================================================
# Heuristic regular expressions
#
# Notes:
# 1. Use named groups (?P<groupname>....) so that we can retrieve matched
#    strings by group name.
#
# 2. Use lookaheads (?=...), (?!...) and lookbehinds (?<=...), (?<!...)
#    to simplify regexes and make matches more robust. Note: lookaheads and
#    lookbehinds as zero width matches so they do not consume characters.
#
# 3. Occasionally, we use raw strings r'....' to make the escaping of
#    special characters less of a chore.
#===========================================================================
# Exceptions
#---------------------------------------------------------------------------
MAXCONSTRUCT=5000

class ImConfused(Exception):
	def __init__(self, msg):
		self.msg = msg

	def __str__(self):
		return self.value

PLACEHOLDERS =['namespace',
	       'endnamespace',
	       'class',
	       'endclass',
	       'structclass',
	       'endstructclass',
	       'inlineclass',
	       'inlinestructclass']
#---------------------------------------------------------------------------
beginnamespace= '(?P<namespace>' \
			   '\\bnamespace\\b\s*(\w+)?\s*[^\{]*(?=\{)\{)'

endnamespace  = '(?P<endnamespace>\}(?!;))'


## templatecmd  = '(?P<template>template'\
## 			   '\s*(<.*>|([^\{\n]+\n)+)\s*)'
## istemplate   = re.compile(templatecmd)

specializedtemplatecmd  = '(?P<template>template\s*<\s*>\s*)'
isspecializedtemplate   = re.compile(specializedtemplatecmd)

begintemplate= '(?P<item>^template([^\{]+)+\s*(?=\{)\{)'

beginclass   = '(?P<class>' \
			   '^[ \t]*%s?' \
			   '(class|struct)[ \t]+[^;\(\{]+(?=\{)\{)' % templatecmd

endclass     = '(?P<endclass>\};)'

struct       = '(?P<struct>^[ \t]*struct'   \
			   '(\s+\w+)?\s*'   \
			   '\{[^\}]+\}'     \
			   '(\s*\w+)?[ \t]*;)'
#===========================================================================
# Compiled regular expressions
#===========================================================================

#leadingblanks= re.compile('(?P<leadingblanks>(^[ \t]*\n)+)',re.M)

# Return class preambles (that is, classs <name>.... {)

beginclasses = re.compile(beginclass,re.M)

# Return template preambles

begintemplates = re.compile(begintemplate,re.M)

# Return namespace preambles (that is, namespace <name> {)

namespaces = re.compile(beginnamespace,re.M)

classtitle = re.compile('%s?'\
			'\s*(class|struct)\s+' \
			'(?P<classtitle>[_a-zA-Z]\w*[^\{]*\{)' % \
			templatecmd, re.M)

classtype  = re.compile('(?P<classtype>\\b(class|struct)\\b)',re.M)

# Return string containing base classes

basenames  = re.compile('(?P<basenames>' \
		       '\s*(?<!:):(?!:)\s*(public|private|protected)?[^\{]+)')

findInlineComments      = re.compile('[ \t]*//[^\n]*|[ \t]*/[*].*(?=\*/)\*/')
getclassname = re.compile(r"\bclass[ \t]+(?P<name>\w+)[ \t]*;",re.M)
#===========================================================================
# Functions
#===========================================================================
def fatal(s):
	print("** Error ** %s" %s)
	sys.exit("\tgoodbye!")
## #---------------------------------------------------------------------------
## # Use (homegrown) CPP to clean-up header before parsing
## #---------------------------------------------------------------------------
## cpp_namespace  = '^ *namespace +[a-zA-Z0-9]+\s*{'
## cpp_tclassname = '^ *template *<[^>]+>\s*class +\w+\s*\w*\s*[^{;]+{'
## cpp_classname  = '^ *class +\w+\s*\w*\s*[^{;]+{'

## cpp_tstructname= '^ *template +<[^>]+>\s*struct +\w+\s*\w*\s*[^{;]+{'
## cpp_structname = '^ *struct +\w+\s*\w*\s*[^{;]+{'
## cpp_typedef    = '^ *typedef +\w+\s*\w*\s*[^;]+'
## cpp_leftbrace  = '{'
## cpp_rightbrace = '};|}'

## cpp_stripbodies= re.compile('(?<={|})\s*{\s*};?', re.M)

## cpp_regex = joinfields([cpp_namespace,
## 						cpp_tclassname,
## 						cpp_classname,
## 						cpp_tstructname,
## 						cpp_structname,
## 						cpp_typedef,
## 						cpp_leftbrace,
## 						cpp_rightbrace],'|')
## cpp_search= re.compile(cpp_regex, re.M)

## gettypedefs  = re.compile('^ *typedef +\w+ *\w* *.*; *$', re.M)
## gettypedefs  = re.compile('^ *typedef .*$', re.M)
## gettypedefs  = re.compile('^ *typedef [a-zA-Z0-9:_!=, ]+$', re.M)
## skipdefs     = re.compile('typename|iterator|_type')

## # Find different comment styles

## # C++-style
## scomment2    = '(?P<scomment2>(^[ \t]*///(?!/)[^\n]*\n))'
## scomment3    = '(?P<scomment3>(^[ \t]*//(?!/)[^\n]*\n)+)'

## # C-style
## ccomment     = '(?P<ccomment>^[ \t]*/[*].+?(?=[*]/)[*]/(?! \)))'

## # Doxygen-style
## ocomment     = '(?P<ocomment>^[ \t]*///[^\n]+?\n[ \t]*/[*][*].+?(?=[*]/)[*]/)'

## groups = (ocomment,scomment3,scomment2,ccomment)
## format = (len(groups)-1)*'%s|'+'%s'
## cpp_stripcomments = re.compile(format % groups,re.M+re.S)
## cpp_stripinlinecomments = re.compile('//.*\n|/\*\*.*\*/\n', re.M)
## cpp_stripbodies   = re.compile('(?<={|})\s*{\s*};?', re.M)
## cpp_stripincludes = re.compile('^#.*\s*', re.M)
## cpp_stripstrings  = re.compile('"[^"]+"\s*', re.M)

## cpp_findweird     = re.compile('(class|struct) +(?P<weird>\w+\s+)\w+')
## cpp_doublecolon   = re.compile('::')
## cpp_singlecolon   = re.compile('(?<!:):(?!=:)')
## cpp_rbracescolon  = re.compile('\} *;')
## cpp_rightbrace    = re.compile('\} ')
## cpp_rightangle    = re.compile('\> (?=[a-zA-Z])')
## cpp_abstract      = re.compile('\bvirtual .*\) *= *0 *;')

## def cpp(record, items):
	
## 	record = cpp_stripcomments.sub("", record)
## 	record = replace(record, "\\\"","")
## 	record = cpp_stripinlinecomments.sub("", record)
## 	record = cpp_stripincludes.sub("",record)
## 	record = cpp_stripstrings.sub("", record)

## 	record = joinfields(split(replace(record, '\n', ' ')), ' ')
## 	record = cpp_rbracescolon.sub("};", record)
## 	record = replace(record,';',';\n')
## 	record = cpp_rightbrace.sub("}\n\n", record)
## 	record = replace(record,'{', '{\n')
## 	record = cpp_doublecolon.sub("@@", record)
## 	record = cpp_singlecolon.sub(":\n", record)
## 	record = replace(record, "@@", "::")
## 	record = cpp_rightangle.sub(">\n", record)

## 	results = map(lambda x: strip(replace(x,'\n',' ')),
## 				  cpp_search.findall(record))

## 	record = ''
## 	col = 0
## 	previous = ''
## 	for result in results:
## 		if find(result, 'class') > -1 or find(result, 'struct') > -1:
## 			m = cpp_findweird.search(result)
## 			if m != None:
## 				weird = m.group('weird')
## 				result = replace(result, weird, '')

## 		if find(result, '{') > -1:
## 			if previous == '{':
## 				col += 1
## 			previous = '{'
## 		elif find(result, '}') > -1:
## 			if previous == '}':
## 				col -= 1
## 			previous = "}"
## 		tab = '  '*col
## 		record += "%s%s\n" % (tab, result)

## 	newrecord = ''
## 	count = 0
## 	while (newrecord != record) and (count < 10):
## 		if newrecord != '': record = newrecord
## 		newrecord = cpp_stripbodies.sub("", record)
## 		count += 1
## 	record = newrecord
## 	return record

#---------------------------------------------------------------------------
# Use (homegrown) CPP to clean-up header before parsing
#---------------------------------------------------------------------------
cpp_namespace  = '^ *namespace +[a-zA-Z0-9]+\s*{'
cpp_tclassname = '^ *template *<[^>]+>\s*class +\w+\s*\w*\s*[^{;]+{'
cpp_classname  = '^ *class +\w+\s*\w*\s*[^{;]+{'

cpp_tstructname= '^ *template +<[^>]+>\s*struct +\w+\s*\w*\s*[^{;]+{'
cpp_structname = '^ *struct +\w+\s*\w*\s*[^{;]+{'
cpp_typedef    = '^ *typedef +\w+\s*\w*\s*[^;]+'
cpp_leftbrace  = '{'
cpp_rightbrace = '};|}'

cpp_stripbodies= re.compile('(?<={|})\s*{\s*};?', re.M)

cpp_regex = '|'.join([cpp_namespace,
		      cpp_tclassname,
		      cpp_classname,
		      cpp_tstructname,
		      cpp_structname,
		      cpp_typedef,
		      cpp_leftbrace,
		      cpp_rightbrace])
cpp_search= re.compile(cpp_regex, re.M)

gettypedefs  = re.compile('^ *typedef +\w+ *\w* *.*; *$', re.M)
gettypedefs  = re.compile('^ *typedef .*$', re.M)
gettypedefs  = re.compile('^ *typedef [a-zA-Z0-9:_!=, ]+$', re.M)
skipdefs     = re.compile('typename|iterator|_type')

# Find different comment styles

# C++-style
scomment2    = '(?P<scomment2>(^[ \t]*///(?!/)[^\n]*\n))'
scomment3    = '(?P<scomment3>(^[ \t]*//(?!/)[^\n]*\n)+)'

# C-style
ccomment     = '(?P<ccomment>^[ \t]*/[*].+?(?=[*]/)[*]/(?! \)))'

# Doxygen-style
ocomment     = '(?P<ocomment>^[ \t]*///[^\n]+?\n[ \t]*/[*][*].+?(?=[*]/)[*]/)'

groups = (ocomment,scomment3,scomment2,ccomment)
format = (len(groups)-1)*'%s|'+'%s'
cpp_stripcomments = re.compile(format % groups,re.M+re.S)
cpp_stripinlinecomments = re.compile('//.*\n|/\*\*.*\*/\n', re.M)
cpp_stripbodies   = re.compile('(?<={|})\s*{\s*};?', re.M)
cpp_stripincludes = re.compile('^#include .*\s*', re.M)
cpp_stripstrings  = re.compile('"[^"]+"\s*', re.M)

cpp_findweird   = re.compile('(class|struct) +(?P<weird>\w+\s+)\w+')

def cpp(record, items):
	record = cpp_stripcomments.sub("", record)
	record = str.replace(record, "\\\"","")
	record = cpp_stripinlinecomments.sub("", record)
	record = cpp_stripincludes.sub("",record)
	record = cpp_stripstrings.sub("", record)
	record = str.replace(record,'{','{\n')
	results= [str.strip(str.replace(x,'\n',' ')) for x in \
		   cpp_search.findall(record)]
	record = ''
	col = 0
	previous = ''
	for result in results:
		if str.find(result, 'class') > -1 or \
                      str.find(result, 'struct') > -1:
			m = cpp_findweird.search(result)
			if m != None:
				weird = m.group('weird')
				result = str.replace(result, weird, '')

		if str.find(result, '{') > -1:
			if previous == '{':
				col += 1
			previous = '{'
		elif str.find(result, '}') > -1:
			if previous == '}':
				col -= 1
			previous = "}"
		tab = '  '*col
		record += "%s%s\n" % (tab, result)
	newrecord = ''
	count = 0
	while (newrecord != record) and (count < 10):
		if newrecord != '': record = newrecord
		newrecord = cpp_stripbodies.sub("", record)
		count += 1
	record = newrecord
	return record
#---------------------------------------------------------------------------
# Class to write out stuff in XML...kinda obvious huh!
#---------------------------------------------------------------------------
def namespaceName(record):
	name = record.split('{', 1)[0]
	name = str.strip(str.replace(name,'namespace',''))
	return name
#---------------------------------------------------------------------------
# Splice together a list of strings at specified locations within a record
#---------------------------------------------------------------------------
def splice(record, strlist, newline='\n'):
	records = []
	ii = 0
	for astr, start, end in strlist:
		jj = start
		if jj >= 0:
			s  = record[ii:jj]
			records.append(s)
                # Important for header2xml!
		records.append('%s%s' % (astr,newline)) 
		ii = end        
	if ii < len(record):
		s  = record[ii:]
		records.append(s)
	return ''.join(records)
#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
# This is the most important routine. If it breaks, nothing will work!
# It is used to find constructs of the form
#
#      <Identifier ...> <leftDelim> .... <rightDelim>
#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
def findComponents(regex, record, leftDelim, rightDelim,
		   same=1, leftCount=0, rightCount=0):
        
	if same:
		groups = findAllSame(regex, record)
	else:
		groups = findAll(regex, record)
                
        
	clist = []
	for astr, group, start, end in groups:
		count = 0
		iii   = start
		nr    = len(record)
		n     = 0
		first = 1
		while iii < nr:
			n += 1
			if n > MAXCONSTRUCT:
				astr = "\tcan't find the end of construct!"
				#print astr
				raise ImConfused(astr)

			# Search for nearest left or right delimeter
			l = str.find(record[iii:],leftDelim)
			r = str.find(record[iii:],rightDelim)
			if l < 0: l = nr
			if r < 0: r = nr
			k = min(l,r)
			iii = iii + k + 1

			if k < nr:
				if k == l:
					count += 1
					if first:
						first = 0
                                                # Location of leftmost delimeter
						left  = iii-1 

				if k == r:
					count -= 1
                                        # Location of rightmost delimeter
					right = iii       

				if count == 0:
					start = max(start - leftCount, 0)
					end   = min(iii + rightCount, nr)
					clist.append((record[start:end],
                                                      group,
						      start,
                                                      left,right,end))
					break
			else:
				message = "\tboundary mismatch ..I'm bailing\n"
				print(message, "\t", astr)
				raise ImConfused(message)

	return clist

def findClasses(record):
	return findComponents(beginclasses,record,'{','}',1,0,1)

def namespaceNames(record):
	return findComponents(namespaces,record,'{','}')
#---------------------------------------------------------------------------
bodies      = re.compile('(?P<body>\{)',re.M)
def findBodies(record):
	return findComponents(bodies,record,'{','}',0)

def parseFunctionBodies(record, items):
	bodybnds = findBodies(record)
	skip = len(bodybnds)*[0]
	for i, (astr, group, start, left, right, end) in enumerate(bodybnds):
		for j, (s, g, a, l, r, b) in enumerate(bodybnds):
			if (start < a) and (b < end):
				skip[j] = 1
	tokens = []
	for i, (astr, group, start, left, right, end) in enumerate(bodybnds):
		if skip[i]: continue
		tokens.append(('{}',start,end))
	if len(tokens) > 0:
		record = splice(record, tokens)
	return record
#---------------------------------------------------------------------------
# Find all strings that satisfy given regular expression.
#---------------------------------------------------------------------------
def findAll(regex,s):
	iterator = regex.finditer(s)
	strlist = []
	for m in iterator:
		keylist = []
		for key in m.groupdict():
			if m.groupdict()[key] != None:
				keylist.append(key)
				if len(keylist) > 1:
					fatal("double match \n%s" % keylist)
				record = str.rstrip(m.group())
				record = stripBlanklines(record) 
				strlist.append((record,key,m.start(),m.end()))
	return strlist

def findAllSame(regex,s):
	iterator = regex.finditer(s)
	strlist = []
	for m in iterator:
		s = (str.rstrip(m.group()),
                     m.groupdict().keys()[0],
                     m.start(),m.end())
		strlist.append(s)
	return strlist
#===========================================================================
# Parse header and try to identify fully scoped class names
#===========================================================================
def parseHeader(file):
	record = str.strip(open(file).read())

	# ---------------------------------------------------
	# Clean up with CPP
	# ---------------------------------------------------
	items = {}
	record = cpp(record, items)
        
	# ---------------------------------------------------
	# Change format of typedefs so that they can be
	# processed the same way as classes
	# ---------------------------------------------------
	tdefs = gettypedefs.findall(record)
	for td in tdefs:
		td = str.strip(td)
		if skipdefs.search(td) != None: continue

		t = str.split(td)
		rec = "\tclass %s {\n\t};" % t[-1]
		record = str.replace(record, td, rec)

## 	# ---------------------------------------------------
## 	# Change format of typedefs so that they can be
## 	# processed the same way as classes
## 	# ---------------------------------------------------
## 	tdefs = gettypedefs.findall(record)
## 	for td in tdefs:
## 		td = strip(td)
## 		if skipdefs.search(td) != None: continue

## 		t = split(td)
## 		tname = joinfields(t[:-1], ' ')+'@' + t[-1]
## 		rec = "\tclass %s {\n\t};" % tname
## 		record = replace(record, td, rec)

	# ---------------------------------------------------
	# Find namespace preambles and ends and replace them.
	# Since these could be nested we need to order the
	# identified constructs before replacing them in
	# record
	# ---------------------------------------------------
	try:
		record = parseNamespaceBoundaries(record, items)
	except:
		return ('',{})
        
	# ---------------------------------------------------    
	# Find class preambles and ends and replace with
	# placeholders. Like namespaces, make sure we sort
	# them in case we have nested classes.
	# ---------------------------------------------------    
	oldrecord = record
	try:
		record = parseClassBoundaries(record, items)
	except:
		return ('', {})
	if record == '': record = oldrecord

	return (record, items)
#---------------------------------------------------------------------------
#------------------------------------ FUNCTIONS USED BY parseHeader --------
#---------------------------------------------------------------------------
def placeHolder(str, n):
	return "\%s%3.3d" % (str,n)
#--------------------------------------------------------
# Apply an OR of the regular expressions to the record
# and return all groups that match in the order in which
# they match.
#--------------------------------------------------------
def splitHeader(record):
	# First find placeholders then look for remaining groups
	# Search for "member" last
	exp = ""
	for item in PLACEHOLDERS:
		exp = exp + '(?P<%s>\\\%s[0-9]{3})|' % (item,item)
	regex  = re.compile(exp,re.M+re.S)
	return findAll(regex, record)

# Compare function for ordering nested constructs

def groupcmp(x, y):
	a = x[2]
	b = y[2]
	if a < b:
		return -1
	elif a > b:
		return 1
	else:
		return 0
#--------------------------------------------------------
# Find namespace preambles and ends and replace them.
# Since these could be nested we need to order the
# identified constructs before replacing them.
#--------------------------------------------------------        
def parseNamespaceBoundaries(record, items):
	namespbnds = namespaceNames(record)
	namespacebnds = []
	tokens = []
	for i, (astr, group, start, left, right, end) in \
                      enumerate(namespbnds):
		endpreamble = left + 1
		endbody     = right- 1
		preamble= stripBlanklines(str.rstrip(record[start:endpreamble]))
		finale      = record[endbody:end]
		namespacebnds.append((preamble,
                                      'namespace',start,endpreamble))
		namespacebnds.append((finale,
                                      'endnamespace',endbody,end))

	namespacebnds.sort(groupcmp)
	tokens = []
	for i, (astr, group, start, end) in enumerate(namespacebnds):
		placeholder = placeHolder(group, i)
		if group == 'endnamespace':
                        placeholder = '\n%s' % placeholder
		items[placeholder] = astr        
		tokens.append((placeholder,start,end))
	if len(tokens) > 0:
		record = splice(record, tokens)

	return record
#--------------------------------------------------------
# Find class preambles and ends and replace with
# placeholders. Like namespaces, make sure we sort
# them in case we have nested classes.
#--------------------------------------------------------
def parseClassBoundaries(record, items):
	classbnds = findClasses(record)
	if len(classbnds) == 0:
		return ''
	
	classbounds = []
	for i,(astr, group, start, left, right, end) in enumerate(classbnds):
		endpreamble = left +1
		endbody     = right-1
		preamble= stripBlanklines(str.rstrip(record[start:endpreamble]))
		preamble    = findInlineComments.sub("", preamble)
		finale      = record[endbody:end]

		m = classtype.search(preamble)
		if m == None:
			fatal("unable to get class type\n%s" % str)
		ctype = m.group("classtype")            
		if ctype == "class":
			group = "class"
		else:
			group = "structclass"
		classbounds.append((preamble,group,start,endpreamble))
		classbounds.append((finale,'end%s' % group,endbody,end))

	tokens = []
	classbounds.sort(groupcmp)
	for i,(astr, group, start, end) in enumerate(classbounds):        
		placeholder = placeHolder(group, i)
		if group[0:3] == 'end': placeholder = '\n'+ \
                   placeholder #NB: \n!
		items[placeholder] = astr
		tokens.append((placeholder,start, end))        
	if len(tokens) > 0:
		record = splice(record, tokens)        
	return record
#---------------------------------------------------------------------------
# Extract class type, name and possible base classes
#---------------------------------------------------------------------------
def getClassname(record):
	m = classtitle.search(record)
	if m == None:
		fatal("classtitle regex FAILED on line\n%s\n" % record)

	template = m.group("template")
	if template != None:
		template = ''.join(str.split(str.strip(template)))

	title = str.strip(m.group("classtitle"))

	m = basenames.search(title)
	if m != None:
		bname = str.strip(str.strip(m.group("basenames"))[1:])
		bname = str.replace(bname,"public ","%spublic " % WEIRD)
		bname = str.replace(bname,"protected ","%sprotected " % \
                                    WEIRD)
		bname = str.replace(bname,"private ","%sprivate " % WEIRD)
		bname = str.split(bname,WEIRD)[1:]
		for i, name in enumerate(bname):
			name = str.strip(name)
			if name[-1] == ',':
				name = name[:-1]
			bname[i] = name
		cname = str.strip(title[:m.start()])
	else:
		bname = []
		cname = str.strip(title[:len(title)-1])

	return (cname,bname,template)
#----------------------------------------------------------------------------
scrunch   = re.compile(r' +(?=[\<])|(?<=[\<]) +|(?<=,) +')
def fixName(name):
	name = scrunch.sub('',name)
	return name
getwords  = re.compile(r'[a-zA-Z0-9]*[:]*[a-zA-Z0-9]*[:]*[a-zA-Z0-9]+')
#----------------------------------------------------------------------------
def cmsswProject():
	if not ("CMSSW_RELEASE_BASE" in os.environ):
		sys.exit("\t** Please setup a CMSSW release")

	if not ("CMSSW_BASE" in os.environ):
		sys.exit("\t** Please setup a CMSSW release")

	if not ("CMSSW_VERSION" in os.environ):
		sys.exit("\t** Please setup a CMSSW release")

	PWD       = os.path.realpath(os.environ['PWD'])
	BASE  = "%s/src/"  % os.path.realpath(os.environ["CMSSW_RELEASE_BASE"])
	LOCALBASE = "%s/src/"  % os.path.realpath(os.environ["CMSSW_BASE"])
	VERSION   = os.environ["CMSSW_VERSION"]
	#-------------------------------------------------------------------
	# Determine project directory
	if len(LOCALBASE[:-1]) < len(PWD):
		project = str.replace(PWD, LOCALBASE, '')
		project = str.split(project, '/')
	else:
		project = []

	np = len(project)
	if   np == 0:
		PACKAGE, SUBPACKAGE = [None, None]
	elif np == 1:
		PACKAGE = project[0]
		SUBPACKAGE = None
	elif np >  1:
		PACKAGE, SUBPACKAGE = project[0:2]
	return (PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION)
#----------------------------------------------------------------------------
# Get author's name
def getauthor():
	regex  = re.compile(r'(?<=[0-9]:)[A-Z]+[a-zA-Z. ]+')
	record = str.strip(os.popen("getent passwd $USER").read())
	author = "Shakespeare's ghost"
	if record != "":
		t = regex.findall(record)
		if len(t) > 0: author = t[0]
	return author

def getAuthor():
	return getauthor()


#----------------------------------------------------------------------------
# Routine to load appropriate library
#----------------------------------------------------------------------------
LOADED_LIBS = {}
def loadLibrary(name):
        if len(LOADED_LIBS) == 0:
                import PhysicsTools.TheNtupleMaker.AutoLoader
                LOADED_LIBS[name] = 1
                return
        name = fixName(name) # remove unnecessary spaces

        if not ClassToHeaderMap.has_key(name): return

        # construct library name from header:
        # lib<subsystem><package>

        library = "lib%s%s" % \
                tuple(str.split(ClassToHeaderMap[name],'/')[:2])

        if library in LOADED_LIBS: return

        LOADED_LIBS[library] = 0

        try:
                ROOT.gSystem.Load(library)
        except:
                print("** failed to load %s for %s" % (library, name))
#----------------------------------------------------------------------------
# For given class, return information about all getters that have arguments
# with fundamental types.
#----------------------------------------------------------------------------
def get_base_classnames(classname, db, depth=0):
        depth += 1
        if depth > 20: return

        c = ROOT.TClass(classname)
        bclasses = c.GetListOfBases()
        nb = bclasses.GetSize()
        for i in range(nb):
                b = bclasses.At(i)
                basename = b.GetName()
                db['baseclassnames'].append(basename)
                get_base_classnames(basename, db, depth)

DEBUG   = 0
def classMethods(classname, db, depth=0):
        loadLibrary(classname)

        depth += 1
        if depth == 1:
                db['methods']     = {}
                db['datamembers'] = {}
                db['classname']   = classname
                db['classlist']   = []
                db['baseclassnames'] = []

        if depth > 20:
                print("**classMethods** lost in trees!")
                return

        tab = "  " * (depth-1)

        cdb = {'classname': classname,
               'methods': []}

        c = ROOT.TClass(classname)

        # get list of all public methods and its base classes
        pubmethods = c.GetListOfAllPublicMethods()

        ###D
        if DEBUG > 0:
                print("\n%s\n%sCLASS: %s" % ('-'*80, tab, classname))
                print("%s METHODS" % tab)

        for i in range(pubmethods.GetSize()):
                m = pubmethods.At(i)
                methodname  = m.GetName()

                # skip some obviously "skipable" stuff
                if skipthis.match(methodname) != None: continue

                # skip constructors and possible destructor
                if classname.find(methodname) > 0: continue

                # skip setter methods and methods that return void*
                retype = m.GetReturnTypeName()
                if retype in ['void', 'void*']: continue

                # we have getters only

                # get (type1 name1, ...)
                signature = m.GetSignature()

                # construct full method
                method  = "%s %s%s" % (retype, methodname, signature)
                rtype, name, argtypes, argnames, qual = decodeMethodName(method)

                # decode number of required arguments
                nargs   = len(argtypes)  # number of required arguments

                skip_function = False
                ndefargs = 0
                for argtype in argtypes:
                        # this could be a defaulted argument
                        # split at "=" sign 
                        t = argtype.split('=')
                        if len(t) == 2:
                                # this is a defaulted argument
                                ndefargs += 1

                        # we keep getters with arguments that are
                        # fundamental types

                        if DEBUG:
                                print("ARGTYPES( %s )" % t)
                        argtype = t[0]
                        if isFuntype.match(argtype): continue

                        # this getter has at least one argument
                        # that is not a fundamental type, so bail.
                        skip_function = True

                if skip_function:
                        if DEBUG > 1:
                                print("SKIP: %s\n" % method)
                        continue

                if DEBUG > 0:
                        print("%s METHOD:    %s" %   (tab, method))
                        print("%s  NAME:     %s" %   (tab, name))
                        print("%s  RETURN:   %s" %   (tab, rtype))
                        print("%s  ARGTYPES: %s" %   (tab, argtypes))
                        print("%s  ARGNAMES: %s" %   (tab, argnames))
                        print("%s  QUAL:     %s" %   (tab, qual))


                # This function has arguments with simple types
                nrargs = nargs - ndefargs
                args   = signature

                # build signature without identifiers and
                # build a calling sequence

                argscall = '('
                args     = "("
                delim    = ""
                for iarg in range(nargs):
                        argname = argnames[iarg]
                        argtype = argtypes[iarg]
                        if iarg < nrargs:
                                value = ""
                        else:
                                argtype, value = argtype.split('=')
                                value = "=%s" % value

                        # If no argument given, make one
                        if argname == "":
                                argname = "x%d" % iarg

                        if DEBUG > 1:
                                print("\tARGTYPE(%s) ARG(%s%s)" % \
                                      (argtype, argname, value))

                        args += "%s%s %s%s" % (delim, argtype,
                                               argname, value)
                        argscall += "%s%s" % (delim, argname)
                        delim = ", "

                args += ")"
                argscall += ")"

                signature  = name + args
                methodcall = name + argscall

                if DEBUG:
                        print("  SIGNATURE: %s\n       CALL: %s\n" % \
                              (signature, methodcall))

                rtype      = str.strip(basicstr.sub("std::string", rtype))
                signature  = basicstr.sub("std::string", signature)
                astr       = "%s  %s" % (rtype, signature)

                if skipmethod.search(astr) != None:
                        if DEBUG > 1:
                                print("\t>>>> skipmethod: %s" % astr)
                        continue

                m = reftype.findall(astr)
                if len(m) != 0:
                        for x in m:
                                cname = "%sRef" % x
                                cmd = re.compile(r"edm::Ref\<.+?,%s\> \>" % x)
                                rtype = cmd.sub(cname, rtype)
                                signature = cmd.sub(cname, signature)

                # Ok, now add to methods list
                rtype     = vsqueeze.sub(">", rtype)
                signature = vsqueeze.sub(">", signature)
                method    = "%32s  %s" % (rtype, signature)

                # Important: make sure we don't have duplicates
                if db['methods'].has_key(method): continue

                db['methods'][method] = (name,
                                         classname,
                                         rtype,
                                         signature,
                                         methodcall)

                cdb['methods'].append(method)

        db['classlist'].append( cdb )

        get_base_classnames(classname, db)
#----------------------------------------------------------------------------
def classDataMembers(classname, db, depth=0):
        loadLibrary(classname)

        depth += 1
        if depth > 20:
                print("lost in trees")
                return
        tab = "  " * (depth-1)

        cdb = {'classname': classname,
               'datamembers': []}

        c  = ROOT.TClass(classname)

        # get all public data members of this class and its base classes.
        dm = c.GetListOfAllPublicDataMembers()
        n  = dm.GetSize()

        for i in xrange(n):
                d     = dm.At(i)
                name  = d.GetName()
                nametype = d.GetFullTypeName()
                nametype = str.strip(basicstr.sub("std::string", nametype))

                # Ok, now added to datamembers list
                nametype = vsqueeze.sub(">", nametype)
                member   = "%32s  %s" % (nametype, name)

                if db['datamembers'].has_key(member): continue

                db['datamembers'][member] = classname
                cdb['datamembers'].append((nametype, member))

        db['classlist'].append( cdb )
#----------------------------------------------------------------------------
# Define a container as a class that contains the methods
# size() and operator[](unsigned ? long ?int)
def isContainer(classname):
        loadLibrary(classname)

        c = ROOT.TClass(classname)
        # get list of all public methods and its base classes
        pubmethods = c.GetListOfAllPublicMethods()

        has_size = False
        has_oper = False
        for i in range(pubmethods.GetSize()):
                m = pubmethods.At(i)
                methodname  = m.GetName()
                if name == "size":
                        # check signature
                        if isvoid.search(name) != None:
                                has_size = True
                elif name == "operator[]":
                        if isint.search(name) != None:
                                has_oper = True

                iscon = has_size and has_oper
                if iscon: return True

        return False
