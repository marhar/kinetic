# Generated automatically from Makefile.in by configure.
#-----------------------------------------------------------------------
# unclassified/Makefile -- what should we do with these things?
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

# note: this directory does not currently build

EXE=
O=
TESTS=parsedfiletest filemgrtest sihashtest sshashtest cmdlocktest

all: $(EXE)

executables: $O
	$(CXX) -o kap-t $(TCL_LIB)

parsedfile: parsedfile.o
filemgr: filemgr.o parsedfile.o
kap-t: kap-t.o parsedfile.o filemgr.o cmdlock.o

parsedfiletest: parsedfiletest.o parsedfile.o
cmdlocktest: cmdlocktest.o cmdlock.o
filemgrtest: filemgrtest.o filemgr.o parsedfile.o

clean:
	-rm -f *.o core

clobber: clean
	-rm -f aiws.so $(EXE)

distclean: clobber
	-rm -f Makefile config.status config.cache config.log

install: all
