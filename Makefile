#
# Master Makefile for the ALPHA-g analyzer
#

#If rootana is within this folder, build it...
ifeq (${ROOTANASYS},${AGRELEASE}/rootana)
DEPS = buildrootana
endif

LIBS = libagana.so libAGTPC.so libaged.so
A2LIBS = alpha2libs
BIN = agana alpha2 reco

all:: $(DEPS) $(LIBS) $(BIN) $(A2LIBS) $(A2)

libAGTPC.so: $(DEPS)
	make -C recolib $(MFLAGS)

libaged.so: $(DEPS)
	make -C aged $(MFLAGS)

libagana.so: $(DEPS)
	make -C analib $(MFLAGS)

agana: | $(LIBS)
	cd ana/ && $(MAKE)

alpha2libs: $(DEPS)
	make -C a2lib
reco: $(LIBS)
	cd reco/ && $(MAKE)

alpha2: $(A2LIBS)
	make -C alpha2 $(MFLAGS)

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
	cd a2lib/ && $(MAKE) clean
	cd alpha2/ && $(MAKE) clean
