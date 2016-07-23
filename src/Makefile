INC_DIR   = $(MIDASSYS)/include
LIB_DIR   = $(MIDASSYS)/linux/lib
SRC_DIR   = $(MIDASSYS)/src

CC     = gcc
CFLAGS = -Wall -O2 -g -Wall -DOS_LINUX -Dextname
CFLAGS += -std=c++11 -Wall -O2 -g -I. -I$(INC_DIR) -I$(MIDASSYS)/../mxml/
LIBS = -lm -lz -lutil -lnsl -lpthread
LIB = $(LIB_DIR)/libmidas.a -lrt

MODULES = $(LIB_DIR)/mfe.o

all:: fealpha16.exe
all:: feevb.exe

fealpha16.exe: $(LIB) $(LIB_DIR)/mfe.o fealpha16.o mscbcxx.o
	$(CXX) -o $@ $(CFLAGS) $^ $(LIB) $(LDFLAGS) $(LIBS)

feevb.exe: $(LIB) $(LIB_DIR)/mfe.o feevb.o
	$(CXX) -o $@ $(CFLAGS) $^ $(LIB) $(LDFLAGS) $(LIBS)

%.o: %.cxx
	$(CXX) $(CFLAGS) -o $@ -c $<

clean::
	-rm -f *.o *~ \#* *.exe

#end
