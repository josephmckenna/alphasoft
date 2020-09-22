# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: June 2018


ROO=rootcint

INCS += -Iinclude
INCS += -I../ana/include
INCS += -I../analib/include
INCS += -I../agana
INCS += -I../simulation/geant4/include
INCS += -I$(ROOTANASYS)/include

LDFLAGS=$(shell root-config --ldflags)
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

INC=$(wildcard include/*.hh) RecoLinkDef.hh
DICT=agtpc.cc
SRC=$(wildcard src/*.cxx) $(DICT)
SRC:= $(patsubst src/%.cxx,obj/%.o,$(SRC))

all: CXXFLAGS=-g -O3 -Wall -Wuninitialized -fPIC $(shell root-config --cflags)
all: linkdirs libAGTPC.so

debug: CXXFLAGS += -O0 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
debug: linkdirs libAGTPC.so

native: CXXFLAGS += -O3 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags)  -march=native
native: linkdirs libAGTPC.so

O2: CXXFLAGS += -O2 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
O2: linkdirs libAGTPC.so

linkdirs:
	mkdir -p obj

libAGTPC.so: $(SRC)
	$(CXX) -shared $(CXXFLAGS) $(INCS) -o $@ $(LDFLAGS) $(LDLIBS) $^

$(DICT): $(INC)
	$(ROO) -f $@ -c $(CXXFLAGS) $(INCS) -p $^

obj/%.o: src/%.cxx include/%.hh
	$(CXX) -c $(CXXFLAGS) -o $@ $(INCS)  -c $<

clean:
	rm -f obj/*.o libAGTPC.so agtpc.cc agtpc_rdict.pcm
