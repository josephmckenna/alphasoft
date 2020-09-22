#
# Makefile for the ALPHA-2 analyzer
#
define get_modules
	$(info Get modules for ${2} in file: $(patsubst $(BINDIR)/%.exe,%.modules,$(2)))
	$(1)+= $$(patsubst %.o,$(OBJDIR)/%.o,$(shell grep -v '\#' $(patsubst $(BINDIR)/%.exe,%.modules,$(2))))
endef


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
LIBS     += -lSpectrum -lGeom -lMinuit2
LIBS     += -L$(shell root-config --libdir)

INCS += -I$(INCDIR)
INCS += -I../recolib/include
INCS += -I../ana/include
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

BINS = $(shell ls *.modules)
#Strip out the extension:
BINS := $(patsubst %.modules,$(BINDIR)/%.exe,$(BINS))

$(foreach BIN,$(BINS),$(eval $(call get_modules,ALL_DEPS,${BIN})))

$(info Building binaries: [${BINS}])
$(info All dependancies: ${ALL_DEPS})

RLIBS = -L../a2lib -L../recolib -L../analib -lalpha2 -lanalib -lagtpc
#USER_LIBS = libanalib.so libagtpc.so libaged.so

ALL += linkdirs gitinfo
#BIN += $(BINDIR)/alphaStrips.exe $(BINDIR)/alphaAnalysis.exe $(BINDIR)/a2ana.exe  $(BINDIR)/alphaOnline.exe
BIN += $(BINS)

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

.PRECIOUS: $(ALL_DEPS)

less_mt: CXXFLAGS += -O2 -g -Wall -Wuninitialized -DNO_EXTRA_MT
less_mt: $(ALL) $(MODULES) $(BIN)

linkdirs:
	mkdir -p obj

gitinfo:
	./GitInfo.sh include

$(BINDIR)/%.exe: $(MAIN) $(USER_LIBS) $(ALL_DEPS) $(MODS)
	$(eval $(call get_modules,$@MODS,$@))
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN) $($@MODS) $(INCS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit2 -lPhysics


$(OBJDIR)/online_mva_module.o: $(SRCDIR)/online_mva_module.cxx
	$(CXX) -o $@  $(filter-out -g,$(CXXFLAGS)) $(INCS) -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
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
