#
# Makefile for the ALPHA-g analyzer
#



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

INCDIR += -I$(ROOTANASYS)/include $(shell cat $(ROOTANASYS)/include/rootana_cflags.txt)
LIBS     += -L$(ROOTANASYS)/lib -lrootana $(shell cat $(ROOTANASYS)/include/rootana_libs.txt)
LIBS     += -lSpectrum -lMinuit2

#CXXFLAGS += -Iana/include
INCDIR += -I../recolib/include
INCDIR += -I../analib/include
INCDIR += -I../aged 
INCDIR += -I../ana/include
INCDIR += -I../a2lib/include
INCDIR += -I../a2lib/legacy
# select the main program - local custom main()
#MAIN := manalyzer_main.o
# or standard main() from rootana
MAIN := $(ROOTANASYS)/obj/manalyzer_main.o

# uncomment and define analyzer modules here

#Legacy reconstruction modules
LEGACY += fake_realtime_module.o
LEGACY += unpack_vf48_module.o 
LEGACY += sis_module.o
LEGACY += mt_hybrid_hits_module.o
##Proto type module, unfinished:
#LEGACY += asic_filter_module.o
LEGACY += talphaevent_module.o
LEGACY += dumper_module.o
#Turn off debug flags for online mva (gcc4.8 gets upset at the number of variables)
LEGACY += online_mva_module.oo 
LEGACY += vertex_display_module.o
LEGACY += official_A2_time_module.o
#Spill log dumps:
LEGACY += dump_marker_module.o
LEGACY += spill_log_module.o
LEGACY += catch_efficiency_module.o

LEGACY := $(patsubst %.o,$(OBJDIR)/%.o,$(LEGACY))
LEGACY := $(patsubst %.oo,$(OBJDIR)/%.oo,$(LEGACY))


#a2ana.exe Reconstruction modules
A2ANA += unpack_vf48_module.o 
#A2ANA += zero_supp_vf48_module.o
A2ANA += mt_hybrid_hits_module.o
#A2ANA += sil_match_module.o
#A2ANA += reco_module.o
A2ANA := $(patsubst %.o,$(OBJDIR)/%.o,$(A2ANA))

#Full reconstruction (development branch style):
MODULES += eos_module.o handle_sequencer.o 
MODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(MODULES))

#Leading edge analysis (master branch style):
ONLY_STRIPS += eos_module.o 
ONLY_STRIPS += unpack_vf48_module.o 
ONLY_STRIPS += strip_peds.o
ONLY_STRIPS := $(patsubst %.o,$(OBJDIR)/%.o,$(ONLY_STRIPS))

STRIPS += strip_peds.o
STRIPS := $(patsubst %.o,$(OBJDIR)/%.o,$(STRIPS))

LAST_MODULES +=  AnalysisReport_module.o
LAST_MODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(LAST_MODULES))

RLIBS = -L../a2lib -L../recolib -L../analib -lalpha2 -lagana -lAGTPC
#USER_LIBS = libagana.so libAGTPC.so libaged.so

ALL += linkdirs gitinfo
BIN += $(BINDIR)/alphaStrips.exe $(BINDIR)/alphaAnalysis.exe $(BINDIR)/a2ana.exe  $(BINDIR)/alphaOnline.exe

all: CXXFLAGS += -O3 -g -Wall -Wuninitialized
all: $(ALL)
all: $(MODULES)
all: $(BIN)

debug: CXXFLAGS += -O0 -g -Wall -Wuninitialized
debug: $(ALL) $(MODULES) $(BIN)

native: CXXFLAGS += -O3 -g -Wall -Wuninitialized -march=native
native: $(ALL) $(MODULES) $(BIN)

O2: CXXFLAGS += -O2 -g -Wall -Wuninitialized
O2: $(ALL) $(MODULES) $(BIN)

less_mt: CXXFLAGS += -O2 -g -Wall -Wuninitialized -DNO_EXTRA_MT
less_mt: $(ALL) $(MODULES) $(BIN)

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
$(BINDIR)/alphaStrips.exe:$(MAIN) $(ONLY_STRIPS) $(LAST_MODULES) $(USER_LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN) $(ONLY_STRIPS)  $(LAST_MODULES) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

#Andrea and Lars style track reconstruction
$(BINDIR)/alphaAnalysis.exe:$(MAIN) $(MODULES) $(LEGACY)  $(LAST_MODULES) $(USER_LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN) $(MODULES) $(LEGACY)  $(LAST_MODULES) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics -lGeom 


$(BINDIR)/alphaOnline.exe:$(MAIN) $(MODULES) $(LEGACY)  $(LAST_MODULES) $(USER_LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN) $(MODULES) $(LEGACY) $(STRIPS) $(LAST_MODULES) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics -lGeom 


$(BINDIR)/a2ana.exe:$(MAIN) $(MODULES) $(A2ANA)  $(LAST_MODULES) $(USER_LIBS)
	$(CXX) $(CXXFLAGS)  -o $@ $(MAIN) $(MODULES) $(A2ANA)  $(LAST_MODULES) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics


$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) $(CXXFLAGS) -o $@ -I$(INCDIR) -c $<

$(OBJDIR)/%.oo: $(SRCDIR)/%.cxx
	$(CXX) -o $@  $(filter-out -g,$(CXXFLAGS)) -I$(INCDIR) -c $<

clean::
	-rm -f $(OBJDIR)/*.o $(OBJDIR)/*.oo *.a ../agana.exe ../agana_noreco.exe ../ag_events.exe

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
