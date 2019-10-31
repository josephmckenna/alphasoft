# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: August 2018

OBJDIR=obj
INCDIR=include
SRCDIR=src

#CXX=$(shell root-config --cxx)
ROO=rootcint


INCS+=-I../recolib/include -I$(ROOTANASYS)/include -I../ana/include -I../aged -I$(INCDIR) -I.

LDFLAGS=$(shell root-config --ldflags) -fPIC
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum

INC= $(wildcard $(INCDIR)/*.h)
INC+=$(wildcard $(INCDIR)/*.hh)
INC+=$(wildcard RootUtils/*.h)
INC+=LibLinkDef.hh
INC:= $(filter-out $(INCDIR)/sqlite3.h,$(INC))

DICT=a2lib.cc

SRC=$(wildcard $(SRCDIR)/*.cxx) $(wildcard RootUtils/*.cxx) $(DICT)
SRC:= $(patsubst $(SRCDIR)/%.cxx,$(OBJDIR)/%.o,$(SRC))
SRC:= $(patsubst RootUtils/%.cxx,$(OBJDIR)/RootUtils/%.o,$(SRC))
CSRC    := $(wildcard src/*.c)
CSRC    := $(patsubst src/%.c,$(OBJDIR)/%.o,$(CSRC))

#-CFLAGS=-c -O3 -g -Wall -Wuninitialized -fPIC  -I../recolib/include -I$(ROOTANASYS)/include -I../ana/include -I../aged -I$(INCDIR) -I.


all: CXXFLAGS=-O3 -g -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
all: CFLAGS=-O3 -g -Wall -Wuninitialized -fPIC 
all: linkdirs libalpha2.so

debug: CXXFLAGS += -O0 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
debug: CFLAGS += -O0 -g  -Wall -Wuninitialized -fPIC 
debug: linkdirs libalpha2.so

native: CXXFLAGS += -O3 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags)  -march=native
native: CFLAGS += -O3 -g  -Wall -Wuninitialized -fPIC   -march=native
native: linkdirs libalpha2.so

O2: CXXFLAGS += -O2 -g  -Wall -Wuninitialized -fPIC $(shell root-config --cflags) 
O2: CFLAGS += -O2 -g  -Wall -Wuninitialized -fPIC
O2: linkdirs libalpha2.so

linkdirs:
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/RootUtils
	$(shell bash build_db.sh)

libalpha2.so: $(SRC) $(CSRC)
	$(CXX) -shared $(CXXFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $(INCS) $^

$(DICT): $(INC)
	$(ROO) -f $@ -c $(CXXFLAGS) $(INCS) -p $^

$(OBJDIR)/RootUtils/%.o: RootUtils/%.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $(INCS) -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $(INCS) -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -o $@ $(CFLAGS) $(INCS) -c $<

clean:
	rm -f $(OBJDIR)/RootUtils/*.o $(OBJDIR)/*.o libalpha2.so a2lib.cc a2lib_rdict.pcm main.db
