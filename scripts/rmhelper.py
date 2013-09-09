import os, re, glob, sys

#=======================================================================# 
#  Get helper name from commandline, generate alternate syntaxes        #
#=======================================================================#
helpername = sys.argv[1]
if "::" in helpername:
    print "syntax error. argument 1 requires no colons"
    exit()
helpernameCMSSWsyntax = ""
capindex = 0
charindex=0
for c in helpername:
    if c in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
        capindex = charindex
        break
    charindex+=1
helpernameCMSSWsyntax = helpername[0:capindex] + "::" + helpername[capindex:]
print "helper name :"+helpername
print "CMSSW-speak :" + helpernameCMSSWsyntax




#=======================================================================# 
#  Identify helper class and save it as a string                        #
#=======================================================================#
helperclass = "unknown"

if os.path.exists("src/"+helpername+".cc"):
    f = open("src/"+helpername+".cc","r")
    lines = f.readlines()
    f.close
    for line in lines:
        if re.search(": HelperFor<", line, re.IGNORECASE):
            beginning = line.index("<")+1
            end = line.index(">")
            helperclass = line[beginning:end]
            break
    print "helper class:"+ helperclass
else:
    print "source file for "+ helpername + " not found"
    exit()



#=======================================================================# 
#  Figure out if the given helper is the only one of its class          #
#=======================================================================#
filelist = glob.glob("src/*.cc")
listsize = len(filelist)
NumberOfHelpersForClass = 0
for filename in filelist:
    f = open(filename,"r")
    lines = f.readlines()
    for line in lines:
        if "  : HelperFor<"+helperclass+">() {}\n" in line:
            NumberOfHelpersForClass+=1
            break
    f.close
if NumberOfHelpersForClass > 1:
    print "Clearly you had too many helpers. Good move."
if NumberOfHelpersForClass == 0:
    print "Error: something terrible happened"



#=======================================================================# 
#  Delete files in package/plugins                                      #
#=======================================================================#
if os.path.exists("interface/"+helpername+".h"):
    os.remove("interface/"+helpername+".h")




#=======================================================================# 
#  Delete and modify files in package/plugins                           #
#=======================================================================#
if os.path.exists("plugins/userplugin_"+helpername+".cc"):
    os.remove("plugins/userplugin_"+helpername+".cc")

    
f = open("plugins/BuildFile.xml", "r")
lines = f.readlines()
f.close()
f= open("plugins/BuildFile.xml","w")
PrimaryTarget = '<library file="userplugin_'+helpername+'.cc" name="'+helpername+'">'+'\n'
SecondaryTarget = False
for line in lines:
    if line==PrimaryTarget:
        SecondaryTarget = True
    if not (line==PrimaryTarget or SecondaryTarget):
        f.write(line)
    if(SecondaryTarget and line!=PrimaryTarget):
        SecondaryTarget = False
f.close()


g = open("plugins/BuildFile.xml~", "r")
lines = g.readlines()
g.close()
g = open("plugins/BuildFile.xml~","w")
PrimaryTarget = '<library file="userplugin_'+helpername+'.cc" name="'+helpername+'">'+'\n'
SecondaryTarget = False
for line in lines:
    if line==PrimaryTarget:
        SecondaryTarget = True
    if not (line==PrimaryTarget or SecondaryTarget):
        g.write(line)
    if(SecondaryTarget and line!=PrimaryTarget):
        SecondaryTarget = False
g.close()



#=======================================================================# 
#  Delete and modify files in package/src                               #
#=======================================================================#
    
h = open("src/classes.h", "r")
lines = h.readlines()
h.close()
h = open("src/classes.h","w")
if NumberOfHelpersForClass > 1:
    for line in lines:
        if not re.search(helpername+".h", line, re.IGNORECASE):#consider putting .h on end of helpername
            h.write(line)
elif NumberOfHelpersForClass == 1:
    for line in lines:
        if not (helpername+".h" in line or \
                "HelperFor<"+helperclass+">" in line):
            h.write(line)
h.close()


i = open("src/classes_def.xml", "r")
lines = i.readlines()
i.close()
i = open("src/classes_def.xml","w")
if NumberOfHelpersForClass > 1:
    print "other helpers exist for this class...keeping additional 'HelperFor' statements"
    for line in lines:
        if not "<class name=\""+helpernameCMSSWsyntax+"\"/>" in line:
            i.write(line)
elif NumberOfHelpersForClass == 1:
    print "no other helpers for this class...deleting relevent 'HelperFor' statements"
    for line in lines:
        if not ("<class name=\""+helpernameCMSSWsyntax+"\"/>" in line or \
                "<class name=\"HelperFor<"+helperclass+">\"/>" in line): 
            i.write(line)
i.close() 


if os.path.exists("src/"+helpername+".cc"):
    os.remove("src/"+helpername+".cc")

#=======================================================================# 
#  Declare victory                                                      #
#=======================================================================#

print helpername +" removed"
