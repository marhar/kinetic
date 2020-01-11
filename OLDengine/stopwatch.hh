//----------------------------------------------------------------------
// stopwatch.hh - simple stopwatch timer class
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

#ifndef _STOPWATCH_HH
#define _STOPWATCH_HH
#include <sys/time.h>
class stopwatch
{
private:
    timeval t0;
    timeval t1;
public:
    void start(void) { gettimeofday(&t0, 0);}
    void stop(void)  { gettimeofday(&t1, 0);}
    long elapsed(void) {
        long sec = t1.tv_sec - t0.tv_sec;
        long usec = t1.tv_usec - t0.tv_usec;
        if (usec < 0) {
            sec -= 1;
            usec += 1000000;
        }
        return sec*1000000 + usec;
    }
};
#endif
