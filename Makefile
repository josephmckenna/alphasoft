#
# Master Makefile for the ALPHA-g analyzer
#

#If rootana is within this folder, build it...
ifeq (${ROOTANASYS},${AGRELEASE}/rootana)
DEPS = buildrootana
endif

LIBS = libagana.so libAGTPC.so libaged.so 
BIN = agana

all:: $(DEPS) $(LIBS) $(BIN) reco

libAGTPC.so: $(DEPS)
	make -C recolib $(MFLAGS)

libaged.so: $(DEPS)
	make -C aged $(MFLAGS)

libagana.so: $(DEPS)
	make -C analib $(MFLAGS)

agana: | $(LIBS)
	cd ana/ && $(MAKE)

reco: $(LIBS)
	cd reco/ && $(MAKE)

buildrootana:
	make -C rootana obj/manalyzer_main.o lib/librootana.a

cleanrootana:
	ifeq (${ROOTANASYS},${AGRELEASE}/rootana)
	cd rootana/ && $(MAKE) clean
	endif

html/index.html:
	-mkdir html
	-make -k dox
	touch html/index.html

dox:
	doxygen Doxyfile

clean::
	$(cleanrootana)
	cd recolib/ && $(MAKE) clean
	cd analib/ && $(MAKE) clean
	cd aged/ && $(MAKE) clean
	cd ana/ && $(MAKE) clean
	cd reco/ && $(MAKE) clean
