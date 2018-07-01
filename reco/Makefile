# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: June 2018


CXX=$(shell root-config --cxx)
ROO=rootcint

CXXFLAGS=-c -g -O2 -Wall -Wuninitialized -fPIC $(shell root-config --cflags)

LDFLAGS=$(shell root-config --ldflags)
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

SOFLAGS=-g -O2 -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags)

INC=TStoreEvent.hh TStoreLine.hh TFitLine.hh TTrack.hh TSpacePoint.hh LinkDef.hh
DICT=agtpc.cc
SRC=TStoreEvent.cc TStoreLine.cc TFitLine.cc TTrack.cc TracksFinder.cc TSpacePoint.cc LookUpTable.cc $(DICT)

all: libAGTPC.so


libAGTPC.so: $(SRC)
	$(CXX) $(SOFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $^
	mv libAGTPC.so agtpc_rdict.pcm ../ana

$(DICT): $(INC)
	$(ROO) -f $@ $(CXXFLAGS) -p $^

clean:
	rm -f ../ana/libAGTPC.so agtpc.cc ../ana/agtpc_rdict.pcm
