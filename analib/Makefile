# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: August 2018


CXX=$(shell root-config --cxx)
ROO=rootcint

CXXFLAGS=-c -O3 -Wall -Wuninitialized -fPIC $(shell root-config --cflags) -I../reco -I.


LDFLAGS=$(shell root-config --ldflags)
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

SOFLAGS=-O3 -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags) -I../reco -I.

INC=TSeq_Event.h TStoreEvent.hh TStoreHelix.hh TStoreLine.hh Sequencer2.h TChrono_Event.h TChronoChannelName.h TSeq_Dump.h TSpill.h RootUtils.h LibLinkDef.hh
DICT=analib.cc
SRC=TSeq_Event.cxx TStoreEvent.cc TStoreHelix.cc TStoreLine.cc Sequencer2.cxx TChrono_Event.cxx TChronoChannelName.cxx TSeq_Dump.cxx  TSpill.cxx RootUtils.cxx $(DICT)

all: libagana.so


libagana.so: $(SRC)
	$(CXX) $(SOFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $^

$(DICT): $(INC)
	$(ROO) -f $@ $(CXXFLAGS) -p $^

clean:
	rm -f libagana.so analib.cc analib_rdict.pcm
