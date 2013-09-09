#!/usr/bin/env python
#------------------------------------------------------------------------------
# File: mkdoc.py
# Description: Create documentation of classes.
# Created: 11-Dec-2009 HBP, (during CERN visit)
#$Id: mkindex.py,v 1.16 2011/05/07 18:39:14 prosper Exp $
#------------------------------------------------------------------------------
import os, sys, glob
from string import *
from time import *
from PhysicsTools.TheNtupleMaker.Lib import nameonly
#------------------------------------------------------------------------------
TEMPLATE='''<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
 <head>
   <title>CMSSW DataFormats</title>
     <meta http-equiv="content-type" content="text/html; charset=ISO-8859-1">
 </head>
 <body>

  <h2><font color="33333cc">%(version)s DataFormats - 
  Created: %(time)s</font></h2>
  <p>
  <hr>
  
  <font size="+2"><b>Index</b></font>
  <p>
  
  <ul>
  %(index)s
  </ul>
  <hr>
  
  <p>
 
  <ul>
  %(list)s
  </ul>
 </body>
</html>
'''
def usage():
	print'''
	mkindex.py
	'''
	sys.exit(0)
#------------------------------------------------------------------------------
def main():
	
	DIR = "html"
	if os.environ.has_key("CMSSW_RELEASE_BASE"):
		version = split(os.environ['CMSSW_RELEASE_BASE'],'/')[-1]
	else:
		version = "CMSSW"

	names = {'version': version,
			 'time': ctime(time())}
	
	# find all html files
	files = map(lambda x: nameonly(x), glob.glob("%s/*.html" % DIR))
	files.sort()

	lastmodule = ''
	ind = ''
	str = ''
	for index, file in enumerate(files):
		if file == "index": continue
		if find(file, "_") > -1: continue
		print "%5d\t%s" % (index+1, file)
		
		rec = os.popen("grep Methods %s/%s.html" % (DIR, file)).read()
		if rec == "": continue

		t = split(file, '.')
		module = t[1]
		if module != lastmodule:
			if index == 0:
				record = ' '
			else:
				record = '  </ul>\n'
			record += ' <li>'\
					  '<a name="%s">'\
					  '<font color="red"><b>%s</b></font></a>\n  <ul>\n' % \
							 (module, module)
			str += record
			ind += ' <li> <a href="#%s"><font color="red"><b>%s</b></font>'\
				   '</a>' % (module, module)
		lastmodule = module

		record = '   <li><a href="%s.html">%s</a>\n' % \
				 (file, joinfields(t[2:],'.'))
		str += record
	str += "  </ul>\n"
	names['index'] = ind
	names['list'] = str
	record = TEMPLATE % names
	open("%s/index.html" % DIR, 'w').write(record)
#------------------------------------------------------------------------------
main()


