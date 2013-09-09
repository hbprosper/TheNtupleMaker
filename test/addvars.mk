#------------------------------------------------------------------------------
# Created: Tue May  8 03:47:22 2012
#------------------------------------------------------------------------------
name	:= addvars
AT      := @
#------------------------------------------------------------------------------
CINT	:= rootcint
CXX     := g++
LDSHARED:= g++
#------------------------------------------------------------------------------
DEBUG	:= -ggdb
CPPFLAGS:= -I. $(shell root-config --cflags)
CXXFLAGS:= $(DEBUG) -pipe -O2 -fPIC -Wall
LDFLAGS := -shared 
#------------------------------------------------------------------------------
LIBS	:= $(shell root-config --glibs)
#------------------------------------------------------------------------------
linkdef	:= $(name)Linkdef.h
header  := addvars.h
cinthdr := dict.h
cintsrc	:= dict.cc
cintobj	:= dict.o

cppsrc 	:= $(name).cc
cppobj  := $(name).o

objects	:= $(cintobj) $(cppobj) 
library	:= lib$(name).so
#-----------------------------------------------------------------------
lib:	$(library)

$(library)	: $(objects)
	@echo "---> Linking $@"
	$(AT)$(LDSHARED) $(LDFLAGS) $+ $(LIBS) -o $@
	@rm -rf $(cinthdr) $(cintsrc) $(objects) 

$(cppobj)	: $(cppsrc)
	@echo "---> Compiling `basename $<`" 
	$(AT)$(CXX)	$(CXXFLAGS) $(CPPFLAGS) -c $<

$(cintobj)	: $(cintsrc)
	@echo "---> Compiling `basename $<`"
	$(AT)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

$(cintsrc) : $(header) $(linkdef)
	@echo "---> Generating dictionary `basename $@`"
	$(AT)$(CINT) -f $@ -c $(CPPFLAGS) $+

$(linkdef)	:
	@echo "---> Creating $(linkdef)"
	@echo -e "#include <map>"  >	$(linkdef)
	@echo -e "#include <string>" >>	$(linkdef)
	@echo -e "#include <vector>" >>	$(linkdef)
	@echo -e "#ifdef __CINT__" >> $(linkdef)
	@echo -e "#pragma link off all globals;" >> $(linkdef)
	@echo -e "#pragma link off all classes;" >> $(linkdef)
	@echo -e "#pragma link off all functions;" >> $(linkdef)
	@echo -e "#pragma link C++ class countvalue+;" >> $(linkdef)
	@echo -e "#pragma link C++ class map<string, countvalue>+;" 	>> $(linkdef)
	@echo -e "#pragma link C++ class map<string, vector<int> >+;" 	>> $(linkdef)
	@echo -e "#pragma link C++ class addvars+;" 	>> $(linkdef)	
	@echo -e "#endif" >> $(linkdef)

clean   :
	@rm -rf dict.* $(objects)  $(library) $(linkdef)
