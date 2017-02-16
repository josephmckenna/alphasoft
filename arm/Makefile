# Makefile

WIRINGPI := $(HOME)/packages/wiringPi/wiringPi

CFLAGS += -O2 -g
CFLAGS += -Wall -Wuninitialized
CFLAGS += -I$(WIRINGPI)

LIBS += -L$(WIRINGPI) -lwiringPi
LIBS += -lpthread
LIBS += -lm

all:: lvdb.exe

%.o: %.cxx
	g++ -c -o $@ $(CFLAGS) $< 

lvdb.exe: %.exe: %.o
	g++ -o $@ $< $(LIBS)

clean::
	-rm -f *.o *.exe

#end
