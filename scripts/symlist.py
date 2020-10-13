#!/usr/bin/env python
#------------------------------------------------------------------------------
# File: symlist.py
# Description: List symbols etc. ldd/nm
# Created:     October-2003 Harrison B. Prosper
# Updated:     02-Oct-2020 HBP adapt for ADL project
# $Revision: 1.2 $
#------------------------------------------------------------------------------
import os, sys, re
from popen2     import popen3
from getopt     import getopt, GetoptError
from glob       import glob
from pprint     import PrettyPrinter
#------------------------------------------------------------------------------
# Constants
#------------------------------------------------------------------------------
USAGE = '''
Usage:
 symlist  [--help] -l [-i] [-e] <shared objects>

 DESCRIPTION
      -l   List shared libraries on which specified ones depend
      -i   List fully qualified (code) symbols
      -e   List fully qualified (code) symbols 
'''
shortOptions = 'lies'
longOptions  = ['help']
internal     = re.compile('(?P<record>^[0-9a-f]+ T (?P<name>[^_].+))')
internalsym  = re.compile('(?P<record>^[0-9a-f]+ B (?P<name>[^_].+))')
external     = re.compile('(?P<record>^\s+U (?P<name>[^_].+))')
incharge     = re.compile('\s*\[[-\w ]*in-charge[-\w ]*\]\s*')
names        = {'errorlog': 'solisterror.log'}
pp           = PrettyPrinter()
simpletype   = re.compile('(?P<name>(const\s+)?(unsigned\s+)?' \
                          '\\bu?((unsigned|char'\
                          '|short|int|long|float|void|double|bool)'\
                          '(\w+_t)?\\b'\
                          '|\w+_t\\b)(\s*([*][*]?|[&]))?)')
#------------------------------------------------------------------------------
# Functions
#------------------------------------------------------------------------------
def usage():
    sys.exit(USAGE)
    
def quit(s):
    sys.exit("\n**error** %s\tgoodbye!" % s)

def nameonly(x):
    return os.path.splitext(os.path.basename(x))[0]
#------------------------------------------------------------------------------
# List shared libraries on which specified shared libraries depend
#------------------------------------------------------------------------------
def ldd(infiles):

    # Run ldd
    
    errors = []
    output = []
    command = ""
    for count, file in enumerate(infiles):
        if not os.path.exists(file):
            quit("could not find file %s" % file)
        
        stdout, stdin, stderr = popen3("ldd %s" % file)

        recs = stdout.readlines()

        output.extend(list(map(lambda x: x.strip(), recs)))

        
    # Check for errors

    if len(errors) > 0:
        errlog = open("%(errorlog)s" % names,"w")
        for error in errors:
            errlog.write("%s\n" % error)
        errlog.close()    

    if len(output) == 0:
        quit("check %(errorlog)s\n" % names)

    # Extract paths and libraries found
    # Keep track of input files with problems
    
    index = 0
    paths = []
    problem = {}
    for s in output:
        if s == infiles[index]:
            file = s
            index += 1
        else:
            t = str.split(s,'=>')
            if len(t) == 2:
                lib  = str.split(t[0],'/').pop()
                path = str.split(t[1])[0]
                if not path in paths:
                    paths.append((lib, path))
                    
    # Write info to screen

    paths.sort()    
    for lib, path in paths:
        print("%-32s %s" % (lib,path))
                
    if len(errors) > 0:
        print("\nWarning: check %(errorlog)s\n" % names)
#------------------------------------------------------------------------------
# List symbols
#------------------------------------------------------------------------------
cleanname = re.compile('[ ]*\[.*\-charge.*\]')
def clean(name):
    name = split(name,'@@')[0]
    name = cleanname.sub('',name)
    return name

def nm(infiles, switches):
    for count, afile in enumerate(infiles):
        if not os.path.exists(afile):
            quit("could not find file %s" % afile)


        inames = {}
        if '-i' in switches:
            command = "nm --demangle --defined-only %s" % afile
            records = os.popen(command).readlines()
            
            for index, record in enumerate(records):
                record = str.strip(record)
                if record[-3:] == '.o:': continue                
                t = str.split(record)
                if len(t) < 3: continue
                vtype = t[1]
                record= ' '.join(t[2:])
                if vtype  in ['T','W','V','B']:
                    record = clean(record)
                    inames[record] = 1
    
        enames = {}
        if '-e' in switches:
            command = "nm --demangle --undefined-only %s" % afile
            records = os.popen(command).readlines()        
            for index, record in enumerate(records):
                record = str.strip(record)
                t   = str.split(record)
                if len(t) < 2: continue
                record = ''.join(t[1:])
                record = clean(record)
                enames[record] = 1 

        library = os.path.basename(afile)
        extension = os.path.splitext(afile)[1]
        path   = str.replace(afile, "/%s" % library, '')
        if path <> afile:
            cmd  = 'cd %s; pwd' % path
            path = str.strip(os.popen(cmd).read())
            names['library'] = nameonly(library) + extension
            names['path']    = path
        else:
            names['library'] = nameonly(library) + extension
            names['path']    = '.'
            
        keys = list(inames.keys())
        keys.sort()
        addline = 1
        for name in keys:
            names['name'] = name
            astr = "%(path)s %(library)s I %(name)s" % names
            if len(astr) < 80:
                print(astr)
                addline = 1
            else:
                if addline: print()
                print(astr)
                print()
                addline = 0
            
        keys = list(enames.keys())
        keys.sort()
        addline = 1
        for name in keys:
            names['name'] = name
            astr = "%(path)s %(library)s E %(name)s\n" % names
            if len(astr) < 80:
                print(astr)
                addline = 1
            else:
                if addline: print()
                print(astr)
                print()
                addline = 0
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
def main():
    try:
        opts, infiles = getopt(sys.argv[1:],shortOptions, longOptions)
    except GetoptError, m:
        print()
        print(m)
        usage()

    if len(infiles) == 0: usage()
        
    # Decode option, value pairs

    switches = []
    for option, value in opts:
        if option == "help":
            usage()


        elif option == "-l":
            ldd(infiles)
            return
        else:
            switches.append(option)
    
    if len(switches) == 0:
        usage()
    else:
        nm(infiles, switches)
main()

