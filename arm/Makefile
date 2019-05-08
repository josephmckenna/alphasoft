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

all:: tempdb.exe lvdb.exe feLaser.exe feOxy.exe

%.o: %.cxx
	g++ -c -o $@ $(CFLAGS) $< 

lvdb.o: %.o: %.cxx
	g++ -c -o $@ $(CFLAGS) $< 

tmodb.o: %.o: ../src/%.cxx
	g++ -c -o $@ $(CFLAGS) $< 

tmfe.o: %.o: ../src/%.cxx
	g++ -c -o $@ $(CFLAGS) $< 

tempdb.exe: tempdb.o tmfe.o tmodb.o
	g++ -o $@ $^ $(LIBS)

lvdb.exe: lvdb.o tmodb.o tmfe.o
	g++ -o $@ $^ $(LIBS)

feLaser.exe: %.exe: %.o tmfe.o tmodb.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LIB) $(LDFLAGS) $(LIBS)

feOxy.exe: %.exe: %.o tmfe.o tmodb.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LIB) $(LDFLAGS) $(LIBS)

fanctrl.exe: fanctrl.o tmfe.o tmodb.o
	g++ -o $@ $^ $(LIBS)

clean::
	-rm -f *.o *.exe

#end
