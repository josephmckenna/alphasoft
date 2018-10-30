# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: June 2018


#CXX=$(shell root-config --cxx)
ROO=rootcint

#CXXFLAGS=-c -g -O2 -Wall -Wuninitialized -fPIC $(shell root-config --cflags)
#CXXFLAGS=-c -pg -Wuninitialized -fPIC $(shell root-config --cflags)
CXXFLAGS=-c -g -O3 -Wall -Wuninitialized -fPIC $(shell root-config --cflags) -Iinclude
#CXXFLAGS=-c -pg -g -O0 -Wall -Wuninitialized -fPIC $(shell root-config --cflags)

LDFLAGS=$(shell root-config --ldflags)
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

#SOFLAGS=-g -O2 -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags)
#SOFLAGS=-pg -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags)
SOFLAGS=-O3 -g -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags) -Iinclude
#SOFLAGS=-g -O0 -pg -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags)

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

obj/%.o: src/%.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

clean:
	rm -f obj/*.o libAGTPC.so agtpc.cc agtpc_rdict.pcm
