#
# Example Makefile for ROOTANA-based projects
#

CXXFLAGS = -g -O2 -Wall -Wuninitialized

# required ZLIB library

CXXFLAGS += -DHAVE_LIBZ

# required ROOTANA library

ifndef ROOTANASYS
ROOTANASYS=..
endif

CXXFLAGS += -I$(ROOTANASYS)/include
LIBS += -L$(ROOTANASYS)/lib -lrootana

# ROOT libraries

ifdef ROOTSYS

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTFEATURES := $(shell root-config --features)
ROOTGLIBS    := $(shell root-config --glibs) -lThread
HAVE_ROOT_HTTP := $(findstring http,$(ROOTFEATURES))
HAVE_ROOT_XML  := $(findstring xml,$(ROOTFEATURES))

CXXFLAGS  += -DHAVE_ROOT $(ROOTCFLAGS)

ifdef HAVE_ROOT_XML
CXXFLAGS  += -DHAVE_ROOT_XML
ROOTGLIBS += -lXMLParser
endif

ifdef HAVE_ROOT_HTTP
CXXFLAGS  += -DHAVE_ROOT_HTTP
ROOTGLIBS += -lRHTTP
endif

endif # ROOTSYS

# optional MIDAS library

ifdef MIDASSYS

LIBS += $(MIDASSYS)/linux/lib/libmidas.a -lutil -lrt
CXXFLAGS += -DHAVE_MIDAS -DOS_LINUX -Dextname -I$(MIDASSYS)/include

UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
LIBS += $(MIDASSYS)/darwin/lib/libmidas.a
endif

endif # MIDASSYS

OBJS:=

OBJS += a16module.o

OBJS += TPCBase.o
OBJS += AGAnalyze.o
OBJS += Signals.o SpacePoints.o RootPlotter.o ChamberGeo.o
OBJS += StraightTrack.o
OBJS += Alpha16.o Unpack.o
OBJS += TrackViewer.o TLookUpTable.o TSpacePoint.o TFitLine.o
OBJS += TLookUpTableDict.o TFitLineDict.o TSpacePointDict.o
OBJS += settings.o

ROOTGLIBS += -lXMLIO
ROOTGLIBS += -lMathMore -lMinuit
ROOTGLIBS += -lRGL -lGeom

all: $(OBJS) agreco.exe

agreco.exe: $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lssl -lutil

%.o: %.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

%Dict.cc: %.hh
	rootcint -f $@ -c -p $^

dox:
	doxygen

clean::
	-rm -f *.o *.a
	-rm -f *.exe
	-rm -rf *.exe.dSYM

# end
