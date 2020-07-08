# Makefile

WIRINGPI := $(HOME)/packages/wiringPi/wiringPi

CFLAGS += -O2 -g -std=c++11
CFLAGS += -Wall -Wuninitialized
CFLAGS += -I$(WIRINGPI)

LIBS += $(WIRINGPI)/libwiringPi.a
LIBS += -lpthread
LIBS += -lm

CFLAGS += -I$(MIDASSYS)/include
LIBS   += -L$(MIDASSYS)/linux-arm/lib -lmidas -lrt -lutil

all:: tempdb.exe lvdb.exe feLaser.exe feOxy.exe

%.o: %.cxx
	g++ -c -o $@ $(CFLAGS) $< 

lvdb.o: %.o: %.cxx
	g++ -c -o $@ $(CFLAGS) $< 

tempdb.exe: tempdb.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

lvdb.exe: lvdb.o
	g++ -o $@ $^ $(LIBS)

feLaser.exe: %.exe: %.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LIB) $(LDFLAGS) $(LIBS)

feOxy.exe: %.exe: %.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LIB) $(LDFLAGS) $(LIBS)

fanctrl.exe: fanctrl.o
	g++ -o $@ $^ $(LIBS)

clean::
	-rm -f *.o *.exe

#end
