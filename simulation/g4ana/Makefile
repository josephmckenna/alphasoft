CXXFLAGS += -O0 -g -Wall -Wuninitialized -Wextra -fPIC $(shell root-config --cflags)
CXXFLAGS += -Iinclude
CXXFLAGS += -I$(AGRELEASE)/ana/include
CXXFLAGS += -I$(AGRELEASE)/reco/include
CXXFLAGS += -I$(AGRELEASE)/analib/include
INCDIR=$(AGRELEASE)/simulation/geant4/include
CXXFLAGS += -I$(INCDIR)
CXXFLAGS += -I$(AGRELEASE)/simulation/garfieldpp/include

LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum
LDLIBS += -L$(AGRELEASE)/reco -lAGTPC

MAIN=obj/g4ana.o
ANASET=obj/AnaSettings.o
DICT=src/G4outDict.cc

SRC=$(wildcard src/*.cc) $(DICT)
OBJ:= $(patsubst src/%.cc,obj/%.o,$(SRC))

all: objdir g4ana.exe

objdir:
	mkdir -p obj

g4ana.exe: $(OBJ) $(ANASET) $(MAIN)
	$(CXX) -o $@ $(LDLIBS) $^

$(MAIN): g4ana.cc
	$(CXX) -o $@ $(CXXFLAGS) -c $<

obj/%.o: src/%.cc
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(DICT): $(INCDIR)/TMChit.hh $(INCDIR)/TWaveform.hh $(INCDIR)/LinkDef.hh
	rootcling -f $@ -s ./G4outDict_rdict.pcm $(CXXFLAGS) $^

$(ANASET): $(AGRELEASE)/analib/src/AnaSettings.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

clean:
	rm -fr obj *.exe *.pcm $(DICT) src/G4outDict_rdict.pcm
