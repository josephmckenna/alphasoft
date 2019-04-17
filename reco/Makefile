# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: June 2018


ROO=rootcint

CXXFLAGS=-c -g -O3 -Wall -Wuninitialized -fPIC $(shell root-config --cflags) -Iinclude
#CXXFLAGS=-c -g -O0 -Wall -Wuninitialized -fPIC $(shell root-config --cflags) -Iinclude
CXXFLAGS += -I../ana/include
CXXFLAGS +=  -I../analib/include

LDFLAGS=$(shell root-config --ldflags)
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

SOFLAGS=-O3 -g -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags) -Iinclude
#SOFLAGS=-O0 -g -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags) -Iinclude
SOFLAGS += -I../ana/include
SOFLAGS +=  -I../analib/include

INC=$(wildcard include/*.hh) RecoLinkDef.hh
DICT=agtpc.cc
SRC=$(wildcard src/*.cxx) $(DICT)
SRC:= $(patsubst src/%.cxx,obj/%.o,$(SRC))
all: linkdirs libAGTPC.so

linkdirs:
	mkdir -p obj

libAGTPC.so: $(SRC)
	$(CXX) $(SOFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $^

$(DICT): $(INC)
	$(ROO) -f $@ $(CXXFLAGS) -p $^

obj/%.o: src/%.cxx include/%.hh
	$(CXX) -o $@ $(CXXFLAGS) -c $<

clean:
	rm -f obj/*.o libAGTPC.so agtpc.cc agtpc_rdict.pcm
