CXXFLAGS += -O3 -g -Wall -Wuninitialized -Wextra -fPIC $(shell root-config --cflags)
INC += -Iinclude
INC += -I$(ROOTANASYS)/include
INC += -I$(AGRELEASE)/ana/include
INC += -I$(AGRELEASE)/recolib/include
INC += -I$(AGRELEASE)/analib/include
INCDIR=$(AGRELEASE)/simulation/geant4/include
INC += -I$(INCDIR)
INC += -I$(AGRELEASE)/simulation/garfieldpp/include

LDLIBS += -L$(AGRELEASE)/recolib -lAGTPC
LDLIBS += -L$(AGRELEASE)/analib -lagana
LDLIBS += $(shell root-config --glibs)
LDLIBS += -lMinuit -lMathMore -lGeom -lGL -lSpectrum 
LDLIBS += -lz -lXMLParser -lXMLIO -lXMLParser -lXMLIO

G4MAIN=obj/g4ana.o
AGMAIN=obj/agreco.o
DICT=src/G4outDict.cc

SRC=$(wildcard src/*.cc) $(DICT)
OBJ:= $(patsubst src/%.cc,obj/%.o,$(SRC))
OBJ:= $(filter-out $(G4MAIN) $(AGMAIN), $(OBJ))

all: objdir g4ana.exe agreco.exe

objdir:
	mkdir -p obj

g4ana.exe: $(OBJ) $(G4MAIN)
	$(CXX) -o $@ $(CXXFLAGS) $(INC)  $^  $(LDLIBS)

$(G4MAIN): g4ana.cc
	$(CXX) -o $@ $(CXXFLAGS) $(INC) $(LDLIBS) -c $<

obj/%.o: src/%.cc
	$(CXX) -o $@ $(CXXFLAGS) $(INC) $(LDLIBS) -c $<

$(DICT): $(INCDIR)/TMChit.hh $(INCDIR)/TWaveform.hh $(INCDIR)/LinkDef.hh
	rootcling -f $@ -s ./G4outDict_rdict.pcm $(CXXFLAGS)  $^

agreco.exe: $(OBJ) $(AGMAIN)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $(INC) $^ $(LDLIBS) 

$(AGMAIN): agreco.cc 
	$(CXX) -o $@ $(CXXFLAGS) $(INC) -c $<

clean:
	rm -fr obj *.exe *.pcm $(DICT) src/G4outDict_rdict.pcm
