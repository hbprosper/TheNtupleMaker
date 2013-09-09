#!/usr/bin/env python
#------------------------------------------------------------------------------
# File: mkslideshow.py
# Description: Create a web-based slideshow from a list of gif files
# Created: 29-Oct-2006 Harrison B. Prosper
#$Id: mkslideshow.py,v 1.17 2013/07/05 21:01:54 prosper Exp $
#------------------------------------------------------------------------------
import os
import sys
from getopt  import getopt, GetoptError
from string import atof, atoi, replace, lower,\
	 upper, joinfields, split, strip, find
from time import sleep, ctime
from glob    import glob
#------------------------------------------------------------------------------
WIDTH = 200
COLS  = 1
TAB   = '  '
USAGE = '''
Usage:
  mkslideshow.py <options> graphics-files

  options:
       -h   print this
       -t   <title>
       -a   <author>

  Note: Optionally, a text file can be associated with each graphics file and
  its contents used as a figure caption. The name of the caption file is
  assumed to be <name>.txt where <name> is the name of the graphics file,
  without its extension.
'''
shortOptions = 'ht:a:'
#------------------------------------------------------------------------------
# Functions
#------------------------------------------------------------------------------
def usage():
    print USAGE
    sys.exit(0)

def quit(s):
    print "\n**error** %s" % s
    sys.exit(1)

def nameonly(x):
    return os.path.splitext(os.path.basename(x))[0]
#------------------------------------------------------------------------------
index = '''<html>
	<head>
		<title>%(title)s</title>
		<meta http-equiv=Content-Type 
		      content="text/html; charset=windows-1252">
	</head>
	
<frameset 
	border=0 
	frameSpacing=0 
	rows=64,* 
	frameBorder=0>
        
	<frame 
		border=0 
		name=banner 
		src="banner.html" 
		frameBorder=0 
		noResize 
		scrolling=no 
		target="contents">

		<frameset cols=240,*>

			<frame border=0 
				name=contents 
				src="contents.html" 
				frameBorder=0 
				scrolling="auto" 
				target="contents">
                                
				<frame 
					border=0 
					name=display 
					src="display.html" 
					frameBorder=0 
					scrolling="auto" 
					target="_self">
		</frameset>
                
		<noframes>
  			<body>
  			<p>Your browser is brain dead</p>
  			</body>
  		</noframes>
 </frameset>

 </html>
'''
#------------------------------------------------------------------------------
banner = '''<html>
 <head>

 <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
 
   <title>%(title)s - banner</title>

   <style type="text/css">
   .navigation
   {
     position:    absolute;
     visibility:  visible;
     top:         5px;
     right:       2px;
   }
   </style>

   <script language="javascript" src="display.html"></script>
   
 </head>

  <body bgcolor="indigo">
   <h2><font color="white">%(title)s</font></h2>
 </body>


  <!-- Navigation elements -->
  
  <div id="nav" class="navigation">
    <table border=1 cellpadding=5 cellspacing=1>
      <tr align=center valign=top>

        <td>
	  <a href="javascript:firstSlide(0)" target="display">
	  <font color="white" size=+1><b>First</b></font>
	  </a>
        </td>
        
        <td>
	  <a href="javascript:changeSlide(-1)" target="display">
	  <font color="white" size=+1><b>Previous</b></font>
	  </a>
        </td>

        <td>
	  <a href="javascript:changeSlide(1)" target="display">
	  <font color="white" size=+1><b>Next</b></font>
	  </a>
        </td>

        <td>
	  <a href="javascript:lastSlide()" target="display">
	  <font color="white" size=+1><b>Last</b></font>
	  </a>
        </td>

      </tr>
    </table>
  </div>

 
</html>
'''
#------------------------------------------------------------------------------
display  = '''<html>
 <head>

 <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
 
   <title>%(title)s - display</title>

   <style type="text/css">

   .slide
   {
     position:    absolute;
     visibility:  hidden;
     top:         80px;
     right:       100px;
   }

   .textbox
   {
     position:    absolute;
     visibility:  visible;
     top:         550px;
     left:        10px;
   }   

   #slide0
   {
     visibility:  visible;
   }
   </style>

   <script language="javascript">

    /*
   Copyright (c) 2001, 2002 by Martin Tsachev. All rights reserved.
   http://www.mt-dev.com

   This script uses JavaScript library functions by Peter-Paul Koch
   http://www.xs4all.nl/~ppk/

   Redistribution and use in source and binary forms,
   with or without modification, are permitted provided
   that the conditions available at
   http://www.opensource.org/licenses/bsd-license.html
   are met.

   Modified by Harrison B. Prosper
   */

   var numberSlides = %(nslides)d;
   var slides  = new Array(numberSlides);
   var curImg  = 0; // index of the array entry
   var lastImg = 0;
   var init    = false;
   var DHTML   = (document.getElementById || document.all || document.slides);
   
   window.onload = init_slides;

   function init_slides() 
   {
      if (!DHTML) 
      {
        alert(\'Your browser is not DHTML capable\');
        return;
      }

      for (i = 0; i < slides.length; i++) 
      {
        var name = \'slide\' + i;
        slides[i] = new getObj(name);
      }
      init = true;
    }

    function getObj(name) 
    {
      if (document.getElementById) 
      {
        this.obj = document.getElementById(name);
        this.style = document.getElementById(name).style;
      } 
      else
      if (document.all) 
      {
        this.obj = document.all[name];
        this.style = document.all[name].style;
      } 
      else
      if (document.layers) 
      {
        this.obj = document.layers[name];
        this.style = document.layers[name];
      }
    }

    function changeSlide(change) 
    {
      if (!init) 
      {
        alert(\'Wait for the page to load\');
        return;
      }
      if (!DHTML) return;

      curImg += change;
      if (curImg < 0) curImg = slides.length-1;
      else
      if (curImg >= slides.length) curImg = 0;
      
      slides[lastImg].style.visibility = \'hidden\';
      slides[curImg].style.visibility =\'visible\';

      lastImg = curImg;
    }

    function showSlide(number) 
    {
      if (!init) 
      {
        alert(\'Page not yet loaded..wait and try again\');
        return;
      }
      if (!DHTML) return;

      curImg = number;
      if (curImg < 0) curImg = slides.length-1;
      else
      if (curImg >= slides.length) curImg = 0;
      
      slides[lastImg].style.visibility = \'hidden\';
      slides[curImg].style.visibility =\'visible\';

      lastImg = curImg;
    }    

    function firstSlide()
    {
      showSlide(0);
    }

    function lastSlide()
    {
      showSlide(slides.length-1);
    }    
    
   </script>

 </head>

 <body bgcolor="white">

%(author)s
Created: %(date)s
%(body)s
    
</body>

</html>
'''
#------------------------------------------------------------------------------
contents = '''<html>
 <head>

 <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
 
   <title>%(title)s - contents</title>

   <style type="text/css">
   .gtable
   {
     position:    absolute;
     visibility:  visible;
     top:         10px;
     left:        8px;
   }
   </style>

   <script language="javascript" src="display.html"></script>

 </head>

 <body bgcolor="blueviolet">

%(body)s

 </body>

</html>
'''
#------------------------------------------------------------------------------
href = '''
%(tab)s  <a href="javascript:showSlide(%(slide)d)" target="display">
%(tab)s    <img src="%(image)s" width=%(width)s>
%(tab)s  </a>
'''
#------------------------------------------------------------------------------
div = '''
%(tab)s<div id="slide%(slide)d" class="slide">
%(tab)s  <img src="%(image)s" border=0>
%(caption)s
%(tab)s</div>
'''
#------------------------------------------------------------------------------
def addTable(items, cols=COLS):
    divrecs = []
    tabrecs = []
    names = {'tab': TAB}
    tabrecs.append('<div id="table0" class="gtable" >')
    tabrecs.append('%(tab)s<table>' % names)

    names['tab'] =  2 * TAB
    i = 0
    while i < len(items):
        tabrecs.append('%(tab)s<tr align=left>' % names)
        names['tab'] =  3 * TAB
        for col in range(cols):
            if i >= len(items): break
            ahref, adiv = items[i]
            
            names['item'] = ahref
            tabrecs.append('%(tab)s<td>%(item)s%(tab)s</td>' % names)

            names['item'] = adiv
            divrecs.append('%(item)s' % names)
            i += 1
        names['tab'] =  2 * TAB
        tabrecs.append('%(tab)s</tr>' % names)

    names['tab'] =  TAB

    tabrecs.append('%(tab)s</table>\n' % names)
    tabrecs.append('</div>\n')
    
    return (joinfields(tabrecs,'\n'), joinfields(divrecs,'\n'))
#------------------------------------------------------------------------------
def main():
    argv = sys.argv[1:]

    #---------------------------------------------------
    # Decode command line using getopt module
    #---------------------------------------------------
    try:
        options, files = getopt(argv, shortOptions)
    except GetoptError, m:
        print
        print m
        usage()
        
    # Make sure we have at least one gif file

    if len(files) == 0: usage()
    title  = ''
    author = ''
    
    for option, value in options:
        if option == "-h":
            usage()
            
        elif option == "-t":
            title = value

        elif option == "-a":
            author = value

    context = {'date':    "%s<br>" % ctime(time()),
               'title':   title,
               'author':  "%s<br>" % author,
               'tab':     3 * TAB,
               'width':   WIDTH,
               'nslides': len(files)}
    
    items = []
    ind = 0
    for file in files:
        if not os.path.exists(file): quit("file %s not found" % file)
        doc = '%s.txt' % os.path.splitext(file)[0]
        if os.path.exists(doc):
            context['caption'] = "<p>%s</p>" % open(doc).read()            
        else:
            context['caption'] = "<p><font size=+1>%s</font></p>" % \
                                 nameonly(file)
        context['slide'] = ind
        context['image'] = file
        items.append((href % context, div % context))
        ind += 1
        
    tabbody, divbody = addTable(items)
    
    html = index % context
    open("index.html","w").write(html)

    html = banner % context
    open("banner.html","w").write(html)

    context['body'] = tabbody
    html = contents % context
    open("contents.html","w").write(html)

    context['body'] = divbody
    html = display % context
    open("display.html","w").write(html)
#------------------------------------------------------------------------------
main()
