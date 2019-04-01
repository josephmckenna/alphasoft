BINDIR	= bin
LIBDIR	= .
INCDIR	= include

# MACHFLAG used to force 32bit compilation on 64bit machines, assuming a 32bit version of root
#MACHFLAG = -m32
MACHFLAG = $(shell $(ROOTSYS)/bin/root-config --ldflags)


SOFLAGS       = -shared

CXX        = $(shell root-config --cxx)
CC         = $(shell root-config --cc)
LD         = $(shell root-config --ld) $(MACHFLAG)

CXXFLAGS   = -I./$(INCDIR) -I$(shell $(ROOTSYS)/bin/root-config --incdir)
CXXFLAGS += -O2 -g  -Wall -Wno-deprecated -fPIC $(MACHFLAG) $(shell $(ROOTSYS)/bin/root-config --cflags)

ROOTLIBS  := $(shell $(ROOTSYS)/bin/root-config --libs) -Wl,-rpath,$(ROOTSYS)/lib
CFLAGS = -Wall -O2 -g -fPIC $(MACHFLAG)
ifdef ALPHA1COMP
CXXFLAGS += -DALPHA1COMP 
CFLAGS   += -DALPHA1COMP
endif
LIB        = $(ROOTLIBS)
LIBCALIB   = $(ROOTLIBS)

vpath %.h ./include
vpath %.cpp ./src

all: $(LIBDIR)/libAlphaAnalysis.so

#---------------------------------------------



CSRC    := $(wildcard src/*.c)
COBJ    := $(patsubst src/%.c,$(BINDIR)/%.o,$(CSRC))

CXXSRC  := $(wildcard src/*.cxx)
CXXOBJ  := $(patsubst src/%.cxx,$(BINDIR)/%.o,$(CXXSRC))

HDRS    := $(wildcard $(INCDIR)/T*.h)
HDRS    := $(patsubst $(INCDIR)/%,%,$(HDRS))
HDRS    := $(filter-out $(INCDIR)/sqlite3.h,$(HDRS))
#HDRS    := $(filter-out $(INCDIR)/alphaAnalysisLinkDef.h,$(HDRS))
#HDRS    += include/alphaAnalysisLinkDef.h


ifeq ($(PLATFORM),)
PLATFORM = $(shell root-config --arch)
endif
ALPHALIBDIR  = ../../alphavmc/lib/tgt_$(PLATFORM)
ALPHALIBINC = ../../alphavmc/include
ALPHALIB = $(ALPHALIBDIR)/libalphavmc.so
CXXFLAGS += -I$(ALPHALIBINC)
LDFLAGS += -L$(ALPHALIBDIR)

OBJS    := $(COBJ) $(CXXOBJ) 
#$(BINDIR)/Dic.o

all :
$(BINDIR)/%.o : src/%.cxx $(INCDIR)/%.h
	@echo $*.cxx
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c src/$*.cxx  -o $(BINDIR)/$*.o 

$(BINDIR)/%.o : src/%.c $(INCDIR)/%.h
	@echo $*.c
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c src/$*.c -o $(BINDIR)/$*.o


$(BINDIR)/Dic.o: $(HDRS)
	$(ROOTSYS)/bin/rootcint  -f Dic.C -c -I$(INCDIR) $(HDRS)
	$(CXX)  Dic.C -c -I$(INCDIR) -o $(BINDIR)/Dic.o $(CXXFLAGS)



#////////////////////////////////////////////////////////////////////
#////////////////////////////////////////////////////////////////////
#////////////////////////////////////////////////////////////////////



$(LIBDIR)/libAlphaAnalysis.so: $(OBJS)
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(LD) $(OBJS) $(SOFLAGS) $(CFLAGS) $(ROOTLIBS) -o $@


#////////////////////////////////////////////////////////////////////
#////////////////////////////////////////////////////////////////////
#////////////////////////////////////////////////////////////////////




#////////////////////////////////////////////////////////////////////
#////////////////////////////////////////////////////////////////////
#////////////////////////////////////////////////////////////////////



#////////////////////////////////////////////////////////////////////
#////////////////////////////////////////////////////////////////////
#////////////////////////////////////////////////////////////////////

clean:
	rm -r $(BINDIR)
	rm -f libAlphaAnalysis.so
	rm -f include/LibVersion.h
