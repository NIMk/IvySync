# makefile written for gnu make
CXX		= g++
CXXFLAGS	= -Wall -ggdb
VERSION		= 0.8
LIB		= ./libxmlrpc++.a

# Add your system-dependent network libs here. These are
# only used to build the tests (your application will need them too).
# Linux: none
# Solaris: -lsocket -lnsl
#SYSTEMLIBS	= -lsocket -lnsl
SYSTEMLIBS	=
LDLIBS		= $(LIB) $(SYSTEMLIBS)

OBJ		= XmlRpcClient.o XmlRpcDispatch.o \
		  XmlRpcServer.o XmlRpcServerConnection.o \
		  XmlRpcServerMethod.o XmlRpcSocket.o XmlRpcSource.o \
		  XmlRpcUtil.o XmlRpcValue.o

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

