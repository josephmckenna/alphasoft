# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: August 2018

OBJDIR=obj
INCDIR=include
SRCDIR=src

#CXX=$(shell root-config --cxx)
ROO=rootcint

INCS+=-I../recolib/include -I../analib/include -I$(ROOTANASYS)/include -I../ana/include -I../aged -I../a2lib/include -I$(INCDIR) -I.

LDFLAGS=$(shell root-config --ldflags) -fPIC
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum -L../analib -lagana

INC= $(wildcard $(INCDIR)/*.h)
INC+=$(wildcard $(INCDIR)/*.hh)
INC+=LibLinkDef.hh

DICT=rootUtilslib.cc

SRC=$(wildcard $(SRCDIR)/*.cxx) $(DICT)
SRC:= $(patsubst $(SRCDIR)/%.cxx,$(OBJDIR)/%.o,$(SRC))

#-CFLAGS=-c -O3 -g -Wall -Wuninitialized -fPIC  -I../recolib/include -I$(ROOTANASYS)/include -I../ana/include -I../aged -I$(INCDIR) -I.


all: CXXFLAGS=-O3 -g -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
all: CFLAGS=-O3 -g -Wall -Wuninitialized -fPIC 
all: linkdirs librootUtils.so

debug: CXXFLAGS += -O0 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
debug: CFLAGS += -O0 -g  -Wall -Wuninitialized -fPIC 
debug: linkdirs librootUtils.so

native: CXXFLAGS += -O3 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags)  -march=native
native: CFLAGS += -O3 -g  -Wall -Wuninitialized -fPIC   -march=native
native: linkdirs librootUtils.so

O2: CXXFLAGS += -O2 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
O2: CFLAGS += -O2 -g  -Wall -Wuninitialized -fPIC
O2: linkdirs librootUtils.so

linkdirs:
	mkdir -p $(OBJDIR)

librootUtils.so: $(SRC)
	$(CXX) -shared $(CXXFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $(INCS) $^

$(DICT): $(INC)
	$(ROO) -f $@ -c $(CXXFLAGS) $(INCS) -p $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $(INCS) -c $<


clean:
	rm -f $(OBJDIR)/*.o librootUtils.so rootUtilslib.cc rootUtilslib_rdict.pcm
