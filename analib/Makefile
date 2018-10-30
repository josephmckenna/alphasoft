# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: August 2018

OBJDIR=obj
INCDIR=include
SRCDIR=src

#CXX=$(shell root-config --cxx)
ROO=rootcint

CXXFLAGS=-c -O3 -g -Wall -Wuninitialized -fPIC $(shell root-config --cflags) -I../reco -I$(ROOTANASYS)/include -I$(INCDIR) -I.


LDFLAGS=$(shell root-config --ldflags) -fPIC
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

SOFLAGS=-O3 -g -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags) -I../reco -I$(ROOTANASYS)/include  -I$(INCDIR) -I.

INC=$(wildcard $(INCDIR)/*.h) $(wildcard $(INCDIR)/*.hh) $(wildcard RootUtils/*.h) LibLinkDef.hh
DICT=analib.cc
SRC=$(wildcard $(SRCDIR)/*.cxx) $(wildcard RootUtils/*.cxx) $(DICT)
SRC:= $(patsubst $(SRCDIR)/%.cxx,$(OBJDIR)/%.o,$(SRC))
SRC:= $(patsubst RootUtils/%.cxx,$(OBJDIR)/RootUtils/%.o,$(SRC))
all: linkdirs libagana.so

linkdirs:
	mkdir -p $(OBJDIR)

libagana.so: $(SRC)
	$(CXX) $(SOFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $^

$(DICT): $(INC)
	$(ROO) -f $@ $(CXXFLAGS) -p $^

$(OBJDIR)/RootUtils/%.o: RootUtils/%.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJDIR)/RootUtils/*.o $(OBJDIR)/*.o libagana.so analib.cc analib_rdict.pcm
