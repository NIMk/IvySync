# We don't need kludgy automatizations here,
# let's use a simple Makefile.
# Just tweak the values below to fix your paths
#
# $Id: Makefile 60 2004-11-12 15:40:18Z jaromil $


CC = gcc
LINKER = ld
# debugging flags:
CFLAGS = -I. -Wall -ggdb 

# optimized flags:
# CFLAGS = -I. -Wall -O2 -fomit-frame-pointer -ffast-math
LIBS = -lpthread

all: pvrsync

#make clean
clean:
	rm -rf *.o *~ pvrsync

# generic make rules
%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


