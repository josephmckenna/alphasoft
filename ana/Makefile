#
# Makefile for the MIDAS analyzer component of ROOTANA
#
# Options:
# make NO_ROOT=1 # build without ROOT support
#

#CXXFLAGS += -g -O2 -Wall -Wuninitialized -I.
#CXXFLAGS += -pg -Wall -Wuninitialized -I.
CXXFLAGS += -O3 -g -Wall -Wuninitialized -I.
#CXXFLAGS += -g -O0 -pg -Wall -Wuninitialized -I.

ifndef ROOTANASYS
norootanasys:
	@echo Error: ROOTANASYS in not defined, please source thisrootana.{sh,csh}
endif
ifndef AGRELEASE
noagrelease:
	@echo Error: AGRELEASE is not defuled, please source agconfig.sh
endif
# get the rootana Makefile settings

CXXFLAGS += -I$(ROOTANASYS)/include $(shell cat $(ROOTANASYS)/include/rootana_cflags.txt)
LIBS     += -L$(ROOTANASYS)/lib -lrootana $(shell cat $(ROOTANASYS)/include/rootana_libs.txt)
#LIBS += -L.
LIBS   += -lSpectrum

CXXFLAGS += -I../reco 
CXXFLAGS += -I../analib

# select the main program - local custom main()
# or standard main() from rootana

#MAIN := manalyzer_main.o
MAIN := $(ROOTANASYS)/obj/manalyzer_main.o

# uncomment and define analyzer modules here
RMODULES = deconv_module.o match_module.o reco_module.o
RMODULES += histo_module.o
RMODULES += calib_module.o
RMODULES += display_module.o


#Modules needed to build a spill log (Sequencer xml and chronobox timing)
LOGMODULES = handle_sequencer.o chrono_module.o spill_log_module.o

#MODULES += ncfm.o unpack_module.o adc_module.o pwb_module.o Alpha16.o feam_module.o TsSync.o Feam.o FeamEVB.o FeamAsm.o PwbAsm.o AgEvent.o AgEVB.o TrgAsm.o Unpack.o AgAsm.o $(RMODULES) wfexport_module.o final_module.o
MODULES += ncfm.o unpack_module.o Alpha16.o TsSync.o Feam.o FeamEVB.o FeamAsm.o PwbAsm.o AgEvent.o AgEVB.o TrgAsm.o Unpack.o AgAsm.o handle_sequencer.o chrono_module.o



RLIBS = -L../reco -L../aged -L../analib -lagana -lAGTPC -laged
USER_LIBS = libagana.so libAGTPC.so libaged.so

ALL += $(USER_LIBS)
ALL += agana_noreco.exe agana.exe ag_events.exe

all:: $(MODULES)
all:: $(ALL)

#Konstatin style leading edge reconstruction
agana_noreco.exe:$(MAIN) $(LOGMODULES) $(MODULES) $(USER_LIBS)
	$(CXX) -o $@ $(MAIN) $(MODULES) $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

#Andrea and Lars style track reconstruction
agana.exe:$(MAIN) $(LOGMODULES) $(MODULES) $(RMODULES) $(USER_LIBS)
	$(CXX) -o $@ $(MAIN) $(MODULES) $(RMODULES) $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

#Spill log style online analysis
ag_events.exe:$(MAIN) $(LOGMODULES) $(USER_LIBS)
	$(CXX) -o $@ $(MAIN) $(LOGMODULES)  $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

%.o: %.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

libAGTPC.so:
	cd ../reco/ && $(MAKE)
#	ln -s ../reco/libAGTPC.so .
#	ln -s ../reco/agtpc_rdict.pcm .

libaged.so:
	cd ../aged/ && $(MAKE)
#	ln -s ../aged/libaged.so .

libagana.so:
	cd ../analib/ && $(MAKE)
#	ln -s ../analib/libagana.so .
#	ln -s ../analib/analib_rdict.pcm .

html/index.html:
	-mkdir html
	-make -k dox
	touch html/index.html

dox:
	doxygen

clean::
	-rm -f *.o *.a *.exe

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
	rm -f libaged.so libAGTPC.so agtpc_rdict.pcm libagana.so analib_rdict.pcm
	cd ../reco/ && $(MAKE) clean
	cd ../analib/ && $(MAKE) clean

# end
