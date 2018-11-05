#
# Makefile for the ALPHA-g analyzer
#


CXXFLAGS += -O3 -g -Wall -Wuninitialized -Iinclude

OBJDIR=obj
SRCDIR=src
INCDIR=include

ifndef ROOTANASYS
norootanasys:
	@echo Error: ROOTANASYS in not defined, please source thisrootana.{sh,csh}
endif
ifndef AGRELEASE
noagrelease:
	@echo Error: AGRELEASE is not defuled, please source agconfig.sh
endif

ifneq ($(ROOTSYS),)
HAVE_ROOT:=1
LIBS   += -lSpectrum
endif

# get the rootana Makefile settings

CXXFLAGS += -I$(ROOTANASYS)/include $(shell cat $(ROOTANASYS)/include/rootana_cflags.txt)
LIBS     += -L$(ROOTANASYS)/lib -lrootana $(shell cat $(ROOTANASYS)/include/rootana_libs.txt)
#LIBS += -L.
LIBS   += -lSpectrum

CXXFLAGS += -I../reco/include
CXXFLAGS += -I../analib/include
CXXFLAGS += -I../aged

# select the main program - local custom main()
# or standard main() from rootana

#MAIN := manalyzer_main.o
MAIN := $(ROOTANASYS)/obj/manalyzer_main.o

# uncomment and define analyzer modules here
RMODULES = deconv_module.o match_module.o reco_module.o
RMODULES += bscint_adc_module.o
RMODULES += histo_module.o
RMODULES += calib_module.o
RMODULES += display_module.o
RMODULES += official_time_module.o
RMODULES += AnalysisReport_module.o
RMODULES := $(patsubst %.o,obj/%.o,$(RMODULES))

#Modules needed to build a spill log (Sequencer xml and chronobox timing)
LOGMODULES = handle_sequencer.o chrono_module.o spill_log_module.o AnalysisReport_module.o
LOGMODULES := $(patsubst %.o,obj/%.o,$(LOGMODULES))

#Full reconstruction (development branch style):
MODULES += eos_module.o handle_sequencer.o chrono_module.o
MODULES += ncfm.o unpack_module.o 
MODULES += Alpha16.o 
MODULES += TsSync.o Feam.o Tdc.o FeamEVB.o FeamAsm.o 
MODULES += PwbAsm.o AgEvent.o AgEVB.o TrgAsm.o
MODULES += Unpack.o AgAsm.o 
MODULES := $(patsubst %.o,obj/%.o,$(MODULES))
MODULES += $(RMODULES)
#Leading edge analysis (master branch style):
NO_RECO_MODULES += handle_sequencer.o chrono_module.o
NO_RECO_MODULES += ncfm.o unpack_module.o adc_module.o 
NO_RECO_MODULES += pwb_module.o Alpha16.o feam_module.o 
NO_RECO_MODULES += TsSync.o Feam.o Tdc.o FeamEVB.o FeamAsm.o 
NO_RECO_MODULES += PwbAsm.o AgEvent.o AgEVB.o TrgAsm.o 
NO_RECO_MODULES += Unpack.o AgAsm.o wfexport_module.o 
NO_RECO_MODULES += final_module.o coinc_module.o 
NO_RECO_MODULES += bsc_module.o
NO_RECO_MODULES += AnalysisReport_module.o
NO_RECO_MODULES := $(patsubst %.o,obj/%.o,$(NO_RECO_MODULES))

RLIBS = -L../reco -L../aged -L../analib -lagana -lAGTPC -laged
USER_LIBS = libagana.so libAGTPC.so libaged.so

ALL += linkdirs gitinfo $(USER_LIBS)
BIN += agana_noreco.exe agana.exe ag_events.exe 


all:: $(ALL)
all:: $(MODULES)
all:: $(BIN)

linkdirs:
	mkdir -p obj

gitinfo: 
	@echo "#define GIT_DATE            " $(shell git log -n 1 --date=raw | grep Date | cut -b 8-19) >include/GitInfo.h
	@echo "#define GIT_REVISION      \"" $(shell git rev-parse --short HEAD ) "\"" >> include/GitInfo.h
	@echo "#define GIT_REVISION_FULL \"" $(shell  git log -n 1 | grep commit | cut -b 8-99) "\"" >> include/GitInfo.h
	@echo "#define GIT_BRANCH        \""' $(shell git branch --remote --no-abbrev --contains) '"\"" >> include/GitInfo.h
	@echo "#define GIT_DIFF_SHORT_STAT \""' $(shell git branch --no-abbrev --contains) : $(shell git diff --shortstat) '"\"" >> include/GitInfo.h
	@echo "#define COMPILATION_DATE    " $(shell date +%s) >>include/GitInfo.h


#Konstatin style leading edge reconstruction
agana_noreco.exe:$(MAIN) $(NO_RECO_MODULES) $(USER_LIBS)
	$(CXX) -o $@ $(MAIN) $(NO_RECO_MODULES) $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

#Andrea and Lars style track reconstruction
agana.exe:$(MAIN) $(MODULES) $(USER_LIBS)
	$(CXX) -o $@ $(MAIN) $(MODULES) $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

#Spill log style online analysis
ag_events.exe:$(MAIN) $(LOGMODULES) $(USER_LIBS)
	$(CXX) -o $@ $(MAIN) $(LOGMODULES)  $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

libAGTPC.so:
	cd ../reco/ && $(MAKE)

libaged.so:
	cd ../aged/ && $(MAKE)

libagana.so:
	cd ../analib/ && $(MAKE)

html/index.html:
	-mkdir html
	-make -k dox
	touch html/index.html

dox:
	doxygen

clean::
	-rm -f $(OBJDIR)/*.o *.a *.exe

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
	rm -f include/GitInfo.h libaged.so libAGTPC.so agtpc_rdict.pcm libagana.so analib_rdict.pcm
	cd ../reco/ && $(MAKE) clean
	cd ../analib/ && $(MAKE) clean
	cd ../aged/ && $(MAKE) clean

# end
