# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: June 2018


CXX=$(shell root-config --cxx)
ROO=rootcint

#CXXFLAGS=-c -g -O2 -Wall -Wuninitialized -fPIC $(shell root-config --cflags)
#CXXFLAGS=-c -pg -Wuninitialized -fPIC $(shell root-config --cflags)
CXXFLAGS=-c -O3 -Wall -Wuninitialized -fPIC $(shell root-config --cflags)
#CXXFLAGS=-c -pg -g -O0 -Wall -Wuninitialized -fPIC $(shell root-config --cflags)

LDFLAGS=$(shell root-config --ldflags)
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

#SOFLAGS=-g -O2 -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags)
#SOFLAGS=-pg -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags)
SOFLAGS=-O3 -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags)
#SOFLAGS=-g -O0 -pg -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags)

INC=TFitHelix.hh TFitLine.hh TTrack.hh TSpacePoint.hh RecoLinkDef.hh
DICT=agtpc.cc
SRC=TFitHelix.cc TFitLine.cc TTrack.cc TracksFinder.cc TSpacePoint.cc LookUpTable.cc tinyspline.cxx $(DICT)

all: libAGTPC.so


libAGTPC.so: $(SRC)
	$(CXX) $(SOFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $^

$(DICT): $(INC)
	$(ROO) -f $@ $(CXXFLAGS) -p $^

clean:
	rm -f libAGTPC.so agtpc.cc agtpc_rdict.pcm
