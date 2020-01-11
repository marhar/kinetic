/*
 * This file is part of KAP, the Kinetic Application Processor.
 * Copyright (c) 1999, AsiaInfo Technologies, Inc.
 *
 * See the file kap-license.txt for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.  Share and Enjoy!
 *
 * The KAP home page is: http://www.markharrison.net/kap
 */

////////////////////////////////////////////////////////////////
// cmdlocktest.cc -- test command locking
////////////////////////////////////////////////////////////////

#include "cmdlock.h"

int myappinit(Tcl_Interp* interp)
{
    CmdLock locker(interp);
    return TCL_OK;
}

int main(int argc, char** argv)
{
    Tcl_Main(argc, argv, myappinit);
}

// eof: cmdlocktest.cc
