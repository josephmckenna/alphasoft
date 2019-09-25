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

ALL= $(DEPS) $(LIBS) $(BIN) $(A2LIBS) $(A2)
all:: $(ALL) FIN

debug: MFLAGS += debug
debug: $(A2) FIN

O2: MFLAGS += O2
O2: $(A2) FIN

native: MFLAGS += native
native: $(A2) FIN


FIN: $(ALL)
	@echo -e "\033[32mSuccess!\033[m"

libAGTPC.so: $(DEPS)
	make -C recolib $(MFLAGS)

libaged.so: $(DEPS)
	make -C aged $(MFLAGS)

libagana.so: $(DEPS)
	make -C analib $(MFLAGS)

agana: | $(LIBS)
	cd ana/ && $(MAKE) $(MFLAGS)

alpha2libs: $(DEPS)
	make -C a2lib $(MFLAGS)

reco: $(LIBS)
	cd reco/ && $(MAKE) $(MFLAGS)

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
