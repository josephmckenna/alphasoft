#
# Makefile for the ALPHA-g analyzer
#



OBJDIR=obj
SRCDIR=src
INCDIR=include
BINDIR=..

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


LIBS     += -L$(ROOTANASYS)/lib -lrootana $(shell cat $(ROOTANASYS)/include/rootana_libs.txt)
LIBS     += -lSpectrum
LIBS     += -L$(shell root-config --libdir)

INCS += -I$(INCDIR)
INCS += -I../recolib/include
INCS += -I../analib/include
INCS += -I../aged/include
INCS += -I$(shell root-config --incdir)
#ALPHA 2 headers...
INCS += -I../alpha2/include
INCS += -I../a2lib/include
INCS += -I../a2lib/legacy
INCS += -I$(ROOTANASYS)/include $(shell cat $(ROOTANASYS)/include/rootana_cflags.txt)

# select the main program - local custom main()
#MAIN := manalyzer_main.o
# or standard main() from rootana
MAIN := $(ROOTANASYS)/obj/manalyzer_main.o

# uncomment and define analyzer modules here
#RMODULES = bscint_adc_module.o # NM
RMODULES = bsc_adc_module.o     # AC
##RMODULES += tdc_module.o
RMODULES += bsc_tdc_module.o	# NM
#RMODULES += fast_deconv_module.o match_module.o reco_module.o
#RMODULES += deconv_module.o match_module.o reco_module.o
RMODULES += deconv_aw_module.o deconv_pad_module.o match_module.o reco_module.o
# RMODULES += adc_module.o pwb_module.o feam_module.o tpc_tree_module.o
RMODULES += histo_module.o
RMODULES += phspectrum_module.o
RMODULES += calib_module.o
RMODULES += display_module.o
RMODULES += official_time_module.o
RMODULES += AnalysisReport_module.o
RMODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(RMODULES))

#Modules needed to build a spill log (Sequencer xml and chronobox timing)
LOGMODULES = handle_sequencer.o chrono_module.o spill_log_module.o AnalysisReport_module.o
LOGMODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(LOGMODULES))

#Full reconstruction (development branch style):
MODULES += eos_module.o handle_sequencer.o chrono_module.o
MODULES += ncfm.o unpack_module.o
MODULES += Alpha16.o
MODULES += TsSync.o Feam.o Tdc.o FeamEVB.o FeamAsm.o
MODULES += PwbAsm.o AgEvent.o AgEVB.o TrgAsm.o
MODULES += Unpack.o AgAsm.o
MODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(MODULES))
MODULES += $(RMODULES)
#Leading edge analysis (master branch style):
NO_RECO_MODULES += eos_module.o handle_sequencer.o chrono_module.o
NO_RECO_MODULES += ncfm.o unpack_module.o adc_module.o
NO_RECO_MODULES += pwb_module.o Alpha16.o feam_module.o
NO_RECO_MODULES += TsSync.o Feam.o Tdc.o FeamEVB.o FeamAsm.o
NO_RECO_MODULES += PwbAsm.o AgEvent.o AgEVB.o TrgAsm.o
NO_RECO_MODULES += Unpack.o AgAsm.o wfexport_module.o
NO_RECO_MODULES += final_module.o coinc_module.o
# NO_RECO_MODULES += tpc_tree_module.o # LM
NO_RECO_MODULES += bsc_module.o # KO
NO_RECO_MODULES += tdc_module.o # AC
NO_RECO_MODULES += AnalysisReport_module.o
NO_RECO_MODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(NO_RECO_MODULES))

RLIBS = -L../recolib -L../aged -L../analib -lagana -lAGTPC -laged
#USER_LIBS = libagana.so libAGTPC.so libaged.so

ALL += linkdirs gitinfo
BIN += $(BINDIR)/agana_noreco.exe $(BINDIR)/agana.exe $(BINDIR)/ag_events.exe

all:: CXXFLAGS += -O3 -g -Wall -Wuninitialized
all:: $(ALL)
all:: $(MODULES)
all:: $(BIN)

debug: CXXFLAGS += -O0 -g -Wall -Wuninitialized
debug: $(ALL) $(MODULES) $(BIN)

native: CXXFLAGS += -O3 -g -Wall -Wuninitialized -march=native
native: $(ALL) $(MODULES) $(BIN)

O2: CXXFLAGS += -O2 -g -Wall -Wuninitialized
O2: $(ALL) $(MODULES) $(BIN)


linkdirs:
	mkdir -p obj

gitinfo:
	$(shell ./GitInfo.sh include)


#Konstatin style leading edge reconstruction
$(BINDIR)/agana_noreco.exe:$(MAIN) $(NO_RECO_MODULES) $(USER_LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN) $(NO_RECO_MODULES) $(INCS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit2 -lPhysics

#Andrea and Lars style track reconstruction
$(BINDIR)/agana.exe:$(MAIN) $(MODULES) $(USER_LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN) $(MODULES) $(INCS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lMinuit2 -lPhysics

#Spill log style online analysis
$(BINDIR)/ag_events.exe:$(MAIN) $(LOGMODULES) $(USER_LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN) $(LOGMODULES) $(INCS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit2 -lPhysics

$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) $(CXXFLAGS) -o $@  $(INCS) -c $<

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
