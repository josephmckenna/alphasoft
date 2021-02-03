#
# Master Makefile for the ALPHA-g analyzer
#

# compilation and version info
DEPS = gitinfo fetchfeGEM fetchagana

#If rootana is within this folder, build it...
ifeq (${ROOTANASYS},${AGRELEASE}/rootana)
DEPS += fetchrootana buildrootana
endif

AGLIBS = libanalib.so libagtpc.so libaged.so
AGBIN  = agana reco

A2LIBS = libanalib.so libagtpc.so alpha2libs
A2BIN  = alpha2


AG = $(AGLIBS) $(AGBIN)
#ALPHA 2 depends on some AG libs
A2 = $(A2LIBS) $(A2BIN)

ALL += $(DEPS)
ALL += $(AG) $(A2) rootUtils.so
#Normal build of all libs and binaries
all: $(DEPS) $(ALL) FIN
all: export BUILD_FLAGS:=-DBUILD_AG -DBUILD_A2

#Fast build for just AG libs and binaries
ag: $(DEPS) $(AG) rootUtils.so
ag: export BUILD_FLAGS:=-DBUILD_AG
ag: @echo -e "\033[32mAG ONLY Success!\033[m"
#Fast build for just A2 libs and binaries
a2: $(DEPS) $(A2) rootUtils.so
a2: export BUILD_FLAGS:=-DBUILD_A2
a2: @echo -e "\033[32mA2 ONLY Success!\033[m"

debug: MFLAGS += debug
debug: $(ALL) FIN
debug: export BUILD_FLAGS:=-DBUILD_AG -DBUILD_A2

O2: MFLAGS += O2
O2: $(ALL) FIN

native: MFLAGS += native
native: $(ALL) FIN

ifeq (, $(shell which cmake3))
CMAKE := cmake
else
CMAKE := cmake3
endif

cmake:
	@echo $(CMAKE)
	@mkdir -p ${AGRELEASE}/build
	@cd ${AGRELEASE}/build && $(CMAKE) ../ && make $(MFLAGS) install

BINARIES = $(shell ls alpha2/*.modules ana/*.modules)
BINARIES := $(patsubst alpha2/%.modules,bin/%.exe,$(BINARIES))
BINARIES := $(patsubst ana/%.modules,bin/%.exe,$(BINARIES))

cclean:
	make clean -C build
	rm -f $(BINARIES)
	rm -rf CMakeCache.txt CMakeFiles

FIN: $(ALL)
	@echo -e "\033[32mSuccess!\033[m"

gitinfo:
	./ana/GitInfo.sh ./ana/include

libagtpc.so: $(DEPS)
	$(MAKE) -C recolib $(MFLAGS)

libaged.so: $(DEPS)
	$(MAKE) -C aged $(MFLAGS)

libanalib.so: $(DEPS)
	$(MAKE) -C analib $(MFLAGS)

agana: $(AGLIBS) $(DEPS)
	cd ana/ && $(MAKE) $(MFLAGS)

alpha2libs: $(DEPS)
	$(MAKE) -C a2lib $(MFLAGS)

reco: $(LIBS)
	cd reco/ && $(MAKE) $(MFLAGS)

alpha2: $(A2LIBS) $(DEPS)
	$(MAKE) -C alpha2 $(MFLAGS)

rootUtils.so: $(DEPS)
	$(MAKE) -C rootUtils $(MFLAGS)

buildrootana:
	@echo "Building rootana submodule"
	cd rootana && $(MAKE) $(MFLAGS)

fetchrootana:
	@echo "Fetching rootana submodule"
	git submodule update --init rootana

fetchfeGEM:
	@echo "Fetching feGEM submodule"
	git submodule update --init feGEM

fetchagana:
	@echo "Fetching agana submodule"
	git submodule update --init agana

#$(MAKE) -C rootana $(MFLAGS)
#cd rootana && $(MAKE) obj/manalyzer_main.o
#$(MAKE) -C rootana

html/index.html:
	-mkdir html
	-make -k dox
	touch html/index.html

dox:
	doxygen Doxyfile

clean::
	cd recolib/ && $(MAKE) clean
	cd analib/ && $(MAKE) clean
	cd aged/ && $(MAKE) clean
	cd ana/ && $(MAKE) clean
	cd reco/ && $(MAKE) clean
	cd a2lib/ && $(MAKE) clean
	cd alpha2/ && $(MAKE) clean
	cd rootUtils/ && $(MAKE) clean

ifeq (${ROOTANASYS},${AGRELEASE}/rootana)
clean::
	cd rootana/ && $(MAKE) clean
	rm -f obj/manalyzer_main.o
endif

