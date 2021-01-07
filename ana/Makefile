#
# Makefile for the ALPHA-g analyzer
#
# Authors: KO, AC and JTMK 
#
define get_modules
	$(info Get modules for ${2} in file: $(patsubst $(BINDIR)/%.exe,%.modules,$(2)))
	$(1)+= $$(patsubst %.o,$(OBJDIR)/%.o,$(shell grep -v '\#' $(patsubst $(BINDIR)/%.exe,%.modules,$(2))))
	#Manually list agana libs needed for unpack_module by had here, cmake's solution (link to static lib) is more eligant 
	$(1)+=$(OBJDIR)/ncfm.o $(OBJDIR)/Alpha16.o $(OBJDIR)/TsSync.o $(OBJDIR)/Feam.o $(OBJDIR)/Tdc.o $(OBJDIR)/FeamEVB.o $(OBJDIR)/FeamAsm.o $(OBJDIR)/PwbAsm.o $(OBJDIR)/AgEvent.o $(OBJDIR)/AgEVB.o $(OBJDIR)/TrgAsm.o $(OBJDIR)/Unpack.o $(OBJDIR)/AgAsm.o 
	# wfsuppress.o wfsuppress2.o wfsuppress_pwb.o 
endef


<<<<<<< HEAD
=======
CXXFLAGS += -O3 -g -Wall -Wuninitialized -Iinclude
#CXXFLAGS += -O0 -g -Wall -Wuninitialized -Iinclude

CXXFLAGS += -DLASER

>>>>>>> laser
OBJDIR=obj
SRCDIR=src
#SRCDIR +=../agana
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


LIBS     += -L$(ROOTANASYS)/lib -lrootana
LIBS     += -lSpectrum -lz -lMinuit2 -lMinuit -lMathMore -lGui -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lTree  -lPhysics -lMathCore -lThread -lXMLParser -lXMLIO -lRHTTP
LIBS     += -L$(shell root-config --libdir)

INCS += -I$(INCDIR)
INCS += -I../recolib/include
INCS += -I../analib/include
INCS += -I../agana
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

<<<<<<< HEAD
BINS = $(shell ls *.modules)
#Strip out the extension:
BINS := $(patsubst %.modules,$(BINDIR)/%.exe,$(BINS))
=======
# uncomment and define analyzer modules here
#RMODULES = bscint_adc_module.o # NM
RMODULES = bsc_adc_module.o     # AC
##RMODULES += tdc_module.o
RMODULES += bsc_tdc_module.o	# NM
#RMODULES += fast_deconv_module.o match_module.o reco_module.o
RMODULES += deconv_module.o match_module.o reco_module.o
# RMODULES += adc_module.o pwb_module.o feam_module.o tpc_tree_module.o
RMODULES += histo_module.o
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
NO_RECO_MODULES += deconv_module.o tpc_tree_module.o # LM
NO_RECO_MODULES += bsc_module.o # KO
NO_RECO_MODULES += tdc_module.o # AC
NO_RECO_MODULES += AnalysisReport_module.o
NO_RECO_MODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(NO_RECO_MODULES))

RLIBS = -L../recolib -L../aged -L../analib -lagana -lAGTPC -laged
#USER_LIBS = libagana.so libAGTPC.so libaged.so
>>>>>>> laser

$(foreach BIN,$(BINS),$(eval $(call get_modules,ALL_DEPS,${BIN})))

$(info Building binaries: [${BINS}])
$(info All dependancies: ${ALL_DEPS})

RLIBS = -L../recolib -L../aged -L../analib -lanalib -lagtpc -laged
#USER_LIBS = libanalib.so libAGTPC.so libaged.so

ALL += linkdirs gitinfo
#BIN += $(BINDIR)/agana_noreco.exe $(BINDIR)/agana.exe $(BINDIR)/ag_events.exe
BIN += $(BINS)

all:: CXXFLAGS += -O3 -g -Wall -Wuninitialized $(BUILD_FLAGS)
all:: $(ALL)
all:: $(MODULES)
all:: $(BIN)

debug: CXXFLAGS += -O0 -g -Wall -Wuninitialized $(BUILD_FLAGS)
debug: $(ALL) $(MODULES) $(BIN)

native: CXXFLAGS += -O3 -g -Wall -Wuninitialized -march=native $(BUILD_FLAGS)
native: $(ALL) $(MODULES) $(BIN)

O2: CXXFLAGS += -O2 -g -Wall -Wuninitialized $(BUILD_FLAGS)
O2: $(ALL) $(MODULES) $(BIN)

.PRECIOUS: $(ALL_DEPS)


linkdirs:
	mkdir -p obj

gitinfo:
	./GitInfo.sh include

$(BINDIR)/%.exe: $(MAIN) $(USER_LIBS) $(ALL_DEPS) $(MODS)
	$(eval $(call get_modules,$@MODS,$@))
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN) $($@MODS) $(INCS) $(RLIBS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) $(CXXFLAGS) -o $@  $(INCS) -c $<

$(OBJDIR)/%.o: $(AGRELEASE)/agana/%.cxx
	$(CXX) $(CXXFLAGS) -o $@  $(INCS) -c $<

clean::
	-rm -f $(OBJDIR)/*.o *.a $(BINS)

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
