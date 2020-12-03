#
# Master Makefile for the ALPHA-g analyzer
#

#If rootana is within this folder, build it...
ifeq (${ROOTANASYS},${AGRELEASE}/rootana)
DEPS = buildrootana
endif

AGLIBS = libanalib.so libagtpc.so libaged.so
AGBIN  = agana reco

A2LIBS = libanalib.so libagtpc.so alpha2libs
A2BIN  = alpha2


AG= $(DEPS) $(AGLIBS) $(AGBIN)
#ALPHA 2 depends on some AG libs
A2= $(DEPS) $(A2LIBS) $(A2BIN)

ALL= $(AG) $(A2) rootUtils.so
#Normal build of all libs and binaries
all:: $(ALL) FIN
#Fast build for just AG libs and binaries
ag: $(AG) rootUtils.so
	@echo -e "\033[32mAG ONLY Success!\033[m"
#Fast build for just A2 libs and binaries
a2: $(A2) rootUtils.so
	@echo -e "\033[32mA2 ONLY Success!\033[m"

debug: MFLAGS += debug
debug: $(ALL) FIN

O2: MFLAGS += O2
O2: $(ALL) FIN

native: MFLAGS += native
native: $(ALL) FIN

ifeq (, $(shell which cmake3))
CMAKE := cmake
else
CMAKE := cmake3
endif

cmake: buildrootana
	@echo $(CMAKE)
	@mkdir -p ${AGRELEASE}/build
	@cd ${AGRELEASE}/build && $(CMAKE) ../ && make $(MFLAGS) install

BINARIES = $(shell ls alpha2/*.modules ana/*.modules)
BINARIES := $(patsubst alpha2/%.modules,bin/%.exe,$(BINARIES))
BINARIES := $(patsubst ana/%.modules,bin/%.exe,$(BINARIES))

cclean:
	make clean -C build
	rm -f $(BINARIES)

FIN: $(ALL)
	@echo -e "\033[32mSuccess!\033[m"

libagtpc.so: $(DEPS)
	make -C recolib $(MFLAGS)

libaged.so: $(DEPS)
	make -C aged $(MFLAGS)

libanalib.so: $(DEPS)
	make -C analib $(MFLAGS)

agana: $(AGLIBS)
	cd ana/ && $(MAKE) $(MFLAGS)

alpha2libs: $(DEPS)
	make -C a2lib $(MFLAGS)

reco: $(LIBS)
	cd reco/ && $(MAKE) $(MFLAGS)

alpha2: $(A2LIBS)
	make -C alpha2 $(MFLAGS)

rootUtils.so:
	make -C rootUtils $(MFLAGS)

buildrootana:
	cd rootana && make obj/manalyzer_main.o
	make -C rootana

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
	cd rootUtils/ && $(MAKE) clean
