/*----------------------------------------------------------------------
 * cfgfile.c -- funky config file
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
 *
 * Any sane person would take the high road to configuration
 * files and just drop in a tcl interpreter to handle
 * the job.  However, netscape is a bit touchy on how it
 * handles plugins, so it's easier all around if we just
 * exclude tcl from the list of potential trouble spots
 * when loading the shared library.  So, let us write
 * yet another quirky, weak configuration processor.
 * The only thing that can be said is that this processor
 * supports a strict subset of tcl.  Supported syntax is:
 *   # for comments
 *   white space ignored
 *   first word of each line is the command
 *   second word of each line is the parameter
 * If necessary, this can be improved, but I would suggest
 * the effort would be better spent integrating Tcl into
 * the application.  For this reason I also lazily put
 * everything into the header file, since it is doubtful
 * there will be more than one of these things per program
 * anyway.
 */

#include "cfgfile.h"

static void setnum(int* ip, char* s)
{
    *ip = atoi(s);
}

static void setstr(char* cp, char* s)
{
    if (cp == 0)
        free(cp);
    if ((cp = (char *)malloc(strlen(s)+1)) == 0) {
        fprintf(stderr, "setstr: malloc");
        abort();
    }
    strcpy(cp, s);
}

static void applist(cfgl* p, char* s)
{
    while (p->next)
        p = p->next;
    p->next = (cfgl*)malloc(sizeof(cfgl));
    p->next->next = 0;
    if ((p->next->data = (char*)malloc(strlen(s)+1)) == 0) {
        fprintf(stderr, "applist: malloc");
        abort();
    }
    strcpy(p->next->data, s);
}

static void freelist(cfgl* p) {
    if (p->next)
        freelist(p->next);
    if (p->data)
        free(p->data);
    free(p);
}

int cfgread(cfgparms* cp, char* fname)
{
    int rc;
    FILE* fp;
    char s[1000];
    char* p;
    char* q;
    char* q2;
    int lineno = 0;

    cp->numengines = 0;
    cp->debug = 0;
    cp->timeout = 0;
    if ((cp->engine = (char*)malloc(100)) == 0) {
        fprintf(stderr, "cfgread: malloc");
        abort();
    }
    strcpy(cp->engine, "undefined");
    if ((cp->partition = (cfgl*)malloc(sizeof(cfgl))) == 0) {
        fprintf(stderr, "cfgread: malloc");
        abort();
    }
    cp->partition->data = 0;
    cp->partition->next = 0;

    if ((fp = fopen(fname, "r")) == 0) {
        printf("KAP: cannot open config file: %s\n", fname);
        rc = 1;
    } else {
        while (fgets(s, 999, fp) != 0) {
            lineno++;
            p = s;
            /* skip any leading whitespace */
            while (isspace(*p) && *p != 0)
                p++;
            /* skip blank lines and comments */
            if (*p == '#' || *p == 0)
                continue;
            q = p;
            while (isgraph(*q) && *q != 0)
                q++;
            *q = 0;
            q++;                    
            /* skip to second word */
            while (isspace(*q) && *q != 0)
                ++q;
            q2 = q;
            while (isgraph(*q2) && *q2 != 0)
                ++q2;
            *q2 = 0;

            if (0)
                ;
            else if (strcmp("debug", p) == 0)
                setnum(&cp->debug, q);
            else if (strcmp("timeout", p) == 0)
                setnum(&cp->timeout, q);
            else if (strcmp("numengines", p) == 0)
                setnum(&cp->numengines, q);
            else if (strcmp("engine", p) == 0) {
                setstr(cp->engine, q);
            }
            else if (strcmp("partition", p) == 0)
                applist(cp->partition, q);
            else {
                printf("KAP: %s(%d) unrecognized parm: %s\n",
                            fname, lineno, p);
            }
        }
        fclose(fp);
        rc = 0;
    }
    return rc;
}

void cfgfree(cfgparms* cp)
{
    free(cp->engine);
    freelist(cp->partition);
}

void cfgprint(cfgparms* cp)
{
    cfgl* p;
    printf("numengines %d\n", cp->numengines);
    printf("debug %d\n", cp->debug);
    printf("timeout %d\n", cp->timeout);
    printf("engine %s\n", cp->engine);
    p = cp->partition;
    while (p->next) {
        printf("partition %s\n", p->next->data);
        p = p->next;
    }
}

/* eof: cfgfile.c */
