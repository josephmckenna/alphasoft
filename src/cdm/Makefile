# Makefile for CDM MIDAS frontend

CROSS_COMPILE_APPS = /home/agdaq/online/firmware/cdm/emcraft/tools/arm-2010q1/bin/arm-uclinuxeabi-

# These are flags/tools used to build user-space programs
CFLAGS		:= -Os -mcpu=cortex-m3 -mthumb -I. -Wall
LDFLAGS		:= -mcpu=cortex-m3 -mthumb
CC		= $(CROSS_COMPILE_APPS)gcc
CXX		= $(CROSS_COMPILE_APPS)g++
DEPS		= lmk04800.h
OBJ		= griffin_cdm.o lmk04800.o

all:: fecdm.exe

fecdm.exe: fecdm.o lmk04800.o
	$(CXX) -o $@ $^ $(CFLAGS)

#%.o: %.c $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cxx $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

clean::
	-rm -f *.exe *.gdb *.o

#end
