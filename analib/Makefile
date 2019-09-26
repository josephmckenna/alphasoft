# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: August 2018

OBJDIR=obj
INCDIR=include
SRCDIR=src

#CXX=$(shell root-config --cxx)
ROO=rootcint


INCS+=-I../recolib/include -I$(ROOTANASYS)/include -I../ana/include -I../aged -I$(INCDIR) -I.

LDFLAGS=$(shell root-config --ldflags) -fPIC
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

INC= $(wildcard $(INCDIR)/*.h)
INC+=$(wildcard $(INCDIR)/*.hh)
INC+=$(wildcard RootUtils/*.h)
INC+=LibLinkDef.hh
DICT=analib.cc
SRC=$(wildcard $(SRCDIR)/*.cxx) $(wildcard RootUtils/*.cxx) $(DICT)
SRC:= $(patsubst $(SRCDIR)/%.cxx,$(OBJDIR)/%.o,$(SRC))
SRC:= $(patsubst RootUtils/%.cxx,$(OBJDIR)/RootUtils/%.o,$(SRC))

all: CXXFLAGS=-O3 -g -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
all: linkdirs libagana.so

debug: CXXFLAGS += -O0 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
debug: linkdirs libagana.so

native: CXXFLAGS += -O3 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags)  -march=native
native: linkdirs libagana.so

O2: CXXFLAGS += -O2 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
O2: linkdirs libagana.so

linkdirs:
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/RootUtils

libagana.so: $(SRC)
	$(CXX) -shared $(CXXFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $(INCS) $^

$(DICT): $(INC)
	$(ROO) -f $@ -c $(CXXFLAGS) $(INCS) -p $^

$(OBJDIR)/RootUtils/%.o: RootUtils/%.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $(INCS) -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $(INCS) -c $<

clean:
	rm -f $(OBJDIR)/RootUtils/*.o $(OBJDIR)/*.o libagana.so analib.cc analib_rdict.pcm
