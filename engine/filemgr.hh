//----------------------------------------------------------------------
// filemgr.hh -- file manager header file
//----------------------------------------------------------------------
// This file is part of KAP, the Kinetic Application Processor.
// Copyright (c) 1999, AsiaInfo Technologies, Inc.
//
// See the file kap-license.txt for information on usage and
// redistribution of this file, and for a DISCLAIMER OF ALL
// WARRANTIES.  Share and Enjoy!
//
// The KAP home page is: http://www.markharrison.net/kap
//----------------------------------------------------------------------

#ifndef _FILEMGR_HH
#define _FILEMGR_HH

#include "parsedfile.hh"
#include "sihash.h"

////////////////////////////////////////////////////////////////
// class FileMgr -- file manager
////////////////////////////////////////////////////////////////

class FileMgr
{
private:
    Tcl_Interp* interp;		// logic engine interpreter
    SIhash cache;
    int maxcache;		// how many files to cache
    int ncached;		// how many files are cached

public:
    FileMgr(Tcl_Interp* _interp, int _maxcache);
    int load(char* filename, char*& summary, char*& detail);
};

#endif
