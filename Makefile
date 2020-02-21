#
# Master Makefile for the ALPHA-g analyzer
#

#If rootana is within this folder, build it...
ifeq (${ROOTANASYS},${AGRELEASE}/rootana)
DEPS = buildrootana
endif

AGLIBS = libagana.so libAGTPC.so libaged.so
AGBIN  = agana reco

A2LIBS = libagana.so libAGTPC.so alpha2libs
A2BIN  = alpha2


AG= $(DEPS) $(AGLIBS) $(AGBIN)
#ALPHA 2 depends on some AG libs
A2= $(DEPS) $(A2LIBS) $(A2BIN)

ALL= $(AG) $(A2)
#Normal build of all libs and binaries
all:: $(ALL) FIN
#Fast build for just AG libs and binaries
ag: $(AG)
	@echo -e "\033[32mAG ONLY Success!\033[m"
#Fast build for just A2 libs and binaries
a2: $(A2)
	@echo -e "\033[32mA2 ONLY Success!\033[m"

debug: MFLAGS += debug
debug: $(ALL) FIN

O2: MFLAGS += O2
O2: $(ALL) FIN

native: MFLAGS += native
native: $(ALL) FIN

cmake: buildrootana
	@mkdir -p ${AGRELEASE}/build
	@cd ${AGRELEASE}/build && cmake3 ../ && make $(MFLAGS) install

FIN: $(ALL)
	@echo -e "\033[32mSuccess!\033[m"

libAGTPC.so: $(DEPS)
	make -C recolib $(MFLAGS)

libaged.so: $(DEPS)
	make -C aged $(MFLAGS)

libagana.so: $(DEPS)
	make -C analib $(MFLAGS)

agana: | $(AGLIBS)
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
