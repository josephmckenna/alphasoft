#
# Makefile for the ALPHA-g analyzer
#


CXXFLAGS += -O3 -g -Wall -Wuninitialized -Iinclude
#CXXFLAGS += -O0 -g -Wall -Wuninitialized -Iinclude

OBJDIR=obj
SRCDIR=src
INCDIR=include
BINDIR=.

ifndef ROOTANASYS
norootanasys:
	@echo Error: ROOTANASYS in not defined, please source thisrootana.{sh,csh}
endif
ifndef AGRELEASE
noagrelease:
	@echo Error: AGRELEASE is not defined, please source agconfig.sh
endif

ifneq ($(ROOTSYS),)
HAVE_ROOT:=1
LIBS   += -lSpectrum
endif

# get the rootana Makefile settings

CXXFLAGS += -I$(ROOTANASYS)/include $(shell cat $(ROOTANASYS)/include/rootana_cflags.txt)
LIBS     += -L$(ROOTANASYS)/lib -lrootana $(shell cat $(ROOTANASYS)/include/rootana_libs.txt)
LIBS     += -lSpectrum

#CXXFLAGS += -Iana/include
CXXFLAGS += -I../reco/include
CXXFLAGS += -I../analib/include
CXXFLAGS += -I../aged 
CXXFLAGS += -I../ana/include
CXXFLAGS += -I../a2lib/include

# select the main program - local custom main()
#MAIN := manalyzer_main.o
# or standard main() from rootana
MAIN := $(ROOTANASYS)/obj/manalyzer_main.o

# uncomment and define analyzer modules here

#Reconstruction modules
RMODULES += unpack_vf48_module.o zero_supp_vf48_module.o
RMODULES += AnalysisReport_module.o
RMODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(RMODULES))

#Modules needed to build a spill log (Sequencer xml and chronobox timing)
LOGMODULES = handle_sequencer.o sis_module.o spill_log_module.o AnalysisReport_module.o
LOGMODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(LOGMODULES))

#Full reconstruction (development branch style):
MODULES += eos_module.o handle_sequencer.o 
MODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(MODULES))
MODULES += $(RMODULES)
#Leading edge analysis (master branch style):
NO_RECO_MODULES += eos_module.o handle_sequencer.o sis_module.o
NO_RECO_MODULES += AnalysisReport_module.o
NO_RECO_MODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(NO_RECO_MODULES))

RLIBS = -L../a2lib -L../reco -L../aged -L../analib -lalpha2 -lagana -lAGTPC -laged
#USER_LIBS = libagana.so libAGTPC.so libaged.so

ALL += linkdirs gitinfo
BIN += $(BINDIR)/alphaStrips.exe $(BINDIR)/alphaAnalysis.exe $(BINDIR)/a2_events.exe


all:: $(ALL)
all:: $(MODULES)
all:: $(BIN)

linkdirs:
	mkdir -p obj

gitinfo:
	@echo "#define GIT_DATE            " $(shell git log -n 1 --date=raw | grep Date | cut -b 8-19) > include/GitInfo.h
	@echo "#define GIT_REVISION      \"" $(shell git rev-parse --short HEAD ) "\"" >> include/GitInfo.h
	@echo "#define GIT_REVISION_FULL \"" '$(shell  git log -n 1 | grep commit | cut -b 8-99) '"\"" >> include/GitInfo.h
	@echo "#define GIT_BRANCH        \""' $(shell git branch --remote --no-abbrev --contains) '"\"" >> include/GitInfo.h
	@echo "#define GIT_DIFF_SHORT_STAT \""' $(shell git branch --no-abbrev --contains) : $(shell git diff --shortstat) '"\"" >> include/GitInfo.h
	@echo "#define COMPILATION_DATE    " $(shell date +%s) >> include/GitInfo.h


#Konstatin style leading edge reconstruction
$(BINDIR)/alphaStrips.exe:$(MAIN) $(NO_RECO_MODULES) $(USER_LIBS)
	$(CXX) -o $@ $(MAIN) $(NO_RECO_MODULES) $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

#Andrea and Lars style track reconstruction
$(BINDIR)/alphaAnalysis.exe:$(MAIN) $(MODULES) $(USER_LIBS)
	$(CXX) -o $@ $(MAIN) $(MODULES) $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

#Spill log style online analysis
$(BINDIR)/a2_events.exe:$(MAIN) $(LOGMODULES) $(USER_LIBS)
	$(CXX) -o $@ $(MAIN) $(LOGMODULES)  $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

clean::
	-rm -f $(OBJDIR)/*.o *.a ../agana.exe ../agana_noreco.exe ../ag_events.exe

clean::
	-rm -f $(ALL)

clean::
	-rm -rf *.exe.dSYM

clean::
	-rm -rf html

#clean::
#	-mkdir OldCalib
#	-mv LookUp*.dat OldCalib

clean::
	rm -f include/GitInfo.h


# end
