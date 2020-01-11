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
// filemgrtest.cc -- file manager test program
// Copyright (c) 1999 AsiaInfo.
////////////////////////////////////////////////////////////////

#include "filemgr.hh"

int main(int argc, char** argv)
{
    if (argc < 1) {
        fprintf(stderr, "usage: %s file\n", argv[0]);
        exit(1);
    }

    Tcl_Interp* interp;
    char* comment1;
    char* comment2;
    interp = Tcl_CreateInterp();
    FileMgr fm(interp, 100);

    for (int i = 1; i < argc; ++i) {
        printf("%s\n", argv[i]);
        fm.load(argv[i], comment1, comment2);
    }
    return 0;
}

//eof: filemgrtest.cc
