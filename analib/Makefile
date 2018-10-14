# MAKEFILE
# for ALPHA-g TPC analysis
# Author: A.Capra
# Date: August 2018


#CXX=$(shell root-config --cxx)
ROO=rootcint

CXXFLAGS=-c -O3 -g -Wall -Wuninitialized -fPIC $(shell root-config --cflags) -I../reco -I$(ROOTANASYS)/include -I.


LDFLAGS=$(shell root-config --ldflags)
LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum -L$(ROOTANASYS)/lib -lrootana

SOFLAGS=-O3 -g -Wall -Wuninitialized -Wextra -fPIC -shared $(shell root-config --cflags) -I../reco -I$(ROOTANASYS)/include  -I.

INC=TSeq_Event.h TStoreEvent.hh TStoreHelix.hh TStoreLine.hh Sequencer2.h Sequencer_Channels.h TChrono_Event.h TChronoChannelName.h TSeq_Dump.h TSpill.h $(wildcard RootUtils/*.h) TAGPlot.h LibLinkDef.hh
DICT=analib.cc
SRC=TSeq_Event.cxx TStoreEvent.cxx TStoreHelix.cxx TStoreLine.cxx Sequencer2.cxx TChrono_Event.cxx TChronoChannelName.cxx TSeq_Dump.cxx TSpill.cxx $(wildcard RootUtils/*.cxx) TAGPlot.cxx $(DICT)
SRC:= $(patsubst %.cxx,%.o,$(SRC))
all: libagana.so



libagana.so: $(SRC)
	$(CXX) $(SOFLAGS) -o $@ $(LDFLAGS) $(LDLIBS) $^

$(DICT): $(INC)
	$(ROO) -f $@ $(CXXFLAGS) -p $^
	
%.o: %.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

clean:
	rm -f RootUtils/*.o *.o libagana.so analib.cc analib_rdict.pcm
