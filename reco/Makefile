CXXFLAGS += -O3 -g -Wall -Wuninitialized -Wextra -fPIC $(shell root-config --cflags)
#CXXFLAGS += -O0 -g -Wall -Wuninitialized -Wextra -fPIC $(shell root-config --cflags)
CXXFLAGS += -Iinclude
CXXFLAGS += -I$(AGRELEASE)/ana/include
CXXFLAGS += -I$(AGRELEASE)/recolib/include
CXXFLAGS += -I$(AGRELEASE)/analib/include
INCDIR=$(AGRELEASE)/simulation/geant4/include
CXXFLAGS += -I$(INCDIR)
CXXFLAGS += -I$(AGRELEASE)/simulation/garfieldpp/include

LDLIBS=$(shell root-config --glibs) -lMinuit -lMathMore -lGeom -lGL -lSpectrum
LDLIBS += -L$(AGRELEASE)/recolib -lAGTPC
LDLIBS += -L../analib -lagana
LDLIBS += -lz -lXMLParser -lXMLIO -lXMLParser -lXMLIO

MAIN=obj/g4ana.o
ANASET=obj/AnaSettings.o
DICT=src/G4outDict.cc

SRC=$(wildcard src/*.cc) $(DICT)
OBJ:= $(patsubst src/%.cc,obj/%.o,$(SRC))

all: objdir g4ana.exe agreco.exe

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

agreco.exe: obj/agreco.o obj/Reco.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

obj/agreco.o: agreco.cc 
	$(CXX) -o $@ $(CXXFLAGS) -c $<

clean:
	rm -fr obj *.exe *.pcm $(DICT) src/G4outDict_rdict.pcm
