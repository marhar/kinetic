//----------------------------------------------------------------------
// filemgr.cc -- file manager
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

#include "filemgr.hh"

////////////////////////////////////////////////////////////////
// FileMgr::FileMgr -- constructor
////////////////////////////////////////////////////////////////

FileMgr::FileMgr(Tcl_Interp* _interp, int _maxcache)
{
    interp = _interp;
    maxcache = _maxcache;
}

#include "stopwatch.hh"

////////////////////////////////////////////////////////////////
// FileMgr::load -- ensure a file has been loaded
////////////////////////////////////////////////////////////////

int FileMgr::load(char* filename, char*& summary, char*& detail)
{
    struct stat statb;
    stopwatch sw;

    // initialize return parameters
    summary = (char *)"(FileMgr::load -- no summary)";
    detail = (char *)"(FileMgr::load -- no detail)";

    // make sure file exists
    if (stat(filename, &statb) < 0) {
        summary = (char *)"file does not exist";
        return 1;
    }

    time_t filetime = statb.st_mtime;
    time_t ourtime = cache.get(filename);

    // if it is not in cache, or if it is out of date, load it

    sw.start();
    if (ourtime == 0 || ourtime < filetime) {
        //dbg("loading %s... ", filename);
        int rc = 0;

        // unload cache if necessary
        if (ncached > maxcache) {
        }

        ParsedFile* pf = new ParsedFile(filename);

        // for debugging
        int dumpfile = 1;
        if (dumpfile) {
            FILE* fp;
            fp = fopen("code.out", "w");
            if (fp) {
                fprintf(fp, "%s", pf->parsed_body());
                fclose(fp);
            }
        }

        if (pf->status() != 0) {
            rc = pf->status() + 100;
            summary = filename;
            detail = pf->parsed_body();
        }
        else {
            int evstat;
            evstat = Tcl_Eval(interp, pf->parsed_body());
            if (evstat != TCL_OK) {
                rc = pf->status() + 200;
                summary = interp->result;
                detail = pf->parsed_body();
            }
            ncached += 1;
            cache.ins(filename, filetime);
        }
        delete pf;
        sw.stop();
        //dbg("%d\n", sw.elapsed());
        return rc;
    }
    else {
        sw.stop();
        //dbg("(cached) %s %d\n", filename, sw.elapsed());
        return 0;
    }
}

//eof: filemgr.cc
