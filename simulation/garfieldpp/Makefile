INCDIR = $(GARFIELD_HOME)/include/Garfield
LIBDIR = $(GARFIELD_HOME)/lib

CFLAGS += -O3 -g -Wall -Wextra -Wno-long-long -Wuninitialized `root-config --cflags` -fno-common -c -I$(INCDIR) -I./include

LDFLAGS = `root-config --glibs` -lGeom -lgfortran -lm -L$(LIBDIR) -lGarfield -g

SOURCES := $(wildcard src/*.cc)
OBJECTS := $(patsubst src/%.cc,obj/%.o,$(SOURCES))
BINARIES := DriftGas.exe PlotMedium.exe ChamberField.exe CalculateGain.exe ChamberDrift.exe ElectronAvalanche.exe TrackElectronAvalanche.exe WireSag.exe polar.exe


all:: odir
all:: $(OBJECTS)
all:: $(BINARIES)
all:: ddir

odir:
	mkdir -p obj

$(BINARIES): %.exe: obj/%.o
	$(CXX) -o $@ $< obj/MagneticFieldMap.o obj/ComponentBmap.o obj/TPCBase.o obj/TPC.o $(LDFLAGS)

$(OBJECTS): obj/%.o: src/%.cc
	$(CXX) -o $@ $(CFLAGS) -c $<

clean:
	rm -f $(BINARIES)
	rm -fr ./obj

ddir:
	rm -fr ./obj
