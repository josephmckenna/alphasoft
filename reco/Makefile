#INCDIR = ../garfieldpp
#LIBDIR = $(INCDIR)
#GARFIDIR = $(GARFIELD_HOME)/Include
#GARFDIR = $(GARFIELD_HOME)/Library

CXXFLAGS = -Wall -Wuninitialized

# optional ZLIB library

CXXFLAGS += -DHAVE_ZLIB

# ROOT libraries

ifdef ROOTSYS
ROOTGLIBS = -L$(ROOTSYS)/lib $(shell $(ROOTSYS)/bin/root-config --glibs) -lXMLParser -lThread
HAVE_RHTTP = $(shell $(ROOTSYS)/bin/root-config --has-http)
ifeq ($(HAVE_RHTTP),yes)
ROOTGLIBS += -lRHTTP
endif
CXXFLAGS += -DHAVE_ROOT $(shell $(ROOTSYS)/bin/root-config --cflags)
endif

ifdef MIDASSYS
MIDASLIBS = $(MIDASSYS)/linux/lib/libmidas.a -lutil -lrt
CXXFLAGS += -DHAVE_MIDAS -DOS_LINUX -Dextname -I$(MIDASSYS)/include

UNAME=$(shell uname)
RTLIB=
ifeq ($(UNAME),Darwin)
#CXXFLAGS += -DOS_LINUX -DOS_DARWIN
MIDASLIBS = $(MIDASSYS)/darwin/lib/libmidas.a
#RPATH=
else
RTLIB=-lrt
endif

endif

#ifdef ROOTSYS
#CXXFLAGS += -DHAVE_LIBNETDIRECTORY -IlibNetDirectory
#endif


# Check for ROOTANASYS env variable.
ifdef ROOTANASYS
ROOTANAINC = -I$(ROOTANASYS)/include
ROOTANALIBS = $(ROOTANASYS)/lib/librootana.a
else
ROOTANAINC = -I../include
ROOTANALIBS = ../lib/librootana.a
endif


# Compiler flags

CXXFLAGS += -Wall -Wextra -Wno-long-long -fno-common -std=c++11
#CXXFLAGS += -I$(INCDIR)
#CXXFLAGS += -I${ANALYSIS_TPC}/include 
#CXXFLAGS += -I$(GARFIDIR)
CXXFLAGS += -I$(ROOTANASYS)/include

# Debug flags
CXXFLAGS += -g -pg -O2

LDFLAGS += $(ROOTGLIBS) -lMathMore -lMinuit -lGeom -lRGL -lgfortran -lm
LDFLAGS += -g

OBJS = TPCBase.o Signals.o SpacePoints.o RootPlotter.o ChamberGeo.o StraightTrack.o Alpha16.o Unpack.o TrackViewer.o TLookUpTable.o TSpacePoint.o TFitLine.o TLookUpTableDict.o TFitLineDict.o TSpacePointDict.o #GarfieldAlpha16EVB.o

analyzer.exe: analyzer.o $(OBJS) AGAnalyze.o
	$(CXX) -o $@ $(CXXFLAGS) $(ROOTANAINC) $^ $(ROOTANALIBS) $(MIDASLIBS) $(ROOTGLIBS) -lm -lz -lpthread $(RPATH) -lssl $(RTLIB) -lutil $(LDFLAGS)

analyzer.o: analyzer.cxx
	$(CXX) -o $@ -c $(CXXFLAGS) $^

test: test.cc $(OBJS)
	$(CXX) -c $(CXXFLAGS) -I$(GARFIDIR) test.cc -o test.o
	$(CXX) -o test test.o $(OBJS) $(LDFLAGS)

acceptance: acceptance.cc StraightTrack.o
	$(CXX) -c $(CXXFLAGS) -I$(GARFIDIR) acceptance.cc -o acceptance.o
	$(CXX) -o acceptance acceptance.o StraightTrack.o ../garfieldpp/TPCBase.o $(LDFLAGS)

cgtest: cgtest.cc ChamberGeo.o
	$(CXX) -c $(CXXFLAGS) $(ROOTGLIBS) -I$(GARFIDIR) cgtest.cc
	$(CXX) -o cgtest cgtest.o ../garfieldpp/TPCBase.o ChamberGeo.o $(LDFLAGS)
	rm -f cgtest.o

GarfieldAlphaPacket.o: GarfieldAlphaPacket.cc GarfieldAlphaPacket.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

GarfieldAlpha16EVB.o: GarfieldAlpha16EVB.cc GarfieldAlpha16EVB.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

SpacePoints.o: SpacePoints.cc SpacePoints.hh
	$(CXX) -o $@ -c $< $(CXXFLAGS)

RootPlotter.o: RootPlotter.cc RootPlotter.hh
	$(CXX) -c $< $(CXXFLAGS)  -o $@

ChamberGeo.o: ChamberGeo.cc ChamberGeo.hh
	$(CXX) -c $< $(CXXFLAGS)  -o $@

TrackViewer.o: TrackViewer.cc TrackViewer.hh
	$(CXX) -c $< $(CXXFLAGS)  -o $@

Unpack.o: Unpack.cxx
	$(CXX) $(CXXFLAGS) $(ROOTANAINC) -c $<

Alpha16.o: Alpha16.cxx
	$(CXX) $(CXXFLAGS) $(ROOTANAINC) -c $<

#TLookupTable.o: %.o: %.cc
#	$(CXX) $(CXXFLAGS) $(ROOTANAINC) -c $<

%.o: %.cc %.hh
	$(CXX) -c $< $(CXXFLAGS) -o $@

%Dict.cc: %.hh
	rootcint -f $@ -c -p $^

clean::
	-rm -f *.o test acceptance analyzer.exe
