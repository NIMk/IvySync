# We don't need kludgy automatizations here,
# let's use a simple Makefile.
# Just tweak the values below to fix your paths
#
# $Id: Makefile 60 2004-11-12 15:40:18Z jaromil $


CC = gcc
CPP = g++
LINKER = ld
# debugging flags:
CPPFLAGS = -I. -Wall -ggdb 

# optimized flags:
# CPPFLAGS = -I. -Wall -O2 -fomit-frame-pointer -ffast-math
LIBS = -lpthread

OBJ = decoder.o thread.o utils.o cmdline.o

all: ivysync

ivysync: decoder.o thread.o utils.o cmdline.o
	$(CPP) $(CPPFLAGS) -o ivysync $(OBJ)

#make clean
clean:
	rm -rf *.o *~ ivysync

# generic make rules
#%: %.c
#	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
#%.o: %.c
#	$(CC) $(CFLAGS) -c -o $@ $<


