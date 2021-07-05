#
# Master Makefile for the ALPHA-g analyzer
#

ifeq (, $(shell which cmake3))
CMAKE := cmake
else
CMAKE := cmake3
endif

cmake:
	@echo $(CMAKE)
	@mkdir -p ${AGRELEASE}/build
	@cd ${AGRELEASE}/build && $(CMAKE) ../ && make $(MFLAGS) install

FIN: $(ALL)
	@echo -e "\033[32mSuccess!\033[m"


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
	make clean -C build
	rm -f ${AGRELEASE}/build ${AGRELEASE}/bin
	rm -rf CMakeCache.txt CMakeFiles

