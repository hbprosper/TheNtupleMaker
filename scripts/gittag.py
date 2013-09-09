#!/usr/bin/env python
import os, sys, string
#-----------------------------------------------------------------------------
argv = sys.argv[1:]
if len(argv) == 0:
	print '''
	Usage:
	    gittag.py <tagname>
	'''
	sys.exit(0)
tagname = argv[0]

username = ''
id = string.split(os.popen('git log').readline())[-1]
cmd = 'git tag %s %s' % (tagname, id)
print cmd
os.system(cmd)



