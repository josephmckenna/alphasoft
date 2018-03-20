# Makefile

WIRINGPI := $(HOME)/packages/wiringPi/wiringPi

CFLAGS += -O2 -g
CFLAGS += -Wall -Wuninitialized
CFLAGS += -I$(WIRINGPI)

LIBS += -L$(WIRINGPI) -lwiringPi
LIBS += -lpthread
LIBS += -lm

CFLAGS += -I../src
CFLAGS += -I$(MIDASSYS)/include
LIBS   += -L$(MIDASSYS)/linux-arm/lib -lmidas -lrt -lutil

all:: lvdb.exe

%.o: %.cxx
	g++ -c -o $@ $(CFLAGS) $< 

tmfe.o tmodb.o: %.o: ../src/%.cxx
	g++ -c -o $@ $(CFLAGS) $< 

lvdb.exe: %.exe: %.o tmfe.o tmodb.o
	g++ -o $@ $^ $(LIBS)

clean::
	-rm -f *.o *.exe

#end
