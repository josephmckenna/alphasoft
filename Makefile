#
# Master Makefile for the ALPHA-g analyzer
#


LIBS = libagana.so libAGTPC.so libaged.so 
BIN = agana
#ALL = agana

all:: $(LIBS) $(BIN)


libAGTPC.so:
	cd reco/ && $(MAKE)

libaged.so:
	cd aged/ && $(MAKE)

libagana.so:
	cd analib/ && $(MAKE)

agana: | $(LIBS)
	cd ana/ && $(MAKE)

html/index.html:
	-mkdir html
	-make -k dox
	touch html/index.html

dox:
	doxygen Doxyfile

clean::
	cd reco/ && $(MAKE) clean
	cd analib/ && $(MAKE) clean
	cd aged/ && $(MAKE) clean
	cd ana/ && $(MAKE) clean
