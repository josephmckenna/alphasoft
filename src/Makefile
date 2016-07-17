INC_DIR   = $(MIDASSYS)/include
LIB_DIR   = $(MIDASSYS)/linux/lib
SRC_DIR   = $(MIDASSYS)/src

CC     = gcc
CFLAGS = -g -Wall -DOS_LINUX -Dextname
CFLAGS += -g -I. -I$(INC_DIR) -I$(MIDASSYS)/../mxml/
LIBS = -lm -lz -lutil -lnsl -lpthread
LIB = $(LIB_DIR)/libmidas.a -lrt

MODULES = $(LIB_DIR)/mfe.o

all: feudp.exe fealpha16.exe

feudp.exe: $(LIB) $(LIB_DIR)/mfe.o feudp.o
	$(CXX) -o $@ $(CFLAGS) $^ $(LIB) $(LDFLAGS) $(LIBS)

fealpha16.exe: $(LIB) $(LIB_DIR)/mfe.o fealpha16.o mscbcxx.o
	$(CXX) -o $@ $(CFLAGS) $^ $(LIB) $(LDFLAGS) $(LIBS)

%.o: %.cxx
	$(CXX) $(CFLAGS) -o $@ -c $<

clean::
	-rm -f *.o *~ \#* *.exe

#end
