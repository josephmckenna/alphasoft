# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: June 2018

CXXFLAGS=-O3 -Wall -Wuninitialized -fPIC $(shell root-config --cflags)
CXXFLAGS += -I../reco/include
CXXFLAGS += -I../ana/include
CXXFLAGS += -I../analib/include

LDFLAGS=$(shell root-config --ldflags)
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum 
LDLIBS += -L../reco -L../analib -lagana -lAGTPC 
LDLIBS += -lz -lXMLParser -lXMLIO -lXMLParser -lXMLIO

all: objdir ../agreco.exe

objdir:
	mkdir -p obj

../agreco.exe: obj/agreco.o
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

obj/agreco.o: ./agreco.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

clean:
	rm -f obj/*.o ../agreco.exe
