# Makefile for a custom application comprised of
# a loable kernel module and a user space program
# (or a set of related modules and user space programs).
# If your application doesn't need to have a kernel module or
# a user space program, edit the two goals below to
# exclude one or the other.
all		: apps 
#modules
clean		: clean_apps 
#clean_modules

# Edit the line below to modify a set of user-space programs
# you need to build
APPS		= fecdm.exe
apps		: $(APPS)

CROSS_COMPILE_APPS = /home/agdaq/online/firmware/cdm/emcraft/tools/arm-2010q1/bin/arm-uclinuxeabi-

# These are flags/tools used to build user-space programs
CFLAGS		:= -Os -mcpu=cortex-m3 -mthumb -I.
LDFLAGS		:= -mcpu=cortex-m3 -mthumb
CC		= $(CROSS_COMPILE_APPS)gcc
CXX		= $(CROSS_COMPILE_APPS)g++
DEPS		= lmk04800.h
OBJ		= griffin_cdm.o lmk04800.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cxx $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

griffin_cdm: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

fecdm.exe: fecdm.o
	$(CXX) -o $@ $^ $(CFLAGS)

# Clean-up after user-space programs
clean_apps	:
	-rm -f $(APPS) *.gdb *.o

# Edit the line below to modify a set of loadable modules
# you need to build 
#obj-m		+= sample.o

# Define dependencies for a particular module
# sample-y	:= some.o

# Path to the kernel modules directory in context of which
# these loadable modules are built
#KERNELDIR	:=  $(INSTALL_ROOT)/linux

# Build the modules in this directory
#modules		:
#	make -C $(KERNELDIR) M=`pwd` modules CFLAGS="" LDFLAGS=""

# Clean-up after modules
#clean_modules:
#	make -C $(KERNELDIR) M=`pwd` clean
#	rm -f modules.order

