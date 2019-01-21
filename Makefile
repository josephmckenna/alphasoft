#
# Master Makefile for the ALPHA-g analyzer
#

DEPS = rona
LIBS = libagana.so libAGTPC.so libaged.so 
BIN = agana

all:: $(DEPS) $(LIBS) $(BIN)

libAGTPC.so: $(DEPS)
	make -C reco $(MFLAGS)

libaged.so: $(DEPS)
	make -C aged $(MFLAGS)

libagana.so: $(DEPS)
	make -C analib $(MFLAGS)

agana: | $(LIBS)
	cd ana/ && $(MAKE)

rona:
	make -C rootana obj/manalyzer_main.o lib/librootana.a

html/index.html:
	-mkdir html
	-make -k dox
	touch html/index.html

dox:
	doxygen Doxyfile

clean::
	cd rootana/ && $(MAKE) clean
	cd reco/ && $(MAKE) clean
	cd analib/ && $(MAKE) clean
	cd aged/ && $(MAKE) clean
	cd ana/ && $(MAKE) clean
