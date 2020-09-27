TheNtupleMaker
==============

## Contents
1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Tutorial](#tutorial)



"Any intelligent fool can make things bigger, more complex, and more violent. It takes a touch of genius, and a lot of courage, to move in the opposite direction."  
*Albert Einstein*


## Introduction <a name="introduction"></a>

__TheNtupleMaker__ (TNM) is a tool that automates the creation of simple [ROOT](http://root.cern.ch) ntuples from data in the (EDM) format developed and used by the [CMS Collaboration](https://cms.cern/). In particular, TNM can be run on CMS mini Analysis Object Data (miniAOD) files.  It also automatically generates a __C++__ and __Python__ analyzer skeleton that can be the basis of code for analyzing the contents of the ntuples. 
TNM works with miniAODs built with ROOT 5, therefore, it is compatible with all versions of CMSSW (https://github.com/cms-sw/cmssw), the CMS Collaboration's codebase, which depends on ROOT 5.  A version of TNM that works with ROOT 6 is under development.

In December 2009, [Sezen Sekmen](https://www.fizikist.com/sezen-sekmen-ile-cern-ve-parcacik-fizigi-uzerine-soylesi/) had an ephiphany. She noted that in spite of the complexity of the data formats used by collaborations such as CMS, and our quarter-centruy infatuation with object oriened programming and C++ objects, in particular, the data that are ultimately used in a physics analysis are simply a collection of numbers each of which is in one-to-one correspondence with an access function that returns a simple type, typically, a floating point number or an integer. Therefore, argued Sekmen, it ought to be possible to build a tool that allows the user to call automatically any combination of these access functions and thereby create the desired combination of data packaged as a ROOT ntuple. In CMSSW these access functions, which number in the thousabds, are packaged into C++ classes. 

TNM, which was developed by Harrison Prosper and Sezen Sekmen, is the first realization of Sekmen's idea and the first step towards the ultimate goal of creating a portal, with something like TNM as a backend, in which access to particle physics data would be a matter of making intuitive queries about what data are available, selecting them, pressing a button and creating an ntuple that can be transparently accessed using ROOT or whatever it evolves into.

## Installation <a name="installation"></a>

We provide instructions for installing TNM within a [docker](https://www.docker.com/) container (a secure, consistent, software environment isolated from the host), specifically one that provides access to the CMS [open data](http://opendata.cern.ch/) consistent with CMSSW version CMSSW_5_3_32. (Note that TNM can be used with any version of CMSSW built using ROOT 5.)
The instructions given are for a Mac, which in addition to __docker__ requires an installation of __[XQuartz](https://www.xquartz.org/)__. When active, XQuartz makes it possible for graphical user interfaces to be used within a docker container (that is, it provides X11 forwarding).

### 1. Configure XQuartz
Run XQuartz (which is located in Applications/Utilities). Click on the XQuartz menu item, then select Preferences. Under Security check *Allow connections from network clients*. Exit XQuartz and re-run to ensure that the settings have taken effect. Now open a terminal window. In that window, be sure to make the host name of your laptop known to X11 using the command `xhost + \`hostname\``. If that does not work, try restarting your laptop, restart the docker daemon, and XQuartz, and try again. (Note that the host name may already be known to X11. You can see this simply by executing the command `xhost` and checking the listing. The host name, perhaps in lowercase, should be listed.)

### 2. Create and run a docker container

In the new terminal window, create a container, here called *testme*, using the image *cmsopendata/cmssw_5_3_32*. (Of course, you can choose whatever name you like for the container. By the way, to remove a container do `docker rm <container-name>`.)
```bash
docker run -it -v $HOME/.ssh:/home/cmsur/.ssh -v $HOME/.Xauthority:/home/cmsusr/.Xauthority -v $HOME:/home/cmsusr/hosthome --net=host --env="DISPLAY=`hostname`:0" --name testme cmsopendata/cmssw_5_3_32 /bin/bash
```
The table below briefly describes the various switches used with the docker command. Note the use of backslashes with the command __hostname__.

| __switch__                   | __description__     |
| :---          | :--- |
-it     | run container in interactive mode |
-v $HOME/.ssh:/home/cmsur/.ssh          | mount the host's .ssh folder at the container mount point of the same name |
-v $HOME/.Xauthority:/home/cmsusr/.Xauthority | mount the host's .Xauthority folder at the container mount point of the same name | 
-v $HOME:/home/cmsusr/hosthome | mount the home folder of the host at container mount point hostname |
--net=host | allow network connections via host |
--env="DISPLAY=\`hostname\`:0" | set environment variable DISPLAY in container to the host name |
--name tnm | name of container |
cmsopendata/cmssw_5_3_32 | image to run |
/bin/bash | shell to be used in container |

You may want to add the following commands to `.bash_profile`
```bash
alias ls="ls --color"
PS1="docker/\W> "
```
and do `source ~/.bash_profile` to tidy up the command line prompt. You should already be in `$HOME/CMSSW_3_5_32/src`, if not move there and execute the command `cmsenv`. Then, to check that the X11 forwarding is working execute the command `root`. The `root` splash screen should appear. If it does, X11 forwarding is working.

### Download and build TheNtupleMaker

Make sure you are in the folder `$HOME/CMSSW_3_5_32/src` and you have executed the command `cmsenv`. Then do
```bash
mkdir PhysicsTools
git clone git://github.com/hbprosper/TheNtupleMaker
cd TheNtupleMaker
```
CMSSW Data formats are slightly version-dependent. But, TNM is designed to be version-independent, which is achieved by running the command
```bash
scripts/initTNM.py
```
This script makes a valiant attempt to guess which of hundreds of C++ classes are most likely to be of interest to those doing physics analysis.  TNM can now be built using the command below
```bash
scram b -j K
```
where *K* should be replaced with the number of cores at your disposal. If you don't know just omit the `-j` switch. If the build succeeds, which should take just a few minutes, you are ready to use TNM.

## Tutorial
In this tutorial, we shall assume you have a miniAOD called __reco.root__ in your TNM area, or a soft link (created with the command `ln -s path-to-root-file reco.root`) with that name. The first thing to be done is to create, either by hand or better still using the horribly command __mkntuplecfi.py__, which runs a GUI that allows ou to build a configuration file that specifies which methods are to be called by TNM in order to extract the data of interest from __reco.root__.  The command runs a GUI that looks like this

[mkntuplecfi](./mkntuplecfi.png)


The methods to be called by TNM are selected (or deselected) from the =Methods= tab, while the =Selected Methods= tab can be used to check which methods have been selected. Once a configuration file fragment has been created, it can be edited by hand. *Note*: the GUI is just an aid; it does not list every possible method known to TNM, but just the ones that are most likely to be of interest. You are free to add methods to the configuration file by hand. If  you add a method that is  not known to TNM, the latter will warn you at runtime.

<br><p>The steps for creating an ntuple are:
   1. *Run mkntuplecfi.py*  from your TNM area to launch the GUI. The first time the command =mkntuplecfi.py= is executed, in a given directory, the GUI uses the list of classes in the TNM file =plugins/classlist.txt= to create a directory called =methods=, which, for each class,  contains a text file listing methods of that class that return fundamental types (e.g., int, float, double, etc.). (Directories =txt= and =html= are also created.)
   1. *Create configuration file* 
      i. Use the =File= menu to open =reco.root=.
      i. Select =File &#8594; Open= from the menu or press the =Open an EDM file= button (at the top left of the GUI).
      i. From the window that appears, select =reco.root= and press =Open=.
      i. Under the =Classes= column, select the class of interest (e.g. =vector<pat::Jet>=). The selected class will be highlighted and all methods of the selected class that return simple types will appear in the =Methods= column and all categories available for the class will appear in the =Category= column (formerly, called =getByLabel=). 
      i. Select a category (e.g., =slimmedJet=). The selected category will be highlighted.
      i. Select methods of interest (e.g., =double pt()=, =double eta()=, etc.). The selected methods will be highlighted. The GUI has a rudimentary =Find method= entry window, but it is currently a tad brain dead in that you need to type the _exact_ name of the method and press =Enter= in order to search for it in the list of methods. 
      i. Repeat the appropriate steps above to select all the methods of interest. You can check which methods have been selected by going to the =Selected Methods= tab. Classes and methods can be deselected from the =Classes= and =Methods= lists by clicking on the names again, whereupon they will no longer be highlighted.
      i. When your selection is complete, select =File &#8594; Save= from the menu or press the =Save configuration file fragment= button (the second button in the toolbar) to save the configuration fragment. The default name for the configuration file fragment is =ntuple_cfi.py=. You should either save the file directly into your =python= directory, or copy the saved file to that directory after you exit the GUI. 
      i. Exit the GUI. 
   1. *Create the ntuple* by executing the command =cmsRun <configuration-file>=, for example:
<pre class="command">
cmsRun Floyd_cfg.py
</pre>
In your directory, you will find a file called =variables.txt= containing the list of ntuple variables, a file called =triggerNames.txt= containing the list of triggers found by TNM, the ntuple file =ntuple.root=, and a  directory called =analyzer= that contains the automatically created analyzer code. The name of the ntuple  file is specified in the configuration file. 

<p>Go to the analyzer directory and check that all is well by doing the following
<pre class="command">
cd analyzer
source setup.sh
make
echo ../ntuple.root > filelist.txt

./analyzer  

and also try the Python version

./analyzer.py
</pre>
If all goes well, in your directory you will find the file =analyzer_histograms.root=, which of course will be empty!
<br>
<br>



