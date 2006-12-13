# makefile written for gnu make
CXX		= g++
CXXFLAGS	= -Wall -ggdb
VERSION		= 19
LIB		= ./libportaudio.a


# Add your system-dependent network libs here. These are
# only used to build the tests (your application will need them too).
# Linux: none
# Solaris: -lsocket -lnsl
#SYSTEMLIBS	= -lsocket -lnsl
SYSTEMLIBS	=
LDLIBS		= $(LIB) $(SYSTEMLIBS)

OBJ		= pa_convert.o pa_lib.o	pa_trace.o pa_unix.o pa_unix_oss.o 
		  

all:		$(LIB)

$(LIB):		$(OBJ)
		$(AR) $(ARFLAGS) $(LIB) $(OBJ)

doc doxygen:
		doxygen Doxyfile

distclean: clean

clean:
		rm -f *.o
		rm -f *~
		rm -f $(LIB)

