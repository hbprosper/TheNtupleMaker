#!/usr/bin/env python
import os, sys
#-----------------------------------------------------------------------------
argv = sys.argv[1:]
if len(argv) == 0:
	sys.exit('''
	Usage:
	    gittag.py <tagname>
	''')
tagname = argv[0]

cmd = 'git tag %s' % tagname
print(cmd)
os.system(cmd)

cmd = 'git push origin --tags'
print(cmd)
os.system(cmd)

