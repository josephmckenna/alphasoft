# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: June 2018


CXX=$(shell root-config --cxx)
ROO=rootcint

CXXFLAGS=-c -g -O2 -Wall -Wuninitialized -Wextra -fPIC $(shell root-config --cflags)

LDFLAGS=$(shell root-config --ldflags)
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

SOFLAGS=-g -O2 -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags)

all: libAGTPC.so


libAGTPC.so: TSpacePoint.cc LookUpTable.cc agtpc.cc
	$(CXX) $(SOFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $^

agtpc.cc: TSpacePoint.hh LinkDef.hh
	$(ROO) -f $@ $(CXXFLAGS) -p $^

clean:
	rm -f *.so agtpc.cc agtpc_rdict.pcm
