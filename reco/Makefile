CXXFLAGS += -O3 -g -Wall -Wuninitialized -Wextra -fPIC $(shell root-config --cflags)
INC += -Iinclude
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

MAIN=obj/g4ana.o
DICT=src/G4outDict.cc

SRC=$(wildcard src/*.cc) $(DICT)
OBJ:= $(patsubst src/%.cc,obj/%.o,$(SRC))

all: objdir g4ana.exe agreco.exe

objdir:
	mkdir -p obj

g4ana.exe: $(OBJ) $(MAIN)
	$(CXX) -o $@ $(CXXFLAGS) $(INC)  $^  $(LDLIBS)
$(MAIN): g4ana.cc
	$(CXX) -o $@ $(CXXFLAGS) $(INC) $(LDLIBS) -c $<

obj/%.o: src/%.cc
	$(CXX) -o $@ $(CXXFLAGS) $(INC) $(LDLIBS) -c $<

$(DICT): $(INCDIR)/TMChit.hh $(INCDIR)/TWaveform.hh $(INCDIR)/LinkDef.hh
	rootcling -f $@ -s ./G4outDict_rdict.pcm $(CXXFLAGS)  $^

agreco.exe: obj/agreco.o obj/Reco.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) $(INC) $(LDLIBS)

obj/agreco.o: agreco.cc 
	$(CXX) -o $@ $(CXXFLAGS) $(INC) -c $<

clean:
	rm -fr obj *.exe *.pcm $(DICT) src/G4outDict_rdict.pcm
