#!/usr/bin/env python
#---------------------------------------------------------------------------
# File:        mkvomit.py
# Description: Create html and txt files giving an exhaustive listing of
#              accessor methods
# Created:     23-Jan-2010 Harrison B. Prosper
# Updated:     15-Feb-2010 HBP, make it possible to be run anywhere
#              09-Mar-2010 HBP, add search of SimDataFormats
#              08-Aug-2010 HBP, fix search of user.h in TheNtupleMaker
#              26-Aug-2011 HBP, get list of potential classes from
#                          python/classmap.py
#              24-Apr-2012 HBP, get list of potential classes from
#                          plugins/classlist.txt instead
#              30-Aug-2020 HBP adapt to python (for ADL project)
#---------------------------------------------------------------------------
import os, sys, re
import ROOT
from time import sleep, ctime
from xml.etree.ElementTree import Element
from getopt import getopt
from PhysicsTools.TheNtupleMaker.classmap import ClassToHeaderMap
from PhysicsTools.TheNtupleMaker.Lib import \
     parseHeader,\
     splitHeader,\
     stripBlanklines,\
     namespaceName,\
     getClassname,\
     convert2html,\
     cmsswProject,\
     findHeaders,\
     classMethods,\
     classDataMembers
from pprint import PrettyPrinter
pp = PrettyPrinter()
#---------------------------------------------------------------------------
# Constants
#---------------------------------------------------------------------------
PACKAGE, SUBPACKAGE, LOCALBASE, BASE, VERSION = cmsswProject()
if PACKAGE == None:
    sys.exit("Please run mkvomit.py in your package directory")

print("Subsystem: %s" % PACKAGE)
print("Package:   %s" % SUBPACKAGE)

PROJECTBASE = "%s%s/%s"   % (LOCALBASE, PACKAGE, SUBPACKAGE)
PLUGINDIR = "%s/plugins"   % PROJECTBASE  
SRCDIR    = "%s/src"       % PROJECTBASE
INCDIR    = "%s/interface" % PROJECTBASE
#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
USAGE='''
Usage:
  mkvomit.py
'''
def usage():
    sys.exit(USAGE)

SHORTOPTIONS = 'hI:'
#----------------------------------------------------------------------------
# Load needed libraries
#import PhysicsTools.TheNtupleMaker.AutoLoader
#----------------------------------------------------------------------------
# Code to extract methods etc.
#----------------------------------------------------------------------------
BLACK     = '"black"'
DARKRED   = '"darkred"'
DARKGREEN = '"darkgreen"'
DARKBLUE  = '"darkblue"'
BLUE      = '"blue"'
#----------------------------------------------------------------------------
def color(c,s):
    return '<font color=%s>%s</font>' % (c,s)
#----------------------------------------------------------------------------
def boldcolor(c,s):
    return '<b><font color=%s>%s</font></b>' % (c,s)
#----------------------------------------------------------------------------
def printMethods(db, out):
    classlist = db['classlist']
    for cdb in classlist:
        if not ('methods' in cdb): continue
        cname   = cdb['classname']
        methods = cdb['methods']
        out.write('\nAccessMethods: %s\n' % cname)
        for method in methods:
            out.write("  %s\n" % method)
#----------------------------------------------------------------------------
def printDataMembers(db, out):
    classlist = db['classlist']
    for cdb in classlist:
        if not ('datamembers' in cdb): continue
        cname = cdb['classname']
        data  = cdb['datamembers']
        out.write('\nDataMembers:   %s\n' % cname)
        for dtype, datum in data:
            out.write("  %s\n" % datum)
#----------------------------------------------------------------------------
def printHeader(db, out):
    cname    = db['classname']
    bname    = db['baseclassnames']
    header   = db['header']
    filestem = db['filestem']

    if cname == '':
        print("\t** %s not processed!" % header)
        out.close()
        os.system("rm -rf txt/%s*" % filestem)
        sys.exit(0)

    # Write class name and its header file

    tab = 15*' '
    out.write('Class:         %s\n\n'  % cname)
    out.write('Header:        %s\n\n'  % header)

    # Write base class names

    if len(bname) > 1:
        delim = ''
        out.write('BaseClasses:   ')
        for i, fullname in enumerate(bname):
            out.write('%s%s\n' % (delim, fullname))
            delim = tab

        out.write("\n")
    out.write('Version:       %s\n' % db['version'])
    out.write('Created:       %s\tmkvomit.py\n' % ctime())
#----------------------------------------------------------------------------
def writeHTML(db, txtfilename):

    fullname = db['classname']
    filestem = db['filestem']

    record = open(txtfilename).read()
    record = convert2html(record)	
    cname  = str.split(str.split(fullname, '<')[0],'::').pop()

    record = str.replace(record, 'Class:',
                         boldcolor(BLUE, 'Class:'))

    record = str.replace(record, 'Header:',
                         boldcolor(BLUE, 'Header:'))

    record = str.replace(record, 'BaseClasses:',
                         boldcolor(BLUE, 'BaseClasses:'))

    record = str.replace(record, 'AccessMethods:',
                         boldcolor(DARKBLUE, 'AccessMethods:'))

    record = str.replace(record, 'DataMembers:',
                         boldcolor(DARKBLUE, 'DataMembers:'))

    htmlfile = "html/%s.%s.html" % (filestem, cname)
    out = open(htmlfile, 'w')
    out.write('<code>\n')
    out.write('\t<font size=+1 color=%s>\n' % BLACK)
    out.write(record)
    out.write('\t</font>\n')
    out.write('</code>\n')
    out.close()
#============================================================================
# Main Program
#  Read class list
#  for each class in list
#     decode class
#============================================================================
def main():

    print("mkvomit.py\n")

    # read classlist.txt
    filename='%sPhysicsTools/TheNtupleMaker/plugins/classlist.txt' % \
        LOCALBASE
    classlist = [x.strip()  for x in  open(filename).readlines()]

    clist = []
    for name in classlist:
        t = name.split()
        if len(t) < 3:
            print("** the format of plugins/classlist.txt is out of date")
            sys.exit("** run scripts/mkclasslist.py to update it")
        elif len(t) == 3:
            pkg, name, ctype = t
        else:
            pkg   = t[0]
            name  = ' '.join(t[1:-1])
            ctype = t[-1]

        headers = findHeaders(name)
        if len(headers) == 0: continue

        clist.append((name, headers[0]))
    clist.sort()
    classlist = clist

    #-------------------------------------------------
    # Loop over classes to be scanned
    #-------------------------------------------------

    # Make sure html and txt directories exist

    os.system("mkdir -p html; mkdir -p txt")

    count = 0
    for index, (classname, header) in enumerate(classlist):

        # Create full pathname to header

        afile = LOCALBASE + header
        if not os.path.exists(afile):
            afile = BASE + header
            if not os.path.exists(afile):
                print("**mkvomit.py** file %s not found" % afile)
                continue

        afile    = os.path.abspath(afile)
        header   = afile		
        fullname = classname

        # For now ignore templates
        if str.find(fullname, '<') > -1: continue

        # Get methods and/or datamembers and write them out

        # Initialize map to contain info about classes, 
        # methods & datamembers

        k = header.rfind("/src/") # search from right
        if k > 0: header = header[k+5:]
        filestem = str.replace(header, 'interface/', '')
        filestem = str.split(filestem, '.h')[0]
        filestem = str.replace(filestem, '/', '.')

        db = {'version':   VERSION,
              'filestem':  filestem,
              'header':    header,
              'classname': fullname}

        classMethods(fullname, db)
        classDataMembers(fullname, db)

        #print()
        #print("="*80)
        #pp.pprint(db)
        #if index > 10:
        #   break
    
        
        nmeth = len(db['methods'])
        ndata = len(db['datamembers'])

        if nmeth > 0 or ndata > 0:

            count += 1
            print("%5d\t%s" % (count, fullname))

            cname  = str.split(fullname,'::').pop()
            methfilename = "txt/%s.%s.txt" % (filestem, cname)

            out = open(methfilename, 'w')
            printHeader(db, out)
            printMethods(db, out)
            printDataMembers(db, out)
            out.close()

            writeHTML(db, methfilename)
        else:
            print("*** no methods found for %s" % fullname)

    # Create index.html
    print("\ncreating html/index.html..")
    os.system("mkindex.py")
    print("\n\tmkvomit.py is done!\n")
#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
main()










