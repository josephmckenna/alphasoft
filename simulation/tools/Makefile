CXXFLAGS += -O3 -g -Wall -Wuninitialized -Wextra -fPIC $(shell root-config --cflags)
CXXFLAGS += -Iinclude
CXXFLAGS += -I$(AGRELEASE)/ana/include
CXXFLAGS += -I$(AGRELEASE)/reco/include
CXXFLAGS += -I$(AGRELEASE)/analib/include
INCDIR=$(AGRELEASE)/simulation/geant4/include
CXXFLAGS += -I$(INCDIR)
CXXFLAGS += -I$(AGRELEASE)/simulation/garfieldpp/include

LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum
LDLIBS += -L$(AGRELEASE)/reco -lAGTPC

all: nominalDrift

nominalDrift: nominalDrift.cc
	$(CXX) -o $@ $(LDLIBS) $(CXXFLAGS) $^

clean:
	rm -fr nominalDrift
