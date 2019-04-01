# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: August 2018

OBJDIR=obj
INCDIR=include
SRCDIR=src

#CXX=$(shell root-config --cxx)
ROO=rootcint

CXXFLAGS=-c -O3 -g -Wall -Wuninitialized -fPIC $(shell root-config --cflags) -I../reco/include -I$(ROOTANASYS)/include -I../ana/include -I../aged -I$(INCDIR) -I.
CFLAGS=-c -O3 -g -Wall -Wuninitialized -fPIC  -I../reco/include -I$(ROOTANASYS)/include -I../ana/include -I../aged -I$(INCDIR) -I.


LDFLAGS=$(shell root-config --ldflags) -fPIC
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

SOFLAGS=-O3 -g -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags) -I../reco/include -I$(ROOTANASYS)/include -I../ana/include -I../aged -I$(INCDIR) -I.

INC=$(wildcard $(INCDIR)/*.h) $(wildcard $(INCDIR)/*.hh) $(wildcard RootUtils/*.h) LibLinkDef.hh
INC:= $(filter-out $(INCDIR)/sqlite3.h,$(INC))

DICT=a2lib.cc

SRC=$(wildcard $(SRCDIR)/*.cxx) $(wildcard RootUtils/*.cxx) $(DICT)
SRC:= $(patsubst $(SRCDIR)/%.cxx,$(OBJDIR)/%.o,$(SRC))
SRC:= $(patsubst RootUtils/%.cxx,$(OBJDIR)/RootUtils/%.o,$(SRC))


CSRC    := $(wildcard src/*.c)
CSRC    := $(patsubst src/%.c,$(OBJDIR)/%.o,$(CSRC))

all: linkdirs libalpha2.so

linkdirs:
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/RootUtils

libalpha2.so: $(SRC) $(CSRC)
	$(CXX) $(SOFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $^

$(DICT): $(INC)
	$(ROO) -f $@ $(CXXFLAGS) -p $^

$(OBJDIR)/RootUtils/%.o: RootUtils/%.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -o $@ $(CFLAGS) -c $<

clean:
	rm -f $(OBJDIR)/RootUtils/*.o $(OBJDIR)/*.o libalpha2.so a2lib.cc a2lib_rdict.pcm main.db
