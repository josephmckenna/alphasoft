#
# Master Makefile for the ALPHA-g analyzer
#

#If rootana is within this folder, build it...
ifeq (${ROOTANASYS},${AGRELEASE}/rootana)
DEPS = buildrootana
endif

LIBS = libagana.so libAGTPC.so libaged.so
BIN = agana
A2 = alpha2libs alpha2

all:: $(DEPS) $(LIBS) $(BIN) $(A2)

libAGTPC.so: $(DEPS)
	make -C reco $(MFLAGS)

libaged.so: $(DEPS)
	make -C aged $(MFLAGS)

libagana.so: $(DEPS)
	make -C analib $(MFLAGS)

agana: | $(LIBS)
	cd ana/ && $(MAKE)

alpha2libs: $(DEPS)
	$(shell bash a2lib/build_db.sh)

alpha2: $(DEPS) alpha2libs
	make -C alpha2

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
	cd reco/ && $(MAKE) clean
	cd analib/ && $(MAKE) clean
	cd aged/ && $(MAKE) clean
	cd ana/ && $(MAKE) clean
