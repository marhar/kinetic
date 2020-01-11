/*----------------------------------------------------------------------
 * cfgfile.hh -- funky config file
 *----------------------------------------------------------------------
 * This file is part of KAP, the Kinetic Application Processor.
 * Copyright (c) 1999, AsiaInfo Technologies, Inc.
 *
 * See the file kap-license.txt for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.  Share and Enjoy!
 *
 * The KAP home page is: http://www.markharrison.net/kap
 *----------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//added by Zhenggang Fan on 7/3/2000 begin:
#include <string.h>
//added by Zhenggang Fan on 7/3/2000 end

typedef struct cfgl {
    char* data;
    struct cfgl* next;
} cfgl;

typedef struct cfgparms {
    int		debug;
    int		timeout;
    int		numengines;
    char*	engine;
    cfgl*	partition;
} cfgparms;

/* user functions */

int cfgread(cfgparms* cp, char* fname);
void cfgfree(cfgparms* cp);
void cfgprint(cfgparms* cp);

/* eof: cfgfile.h */
