# Generated automatically from Makefile.in by configure.
#-----------------------------------------------------------------------
# engine/Makefile -- makefile for kap engine
#-----------------------------------------------------------------------
# This file is part of KAP, the Kinetic Application Processor.
# Copyright (c) 1999, AsiaInfo Technologies, Inc.
#
# See the file kap-license.txt for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.  Share and Enjoy!
#
# The KAP home page is: http://www.markharrison.net/kap
#-----------------------------------------------------------------------

CXX=c++
EXE=kap-t
O=cmdlock.o filemgr.o kap-t.o parsedfile.o chdir.o
TCLDIR=/opt/tcl8.0
LIB_RUNTIME_DIR=/opt/tcl8.0/lib
LIBES=-L/opt/tcl8.0/lib -ltcl8.0${TCL_DBGX}  -ldl  -lm

CXXFLAGS= -I$(TCLDIR)/include
CFLAGS= -I$(TCLDIR)/include

all: $(EXE)

kap-t: $O
	$(CXX) -o kap-t $O $(LIBES)

clean:
	-rm -f *.o core

clobber: clean
	-rm -f $(EXE)

distclean: clobber
	-rm -f Makefile config.status config.cache config.log

# install is done in netscape or apache dir, so nothing need
# be done here.
# but now it is told how to install on HPUX by Zhenggang Fan

install: all
	echo nothing
